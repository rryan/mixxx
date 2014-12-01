#include <QtDebug>

#include "engine/scratchcontroller.h"

#include "util/time.h"
#include "util/math.h"

const bool sDebug = true;

ScratchController::ScratchController(const QString& group)
        : m_ticks(0),
          m_tickScaleFactor(0.0),
          m_bEnabled(false),
          m_bRamping(false),
          m_dRampTo(0.0),
          m_dRampFactor(0.0),
          m_lastMovementNs(0),
          m_bBrakeActive(false),
          m_play(group, "play"),
          m_rate(group, "rate"),
          m_rateDir(group, "rate_dir"),
          m_rateRange(group, "rateRange"),
          m_reverse(group, "reverse"),
          m_scratchEnable(group, "scratch2_enable"),
          m_scratchRate(group, "scratch2") {
}

bool ScratchController::isScratching() const {
    // Don't report that we are scratching if we're ramping.
    return m_scratchEnable.toBool() && !m_bRamping;
}

void ScratchController::enable(double dt, double tickScaleFactor,
                               double alpha, double beta, bool ramp) {
    if (sDebug) {
        qDebug() << "ScratchController::enable" << dt << tickScaleFactor
                 << alpha << beta << ramp;
    }
    m_bEnabled = true;
    m_ticks = 0;
    m_tickScaleFactor = tickScaleFactor;
    m_bRamping = false;
    // By default, ramp factor is dt.
    m_dRampFactor = dt;
    m_bBrakeActive = false;

    // Ramp velocity, default to stopped.
    double initialVelocity = 0.0;

    // If ramping is desired, figure out the deck's current speed
    if (ramp) {
        // See if the deck is already being scratched
        if (m_scratchEnable.toBool()) {
            initialVelocity = m_scratchRate.get();
        } else if (m_play.toBool()) {
            // If the deck is playing, set the filter's initial velocity to the
            // playback speed.
            initialVelocity = getDeckRate();
        }
    }

    // Setup the alpha-beta filter.
    m_filter.init(dt, initialVelocity, alpha, beta);

    // Enable scratching.
    m_scratchEnable.set(1.0);
}

void ScratchController::disable(bool ramp) {
    if (sDebug) {
        qDebug() << "ScratchController::disable" << ramp;
    }
    // By default, ramp to zero.
    m_dRampTo = 0.0;

    // If no ramping is desired, disable scratching immediately.
    if (!ramp) {
        // Clear scratch2_enable
        m_scratchEnable.set(0.0);
        // Can't return here because we need process() to stop the timer. So
        // it's still actually ramping, we just won't hear or see it.
    } else if (m_play.toBool()) {
        // If ramping is desired and we are playing then ramp up to the deck's
        // normal playback speed.
        m_dRampTo = getDeckRate();
    }

    m_lastMovementNs = Time::elapsed();
    // Activate the ramping in process().
    m_bRamping = true;
}

void ScratchController::brake(bool activate, double dt, double factor, double rate,
                              double alpha, double beta) {
    if (sDebug) {
        qDebug() << "ScratchController::brake" << activate << dt << factor
                 << rate << alpha << beta;
    }
    m_scratchEnable.set(activate ? 1.0 : 0.0);

    // used in process() for the different timer behavior we need
    m_bBrakeActive = activate;

    if (activate) {
        // store the new values for this spinback/brake effect
        m_dRampFactor = rate * factor / 100000.0; // approx 1 second for a factor of 1
        m_dRampTo = 0.0;

        // setup timer and set scratch2
        m_scratchRate.set(rate);

        // setup the filter using the provided parameters
        m_filter.init(dt, rate, alpha, beta);

        // activate the ramping in process()
        m_bRamping = true;
    }
}

void ScratchController::tick(int ticks) {
    m_lastMovementNs = Time::elapsed();
    m_ticks.fetchAndAddRelease(ticks);
}

void ScratchController::process() {
    const double oldRate = m_filter.predictedVelocity();

    // Feed the filter the latest ticks. If we're ramping to end scratching and
    // the wheel hasn't been turned very recently (spinback after lift-off) feed
    // fixed data.
    const qint64 kMillisecond = 1000000;
    if (m_bRamping && ((Time::elapsed() - m_lastMovementNs) > kMillisecond)) {
        // We want the filter to speed up to m_dRampTo so we pretend we got
        // m_dRampTo * m_dRampFactor ticks per process call. For regular
        // scratching this is just dt but for 'brake' mode we use a custom
        // ramp-factor. Either way, the filter should approach m_dRampTo.
        m_filter.observation(m_dRampTo * m_dRampFactor);

        // Once this code path is run, latch so it always runs until reset
        //m_lastMovementNs += 1000000000;

        // Reset ticks to 0.
        // NOTE(rryan) this was done in the original code. Not sure if we should
        // keep it.
        m_ticks = 0;
    } else {
        // This will (and should) be 0 if no net ticks have been accumulated
        // (i.e. the wheel is stopped)

        // Reset ticks to 0 atomically.
        double ticks = m_ticks.fetchAndStoreOrdered(0);
        m_filter.observation(m_tickScaleFactor * ticks);
    }

    // Get the new rate and actually do the scratching
    const double newRate = m_filter.predictedVelocity();
    m_scratchRate.set(newRate);

    if (sDebug) {
        qDebug() << "ScratchController::process" << oldRate << "->" << newRate;
    }

    // If we're ramping and the current rate is really close to the rampTo value
    // or we're in brake mode and have crossed over the zero value, end
    // scratching
    const double kEpsilon = 0.00001;
    if ((m_bRamping && fabs(m_dRampTo - newRate) <= kEpsilon) ||
        (m_bBrakeActive && (
            (oldRate > 0.0 && newRate < 0.0) ||
            (oldRate < 0.0 && newRate > 0.0)))) {
        // Not ramping no mo'
        m_bRamping = false;

        if (m_bBrakeActive) {
            // If in brake mode, set scratch rate to 0 and turn off the play
            // button.
            m_scratchRate.set(0.0);
            m_play.set(0.0);
        }

        // End scratching.
        m_scratchEnable.set(0.0);

        m_tickScaleFactor = 0.0;
        m_bBrakeActive = false;
        m_bEnabled = false;
    }
}

double ScratchController::getDeckRate() const {
    double rate = 1.0 + m_rate.get() * m_rateDir.get() * m_rateRange.get();
    return m_reverse.toBool() ? -rate : rate;
}
