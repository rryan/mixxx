#ifndef ENGINESTATE_H
#define ENGINESTATE_H

#include "configobject.h"
#include "engine/callbackcontrolmanager.h"
#include "engine/callbacktrackmanager.h"

class EngineState {
  public:
    EngineState(ConfigObject<ConfigValue>* pConfig)
            : m_config(pConfig) {
    }

    inline CallbackControlManager* getControlManager() {
        return &m_controlManager;
    }

    inline CallbackTrackManager* getTrackManager() {
        return &m_trackManager;
    }

    inline ConfigObject<ConfigValue>* getConfig() {
        return m_config;
    }

  private:
    ConfigObject<ConfigValue>* m_config;
    CallbackControlManager m_controlManager;
    CallbackTrackManager m_trackManager;
};


#endif /* ENGINESTATE_H */
