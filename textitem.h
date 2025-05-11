#ifndef TEXTITEM_H
#define TEXTITEM_H

#include <QGraphicsTextItem>
#include <QStyleOptionGraphicsItem>

class TextItem : public QGraphicsTextItem
{
public:
    TextItem(const QString &text, QGraphicsItem *parent = nullptr);
    TextItem(QGraphicsItem *parent = nullptr);
    QString toHtml() const;
    QRectF boundingRect() const override;
    QPainterPath shape() const override;

protected:
    void focusOutEvent(QFocusEvent *event) override;

protected:
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;


private:
    QPointF dragOffset;

};

#endif // TEXTITEM_H
