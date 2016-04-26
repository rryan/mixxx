#ifndef GLWAVEFORMRENDERERRGB_H
#define GLWAVEFORMRENDERERRGB_H

#include "waveformrenderersignalbase.h"

class ControlObject;

class GLWaveformRendererRGB : public WaveformRendererSignalBase {
  public:
    explicit GLWaveformRendererRGB(WaveformWidgetRenderer* waveformWidgetRenderer);
    ~GLWaveformRendererRGB() override;

    void onSetup(const QDomNode& node) override;
    void draw(QPainter* painter, QPaintEvent* event) override;

  private:
    DISALLOW_COPY_AND_ASSIGN(GLWaveformRendererRGB);
};

#endif // GLWAVEFORMRENDERERRGB_H
