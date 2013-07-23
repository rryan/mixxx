#ifndef VECTORCONTROL_H
#define VECTORCONTROL_H

#include <QVector3D>

#include "configobject.h"
#include "controlobject.h"

class Vector3DControl {
  public:
    Vector3DControl(const ConfigKey& baseKey) {
        m_pVectorX = new ControlObject(
            ConfigKey(baseKey.group , baseKey.item + "_x"));
        m_pVectorY = new ControlObject(
            ConfigKey(baseKey.group , baseKey.item + "_y"));
        m_pVectorZ = new ControlObject(
            ConfigKey(baseKey.group , baseKey.item + "_z"));
    }

    virtual ~Vector3DControl() {
        delete m_pVectorX;
        delete m_pVectorY;
        delete m_pVectorZ;
    }

    QVector3D getVector() const {
        return QVector3D(m_pVectorX->get(),
                         m_pVectorY->get(),
                         m_pVectorZ->get());
    }

  private:
    ControlObject* m_pVectorX;
    ControlObject* m_pVectorY;
    ControlObject* m_pVectorZ;
};


#endif /* VECTORCONTROL_H */
