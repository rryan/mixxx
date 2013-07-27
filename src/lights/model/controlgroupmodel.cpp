
#include "lights/model/controlgroupmodel.h"
#include "lights/lightcontroller.h"
#include "lights/controlgroup.h"

#include "lights/triggermode.h"
#include "lights/transitionmode.h"
#include "lights/controlmode.h"
#include "lights/colorgenerator.h"

ControlGroupModel::ControlGroupModel(LightController* pController)
        : m_pLightController(pController) {
}

ControlGroupModel::~ControlGroupModel() {

}

void ControlGroupModel::controlGroupsUpdated() {
    emit(dataChanged(index(0, 0), index(m_pLightController->numControlGroups()-1, NUM_COLUMNS-1)));
}

int ControlGroupModel::rowCount(const QModelIndex& index) const {
    if (index.isValid()) {
        return 0;
    }

    return m_pLightController->numControlGroups();
}

int ControlGroupModel::columnCount(const QModelIndex& index) const {
    if (index.isValid()) {
        return 0;
    }
    return ControlGroupModel::NUM_COLUMNS;
}

QModelIndex ControlGroupModel::index(int row, int column, const QModelIndex& parent) const {
    if (row < 0 || row >= rowCount(parent)) {
        return QModelIndex();
    }
    if (column < 0 || column >= columnCount(parent)) {
        return QModelIndex();
    }
    return createIndex(row, column);
}

QModelIndex ControlGroupModel::parent(const QModelIndex& index) const {
    return QModelIndex();
}


QVariant ControlGroupModel::headerData(int section, Qt::Orientation orientation, int role) const {

    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        switch (section) {
            case ControlGroupModel::NAME:
                return QString(tr("Name"));
            case ControlGroupModel::TRIGGER_MODE:
                return QString(tr("Trigger"));
            case ControlGroupModel::TRANSITION_MODE:
                return QString(tr("Transition"));
            case ControlGroupModel::CONTROL_MODE:
                return QString(tr("Mode"));
            case ControlGroupModel::COLOR_GENERATOR:
                return QString(tr("Color Gen."));
            case ControlGroupModel::NUM_LIGHTS:
                return QString(tr("# Lights"));
            default:
                return QString(tr("Unknown"));
        }
    }

    return QVariant();
}

QVariant ControlGroupModel::data(const QModelIndex& index, int role) const {

    ControlGroup* pGroup = m_pLightController->getControlGroup(index.row());
    ColorGenerator* pGenerator = NULL;

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
            case ControlGroupModel::NAME:
                return pGroup->getName();
            case ControlGroupModel::TRIGGER_MODE:
                return triggerModeToString(pGroup->getTriggerMode());
            case ControlGroupModel::TRANSITION_MODE:
                return transitionModeToString(pGroup->getTransitionMode());
            case ControlGroupModel::CONTROL_MODE:
                return controlModeToString(pGroup->getControlMode());
            case ControlGroupModel::NUM_LIGHTS:
                return pGroup->numLights();
            case ControlGroupModel::COLOR_GENERATOR:
                pGenerator = pGroup->getColorGenerator();
                if (pGenerator != NULL)
                    return pGenerator->getName();
            default:
                return QString(tr("Unknown"));
        }
    }
    return QVariant();
}

bool ControlGroupModel::setData(const QModelIndex& index, const QVariant& value, int role) {
    if (!index.isValid())
        return false;

    if (role == Qt::EditRole) {
        ControlGroup* pGroup = m_pLightController->getControlGroup(index.row());
        ColorGenerator* pGenerator = NULL;
        int generatorIndex;
        switch (index.column()) {
            case ControlGroupModel::TRIGGER_MODE:
                pGroup->setTriggerMode((TriggerMode)value.toInt());
                return true;
            case ControlGroupModel::TRANSITION_MODE:
                pGroup->setTransitionMode((TransitionMode)value.toInt());
                return true;
            case ControlGroupModel::CONTROL_MODE:
                pGroup->setControlMode((ControlMode)value.toInt());
                return true;
            case ControlGroupModel::COLOR_GENERATOR:
                generatorIndex = value.toInt();
                pGenerator = m_pLightController->getColorGenerator(generatorIndex);
                if (pGenerator) {
                    pGroup->setColorGenerator(pGenerator);
                    return true;
                }
            case ControlGroupModel::NAME:
            case ControlGroupModel::NUM_LIGHTS:

            default:
                return false;
        }
    }
    return false;
}

Qt::ItemFlags	ControlGroupModel::flags(const QModelIndex& index) const {
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }

    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    switch (index.column()) {
        case ControlGroupModel::TRIGGER_MODE:
        case ControlGroupModel::TRANSITION_MODE:
        case ControlGroupModel::CONTROL_MODE:
        case ControlGroupModel::COLOR_GENERATOR:
            flags |= Qt::ItemIsEditable;
            break;
        case ControlGroupModel::NAME:
        case ControlGroupModel::NUM_LIGHTS:
        default:
            break;
    }

    return flags;
}
