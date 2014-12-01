#ifndef SCRATCHCONTROLLER_H
#define SCRATCHCONTROLLER_H

#include "controlobjectslave.h"
#include "util/alphabetafilter.h"

class ScratchController {
  public:
    ScratchController(const QString& group);

    bool enabled() const {
        return m_bEnabled;
    }

    bool ramping() const {
        return m_bRamping;
    }

    bool isScratching() const;

    void enable(double dt, double tickScaleFactor,
                double alpha, double beta, bool ramp);

    // Does not actually disable the controller! We switch to disabled on the
    // next process call.
    void disable(bool ramp);

    void brake(bool activate, double dt, double factor, double rate,
               double alpha, double beta);

    void tick(int ticks);

    void process();

  private:
    double getDeckRate() const;

    AlphaBetaFilter m_filter;
    QAtomicInt m_ticks;
    double m_tickScaleFactor;
    bool m_bEnabled;
    bool m_bRamping;
    double m_dRampTo;
    double m_dRampFactor;
    qint64 m_lastMovementNs;
    bool m_bBrakeActive;

    ControlObjectSlave m_play;
    ControlObjectSlave m_rate;
    ControlObjectSlave m_rateDir;
    ControlObjectSlave m_rateRange;
    ControlObjectSlave m_reverse;
    ControlObjectSlave m_scratchEnable;
    ControlObjectSlave m_scratchRate;
};

#endif /* SCRATCHCONTROLLER_H */
