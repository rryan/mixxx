#include <QtDebug>

#include "lights/lightbrickmanager.h"
#include "lights/lightcontroller.h"

LightBrickManager::LightBrickManager(QObject* pParent)
        : LightManager(pParent) {

}

LightBrickManager::~LightBrickManager() {

}

Light* LightBrickManager::newLight(QString id, QString ip_address, QString port) {
    LightBrick* pLight = new LightBrick(this, id, ip_address, port);
    m_lights.append(pLight);
    return pLight;
}

void LightBrickManager::sync() {
    foreach(LightBrick* pLight, m_lights) {
        pLight->sync();
    }
}

// static
LightBrickManager* LightBrickManager::fromXml(LightController* pController, QDomNode node) {
    LightBrickManager* pManager = new LightBrickManager(pController);
    Q_ASSERT(node.nodeName() == "LightBrickManager");

    QDomNodeList children = node.childNodes();
    for (int i = 0; i < children.count(); ++i) {
        QDomElement child = children.at(i).toElement();
        Q_ASSERT(child.nodeName() == "LightBrick");
        QString ip = child.attribute("ipaddress", "");
        QString name = child.attribute("name", "");
        QString id = child.attribute("id", "");
        QString port = child.attribute("port", "");

        Q_ASSERT(ip != "" && id != "" && port != "");

        Light* pLight = pManager->newLight(id, ip, port);
        if (name != "")
            pLight->setName(name);
        pController->addLight(pLight);
    }

    return pManager;
}
