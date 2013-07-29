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

void FeatureCollector::writeOSCBool(const QString& group,
                                    const QString& feature,
                                    float time,
                                    bool value) {
    QString location = locationForFeature(group, feature);
    if (lo_send(m_osc_destination, location.toAscii().constData(),
                (value ? "fT" : "fF"), time) < 0) {
        qDebug() << "OSC error " << lo_address_errno(m_osc_destination)
                 << ": " << lo_address_errstr(m_osc_destination);
    }
}

void FeatureCollector::writeOSCFloat(const QString& group,
                                     const QString& feature,
                                     float time,
                                     float value) {
    QString location = locationForFeature(group, feature);
    if (lo_send(m_osc_destination, location.toAscii().constData(),
                "ff", time, value) < 0) {
        qDebug() << "OSC error " << lo_address_errno(m_osc_destination)
                 << ": " << lo_address_errstr(m_osc_destination);
    }
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
            writeOSCBool(group, "silence", feature.time(), feature.silence());
        }

        if (feature.has_beat()) {
            writeOSCBool(group, "beat", feature.time(), feature.beat());
        }

        if (feature.has_bpm()) {
            writeOSCFloat(group, "bpm", feature.time(), feature.bpm());
        }

        if (feature.has_pitch()) {
            writeOSCFloat(group, "pitch", feature.time(), feature.pitch());
        }

        if (feature.has_onset()) {
            writeOSCBool(group, "onset", feature.time(), feature.onset());
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
