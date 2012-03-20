#include "qmlengine.h"

#include "configobject.h"
#include "controlobject.h"
#include "controlobjectthread.h"

QmlEngine::QmlEngine(QDeclarativeItem *parent) :
    QDeclarativeItem(parent)
{
    // register all QML Widget Proxies here
    /*
    qmlRegisterType<QmlExample>("Mixxx", 1,0, "MixxxExample");
    */
}

void QmlEngine::setup(PlayerManager* pPlayerManager, Library* pLibrary, QDeclarativeView* pMyView) {
    Q_UNUSED(pMyView);    
    m_pPlayerManager = pPlayerManager;
    m_pLibrary = pLibrary;

    // setup all QML Widget Proyxies here
    /*
    QList<QmlOverview *> pOverviewWidgets = pMyView->rootObject()->findChildren<QmlExample*>("MixxxExample");	
    for (int i = 0; i < pOverviewWidgets.size(); ++i) {
        if (pOverviewWidgets.at(i) != NULL)
            pOverviewWidgets.at(i)->setup();
    }
    */
}

double QmlEngine::getValue(QString group, QString name) {
    ControlObjectThread *cot = getControlObjectThread(group, name);
    if (cot == NULL) {
        qWarning() << "QmlEngine::getValue: Unknown control" << group << name;
        return 0.0;
    }
    return cot->getControlObject()->getValueToWidget(cot->get());
}

QString QmlEngine::getTrackProperty(QString group, QString property) {
    BaseTrackPlayer* player = m_pPlayerManager->getPlayer(group);
    if (player == NULL) {
        qWarning() << "QmlEngine::getTrackProperty: Unknown player " << group;
        return "-";
    }

    TrackPointer track = player->getLoadedTrack();
    if (track == NULL) {
        qWarning() << "QmlEngine::getTrackProperty: no loaded Track " << group;
        return "-";
    }

    QVariant pProperty = track->property(property.toAscii().constData());
    if (pProperty.isValid() && qVariantCanConvert<QString>(pProperty)) {
        return pProperty.toString();
    }

    qWarning() << "QmlEngine::getTrackProperty: cant read property " << group;
    return "-";
}

void QmlEngine::setValue(QString group, QString name, double newValue) {
    if(isnan(newValue)) {
        qWarning() << "QmlEngine::setValue: setting [" << group << "," << name << "] to NotANumber, ignoring.";
        return;
    }
    ControlObjectThread *cot = getControlObjectThread(group, name);

    if(cot != NULL) {
        cot->getControlObject()->set(cot->getControlObject()->getValueFromWidget(newValue));
    }
}

void QmlEngine::enableEvent(QString group, QString name) {
    ControlObjectThread *cot = getControlObjectThread(group, name);
    if (cot == NULL) {
        qWarning() << "QMLMixxxEngine enableEvent: Unknown control" << group << name;
        return;
    }

    ControlObject *cobj = cot->getControlObject();
    connect(cobj, SIGNAL(valueChanged(double)), this, SLOT(slotValueChanged(double)));
    connect(cobj, SIGNAL(valueChangedFromEngine(double)), this, SLOT(slotValueChanged(double)));

    QString keyString = group + "," + name;
    emit(mixxxEvent(keyString, cobj->getValueToWidget(cot->get())));
}

void QmlEngine::disableEvent(QString group, QString name) {
    ControlObjectThread *cot = getControlObjectThread(group, name);
    if (cot == NULL) {
        qWarning() << "QML Mixxx Engine enableEvent: Unknown control" << group << name;
        return;
    }

    ControlObject *cobj = cot->getControlObject();
    this->disconnect(cobj, SIGNAL(valueChanged(double)), this, SLOT(slotValueChanged(double)));
    this->disconnect(cobj, SIGNAL(valueChangedFromEngine(double)), this, SLOT(slotValueChanged(double)));
}

void QmlEngine::enablePlayerEvents(QString group) {
    BaseTrackPlayer* track = m_pPlayerManager->getPlayer(group);
    if (track == NULL) {
        qWarning() << "QmlEngine::enablePlayer: Unknown player " << group;
        return;
    }

    connect(track, SIGNAL(loadTrack(TrackPointer)), this, SLOT(slotLoadTrack(TrackPointer)));
    connect(track, SIGNAL(newTrackLoaded(TrackPointer)), this, SLOT(slotNewTrackLoaded(TrackPointer)));
    connect(track, SIGNAL(unloadingTrack(TrackPointer)), this, SLOT(slotUnloadingTrack(TrackPointer)));
}

void QmlEngine::disablePlayerEvents(QString group) {
    BaseTrackPlayer* track = m_pPlayerManager->getPlayer(group);
    if (track == NULL) {
        qWarning() << "QmlEngine::enablePlayer: Unknown player " << group;
        return;
    }

    this->disconnect(track, SIGNAL(loadTrack(TrackPointer)), this, SLOT(slotLoadTrack(TrackPointer)));
    this->disconnect(track, SIGNAL(newTrackLoaded(TrackPointer)), this, SLOT(slotNewTrackLoaded(TrackPointer)));
    this->disconnect(track, SIGNAL(unloadingTrack(TrackPointer)), this, SLOT(slotUnloadingTrack(TrackPointer)));
}

void QmlEngine::slotLoadTrack(TrackPointer) {
    BaseTrackPlayer* sender = (BaseTrackPlayer*)this->sender();
    if(sender == NULL) {
        qWarning() << "QmlEngine::slotLoadTrack() Shouldn't happen -- sender == NULL";
        return;
    }
    emit(mixxxEvent(sender->getGroup() + ",loadTrack", 1));
}

void QmlEngine::slotNewTrackLoaded(TrackPointer) {
    BaseTrackPlayer* sender = (BaseTrackPlayer*)this->sender();
    if(sender == NULL) {
        qWarning() << "QmlEngine::slotNewTrackLoaded() Shouldn't happen -- sender == NULL";
        return;
    }
    emit(mixxxEvent(sender->getGroup() + ",newTrackLoaded", 1));
}

void QmlEngine::slotUnloadingTrack(TrackPointer) {
    BaseTrackPlayer* sender = (BaseTrackPlayer*)this->sender();
    if(sender == NULL) {
        qWarning() << "QmlEngine::slotUnloadingTrack() Shouldn't happen -- sender == NULL";
        return;
    }
    emit(mixxxEvent(sender->getGroup() + ",unloadingTrack", 1));
}

void QmlEngine::slotValueChanged(double value) {
    ControlObject* sender = (ControlObject*)this->sender();
    if(sender == NULL) {
        qWarning() << "QmlEngine::slotValueChanged() Shouldn't happen -- sender == NULL";
        return;
    }
    ConfigKey key = sender->getKey();
    QString keyString = key.group + "," + key.item;
    emit(mixxxEvent(keyString, sender->getValueToWidget(value)));
}

ControlObjectThread* QmlEngine::getControlObjectThread(QString group, QString name) {
    ConfigKey key = ConfigKey(group, name);
    ControlObjectThread *cot = NULL;

    if(!m_controlCache.contains(key)) {
        ControlObject *co = ControlObject::getControl(key);
        if(co != NULL) {
            cot = new ControlObjectThread(co);
            m_controlCache.insert(key, cot);
        }
    } else {
        cot = m_controlCache.value(key);
    }

    return cot;
}

void QmlEngine::initialized() {
    emit(mixxxInitialized());
}

