#include <QtDebug>
#include <QMetaType>

#include "engine/callbacktrackmanager.h"

TrackWatcher::TrackWatcher() {
}

TrackWatcher::TrackWatcher(const TrackPointer& pTrack)
        : QObject(),
          m_pTrack(pTrack) {
    connect(m_pTrack.data(), SIGNAL(cuesUpdated()),
            this, SLOT(slotCuesUpdated()),
            Qt::DirectConnection);
    connect(m_pTrack.data(), SIGNAL(beatsUpdated()),
            this, SLOT(slotBeatsUpdated()),
            Qt::DirectConnection);
}

TrackWatcher::TrackWatcher(const TrackWatcher& watcher)
        : QObject() {
    m_pTrack = watcher.m_pTrack;
    connect(m_pTrack.data(), SIGNAL(cuesUpdated()),
            this, SLOT(slotCuesUpdated()),
            Qt::DirectConnection);
    connect(m_pTrack.data(), SIGNAL(beatsUpdated()),
            this, SLOT(slotBeatsUpdated()),
            Qt::DirectConnection);
}

TrackWatcher::~TrackWatcher() {
}

void TrackWatcher::slotCuesUpdated() {
    //qDebug() << "TrackWatcher::slotCuesUpdated()";
    TrackUpdate update;
    update.type = TrackUpdate::CUES_UPDATED;
    emit(trackUpdated(m_pTrack, update));
}

void TrackWatcher::slotBeatsUpdated() {
    //qDebug() << "TrackWatcher::slotBeatsUpdated()";
    TrackUpdate update;
    update.type = TrackUpdate::BEATS_UPDATED;
    emit(trackUpdated(m_pTrack, update));
}

CallbackTrackWatcher::CallbackTrackWatcher() {
}

CallbackTrackWatcher::~CallbackTrackWatcher() {
}

void CallbackTrackWatcher::receiveUpdate(TrackUpdate update) {
    switch (update.type) {
        case TrackUpdate::CUES_UPDATED:
            emit(cuesUpdated());
            break;
        case TrackUpdate::BEATS_UPDATED:
            emit(beatsUpdated());
            break;
        default:
            //qDebug() << "Unhandled TrackUpdate type in CallbackTrackWatcher::receiveUpdate";
            break;
    }
}

void CallbackTrackWatcher::watchTrack(const TrackPointer& pTrack) {
    emit(startWatchingTrack(pTrack, this));
}

void CallbackTrackWatcher::unwatchTrack(const TrackPointer& pTrack) {
    emit(stopWatchingTrack(pTrack, this));
}

CallbackTrackManager::CallbackTrackManager()
        : m_incomingUpdateFifo(4096) {
    qRegisterMetaType<TrackUpdate>("TrackUpdate");
}

CallbackTrackManager::~CallbackTrackManager() {
}

CallbackTrackWatcher* CallbackTrackManager::createTrackWatcher() {
    CallbackTrackWatcher* pWatcher = new CallbackTrackWatcher();
    connect(pWatcher, SIGNAL(startWatchingTrack(TrackPointer, CallbackTrackWatcher*)),
            this, SLOT(addTrackWatcher(TrackPointer, CallbackTrackWatcher*)),
            Qt::DirectConnection);
    connect(pWatcher, SIGNAL(stopWatchingTrack(TrackPointer, CallbackTrackWatcher*)),
            this, SLOT(removeTrackWatcher(TrackPointer, CallbackTrackWatcher*)),
            Qt::DirectConnection);
    return pWatcher;
}

void CallbackTrackManager::addTrackWatcher(const TrackPointer& pTrack,
                                           CallbackTrackWatcher* pTrackWatcher) {
    //qDebug() << "addTrackWatcher()" << pTrack << pTrackWatcher;
    QHash<TrackInfoObject*, TrackWatchers>::iterator it = m_trackWatchers.find(pTrack.data());
    if (it == m_trackWatchers.end()) {
        TrackWatchers watchers(pTrack);
        it = m_trackWatchers.insert(pTrack.data(), watchers);
        connect(&it.value().watcher, SIGNAL(trackUpdated(TrackPointer, TrackUpdate)),
                this, SLOT(slotTrackUpdated(TrackPointer, TrackUpdate)),
                Qt::DirectConnection);
    }
    it.value().callback_watchers.append(pTrackWatcher);
}

void CallbackTrackManager::removeTrackWatcher(const TrackPointer& pTrack,
                                              CallbackTrackWatcher* pTrackWatcher) {
    //qDebug() << "removeTrackWatcher()" << pTrack << pTrackWatcher;
    QHash<TrackInfoObject*, TrackWatchers>::iterator it =
            m_trackWatchers.find(pTrack.data());
    if (it != m_trackWatchers.end()) {
        it.value().callback_watchers.removeAll(pTrackWatcher);
    }
}

void CallbackTrackManager::slotTrackUpdated(const TrackPointer& pTrack,
                                            TrackUpdate update) {
    //qDebug() << "CallbackTrackManager::slotTrackUpdated()" << pTrack;
    if (!pTrack) {
        return;
    }

    QMutexLocker locker(&m_incomingUpdateMutex);
    CallbackTrackMessage message;
    message.track = pTrack.data();
    message.update = update;
    m_incomingUpdateFifo.writeBlocking(&message, 1);
}

void CallbackTrackManager::callbackProcessIncomingUpdates() {
    CallbackTrackMessage message;
    while (m_incomingUpdateFifo.read(&message, 1) == 1) {
        if (!message.track) {
            continue;
        }
        QHash<TrackInfoObject*, TrackWatchers>::iterator it =
                m_trackWatchers.find(message.track);
        if (it != m_trackWatchers.end()) {
            TrackWatchers& watchers = it.value();
            for (QList<CallbackTrackWatcher*>::iterator it2 = watchers.callback_watchers.begin();
                 it2 != watchers.callback_watchers.end(); ++it2) {
                (*it2)->receiveUpdate(message.update);
            }
        }
    }
}

