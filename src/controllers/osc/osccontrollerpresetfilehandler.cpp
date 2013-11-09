#include "controllers/osc/osccontrollerpresetfilehandler.h"

ControllerPresetPointer OscControllerPresetFileHandler::load(const QDomElement root,
                                                             const QString deviceName,
                                                             const bool forceLoad) {
    if (root.isNull()) {
        return ControllerPresetPointer();
    }

    QDomElement controller = getControllerNode(root, deviceName, forceLoad);
    if (controller.isNull()) {
        return ControllerPresetPointer();
    }

    OscControllerPreset* preset = new OscControllerPreset();

    // Superclass handles parsing <info> tag and script files
    parsePresetInfo(root, preset);
    addScriptFilesToPreset(controller, preset);

    return ControllerPresetPointer(preset);
}

bool OscControllerPresetFileHandler::save(const OscControllerPreset& preset,
                                           const QString deviceName,
                                           const QString fileName) const {
    qDebug() << "Saving preset for" << deviceName << "to" << fileName;
    QDomDocument doc = buildRootWithScripts(preset, deviceName);
    addControlsToDocument(preset, &doc);
    return writeDocument(doc, fileName);
}

void OscControllerPresetFileHandler::addControlsToDocument(const OscControllerPreset& preset,
                                                           QDomDocument* doc) const {
}
