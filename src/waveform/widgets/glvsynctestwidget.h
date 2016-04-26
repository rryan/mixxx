#ifndef GLVSYNCTESTWIDGET_H
#define GLVSYNCTESTWIDGET_H

#include <QGLWidget>

#include "waveformwidgetabstract.h"

class GLVSyncTestWidget : public QGLWidget, public WaveformWidgetAbstract {
    Q_OBJECT
  public:
    GLVSyncTestWidget(const char* group, QWidget* parent);
    ~GLVSyncTestWidget() override;

    WaveformWidgetType::Type getType() const override { return WaveformWidgetType::GLVSyncTest; }

    static inline QString getWaveformWidgetName() { return tr("VSyncTest"); }
    static inline bool useOpenGl() { return true; }
    static inline bool useOpenGLShaders() { return false; }
    static inline bool developerOnly() { return true; }

  protected:
    void castToQWidget() override;
    void paintEvent(QPaintEvent* event) override;
    mixxx::Duration render() override;

  private:
    friend class WaveformWidgetFactory;
};
#endif // GLVSYNCTESTWIDGET_H
