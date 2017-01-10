#include "effects/native/reverbeffect.h"

#include <QtDebug>

#include "util/sample.h"

// static
QString ReverbEffect::getId() {
    return "org.mixxx.effects.reverb";
}

// static
EffectManifest ReverbEffect::getManifest() {
    EffectManifest manifest;
    manifest.setId(getId());
    manifest.setName(QObject::tr("Reverb"));
    manifest.setAuthor("The Mixxx Team, CAPS Plugins");
    manifest.setVersion("1.0");
    manifest.setDescription("This is a port of the GPL'ed CAPS Reverb plugin, "
            "which has the following description:"
            "This is based on some of the famous Stanford CCRMA reverbs "
            "(NRev, KipRev) all based on the Chowning/Moorer/Schroeder "
            "reverberators, which use networks of simple allpass and comb"
            "delay filters.");

    EffectManifestParameter* time = manifest.addParameter();
    time->setId("bandwidth");
    time->setName(QObject::tr("Bandwidth"));
    time->setShortName(QObject::tr("BW"));
    time->setDescription(QObject::tr("Bandwidth of the low pass filter at the input. "
            "Higher values result in less attenuation of high frequencies."));
    time->setControlHint(EffectManifestParameter::CONTROL_KNOB_LINEAR);
    time->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    time->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    time->setMinimum(0);
    time->setDefault(1);
    time->setMaximum(1);

    EffectManifestParameter* decay = manifest.addParameter();
    decay->setId("decay");
    decay->setName(QObject::tr("Decay"));
    decay->setDescription(QObject::tr("Lower decay values cause reverberations to die out more quickly."));
    decay->setControlHint(EffectManifestParameter::CONTROL_KNOB_LINEAR);
    decay->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    decay->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    decay->setMinimum(0);
    decay->setDefault(0.5);
    decay->setMaximum(1);

    EffectManifestParameter* damping = manifest.addParameter();
    damping->setId("damping");
    damping->setName(QObject::tr("Damping"));
    damping->setDescription(QObject::tr("Higher damping values cause "
            "high frequencies to decay more quickly than low frequencies."));
    damping->setControlHint(EffectManifestParameter::CONTROL_KNOB_LINEAR);
    damping->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    damping->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    damping->setMinimum(0);
    damping->setDefault(0);
    damping->setMaximum(1);

    EffectManifestParameter* blend = manifest.addParameter();
    blend->setId("blend");
    blend->setName(QObject::tr("Blend"));
    blend->setDescription(QObject::tr("Wet/Dry mix. Higher blend values causes a larger amount of"
            "reverb to be added to the signal path"));
    blend->setControlHint(EffectManifestParameter::CONTROL_KNOB_LINEAR);
    blend->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    blend->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    blend->setDefaultLinkType(EffectManifestParameter::LINK_LINKED);
    blend->setDefaultLinkInversion(EffectManifestParameter::LinkInversion::NOT_INVERTED);
    blend->setMinimum(0);
    blend->setDefault(0);
    blend->setMaximum(1);

    return manifest;
}

ReverbEffect::ReverbEffect(EngineEffect* pEffect,
                             const EffectManifest& manifest)
        : m_pBandWidthParameter(pEffect->getParameterById("bandwidth")),
          m_pDecayParameter(pEffect->getParameterById("decay")),
          m_pDampingParameter(pEffect->getParameterById("damping")),
          m_pBlendParameter(pEffect->getParameterById("blend")) {
    Q_UNUSED(manifest);
}

ReverbEffect::~ReverbEffect() {
    //qDebug() << debugString() << "destroyed";
}

void ReverbEffect::processChannel(const ChannelHandle& handle,
                                ReverbGroupState* pState,
                                const CSAMPLE* pInput, CSAMPLE* pOutput,
                                const unsigned int numSamples,
                                const unsigned int sampleRate,
                                const EffectProcessor::EnableState enableState,
                                const GroupFeatureState& groupFeatures) {
    Q_UNUSED(handle);
    Q_UNUSED(enableState);
    Q_UNUSED(groupFeatures);

    const auto bandwidth = m_pBandWidthParameter->value();
    const auto decay = m_pDecayParameter->value();
    const auto damping = m_pDampingParameter->value();
    const auto blend = m_pBlendParameter->value();

    // Set the sample rate (TODO XXX: Maybe we want to call .init() after this?)
    pState->reverb.setSampleRate(sampleRate);

    // Process the buffer using the new current parameter values
    pState->reverb.processBuffer(pInput, pOutput, numSamples, bandwidth, decay, damping, blend);
}
