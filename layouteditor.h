#ifndef LAYOUTEDITOR_H
#define LAYOUTEDITOR_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>

class LayoutEditor : public QGraphicsView
{
    Q_OBJECT
public:
    explicit LayoutEditor(QWidget *parent = nullptr);
    void addImageItem(const QString &filePath);
    void addTextItem(const QString &text);
    QString generateHTML() const;
    QGraphicsScene *getScene() const;
    QPointF currentSnapLineV;
    QPointF currentSnapLineH;
    QGraphicsItem *draggingItem = nullptr;
    QPoint lastMousePos;
    void saveToJson(const QString &filePath);
    void loadFromJson(const QString &filePath);

private:
    QGraphicsScene *scene;
    int snapThreshold = 5;
    QPointF trySnap(QGraphicsItem *movingItem, QPointF newPos);

protected:
    void mouseMoveEvent(QMouseEvent *event) override;
    void drawForeground(QPainter *painter, const QRectF &rect) override;
    void keyPressEvent(QKeyEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;


public slots:
    void groupSelectedItems();
    void ungroupSelectedItems();

private:
    bool showGrid = true;      // 是否显示网格
    bool snapToGrid = false;    // 是否吸附到网格
    int gridSize = 20;         // 网格间距像素

public slots:
    void toggleGrid();         // 切换网格显示
    void toggleSnapToGrid();   // 切换吸附功能

protected:
    void drawBackground(QPainter *painter, const QRectF &rect) override;
};

#endif // LAYOUTEDITOR_H
