#ifndef CLOCKCONTROL_H
#define CLOCKCONTROL_H

#include "engine/enginecontrol.h"
#include "track/beats.h"
#include "trackinfoobject.h"

class EngineState;
class CallbackControl;
class CallbackTrackWatcher;

class ClockControl: public EngineControl {
    Q_OBJECT
  public:
    ClockControl(const char * pGroup,
                 EngineState* pConfig);
    virtual ~ClockControl();

    double process(const double dRate, const double currentSample,
                   const double totalSamples, const int iBufferSize);

  public slots:
    virtual void trackLoaded(TrackPointer pTrack);
    virtual void trackUnloaded(TrackPointer pTrack);
    void slotBeatsUpdated();

  private:
    CallbackControl* m_pCOBeatActive;
    CallbackControl* m_pCOSampleRate;
    CallbackTrackWatcher* m_pTrackWatcher;
    TrackPointer m_pTrack;
    BeatsPointer m_pBeats;
};

#endif /* CLOCKCONTROL_H */
