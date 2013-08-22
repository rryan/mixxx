#include <QtDebug>

#include "engine/audioscene.h"
#include "engine/enginechannel.h"
#include "util/timer.h"
#include "engine/featurecollector.h"
#include "util/uptime.h"

#define AUDIO_SCENE "[AudioScene]"

#define LOAD_PROC(x,t)  ((x) = (t)alcGetProcAddress(NULL, #x))
static LPALCLOOPBACKOPENDEVICESOFT alcLoopbackOpenDeviceSOFT;
static LPALCISRENDERFORMATSUPPORTEDSOFT alcIsRenderFormatSupportedSOFT;
static LPALCRENDERSAMPLESSOFT alcRenderSamplesSOFT;

const char *ChannelsName(ALenum chans)
{
    switch(chans)
    {
    case AL_MONO_SOFT: return "Mono";
    case AL_STEREO_SOFT: return "Stereo";
    case AL_REAR_SOFT: return "Rear";
    case AL_QUAD_SOFT: return "Quadraphonic";
    case AL_5POINT1_SOFT: return "5.1 Surround";
    case AL_6POINT1_SOFT: return "6.1 Surround";
    case AL_7POINT1_SOFT: return "7.1 Surround";
    }
    return "Unknown Channels";
}

const char *TypeName(ALenum type)
{
    switch(type)
    {
    case AL_BYTE_SOFT: return "S8";
    case AL_UNSIGNED_BYTE_SOFT: return "U8";
    case AL_SHORT_SOFT: return "S16";
    case AL_UNSIGNED_SHORT_SOFT: return "U16";
    case AL_INT_SOFT: return "S32";
    case AL_UNSIGNED_INT_SOFT: return "U32";
    case AL_FLOAT_SOFT: return "Float32";
    case AL_DOUBLE_SOFT: return "Float64";
    }
    return "Unknown Type";
}

ALsizei FramesToBytes(ALsizei size, ALenum channels, ALenum type)
{
    switch(channels)
    {
    case AL_MONO_SOFT:    size *= 1; break;
    case AL_STEREO_SOFT:  size *= 2; break;
    case AL_REAR_SOFT:    size *= 2; break;
    case AL_QUAD_SOFT:    size *= 4; break;
    case AL_5POINT1_SOFT: size *= 6; break;
    case AL_6POINT1_SOFT: size *= 7; break;
    case AL_7POINT1_SOFT: size *= 8; break;
    }

    switch(type)
    {
    case AL_BYTE_SOFT:           size *= sizeof(ALbyte); break;
    case AL_UNSIGNED_BYTE_SOFT:  size *= sizeof(ALubyte); break;
    case AL_SHORT_SOFT:          size *= sizeof(ALshort); break;
    case AL_UNSIGNED_SHORT_SOFT: size *= sizeof(ALushort); break;
    case AL_INT_SOFT:            size *= sizeof(ALint); break;
    case AL_UNSIGNED_INT_SOFT:   size *= sizeof(ALuint); break;
    case AL_FLOAT_SOFT:          size *= sizeof(ALfloat); break;
    case AL_DOUBLE_SOFT:         size *= sizeof(ALdouble); break;
    }

    return size;
}

void transform_to_rhs(ALfloat* vec3) {
    // in firemix's coordinate system, y is z.
    ALfloat y = vec3[1];
    vec3[1] = vec3[2];
    vec3[2] = y;
}

ALsizei BytesToFrames(ALsizei size, ALenum channels, ALenum type)
{
    return size / FramesToBytes(1, channels, type);
}

AudioEmitter::AudioEmitter(AudioScene* pScene, EngineChannel* pChannel)
        : m_pScene(pScene),
          m_pChannel(pChannel),
          m_pConversion(SampleUtil::alloc(MAX_BUFFER_LEN)) {
    SampleUtil::applyGain(m_pConversion, 0, MAX_BUFFER_LEN);

    const int numBuffers = 10;

    while (m_buffers.size() < numBuffers) {
        ALuint buffer = 0;
        alGenBuffers(1, &buffer);
        if (alGetError() != AL_NO_ERROR) {
            qDebug() << "Failed to create buffer.";
            return;
        }
        qDebug() << "Created buffer:" << buffer;
        m_buffers.push_back(buffer);
    }

    alGenSources(1, &m_source);
    ALenum err = alGetError();
    if (err != AL_NO_ERROR) {
        qDebug() << "Failed to create source." << err;
        return;
    }

    // Set parameters so mono sources play out the front-center speaker and
    // won't distance attenuate.
    alSource3f(m_source, AL_POSITION, 0, 0, 0);
    alSource3f(m_source, AL_VELOCITY, 0, 0, 0);
    alSource3f(m_source, AL_DIRECTION, 0, 0, 0);
    //alSourcei(m_source, AL_SOURCE_RELATIVE, AL_TRUE);
    //alSourcei(m_source, AL_ROLLOFF_FACTOR, 0.8);
    //alSourcei(m_source, AL_ROLLOFF_FACTOR, 0);

    alSourceRewind(m_source);
    alSourcei(m_source, AL_BUFFER, 0);
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

    bool print = m_pChannel->getGroup() == "[Channel1]" && false;
    if (print) {
        qDebug() << m_pChannel->getGroup() << vec[0] << vec[1] << vec[2];
    }
    transform_to_rhs(vec);
    alSourcefv(m_source, AL_POSITION, vec);

    velocity(vec);
    transform_to_rhs(vec);
    alSourcefv(m_source, AL_VELOCITY, vec);

    orientation(vec);
    transform_to_rhs(vec);
    alSourcefv(m_source, AL_DIRECTION, vec);

    alSourcef(m_source, AL_GAIN, 1.0);
    alSourcef(m_source, AL_MIN_GAIN, 0.0);
    alSourcef(m_source, AL_MAX_GAIN, 2.0);
    alSourcef(m_source, AL_MAX_DISTANCE, 1000);

    if (print) {
        qDebug() << m_pChannel->getGroup() << "rolloff" << m_pScene->m_rolloffFactor.get();
    }
    alSourcef(m_source, AL_ROLLOFF_FACTOR, m_pScene->m_rolloffFactor.get());
    if (print) {
        qDebug() << m_pChannel->getGroup() << "reference_distance" << m_pScene->m_referenceDistance.get();
    }
    alSourcef(m_source, AL_REFERENCE_DISTANCE, m_pScene->m_referenceDistance.get());
    if (print) {
        qDebug() << m_pChannel->getGroup() << "distance model" << m_pScene->m_distanceModel.get();
    }
    alSourcei(m_source, AL_DISTANCE_MODEL, m_pScene->m_distanceModel.get());
}

void AudioEmitter::receiveBuffer(CSAMPLE* pBuffer, const int iNumFrames,
                                 const int iSampleRate) {
    for (int i = 0; i < iNumFrames; ++i) {
        m_pConversion[i] = (pBuffer[i*2]+ pBuffer[i*2 + 1]) * 0.5/ SHRT_MAX;
        if (m_pConversion[i] > 1.0 || m_pConversion[i] < -1.0) {
            qDebug() << "receive buffer clipped";
        }
    }

    if (m_buffers.size() == 0) {
        ALint processed = 0;
        alGetSourcei(m_source, AL_BUFFERS_PROCESSED, &processed);

        while (processed > 0) {
            ALuint bufid = 0;
            alSourceUnqueueBuffers(m_source, 1, &bufid);
            processed--;
            m_buffers.push_back(bufid);
        }
    }

    if (m_buffers.size() > 0) {
        ALuint buffer = m_buffers.takeFirst();
        // TODO(rryan): samplerate
        alBufferData(buffer, AL_FORMAT_MONO_FLOAT32, m_pConversion,
                     iNumFrames * sizeof(CSAMPLE), iSampleRate);

        if (alGetError() != AL_NO_ERROR) {
            //qDebug() << "Error buffering data:" << alGetError();
        }

        alSourceQueueBuffers(m_source, 1, &buffer);

        if (alGetError() != AL_NO_ERROR) {
            qDebug() << "Error queueing data:" << alGetError();
        }
    } else {
        qDebug() << "Buffer overrun in AudioScene.";
    }

    ALint state;
    alGetSourcei(m_source, AL_SOURCE_STATE, &state);

    if (state != AL_PLAYING && state != AL_PAUSED) {
        ALint queued;
        /* If no buffers are queued, playback is finished */
        alGetSourcei(m_source, AL_BUFFERS_QUEUED, &queued);
        if (queued == 0) {
            return;
        }

        alSourcePlay(m_source);
        if (alGetError() != AL_NO_ERROR) {
            qDebug() << "Error restarting playback";
            return;
        }
    }
}

AudioListener::AudioListener(const QString& group)
        : m_position(ConfigKey(group, "position")),
          m_orientation(ConfigKey(group, "orientation")),
          m_velocity(ConfigKey(group, "velocity")) {
}

AudioListener::~AudioListener() {
}

AudioScene::AudioScene(ConfigObject<ConfigValue>* pConfig, int sampleRate)
        : m_pConfig(pConfig),
          m_rolloffFactor(ConfigKey(AUDIO_SCENE, "rolloff_factor")),
          m_referenceDistance(ConfigKey(AUDIO_SCENE, "reference_distance")),
          m_distanceModel(ConfigKey(AUDIO_SCENE, "distance_model")),
          m_iSampleRate(sampleRate),
          m_pInterleavedBuffer(NULL),
          m_listener(AUDIO_SCENE),
          m_pDevice(NULL),
          m_pContext(NULL),
          m_iFrameSize(0) {

    const int num_channels = 6;
    m_pInterleavedBuffer = SampleUtil::alloc(num_channels * MAX_BUFFER_LEN);
    SampleUtil::applyGain(m_pInterleavedBuffer, 0, num_channels * MAX_BUFFER_LEN);
    for (int i = 0; i < num_channels; ++i) {
        CSAMPLE* pBuffer = SampleUtil::alloc(MAX_BUFFER_LEN);
        SampleUtil::applyGain(pBuffer, 0, MAX_BUFFER_LEN);
        m_buffers.push_back(pBuffer);
    }

    initialize();
    loadSettingsFromConfig();
}

AudioScene::~AudioScene() {
    shutdown();
    SampleUtil::free(m_pInterleavedBuffer);
    m_pInterleavedBuffer = NULL;
    while (m_buffers.size() > 0) {
        SampleUtil::free(m_buffers.takeLast());
    }
}


bool AudioScene::initialize() {
    if (!(alcIsExtensionPresent(NULL, "ALC_SOFT_loopback"))) {
        qDebug() << "WARNING: ALC_SOFT_loopback not supported.";
        shutdown();
    }

    LOAD_PROC(alcLoopbackOpenDeviceSOFT, LPALCLOOPBACKOPENDEVICESOFT);
    LOAD_PROC(alcIsRenderFormatSupportedSOFT, LPALCISRENDERFORMATSUPPORTEDSOFT);
    LOAD_PROC(alcRenderSamplesSOFT, LPALCRENDERSAMPLESSOFT);

    if (!alcLoopbackOpenDeviceSOFT ||
        !alcIsRenderFormatSupportedSOFT ||
        !alcRenderSamplesSOFT) {
        qDebug() << "WARNING: Couldn't load OpenAL functions.";
        shutdown();
        return false;
    }

    m_pDevice = alcLoopbackOpenDeviceSOFT(NULL);

    if (!m_pDevice) {
        qDebug() << "WARNING: Couldn't open OpenAL loopback device.";
        shutdown();
        return false;
    }


    ALCint attrs[16];
    attrs[0] = ALC_FORMAT_CHANNELS_SOFT;
    attrs[1] = ALC_5POINT1_SOFT;
    attrs[2] = ALC_FORMAT_TYPE_SOFT;
    attrs[3] = ALC_FLOAT_SOFT;
    attrs[4] = ALC_FREQUENCY;
    attrs[5] = m_iSampleRate;
    attrs[6] = 0;

    if (alcIsRenderFormatSupportedSOFT(m_pDevice, attrs[5], attrs[1], attrs[3]) == ALC_FALSE) {
        qDebug() << "WARNING: OpenAL render format is not supported.";
        shutdown();
        return false;
    }

    m_pContext = alcCreateContext(m_pDevice, attrs);

    if (!m_pContext) {
        qDebug() << "WARNING: Couldn't create OpenAL context.";
        shutdown();
        return false;
    }

    if (alcMakeContextCurrent(m_pContext) == ALC_FALSE) {
        qDebug() << "WARNING: Couldn't make listener OpenAL context current.";
        shutdown();
        return false;
    }

    m_iFrameSize = FramesToBytes(1, attrs[1], attrs[3]);
    qDebug() << "Successfully initialized AudioScene"
             << "Channels:" << ChannelsName(attrs[1])
             << "Sample Type:" << TypeName(attrs[3])
             << "Sample Rate:" << attrs[5]
             << "Frame Size:" << m_iFrameSize;

    return true;
}

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


    QString listenerPosition = m_pConfig->getValueString(
        ConfigKey(AUDIO_SCENE, "listener_position"), "500,500,0");
    QVector3D pos;

    if (parse3DVectorFromString(listenerPosition, &pos)) {
        m_listener.m_position.set(pos);
    }
}

void AudioScene::shutdown() {
    if (m_pDevice) {
        qDebug() << "Destroying OpenAL device.";
        alcCloseDevice(m_pDevice);
        m_pDevice = NULL;
    }
    if (m_pContext) {
        qDebug() << "Destroying OpenAL context.";
        alcDestroyContext(m_pContext);
        m_pContext = NULL;
    }
    m_iFrameSize = 0;
}

void AudioScene::onCallbackStart() {
    foreach (CSAMPLE* pBuffer, m_buffers) {
        SampleUtil::applyGain(pBuffer, 0, MAX_BUFFER_LEN);
    }

    alDistanceModel(m_distanceModel.get());
    if (alGetError() != AL_NO_ERROR) {
        qDebug() << "Could not set distance model to:" << m_distanceModel.get();
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

void AudioScene::process(const int iNumFrames) {
    ScopedTimer t("AudioScene::process");

    ALfloat vec[3];

    m_listener.position(vec);

    FeatureCollector* pCollector = FeatureCollector::instance();
    if (pCollector) {
        mixxx::Features features;
        features.set_time(static_cast<float>(Uptime::uptimeNanos()) / 1e9);
        features.set_group("[AudioScene]");
        features.add_pos(vec[0]);
        features.add_pos(vec[1]);
        features.add_pos(vec[2]);
        pCollector->write(features);
    }

    //qDebug() << "alListenerfv AL_POSITION" << vec[0] << vec[1] << vec[2];
    transform_to_rhs(vec);
    alListenerfv(AL_POSITION, vec);

    m_listener.velocity(vec);
    transform_to_rhs(vec);
    alListenerfv(AL_VELOCITY, vec);

    //m_listener.orientation(vec);
    ALfloat orientation[6];
    // At: looking at origin
    orientation[0] = 0;
    orientation[1] = 0;
    orientation[2] = 0;
    transform_to_rhs(orientation);

    // Up: Vertical
    orientation[3] = 0;
    orientation[4] = 1;
    orientation[5] = 0;
    //alListenerfv(AL_ORIENTATION, vec);

    SampleUtil::applyGain(m_pInterleavedBuffer, 0, iNumFrames * m_buffers.size());
    alcRenderSamplesSOFT(m_pDevice, m_pInterleavedBuffer, iNumFrames);

    // De-interleave the buffer.
    const int num_buffers = m_buffers.size();
    for (int j = 0; j < num_buffers; ++j) {
        CSAMPLE* pBuffer = m_buffers[j];
        for (int i = 0; i < iNumFrames; ++i) {
            pBuffer[i] = m_pInterleavedBuffer[i * num_buffers + j];
            if (pBuffer[i] > 1.0 || pBuffer[i] < -1.0) {
                qDebug() << "deinterleave clipped.";
            }
            pBuffer[i] *= SHRT_MAX;
        }
    }
}

void AudioScene::addEmitter(EngineChannel* pChannel) {
    m_emitters.insert(pChannel->getGroup(), new AudioEmitter(this, pChannel));
}

CSAMPLE* AudioScene::buffer(const AudioOutput& output) {
    unsigned char index = output.getIndex();
    if (index < m_buffers.size()) {
        return m_buffers[index];
    }
    return NULL;
}
