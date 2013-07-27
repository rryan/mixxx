#ifndef COLORGENERATOR_H
#define COLORGENERATOR_H

#include <QString>
#include <QColor>

class ColorGenerator {
  public:
    ColorGenerator() {
        m_name = "";
    }

    ColorGenerator(QString name)
            : m_name(name) {
    }

    virtual QColor nextColor() = 0;

    void setName(QString name) {
        m_name = name;
    }
    QString getName() {
        return m_name;
    }
  private:
    QString m_name;
};

#endif /* COLORGENERATOR_H */
