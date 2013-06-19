#include "engine/clockcontrol.h"

#include "cachingreader.h"
#include "configobject.h"
#include "controlobject.h"
#include "engine/callbackcontrolmanager.h"
#include "engine/enginecontrol.h"
#include "engine/enginestate.h"

ClockControl::ClockControl(const char* pGroup,
                           EngineState* pEngineState)
        : EngineControl(pGroup, pEngineState->getConfig()) {
    m_pTrackWatcher = pEngineState->getTrackManager()->createTrackWatcher();
    connect(m_pTrackWatcher, SIGNAL(beatsUpdated()),
            this, SLOT(slotBeatsUpdated()),
            Qt::DirectConnection);
    CallbackControlManager* pCallbackControlManager =
            pEngineState->getControlManager();
    m_pCOBeatActive = pCallbackControlManager->addControl(
        new ControlObject(ConfigKey(pGroup, "beat_active")), 1);
    m_pCOSampleRate = pCallbackControlManager->getControl(
        ConfigKey("[Master]", "samplerate"));
}

ClockControl::~ClockControl() {
    delete m_pCOBeatActive;
    delete m_pTrackWatcher;
}

void ClockControl::trackLoaded(TrackPointer pTrack) {
    // Clear on-beat control
    m_pCOBeatActive->set(0.0f);

    // Disconnect any previously loaded track/beats
    if (m_pTrack) {
        m_pTrackWatcher->unwatchTrack(m_pTrack);
    }
    m_pBeats.clear();
    m_pTrack.clear();

    if (pTrack) {
        m_pTrack = pTrack;
        m_pBeats = m_pTrack->getBeats();
        m_pTrackWatcher->watchTrack(m_pTrack);
    }
}

void ClockControl::trackUnloaded(TrackPointer pTrack) {
    Q_UNUSED(pTrack)
    trackLoaded(TrackPointer());
}

void ClockControl::slotBeatsUpdated() {
    //qDebug() << "ClockControl::slotBeatsUpdated";
    if (m_pTrack) {
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
        double distanceToClosestBeat = fabs(currentSample - closestBeat);
        m_pCOBeatActive->set(distanceToClosestBeat < blinkIntervalSamples / 2.0);
    }

    return kNoTrigger;
}
