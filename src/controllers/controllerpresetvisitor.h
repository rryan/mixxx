#ifndef CONTROLLERPRESETVISITOR_H
#define CONTROLLERPRESETVISITOR_H

class MidiControllerPreset;
class HidControllerPreset;
class OscControllerPreset;

class ControllerPresetVisitor {
  public:
    virtual void visit(MidiControllerPreset* preset) = 0;
    virtual void visit(HidControllerPreset* preset) = 0;
};

class ConstControllerPresetVisitor {
  public:
    virtual void visit(const MidiControllerPreset* preset) = 0;
    virtual void visit(const HidControllerPreset* preset) = 0;
    virtual void visit(const OscControllerPreset* preset) = 0;
};

#endif /* CONTROLLERPRESETVISITOR_H */
