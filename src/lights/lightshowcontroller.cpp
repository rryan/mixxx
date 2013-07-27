
#include <aubio/aubio.h>

#include "lights/lightshowcontroller.h"
#include "lights/controlgroup.h"
#include "lights/lightcontroller.h"
#include "lights/featurestate.h"

LightShowController::LightShowController(LightController* pController)
        : QObject(pController),
          m_pLightController(pController) {
  m_timeSinceLastUpdate = QTime::currentTime();
}

LightShowController::~LightShowController() {

}

void LightShowController::addControlGroup(ControlGroup* pGroup) {
    m_controlGroups.append(pGroup);
}

void LightShowController::process(FeatureState* pState) {

  if (m_timeSinceLastUpdate.msecsTo(pState->current_time) > 10000) {

    // do update

    ControlGroup* pBricks = m_pLightController->getControlGroup(0);

    // Switch between

    // FLASH, FADE250, SET
    // ONSET, BEATX2, BEAT
    // CYCLE, SHIFTRIGHT

    // haxmode
    static int updateMode = 0;
    static int updateTransition = 0;
    static int updateTrigger = 0;

    if (updateMode++ > 5) {
      switch(pBricks->getControlMode()) {
      case CONTROL_OFF:
        pBricks->setControlMode(CONTROL_CYCLE);
        break;
      case CONTROL_CYCLE:
        pBricks->setControlMode(CONTROL_SHIFTER_RIGHT);
        break;
      case CONTROL_SHIFTER_RIGHT:
        pBricks->setControlMode(CONTROL_CYCLE);
        break;
      }
      updateMode = 0;
    }

    if (updateTrigger++ > 2) {
      TriggerMode newMode = pBricks->getTriggerMode();
      switch (pBricks->getTriggerMode()) {
      case ONSET:
        newMode = BEAT;
        break;
      case BEAT:
        newMode = BEAT_DIV2;
        break;
      case BEAT_DIV2:
        newMode = BEAT_X2;
        break;
      case BEAT_X2:
        newMode = ONSET;
        break;
      }
      pBricks->setTriggerMode(newMode);
      updateTrigger = 0;
    }

    if (updateTransition++ > 1) {
      TransitionMode newTrans = pBricks->getTransitionMode();
      switch (newTrans) {
      case TRANSITION_FADE_50MS:
        newTrans = TRANSITION_FLASH_500MS;
        break;
      case TRANSITION_FLASH_500MS:
        newTrans = TRANSITION_FADE_250MS;
        break;
      case TRANSITION_FADE_250MS:
        newTrans = TRANSITION_FADE_50MS;
        break;
      }
      pBricks->setTransitionMode(newTrans);
      updateTransition = 0;
    }
    m_timeSinceLastUpdate = pState->current_time;
  }
    int fft_bins = pState->fft->length;

    int bins_per_group = fft_bins / 3;
    qreal low_sum = 0;
    qreal mid_sum = 0;
    qreal high_sum = 0;

    for (int i = 0; i < bins_per_group; ++i) {
        low_sum += cvec_read_norm(pState->fft, i);
        mid_sum += cvec_read_norm(pState->fft, bins_per_group + i);
        high_sum += cvec_read_norm(pState->fft, 2*bins_per_group + i);
    }

    //qDebug() << "bins per group" << bins_per_group << "fft_bins" << fft_bins;

    low_sum /= bins_per_group;
    mid_sum /= bins_per_group;
    high_sum /= bins_per_group;

    qreal alpha = 0.1f;
    // m_lowFftAverage = m_lowFftAverage * (1-alpha) + alpha * low_sum;
    // m_midFftAverage = m_midFftAverage * (1-alpha) + alpha * mid_sum;
    // m_highFftAverage = m_highFftAverage * (1-alpha) + alpha * high_sum;

    //qDebug() << m_lowFftAverage << m_midFftAverage << m_highFftAverage;
}
