#ifndef EMPTYWAVEFORM_H
#define EMPTYWAVEFORM_H

#include <QWidget>

#include "waveform/widgets/waveformwidgetabstract.h"

// This class can be used as a template file to create new WaveformWidgets it
// contain minimal set of method to re-implement

class EmptyWaveformWidget : public QWidget, public WaveformWidgetAbstract {
    Q_OBJECT
  public:
    ~EmptyWaveformWidget() override;

    WaveformWidgetType::Type getType() const override { return WaveformWidgetType::EmptyWaveform; }

    static inline QString getWaveformWidgetName() { return tr("Empty"); }
    static inline bool useOpenGl() { return false; }
    static inline bool useOpenGLShaders() { return false; }
    static inline bool developerOnly() { return false; }

  protected:
    void castToQWidget() override;
    void paintEvent(QPaintEvent* event) override;
    mixxx::Duration render() override;

  private:
    EmptyWaveformWidget(const char* group, QWidget* parent);
    friend class WaveformWidgetFactory;
};

#endif // EMPTYWAVEFORM_H
