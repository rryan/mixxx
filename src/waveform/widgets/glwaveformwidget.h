#ifndef GLWAVEFORMWIDGET_H
#define GLWAVEFORMWIDGET_H

#include <QGLWidget>

#include "waveformwidgetabstract.h"

class GLWaveformWidget : public QGLWidget, public WaveformWidgetAbstract {
    Q_OBJECT
  public:
    GLWaveformWidget(const char* group, QWidget* parent);
    ~GLWaveformWidget() override;

    WaveformWidgetType::Type getType() const override { return WaveformWidgetType::GLFilteredWaveform; }

    static inline QString getWaveformWidgetName() { return tr("Filtered"); }
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

#endif // GLWAVEFORMWIDGET_H
