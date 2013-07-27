#ifndef COLORDELEGATE_H
#define COLORDELEGATE_H

#include <QItemDelegate>
#include <QPainter>
#include <QStyleOptionViewItem>
#include <QModelIndex>

class ColorDelegate : public QItemDelegate {
  public:
    ColorDelegate();
    virtual ~ColorDelegate();

    virtual void paint(QPainter* painter,
                       const QStyleOptionViewItem& option,
                       const QModelIndex& index) const;
};


#endif /* COLORDELEGATE_H */
