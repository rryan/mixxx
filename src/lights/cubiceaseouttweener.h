#ifndef CUBICEASEOUTTWEENER_H
#define CUBICEASEOUTTWEENER_H

#include "lights/tweener.h"

class CubicEaseOutTweener : public Tweener {
  public:
    CubicEaseOutTweener() { }
    virtual ~CubicEaseOutTweener() { }

    double getValue(double time) {
        // return c*((t=t/d-1)*t*t + 1) + b;
        time = time / m_duration - 1;
        return m_change * (time * time * time + 1) + m_initial;
    }
};

#endif /* CUBICEASEOUTTWEENER_H */
