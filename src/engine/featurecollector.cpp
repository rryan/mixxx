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
        if (feature.beat()) {
            qDebug() << feature.time() << group << "beat";
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
