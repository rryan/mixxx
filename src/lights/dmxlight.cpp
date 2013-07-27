
#include "lights/dmxlight.h"
#include "lights/dmxlightmanager.h"


DMXLight::DMXLight(DMXLightManager* pManager, QString id,  int lightNumber)
        : Light(pManager, id),
          m_pManager(pManager),
          m_iLightNumber(lightNumber) {
    setName(QString("DMX%1").arg(m_iLightNumber));
}

DMXLight::~DMXLight() {

}
