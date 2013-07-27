
#ifndef LIGHTSTATE_H
#define LIGHTSTATE_H

#include <QString>
#include <QObject>

// WARNING: If you add a new light state you must update lightstate.cpp to
// provide a string version of the mode and add it to the available mode list.

enum LightState {
    LIGHT_OFF = 0,
    LIGHT_ON,
    LIGHT_FADE,
    // Unused
    LIGHT_CYCLE,
    LIGHT_RANDOM,
    LIGHT_FFT
};

QString lightStateToString(LightState mode);

#endif /* LIGHTSTATE_H */
