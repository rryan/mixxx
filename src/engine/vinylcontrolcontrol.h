#ifndef VINYLCONTROLCONTROL_H
#define VINYLCONTROLCONTROL_H

#include "controlobject.h"
#include "controlpushbutton.h"
#include "engine/enginecontrol.h"
#include "trackinfoobject.h"

class CallbackControl;
class EngineState;

class VinylControlControl : public EngineControl {
    Q_OBJECT
  public:
    VinylControlControl(const char* pGroup, EngineState* pEngineState);
    virtual ~VinylControlControl();

    void trackLoaded(TrackPointer pTrack);
    void trackUnloaded(TrackPointer pTrack);
    bool isEnabled();
    bool isScratching();

  private slots:
    void slotControlVinylSeek(double value);

  private:
    CallbackControl* m_pControlVinylEnabled;
    CallbackControl* m_pControlVinylMode;
    CallbackControl* m_pControlVinylCueing;
    CallbackControl* m_pControlVinylScratching;
    TrackPointer m_pCurrentTrack;
};

#endif /* VINYLCONTROLCONTROL_H */
