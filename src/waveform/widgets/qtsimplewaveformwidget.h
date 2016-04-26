#ifndef QTSIMPLEWAVEFORMWIDGET_H
#define QTSIMPLEWAVEFORMWIDGET_H

#include <QGLWidget>

#include "waveformwidgetabstract.h"

class QtSimpleWaveformWidget : public QGLWidget, public WaveformWidgetAbstract {
    Q_OBJECT
  public:
    QtSimpleWaveformWidget(const char* group, QWidget* parent);
    ~QtSimpleWaveformWidget() override;


    WaveformWidgetType::Type getType() const override { return WaveformWidgetType::GLSimpleWaveform; }

    static inline QString getWaveformWidgetName() { return tr("Simple") + " - Qt"; }
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

#endif // QTSIMPLEWAVEFORMWIDGET_H
