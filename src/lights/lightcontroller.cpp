
#include <QtDebug>

#undef VERSION
#define AUBIO_UNSTABLE 1
#include <aubio/aubio.h>
#include <lo/lo.h>
#include <QMutexLocker>

#include "lights/light.h"
#include "lights/controlgroup.h"
#include "lights/lightmanager.h"
#include "lights/dmxlightmanager.h"
#include "lights/lightbrickmanager.h"
#include "lights/lightshowcontroller.h"

#include "lights/lightcontroller.h"
#include "lights/rgbcycler.h"
#include "lights/hsvspinner.h"
#include "lights/colorgenerator.h"

#define ANALYZER_BEAT_BUFSIZE 1024
#define ANALYZER_BEAT_OLAPSIZE 512

#define ANALYZER_BEAT_WINSIZE 1024
#define ANALYZER_BEAT_HOPSIZE 256
#define ANALYZER_BEAT_THRESHOLD 0.2
#define ANALYZER_BEAT_DCTHRESHOLD 0.8
#define ANALYZER_BEAT_SILENCE -60.

LightController* LightController::sInstance = NULL;

LightController::LightController() {

    const uint_t channels = 1;
    const uint_t samplerate = 48000;

    sInstance = this;

    QString host = "localhost";
    QString port = "2447";

    m_osc_destination = lo_address_new(host.toAscii().data(),
                                       port.toAscii().data());

    m_iCurInput = 0;
    m_input_buf = new_fvec(ANALYZER_BEAT_BUFSIZE * channels);
    // onset
    char* onset_method = "default";

    m_aubio_onset = new_aubio_onset(onset_method,
                                    ANALYZER_BEAT_BUFSIZE,
                                    ANALYZER_BEAT_OLAPSIZE,
                                    samplerate);
    m_onset_output = new_fvec(ANALYZER_BEAT_OLAPSIZE * channels);

    m_aubio_fft = new_aubio_fft(ANALYZER_BEAT_WINSIZE);
    m_fft_output = new_cvec(ANALYZER_BEAT_WINSIZE/2 * channels);
    m_features.fft = m_fft_output;

    // beattracking
    m_aubio_tempo = new_aubio_tempo(onset_method,
                                    ANALYZER_BEAT_BUFSIZE,
                                    ANALYZER_BEAT_OLAPSIZE,
                                    samplerate);
    m_tempo_output = new_fvec(2 * channels);

    m_aubio_pitch = new_aubio_pitch(onset_method,
                                    ANALYZER_BEAT_BUFSIZE,
                                    ANALYZER_BEAT_OLAPSIZE,
                                    samplerate);
    aubio_pitch_set_unit(m_aubio_pitch, "midi");
    m_pitch_output = new_fvec(1 * channels);

    RGBCycler* cycler = new RGBCycler("RGBCycle", 20, 200, 70, 50, 100, 20);
    addColorGenerator(cycler);

    // You spin me right round.
    HSVSpinner* spinner = new HSVSpinner("Wash20Step", 1.0, 1.0, 1.0, 0.05);
    addColorGenerator(spinner);

    addColorGenerator(new HSVSpinner("Wash10Step", 1.0, 1.0, 1.0, 0.1));
    addColorGenerator(new HSVSpinner("Wash100Step", 1.0, 1.0, 1.0, 0.01));
    addColorGenerator(new HSVSpinner("Wash1000Step", 1.0, 1.0, 1.0, 0.001));
    addColorGenerator(new HSVSpinner("Wash10000Step", 1.0, 1.0, 1.0, 0.0001));

    m_pLightShowController = new LightShowController(this);
}

LightController::~LightController() {
    del_aubio_onset(m_aubio_onset);
    m_aubio_onset = NULL;
    del_aubio_tempo(m_aubio_tempo);
    m_aubio_tempo = NULL;
    del_aubio_pitch(m_aubio_pitch);
    m_aubio_pitch = NULL;
    del_aubio_fft(m_aubio_fft);
    m_aubio_fft = NULL;
}

Light* LightController::getLightById(QString id) {
    if (m_lightMap.contains(id)) {
        return m_lightMap[id];
    }
    return NULL;
}

void LightController::addLight(Light* pLight) {
    Q_ASSERT(getLightById(pLight->getId()) == NULL);
    m_lights.append(pLight);
    m_lightMap.insert(pLight->getId(), pLight);
}


void LightController::process_buffer() {
    //aubio_onset_do(m_aubio_onset, m_input_buf, m_onset_output);
    aubio_tempo_do(m_aubio_tempo, m_input_buf, m_tempo_output);
    aubio_pitch_do(m_aubio_pitch, m_input_buf, m_pitch_output);
    aubio_fft_do(m_aubio_fft, m_input_buf, m_fft_output);


    m_features.is_silence = aubio_silence_detection(m_input_buf, ANALYZER_BEAT_SILENCE);
    m_features.is_beat = fvec_read_sample(m_tempo_output, 0) > 0;
    m_features.is_onset = fvec_read_sample(m_tempo_output, 1) > 0;
    m_features.pitch = fvec_read_sample(m_pitch_output, 0);

    if (m_features.is_beat) {
        m_features.beat_count++;

        m_features.previous_beat_time = m_features.current_beat_time;
        m_features.current_beat_time = QTime::currentTime();

        m_features.previous_beat_length = m_features.current_beat_length;
        m_features.current_beat_length =
                m_features.previous_beat_time.msecsTo(m_features.current_beat_time);
    }

    // Indicate that the features were generated this frame.
    m_features.is_fresh = true;

    //qDebug() << "beat: " << m_features.is_beat << " onset: " << m_state.is_onset;
    //qDebug() << "pitch:" << m_features.pitch;
    //qDebug() << "fft:" << m_fft_output->norm[0][0] << m_fft_output->norm[0][1];
}

void LightController::process(SAMPLE* pSample, int iFramesPerBuffer) {
    QMutexLocker locker(&m_mutex);

    // Set the feature struct to not be fresh. If new features are generated
    // this frame, then it will be set appropriately.
    m_features.is_fresh = false;

    Q_ASSERT(iFramesPerBuffer % 2 == 0);
    int monoSamples = iFramesPerBuffer / 2;

    for (int i = 0; i < monoSamples; ++i) {
        float monoSample = 0.5f * (float(pSample[i*2+0]) + float(pSample[i*2+1]));
        fvec_write_sample(m_input_buf, monoSample, m_iCurInput);

        // We filled up the buffer, process it.
        if (m_iCurInput == ANALYZER_BEAT_OLAPSIZE - 1) {
            process_buffer();
            m_iCurInput = -1; // gets inc'd to 0 below
        }
        m_iCurInput++;
    }

    m_features.previous_time = m_features.current_time;
    m_features.current_time = QTime::currentTime();

    // If you want the LightShowController to automatically switch the modes of
    // control groups, uncomment this.
    //m_pLightShowController->process(&m_features);

    // Process state updates for all the lights in each control group
    foreach (ControlGroup* pGroup, m_controlGroups) {
        pGroup->process(&m_features);
    }

    // Tell all the light managers to synchronize their lights with their
    // physical counterparts.
    foreach (LightManager* pManager, m_lightManagers) {
        pManager->sync();
    }

    emit(stateUpdated());
}

int LightController::numControlGroups() {
    return m_controlGroups.size();
}

void LightController::addControlGroup(ControlGroup* pGroup) {
    // TODO Workaround for stupid bug.
    if (pGroup->getColorGenerator() == NULL && m_colorGenerators.size() > 0) {
        pGroup->setColorGenerator(m_colorGenerators[0]);
    }
    m_pLightShowController->addControlGroup(pGroup);
    m_controlGroups.append(pGroup);
}

ControlGroup* LightController::getControlGroup(int group) {
    if (group < 0 || group >= m_controlGroups.size()) {
        return NULL;
    }

    return m_controlGroups.at(group);
}

int LightController::numLights() {
    return m_lights.size();
}

Light* LightController::getLight(int light) {
    if (light < 0 || light >= m_lights.size()) {
        return NULL;
    }
    return m_lights.at(light);
}

void LightController::addColorGenerator(ColorGenerator* pColorGenerator) {
    Q_ASSERT(pColorGenerator);
    m_colorGenerators.append(pColorGenerator);
}

ColorGenerator* LightController::getColorGenerator(int generator) {
    if (generator < 0 || generator >= m_colorGenerators.size()) {
        return NULL;
    }
    return m_colorGenerators.at(generator);
}

int LightController::numColorGenerators() {
    return m_colorGenerators.size();
}

void LightController::addLightManager(LightManager* pManager) {
    Q_ASSERT(pManager);
    m_lightManagers.append(pManager);
}

// static
LightController* LightController::fromXml(QDomNode node) {
    Q_ASSERT(node.nodeName() == "LightController");
    QDomNodeList children = node.childNodes();

    LightController* pController = new LightController();

    for (int i = 0; i < children.count(); ++i) {
        QDomNode child = children.at(i);
        QString tag = child.nodeName();

        if (tag == "LightManagers") {
            QDomNodeList managers = child.childNodes();
            for (int j = 0; j < managers.count(); ++j) {
                QDomNode manager = managers.at(j);
                LightManager* pManager = NULL;
                if (manager.nodeName() == "LightBrickManager") {
                    pManager = LightBrickManager::fromXml(pController, manager);
                } else if (manager.nodeName() == "DMXLightManager") {
                    pManager = DMXLightManager::fromXml(pController, manager);
                }
                if (pManager != NULL) {
                    pController->addLightManager(pManager);
                }
            }
        } else if (tag == "ControlGroups") {
            QDomNodeList groups = child.childNodes();
            for (int j = 0; j < groups.count(); ++j) {
                QDomNode group = groups.at(j);
                ControlGroup* pGroup = ControlGroup::fromXml(pController, group);
                if (pGroup != NULL) {
                    pController->addControlGroup(pGroup);
                }
            }
        }
    }
    return pController;
}
