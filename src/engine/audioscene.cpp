#include <QtDebug>

#include "engine/audioscene.h"
#include "engine/enginechannel.h"
#include "util/timer.h"
#include "engine/featurecollector.h"
#include "util/uptime.h"
#include "engine/channelmixer.h"

#define AUDIO_SCENE "[AudioScene]"

bool parse3DVectorFromString(const QString& vector, QVector3D* pResult) {
    QStringList components = vector.split(",");
    if (components.size() != 3) {
        return false;
    }

    bool ok = false;
    float x = components[0].toFloat(&ok);
    if (!ok) {
        return false;
    }
    pResult->setX(x);

    float y = components[1].toFloat(&ok);
    if (!ok) {
        return false;
    }
    pResult->setY(y);

    float z = components[2].toFloat(&ok);
    if (!ok) {
        return false;
    }
    pResult->setZ(z);

    return true;
}

AudioEmitter::AudioEmitter(AudioScene* pScene, EngineChannel* pChannel)
        : m_pScene(pScene),
          m_pChannel(pChannel),
          m_pConversion(SampleUtil::alloc(MAX_BUFFER_LEN)) {
    SampleUtil::applyGain(m_pConversion, 0, MAX_BUFFER_LEN);
}

AudioEmitter::~AudioEmitter() {
    SampleUtil::free(m_pConversion);
    m_pConversion = NULL;
}

void AudioEmitter::process(int iNumFrames) {
    ALfloat vec[3];
    position(vec);

    FeatureCollector* pCollector = FeatureCollector::instance();
    if (pCollector) {
        mixxx::Features features;
        features.set_time(static_cast<float>(Uptime::uptimeNanos()) / 1e9);
        features.set_group(m_pChannel->getGroup().toStdString());
        features.add_pos(vec[0]);
        features.add_pos(vec[1]);
        features.add_pos(vec[2]);
        pCollector->write(features);
    }
}

void AudioEmitter::receiveBuffer(CSAMPLE* pBuffer, const int iNumFrames,
                                 const int iSampleRate) {
}

AudioListener::AudioListener(const QString& group)
        : m_group(group),
          m_position(ConfigKey(group, "position")),
          m_orientation(ConfigKey(group, "orientation")),
          m_velocity(ConfigKey(group, "velocity")),
          m_pBuffer(SampleUtil::alloc(MAX_BUFFER_LEN)) {
    SampleUtil::applyGain(m_pBuffer, 0, MAX_BUFFER_LEN);
}

AudioListener::~AudioListener() {
    SampleUtil::free(m_pBuffer);
}

void AudioListener::process() {
    FeatureCollector* pCollector = FeatureCollector::instance();
    if (pCollector) {
        mixxx::Features features;
        features.set_time(static_cast<float>(Uptime::uptimeNanos()) / 1e9);
        features.set_group(m_group.toStdString());
        features.add_pos(m_position.x());
        features.add_pos(m_position.y());
        features.add_pos(m_position.z());
        pCollector->write(features);
    }
}

void AudioListener::loadSettingsFromConfig(ConfigObject<ConfigValue>* pConfig) {
    QString listenerPosition = pConfig->getValueString(
        ConfigKey(m_group, "position"), "500,500,0");
    QVector3D pos;
    if (parse3DVectorFromString(listenerPosition, &pos)) {
        m_position.set(pos);
    }
}

AudioScene::AudioScene(ConfigObject<ConfigValue>* pConfig, int sampleRate)
        : m_pConfig(pConfig),
          m_rolloffFactor(ConfigKey(AUDIO_SCENE, "rolloff_factor")),
          m_referenceDistance(ConfigKey(AUDIO_SCENE, "reference_distance")),
          m_distanceModel(ConfigKey(AUDIO_SCENE, "distance_model")),
          m_iSampleRate(sampleRate) {
    const int num_channels = 6;
    for (int i = 0; i < num_channels; ++i) {
        QString group = QString("[Dome%1]").arg(i+1);
        m_listeners.insert(group, new AudioListener(group));
    }

    initialize();
    loadSettingsFromConfig();
}

AudioScene::~AudioScene() {
    shutdown();
}

bool AudioScene::initialize() {
    return true;
}

void AudioScene::loadSettingsFromConfig() {
    bool ok = false;

    QString referenceDistance = m_pConfig->getValueString(
        ConfigKey(AUDIO_SCENE, "reference_distance"), "");
    double dReferenceDistance = referenceDistance.toDouble(&ok);
    if (!referenceDistance.isEmpty() && ok) {
        m_referenceDistance.set(dReferenceDistance);
        //qDebug() << "REFERENCE_DISTANCE" << m_referenceDistance.get();
    }

    QString rolloffFactor = m_pConfig->getValueString(
        ConfigKey(AUDIO_SCENE, "rolloff_factor"), "");
    double dRolloffFactor = rolloffFactor.toDouble(&ok);
    if (!rolloffFactor.isEmpty() && ok) {
        m_rolloffFactor.set(dRolloffFactor);
        //qDebug() << "ROLLOFF_FACTOR" << m_rolloffFactor.get();
    }

    QString distanceMode = m_pConfig->getValueString(
        ConfigKey(AUDIO_SCENE, "distance_mode"), "AL_NONE");
    QHash<QString, ALenum> distanceMap;
    distanceMap["AL_INVERSE_DISTANCE"] = AL_INVERSE_DISTANCE;
    distanceMap["AL_INVERSE_DISTANCE_CLAMPED"] = AL_INVERSE_DISTANCE_CLAMPED;
    distanceMap["AL_LINEAR_DISTANCE"] = AL_LINEAR_DISTANCE;
    distanceMap["AL_LINEAR_DISTANCE_CLAMPED"] = AL_LINEAR_DISTANCE_CLAMPED;
    distanceMap["AL_EXPONENT_DISTANCE"] = AL_EXPONENT_DISTANCE;
    distanceMap["AL_EXPONENT_DISTANCE_CLAMPED"] = AL_EXPONENT_DISTANCE_CLAMPED;
    distanceMap["AL_NONE"] = AL_NONE;
    //qDebug() << "alDistanceModel" << distanceMode;
    m_distanceModel.set(distanceMap.value(distanceMode, AL_NONE));

    for (QMap<QString, AudioListener*>::const_iterator it = m_listeners.begin();
         it != m_listeners.end(); ++it) {
        it.value()->loadSettingsFromConfig(m_pConfig);
    }
}

void AudioScene::shutdown() {
}

void AudioScene::onCallbackStart() {
    for (QMap<QString, AudioListener*>::const_iterator it = m_listeners.begin();
         it != m_listeners.end(); ++it) {
        SampleUtil::applyGain(it.value()->m_pBuffer, 0, MAX_BUFFER_LEN);
    }
}

void AudioScene::receiveBuffer(const QString& group, CSAMPLE* pBuffer,
                               const int iNumFrames, const int iSampleRate) {
    ScopedTimer t("AudioScene::receiveBuffer");

    // // Passthrough for testing.
    // const int num_buffers = m_buffers.size();
    // for (int j = 0; j < num_buffers; ++j) {
    //     CSAMPLE* pOutBuffer = m_buffers[j];
    //     for (int i = 0; i < iNumFrames; ++i) {
    //         pOutBuffer[i] += pBuffer[i*2];
    //     }
    // }

    AudioEmitter* pEmitter = m_emitters.value(group, NULL);
    if (pEmitter == NULL) {
        qDebug() << "No group registered for:" << group;
        return;
    }
    pEmitter->process(iNumFrames);
    pEmitter->receiveBuffer(pBuffer, iNumFrames, iSampleRate);
}

void AudioScene::process(const QList<EngineMaster::ChannelInfo*>& channels,
                         unsigned int channelBitvector,
                         unsigned int maxChannels,
                         const int iNumFrames) {
    ScopedTimer t("AudioScene::process");


    for (QMap<QString, AudioListener*>::const_iterator it = m_listeners.begin();
         it != m_listeners.end(); ++it) {
        it.value()->process();
    }
}

void AudioScene::addEmitter(EngineChannel* pChannel) {
    m_emitters.insert(pChannel->getGroup(), new AudioEmitter(this, pChannel));
}

CSAMPLE* AudioScene::buffer(const AudioOutput& output) {
    unsigned char index = output.getIndex();
    QString group = QString("[Dome%1]").arg(index + 1);
    AudioListener* pListener = m_listeners.value(group, NULL);
    if (pListener != NULL) {
        return pListener->m_pBuffer;
    }
    return NULL;
}
