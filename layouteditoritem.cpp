#include "layouteditoritem.h"
#include "resizehandleitem.h"
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QGraphicsScene>
#include <QtMath>


LayoutEditorItem::LayoutEditorItem(const QPixmap &pix, const QString &src, QGraphicsItem *parent)
    : QGraphicsPixmapItem(pix, parent), filePath(src),originalPixmap(pix)
{
    setPixmap(pix);
     resizeHandle = new ResizeHandleItem(this);
     resizeHandle->setPos(boundingRect().width() - 5, boundingRect().height() - 5);
}

QString LayoutEditorItem::source() const
{
    return filePath;
}

void LayoutEditorItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    QMenu menu;
    QAction *deleteAction = menu.addAction("删除");
    QAction *selectedAction = menu.exec(event->screenPos());
    if (selectedAction == deleteAction) {
        scene()->removeItem(this);
        delete this;
    }
}

void LayoutEditorItem::resizeTo(const QSizeF &newSize)
{
    if (originalPixmap.isNull())
        return;

    QPixmap scaled = originalPixmap.scaled(newSize.toSize(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    setPixmap(scaled);

    if (resizeHandle)
        resizeHandle->setPos(boundingRect().width() -5, boundingRect().height() - 5);
    if (!resizeHandle)
        return;
}



