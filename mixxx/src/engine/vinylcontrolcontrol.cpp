#include "engine/vinylcontrolcontrol.h"

#include "engine/callbackcontrolmanager.h"
#include "engine/enginestate.h"
#include "library/dao/cue.h"
#include "mathstuff.h"
#include "vinylcontrol/vinylcontrol.h"

VinylControlControl::VinylControlControl(
    const char* pGroup, EngineState* pEngineState)
        : EngineControl(pGroup, pEngineState->getConfig()) {
    CallbackControlManager* pCallbackControlManager =
            pEngineState->getControlManager();
    pCallbackControlManager->addControl(
        new ControlObject(ConfigKey(pGroup, "vinylcontrol_status")), 1)
            ->setParent(this);
    ControlObject* pControlVinylSpeedType =
            new ControlObject(ConfigKey(pGroup, "vinylcontrol_speed_type"));
    // Convert the ConfigKey's value into a double for the CO (for fast reads).
    QString strVinylSpeedType = pEngineState->getConfig()->getValueString(
        ConfigKey(pGroup, "vinylcontrol_speed_type"));
    if (strVinylSpeedType == MIXXX_VINYL_SPEED_33) {
        pControlVinylSpeedType->set(MIXXX_VINYL_SPEED_33_NUM);
    } else if (strVinylSpeedType == MIXXX_VINYL_SPEED_45) {
        pControlVinylSpeedType->set(MIXXX_VINYL_SPEED_45_NUM);
    } else {
        pControlVinylSpeedType->set(MIXXX_VINYL_SPEED_33_NUM);
    }
    pCallbackControlManager->addControl(pControlVinylSpeedType, 1)
            ->setParent(this);

    ControlObject* pControlVinylSeek =
            new ControlObject(ConfigKey(pGroup, "vinylcontrol_seek"));
    connect(pControlVinylSeek, SIGNAL(valueChanged(double)),
            this, SLOT(slotControlVinylSeek(double)),
            Qt::DirectConnection);
    pCallbackControlManager->addControl(pControlVinylSeek, 1)
            ->setParent(this);

    ControlPushButton* pControlVinylEnabled =
            new ControlPushButton(ConfigKey(pGroup, "vinylcontrol_enabled"));
    pControlVinylEnabled->setButtonMode(ControlPushButton::TOGGLE);
    m_pControlVinylEnabled = pCallbackControlManager->addControl(
        pControlVinylEnabled, 1);

    ControlPushButton* pControlVinylWantEnabled = new ControlPushButton(
        ConfigKey(pGroup, "vinylcontrol_wantenabled"));
    pControlVinylWantEnabled->setButtonMode(ControlPushButton::TOGGLE);
    CallbackControl* pCallbackControlVinylWantEnabled =
            pCallbackControlManager->addControl(pControlVinylWantEnabled, 1);
    pCallbackControlVinylWantEnabled->setParent(this);

    ControlPushButton* pControlVinylMode = new ControlPushButton(
        ConfigKey(pGroup, "vinylcontrol_mode"));
    pControlVinylMode->setStates(3);
    pControlVinylMode->setButtonMode(ControlPushButton::TOGGLE);
    m_pControlVinylMode = pCallbackControlManager->addControl(
        pControlVinylMode, 1);

    ControlPushButton* pControlVinylCueing = new ControlPushButton(
        ConfigKey(pGroup, "vinylcontrol_cueing"));
    pControlVinylCueing->setStates(3);
    pControlVinylCueing->setButtonMode(ControlPushButton::TOGGLE);
    m_pControlVinylCueing = pCallbackControlManager->addControl(
        pControlVinylCueing, 1);

    ControlPushButton* pControlVinylSignalEnabled = new ControlPushButton(
        ConfigKey(pGroup, "vinylcontrol_signal_enabled"));
    pControlVinylSignalEnabled->set(1);
    pControlVinylSignalEnabled->setButtonMode(ControlPushButton::TOGGLE);
    CallbackControl* pCallbackControlVinylSignalEnabled =
            pCallbackControlManager->addControl(pControlVinylSignalEnabled, 1);
    pCallbackControlVinylSignalEnabled->setParent(this);

    ControlPushButton* pControlVinylScratching = new ControlPushButton(
        ConfigKey(pGroup, "vinylcontrol_scratching"));
    pControlVinylScratching->set(0);
    pControlVinylScratching->setButtonMode(ControlPushButton::TOGGLE);
    m_pControlVinylScratching = pCallbackControlManager->addControl(
        pControlVinylScratching, 1);
}

VinylControlControl::~VinylControlControl() {
    delete m_pControlVinylEnabled;
    delete m_pControlVinylMode;
    delete m_pControlVinylCueing;
}

void VinylControlControl::trackLoaded(TrackPointer pTrack) {
    m_pCurrentTrack = pTrack;
}

void VinylControlControl::trackUnloaded(TrackPointer pTrack) {
    Q_UNUSED(pTrack);
    m_pCurrentTrack.clear();
}

void VinylControlControl::slotControlVinylSeek(double change) {
    if(isnan(change) || change > 1.14 || change < -1.14) {
        // This seek is ridiculous.
        return;
    }

    double total_samples = getTotalSamples();
    double new_playpos = round(change*total_samples);

    // Do nothing if no track is loaded.
    if (!m_pCurrentTrack) {
        return;
    }

    if (m_pControlVinylEnabled->get() > 0.0 && m_pControlVinylMode->get() == MIXXX_VCMODE_RELATIVE) {
        int cuemode = (int)m_pControlVinylCueing->get();

        //if in preroll, always seek
        if (new_playpos < 0) {
            seek(change);
            return;
        } else if (cuemode == MIXXX_RELATIVE_CUE_OFF) {
            return;  //if off, do nothing
        } else if (cuemode == MIXXX_RELATIVE_CUE_ONECUE) {
            //if onecue, just seek to the regular cue
            seekAbs(m_pCurrentTrack->getCuePoint());
            return;
        }

        double distance = 0;
        int nearest_playpos = -1;

        QList<Cue*> cuePoints = m_pCurrentTrack->getCuePoints();
        QListIterator<Cue*> it(cuePoints);
        while (it.hasNext()) {
            Cue* pCue = it.next();
            if (pCue->getType() != Cue::CUE || pCue->getHotCue() == -1) {
                continue;
            }

            int cue_position = pCue->getPosition();
            //pick cues closest to new_playpos
            if ((nearest_playpos == -1) ||
                (fabs(new_playpos - cue_position) < distance)) {
                nearest_playpos = cue_position;
                distance = fabs(new_playpos - cue_position);
            }
        }

        if (nearest_playpos == -1) {
            if (new_playpos >= 0) {
                //never found an appropriate cue, so don't seek?
                return;
            }
            //if negative, allow a seek by falling down to the bottom
        } else {
            seekAbs(nearest_playpos);
            return;
        }
    }

    // just seek where it wanted to originally
    seek(change);
}

bool VinylControlControl::isEnabled()
{
    return m_pControlVinylEnabled->get();
}

bool VinylControlControl::isScratching()
{
    return m_pControlVinylScratching->get();
}
