#ifndef MIDICLOCKTHREAD_H
#define MIDICLOCKTHREAD_H

#include <QThread>
#include <QAtomicInt>
#include <QMutex>
#include <QWaitCondition>

#include <vector>

#include "trackinfoobject.h"
#include "track/beats.h"

class ControlObjectThread;

class MidiClockThread : public QThread {
    Q_OBJECT;

  public:
    MidiClockThread();
    virtual ~MidiClockThread();

    static const int SYNC_STATUS_UNSYNCED=0;
    static const int SYNC_STATUS_RESYNC=1;
    static const int SYNC_STATUS_SYNCED=2;

    void stop();

  public slots:
    void slotBeatChanged(double beat);
    void slotBPMChanged(double bpm);
    void slotEnable(double enable);
    void slotBeatLoopChanged(double enabled);
    void slotResync(double value);
    void slotChangeChannel(double channel);

  signals:
    void signalMidiClockTick();
    void signalMidiClockStart();
    void signalMidiClockStop();

  protected:
    void run();

  private:
    QAtomicInt m_stop;
    QAtomicInt m_resetPos;
    QWaitCondition m_sleeper;
    QMutex m_mutex;
    QMutex m_ticker_mutex;
    double m_startTime;
    double m_tickSpacing;
    int m_n;
    ControlObjectThread* m_pControlBPM;
    ControlObjectThread* m_pControlTrackSamples;
    ControlObjectThread* m_pControlPlayPosition;
    ControlObjectThread* m_pControlBeatPrev;
    ControlObjectThread* m_pControlSampleRate;
    ControlObjectThread* m_pControlBeatLoop;
    ControlObjectThread* m_pControlMidiClockOut;
    ControlObjectThread* m_pControlMidiClockSync;
    ControlObjectThread* m_pControlMidiClockDoSync;
    ControlObjectThread* m_pControlMidiClockChannel;

    double getNow();
    double getWhatTime(double now,double cur_frame,double frames_per_second,double frame);
    void precisionSleep(double msecs);
    double bpmToMs(double bpm);
    void changeTempo(double bpm,double now);
    void interruptTimer();
    void desync();
    void setup(QString channel);
    void connectChannelInputs();
    void disconnectChannelInputs();
    void changeChannel(int channel);
    void setChannel(int channel);
    void unsetChannel();
};

#endif  // MIDICLOCKTHREAD_H

