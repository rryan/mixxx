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

    void maybeWriteOSCBool(const QString& group, const QString& feature,
                           float time, bool value, QHash<QString, bool>* cache,
                           bool no_cache=false);
    void maybeWriteOSCFloat(const QString& group, const QString& feature,
                            float time, float value, QHash<QString, float>* cache,
                            bool no_cache=false);
    void writeOSCFloatValues(QString group, QString feature,
                             float time, const QList<float>& values);

    static FeatureCollector* s_pInstance;

    ConfigObject<ConfigValue>* m_pConfig;
    volatile bool m_bQuit;
    QMutex m_featuresLock;
    QList<mixxx::Features> m_features;
    lo_address m_osc_destination, m_osc_destination_secondary;

    QHash<QString, bool> m_silenceCache;
    QHash<QString, bool> m_beatCache;
    QHash<QString, float> m_bpmCache;
    QHash<QString, float> m_pitchCache;
    QHash<QString, float> m_vumeterCache;
    QHash<QString, bool> m_onsetCache;
    QHash<QString, float> m_posXCache;
    QHash<QString, float> m_posYCache;
    QHash<QString, float> m_posZCache;

    QWaitCondition m_wait;
    QMutex m_waitMutex;
};

#endif /* FEATURECOLLECTOR_H */
