#ifndef OSCCONTROLLER_H
#define OSCCONTROLLER_H

#include "controllers/controller.h"
#include "controllers/osc/osccontrollerpreset.h"
#include "controllers/osc/osccontrollerpresetfilehandler.h"
#include "controllers/osc/oscmessage.h"
#include "controllers/defs_controllers.h"

class OscController : public Controller {
    Q_OBJECT
  public:
    OscController();
    virtual ~OscController();

    QString presetExtension() {
        return OSC_PRESET_EXTENSION;
    }

    virtual ControllerPresetPointer getPreset() const {
        OscControllerPreset* pClone = new OscControllerPreset();
        *pClone = m_preset;
        return ControllerPresetPointer(pClone);
    }

    virtual bool savePreset(const QString fileName) const;

    virtual ControllerPresetFileHandler* getFileHandler() const {
        return new OscControllerPresetFileHandler();
    }

    virtual void visit(const MidiControllerPreset* preset);
    virtual void visit(const HidControllerPreset* preset);
    virtual void visit(const OscControllerPreset* preset);

    virtual bool isMappable() const {
        return m_preset.isMappable();
    }

    virtual bool matchPreset(const PresetInfo& preset);

  protected slots:
    virtual void receive(const QByteArray data);

    // Initializes the engine and static output mappings.
    void applyPreset(QList<QString> scriptPaths);

  private slots:
    int open();
    int close();

  private:
    void send(QByteArray data);

    bool isPolling() const {
        return false;
    }

    // Returns a pointer to the currently loaded controller preset. For internal
    // use only.
    virtual ControllerPreset* preset() {
        return &m_preset;
    }

    OscControllerPreset m_preset;
};


#endif /* OSCCONTROLLER_H */
