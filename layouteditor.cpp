#include "layouteditor.h"
#include "textitem.h"
#include "layouteditoritem.h"
#include <QGraphicsScene>
#include <QPixmap>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QMenu>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QFile>

LayoutEditor::LayoutEditor(QWidget *parent) : QGraphicsView(parent)
{
    scene = new QGraphicsScene(this);
    setScene(scene);
    setRenderHint(QPainter::Antialiasing);
    setDragMode(QGraphicsView::RubberBandDrag);
    scene->setSceneRect(0, 0, 1920, 1080);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);  // 禁止水平滚动条
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);     // 允许垂直滚动条
    currentSnapLineV = QPointF(-1, -1);
    currentSnapLineH = QPointF(-1, -1);
    setDragMode(QGraphicsView::RubberBandDrag);          // 支持框选
    scene->setSelectionArea(QPainterPath());
    scene->setItemIndexMethod(QGraphicsScene::NoIndex);
}

void LayoutEditor::addImageItem(const QString &filePath)
{
    QPixmap pix(filePath);
    auto *item = new LayoutEditorItem(pix, filePath);
    item->setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable);
    scene->addItem(item);
    item->setPos(scene->sceneRect().center());
}

void LayoutEditor::addTextItem(const QString &text)
{
    auto *item = new TextItem(text);
    scene->addItem(item);
    item->setPos(scene->sceneRect().center());
}

QString LayoutEditor::generateHTML() const
{
    QString html = "<!DOCTYPE html>\n<html>\n<head><meta charset=\"utf-8\">\n"
                   "</head>\n<body>\n";

    for (auto *item : scene->items()) {
        if (auto *imgItem = qgraphicsitem_cast<LayoutEditorItem *>(item)) {
            QRectF bounds = imgItem->sceneBoundingRect();
            QString imgTag = QString("<img src=\"%1\" style=\"position:absolute; left:%2px; top:%3px; width:%4px; height:%5px;\">\n")
                                 .arg(imgItem->source())
                                 .arg(int(bounds.left()))
                                 .arg(int(bounds.top()))
                                 .arg(int(bounds.width()))
                                 .arg(int(bounds.height()));
            html += imgTag;
        }
        else if (auto *textItem = qgraphicsitem_cast<TextItem *>(item)) {
            html += textItem->toHtml();  // 样式导出
        }
    }

    html += "</body>\n</html>\n";
    return html;
}

QGraphicsScene *LayoutEditor::getScene() const
{
    return scene;
}

void LayoutEditor::mousePressEvent(QMouseEvent *event)
{
    lastMousePos = event->pos();
    draggingItem = nullptr;
    QPointF scenePos = mapToScene(event->pos());
    QGraphicsItem *item = QGraphicsView::scene()->itemAt(scenePos, transform());

    if (item && item->isSelected())
        draggingItem = item;

    QGraphicsView::mousePressEvent(event);


}

void LayoutEditor::mouseMoveEvent(QMouseEvent *event)
{
    QGraphicsView::mouseMoveEvent(event);

    if (!draggingItem || !(event->buttons() & Qt::LeftButton))
        return;

    if (auto *textItem = dynamic_cast<QGraphicsTextItem *>(draggingItem)) {
        if (textItem->textInteractionFlags() != Qt::NoTextInteraction) {
            return;
        }
    }
    // 鼠标相对移动量
    QPointF delta = mapToScene(event->pos()) - mapToScene(lastMousePos);
    lastMousePos = event->pos();

    // 计算新位置
    QPointF rawNewPos = draggingItem->pos() + delta;
    QPointF snappedPos = trySnap(draggingItem, rawNewPos);
    if (snapToGrid) {
        snappedPos.setX(qRound(snappedPos.x() / gridSize) * gridSize);
        snappedPos.setY(qRound(snappedPos.y() / gridSize) * gridSize);
    }
    draggingItem->setPos(snappedPos);

    viewport()->update();
}



QPointF LayoutEditor::trySnap(QGraphicsItem *movingItem, QPointF newPos)
{
    QRectF movingRect = movingItem->boundingRect().translated(newPos);
    currentSnapLineV = QPointF(-1, -1);
    currentSnapLineH = QPointF(-1, -1);

    for (auto *item : scene->items()) {
        if (item == movingItem) continue;

        QRectF other = item->sceneBoundingRect();

        // 横向吸附
        if (std::abs(movingRect.top() - other.top()) < snapThreshold) {
            newPos.setY(other.top() - movingItem->boundingRect().top());
            currentSnapLineH = QPointF(0, other.top());
        }
        else if (std::abs(movingRect.center().y() - other.center().y()) < snapThreshold) {
            newPos.setY(other.center().y() - movingItem->boundingRect().height() / 2);
            currentSnapLineH = QPointF(0, other.center().y());
        }

        // 纵向吸附
        if (std::abs(movingRect.left() - other.left()) < snapThreshold) {
            newPos.setX(other.left() - movingItem->boundingRect().left());
            currentSnapLineV = QPointF(other.left(), 0);
        }
        else if (std::abs(movingRect.center().x() - other.center().x()) < snapThreshold) {
            newPos.setX(other.center().x() - movingItem->boundingRect().width() / 2);
            currentSnapLineV = QPointF(other.center().x(), 0);
        }
    }

    return newPos;
}

void LayoutEditor::drawForeground(QPainter *painter, const QRectF &rect)
{
    QGraphicsView::drawForeground(painter, rect);

    painter->setPen(QPen(Qt::red, 1, Qt::DashLine));

    if (currentSnapLineV.x() >= 0)
        painter->drawLine(QLineF(currentSnapLineV.x(), 0, currentSnapLineV.x(), height()));
    if (currentSnapLineH.y() >= 0)
        painter->drawLine(QLineF(0, currentSnapLineH.y(), width(), currentSnapLineH.y()));
}

void LayoutEditor::keyPressEvent(QKeyEvent *event)
{
    if (QGraphicsTextItem *textItem = dynamic_cast<QGraphicsTextItem *>(scene->focusItem())) {
        QGraphicsView::keyPressEvent(event);  // 把事件交给文本项处理（编辑文字）
        return;
    }
    if (event->key() == Qt::Key_Backspace) {
        QList<QGraphicsItem*> selected = scene->selectedItems();
        for (QGraphicsItem *item : selected) {
            if (auto *group = dynamic_cast<QGraphicsItemGroup *>(item)) {
                scene->destroyItemGroup(group);
            } else {
                scene->removeItem(item);
                delete item;
            }
        }
    }
    // 判断移动步长
    int delta = event->modifiers() & Qt::ShiftModifier ? 10 : 1;

    // 方向键映射位移向量
    QPointF offset;
    switch (event->key()) {
    case Qt::Key_Up:    offset = QPointF(0, -delta); break;
    case Qt::Key_Down:  offset = QPointF(0, delta);  break;
    case Qt::Key_Left:  offset = QPointF(-delta, 0); break;
    case Qt::Key_Right: offset = QPointF(delta, 0);  break;
    default:
        QGraphicsView::keyPressEvent(event);  // 非方向键，交给基类处理
        return;
    }

    // 移动所有选中的图形项
    for (auto *item : scene->selectedItems()) {
        item->moveBy(offset.x(), offset.y());
    }

    viewport()->update();  // 更新画布显示


}

void LayoutEditor::mouseReleaseEvent(QMouseEvent *event)
{
    QPointF scenePos = mapToScene(event->pos());
    QGraphicsItem *item = QGraphicsView::scene()->itemAt(scenePos, transform());

    if (item && item->isSelected())
        draggingItem = item;
    else
        draggingItem = nullptr;

    // 清除对齐线状态
    currentSnapLineH = QPointF(-1, -1);
    currentSnapLineV = QPointF(-1, -1);
    QGraphicsView::mouseReleaseEvent(event);
    viewport()->update();  // 强制重绘以移除红线
}

void LayoutEditor::contextMenuEvent(QContextMenuEvent *event)
{
    QPointF scenePos = mapToScene(event->pos());
    QGraphicsItem *clickedItem = QGraphicsView::scene()->itemAt(scenePos, transform());

    if (!clickedItem) return;

    QMenu menu;
    QAction *groupAction = menu.addAction("组合");
    QAction *ungroupAction = menu.addAction("取消组合");
    QAction *deleteAction = menu.addAction("删除");

    QAction *selected = menu.exec(event->globalPos());
    if (selected == groupAction)
        groupSelectedItems();
    else if (selected == ungroupAction)
        ungroupSelectedItems();
    else if (selected == deleteAction) {
        QList<QGraphicsItem *> selectedItems = scene->selectedItems();
        for (QGraphicsItem *item : selectedItems) {
            if (auto *group = dynamic_cast<QGraphicsItemGroup *>(item)) {
                scene->destroyItemGroup(group);
            } else {
                scene->removeItem(item);
                delete item;
            }
        }
    }
}

void LayoutEditor::groupSelectedItems()
{
    QList<QGraphicsItem *> itemsToGroup = scene->selectedItems();

    if (itemsToGroup.count() < 2) return;

    QGraphicsItemGroup *group = scene->createItemGroup(itemsToGroup);
    group->setFlags(QGraphicsItem::ItemIsMovable |
                    QGraphicsItem::ItemIsSelectable |
                    QGraphicsItem::ItemIsFocusable);
    group->setZValue(100);  // 保证不被遮挡
}

void LayoutEditor::ungroupSelectedItems()
{
    QList<QGraphicsItem *> selected = scene->selectedItems();

    for (QGraphicsItem *item : selected) {
        if (auto *group = dynamic_cast<QGraphicsItemGroup *>(item)) {
            scene->destroyItemGroup(group);
        }
    }
}

void LayoutEditor::drawBackground(QPainter *painter, const QRectF &rect)
{
    if (!showGrid) return;

    QPen pen(QColor(230, 230, 230));
    pen.setWidth(1);
    painter->setPen(pen);

    int left = static_cast<int>(rect.left());
    int right = static_cast<int>(rect.right());
    int top = static_cast<int>(rect.top());
    int bottom = static_cast<int>(rect.bottom());

    for (int x = left - (left % gridSize); x <= right; x += gridSize)
        painter->drawLine(x, top, x, bottom);
    for (int y = top - (top % gridSize); y <= bottom; y += gridSize)
        painter->drawLine(left, y, right, y);
}

void LayoutEditor::toggleGrid()
{
    showGrid = !showGrid;
    viewport()->update();  // 触发重新绘制
}

void LayoutEditor::toggleSnapToGrid()
{
    snapToGrid = !snapToGrid;
}

void LayoutEditor::saveToJson(const QString &filePath)
{
    QJsonArray itemArray;

    for (QGraphicsItem *item : scene->items()) {
        QJsonObject obj;

        if (auto *img = dynamic_cast<LayoutEditorItem *>(item)) {
            obj["type"] = "image";
            obj["x"] = img->pos().x();
            obj["y"] = img->pos().y();
            obj["source"] = img->source();
            obj["width"] = img->boundingRect().width();
            obj["height"] = img->boundingRect().height();
        } else if (auto *text = dynamic_cast<TextItem *>(item)) {
            obj["type"] = "text";
            obj["x"] = text->pos().x();
            obj["y"] = text->pos().y();
            obj["text"] = text->toPlainText();

            QFont font = text->font();
            obj["fontSize"] = font.pointSize();
            obj["fontBold"] = font.bold();
            obj["fontFamily"] = font.family();
        }


        itemArray.append(obj);
    }

    QJsonObject root;
    root["items"] = itemArray;

    QJsonDocument doc(root);
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        file.write(doc.toJson(QJsonDocument::Indented));
        file.close();
    }
}

void LayoutEditor::loadFromJson(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    scene->clear();  // 清除旧的元素

    QJsonArray items = doc.object()["items"].toArray();
    for (const QJsonValue &val : items) {
        QJsonObject obj = val.toObject();
        QString type = obj["type"].toString();

        if (type == "image") {
            QString src = obj["source"].toString();
            QPixmap pix(src);
            if (!pix.isNull()) {
                auto *img = new LayoutEditorItem(pix, src);
                img->setFlags(QGraphicsItem::ItemIsSelectable);
                scene->addItem(img);
                img->setPos(obj["x"].toDouble(), obj["y"].toDouble());
                // 可加缩放
            }
        } else if (type == "text") {
            auto *txt = new TextItem();
            txt->setPlainText(obj["text"].toString());
            QFont font;
            font.setPointSize(obj["fontSize"].toInt());
            font.setBold(obj["fontBold"].toBool());
            font.setFamily(obj["fontFamily"].toString());
            txt->setFont(font);
            txt->setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsFocusable);
            scene->addItem(txt);
            txt->setPos(obj["x"].toDouble(), obj["y"].toDouble());
        }
    }
}
