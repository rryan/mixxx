#ifndef CALLBACKTRACKMANAGER_H
#define CALLBACKTRACKMANAGER_H

#include <QHash>
#include <QList>

#include "trackinfoobject.h"
#include "util/fifo.h"

struct TrackUpdate {
    enum Type {
        BEATS_UPDATED,
        CUES_UPDATED
    };

    Type type;
};

struct CallbackTrackMessage {
    TrackInfoObject* track;
    TrackUpdate update;
};

class TrackWatcher : public QObject {
    Q_OBJECT
  public:
    TrackWatcher();
    TrackWatcher(const TrackPointer& pTrack);
    TrackWatcher(const TrackWatcher& trackWatcher);
    virtual ~TrackWatcher();

    TrackWatcher& operator=(const TrackWatcher& trackWatcher) {
        *this = TrackWatcher(trackWatcher);
        return *this;
    }

    const TrackPointer& getTrack() const {
        return m_pTrack;
    }

  signals:
    void trackUpdated(TrackPointer pTrack, TrackUpdate update);

  private slots:
    void slotCuesUpdated();
    void slotBeatsUpdated();

  private:
    TrackPointer m_pTrack;
};

class CallbackTrackWatcher : public QObject {
    Q_OBJECT
  public:
    CallbackTrackWatcher();
    virtual ~CallbackTrackWatcher();

    // Receive update from CallbackTrackManager
    void receiveUpdate(TrackUpdate update);

    void watchTrack(const TrackPointer& pTrack);
    void unwatchTrack(const TrackPointer& pTrack);

  signals:
    void cuesUpdated();
    void beatsUpdated();

    void startWatchingTrack(TrackPointer pTrack, CallbackTrackWatcher* pTrackWatcher);
    void stopWatchingTrack(TrackPointer pTrack, CallbackTrackWatcher* pTrackWatcher);
};

class CallbackTrackManager : public QObject {
    Q_OBJECT
  public:
    explicit CallbackTrackManager();
    virtual ~CallbackTrackManager();

    CallbackTrackWatcher* createTrackWatcher();

    void callbackProcessIncomingUpdates();

  public slots:
    // Must only invoke from the callback.
    void addTrackWatcher(const TrackPointer& pTrack, CallbackTrackWatcher* pTrackWatcher);
    void removeTrackWatcher(const TrackPointer& pTrack, CallbackTrackWatcher* pTrackWatcher);

    // Thread safe but NOT callback-safe. Indicates that the track 'pTrack' was
    // updated with the update 'update'.
    void slotTrackUpdated(const TrackPointer& pTrack, TrackUpdate update);

  private:
    ////////////////////////////////////////////////////////////////////////////
    // The following may only be called/touched while m_incomingUpdateMutex is
    // held.
    ////////////////////////////////////////////////////////////////////////////

    QMutex m_incomingUpdateMutex;
    FIFO<CallbackTrackMessage> m_incomingUpdateFifo;

    ////////////////////////////////////////////////////////////////////////////
    // The following may only be touched from within the callback
    ////////////////////////////////////////////////////////////////////////////

    struct TrackWatchers {
        TrackWatchers(const TrackPointer& pTrack)
                : watcher(pTrack) {
        }

        TrackWatchers(const TrackWatchers& other_watchers)
                : watcher(other_watchers.watcher),
                  callback_watchers(other_watchers.callback_watchers) {
        }

        TrackWatcher watcher;
        QList<CallbackTrackWatcher*> callback_watchers;
    };

    QHash<TrackInfoObject*, TrackWatchers> m_trackWatchers;
};

#endif /* CALLBACKTRACKMANAGER_H */
