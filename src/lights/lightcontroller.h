#ifndef LIGHTCONTROLLER_H
#define LIGHTCONTROLLER_H

#include <QColor>
#include <QString>
#include <QObject>
#include <QList>
#include <QMutex>
#include <QMap>
#include <QDomNode>

#include <aubio/aubio.h>
#include <lo/lo.h>

#include "defs.h"

#include "lights/featurestate.h"

class Light;
class ControlGroup;
class ColorGenerator;
class LightManager;
class DMXLightManager;
class LightBrickManager;
class LightShowController;

class LightController : public QObject {
    Q_OBJECT
  public:
    LightController();
    virtual ~LightController();

    void process(SAMPLE* pBuffer, int iFramesPerBuffer);

    void addControlGroup(ControlGroup* pGroup);
    ControlGroup* getControlGroup(int i);
    int numControlGroups();

    void addLight(Light* pLight);
    Light* getLight(int i);
    Light* getLightById(QString id);
    int numLights();

    ColorGenerator* getColorGenerator(int i);
    int numColorGenerators();
    void addColorGenerator(ColorGenerator* pGenerator);

    void addLightManager(LightManager* pManager);

    static LightController* fromXml(QDomNode node);

  signals:
    void stateUpdated();

  private:
    void process_buffer();

    // OSC address
    QString m_base_location;
    lo_address m_osc_destination;

    QMutex m_mutex;

    // aubio structures
    aubio_fft_t* m_aubio_fft;
    aubio_onset_t* m_aubio_onset;
    aubio_tempo_t* m_aubio_tempo;
    aubio_pitch_t* m_aubio_pitch;

    int m_iCurInput;
    fvec_t* m_input_buf;
    fvec_t* m_tempo_output;
    fvec_t* m_onset_output;
    fvec_t* m_pitch_output;
    cvec_t* m_fft_output;

    FeatureState m_features;

    LightShowController* m_pLightShowController;
    QList<LightManager*> m_lightManagers;
    QList<Light*> m_lights;
    QMap<QString, Light*> m_lightMap;
    QList<ControlGroup*> m_controlGroups;
    QList<ColorGenerator*> m_colorGenerators;

  public:
    // Dirty dirty
    static LightController* sInstance;
};

#endif /* LIGHTCONTROLLER_H */
