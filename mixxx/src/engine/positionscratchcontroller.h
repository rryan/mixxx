#ifndef POSITIONSCRATCHCONTROLLER_H
#define POSITIONSCRATCHCONTROLLER_H

#include <QObject>
#include <QString>

class CallbackControl;
class EngineState;
class VelocityController;
class RateIIFilter;

class PositionScratchController : public QObject {
  public:
    PositionScratchController(const char* pGroup, EngineState* pEngineState);
    virtual ~PositionScratchController();

    void process(double currentSample, double releaseRate,
            int iBufferSize, double baserate);
    bool isEnabled();
    double getRate();
    void notifySeek(double currentSample);

  private:
    const QString m_group;
    CallbackControl* m_pScratchEnable;
    CallbackControl* m_pScratchPosition;
    CallbackControl* m_pMasterSampleRate;
    VelocityController* m_pVelocityController;
    RateIIFilter* m_pRateIIFilter;
    bool m_bScratching;
    bool m_bEnableInertia;
    double m_dLastPlaypos;
    double m_dPositionDeltaSum;
    double m_dTargetDelta;
    double m_dStartScratchPosition;
    double m_dRate;
    double m_dMoveDelay;
    double m_dMouseSampeTime;
    double m_scratchPosition;
};

#endif /* POSITIONSCRATCHCONTROLLER_H */
