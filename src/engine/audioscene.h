#ifndef AUDIOSCENE_H
#define AUDIOSCENE_H

#include <QObject>
#include <QMap>
#include <QList>

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>

#include "configobject.h"
#include "defs.h"
#include "sampleutil.h"
#include "soundmanagerutil.h"
#include "engine/enginechannel.h"
#include "controlobject.h"
#include "control/vectorcontrol.h"
#include "engine/enginemaster.h"

inline void QVector3DTofloat(const QVector3D& vector, float* pVector) {
    pVector[0] = vector.x();
    pVector[1] = vector.y();
    pVector[2] = vector.z();
}

class AudioEntity {
  public:
    virtual ~AudioEntity() { }
    virtual void position(float* pPosition) = 0;
    virtual void velocity(float* pVelocity) = 0;
    virtual void orientation(float* pOrientation) = 0;
};

class AudioScene;

class AudioEmitter : public AudioEntity {
  public:
    AudioEmitter(AudioScene* pScene, EngineChannel* pChannel);
    virtual ~AudioEmitter();

    void process(int iNumFrames);
    void receiveBuffer(CSAMPLE* pBuffer, const int iNumFrames, const int iSampleRate);

    void position(float* pPosition) {
        QVector3D vec = m_pChannel->position();
        QVector3DTofloat(vec, pPosition);
    }

    void orientation(float* pOrientation) {
        QVector3D vec = m_pChannel->orientation();
        QVector3DTofloat(vec, pOrientation);
    }

    void velocity(float* pVelocity) {
        QVector3D vec = m_pChannel->velocity();
        QVector3DTofloat(vec, pVelocity);
    }

  private:
    AudioScene* m_pScene;
    EngineChannel* m_pChannel;
    CSAMPLE* m_pConversion;
};

class DistanceBasedGainCalculator : public EngineMaster::GainCalculator {
  public:
    DistanceBasedGainCalculator()
            : m_distance_model(AL_NONE) {
    }

    void setPosition(const QVector3D& position) {
        m_position = position;
    }

    void setDistanceModel(ALenum model) {
        m_distance_model = model;
    }

    void setReferenceDistance(qreal distance) {
        m_reference_distance = distance;
    }

    void setRolloffFactor(qreal factor) {
        m_rolloff_factor = factor;
    }

    double getGain(EngineMaster::ChannelInfo* pChannelInfo) const;

  private:
    QVector3D m_position;
    qreal m_reference_distance;
    qreal m_rolloff_factor;
    ALenum m_distance_model;
};

class AudioListener : public AudioEntity {
  public:
    explicit AudioListener(const QString& group, AudioScene* pScene);
    virtual ~AudioListener();

    void position(float* pPosition) {
        QVector3D vec = m_position.get();
        QVector3DTofloat(vec, pPosition);
    }

    void orientation(float* pOrientation) {
        QVector3D vec = m_orientation.get();
        QVector3DTofloat(vec, pOrientation);
    }

    void velocity(float* pVelocity) {
        QVector3D vec = m_velocity.get();
        QVector3DTofloat(vec, pVelocity);
    }

    void loadSettingsFromConfig(ConfigObject<ConfigValue>* pConfig);
    void process(const QList<EngineMaster::ChannelInfo*>& channels,
                 unsigned int channelBitvector,
                 unsigned int maxChannels,
                 const int iNumFrames);

    QString m_group;
    AudioScene* m_pScene;
    Vector3DControl m_position;
    Vector3DControl m_orientation;
    Vector3DControl m_velocity;
    CSAMPLE* m_pBuffer;
    CSAMPLE* m_pStereoBuffer;
    QList<CSAMPLE> m_channelGainCache;
    DistanceBasedGainCalculator m_gain;
    ControlObject m_distanceModel;
};

class AudioScene : public QObject {
  public:
    AudioScene(ConfigObject<ConfigValue>* pConfig, int sampleRate);
    virtual ~AudioScene();

    void addEmitter(EngineChannel* pChannel);

    CSAMPLE* buffer(const AudioOutput& output);

    bool initialize();
    void shutdown();
    void onCallbackStart();
    void process(const QList<EngineMaster::ChannelInfo*>& channels,
                 unsigned int channelBitvector,
                 unsigned int maxChannels,
                 const int iNumFrames);
    void receiveBuffer(const QString& group, CSAMPLE* pBuffer, const int iNumFrames,
                       const int iSampleRate);

  private:
    void loadSettingsFromConfig();

    ConfigObject<ConfigValue>* m_pConfig;
    ControlObject m_rolloffFactor;
    ControlObject m_referenceDistance;
    ControlObject m_distanceModel;
    int m_iSampleRate;
    QMap<QString, AudioEmitter*> m_emitters;
    QMap<QString, AudioListener*> m_listeners;

    friend class AudioEmitter;
    friend class AudioListener;
};

#endif /* AUDIOSCENE_H */
