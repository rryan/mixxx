#include <QtDebug>
#include <QMutexLocker>

#include "lights/light.h"

#include "mathstuff.h"

#include "lights/controlgroup.h"
#include "lights/lineartweener.h"
#include "lights/cubiceaseouttweener.h"
#include "lights/cubiceaseintweener.h"

static int lightCount = 0;

Light::Light(QObject* pParent, QString id)
        : QObject(pParent),
          m_name(QString("Light %1").arg(lightCount++)),
          m_id(id),
          m_state(LIGHT_OFF),
          m_color(Qt::black),
          m_pControlGroup(NULL),
          m_hueTweener(new CubicEaseOutTweener()),
          m_satTweener(new CubicEaseOutTweener()),
          m_valTweener(new CubicEaseOutTweener())
          // m_hueTweener(new LinearTweener()),
          // m_satTweener(new LinearTweener()),
          // m_valTweener(new LinearTweener())
{

}

Light::~Light() {

}

void Light::setColor(const QColor& color) {
    bool changed = m_color != color;
    m_color = color;
    setState(LIGHT_ON);
    if (changed)
        emit(updated());
}

QColor Light::getColor() {
    return m_color;
}

ControlGroup* Light::getControlGroup() {
    QMutexLocker locker(&m_mutex);
    return m_pControlGroup;
}

void Light::setControlGroup(ControlGroup* pGroup) {
    QMutexLocker locker(&m_mutex);
    m_pControlGroup = pGroup;
}

void Light::fadeDown(int millis) {
    fadeToValue(0.0f, millis);
}

void Light::fadeTo(const QColor& targetColor, int millis) {
    double h, s, v;
    double th, ts, tv;
    double dh, ds, dv;

    m_color.getHsvF(&h, &s, &v);
    targetColor.getHsvF(&th, &ts, &tv);

    dh = (th - h);

    // Measure the distance to take the 'other way' and take whichever path is
    // smaller in absolute value.
    double dho = dh + (th < h ? 1.0f : -1.0f);
    if (fabs(dho) < fabs(dh)) {
        dh = dho;
    }

    ds = (ts - s);
    dv = (tv - v);

    // The parameters all range from 0.0 to 1.0f for now. This could be a
    // time parameter int the future.
    m_hue_parameter = m_sat_parameter = m_val_parameter = 0.0f;
    m_hueTweener->init(h, dh, millis);
    m_satTweener->init(s, ds, millis);
    m_valTweener->init(v, dv, millis);

    setState(LIGHT_FADE);
}

void Light::fadeToHue(qreal target_hue, int millis) {
    double h, s, v;
    m_color.getHsvF(&h, &s, &v);
    fadeTo(QColor::fromHsvF(target_hue, s, v), millis);
}

void Light::fadeToValue(qreal target_value, int millis) {
    double h, s, v;
    m_color.getHsvF(&h, &s, &v);
    fadeTo(QColor::fromHsvF(h, s, target_value), millis);
}

void Light::animate(qreal deltaMillis) {
    double h, s, v;
    double h_duration, s_duration, v_duration;
    m_color.getHsvF(&h, &s, &v);

    switch (m_state) {
        case LIGHT_OFF:
        case LIGHT_ON:
            // Do nothing
            break;
        case LIGHT_FADE:
            h_duration = m_hueTweener->getDuration();
            s_duration = m_satTweener->getDuration();
            v_duration = m_valTweener->getDuration();

            // Make sure they stay below 1.0f
            m_hue_parameter = math_min(h_duration, m_hue_parameter + deltaMillis);
            m_sat_parameter = math_min(s_duration, m_sat_parameter + deltaMillis);
            m_val_parameter = math_min(v_duration, m_val_parameter + deltaMillis);

            h = m_hueTweener->getValue(m_hue_parameter);
            s = m_satTweener->getValue(m_sat_parameter);
            v = m_valTweener->getValue(m_val_parameter);

            // For sanity, clamp 0..1, except for hue, which take mod 1.0f.

            //h = math_min(math_max(0.0f, h), 1.0f);
            while (h < 0) {
                h += 1.0f;
            }
            while (h > 1.0f) {
                h -= 1.0f;
            }

            s = math_min(math_max(0.0f, s), 1.0f);
            v = math_min(math_max(0.0f, v), 1.0f);

            // If parameters have all reached their end, state transition.
            if (m_hue_parameter >= h_duration &&
                m_sat_parameter >= s_duration &&
                m_val_parameter >= v_duration) {
                setState(LIGHT_ON);
            }

            m_color.setHsvF(h, s, v);
            break;
        case LIGHT_CYCLE:
        case LIGHT_FFT:
        case LIGHT_RANDOM:
        default:
            break;
    }
}

LightState Light::getState() {
    return m_state;
}

void Light::setState(LightState state) {
    bool changed = m_state != state;
    m_state = state;
    if (changed)
        emit(updated());
}

void Light::setName(QString name) {
    m_name = name;
}

QString Light::getName() {
    return m_name;
}

QString Light::getId() {
    return m_id;
}
