#ifndef QUANTIZECONTROL_H
#define QUANTIZECONTROL_H

#include <QObject>

#include "engine/enginecontrol.h"
#include "track/beats.h"
#include "trackinfoobject.h"

class CallbackControl;
class EngineState;
class CallbackTrackWatcher;

class QuantizeControl : public EngineControl {
    Q_OBJECT
  public:
    QuantizeControl(const char* pGroup, EngineState* pEngineState);
    virtual ~QuantizeControl();

    double process(const double dRate,
                   const double currentSample,
                   const double totalSamples,
                   const int iBufferSize);

  public slots:
    virtual void trackLoaded(TrackPointer pTrack);
    virtual void trackUnloaded(TrackPointer pTrack);

  private slots:
    void slotBeatsUpdated();

  private:
    CallbackControl* m_pCOQuantizeEnabled;
    CallbackControl* m_pCONextBeat;
    CallbackControl* m_pCOPrevBeat;
    CallbackControl* m_pCOClosestBeat;

    CallbackTrackWatcher* m_pTrackWatcher;
    TrackPointer m_pTrack;
    BeatsPointer m_pBeats;
};

#endif // QUANTIZECONTROL_H
