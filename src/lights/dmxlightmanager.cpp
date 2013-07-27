#include <QtDebug>

#include "lights/dmxlightmanager.h"
#include "lights/dmxlight.h"
#include "lights/lightcontroller.h"

DMXLightManager::DMXLightManager(QObject* pParent, QString ip_address, int type, DWord universe)
        : LightManager(pParent) {
    m_pHandle = new_dmx_handle(ip_address.toAscii().data(), type);
    m_iUniverse = universe;
}

DMXLightManager::~DMXLightManager() {

}

Light* DMXLightManager::newLight(QString id, int light_number) {
    DMXLight* pLight = new DMXLight(this, id, light_number);
    m_lights.append(pLight);
    return pLight;
}

void DMXLightManager::setColor(Light* pLight, QColor color) {
    DMXLight* pDMXLight = dynamic_cast<DMXLight*>(pLight);
    Q_ASSERT(pDMXLight);

    set_dmx_color(m_pHandle, pDMXLight->getLightNumber(), color.red(), color.green(), color.blue());
}

void DMXLightManager::sync() {
    foreach (DMXLight* pLight, m_lights) {
        setColor(pLight, pLight->getColor());
    }
    publish_dmx_update(m_pHandle, m_iUniverse);
}

// static
DMXLightManager* DMXLightManager::fromXml(LightController* pController, QDomNode node) {
    Q_ASSERT(node.nodeName() == "DMXLightManager");
    QDomElement element = node.toElement();
    QString ip_address = element.attribute("ipaddress", "");
    bool ok = false;

    QString type = element.attribute("type", "");
    int iType = type.toInt(&ok);
    if (!ok) {
        iType = 1;
    }

    QString universe = element.attribute("universe", "");
    DWord uni = universe.toInt(&ok);
    if (!ok) {
        uni = -1;
    }
    Q_ASSERT(ip_address != "");

    DMXLightManager* pManager = new DMXLightManager(pController, ip_address, iType, uni);
    qDebug() << "Constructing DMXLightManager with ip" << ip_address
             << "universe" << uni << "type" << iType;

    QDomNodeList children = node.childNodes();
    for (int i = 0; i < children.count(); ++i) {
        QDomElement child = children.at(i).toElement();
        Q_ASSERT(child.nodeName() == "DMXLight");
        QString id = child.attribute("id", "");
        QString name = child.attribute("name", "");
        QString number = child.attribute("number", "");
        Q_ASSERT(number != "" && id != "");
        int iNumber = number.toInt();

        Light* pLight = pManager->newLight(id, iNumber);
        if (name != "")
            pLight->setName(name);
        pController->addLight(pLight);
    }

    return pManager;
}
