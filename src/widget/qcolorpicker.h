#ifndef QCOLORPICKER_H
#define QCOLORPICKER_H

#include <QObject>
#include <QFrame>
#include <QPoint>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPixmap>


class QColorPicker : public QFrame
{
    Q_OBJECT
public:
    QColorPicker(QWidget* parent);
    ~QColorPicker();

public slots:
    void setCol(int h, int s);

signals:
    void newCol(int h, int s);

protected:
    QSize sizeHint() const;
    void paintEvent(QPaintEvent*);
    void mouseMoveEvent(QMouseEvent *);
    void mousePressEvent(QMouseEvent *);

private:
    int hue;
    int sat;

    QPoint colPt();
    int huePt(const QPoint &pt);
    int satPt(const QPoint &pt);
    void setCol(const QPoint &pt);

    QPixmap *pix;
};


#endif /* QCOLORPICKER_H */
