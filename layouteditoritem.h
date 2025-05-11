#ifndef LAYOUTEDITORITEM_H
#define LAYOUTEDITORITEM_H

#include <QGraphicsPixmapItem>
#include <QPixmap>
#include "resizehandleitem.h"

class LayoutEditorItem : public QGraphicsPixmapItem
{
public:
    LayoutEditorItem(const QPixmap &pix, const QString &src, QGraphicsItem *parent = nullptr);
    QString source() const;
    void resizeTo(const QSizeF &newSize);  //缩放图片函数

private:
    QString filePath; // 保存图片路径
    ResizeHandleItem *resizeHandle;
    QPixmap originalPixmap;
    QPointF dragOffset; // 鼠标点击时相对于左上角的偏移

protected:
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;


};

#endif // LAYOUTEDITORITEM_H
