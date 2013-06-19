// loopingcontrol.h
// Created on Sep 23, 2008
// Author: asantoni, rryan

#ifndef LOOPINGCONTROL_H
#define LOOPINGCONTROL_H

#include <QObject>

#include "engine/enginecontrol.h"
#include "trackinfoobject.h"
#include "track/beats.h"

#define MINIMUM_AUDIBLE_LOOP_SIZE   30  // In samples

class EngineState;
class CallbackControl;
class BeatLoopingControl;
class CallbackTrackWatcher;

class LoopingControl : public EngineControl {
    Q_OBJECT
  public:
    LoopingControl(const char * _group, EngineState* pEngineState);
    virtual ~LoopingControl();

    // process() updates the internal state of the LoopingControl to reflect the
    // correct current sample. If a loop should be taken LoopingControl returns
    // the sample that should be seeked to. Otherwise it returns currentSample.
    double process(const double dRate,
                   const double currentSample,
                   const double totalSamples,
                   const int iBufferSize);

    // nextTrigger returns the sample at which the engine will be triggered to
    // take a loop, given the value of currentSample and dRate.
    double nextTrigger(const double dRate,
                       const double currentSample,
                       const double totalSamples,
                       const int iBufferSize);

    // getTrigger returns the sample that the engine will next be triggered to
    // loop to, given the value of currentSample and dRate.
    double getTrigger(const double dRate,
                      const double currentSample,
                      const double totalSamples,
                      const int iBufferSize);

    // hintReader will add to hintList hints both the loop in and loop out
    // sample, if set.
    void hintReader(QList<Hint>& hintList);

    void notifySeek(double dNewPlaypos);

  public slots:
    void slotLoopIn(double);
    void slotLoopOut(double);
    void slotLoopExit(double);
    void slotReloopExit(double);
    void slotLoopStartPos(double);
    void slotLoopEndPos(double);
    virtual void trackLoaded(TrackPointer pTrack);
    virtual void trackUnloaded(TrackPointer pTrack);
    void slotUpdatedTrackBeats();

    // Generate a loop of 'beats' length. It can also do fractions for a
    // beatslicing effect.
    void slotBeatLoop(double loopSize, bool keepStartPoint=false);
    void slotBeatLoopActivate(BeatLoopingControl* pBeatLoopControl);
    void slotBeatLoopActivateRoll(BeatLoopingControl* pBeatLoopControl);
    void slotBeatLoopDeactivate(BeatLoopingControl* pBeatLoopControl);
    void slotBeatLoopDeactivateRoll(BeatLoopingControl* pBeatLoopControl);

    void slotLoopScale(double);
    void slotLoopDouble(double);
    void slotLoopHalve(double);

  private:
    void setLoopingEnabled(bool enabled);
    void clearActiveBeatLoop();

    CallbackControl* m_pCOLoopStartPosition;
    CallbackControl* m_pCOLoopEndPosition;
    CallbackControl* m_pCOLoopEnabled;
    CallbackControl* m_pLoopInButton;
    CallbackControl* m_pLoopOutButton;
    CallbackControl* m_pLoopExitButton;
    CallbackControl* m_pReloopExitButton;
    CallbackControl* m_pCOLoopScale;
    CallbackControl* m_pLoopHalveButton;
    CallbackControl* m_pLoopDoubleButton;
    CallbackControl* m_pSlipEnabled;

    bool m_bLoopingEnabled;
    int m_iLoopEndSample;
    int m_iLoopStartSample;
    int m_iCurrentSample;

    // Controls not owned by LoopingControl
    CallbackControl* m_pQuantizeEnabled;
    CallbackControl* m_pNextBeat;
    CallbackControl* m_pClosestBeat;
    CallbackControl* m_pTrackSamples;

    BeatLoopingControl* m_pActiveBeatLoop;

    // Base BeatLoop Control Object.
    CallbackControl* m_pCOBeatLoop;
    // Different sizes for Beat Loops/Seeks.
    static double s_dBeatSizes[];
    // Array of BeatLoopingControls, one for each size.
    QList<BeatLoopingControl*> m_beatLoops;

    CallbackTrackWatcher* m_pTrackWatcher;
    TrackPointer m_pTrack;
    BeatsPointer m_pBeats;
};

// Class for handling beat loops of a set size. This allows easy access from
// skins.
class BeatLoopingControl : public QObject {
    Q_OBJECT
  public:
    BeatLoopingControl(const char* pGroup, double size,
                       EngineState* pEngineState);
    virtual ~BeatLoopingControl();

    void activate();
    void deactivate();
    inline double getSize() {
        return m_dBeatLoopSize;
    }
  public slots:
    void slotLegacy(double value);
    void slotActivate(double value);
    void slotActivateRoll(double value);
    void slotToggle(double value);

  signals:
    void activateBeatLoop(BeatLoopingControl*);
    void deactivateBeatLoop(BeatLoopingControl*);
    void activateBeatLoopRoll(BeatLoopingControl*);
    void deactivateBeatLoopRoll(BeatLoopingControl*);

  private:
    // Used simply to generate the beatloop_%SIZE and beatseek_%SIZE CO
    // ConfigKeys.
    ConfigKey keyForControl(const char * _group, QString ctrlName, double num);
    double m_dBeatLoopSize;
    bool m_bActive;
    CallbackControl* m_pLegacy;
    CallbackControl* m_pActivate;
    CallbackControl* m_pActivateRoll;
    CallbackControl* m_pToggle;
    CallbackControl* m_pEnabled;
};

#endif /* LOOPINGCONTROL_H */
