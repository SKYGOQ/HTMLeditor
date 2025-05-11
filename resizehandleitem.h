#ifndef RESIZEHANDLEITEM_H
#define RESIZEHANDLEITEM_H

#include <QGraphicsRectItem>

class LayoutEditorItem;

class ResizeHandleItem : public QGraphicsRectItem
{
public:
    ResizeHandleItem(LayoutEditorItem *parentItem);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;

private:
    QPointF dragStart;
    QSizeF originalSize;
    LayoutEditorItem *parent;
};

#endif // RESIZEHANDLEITEM_H
