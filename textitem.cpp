#include "textitem.h"
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QGraphicsScene>
#include <QStyleOptionGraphicsItem>
#include <QtMath>
#include <QTextCursor>

TextItem::TextItem(QGraphicsItem *parent)
    : QGraphicsTextItem(parent)
{
    // 设置默认行为
    setFlags(ItemIsMovable | ItemIsSelectable | ItemIsFocusable);
    setAcceptHoverEvents(true);
}

TextItem::TextItem(const QString &text, QGraphicsItem *parent)
    : QGraphicsTextItem(text, parent)
{
    setFlags(QGraphicsItem::ItemIsMovable   |
             QGraphicsItem::ItemIsSelectable |
             QGraphicsItem::ItemIsFocusable);
    setTextInteractionFlags(Qt::NoTextInteraction);
}

void TextItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    setTextInteractionFlags(Qt::TextEditorInteraction); // 双击后启用编辑
    setFocus(Qt::MouseFocusReason);                     // 获取焦点
    setFlag(ItemIsMovable, false);
    QGraphicsTextItem::mouseDoubleClickEvent(event);
}

void TextItem::focusOutEvent(QFocusEvent *event)
{
    setTextInteractionFlags(Qt::NoTextInteraction);
    // 离开编辑时关闭编辑状态
    setFlag(ItemIsMovable, true);
    // 清除选区（选中文字的高亮）
    QTextCursor cursor = textCursor();
    cursor.clearSelection();
    setTextCursor(cursor);

    // 清除焦点
    clearFocus();
    QGraphicsTextItem::focusOutEvent(event);
}

QString TextItem::toHtml() const
{
    QPointF pos = scenePos();
    QFont font = this->font();
    QColor color = this->defaultTextColor();

    QString style = QString("position:absolute; left:%1px; top:%2px; "
                            "font-family:'%3'; font-size:%4pt; font-weight:%5; color:%6;")
                        .arg(int(pos.x()))
                        .arg(int(pos.y()))
                        .arg(font.family())
                        .arg(font.pointSize())
                        .arg(font.bold() ? "bold" : "normal")
                        .arg(color.name());  // 输出为 "#RRGGBB"

    return QString("<div style=\"%1\">%2</div>\n")
        .arg(style)
        .arg(this->toPlainText().toHtmlEscaped());
}

QRectF TextItem::boundingRect() const
{
    QRectF rect = QGraphicsTextItem::boundingRect();
    return rect.adjusted(-10, -10, 10, 10);  // 四周扩展
}

QPainterPath TextItem::shape() const
{
    QPainterPath path;
    path.addRect(boundingRect());
    return path;
}

void TextItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QGraphicsTextItem::paint(painter, option, widget);

    if (option->state & QStyle::State_Selected || option->state & QStyle::State_MouseOver) {
        QRectF rect = QGraphicsTextItem::boundingRect().adjusted(-5, -5, 5, 5);
        painter->setPen(QPen(Qt::blue, 1, Qt::DashLine));
        painter->drawRect(rect);
    }
}
void TextItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    QMenu menu;
    QAction *deleteAction = menu.addAction("删除");
    QAction *selectedAction = menu.exec(event->screenPos());
    if (selectedAction == deleteAction) {
        scene()->removeItem(this);
        delete this;
    }
}

void TextItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{

    if (textInteractionFlags() == Qt::TextEditorInteraction) {
        // 让 QGraphicsTextItem 正常处理鼠标事件
        QGraphicsTextItem::mousePressEvent(event);
        return;
    }

    // 单击时只选中，不进入编辑
    clearFocus();                                 // 取消焦点，避免意外编辑
    setTextInteractionFlags(Qt::NoTextInteraction);  // 禁止编辑模式
    QGraphicsTextItem::mousePressEvent(event);    // 继续执行默认的选中逻辑
}
