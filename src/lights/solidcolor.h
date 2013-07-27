
#ifndef SOLIDCOLOR_H
#define SOLIDCOLOR_H

#include <QObject>
#include <QMutex>

#include "lights/colorgenerator.h"

class SolidColor : public QObject, public ColorGenerator {
    Q_OBJECT
  public:
    SolidColor(QString name, QColor color);
    virtual ~SolidColor();
    QColor nextColor();
  public slots:
    void setColor(QColor color);
  private:
    QColor m_color;
    QMutex m_mutex;
};


#endif /* SOLIDCOLOR_H */
