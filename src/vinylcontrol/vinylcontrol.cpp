#include "vinylcontrol/vinylcontrol.h"
#include "controlobjectthread.h"
#include "controlobject.h"

VinylControl::VinylControl(ConfigObject<ConfigValue> * pConfig, QString group)
{
    m_pConfig = pConfig;
    m_group = group;

    iSampleRate = m_pConfig->getValueString(ConfigKey("[Soundcard]","Samplerate")).toULong();

    // Get Control objects
    playPos             = new ControlObjectThread(group, "playposition");    //Range: -.14 to 1.14
    trackSamples        = new ControlObjectThread(group, "track_samples");
    trackSampleRate     = new ControlObjectThread(group, "track_samplerate");
    vinylSeek           = new ControlObjectThread(group, "vinylcontrol_seek");
    controlScratch      = new ControlObjectThread(group, "scratch2");
    rateSlider          = new ControlObjectThread(group, "rate");    //Range -1.0 to 1.0
    playButton          = new ControlObjectThread(group, "play");
    reverseButton       = new ControlObjectThread(group, "reverse");
    duration            = new ControlObjectThread(group, "duration");
    mode                = new ControlObjectThread(group, "vinylcontrol_mode");
    enabled             = new ControlObjectThread(group, "vinylcontrol_enabled");
    wantenabled         = new ControlObjectThread(group, "vinylcontrol_wantenabled");
    cueing              = new ControlObjectThread(group, "vinylcontrol_cueing");
    scratching          = new ControlObjectThread(group, "vinylcontrol_scratching");
    rateRange           = new ControlObjectThread(group, "rateRange");
    vinylStatus         = new ControlObjectThread(group, "vinylcontrol_status");
    rateDir             = new ControlObjectThread(group, "rate_dir");
    loopEnabled         = new ControlObjectThread(group, "loop_enabled");
    signalenabled       = new ControlObjectThread(group, "vinylcontrol_signal_enabled");

    dVinylPitch = 0.0f;
    dVinylPosition = 0.0f;
    dVinylScratch = 0.0f;
    dDriftControl   = 0.0f;
    fRateRange = 0.0f;
    m_fTimecodeQuality = 0.0f;

    //Get the vinyl type
    strVinylType = m_pConfig->getValueString(ConfigKey(group,"vinylcontrol_vinyl_type"));

    //Get the vinyl speed
    strVinylSpeed = m_pConfig->getValueString(ConfigKey(group,"vinylcontrol_speed_type"));

    //Get the lead-in time
    iLeadInTime = m_pConfig->getValueString(ConfigKey(VINYL_PREF_KEY,"lead_in_time")).toInt();

    //Enabled or not -- load from saved value in case vinyl control is restarting
    bIsEnabled = wantenabled->get();

    //Gain
    ControlObject::set(ConfigKey(VINYL_PREF_KEY, "gain"),
        m_pConfig->getValueString(ConfigKey(VINYL_PREF_KEY,"gain")).toInt());
}

bool VinylControl::isEnabled() {
    return bIsEnabled;
}

void VinylControl::toggleVinylControl(bool enable)
{
    bIsEnabled = enable;
    if (m_pConfig)
    {
        m_pConfig->set(ConfigKey(m_group,"vinylcontrol_enabled"), ConfigValue((int)enable));
    }

    enabled->slotSet(enable);

    //Reset the scratch control to make sure we don't get stuck moving forwards or backwards.
    //actually that might be a good thing
    //if (!enable)
    //    controlScratch->slotSet(0.0f);
}

VinylControl::~VinylControl()
{
    bool wasEnabled = bIsEnabled;
    enabled->slotSet(false);
    vinylStatus->slotSet(VINYL_STATUS_DISABLED);
    if (wasEnabled)
    {
        //if vinyl control is just restarting, indicate that it should
        //be enabled
        wantenabled->slotSet(true);
    }

    delete playPos;
    delete trackSamples;
    delete trackSampleRate;
    delete vinylSeek;
    delete controlScratch;
    delete rateSlider;
    delete playButton;
    delete reverseButton;
    delete duration;
    delete mode;
    delete enabled;
    delete wantenabled;
    delete cueing;
    delete scratching;
    delete rateRange;
    delete vinylStatus;
    delete rateDir;
    delete loopEnabled;
    delete signalenabled;
}

float VinylControl::getSpeed()
{
    return dVinylScratch;
}

