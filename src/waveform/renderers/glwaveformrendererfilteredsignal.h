#ifndef GLWAVEFROMRENDERERFILTEREDSIGNAL_H
#define GLWAVEFROMRENDERERFILTEREDSIGNAL_H

#include "waveformrenderersignalbase.h"

class ControlObject;

class GLWaveformRendererFilteredSignal : public WaveformRendererSignalBase {
  public:
    explicit GLWaveformRendererFilteredSignal(WaveformWidgetRenderer* waveformWidgetRenderer);
    ~GLWaveformRendererFilteredSignal() override;

    void onSetup(const QDomNode &node) override;
    void draw(QPainter* painter, QPaintEvent* event) override;
};

#endif // GLWAVEFROMRENDERERFILTEREDSIGNAL_H
