#ifndef WAVEFORMRENDERBACKGROUND_H
#define WAVEFORMRENDERBACKGROUND_H

#include <QColor>
#include <QDomNode>
#include <QPaintEvent>
#include <QPainter>

#include "skin/skincontext.h"
#include "util/class.h"
#include "waveformrendererabstract.h"

class WaveformWidgetRenderer;

class WaveformRenderBackground : public WaveformRendererAbstract {
  public:
    explicit WaveformRenderBackground(WaveformWidgetRenderer* waveformWidgetRenderer);
    ~WaveformRenderBackground() override;

    void setup(const QDomNode& node, const SkinContext& context) override;
    void draw(QPainter* painter, QPaintEvent* event) override;

  private:
    void generateImage();

    QString m_backgroundPixmapPath;
    QColor m_backgroundColor;
    QImage m_backgroundImage;

    DISALLOW_COPY_AND_ASSIGN(WaveformRenderBackground);
};

#endif /* WAVEFORMRENDERBACKGROUND_H */
