#ifndef LINEARTWEENER_H
#define LINEARTWEENER_H

#include "lights/tweener.h"

class LinearTweener : public Tweener {
  public:
    LinearTweener() { }
    virtual ~LinearTweener() { }

    double getValue(double time) {
        return m_change*time/m_duration + m_initial;
    }
};


#endif /* LINEARTWEENER_H */
