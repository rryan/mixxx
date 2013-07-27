
#include "lights/triggermode.h"

QString triggerModeToString(TriggerMode mode) {
    switch(mode) {
        case CONTINUOUS:
            return QObject::tr("Continuous");
        case BEAT:
            return QObject::tr("Beat");
        case ONSET:
            return QObject::tr("Onset");
        case BEAT_DIV2:
            return QObject::tr("Beat/2");
        case BEAT_DIV4:
            return QObject::tr("Beat/4");
        case BEAT_DIV8:
            return QObject::tr("Beat/8");
        case BEAT_X2:
            return QObject::tr("Beat x2");
    }
    return QObject::tr("Unknown");
}

QList<QString> getTriggerModeOptions() {
    QList<QString> options;
    options.append(triggerModeToString(CONTINUOUS));
    options.append(triggerModeToString(BEAT));
    options.append(triggerModeToString(ONSET));
    options.append(triggerModeToString(BEAT_DIV2));
    options.append(triggerModeToString(BEAT_DIV4));
    options.append(triggerModeToString(BEAT_DIV8));
    options.append(triggerModeToString(BEAT_X2));
    return options;
}

TriggerMode triggerModeFromString(QString mode) {
    TriggerMode result = CONTINUOUS;

    if (mode == "CONTINUOUS") {
        result = CONTINUOUS;
    } else if (mode == "BEAT") {
        result = BEAT;
    } else if (mode == "ONSET") {
        result = ONSET;
    } else if (mode == "BEAT_DIV2") {
        result = BEAT_DIV2;
    } else if (mode == "BEAT_DIV4") {
        result = BEAT_DIV4;
    } else if (mode == "BEAT_DIV8") {
        result = BEAT_DIV8;
    } else if (mode == "BEAT_X2") {
        result = BEAT_X2;
    }
    return result;
}
