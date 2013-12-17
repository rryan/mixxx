#ifndef FEATUREEXTRACTOR_H
#define FEATUREEXTRACTOR_H

#include <QString>

#include <aubio/aubio.h>

#include "defs.h"
#include "controlobjectthread.h"

class FeatureExtractor {
  public:
    FeatureExtractor(const char* pGroup)
            : m_pGroup(pGroup) {
    }
    virtual ~FeatureExtractor() {}

    virtual void process(CSAMPLE* pBuffer, const int iNumChannels, const int iSamplePerBuffer) = 0;
    virtual void setSampleRate(int iSampleRate) = 0;

  protected:
    const char* m_pGroup;
};

class AubioFeatureExtractor : public FeatureExtractor {
  public:
    AubioFeatureExtractor(const char* pGroup, int iSampleRate);
    virtual ~AubioFeatureExtractor();

    void setSampleRate(int iSampleRate);
    void init(int iSampleRate);
    void shutdown();
    virtual void process(CSAMPLE* pBuffer, const int iNumChannels, const int iFramesPerBuffer);

  protected:
    bool m_bReportBeats;

  private:
    void processBuffer();

    int m_iSampleRate;
    int m_iCurInput;
    int m_iInputBufferSize;

    aubio_fft_t* m_aubio_fft;
    aubio_onset_t* m_aubio_onset;
    aubio_tempo_t* m_aubio_tempo;
    aubio_pitch_t* m_aubio_pitch;

    fvec_t* m_input_buf;
    fvec_t* m_tempo_output;
    fvec_t* m_onset_output;
    fvec_t* m_pitch_output;
    cvec_t* m_fft_output;
    fvec_t* m_fft_norm_output;
};

class EngineBufferFeatureExtractor : public AubioFeatureExtractor {
  public:
    EngineBufferFeatureExtractor(const char* pGroup, int iSampleRate);
    virtual ~EngineBufferFeatureExtractor();

    virtual void process(CSAMPLE* pBuffer, const int iNumChannels, const int iFramesPerBuffer);

  private:
    ControlObjectThread m_beatActiveThisFrame;
    ControlObjectThread m_bpm;
};

#endif /* FEATUREEXTRACTOR_H */
