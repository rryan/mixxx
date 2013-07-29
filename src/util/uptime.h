#ifndef UPTIME_H
#define UPTIME_H

#include "singleton.h"
#include "util/performancetimer.h"

class Uptime {
  public:
    static inline void initialize() {
        s_timer.start();
    }

    // Returns nanoseconds since initialize was called.
    static qint64 uptimeNanos() {
        return s_timer.elapsed();
    }

  private:
    static PerformanceTimer s_timer;
};

#endif /* UPTIME_H */
