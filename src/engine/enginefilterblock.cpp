/***************************************************************************
                          enginefilterblock.cpp  -  description
                             -------------------
    begin                : Thu Apr 4 2002
    copyright            : (C) 2002 by Tue and Ken Haste Andersen
    email                :
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include <QtDebug>

#include "engine/enginefilterblock.h"

#include "controllogpotmeter.h"
#include "controlpushbutton.h"
#include "engine/enginefilter.h"
#include "engine/enginefilterbutterworth8.h"
#include "engine/enginefilteriir.h"
#include "engine/enginestate.h"
#include "sampleutil.h"

EngineFilterBlock::EngineFilterBlock(const char * group,
                                     EngineState* pEngineState) {
    ilowFreq = 0;
    ihighFreq = 0;
    blofi = false;

#ifdef __LOFI__
    low = new EngineFilterIIR(bessel_lowpass4_DJM800,4);
    band = new EngineFilterIIR(bessel_bandpass8_DJM800,8);
    high = new EngineFilterIIR(bessel_highpass4_DJM800,4);
    qDebug() << "Using LoFi EQs";
#else

    //Setup Filter Controls
    CallbackControlManager* pCallbackControlManager =
            pEngineState->getControlManager();

    m_loEqFreq = pCallbackControlManager->getControl(
        ConfigKey("[Mixer Profile]", "LoEQFrequency"));
    if (m_loEqFreq == NULL) {
        m_loEqFreq = pCallbackControlManager->addControl(
            new ControlPotmeter(ConfigKey("[Mixer Profile]", "LoEQFrequency"),
                                0., 22040), 1);
        m_loEqFreq->setParent(this);
    }

    m_hiEqFreq = pCallbackControlManager->getControl(
        ConfigKey("[Mixer Profile]", "HiEQFrequency"));
    if (m_hiEqFreq == NULL) {
        m_hiEqFreq = pCallbackControlManager->addControl(
            new ControlPotmeter(ConfigKey("[Mixer Profile]", "HiEQFrequency"),
                                0., 22040), 1);
        m_hiEqFreq->setParent(this);
    }

    m_lofiEq = pCallbackControlManager->getControl(
        ConfigKey("[Mixer Profile]", "LoFiEQs"));
    if (m_lofiEq == NULL) {
        m_lofiEq = pCallbackControlManager->addControl(
            new ControlPushButton(ConfigKey("[Mixer Profile]", "LoFiEQs")), 1);
        m_lofiEq->setParent(this);
    }

    high = band = low = NULL;

    //Load Defaults
    setFilters(true);

#endif
    /*
       lowrbj = new EngineFilterRBJ();
       lowrbj->calc_filter_coeffs(6, 100., 44100., 0.3, 0., true);
       midrbj = new EngineFilterRBJ();
       midrbj->calc_filter_coeffs(6, 1000., 44100., 0.3, 0., true);
       highrbj = new EngineFilterRBJ();
       highrbj->calc_filter_coeffs(8, 10000., 48000., 0.3, 0., true);

       lowrbj = new EngineFilterRBJ();
       lowrbj->calc_filter_coeffs(0, 100., 48000., 0.3., 0., false);
       highrbj = new EngineFilterRBJ();
       highrbj->calc_filter_coeffs(1, 10000., 48000., 0.3., 0., false);
     */

    filterpotLow = pCallbackControlManager->addControl(
        new ControlLogpotmeter(ConfigKey(group, "filterLow"), 4.), 1);
    ControlPushButton* pFilterKillLow = new ControlPushButton(
        ConfigKey(group, "filterLowKill"));
    pFilterKillLow->setButtonMode(ControlPushButton::POWERWINDOW);
    filterKillLow = pCallbackControlManager->addControl(pFilterKillLow, 1);

    filterpotMid = pCallbackControlManager->addControl(
        new ControlLogpotmeter(ConfigKey(group, "filterMid"), 4.), 1);
    ControlPushButton* pFilterKillMid = new ControlPushButton(
        ConfigKey(group, "filterMidKill"));
    pFilterKillMid->setButtonMode(ControlPushButton::POWERWINDOW);
    filterKillMid = pCallbackControlManager->addControl(pFilterKillMid, 1);

    filterpotHigh = pCallbackControlManager->addControl(
        new ControlLogpotmeter(ConfigKey(group, "filterHigh"), 4.), 1);
    ControlPushButton* pFilterKillHigh = new ControlPushButton(
        ConfigKey(group, "filterHighKill"));
    pFilterKillHigh->setButtonMode(ControlPushButton::POWERWINDOW);
    filterKillHigh = pCallbackControlManager->addControl(pFilterKillHigh, 1);

    m_pTemp1 = new CSAMPLE[MAX_BUFFER_LEN];
    m_pTemp2 = new CSAMPLE[MAX_BUFFER_LEN];
    m_pTemp3 = new CSAMPLE[MAX_BUFFER_LEN];

    memset(m_pTemp1, 0, sizeof(CSAMPLE) * MAX_BUFFER_LEN);
    memset(m_pTemp2, 0, sizeof(CSAMPLE) * MAX_BUFFER_LEN);
    memset(m_pTemp3, 0, sizeof(CSAMPLE) * MAX_BUFFER_LEN);
}

EngineFilterBlock::~EngineFilterBlock()
{
    delete high;
    delete band;
    delete low;
    delete [] m_pTemp3;
    delete [] m_pTemp2;
    delete [] m_pTemp1;
    delete filterpotLow;
    delete filterKillLow;
    delete filterpotMid;
    delete filterKillMid;
    delete filterpotHigh;
    delete filterKillHigh;
}

void EngineFilterBlock::setFilters(bool forceSetting) {
    if((ilowFreq != (int)m_loEqFreq->get()) ||
       (ihighFreq != (int)m_hiEqFreq->get()) ||
       (blofi != (int)m_lofiEq->get()) || forceSetting)
    {
        delete low;
        delete band;
        delete high;
        ilowFreq = (int)m_loEqFreq->get();
        ihighFreq = (int)m_hiEqFreq->get();
        blofi = (int)m_lofiEq->get();
        if(blofi)
        {
            // why is this DJM800 at line ~34 (LOFI ifdef) and just
            // bessel_lowpass# here? bkgood
            low = new EngineFilterIIR(bessel_lowpass4,4);
            band = new EngineFilterIIR(bessel_bandpass,8);
            high = new EngineFilterIIR(bessel_highpass4,4);
        }
        else
        {
            low = new EngineFilterButterworth8(FILTER_LOWPASS, 44100, (int)m_loEqFreq->get());
            band = new EngineFilterButterworth8(FILTER_BANDPASS, 44100, (int)m_loEqFreq->get(), (int)m_hiEqFreq->get());
            high = new EngineFilterButterworth8(FILTER_HIGHPASS, 44100, (int)m_hiEqFreq->get());
        }

    }
}

void EngineFilterBlock::process(const CSAMPLE * pIn, const CSAMPLE * pOut,
                                const int iBufferSize) {
    CSAMPLE * pOutput = (CSAMPLE *)pOut;
    float fLow = 0.f, fMid = 0.f, fHigh = 0.f;

    if (filterKillLow->get()==0.)
        fLow = filterpotLow->get(); //*0.7;
    if (filterKillMid->get()==0.)
        fMid = filterpotMid->get(); //*1.1;
    if (filterKillHigh->get()==0.)
        fHigh = filterpotHigh->get(); //*1.2;

#ifndef __LOFI__
    setFilters();
#endif

    low->process(pIn, m_pTemp1, iBufferSize);
    band->process(pIn, m_pTemp2, iBufferSize);
    high->process(pIn, m_pTemp3, iBufferSize);

    SampleUtil::copy3WithGain(pOutput,
                              m_pTemp1, fLow,
                              m_pTemp2, fMid,
                              m_pTemp3, fHigh, iBufferSize);
}

