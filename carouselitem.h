#ifndef CAROUSELITEM_H
#define CAROUSELITEM_H

#include <QGraphicsObject>
#include <QPixmap>
#include <QTimer>
#include <QStringList>

class CarouselItem : public QGraphicsObject
{
    Q_OBJECT

public:
    CarouselItem(const QStringList &imagePaths, QGraphicsItem *parent = nullptr);

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) override;

    void addImage(const QString &path);
    QStringList imageSources() const;

protected:
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;

private slots:
    void nextSlide();

private:
    QStringList images;
    int currentIndex = 0;
    QTimer *slideTimer;
    QSize displaySize = QSize(300, 200);  // 默认尺寸
};

#endif // CAROUSELITEM_H
