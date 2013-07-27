#ifndef DMXLIGHTMANAGER_H
#define DMXLIGHTMANAGER_H

#include <QList>
#include <QDomNode>

#include "lights/dmx-eth.h"
#include "lights/dmxlight.h"
#include "lights/light.h"
#include "lights/lightmanager.h"

class LightController;

class DMXLightManager : public LightManager {
  public:
    DMXLightManager(QObject* pParent, QString ip_address, int type, DWord universe);
    virtual ~DMXLightManager();
    Light* newLight(QString id, int light_number);
    void sync();

    static DMXLightManager* fromXml(LightController* pController, QDomNode node);
  private:
    void setColor(Light* pLight, QColor color);

    QList<DMXLight*> m_lights;
    DMX_Handle* m_pHandle;
    DWord m_iUniverse;
    friend class DMXLight;
};

#endif /* DMXLIGHTMANAGER_H */
