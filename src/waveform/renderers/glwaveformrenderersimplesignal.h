#ifndef GLWAVEFORMRENDERERSIMPLESIGNAL_H
#define GLWAVEFORMRENDERERSIMPLESIGNAL_H

#include "waveformrenderersignalbase.h"

class ControlObject;

class GLWaveformRendererSimpleSignal : public WaveformRendererSignalBase {
public:
    explicit GLWaveformRendererSimpleSignal(WaveformWidgetRenderer* waveformWidgetRenderer);
    ~GLWaveformRendererSimpleSignal() override;

    void onSetup(const QDomNode &node) override;
    void draw(QPainter* painter, QPaintEvent* event) override;
};

#endif // GLWAVEFORMRENDERERSIMPLESIGNAL_H
