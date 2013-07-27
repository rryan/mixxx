#include <QtDebug>

#include "lights/model/comboboxdelegate.h"

ComboBoxDelegate::ComboBoxDelegate(QStringList choices)
        : m_choices(choices) {

}

QWidget* ComboBoxDelegate::createEditor(QWidget *parent,
                                        const QStyleOptionViewItem &option,
                                        const QModelIndex &index) const {
    QComboBox* pCombo = new QComboBox(parent);
    QStringListModel* choices = new QStringListModel(m_choices);
    pCombo->setModel(choices);
    connect(pCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(comboBoxIndexChanged(int)));
    return pCombo;
}

void ComboBoxDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const {
    QComboBox* pCombo = dynamic_cast<QComboBox*>(editor);
    QString value = index.data().toString();
    int comboIndex = pCombo->findText(value);
    if (comboIndex >= 0)
        pCombo->setCurrentIndex(comboIndex);
}

void ComboBoxDelegate::setModelData(QWidget* editor, QAbstractItemModel *model,
                                    const QModelIndex& index) const {
    QComboBox* pCombo = dynamic_cast<QComboBox*>(editor);
    model->setData(index, pCombo->currentIndex(), Qt::EditRole);
}

void ComboBoxDelegate::comboBoxIndexChanged(int index) {
    QComboBox* pCombo = dynamic_cast<QComboBox*>(sender());
    if (pCombo) {
        pCombo->clearFocus();
    }
}
