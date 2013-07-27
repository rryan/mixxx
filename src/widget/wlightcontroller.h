#ifndef WLIGHTCONTROLLER_H
#define WLIGHTCONTROLLER_H

#include "ui_dlglights.h"

#include <QWidget>
#include <QDomNode>

#include "lights/lightcontroller.h"
#include "widget/qcolorpicker.h"

class SolidColor;

class WLightController : public QWidget, public Ui::DlgLights {
    Q_OBJECT
  public:
    WLightController(QWidget *pParent);
    virtual ~WLightController();

    void setup(QDomNode node);
  private slots:
    void slotSetColor(int hue, int sat);
  private:
    LightController* m_pLightController;
    SolidColor* m_pSolidColor;
};

#endif /* WLIGHTCONTROLLER_H */
