/***************************************************************************
                          enginechannel.cpp  -  description
                             -------------------
    begin                : Sun Apr 28 2002
    copyright            : (C) 2002 by
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

#include "engine/enginechannel.h"

#include "controlobject.h"
#include "controlpushbutton.h"
#include "engine/enginestate.h"

EngineChannel::EngineChannel(const char* pGroup,
                             EngineChannel::ChannelOrientation defaultOrientation,
                             EngineState* pEngineState)
        : m_group(pGroup) {
    CallbackControlManager* pCallbackControlManager =
            pEngineState->getControlManager();
    ControlPushButton* pPFL = new ControlPushButton(ConfigKey(m_group, "pfl"));
    pPFL->setButtonMode(ControlPushButton::TOGGLE);
    m_pPFL = pCallbackControlManager->addControl(pPFL, 1);
    ControlPushButton* pMaster = new ControlPushButton(ConfigKey(m_group, "master"));
    pMaster->setButtonMode(ControlPushButton::TOGGLE);
    m_pMaster = pCallbackControlManager->addControl(pMaster, 1);
    m_pOrientation = pCallbackControlManager->addControl(
        new ControlObject(ConfigKey(m_group, "orientation")), 1);
    m_pOrientation->set(defaultOrientation);
}

EngineChannel::~EngineChannel() {
    delete m_pMaster;
    delete m_pPFL;
    delete m_pOrientation;
}

const QString& EngineChannel::getGroup() const {
    return m_group;
}

bool EngineChannel::isPFL() {
    return m_pPFL->get() > 0.0;
}

bool EngineChannel::isMaster() {
    return m_pMaster->get() > 0.0;
}

EngineChannel::ChannelOrientation EngineChannel::getOrientation() {
    double dOrientation = m_pOrientation->get();
    if (dOrientation == LEFT) {
        return LEFT;
    } else if (dOrientation == CENTER) {
        return CENTER;
    } else if (dOrientation == RIGHT) {
        return RIGHT;
    }
    return CENTER;
}
