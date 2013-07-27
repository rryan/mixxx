#ifndef LIGHT_H
#define LIGHT_H

#include <QObject>
#include <QColor>
#include <QMutex>

#include "lights/lightstate.h"
#include "lights/tweener.h"

class ControlGroup;

class Light : public QObject {
    Q_OBJECT
  public:
    Light(QObject* pParent, QString id);
    virtual ~Light();

    virtual QString getName();
    virtual void setName(QString name);

    QString getId();

    void setColor(const QColor& color);
    QColor getColor();
    LightState getState();
    ControlGroup* getControlGroup();
    void setControlGroup(ControlGroup* pGroup);

    virtual bool isSegmented() {
        return false;
    }

    void animate(qreal deltaMillis);
    void fadeDown(int millis);
    void fadeTo(const QColor& targetColor, int millis);
    void fadeToHue(qreal hue, int millis);
    void fadeToValue(qreal value, int millis);

  signals:
    void updated();

  private:
    void setState(LightState state);

    QMutex m_mutex;
    QString m_name, m_id;
    LightState m_state;
    QColor m_color;
    ControlGroup* m_pControlGroup;

    // Fade parameters
    qreal m_hue_parameter, m_sat_parameter, m_val_parameter;

    // Tweeners for fading and transitions
    Tweener* m_hueTweener;
    Tweener* m_satTweener;
    Tweener* m_valTweener;
};


#endif /* LIGHT_H */
