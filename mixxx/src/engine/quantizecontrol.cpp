// quantizecontrol.cpp
// Created on Sat 5, 2011
// Author: pwhelan

#include <QObject>

#include "cachingreader.h"
#include "configobject.h"
#include "controlobject.h"
#include "controlpushbutton.h"
#include "engine/callbackcontrolmanager.h"
#include "engine/enginecontrol.h"
#include "engine/enginestate.h"
#include "engine/quantizecontrol.h"
#include "mathstuff.h"

QuantizeControl::QuantizeControl(const char* pGroup,
                                 EngineState* pEngineState)
        : EngineControl(pGroup, pEngineState->getConfig()) {
    m_pTrackWatcher = pEngineState->getTrackManager()->createTrackWatcher();
    connect(m_pTrackWatcher, SIGNAL(beatsUpdated()),
            this, SLOT(slotBeatsUpdated()),
            Qt::DirectConnection);

    CallbackControlManager* pCallbackControlManager =
            pEngineState->getControlManager();

    // Turn quantize OFF by default. See Bug #898213
    ControlPushButton* pCOQuantizeEnabled = new ControlPushButton(
        ConfigKey(pGroup, "quantize"));
    pCOQuantizeEnabled->setButtonMode(ControlPushButton::TOGGLE);
    m_pCOQuantizeEnabled = pCallbackControlManager->addControl(
        pCOQuantizeEnabled, 1);
    m_pCONextBeat = pCallbackControlManager->addControl(
        new ControlObject(ConfigKey(pGroup, "beat_next")), 1);
    m_pCONextBeat->set(-1);
    m_pCOPrevBeat = pCallbackControlManager->addControl(
        new ControlObject(ConfigKey(pGroup, "beat_prev")), 1);
    m_pCOPrevBeat->set(-1);
    m_pCOClosestBeat = pCallbackControlManager->addControl(
        new ControlObject(ConfigKey(pGroup, "beat_closest")), 1);
    m_pCOClosestBeat->set(-1);
}

QuantizeControl::~QuantizeControl() {
    delete m_pCOQuantizeEnabled;
    delete m_pCONextBeat;
    delete m_pCOPrevBeat;
    delete m_pCOClosestBeat;
    delete m_pTrackWatcher;
}

void QuantizeControl::trackLoaded(TrackPointer pTrack) {
    if (m_pTrack) {
        trackUnloaded(m_pTrack);
    }

    if (pTrack) {
        m_pTrack = pTrack;
        m_pBeats = m_pTrack->getBeats();
        m_pTrackWatcher->watchTrack(m_pTrack);
    }
}

void QuantizeControl::trackUnloaded(TrackPointer pTrack) {
    Q_UNUSED(pTrack);
    if (m_pTrack) {
        m_pTrackWatcher->unwatchTrack(pTrack);
    }
    m_pTrack.clear();
    m_pBeats.clear();
    m_pCOPrevBeat->set(-1);
    m_pCONextBeat->set(-1);
    m_pCOClosestBeat->set(-1);
}

void QuantizeControl::slotBeatsUpdated() {
    //qDebug() << "QuantizeControl::slotBeatsUpdated";
    if (m_pTrack) {
        m_pBeats = m_pTrack->getBeats();
    }
}

double QuantizeControl::process(const double dRate,
                                const double currentSample,
                                const double totalSamples,
                                const int iBufferSize) {
    Q_UNUSED(dRate);
    Q_UNUSED(totalSamples);
    Q_UNUSED(iBufferSize);
    int iCurrentSample = currentSample;
    if (!even(iCurrentSample)) {
        iCurrentSample--;
    }

    if (!m_pBeats) {
        return kNoTrigger;
    }

    double prevBeat = m_pCOPrevBeat->get();
    double nextBeat = m_pCONextBeat->get();
    double closestBeat = m_pCOClosestBeat->get();
    double currentClosestBeat =
            floorf(m_pBeats->findClosestBeat(iCurrentSample));

    if (closestBeat != currentClosestBeat) {
        if (!even(currentClosestBeat)) {
            currentClosestBeat--;
        }
        m_pCOClosestBeat->set(currentClosestBeat);
    }

    if (prevBeat == -1 || nextBeat == -1 ||
        currentSample >= nextBeat || currentSample <= prevBeat) {
        // TODO(XXX) are the floor and even checks necessary?
        nextBeat = floorf(m_pBeats->findNextBeat(iCurrentSample));
        prevBeat = floorf(m_pBeats->findPrevBeat(iCurrentSample));

        if (!even(nextBeat))
            nextBeat--;
        if (!even(prevBeat))
            prevBeat--;

        m_pCONextBeat->set(nextBeat);
        m_pCOPrevBeat->set(prevBeat);
    }

    return kNoTrigger;
}
