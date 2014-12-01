#ifndef SIMPLEJOGHWEEL_H
#define SIMPLEJOGHWEEL_H

#include <QObject>
#include <QString>
#include <QAtomicInt>

#include "controlpushbutton.h"
#include "controlobject.h"
#include "controlobjectslave.h"
#include "engine/scratchcontroller.h"

class SimpleJogWheel : public QObject {
    Q_OBJECT
  public:
    SimpleJogWheel(QObject* pParent, const QString& group);
    virtual ~SimpleJogWheel() {}

    void process();

  private slots:
    void slotJogTick(double ticks);
    void slotJogScratchMode(double v);
    void slotBrake(double v);
    void slotSpinback(double v);

  private:
    ScratchController m_scratchController;
    ControlPushButton m_brake;
    ControlPushButton m_spinback;
    ControlPushButton m_jogScratchMode;

    ControlObject m_jogTicks;
    ControlObject m_jogTickSensitivity;
    ControlObject m_scratchAlpha;
    ControlObject m_scratchBeta;

    ControlObjectSlave m_masterLatency;

    QAtomicInt m_queueEnable;
    QAtomicInt m_queueDisable;
    QAtomicInt m_queueSpinback;
    QAtomicInt m_queueBrake;
};

#endif /* SIMPLEJOGHWEEL_H */
