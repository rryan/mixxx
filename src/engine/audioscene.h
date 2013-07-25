#ifndef AUDIOSCENE_H
#define AUDIOSCENE_H

#include <QObject>
#include <QList>

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>

#include "defs.h"
#include "sampleutil.h"
#include "soundmanagerutil.h"
#include "engine/enginechannel.h"
#include "control/vectorcontrol.h"

inline void QVector3DToALfloat(const QVector3D& vector, ALfloat* pVector) {
    pVector[0] = vector.x();
    pVector[1] = vector.y();
    pVector[2] = vector.z();
}

class AudioEntity {
  public:
    virtual ~AudioEntity() { }
    virtual void position(ALfloat* pPosition) = 0;
    virtual void velocity(ALfloat* pVelocity) = 0;
    virtual void orientation(ALfloat* pOrientation) = 0;
};

class AudioEmitter : public AudioEntity {
  public:
    AudioEmitter(EngineChannel* pChannel) : m_pChannel(pChannel) {}
    virtual ~AudioEmitter() {}

    void position(ALfloat* pPosition) {
        QVector3D vec = m_pChannel->position();
        QVector3DToALfloat(vec, pPosition);
    }

    void orientation(ALfloat* pOrientation) {
        QVector3D vec = m_pChannel->orientation();
        QVector3DToALfloat(vec, pOrientation);
    }

    void velocity(ALfloat* pVelocity) {
        QVector3D vec = m_pChannel->velocity();
        QVector3DToALfloat(vec, pVelocity);
    }

  private:
    EngineChannel* m_pChannel;
};

class AudioListener : public AudioEntity {
  public:
    AudioListener(const QString& group)
            : m_position(ConfigKey(group, "position")),
              m_orientation(ConfigKey(group, "orientation")),
              m_velocity(ConfigKey(group, "velocity")),
              m_pBuffer(SampleUtil::alloc(MAX_BUFFER_LEN)) {
        SampleUtil::applyGain(m_pBuffer, 0, MAX_BUFFER_LEN);
    }
    virtual ~AudioListener() {
        SampleUtil::free(m_pBuffer);
        m_pBuffer = NULL;
    }

    CSAMPLE* buffer() {
        return m_pBuffer;
    }

    void position(ALfloat* pPosition) {
        QVector3D vec = m_position.get();
        QVector3DToALfloat(vec, pPosition);
    }

    void orientation(ALfloat* pOrientation) {
        QVector3D vec = m_orientation.get();
        QVector3DToALfloat(vec, pOrientation);
    }

    void velocity(ALfloat* pVelocity) {
        QVector3D vec = m_velocity.get();
        QVector3DToALfloat(vec, pVelocity);
    }

  private:
    Vector3DControl m_position;
    Vector3DControl m_orientation;
    Vector3DControl m_velocity;
    CSAMPLE* m_pBuffer;
};

class AudioScene : public QObject {
  public:
    AudioScene();
    virtual ~AudioScene();

    void addEmitter(EngineChannel* pChannel);
    void addListener();

    CSAMPLE* buffer(const AudioOutput& output);

    bool initialize();
    void shutdown();
    void process();

  private:
    QList<AudioEmitter*> m_emitters;
    QList<AudioListener*> m_listeners;

    ALCdevice* m_pDevice;
    ALCcontext* m_pContext;
    ALCsizei m_iFrameSize;
};

#endif /* AUDIOSCENE_H */
