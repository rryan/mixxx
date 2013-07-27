
#include "lights/transitionmode.h"

QString transitionModeToString(TransitionMode mode) {
    switch(mode) {
        case TRANSITION_SET:
            return QObject::tr("Set");
        case TRANSITION_FADE_50MS:
            return QObject::tr("Fade 50ms");
        case TRANSITION_FADE_250MS:
            return QObject::tr("Fade 250ms");
        case TRANSITION_FADE_500MS:
            return QObject::tr("Fade 500ms");
        case TRANSITION_FLASH_500MS:
            return QObject::tr("Flash 500ms");
        case TRANSITION_FADEUP_250MS:
            return QObject::tr("FadeUp 250ms");
        case TRANSITION_FADEUP_500MS:
            return QObject::tr("FadeUp 500ms");
        case TRANSITION_WHITEFLASH_500MS:
            return QObject::tr("WhiteFlash 500ms");
    }
    return QObject::tr("Unknown");
}

QList<QString> getTransitionModeOptions() {
    QList<QString> options;
    options.append(transitionModeToString(TRANSITION_SET));
    options.append(transitionModeToString(TRANSITION_FADE_50MS));
    options.append(transitionModeToString(TRANSITION_FADE_250MS));
    options.append(transitionModeToString(TRANSITION_FADE_500MS));
    options.append(transitionModeToString(TRANSITION_FLASH_500MS));
    options.append(transitionModeToString(TRANSITION_FADEUP_250MS));
    options.append(transitionModeToString(TRANSITION_FADEUP_500MS));
    options.append(transitionModeToString(TRANSITION_WHITEFLASH_500MS));
    return options;
}

