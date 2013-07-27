
#include "lights/controlmode.h"

QString controlModeToString(ControlMode mode) {
    switch(mode) {
        case CONTROL_OFF:
            return QObject::tr("Off");
        case CONTROL_CHASER:
            return QObject::tr("Chaser");
        case CONTROL_SHIFTER_RIGHT:
            return QObject::tr("ShiftRight");
        case CONTROL_SHIFTER_LEFT:
            return QObject::tr("ShiftLeft");
        case CONTROL_MIRROR_INWARD:
            return QObject::tr("MirrorIn");
        case CONTROL_MIRROR_OUTWARD:
            return QObject::tr("MirrorOut");
        case CONTROL_CYCLE:
            return QObject::tr("Cycle");
        case CONTROL_TWINKLE:
            return QObject::tr("Twinkle");
        case CONTROL_GLOW:
            return QObject::tr("Glow");
        case CONTROL_FLASH:
            return QObject::tr("Flash");
    }
    return QObject::tr("Unknown");
}

QList<QString> getControlModeOptions() {
    QList<QString> options;
    options.append(controlModeToString(CONTROL_OFF));
    options.append(controlModeToString(CONTROL_CHASER));
    options.append(controlModeToString(CONTROL_SHIFTER_RIGHT));
    options.append(controlModeToString(CONTROL_SHIFTER_LEFT));
    options.append(controlModeToString(CONTROL_MIRROR_INWARD));
    options.append(controlModeToString(CONTROL_MIRROR_OUTWARD));
    options.append(controlModeToString(CONTROL_CYCLE));
    options.append(controlModeToString(CONTROL_TWINKLE));
    options.append(controlModeToString(CONTROL_GLOW));
    options.append(controlModeToString(CONTROL_FLASH));
    return options;
}

ControlMode controlModeFromString(QString mode) {
    ControlMode result = CONTROL_OFF;

    if (mode == "CONTROL_OFF") {
        result = CONTROL_OFF;
    } else if (mode == "CONTROL_CHASER") {
        result = CONTROL_CHASER;
    } else if (mode == "CONTROL_SHIFTER_RIGHT") {
        result = CONTROL_SHIFTER_RIGHT;
    } else if (mode == "CONTROL_SHIFTER_LEFT") {
        result = CONTROL_SHIFTER_LEFT;
    } else if (mode == "CONTROL_MIRROR_INWARD") {
        result = CONTROL_MIRROR_INWARD;
    } else if (mode == "CONTROL_MIRROR_OUTWARD") {
        result = CONTROL_MIRROR_OUTWARD;
    } else if (mode == "CONTROL_CYCLE") {
        result = CONTROL_CYCLE;
    } else if (mode == "CONTROL_TWINKLE") {
        result = CONTROL_TWINKLE;
    } else if (mode == "CONTROL_GLOW") {
        result = CONTROL_GLOW;
    } else if (mode == "CONTROL_FLASH") {
        result = CONTROL_FLASH;
    } else if (mode == "CONTROL_FLASH_WHITE") {
        result = CONTROL_FLASH_WHITE;
    }

    return result;
}
