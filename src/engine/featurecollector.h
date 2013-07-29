#ifndef FEATURECOLLECTOR_H
#define FEATURECOLLECTOR_H

#include <QList>
#include <QMutex>
#include <QWaitCondition>
#include <QThread>

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

    static FeatureCollector* s_pInstance;

    ConfigObject<ConfigValue>* m_pConfig;
    volatile bool m_bQuit;
    QMutex m_featuresLock;
    QList<mixxx::Features> m_features;

    QWaitCondition m_wait;
    QMutex m_waitMutex;
};

#endif /* FEATURECOLLECTOR_H */
