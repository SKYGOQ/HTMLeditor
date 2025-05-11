#include "resizehandleitem.h"
#include "layouteditoritem.h"
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsScene>
#include <QBrush>
#include <QCursor>

ResizeHandleItem::ResizeHandleItem(LayoutEditorItem *parentItem)
    : QGraphicsRectItem(0, 0, 5, 5, parentItem), parent(parentItem)
{
    setBrush(Qt::blue);
    setFlags(ItemIsMovable | ItemSendsScenePositionChanges);
    setCursor(Qt::SizeFDiagCursor);
    setZValue(100);  // 显示在最上层
}

void ResizeHandleItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    dragStart = event->scenePos();
    originalSize = parent->boundingRect().size();
    QGraphicsRectItem::mousePressEvent(event);
}

void ResizeHandleItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    QPointF delta = event->scenePos() - dragStart;

    QSizeF newSize = originalSize + QSizeF(delta.x(), delta.y());
    if (newSize.width() < 5 || newSize.height() < 5)
        return;

    parent->resizeTo(newSize);
}
