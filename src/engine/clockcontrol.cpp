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
    m_dOldNextBeat = 0;
    m_dLastPosition = 0;
    m_bEmitted = false;
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
        double nextBeat = m_pBeats->findNextBeat(currentSample);
        double prevBeat = m_pBeats->findPrevBeat(currentSample);

        double nextDist = nextBeat - currentSample;
        double prevDist = currentSample - prevBeat;

        bool beatThisFrame = (nextDist >= 0 && nextDist <= iBuffersize) || prevDist == 0;
        m_pCOBeatActiveThisFrame->set(beatThisFrame ? 1 : 0);

        double closestBeat = (nextDist > prevDist) ? prevBeat : nextBeat;
        double distanceToBeat = closestBeat - currentSample;
        m_pCOBeatActive->set(fabs(distanceToBeat) < blinkIntervalSamples / 2.0);


        // Sometimes we miss beats. Check that we emitted the last beat that was
        // the next beat.
        if (m_dOldNextBeat == nextBeat) {
            if (beatThisFrame) {
                m_bEmitted = true;
            }
        } else if (m_dOldNextBeat != nextBeat) {
            if (m_bEmitted) {
                m_bEmitted = false;
            } else {
                // qDebug() << "Didn't emit for beat" << m_dOldNextBeat
                //          << "lastPos" << m_dLastPosition
                //          << "prev" << prevBeat
                //          << "next" << nextBeat
                //          << "prevdist" << prevDist
                //          << "current" << currentSample
                //          << "iBuffersize" << iBuffersize;
                // lolhax
                m_pCOBeatActiveThisFrame->set(1);
            }
            m_dOldNextBeat = nextBeat;
        }

        m_dLastPosition = currentSample;

    }

    return kNoTrigger;
}
