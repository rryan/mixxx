#ifndef SOFTWAREWAVEFORMWIDGET_H
#define SOFTWAREWAVEFORMWIDGET_H

#include <QWidget>

#include "waveformwidgetabstract.h"

class SoftwareWaveformWidget : public QWidget, public WaveformWidgetAbstract {
    Q_OBJECT
  public:
    ~SoftwareWaveformWidget() override;

    WaveformWidgetType::Type getType() const override { return WaveformWidgetType::SoftwareWaveform; }

    static inline QString getWaveformWidgetName() { return tr("Filtered") + " - " + tr("Software"); }
    static inline bool useOpenGl() { return false; }
    static inline bool useOpenGLShaders() { return false; }
    static inline bool developerOnly() { return false; }

  protected:
    void castToQWidget() override;
    void paintEvent(QPaintEvent* event) override;

  private:
    SoftwareWaveformWidget(const char* group, QWidget* parent);
    friend class WaveformWidgetFactory;
};

#endif // SOFTWAREWAVEFORMWIDGET_H
