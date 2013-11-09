#ifndef OSCCONTROLLERPRESETFILEHANDLER_H
#define OSCCONTROLLERPRESETFILEHANDLER_H

#include "controllers/controllerpresetfilehandler.h"
#include "controllers/osc/osccontrollerpreset.h"
#include "controllers/mixxxcontrol.h"

class OscControllerPresetFileHandler : public ControllerPresetFileHandler {
  public:
    OscControllerPresetFileHandler() {};
    virtual ~OscControllerPresetFileHandler() {};

    bool save(const OscControllerPreset& preset,
              const QString deviceName, const QString fileName) const;

  private:
    virtual ControllerPresetPointer load(const QDomElement root, const QString deviceName,
                                         const bool forceLoad);

    void addControlsToDocument(const OscControllerPreset& preset,
                               QDomDocument* doc) const;
};

#endif /* OSCCONTROLLERPRESETFILEHANDLER_H */
