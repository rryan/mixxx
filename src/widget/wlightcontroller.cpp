#include <QtDebug>
#include <QPalette>

#include "defs.h"

#include "widget/wlightcontroller.h"
#include "widget/wwidget.h"
#include "widget/wskincolor.h"

#include "lights/solidcolor.h"
#include "lights/model/controlgroupmodel.h"
#include "lights/model/lightmodel.h"
#include "lights/model/comboboxdelegate.h"
#include "lights/model/colordelegate.h"
#include "lights/triggermode.h"
#include "lights/transitionmode.h"
#include "lights/controlmode.h"
#include "lights/controlgroup.h"


WLightController::WLightController(QWidget* pParent) : QWidget(pParent) {
    // TODO(XXX) kill this someday
    m_pLightController = LightController::sInstance;

    // We should have been setup after the engine
    Q_ASSERT(m_pLightController);

    setupUi(this);

    connect(m_pColorPicker, SIGNAL(newCol(int, int)), this, SLOT(slotSetColor(int, int)));

    m_pSolidColor = new SolidColor("ColorPicker", Qt::black);
    m_pLightController->addColorGenerator(m_pSolidColor);

    // Setup the controlgroup table.
    ControlGroupModel* pControlGroupModel = new ControlGroupModel(m_pLightController);
    connect(m_pLightController, SIGNAL(stateUpdated()),
            pControlGroupModel, SLOT(controlGroupsUpdated()));

    m_controlGroupTable->setModel(pControlGroupModel);
    QStringList controlModeOptions = getControlModeOptions();
    QStringList triggerModeOptions = getTriggerModeOptions();
    QStringList transitionModeOptions = getTransitionModeOptions();
    m_controlGroupTable->setItemDelegateForColumn(ControlGroupModel::TRIGGER_MODE,
                                                  new ComboBoxDelegate(triggerModeOptions));
    m_controlGroupTable->setItemDelegateForColumn(ControlGroupModel::TRANSITION_MODE,
                                                  new ComboBoxDelegate(transitionModeOptions));
    m_controlGroupTable->setItemDelegateForColumn(ControlGroupModel::CONTROL_MODE,
                                                  new ComboBoxDelegate(controlModeOptions));

    QStringList colorGeneratorOptions;
    for (int i = 0; i < m_pLightController->numColorGenerators(); ++i) {
        ColorGenerator* pColorGenerator = m_pLightController->getColorGenerator(i);
        colorGeneratorOptions << pColorGenerator->getName();
    }
    m_controlGroupTable->setItemDelegateForColumn(ControlGroupModel::COLOR_GENERATOR,
                                                  new ComboBoxDelegate(colorGeneratorOptions));


    // Setup the light table
    LightModel* pLightModel = new LightModel(m_pLightController);
    m_lightsTable->setModel(pLightModel);
    connect(m_pLightController, SIGNAL(stateUpdated()),
            pLightModel, SLOT(lightsUpdated()));
    m_lightsTable->setItemDelegateForColumn(LightModel::COLOR,
                                            new ColorDelegate());

}

WLightController::~WLightController() {
}

void WLightController::slotSetColor(int hue, int value) {
    QColor color = QColor::fromHsv(hue, 255, value);
    m_pSolidColor->setColor(color);

    QModelIndexList selectedGroups = m_controlGroupTable->selectionModel()->selectedIndexes();

    if (selectedGroups.size() == 0) {
        // Do to all? Do to none?
    } else {
        foreach (QModelIndex index, selectedGroups) {
            int groupRow = index.row();
            ControlGroup* pGroup = m_pLightController->getControlGroup(groupRow);
            pGroup->setColorGenerator(m_pSolidColor);
            if (pGroup->getControlMode() == CONTROL_OFF) {
                pGroup->setControlMode(CONTROL_CYCLE);
            }
        }
    }
}


void WLightController::setup(QDomNode node) {
    QPalette pal = palette();

    // Row colors
    if (!WWidget::selectNode(node, "BgColorRowEven").isNull() &&
        !WWidget::selectNode(node, "BgColorRowUneven").isNull()) {
        QColor r1;
        r1.setNamedColor(WWidget::selectNodeQString(node, "BgColorRowEven"));
        r1 = WSkinColor::getCorrectColor(r1);
        QColor r2;
        r2.setNamedColor(WWidget::selectNodeQString(node, "BgColorRowUneven"));
        r2 = WSkinColor::getCorrectColor(r2);

        // For now make text the inverse of the background so it's readable In
        // the future this should be configurable from the skin with this as the
        // fallback option
        QColor text(255 - r1.red(), 255 - r1.green(), 255 - r1.blue());

        pal.setColor(QPalette::Base, r1);
        pal.setColor(QPalette::AlternateBase, r2);
        pal.setColor(QPalette::Text, text);
    }

    setPalette(pal);
}
