#ifndef LIGHTMODEL_H
#define LIGHTMODEL_H

#include <QSignalMapper>
#include <QVariant>
#include <QModelIndex>
#include <QAbstractItemModel>
#include <QComboBox>

class LightController;

class LightModel : public QAbstractItemModel {
    Q_OBJECT
  public:
    LightModel(LightController* pLightController);
    virtual ~LightModel();

    enum LightModelColumns {
        NAME = 0,
        STATE,
        COLOR,
        GROUP,
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
    void lightsUpdated();
  private slots:
    void lightUpdated(int number);
  private:
    QSignalMapper m_mapper;
    mutable LightController* m_pLightController;
};



#endif /* LIGHTMODEL_H */
