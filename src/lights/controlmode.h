#ifndef CONTROLMODE_H
#define CONTROLMODE_H

#include <QString>
#include <QList>
#include <QObject>

// WARNING: If you add a new control mode you must update controlmode.cpp to
// provide a string version of the mode and add it to the available mode list.

enum ControlMode {
    CONTROL_OFF = 0,
    CONTROL_CHASER,
    CONTROL_SHIFTER_RIGHT,
    CONTROL_SHIFTER_LEFT,
    CONTROL_MIRROR_INWARD,
    CONTROL_MIRROR_OUTWARD,
    CONTROL_CYCLE,
    CONTROL_TWINKLE,
    CONTROL_GLOW,
    CONTROL_FLASH,
    CONTROL_FLASH_WHITE
};

QString controlModeToString(ControlMode mode);
QList<QString> getControlModeOptions();
ControlMode controlModeFromString(QString mode);

#endif /* CONTROLMODE_H */
