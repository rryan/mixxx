
// bpmcontrol.h
// Created 7/5/2009 by RJ Ryan (rryan@mit.edu)

#ifndef BPMCONTROL_H
#define BPMCONTROL_H

#include "engine/enginecontrol.h"
#include "tapfilter.h"

class CallbackControl;
class EngineState;
class CallbackTrackWatcher;
class EngineBuffer;

class BpmControl : public EngineControl {
    Q_OBJECT

  public:
    BpmControl(const char* _group, ConfigObject<ConfigValue>* _config,
               EngineState* pEngineState);
    virtual ~BpmControl();
    double getBpm();
    double getFileBpm();

  public slots:
    virtual void trackLoaded(TrackPointer pTrack);
    virtual void trackUnloaded(TrackPointer pTrack);

  private slots:
    void slotSetEngineBpm(double);
    void slotControlBeatSync(double);
    void slotControlBeatSyncPhase(double);
    void slotControlBeatSyncTempo(double);
    void slotTapFilter(double,int);
    void slotBpmTap(double);
    void slotAdjustBpm();
    void slotUpdatedTrackBeats();
    void slotBeatsTranslate(double);

  private:
    EngineBuffer* pickSyncTarget();
    bool syncTempo(EngineBuffer* pOtherEngineBuffer);
    bool syncPhase(EngineBuffer* pOtherEngineBuffer);

    // ControlObjects that come from PlayerManager
    CallbackControl* m_pNumDecks;

    // CallbackControls that come from EngineBuffer
    CallbackControl* m_pPlayButton;
    CallbackControl* m_pRateSlider;
    CallbackControl* m_pRateRange;
    CallbackControl* m_pRateDir;

    // CallbackControls that come from LoopingControl
    CallbackControl* m_pLoopEnabled;
    CallbackControl* m_pLoopStartPosition;
    CallbackControl* m_pLoopEndPosition;

    // The current loaded file's detected BPM
    CallbackControl* m_pFileBpm;

    // The current effective BPM of the engine
    CallbackControl* m_pEngineBpm;

    // Used for bpm tapping from GUI and MIDI
    CallbackControl* m_pButtonTap;

    // Button for sync'ing with the other EngineBuffer
    CallbackControl* m_pButtonSync;
    CallbackControl* m_pButtonSyncPhase;
    CallbackControl* m_pButtonSyncTempo;

    // Button that translates the beats so the nearest beat is on the current
    // playposition.
    CallbackControl* m_pTranslateBeats;

    TapFilter m_tapFilter;

    CallbackTrackWatcher* m_pTrackWatcher;
    TrackPointer m_pTrack;
    BeatsPointer m_pBeats;
};


#endif /* BPMCONTROL_H */
