#ifndef AUDIOSCENE_H
#define AUDIOSCENE_H

#include <QObject>
#include <QMap>
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
    AudioEmitter(EngineChannel* pChannel);
    virtual ~AudioEmitter();

    void process(int iNumFrames);
    void receiveBuffer(CSAMPLE* pBuffer, const int iNumFrames, const int iSampleRate);

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
    CSAMPLE* m_pConversion;
    QList<ALuint> m_buffers;
    ALuint m_source;
};

class AudioListener : public AudioEntity {
  public:
    explicit AudioListener(const QString& group);
    virtual ~AudioListener();

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

    Vector3DControl m_position;
    Vector3DControl m_orientation;
    Vector3DControl m_velocity;
};

class AudioScene : public QObject {
  public:
    AudioScene(int sampleRate);
    virtual ~AudioScene();

    void addEmitter(EngineChannel* pChannel);

    CSAMPLE* buffer(const AudioOutput& output);

    bool initialize();
    void shutdown();
    void onCallbackStart();
    void process(const int iNumFrames);
    void receiveBuffer(const QString& group, CSAMPLE* pBuffer, const int iNumFrames,
                       const int iSampleRate);

  private:
    int m_iSampleRate;
    QMap<QString, AudioEmitter*> m_emitters;
    QList<CSAMPLE*> m_buffers;
    CSAMPLE* m_pInterleavedBuffer;
    AudioListener m_listener;

    ALCdevice* m_pDevice;
    ALCcontext* m_pContext;
    ALCsizei m_iFrameSize;
};

#endif /* AUDIOSCENE_H */
