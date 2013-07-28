#ifndef FEATUREEXTRACTOR_H
#define FEATUREEXTRACTOR_H

#include <QString>

#include <aubio/aubio.h>

#include "defs.h"

class FeatureExtractor {
  public:
    virtual ~FeatureExtractor() {}
    virtual void process(CSAMPLE* pBuffer, const int iNumChannels, const int iSamplePerBuffer) = 0;
    virtual void setSampleRate(int iSampleRate) = 0;
};

class AubioFeatureExtractor : public FeatureExtractor {
  public:
    AubioFeatureExtractor(const char* pGroup, int iSampleRate);
    virtual ~AubioFeatureExtractor();

    void setSampleRate(int iSampleRate);
    void init(int iSampleRate);
    void shutdown();
    void process(CSAMPLE* pBuffer, const int iNumChannels, const int iFramesPerBuffer);

  private:
    void processBuffer();

    QString m_group;
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
};

#endif /* FEATUREEXTRACTOR_H */
