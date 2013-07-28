#include "engine/clockcontrol.h"

#include "controlobject.h"
#include "configobject.h"
#include "cachingreader.h"
#include "engine/enginecontrol.h"

ClockControl::ClockControl(const char* pGroup, ConfigObject<ConfigValue>* pConfig)
        : EngineControl(pGroup, pConfig) {
    m_pCOBeatActive = new ControlObject(ConfigKey(pGroup, "beat_active"));
    m_pCOBeatActive->set(0.0f);
    m_pCOBeatActiveThisFrame = new ControlObject(ConfigKey(pGroup, "beat_active_this_frame"));
    m_pCOBeatActiveThisFrame->set(0.0f);
    m_pCOSampleRate = ControlObject::getControl(ConfigKey("[Master]","samplerate"));
}

ClockControl::~ClockControl() {
    delete m_pCOBeatActive;
}

void ClockControl::trackLoaded(TrackPointer pTrack) {
    // Clear on-beat control
    m_pCOBeatActive->set(0.0f);

    // Disconnect any previously loaded track/beats
    if (m_pTrack) {
        disconnect(m_pTrack.data(), SIGNAL(beatsUpdated()),
                   this, SLOT(slotBeatsUpdated()));
    }
    m_pBeats.clear();
    m_pTrack.clear();

    if (pTrack) {
        m_pTrack = pTrack;
        m_pBeats = m_pTrack->getBeats();
        connect(m_pTrack.data(), SIGNAL(beatsUpdated()),
                this, SLOT(slotBeatsUpdated()));
    }
}

void ClockControl::trackUnloaded(TrackPointer pTrack) {
    Q_UNUSED(pTrack)
    trackLoaded(TrackPointer());
}

void ClockControl::slotBeatsUpdated() {
    if(m_pTrack) {
        m_pBeats = m_pTrack->getBeats();
    }
}

double ClockControl::process(const double dRate,
                             const double currentSample,
                             const double totalSamples,
                             const int iBuffersize) {
    Q_UNUSED(totalSamples);
    Q_UNUSED(iBuffersize);
    double samplerate = m_pCOSampleRate->get();

    // TODO(XXX) should this be customizable, or latency dependent?
    const double blinkSeconds = 0.100;

    // Multiply by two to get samples from frames. Interval is scaled linearly
    // by the rate.
    const double blinkIntervalSamples = 2.0 * samplerate * (1.0 * dRate) * blinkSeconds;

    if (m_pBeats) {
        double closestBeat = m_pBeats->findClosestBeat(currentSample);
        double distanceToBeat = closestBeat - currentSample;
        bool beatThisFrame = distanceToBeat >= 0 && distanceToBeat <= iBuffersize;
        m_pCOBeatActiveThisFrame->set(beatThisFrame ? 1 : 0);
        m_pCOBeatActive->set(fabs(distanceToBeat) < blinkIntervalSamples / 2.0);
    }

    return kNoTrigger;
}
