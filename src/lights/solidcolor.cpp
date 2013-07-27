
#include <QMutexLocker>

#include "lights/solidcolor.h"

SolidColor::SolidColor(QString name, QColor color)
        : ColorGenerator(name),
          m_color(color) {

}

SolidColor::~SolidColor() {

}

QColor SolidColor::nextColor() {
    //QMutexLocker locker(&m_mutex);
    return m_color;
}

void SolidColor::setColor(QColor color) {
    //QMutexLocker locker(&m_mutex);
    m_color = color;
}
