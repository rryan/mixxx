/**
  * Eric Van Albert
  * Midi clock out.
  */

#include "controllers/midiclockthread.h"

#include <QDebug>
#include <QDateTime>

#include "controlobject.h"
#include "controlobjectthread.h"
#include "playerinfo.h"

#include <algorithm>
#include <functional>
#include <cmath>

// A new thread for precise timing.
MidiClockThread::MidiClockThread() : QThread() {

    // Get COs
    m_pControlSampleRate=new ControlObjectThread(ControlObject::getControl(ConfigKey("[Master]","samplerate")));

    // New COs:
    // - midi_clock_out: whether or not the thread runs at all
    // - midi_clock_sync: current synchronization state. 0=unsynced, 1=syncing (waiting for a beat), 2=synced (re-adjusts timing every beat)
    // - misi_clock_do_sync: fires when a resync is requested
    // - midi_clock_out: whether or not the thread runs at all
    // - midi_clock_channel: which channel to sync to
    m_pControlMidiClockOut=new ControlObjectThread(ControlObject::getControl(ConfigKey("[Master]","midi_clock_out")));
    m_pControlMidiClockSync=new ControlObjectThread(ControlObject::getControl(ConfigKey("[Master]","midi_clock_sync")));
    m_pControlMidiClockDoSync=new ControlObjectThread(ControlObject::getControl(ConfigKey("[Master]","midi_clock_do_sync")));
    m_pControlMidiClockChannel=new ControlObjectThread(ControlObject::getControl(ConfigKey("[Master]","midi_clock_channel")));

    // Set initially which channel to listen to
    int channel=(int)(m_pControlMidiClockChannel->get());
    setChannel(channel+1); // (0 to 1 indexed)

    // m_stop=1: thread is stopped.
    m_stop=1;

    // Listen for changes to any of the things we care about
    connect(m_pControlMidiClockOut, SIGNAL(valueChanged(double)), this, SLOT(slotEnable(double)));
    connect(m_pControlMidiClockDoSync, SIGNAL(valueChanged(double)), this, SLOT(slotResync(double)));
    connect(m_pControlMidiClockChannel, SIGNAL(valueChanged(double)), this, SLOT(slotChangeChannel(double)));
}

// If we change channel using the interface button...
void MidiClockThread::slotChangeChannel(double channel)
{
    changeChannel((int)channel+1);
}

// Change the channel we are syncing to
void MidiClockThread::changeChannel(int channel)
{
    unsetChannel();
    setChannel(channel);
}

// Unsets the current channel we are listening to. First, stop trying to sync every beat. Second, remove all the signal connections to that channel.
void MidiClockThread::unsetChannel()
{
    desync();
    disconnectChannelInputs();
}

// Sets the channel we are listening to for sync. First, setup the member variable COTs (setup.) Then, make all the signal connections. Then, adjust the tempo. Do not adjust the phase, wait for a sync command to do that (otherwise may cause skipping)
void MidiClockThread::setChannel(int channel)
{
    QString s=QString("[Channel%1]").arg(channel);
    setup(s);
    connectChannelInputs();
    changeTempo(m_pControlBPM->get(),m_startTime);
}

// Setup all the member variable COTs for a given channel.
void MidiClockThread::setup(QString channel)
{
    m_pControlBPM=new ControlObjectThread(ControlObject::getControl(ConfigKey(channel,"bpm")));
    m_pControlTrackSamples=new ControlObjectThread(ControlObject::getControl(ConfigKey(channel,"track_samples")));
    m_pControlPlayPosition=new ControlObjectThread(ControlObject::getControl(ConfigKey(channel,"visual_playposition")));
    m_pControlBeatPrev=new ControlObjectThread(ControlObject::getControl(ConfigKey(channel,"beat_prev")));

    // TODO: Many more things need to trigger a desync. I could not think of all of them, so right now only loops do.
    // Examples include: playback stop/start, scratching, etc. Anything that makes the beats irregular.
    m_pControlBeatLoop=new ControlObjectThread(ControlObject::getControl(ConfigKey(channel,"loop_enabled")));
}

// Make all the necessary signal connections for the current channel.
void MidiClockThread::connectChannelInputs()
{
    connect(m_pControlBeatPrev, SIGNAL(valueChanged(double)), this, SLOT(slotBeatChanged(double)));
    connect(m_pControlBPM, SIGNAL(valueChanged(double)), this, SLOT(slotBPMChanged(double)));
    connect(m_pControlBeatLoop, SIGNAL(valueChanged(double)), this, SLOT(slotBeatLoopChanged(double)));
}

// Disconnect the signals for the current channel.
void MidiClockThread::disconnectChannelInputs()
{
    disconnect(m_pControlBeatPrev, SIGNAL(valueChanged(double)), this, SLOT(slotBeatChanged(double)));
    disconnect(m_pControlBPM, SIGNAL(valueChanged(double)), this, SLOT(slotBPMChanged(double)));
    disconnect(m_pControlBeatLoop, SIGNAL(valueChanged(double)), this, SLOT(slotBeatLoopChanged(double)));
}

// Main thread loop. Wait for the next beat, then send it.
void MidiClockThread::run() {
    m_stop=0;

    double now,go_time;

    // How far ahead of time to send the MIDI tick, to account for onset delay and things like that. Probably should be a CO.
    double anticipation=-50; // TODO FIX THIS WHY IS THIS NEGATIVE OH GOD

    // If we are just starting, then start immediately, start with tick zero, and start at 125 BPM if no BPM is available.
    m_startTime=getNow();
    m_n=0;
    m_tickSpacing=20; // Will probably be modified in changeTempo, but must be non-zero for now

    // Start the midi clock
    emit(signalMidiClockStart());

    // Main loop
    while(m_stop == 0)
    {
        now=getNow();
        m_ticker_mutex.lock();
        // go_time = time of next tick
        go_time=m_startTime+m_n*m_tickSpacing-anticipation;
        if(now>=go_time)
        {
            // If it's go_time, emit a tick
            if(m_n % 24 == 0)
            {
               qDebug() << "BEAT" << m_n << "at" << (long)now;
            }
            m_n++;
            emit(signalMidiClockTick());
            m_ticker_mutex.unlock();
        }
        else
        {
            // Else, sleep
            m_ticker_mutex.unlock();
            precisionSleep(go_time-now);
        }
    }
    // If thread is stopped, stop trying to sync
    desync();
    emit(signalMidiClockStop()); // Stop the MIDI instrument
}

// Gets the current time. TODO: make this more precise.
double MidiClockThread::getNow()
{
    return QDateTime::currentMSecsSinceEpoch();
}

// If a beatloop is enabled, desync.
void MidiClockThread::slotBeatLoopChanged(double enabled)
{
    if(enabled)
        desync();
}

// Logic to connect the enable button to starting / stopping of the thread.
void MidiClockThread::slotEnable(double enable)
{
    if(enable && m_stop)
    {
        start();
    }
    else if(!enable && !m_stop)
    {
        stop();
    }
}

// Gets what time (ms) a beat will occur / has occurred. now=current time (ms), cur_frame, frames_per_Second: self explanatory, frame: frame containing beat
double MidiClockThread::getWhatTime(double now,double cur_frame,double frames_per_second,double frame)
{
    return now+1000*(frame-cur_frame)/frames_per_second;
}

// Sleep for a precise amount of time. TODO: make this more precise.
void MidiClockThread::precisionSleep(double msecs)
{
    m_sleeper.wait(&m_mutex,(long)msecs);
}

// Converts a BPM value to a tick spacking (ms)
double MidiClockThread::bpmToMs(double bpm)
{
    return 2500/bpm;
}

// If the BPM of the song changes, adjust the midi clock accordingly.
void MidiClockThread::slotBPMChanged(double bpm)
{
    changeTempo(bpm,getNow());
}

// Stop trying to sync every beat.
void MidiClockThread::desync()
{ 
    m_pControlMidiClockSync->slotSet(SYNC_STATUS_UNSYNCED);
}

// Starts trying to resync.
void MidiClockThread::slotResync(double value)
{
    m_pControlMidiClockSync->slotSet(SYNC_STATUS_RESYNC);
}

// Called everytime prev_beat changes, indicating a beat happened. Different behavior based on current sync state.
void MidiClockThread::slotBeatChanged(double beat)
{
    double SAMPLES_PER_FRAME=2;
    double now=getNow();
    double beat_frame=beat/2;
    double cur_frame=m_pControlTrackSamples->get()*m_pControlPlayPosition->get()/SAMPLES_PER_FRAME;
    double frames_per_second=m_pControlSampleRate->get();
    int sync_status=(int)(m_pControlMidiClockSync->get());
    double lastRegularBeat=getWhatTime(now,cur_frame,frames_per_second,beat_frame);

    // This slot was not called at the exact time of the beat, due to latency. So, quantize m_b to get the m_n of the beat that just happened.
    double n_of_beat=floor((m_n+12)/24)*24; // Allow up to 12 ticks behind (pauses rather than speeding up to maintain sync)

    switch(sync_status)
    {
      case SYNC_STATUS_SYNCED: // If we are synced...
        m_ticker_mutex.lock();
        m_startTime=lastRegularBeat-n_of_beat*m_tickSpacing; // Resync by adjusting the m_startTime to match exactly with the beat we just heard.
        m_ticker_mutex.unlock();
        interruptTimer(); // Since go_time may have changed, interrupt the sleeper so it recalculates go_time.
        break;
      case SYNC_STATUS_RESYNC: // If we are resyncing and just heard a beat...
        m_ticker_mutex.lock();
        m_n=0; // Start at tick 0 again
        m_startTime=lastRegularBeat; // Set the startTime to be that last beat we heard
        // BIG NOTE: Due to latency, this function may have been called a few ticks after lastRegularBeat. Those ticks will be emitted, but in fast succession. This may mess up some MIDI instruments since the ticks are back-to-back with the START message as well as each other.
        // Stop and start to reset the MIDI instrument. This too may mess up some instruments due to the fast succession of STOP + START.
        emit(signalMidiClockStop());
        emit(signalMidiClockStart());
        m_ticker_mutex.unlock();
        interruptTimer(); // go_time has almost certainly changed.

        // Assuming we didn't fuck up the MIDI instrument, it is now resynced (or will be as soon as the queued ticks are emitted)
        qDebug() << "Resynced."; 
        m_pControlMidiClockSync->slotSet(SYNC_STATUS_SYNCED);
        break;
    }
}

// Change tempo of midi clock without changing phase. now = current time (ms)
void MidiClockThread::changeTempo(double bpm,double now)
{
    // Do not change tempo if there is no tempo to change to
    if(bpm==0)
        return;
    m_ticker_mutex.lock();
    double last_tick_time=m_startTime+(m_n-1)*m_tickSpacing; // Time that the last tick occurred
    double fractional_tick_complete=(now-last_tick_time)/m_tickSpacing; // How far through the current tick we are (0-1)
    double newTickSpacing=bpmToMs(bpm); // Self-explanatory
    double next_tick_time=now+(1-fractional_tick_complete)*newTickSpacing; // When the next tick should be (completing the current tick at the new BPM)
    double first_tick_time=next_tick_time-newTickSpacing*m_n; // When the first tick would have been if it had always been this tempo
    // All this preserves m_n in case we want to implement song position / MIDI continue in the future.
    // Set these to member variables
    m_tickSpacing=newTickSpacing;
    m_startTime=first_tick_time;
    m_ticker_mutex.unlock(); // Done with read-modify-write cycle
}

// Interrupts the thread if its sleeping. Used if go_time changes.
void MidiClockThread::interruptTimer()
{
    m_sleeper.wakeAll();
}

// Stop the MIDI clock output.
void MidiClockThread::stop() {
    m_stop=1; // mark as stopped
    interruptTimer(); // interrupt
    wait(); // join
}

// Nothing unexpected here. Leaves some connections open, since this object should really never be deleted except as the program is closing
MidiClockThread::~MidiClockThread() {
    delete m_pControlBPM;
    delete m_pControlTrackSamples;
    delete m_pControlPlayPosition;
    delete m_pControlBeatPrev;
    delete m_pControlBeatLoop;
    delete m_pControlMidiClockOut;
}

