#ifndef FEATURECOLLECTOR_H
#define FEATURECOLLECTOR_H

#include <QList>
#include <QMutex>
#include <QWaitCondition>
#include <QThread>

#include <lo/lo.h>

#include "configobject.h"
#include "proto/audio_features.pb.h"

class FeatureCollector : public QThread {
  public:
    FeatureCollector(ConfigObject<ConfigValue>* pConfig);
    virtual ~FeatureCollector();

    inline static FeatureCollector* instance() {
        return s_pInstance;
    }

    void shutdown();
    void write(const mixxx::Features& features);


  protected:
    void run();

  private:
    void process();

    void writeOSCBool(const QString& group, const QString& feature,
                      float time, bool value);
    void writeOSCFloat(const QString& group, const QString& feature,
                       float time, float value);

    static FeatureCollector* s_pInstance;

    ConfigObject<ConfigValue>* m_pConfig;
    volatile bool m_bQuit;
    QMutex m_featuresLock;
    QList<mixxx::Features> m_features;
    lo_address m_osc_destination;

    QWaitCondition m_wait;
    QMutex m_waitMutex;
};

#endif /* FEATURECOLLECTOR_H */
