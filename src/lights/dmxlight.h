
#ifndef DMXLIGHT_H
#define DMXLIGHT_H

#include <QObject>
#include "lights/light.h"

class DMXLightManager;

class DMXLight : public Light {
    Q_OBJECT
  public:
    DMXLight(DMXLightManager* pManager, QString id, int light_number);
    virtual ~DMXLight();

    int getLightNumber() {
        return m_iLightNumber;
    }

  private:
    DMXLightManager* m_pManager;
    const int m_iLightNumber;
};


#endif /* DMXLIGHT_H */
