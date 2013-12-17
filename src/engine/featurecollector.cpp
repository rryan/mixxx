#include <QMutexLocker>
#include <QtDebug>

#include "engine/featurecollector.h"
#include "util/uptime.h"
#include "util/stat.h"
#include "util/timer.h"

FeatureCollector* FeatureCollector::s_pInstance = NULL;

FeatureCollector::FeatureCollector(ConfigObject<ConfigValue>* pConfig)
        : m_pConfig(pConfig),
          m_bQuit(false) {
    // There can only be one.
    Q_ASSERT(s_pInstance == NULL);
    s_pInstance = this;

    QString host = "localhost";
    QString port = "2447";

    m_osc_destination = lo_address_new(host.toAscii().constData(),
                                       port.toAscii().constData());

    QString port2 = "2449";
    m_osc_destination_secondary = lo_address_new(host.toAscii().constData(),
                                                 port2.toAscii().constData());

    start();
}

FeatureCollector::~FeatureCollector() {
    shutdown();
    s_pInstance = NULL;
}

void FeatureCollector::shutdown() {
    m_bQuit = true;
    m_waitMutex.lock();
    m_wait.wakeAll();
    m_waitMutex.unlock();
    wait();
}

void FeatureCollector::write(const mixxx::Features& features) {
    QMutexLocker locker(&m_featuresLock);
    m_features.push_back(features);
    m_wait.wakeAll();
}

inline QString locationForFeature(const QString& group, const QString& feature) {
    return QString("/%1/%2").arg(group, feature);
}

void FeatureCollector::maybeWriteOSCBool(const QString& group,
                                         const QString& feature,
                                         float time,
                                         bool value,
                                         QHash<QString, bool>* pCache,
                                         bool no_cache) {
    QHash<QString, bool>::iterator it = pCache->find(group);
    if (!no_cache && it != pCache->end() && it.value() == value) {
        return;
    }

    QString location = locationForFeature(group, feature);
    if (lo_send(m_osc_destination, location.toAscii().constData(),
                (value ? "fT" : "fF"), time) < 0) {
        qDebug() << "OSC error " << lo_address_errno(m_osc_destination)
                 << ": " << lo_address_errstr(m_osc_destination);
    }
    // if (lo_send(m_osc_destination_secondary, location.toAscii().constData(),
    //             (value ? "fT" : "fF"), time) < 0) {
    //     qDebug() << "OSC error " << lo_address_errno(m_osc_destination)
    //              << ": " << lo_address_errstr(m_osc_destination);
    // }

    if (it != pCache->end()) {
        it.value() = value;
    } else {
        pCache->insert(group, value);
    }
}

void FeatureCollector::maybeWriteOSCFloat(const QString& group,
                                          const QString& feature,
                                          float time,
                                          float value,
                                          QHash<QString, float>* pCache,
                                          bool no_cache) {
    QHash<QString, float>::iterator it = pCache->find(group);
    if (!no_cache && it != pCache->end() && it.value() == value) {
        return;
    }

    QString location = locationForFeature(group, feature);
    if (lo_send(m_osc_destination, location.toAscii().constData(),
                "ff", time, value) < 0) {
        qDebug() << "OSC error " << lo_address_errno(m_osc_destination)
                 << ": " << lo_address_errstr(m_osc_destination);
    }
    if (feature == "pos_x" || feature == "pos_y") {
        if (lo_send(m_osc_destination_secondary, location.toAscii().constData(),
                    "ff", time, value) < 0) {
            qDebug() << "OSC error " << lo_address_errno(m_osc_destination)
                     << ": " << lo_address_errstr(m_osc_destination);
        }
    }
    if (it != pCache->end()) {
        it.value() = value;
    } else {
        pCache->insert(group, value);
    }
}

void FeatureCollector::writeOSCFloatValues(QString group,
                                           QString feature,
                                           float time,
                                           const QList<float>& values) {
    // QString location = locationForFeature(group, feature);
    // QString type = "f";
    // lo_bundle bundle;

    // for (int i = 0; i < values.size(); ++i) {
    //     type += "f";
    // }




    // if (lo_send(m_osc_destination, location.toAscii().constData(),
    //             type.toAscii().constData(), time,
    //             (value ? "fT" : "fF"), time) < 0) {
    //     qDebug() << "OSC error " << lo_address_errno(m_osc_destination)
    //              << ": " << lo_address_errstr(m_osc_destination);
    // }

}

void FeatureCollector::process() {
    m_featuresLock.lock();
    QList<mixxx::Features> features = m_features;
    m_features.clear();
    m_featuresLock.unlock();

    float now = static_cast<float>(Uptime::uptimeNanos()) / 1e9;

    foreach (const mixxx::Features& feature, features) {
        Stat::track("FeatureCollector latency", Stat::DURATION_SEC,
                    kDefaultComputeFlags, now - feature.time());

        QString group = QString::fromStdString(feature.group());
        QString baseDestination = QString("/%1/").arg(group);

        if (feature.has_silence()) {
            maybeWriteOSCBool(group, "silence", feature.time(), feature.silence(),
                              &m_silenceCache);
        }

        if (feature.has_beat()) {
            maybeWriteOSCBool(group, "beat", feature.time(), feature.beat(),
                              &m_beatCache);
        }

        if (feature.has_bpm()) {
            maybeWriteOSCFloat(group, "bpm", feature.time(), feature.bpm(),
                               &m_bpmCache);
        }

        if (feature.has_pitch()) {
            maybeWriteOSCFloat(group, "pitch", feature.time(), feature.pitch(),
                               &m_pitchCache);
        }

        if (feature.has_onset()) {
            maybeWriteOSCBool(group, "onset", feature.time(), feature.onset(),
                              &m_onsetCache);
        }

        if (feature.has_vumeter()) {
            maybeWriteOSCFloat(group, "vumeter", feature.time(), feature.vumeter(),
                               &m_vumeterCache);
        }

        // if (feature.has_fft()) {
        //     QList<float> fft;
        //     for (int i = 0; i < feature.fft_size(); ++i) {
        //         fft.push_back(feature.fft(i)));
        //     }
        //     writeOSCFloatValues(group, "fft", feature.time(), fft);
        // }

        if (feature.pos_size() == 3) {
            maybeWriteOSCFloat(group, "pos_x", feature.time(), feature.pos(0), &m_posXCache, true);
            maybeWriteOSCFloat(group, "pos_y", feature.time(), feature.pos(1), &m_posYCache, true);
            maybeWriteOSCFloat(group, "pos_z", feature.time(), feature.pos(2), &m_posZCache, true);
        }
    }
}

void FeatureCollector::run() {
    while (!m_bQuit) {
        process();

        m_waitMutex.lock();
        m_wait.wait(&m_waitMutex);
        m_waitMutex.unlock();
    }
}
