#include "lights/model/colordelegate.h"

ColorDelegate::ColorDelegate() {
}

ColorDelegate::~ColorDelegate() {

}

void ColorDelegate::paint(QPainter* painter,
                          const QStyleOptionViewItem& option,
                          const QModelIndex& index) const {
    painter->save();

    QVariant data = index.data();
    if (qVariantCanConvert<QColor>(data)) {
        QColor color = data.value<QColor>();
        painter->fillRect(option.rect, color);
    }

    painter->restore();
}
