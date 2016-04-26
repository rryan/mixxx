#ifndef GLRGBWAVEFORMWIDGET_H
#define GLRGBWAVEFORMWIDGET_H

#include <QGLWidget>

#include "waveformwidgetabstract.h"

class GLRGBWaveformWidget : public QGLWidget, public WaveformWidgetAbstract {
    Q_OBJECT
  public:
    GLRGBWaveformWidget(const char* group, QWidget* parent);
    ~GLRGBWaveformWidget() override;

    WaveformWidgetType::Type getType() const override { return WaveformWidgetType::GLRGBWaveform; }

    static inline QString getWaveformWidgetName() { return tr("RGB"); }
    static inline bool useOpenGl() { return true; }
    static inline bool useOpenGLShaders() { return false; }
    static inline bool developerOnly() { return false; }

  protected:
    void castToQWidget() override;
    void paintEvent(QPaintEvent* event) override;
    mixxx::Duration render() override;

  private:
    friend class WaveformWidgetFactory;
};

#endif // GLRGBWAVEFORMWIDGET_H
