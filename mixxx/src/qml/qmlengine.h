#ifndef QMLENGINE_H
#define QMLENGINE_H

#include <QDeclarativeView>
#include <QDeclarativeItem>

#include "playermanager.h"
#include "basetrackplayer.h"
#include "library/library.h"

class ControlObjectThread;

class QmlEngine : public QDeclarativeItem
{
    Q_OBJECT

public:
    // Engine Values read write
    Q_INVOKABLE double getValue(QString group, QString name);
    Q_INVOKABLE void setValue(QString group, QString name, double newValue);
    // Track Properties
    Q_INVOKABLE QString getTrackProperty(QString group, QString property);
    // Engine Events
    Q_INVOKABLE void enableEvent(QString group, QString name);
    Q_INVOKABLE void disableEvent(QString group, QString name);
    // Player Events
    Q_INVOKABLE void enablePlayerEvents(QString group);
    Q_INVOKABLE void disablePlayerEvents(QString group);

    explicit QmlEngine(QDeclarativeItem *parent = 0);
    // Setup the MixxxEngine Item
    void setup(PlayerManager* pPlayerManager, Library* pLibrary, QDeclarativeView* pMyView);
    void initialized();
signals:
    void mixxxEvent(QString eventKey, double value);
    void mixxxInitialized();

public slots:
    // Engine Slots
    void slotValueChanged(double value);
    // Track Slots
    void slotLoadTrack(TrackPointer);
    void slotNewTrackLoaded(TrackPointer);
    void slotUnloadingTrack(TrackPointer);

private:
    ControlObjectThread* getControlObjectThread(QString group, QString name);
    QHash<ConfigKey, ControlObjectThread*> m_controlCache;
    PlayerManager* m_pPlayerManager;
    Library* m_pLibrary;
};

#endif // QMLMIXXXENGINE_H

