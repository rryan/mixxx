#include <QtDebug>

#include "engine/audioscene.h"
#include "engine/enginechannel.h"

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

ALsizei BytesToFrames(ALsizei size, ALenum channels, ALenum type)
{
    return size / FramesToBytes(1, channels, type);
}

AudioScene::AudioScene()
        : m_pDevice(NULL),
          m_pContext(NULL),
          m_iFrameSize(0) {
    initialize();
}

AudioScene::~AudioScene() {
    shutdown();
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
    attrs[1] = ALC_STEREO_SOFT;
    attrs[2] = ALC_FORMAT_TYPE_SOFT;
    attrs[3] = ALC_FLOAT_SOFT;
    attrs[4] = ALC_FREQUENCY;
    attrs[5] = 44100; // TODO
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
        qDebug() << "WARNING: Couldn't make OpenAL context current.";
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

void AudioScene::shutdown() {
    if (m_pContext) {
        qDebug() << "Destroying OpenAL context.";
        alcDestroyContext(m_pContext);
        m_pContext = NULL;
    }
    if (m_pDevice) {
        qDebug() << "Destroying OpenAL device.";
        alcCloseDevice(m_pDevice);
        m_pDevice = NULL;
    }
    m_iFrameSize = 0;
}

void AudioScene::addEmitter(EngineChannel* pChannel) {
    m_emitters.push_back(new AudioEmitter(pChannel));
}

void AudioScene::addListener() {
    QString group = QString("[Dome%1]").arg(m_listeners.size() + 1);
    m_listeners.push_back(new AudioListener(group));
}

CSAMPLE* AudioScene::buffer(const AudioOutput& output) {
    unsigned char index = output.getIndex();
    if (index < m_listeners.size()) {
        return m_listeners[index]->buffer();
    }
    return NULL;
}
