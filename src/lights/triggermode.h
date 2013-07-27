#ifndef TRIGGERMODE_H
#define TRIGGERMODE_H

#include <QObject>
#include <QString>
#include <QList>

// WARNING: If you add a new trigger mode you must update triggermode.cpp to
// provide a string version of the mode and add it to the available mode list.

enum TriggerMode {
    CONTINUOUS = 0,
    BEAT,
    ONSET,
    BEAT_DIV2,
    BEAT_DIV4,
    BEAT_DIV8,
    BEAT_X2
};

QString triggerModeToString(TriggerMode mode);
QList<QString> getTriggerModeOptions();
TriggerMode triggerModeFromString(QString mode);

#endif /* TRIGGERMODE_H */
