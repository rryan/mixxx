#ifndef WAVEFORMRENDERMARK_H
#define WAVEFORMRENDERMARK_H

#include "skin/skincontext.h"
#include "util/class.h"
#include "waveform/renderers/waveformmarkset.h"
#include "waveform/renderers/waveformrendererabstract.h"

class WaveformRenderMark : public WaveformRendererAbstract {
  public:
    explicit WaveformRenderMark(WaveformWidgetRenderer* waveformWidgetRenderer);

    void setup(const QDomNode& node, const SkinContext& context) override;
    void draw(QPainter* painter, QPaintEvent* event) override;

  private:
    void generateMarkImage(WaveformMark& mark);

    WaveformMarkSet m_marks;
    DISALLOW_COPY_AND_ASSIGN(WaveformRenderMark);
};

#endif
