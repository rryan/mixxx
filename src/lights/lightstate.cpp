
#include "lights/lightstate.h"

QString lightStateToString(LightState state) {
    switch (state) {
        case LIGHT_OFF:
            return QObject::tr("Off");
        case LIGHT_ON:
            return QObject::tr("Static");
        case LIGHT_FADE:
            return QObject::tr("Fading");
        case LIGHT_CYCLE:
            return QObject::tr("Cycling");
        case LIGHT_RANDOM:
            return QObject::tr("Random");
        case LIGHT_FFT:
            return QObject::tr("FFT");
    }
    return QObject::tr("Unknown");


}
