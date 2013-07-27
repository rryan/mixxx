#ifndef CONTROLGROUP_H
#define CONTROLGROUP_H

#include <QtCore>
#include <QList>
#include <QMutex>
#include <QMutexLocker>
#include <QMap>
#include <QLinkedList>
#include <QDomNode>

#include "lights/colorgenerator.h"
#include "lights/controlmode.h"
#include "lights/triggermode.h"
#include "lights/transitionmode.h"
#include "lights/light.h"

class LightController;
class FeatureState;

class ControlGroup : public QObject {
    Q_OBJECT
  public:
    ControlGroup(QObject* pParent, QString name);
    virtual ~ControlGroup();

    QString getName() {
        QMutexLocker locker(&m_mutex);
        return m_name;
    }
    void setName(QString name) {
        QMutexLocker locker(&m_mutex);
        m_name = name;
    }

    void setControlMode(ControlMode controlMode);
    ControlMode getControlMode();

    void setTriggerMode(TriggerMode triggerMode);
    TriggerMode getTriggerMode();

    void setTransitionMode(TransitionMode transitionMode);
    TransitionMode getTransitionMode();

    void setColorGenerator(ColorGenerator* pGenerator);
    ColorGenerator* getColorGenerator();

    void addLight(Light* pLight);
    bool removeLight(Light* pLight);

    int numLights() {
        QMutexLocker locker(&m_mutex);
        return m_lights.size();
    }

    void process(FeatureState* pFrame);

    static ControlGroup* fromXml(LightController* pController, QDomNode node);

  signals:
    void attributeChanged();

  private:
    bool isTriggered(FeatureState* pState);
    void update_cycle(FeatureState* pState);
    void update_chaser(FeatureState* pState);
    void update_shifter(FeatureState* pState);
    void update_mirror(FeatureState* pState);

    void setLightColor(Light* pLight, const QColor& color);

    void initialize_mode();
    void init_chaser();
    void init_shifter();
    void init_mirror();

    QMutex m_mutex;
    QString m_name;
    QList<Light*> m_lights;


    // Trigger x2
    bool m_bBeatX2Triggered;

    ControlMode m_controlMode;
    TriggerMode m_triggerMode;
    TransitionMode m_transitionMode;

    ColorGenerator* m_pColorGenerator;

    int m_iChaserPosition, m_iChaserDirection;
    QLinkedList<QColor> m_shifterQueue;
    QLinkedList<QColor> m_mirrorQueue;;
};

#endif /* CONTROLGROUP_H */
