#ifndef RGBWAVEFORMWIDGET_H
#define RGBWAVEFORMWIDGET_H

#include <QWidget>

#include "waveformwidgetabstract.h"

class RGBWaveformWidget : public QWidget, public WaveformWidgetAbstract {
    Q_OBJECT
  public:
    ~RGBWaveformWidget() override;

    WaveformWidgetType::Type getType() const override { return WaveformWidgetType::RGBWaveform; }

    static inline QString getWaveformWidgetName() { return tr("RGB"); }
    static inline bool useOpenGl() { return false; }
    static inline bool useOpenGLShaders() { return false; }
    static inline bool developerOnly() { return false; }

  protected:
    void castToQWidget() override;
    void paintEvent(QPaintEvent* event) override;

  private:
    RGBWaveformWidget(const char* group, QWidget* parent);
    friend class WaveformWidgetFactory;
};

#endif // RGBWAVEFORMWIDGET_H
