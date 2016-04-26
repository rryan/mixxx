#ifndef QTWAVEFORMWIDGET_H
#define QTWAVEFORMWIDGET_H

#include <QGLWidget>

#include "waveformwidgetabstract.h"

class QtWaveformWidget : public QGLWidget, public WaveformWidgetAbstract {
    Q_OBJECT
  public:
    QtWaveformWidget(const char* group, QWidget* parent);
    ~QtWaveformWidget() override;

    WaveformWidgetType::Type getType() const override { return WaveformWidgetType::QtWaveform; }

    static inline QString getWaveformWidgetName() { return tr("Filtered") + " - Qt"; }
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

#endif // QTWAVEFORMWIDGET_H
