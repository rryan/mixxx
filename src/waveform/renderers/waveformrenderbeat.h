#ifndef WAVEFORMRENDERBEAT_H
#define WAVEFORMRENDERBEAT_H

#include <QColor>

#include "skin/skincontext.h"
#include "util/class.h"
#include "waveform/renderers/waveformrendererabstract.h"

class WaveformRenderBeat : public WaveformRendererAbstract {
  public:
    explicit WaveformRenderBeat(WaveformWidgetRenderer* waveformWidgetRenderer);
    ~WaveformRenderBeat() override;

    void setup(const QDomNode& node, const SkinContext& context) override;
    void draw(QPainter* painter, QPaintEvent* event) override;

  private:
    QColor m_beatColor;
    QVector<QLineF> m_beats;

    DISALLOW_COPY_AND_ASSIGN(WaveformRenderBeat);
};

#endif //WAVEFORMRENDERBEAT_H
