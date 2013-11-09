#include "controllers/osc/osccontroller.h"

OscController::OscController() {
}

OscController::~OscController() {
}

bool OscController::savePreset(const QString fileName) const {
    return false;
}

void OscController::visit(const MidiControllerPreset* preset) {
    Q_UNUSED(preset);
    // TODO(XXX): throw a hissy fit.
    qDebug() << "ERROR: Attempting to load a MidiControllerPreset to an OscController!";
}

void OscController::visit(const HidControllerPreset* preset) {
    Q_UNUSED(preset);
    // TODO(XXX): throw a hissy fit.
    qDebug() << "ERROR: Attempting to load a HidControllerPreset to an OscController!";
}

void OscController::visit(const OscControllerPreset* preset) {
    m_preset = *preset;
    // Emit presetLoaded with a clone of the preset.
    emit(presetLoaded(getPreset()));
}

bool OscController::matchPreset(const PresetInfo& preset) {
    return false;
}

void OscController::receive(const QByteArray data) {
}

void OscController::applyPreset(QList<QString> scriptPaths) {
}

int OscController::open() {
    return 0;
}

int OscController::close() {
    return 0;
}

void OscController::send(QByteArray data) {
}
