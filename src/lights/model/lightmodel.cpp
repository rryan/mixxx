#include <QtDebug>

#include "lights/model/lightmodel.h"
#include "lights/lightcontroller.h"
#include "lights/controlgroup.h"

#include "lights/triggermode.h"
#include "lights/controlmode.h"

LightModel::LightModel(LightController* pController)
        : m_pLightController(pController) {

    // This method is too spammy and causes the table to be updated O(n) times
    // per frame where n is lights.

    // for (int i = 0; i < m_pLightController->numLights(); ++i) {
    //     Light* pLight = m_pLightController->getLight(i);
    //     m_mapper.setMapping(pLight, i);
    //     connect(pLight, SIGNAL(updated()), &m_mapper, SLOT(map()));
    // }
    // connect(&m_mapper, SIGNAL(mapped(int)), this, SLOT(lightUpdated(int)));
}

LightModel::~LightModel() {

}

void LightModel::lightsUpdated() {
    static int counter = 0;
    counter++;

    // Update at 30hz, 5ms latency means 200
    if (counter > 10) {
        emit(dataChanged(index(0, 0), index(m_pLightController->numLights()-1, NUM_COLUMNS-1)));
        counter = 0;
    }
}

void LightModel::lightUpdated(int lightNumber) {
    emit(dataChanged(index(lightNumber, 0), index(lightNumber, NUM_COLUMNS-1)));
}

int LightModel::rowCount(const QModelIndex& index) const {
    if (index.isValid()) {
        return 0;
    }

    return m_pLightController->numLights();
}

int LightModel::columnCount(const QModelIndex& index) const {
    if (index.isValid()) {
        return 0;
    }
    return LightModel::NUM_COLUMNS;
}

QModelIndex LightModel::index(int row, int column, const QModelIndex& parent) const {
    if (row < 0 || row >= rowCount(parent)) {
        return QModelIndex();
    }
    if (column < 0 || column >= columnCount(parent)) {
        return QModelIndex();
    }
    return createIndex(row, column);
}

QModelIndex LightModel::parent(const QModelIndex& index) const {
    return QModelIndex();
}


QVariant LightModel::headerData(int section, Qt::Orientation orientation, int role) const {

    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        switch (section) {
            case LightModel::NAME:
                return tr("Name");
            case LightModel::STATE:
                return tr("State");
            case LightModel::COLOR:
                return tr("Color");
            case LightModel::GROUP:
                return tr("Group");
            default:
                return tr("Unknown");
        }
    }

    return QVariant();
}

QVariant LightModel::data(const QModelIndex& index, int role) const {
    Light* pLight = m_pLightController->getLight(index.row());
    ControlGroup* pControlGroup = NULL;

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
            case LightModel::NAME:
                return pLight->getName();
            case LightModel::STATE:
                return lightStateToString(pLight->getState());
            case LightModel::COLOR:
                return pLight->getColor();
            case LightModel::GROUP:
                pControlGroup = pLight->getControlGroup();
                if (pControlGroup) {
                    return pControlGroup->getName();
                }
            default:
                return tr("Unknown");
        }
    }

    return QVariant();
}

bool LightModel::setData(const QModelIndex& index, const QVariant& value, int role) {
    if (!index.isValid())
        return false;

    if (role == Qt::EditRole) {
        Light* pLight = m_pLightController->getLight(index.row());
        switch (index.column()) {
            case LightModel::COLOR:
                if (qVariantCanConvert<QColor>(value)) {
                    pLight->setColor(value.value<QColor>());
                    return true;
                }
            case LightModel::STATE:
            case LightModel::NAME:
            case LightModel::GROUP: // Not supported ATM
            default:
                break;
        }
    }
    return false;
}

Qt::ItemFlags	LightModel::flags(const QModelIndex& index) const {
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }

    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    switch (index.column()) {
        case LightModel::COLOR:
            flags |= Qt::ItemIsEditable;;
            break;
        case LightModel::NAME:
        case LightModel::STATE:
        case LightModel::GROUP:
        default:
            break;
    }

    return flags;
}
