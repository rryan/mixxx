#include <QtDebug>

#include "engine/featureextractor.h"
#include "engine/featurecollector.h"
#include "proto/audio_features.pb.h"
#include "util/uptime.h"

#define AUBIO_PITCH_SIZE 512
#define AUBIO_HOP_SIZE 256
#define AUBIO_FFT_WINSIZE 1024
#define AUBIO_SILENCE_THRESHOLD -60.0

EngineBufferFeatureExtractor::EngineBufferFeatureExtractor(const char* pGroup, int iSampleRate)
        : AubioFeatureExtractor(pGroup, iSampleRate),
          m_beatActiveThisFrame(ConfigKey(pGroup, "beat_active_this_frame")),
          m_bpm(ConfigKey(pGroup, "bpm")) {
    // We get our beats from the beat_active_this_frame control.
    m_bReportBeats = false;
}

EngineBufferFeatureExtractor::~EngineBufferFeatureExtractor() {
}

void EngineBufferFeatureExtractor::process(CSAMPLE* pBuffer, const int iNumChannels,
                                           const int iFramesPerBuffer) {
    AubioFeatureExtractor::process(pBuffer, iNumChannels, iFramesPerBuffer);


    FeatureCollector* pCollector = FeatureCollector::instance();
    if (pCollector) {
        mixxx::Features features;
        features.set_time(static_cast<float>(Uptime::uptimeNanos()) / 1e9);
        features.set_group(m_pGroup);
        features.set_beat(m_beatActiveThisFrame.get() > 0);
        double bpm = m_bpm.get();
        if (bpm > 0) {
            features.set_bpm(bpm);
        }
        pCollector->write(features);
    }


}

AubioFeatureExtractor::AubioFeatureExtractor(const char* pGroup, int iSampleRate)
        : FeatureExtractor(pGroup),
          m_bReportBeats(true),
          m_iSampleRate(iSampleRate),
          m_iCurInput(0),
          m_iInputBufferSize(0),
          m_aubio_fft(NULL),
          m_aubio_onset(NULL),
          m_aubio_tempo(NULL),
          m_aubio_pitch(NULL),
          m_input_buf(NULL),
          m_tempo_output(NULL),
          m_onset_output(NULL),
          m_pitch_output(NULL),
          m_fft_output(NULL) {
    init(iSampleRate);
}

AubioFeatureExtractor::~AubioFeatureExtractor() {
    shutdown();
}

void AubioFeatureExtractor::setSampleRate(int iSampleRate) {
    if (iSampleRate != m_iSampleRate) {
        shutdown();
        init(iSampleRate);
    }
}

void AubioFeatureExtractor::shutdown() {
    m_iSampleRate = 0;
    m_iCurInput = 0;
    m_iInputBufferSize = 0;
    del_aubio_onset(m_aubio_onset);
    m_aubio_onset = NULL;
    del_aubio_tempo(m_aubio_tempo);
    m_aubio_tempo = NULL;
    del_aubio_pitch(m_aubio_pitch);
    m_aubio_pitch = NULL;
    del_aubio_fft(m_aubio_fft);
    m_aubio_fft = NULL;
    del_fvec(m_input_buf);
    m_input_buf = NULL;
    del_fvec(m_tempo_output);
    m_tempo_output = NULL;
    del_fvec(m_onset_output);
    m_onset_output = NULL;
    del_fvec(m_pitch_output);
    m_pitch_output = NULL;
    del_cvec(m_fft_output);
    m_fft_output = NULL;
}

void AubioFeatureExtractor::init(int iSampleRate) {
    m_iSampleRate = iSampleRate;
    m_iCurInput = 0;
    m_iInputBufferSize = AUBIO_HOP_SIZE;
    m_input_buf = new_fvec(m_iInputBufferSize);

    char* onset_method = "default";
    m_aubio_onset = new_aubio_onset(onset_method,
                                    AUBIO_FFT_WINSIZE,
                                    m_iInputBufferSize,
                                    m_iSampleRate);
    m_onset_output = new_fvec(1);

    m_aubio_fft = new_aubio_fft(AUBIO_FFT_WINSIZE);
    m_fft_output = new_cvec(AUBIO_FFT_WINSIZE/2);
    m_fft_norm_output = new_fvec(AUBIO_FFT_WINSIZE/2);

    char* tempo_method = "default";
    m_aubio_tempo = new_aubio_tempo(tempo_method,
                                    AUBIO_FFT_WINSIZE,
                                    m_iInputBufferSize,
                                    m_iSampleRate);
    m_tempo_output = new_fvec(2);

    char* pitch_method = "default";
    m_aubio_pitch = new_aubio_pitch(pitch_method,
                                    AUBIO_PITCH_SIZE,
                                    m_iInputBufferSize,
                                    m_iSampleRate);
    aubio_pitch_set_unit(m_aubio_pitch, "midi");
    m_pitch_output = new_fvec(1);
}

void AubioFeatureExtractor::process(CSAMPLE* pBuffer, const int iNumChannels,
                                    const int iFramesPerBuffer) {
    for (int i = 0; i < iFramesPerBuffer; ++i) {
        CSAMPLE monoSample = 0;
        if (iNumChannels == 1) {
            monoSample = pBuffer[i];
        } else if (iNumChannels >= 2) {
            CSAMPLE* pFrame = pBuffer + i*iNumChannels;
            monoSample = 0.5f * (pFrame[0] + pFrame[1]);
        }

        fvec_write_sample(m_input_buf, monoSample, m_iCurInput);

        // We filled up the buffer, process it.
        if (m_iCurInput == m_iInputBufferSize - 1) {
            processBuffer();
            m_iCurInput = -1; // gets inc'd to 0 below
        }
        m_iCurInput++;
    }

}

void AubioFeatureExtractor::processBuffer() {
    aubio_onset_do(m_aubio_onset, m_input_buf, m_onset_output);
    aubio_tempo_do(m_aubio_tempo, m_input_buf, m_tempo_output);
    aubio_pitch_do(m_aubio_pitch, m_input_buf, m_pitch_output);
    aubio_fft_do(m_aubio_fft, m_input_buf, m_fft_output);

    bool is_silence = aubio_silence_detection(
        m_input_buf, AUBIO_SILENCE_THRESHOLD) > 0;
    bool is_beat = fvec_read_sample(m_tempo_output, 0) > 0;
    bool is_tempo_onset = fvec_read_sample(m_tempo_output, 1) > 0;
    bool is_onset = fvec_read_sample(m_onset_output, 0) > 0;
    if (is_onset != is_tempo_onset) {
        qDebug() << "Tempo onset and onset do not agree.";
    }
    float pitch = fvec_read_sample(m_pitch_output, 0);

    FeatureCollector* pCollector = FeatureCollector::instance();
    if (pCollector) {
        mixxx::Features features;
        features.set_time(static_cast<float>(Uptime::uptimeNanos()) / 1e9);
        features.set_group(m_pGroup);
        features.set_silence(is_silence);
        features.set_onset(is_tempo_onset);
        if (m_bReportBeats) {
            features.set_beat(is_beat);
            float bpm = aubio_tempo_get_bpm(m_aubio_tempo);
            if (bpm > 0) {
                features.set_bpm(bpm);
            }
        }
        aubio_fft_get_norm(m_fft_norm_output, m_fft_output);
        for (int i = 0; i < AUBIO_FFT_WINSIZE; ++i) {
            features.add_fft(fvec_read_sample(m_fft_norm_output, i));
        }
        features.set_pitch(pitch);
        pCollector->write(features);
    }
}
