#ifndef QTWAVEFORMRENDERERSIMPLESIGNAL_H
#define QTWAVEFORMRENDERERSIMPLESIGNAL_H

#include "waveformrenderersignalbase.h"

#include <QBrush>
#include <QPen>

#include <QVector>

class ControlObject;

class QtWaveformRendererSimpleSignal : public WaveformRendererSignalBase {
public:
    explicit QtWaveformRendererSimpleSignal(WaveformWidgetRenderer* waveformWidgetRenderer);
    ~QtWaveformRendererSimpleSignal() override;

    void onSetup(const QDomNode &node) override;
    void draw(QPainter* painter, QPaintEvent* event) override;

protected:
    void onResize() override;

private:
    QBrush m_brush;
    QPen m_borderPen;
    QVector<QPointF> m_polygon;
};

#endif // QTWAVEFORMRENDERERSIMPLESIGNAL_H
