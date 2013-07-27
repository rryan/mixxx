#ifndef COMBOBOXDELEGATE_H
#define COMBOBOXDELEGATE_H

#include <QStringList>
#include <QStringListModel>
#include <QItemDelegate>
#include <QComboBox>

class ComboBoxDelegate : public QItemDelegate {
    Q_OBJECT
  public:
    ComboBoxDelegate(QStringList choices);

    QWidget *createEditor(QWidget *parent,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const;

    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
  private slots:
    void comboBoxIndexChanged(int index);
  private:
    QStringList m_choices;
};

#endif /* COMBOBOXDELEGATE_H */
