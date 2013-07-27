#ifndef CONTROLGROUPMODEL_H
#define CONTROLGROUPMODEL_H

#include <QVariant>
#include <QModelIndex>
#include <QAbstractItemModel>
#include <QItemDelegate>
#include <QComboBox>
#include <QStringList>
#include <QStringListModel>

class LightController;

class ControlGroupModel : public QAbstractItemModel {
    Q_OBJECT
  public:
    ControlGroupModel(LightController* pLightController);
    virtual ~ControlGroupModel();

    enum ControlGroupColumns {
        NAME = 0,
        TRIGGER_MODE,
        TRANSITION_MODE,
        CONTROL_MODE,
        COLOR_GENERATOR,
        NUM_LIGHTS,
        NUM_COLUMNS
    };

    virtual int columnCount(const QModelIndex& index) const;
    virtual int rowCount(const QModelIndex& index) const;
    virtual QModelIndex	index(int row, int column, const QModelIndex& parent=QModelIndex()) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role=Qt::DisplayRole) const;
    virtual QVariant data(const QModelIndex& index, int role=Qt::DisplayRole) const;
    virtual QModelIndex parent(const QModelIndex& index) const;
    virtual bool setData(const QModelIndex& index, const QVariant& value, int role=Qt::EditRole);
    virtual Qt::ItemFlags	flags(const QModelIndex& index) const;

  public slots:
    void controlGroupsUpdated();

  private:
    mutable LightController* m_pLightController;
};

#endif /* CONTROLGROUPMODEL_H */
