#ifndef OSCCONTROLLERPRESET_H
#define OSCCONTROLLERPRESET_H

#include "controllers/controllerpreset.h"
#include "controllers/controllerpresetvisitor.h"
#include "controllers/mixxxcontrol.h"
#include "controllers/osc/oscmessage.h"

class OscControllerPreset : public ControllerPreset {
  public:
    OscControllerPreset() {}
    virtual ~OscControllerPreset() {}

    virtual void accept(ControllerPresetVisitor* visitor) const {
        if (visitor) {
            visitor->visit(this);
        }
    }

    virtual bool isMappable() const {
        return true;
    }
};

#endif /* OSCCONTROLLERPRESET_H */
