#ifndef GLSIMPLEWAVEFORMWIDGET_H
#define GLSIMPLEWAVEFORMWIDGET_H

#include <QGLWidget>

#include "waveformwidgetabstract.h"

class GLSimpleWaveformWidget : public QGLWidget, public WaveformWidgetAbstract {
    Q_OBJECT
  public:
    GLSimpleWaveformWidget(const char* group, QWidget* parent);
    ~GLSimpleWaveformWidget() override;

    WaveformWidgetType::Type getType() const override { return WaveformWidgetType::GLSimpleWaveform; }

    static inline QString getWaveformWidgetName() { return tr("Simple"); }
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
#endif // GLSIMPLEWAVEFORMWIDGET_H
