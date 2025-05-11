#include "carouselitem.h"
#include <QPainter>
#include <QGraphicsSceneMouseEvent>

CarouselItem::CarouselItem(const QStringList &imagePaths, QGraphicsItem *parent)
    : QGraphicsObject(parent), images(imagePaths)
{
    setFlags(ItemIsMovable | ItemIsSelectable | ItemSendsScenePositionChanges);
    setAcceptHoverEvents(true);

    slideTimer = new QTimer(this);
    connect(slideTimer, &QTimer::timeout, this, &CarouselItem::nextSlide);
    slideTimer->start(3000);
}

QRectF CarouselItem::boundingRect() const
{
    return QRectF(0, 0, displaySize.width(), displaySize.height());
}

void CarouselItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    if (images.isEmpty()) return;

    QPixmap pix(images[currentIndex]);
    painter->drawPixmap(boundingRect(), pix, pix.rect());

    if (isSelected()) {
        painter->setPen(QPen(Qt::blue, 2, Qt::DashLine));
        painter->drawRect(boundingRect());
    }
}

void CarouselItem::addImage(const QString &path)
{
    images.append(path);
}

QStringList CarouselItem::imageSources() const
{
    return images;
}

void CarouselItem::nextSlide()
{
    if (images.isEmpty()) return;
    currentIndex = (currentIndex + 1) % images.size();
    update();  // 切换图片后重新绘制
}

void CarouselItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *)
{
    // 双击暂停 / 启动轮播
    if (slideTimer->isActive()) slideTimer->stop();
    else slideTimer->start(3000);
}
