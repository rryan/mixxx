#ifndef CUBICEASEINTWEENER_H
#define CUBICEASEINTWEENER_H

#include "lights/tweener.h"

class CubicEaseInTweener : public Tweener {
  public:
    CubicEaseInTweener() { }
    virtual ~CubicEaseInTweener() { }

    double getValue(double time) {
        // return c*(t/=d)*t*t + b;
        time = time / m_duration;
        return m_change * time * time * time + m_initial;
    }
};


#endif /* CUBICEASEINTWEENER_H */
