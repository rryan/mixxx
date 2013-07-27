#ifndef LIGHTSHOWCONTROLLER_H
#define LIGHTSHOWCONTROLLER_H

#include <QTime>
#include <QList>
#include <QMap>

class LightController;
class ControlGroup;
class FeatureState;

class LightShowController : public QObject {
    Q_OBJECT
  public:
    LightShowController(LightController* pController);
    virtual ~LightShowController();

    void addControlGroup(ControlGroup* pGroup);
    void process(FeatureState* pState);

  private:
    LightController* m_pLightController;
    QList<ControlGroup*> m_controlGroups;
    QTime m_timeSinceLastUpdate;
    QMap<ControlGroup*, QTime> m_lastTransitions;
};

#endif /* LIGHTSHOWCONTROLLER_H */
