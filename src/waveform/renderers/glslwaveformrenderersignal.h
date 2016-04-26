#ifndef GLWAVEFORMRENDERERSIGNALSHADER_H
#define GLWAVEFORMRENDERERSIGNALSHADER_H

#include <QGLFramebufferObject>
#include <QGLShaderProgram>
#include <QtOpenGL>

#include "waveformrenderersignalbase.h"

class GLSLWaveformRendererSignal : public WaveformRendererSignalBase {
  public:
    explicit GLSLWaveformRendererSignal(
            WaveformWidgetRenderer* waveformWidgetRenderer, bool rgbShader);
    ~GLSLWaveformRendererSignal() override;

    bool onInit() override;
    void onSetup(const QDomNode& node) override;
    void draw(QPainter* painter, QPaintEvent* event) override;

    void onSetTrack() override;
    void onResize() override;

    void debugClick();
    bool loadShaders();
    bool loadTexture();

  private:
    void createGeometry();
    void createFrameBuffers();

    GLint m_unitQuadListId;
    GLuint m_textureId;

    int m_loadedWaveform;

    //Frame buffer for two pass rendering
    bool m_frameBuffersValid;
    QGLFramebufferObject* m_framebuffer;

    bool m_bDumpPng;

    // shaders
    bool m_shadersValid;
    bool m_rgbShader;
    QGLShaderProgram* m_frameShaderProgram;
};

class GLSLWaveformRendererFilteredSignal : public GLSLWaveformRendererSignal {
  public:
    explicit GLSLWaveformRendererFilteredSignal(
        WaveformWidgetRenderer* waveformWidgetRenderer)
        : GLSLWaveformRendererSignal(waveformWidgetRenderer, false) {}
    ~GLSLWaveformRendererFilteredSignal() override = default;
};

class GLSLWaveformRendererRGBSignal : public GLSLWaveformRendererSignal {
  public:
    explicit GLSLWaveformRendererRGBSignal(
        WaveformWidgetRenderer* waveformWidgetRenderer)
        : GLSLWaveformRendererSignal(waveformWidgetRenderer, true) {}
    ~GLSLWaveformRendererRGBSignal() override = default;
};

#endif // GLWAVEFORMRENDERERSIGNALSHADER_H
