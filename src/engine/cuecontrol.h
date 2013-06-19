// cuecontrol.h
// Created 11/5/2009 by RJ Ryan (rryan@mit.edu)

#ifndef CUECONTROL_H
#define CUECONTROL_H

#include <QList>

#include "engine/enginecontrol.h"
#include "configobject.h"
#include "trackinfoobject.h"

#define NUM_HOT_CUES 37

class CallbackControl;
class Cue;
class EngineState;
class CallbackTrackWatcher;

class HotcueControl : public QObject {
    Q_OBJECT
  public:
    HotcueControl(const char* pGroup, int hotcueNumber,
                  EngineState* pEngineState);
    virtual ~HotcueControl();

    inline int getHotcueNumber() { return m_iHotcueNumber; }
    inline Cue* getCue() { return m_pCue; }
    inline void setCue(Cue* pCue) { m_pCue = pCue; }
    inline CallbackControl* getPosition() { return m_hotcuePosition; }
    inline CallbackControl* getEnabled() { return m_hotcueEnabled; }

    // Used for caching the preview state of this hotcue control.
    inline bool isPreviewing() { return m_bPreviewing; }
    inline void setPreviewing(bool bPreviewing) { m_bPreviewing = bPreviewing; }
    inline int getPreviewingPosition() { return m_iPreviewingPosition; }
    inline void setPreviewingPosition(int iPosition) { m_iPreviewingPosition = iPosition; }

  private slots:
    void slotHotcueSet(double v);
    void slotHotcueGoto(double v);
    void slotHotcueGotoAndPlay(double v);
    void slotHotcueGotoAndStop(double v);
    void slotHotcueActivate(double v);
    void slotHotcueActivatePreview(double v);
    void slotHotcueClear(double v);
    void slotHotcuePositionChanged(double newPosition);

  signals:
    void hotcueSet(HotcueControl* pHotcue, double v);
    void hotcueGoto(HotcueControl* pHotcue, double v);
    void hotcueGotoAndPlay(HotcueControl* pHotcue, double v);
    void hotcueGotoAndStop(HotcueControl* pHotcue, double v);
    void hotcueActivate(HotcueControl* pHotcue, double v);
    void hotcueActivatePreview(HotcueControl* pHotcue, double v);
    void hotcueClear(HotcueControl* pHotcue, double v);
    void hotcuePositionChanged(HotcueControl* pHotcue, double newPosition);
    void hotcuePlay(double v);

  private:
    ConfigKey keyForControl(int hotcue, QString name);

    const char* m_pGroup;
    int m_iHotcueNumber;
    Cue* m_pCue;

    // Hotcue state controls
    CallbackControl* m_hotcuePosition;
    CallbackControl* m_hotcueEnabled;
    // Hotcue button controls
    CallbackControl* m_hotcueSet;
    CallbackControl* m_hotcueGoto;
    CallbackControl* m_hotcueGotoAndPlay;
    CallbackControl* m_hotcueGotoAndStop;
    CallbackControl* m_hotcueActivate;
    CallbackControl* m_hotcueActivatePreview;
    CallbackControl* m_hotcueClear;

    bool m_bPreviewing;
    int m_iPreviewingPosition;
};

class CueControl : public EngineControl {
    Q_OBJECT
  public:
    CueControl(const char * _group, EngineState* pEngineState);
    virtual ~CueControl();

    virtual void hintReader(QList<Hint>& hintList);

  public slots:
    virtual void trackLoaded(TrackPointer pTrack);
    virtual void trackUnloaded(TrackPointer pTrack);

  private slots:
    void trackCuesUpdated();
    void hotcueSet(HotcueControl* pControl, double v);
    void hotcueGoto(HotcueControl* pControl, double v);
    void hotcueGotoAndPlay(HotcueControl* pControl, double v);
    void hotcueGotoAndStop(HotcueControl* pControl, double v);
    void hotcueActivate(HotcueControl* pControl, double v);
    void hotcueActivatePreview(HotcueControl* pControl, double v);
    void hotcueClear(HotcueControl* pControl, double v);
    void hotcuePositionChanged(HotcueControl* pControl, double newPosition);

    void cueSet(double v);
    void cueGoto(double v);
    void cueGotoAndPlay(double v);
    void cueGotoAndStop(double v);
    void cueSimple(double v);
    void cuePreview(double v);
    void cueCDJ(double v);
    void cueDefault(double v);
    void playFromCuePreview(double v);

  private:
    void createControls(EngineState* pEngineState);
    void attachCue(Cue* pCue, int hotcueNumber);
    void detachCue(int hotcueNumber);
    void saveCuePoint(double cuePoint);

    bool m_bHotcueCancel;
    bool m_bPreviewing;
    bool m_bPreviewingHotcue;
    int m_iCurrentlyPreviewingHotcues;

    // Controls not owned by CueControl
    CallbackControl* m_pPlayButton;
    CallbackControl* m_pTrackSamples;
    CallbackControl* m_pQuantizeEnabled;
    CallbackControl* m_pNextBeat;
    CallbackControl* m_pClosestBeat;

    const int m_iNumHotCues;
    QList<HotcueControl*> m_hotcueControl;

    CallbackControl* m_pCuePoint;
    CallbackControl* m_pCueMode;
    CallbackControl* m_pCueSet;
    CallbackControl* m_pCueSimple;
    CallbackControl* m_pCueCDJ;
    CallbackControl* m_pCueDefault;
    CallbackControl* m_pCueGoto;
    CallbackControl* m_pCueGotoAndPlay;
    CallbackControl* m_pCueGotoAndStop;
    CallbackControl* m_pCuePreview;

    CallbackTrackWatcher* m_pTrackWatcher;
    TrackPointer m_pLoadedTrack;

    // Tells us which controls map to which hotcue
    QMap<QObject*, int> m_controlMap;
};


#endif /* CUECONTROL_H */
