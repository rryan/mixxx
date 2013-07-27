
#ifndef LIGHTBRICKMANAGER_H
#define LIGHTBRICKMANAGER_H

#include <QDomNode>
#include <QList>

#include "lights/lightbrick.h"
#include "lights/lightmanager.h"

class LightController;

class LightBrickManager : public LightManager {
    Q_OBJECT
  public:
    LightBrickManager(QObject* pParent);
    virtual ~LightBrickManager();

    Light* newLight(QString id, QString ip_address, QString port);
    void sync();

    static LightBrickManager* fromXml(LightController* pController, QDomNode node);
  private:
    QList<LightBrick*> m_lights;
};


#endif /* LIGHTBRICKMANAGER_H */
