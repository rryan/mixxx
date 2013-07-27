#include "lights/colorgenerator.h"

class RGBCycler : public ColorGenerator {

  public:
    RGBCycler(QString name, int r, int g, int b, int dr, int dg, int db)
            : ColorGenerator(name),
              m_r(r),
              m_g(g),
              m_b(b),
              m_dr(dr),
              m_dg(dg),
              m_db(db) {

    }

    QColor nextColor() {
        m_r = (m_r+m_dr) % 255;
        m_g = (m_g+m_dg) % 255;
        m_b = (m_b+m_db) % 255;
        return QColor(m_r,m_g,m_b);
    }

  private:
    int m_r, m_g, m_b, m_dr, m_dg, m_db;
};
