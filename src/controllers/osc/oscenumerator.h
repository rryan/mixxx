#ifndef OSCENUMERATOR_H
#define OSCENUMERATOR_H

#include "controllers/controllerenumerator.h"

// An enumerator for OSC devices.
class OscEnumerator : public ControllerEnumerator {
    Q_OBJECT
  public:
    OscEnumerator();
    virtual ~OscEnumerator();

    QList<Controller*> queryDevices();

    void addController();
    void removeController();

  private:
    QList<Controller*> m_devices;
};

#endif /* OSCENUMERATOR_H */
