#ifndef WAVEFORMRENDERERHSV_H
#define WAVEFORMRENDERERHSV_H

#include "util/class.h"
#include "waveformrenderersignalbase.h"

class WaveformRendererHSV : public WaveformRendererSignalBase {
  public:
    explicit WaveformRendererHSV(
        WaveformWidgetRenderer* waveformWidgetRenderer);
    ~WaveformRendererHSV() override;

    void onSetup(const QDomNode& node) override;

    void draw(QPainter* painter, QPaintEvent* event) override;

  private:
    DISALLOW_COPY_AND_ASSIGN(WaveformRendererHSV);
};

#endif // WAVEFORMRENDERERFILTEREDSIGNAL_H
