#include "engine/simplejogwheel.h"

SimpleJogWheel::SimpleJogWheel(QObject* pParent, const QString& group)
        : QObject(pParent),
          m_scratchController(group),
          m_brake(ConfigKey(group, "brake")),
          m_spinback(ConfigKey(group, "spinback")),
          m_jogScratchMode(ConfigKey(group, "jog_scratch_mode")),
          // Do not ignore no-ops.
          m_jogTicks(ConfigKey(group, "jog_tick"), false),
          m_jogTickSensitivity(ConfigKey(group, "jog_tick_sensitivity")),
          m_scratchAlpha(ConfigKey(group, "jog_scratch_parameter_alpha")),
          m_scratchBeta(ConfigKey(group, "jog_scratch_parameter_beta")),
          m_masterLatency(ConfigKey("[Master]", "latency")),
          m_queueEnable(0),
          m_queueDisable(0),
          m_queueSpinback(0),
          m_queueBrake(0) {
    m_jogScratchMode.setButtonMode(ControlPushButton::TOGGLE);
    m_jogTicks.connectValueChangeRequest(this, SLOT(slotJogTick(double)),
                                         Qt::DirectConnection);

    const double rpm = 33 + 1.0 / 3.0;
    const double ints_per_rev = 128;
    const double scaleFactor = 60.0 / (rpm * ints_per_rev);
    m_jogTickSensitivity.set(scaleFactor);

    //const double kDefaultAlpha = 1.0 / 512.0;
    const double kDefaultAlpha = 1.0 / 8.0;
    //const double kDefaultBeta = 1.0 / 512.0 / 1024.0;
    const double kDefaultBeta = kDefaultAlpha / 20.0;
    m_scratchAlpha.set(kDefaultAlpha);
    m_scratchBeta.set(kDefaultBeta);

    connect(&m_jogScratchMode, SIGNAL(valueChanged(double)),
            this, SLOT(slotJogScratchMode(double)), Qt::DirectConnection);
    connect(&m_brake, SIGNAL(valueChanged(double)),
            this, SLOT(slotBrake(double)), Qt::DirectConnection);
    connect(&m_spinback, SIGNAL(valueChanged(double)),
            this, SLOT(slotSpinback(double)), Qt::DirectConnection);
}

void SimpleJogWheel::slotJogScratchMode(double v) {
    if (v > 0.0) {
        m_queueEnable = 1;
    } else {
        m_queueDisable = 1;
    }
}

void SimpleJogWheel::slotBrake(double v) {
    if (v > 0.0) {
        m_queueBrake = 1;
    }
}

void SimpleJogWheel::slotSpinback(double v) {
    if (v > 0.0) {
        m_queueSpinback = 1;
    }
}

void SimpleJogWheel::slotJogTick(double ticks) {
    if (m_scratchController.enabled()) {
        m_scratchController.tick(ticks * m_jogTickSensitivity.get());
    } else {
        // jog tick
    }
}

void SimpleJogWheel::process() {
    // Default filter values were concluded experimentally for time code vinyl.
    const double dt = m_masterLatency.get() / 1000.0;
    const double alpha = m_scratchAlpha.get();
    const double beta = m_scratchBeta.get();
    const bool ramp = true;
    // We scale ticks ourselves.
    const double scaleFactor = 1.0;

    // // We treat enabled as the combination of these two since isScratching can
    // // return true if a script engine ScratchController is scratching the deck.
    // bool scratchEnabled = m_scratchController.isScratching() &&
    //         m_scratchController.enabled();
    // bool scratchMode = m_jogScratchMode.toBool();
    // if (scratchEnabled != scratchMode) {
    //     if (scratchMode) {
    //         m_scratchController.enable(dt, scaleFactor, alpha, beta, ramp);
    //     } else {
    //         m_scratchController.disable(ramp);
    //     }
    // }

    if (m_queueEnable.testAndSetRelaxed(1, 0)) {
        m_scratchController.enable(dt, scaleFactor, alpha, beta, ramp);
    } else if (m_queueDisable.testAndSetRelaxed(1, 0)) {
        m_scratchController.disable(ramp);
    }

    if (m_scratchController.enabled() || m_scratchController.ramping()) {
        m_scratchController.process();
    }

    if (m_queueBrake.testAndSetRelaxed(1, 0)) {
        m_scratchController.brake(true, dt, 0.9, 1.0, alpha, beta);
    }

    if (m_queueSpinback.testAndSetRelaxed(1, 0)) {
        m_scratchController.brake(true, dt, 1.8, -10.0, alpha, beta);
    }
}
