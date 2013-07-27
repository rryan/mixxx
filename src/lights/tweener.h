#ifndef TWEENER_H
#define TWEENER_H

class Tweener {
  public:
    void init(double initial, double change, double duration) {
        m_initial = initial;
        m_change = change;
        m_duration = duration;
    }

    virtual double getValue(double time) = 0;

    double getDuration() { return m_duration; }
    double getInitial() { return m_initial; }
    double getChange() { return m_change; }

  protected:
    double m_initial, m_change, m_duration;
};

#endif /* TWEENER_H */
