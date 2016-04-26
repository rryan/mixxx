#ifndef QTWAVEFROMRENDERERFILTEREDSIGNAL_H
#define QTWAVEFROMRENDERERFILTEREDSIGNAL_H

#include "waveformrenderersignalbase.h"

#include <QBrush>
#include <QVector>

class ControlObject;

class QtWaveformRendererFilteredSignal : public WaveformRendererSignalBase {
  public:
    explicit QtWaveformRendererFilteredSignal(WaveformWidgetRenderer* waveformWidgetRenderer);
    ~QtWaveformRendererFilteredSignal() override;

    void onSetup(const QDomNode &node) override;
    void draw(QPainter* painter, QPaintEvent* event) override;

  protected:
    void onResize() override;
    int buildPolygon();

  protected:
    QBrush m_lowBrush;
    QBrush m_midBrush;
    QBrush m_highBrush;
    QBrush m_lowKilledBrush;
    QBrush m_midKilledBrush;
    QBrush m_highKilledBrush;

    QVector<QPointF> m_polygon[3];
};

#endif // QTWAVEFROMRENDERERFILTEREDSIGNAL_H
