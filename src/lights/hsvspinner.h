#ifndef HSVSPINNER_H
#define HSVSPINNER_H

#include "lights/colorgenerator.h"

class HSVSpinner : public ColorGenerator {
  public:
    HSVSpinner(QString name, qreal h, qreal s, qreal v, qreal dh)
            : ColorGenerator(name), m_h(h), m_s(s), m_v(v), m_dh(dh) {
    }

    QColor nextColor() {
        m_h = (m_h + m_dh);
        while (m_h < 0) {
            m_h += 1.0f;
        }
        while (m_h > 1.0f) {
            m_h -= 1.0f;
        }
        return QColor::fromHsvF(m_h, m_s, m_v);
    }

  private:
    qreal m_h, m_s, m_v, m_dh;
};

#endif /* HSVSPINNER_H */
