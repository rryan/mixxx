#ifndef LIGHTBRICK_H
#define LIGHTBRICK_H

#include <QString>
#include <lo/lo.h>

#include "lights/light.h"

class LightBrick : public Light {
    Q_OBJECT
  public:
    LightBrick(QObject* pParent, QString id, QString ip_address, QString port);
    virtual ~LightBrick();

    void sync();
  private:
    QString m_ip_address, m_port;
    lo_address m_osc_destination;
};


#endif /* LIGHTBRICK_H */
