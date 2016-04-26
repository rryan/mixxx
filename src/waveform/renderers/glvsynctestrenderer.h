#ifndef GLVSYNCTESTRENDERER_H
#define GLVSYNCTESTRENDERER_H

#include "waveformrenderersignalbase.h"

class ControlObject;

class GLVSyncTestRenderer : public WaveformRendererSignalBase {
  public:
    explicit GLVSyncTestRenderer(WaveformWidgetRenderer* waveformWidgetRenderer);
    ~GLVSyncTestRenderer() override;

    void onSetup(const QDomNode &node) override;
    void draw(QPainter* painter, QPaintEvent* event) override;
  private:
    int m_drawcount;
};

#endif // GLVSYNCTESTRENDERER_H
