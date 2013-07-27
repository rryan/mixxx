#ifndef TRANSITIONMODE_H
#define TRANSITIONMODE_H

#include <QObject>
#include <QString>
#include <QList>

// WARNING: If you add a new mode you must update transitionmode.cpp to provide
// a string version of the mode and add it to the available mode list.

enum TransitionMode {
    TRANSITION_SET,
    TRANSITION_FADE_50MS,
    TRANSITION_FADE_250MS,
    TRANSITION_FADE_500MS,
    TRANSITION_FLASH_500MS,
    TRANSITION_FADEUP_250MS,
    TRANSITION_FADEUP_500MS,
    TRANSITION_WHITEFLASH_500MS
};

QString transitionModeToString(TransitionMode mode);
QList<QString> getTransitionModeOptions();

#endif /* TRANSITIONMODE_H */
