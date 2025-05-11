#include "mainwindow.h"
#include "layouteditor.h"
#include "textitem.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QGraphicsScene>
#include <QLabel>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // 创建中心编辑区域
    auto *editor = new LayoutEditor(this);
    setCentralWidget(editor);

    // 创建菜单栏
    QMenuBar *menuBar = new QMenuBar(this);
    setMenuBar(menuBar);  // 设置菜单栏到窗口

    // 创建菜单项
    QMenu *fileMenu = menuBar->addMenu("文件");
    QMenu *insertMenu = menuBar->addMenu("插入");
    QMenu *viewMenu = menuBar->addMenu("视图");

    // 创建菜单动作
    QAction *insertImageAction = new QAction("插入图片", this);
    QAction *exportHtmlAction = new QAction("导出HTML", this);
    QAction *insertTextAction = new QAction("插入文本", this);

    // 添加动作到菜单
    insertMenu->addAction(insertImageAction);
    fileMenu->addAction(exportHtmlAction);
    insertMenu->addAction(insertTextAction);

    // 连接动作到槽函数
    connect(insertImageAction, &QAction::triggered, this, &MainWindow::on_actionInsertImage_triggered);
    connect(exportHtmlAction, &QAction::triggered, this, &MainWindow::on_actionExportHTML_triggered);
    connect(insertTextAction, &QAction::triggered, this, [this]() {
        auto *editor = qobject_cast<LayoutEditor *>(centralWidget());
        if (editor)
            editor->addTextItem("双击进行编辑");
    });

    // 1. 创建工具栏
    styleToolbar = addToolBar("样式");
    styleToolbar->setMovable(false);

    // 2. 字体选择器
    fontCombo = new QFontComboBox(this);
    styleToolbar->addWidget(fontCombo);

    // 3. 字号选择
    fontSizeBox = new QSpinBox(this);
    fontSizeBox->setRange(6, 72);
    fontSizeBox->setValue(14);
    styleToolbar->addWidget(fontSizeBox);

    // 4. 加粗按钮
    boldButton = new QToolButton(this);
    boldButton->setText("B");
    boldButton->setCheckable(true);
    styleToolbar->addWidget(boldButton);

    // 5. 字体颜色选择
    colorButton = new QToolButton(this);
    colorButton->setText("color");
    styleToolbar->addWidget(colorButton);

    styleToolbar->addSeparator();

    styleToolbar->addWidget(new QLabel("X:"));
    posXBox = new QSpinBox(this);
    posXBox->setRange(0, 5000);
    styleToolbar->addWidget(posXBox);

    styleToolbar->addWidget(new QLabel("Y:"));
    posYBox = new QSpinBox(this);
    posYBox->setRange(0, 5000);
    styleToolbar->addWidget(posYBox);

    connect(fontSizeBox, QOverload<int>::of(&QSpinBox::valueChanged), this, [=](int size) {
        auto items = editor->getScene()->selectedItems();
        if (!items.isEmpty()) {
            auto *textItem = qgraphicsitem_cast<TextItem *>(items.first());
            if (textItem) {
                QFont font = textItem->font();
                font.setPointSize(size);
                textItem->setFont(font);
            }
        }
    });

    connect(fontCombo, &QFontComboBox::currentFontChanged, this, [=](const QFont &f) {
        auto items = editor->getScene()->selectedItems();
        if (!items.isEmpty()) {
            auto *textItem = qgraphicsitem_cast<TextItem *>(items.first());
            if (textItem) {
                QFont font = textItem->font();
                font.setFamily(f.family());
                textItem->setFont(font);
            }
        }
    });

    connect(boldButton, &QToolButton::toggled, this, [=](bool checked) {
        auto items = editor->getScene()->selectedItems();
        if (!items.isEmpty()) {
            auto *textItem = qgraphicsitem_cast<TextItem *>(items.first());
            if (textItem) {
                QFont font = textItem->font();
                font.setBold(checked);
                textItem->setFont(font);
            }
        }
    });

    connect(colorButton, &QToolButton::clicked, this, [=]() {
        auto *textItem = qgraphicsitem_cast<TextItem *>(editor->getScene()->focusItem());
        if (textItem) {
            QColor color = QColorDialog::getColor(textItem->defaultTextColor(), this, "Choose Font Color");
            if (color.isValid())
                textItem->setDefaultTextColor(color);
        }
    });

    // 当选中项改变时更新样式面板
    connect(editor->getScene(), &QGraphicsScene::selectionChanged, this, [=]() {
        auto items = editor->getScene()->selectedItems();
        if (!items.isEmpty()) {
            auto *textItem = qgraphicsitem_cast<TextItem *>(items.first());
            if (textItem) {
                QFont font = textItem->font();
                fontCombo->setCurrentFont(font);
                fontSizeBox->setValue(font.pointSize());
                boldButton->setChecked(font.bold());
            }
        }
    });

    connect(editor->getScene(), &QGraphicsScene::selectionChanged, this, [=]() {
        auto items = editor->getScene()->selectedItems();
        if (!items.isEmpty()) {
            QGraphicsItem *item = items.first();
            QPointF pos = item->pos();
            posXBox->blockSignals(true);
            posYBox->blockSignals(true);
            posXBox->setValue(int(pos.x()));
            posYBox->setValue(int(pos.y()));
            posXBox->blockSignals(false);
            posYBox->blockSignals(false);
        }
    });

    connect(posXBox, QOverload<int>::of(&QSpinBox::valueChanged), this, [=](int x) {
        auto items = editor->getScene()->selectedItems();
        if (!items.isEmpty()) {
            QGraphicsItem *item = items.first();
            item->setPos(x, posYBox->value());
        }
    });

    connect(posYBox, QOverload<int>::of(&QSpinBox::valueChanged), this, [=](int y) {
        auto items = editor->getScene()->selectedItems();
        if (!items.isEmpty()) {
            QGraphicsItem *item = items.first();
            item->setPos(posXBox->value(), y);
        }
    });

    QAction *toggleGridAction = new QAction("网格线显示开关", this);
    toggleGridAction->setCheckable(true);
    toggleGridAction->setChecked(true);
    viewMenu->addAction(toggleGridAction);
    connect(toggleGridAction, &QAction::triggered, editor, &LayoutEditor::toggleGrid);


    QAction *toggleSnapAction = new QAction("网格线吸附开关", this);
    toggleSnapAction->setCheckable(true);
    toggleSnapAction->setChecked(false);
    viewMenu->addAction(toggleSnapAction);
    connect(toggleSnapAction, &QAction::triggered, editor, &LayoutEditor::toggleSnapToGrid);

    QAction *saveJsonAction = new QAction("保存为JSON文件", this);
    fileMenu->addAction(saveJsonAction);

    // 3. 连接信号槽
    connect(saveJsonAction, &QAction::triggered, this, [=]() {
        QString path = QFileDialog::getSaveFileName(this, "Save Layout", "", "JSON Files (*.json)");
        if (!path.isEmpty()) {
            editor->saveToJson(path);  // 假设你前面已经实现了 LayoutEditor::saveToJson
        }
    });

    QAction *loadJsonAction = new QAction("导入JSON文件", this);
    fileMenu->addAction(loadJsonAction);

    connect(loadJsonAction, &QAction::triggered, this, [=]() {
        QString path = QFileDialog::getOpenFileName(this, "Load Layout", "", "JSON Files (*.json)");
        if (!path.isEmpty()) {
            editor->loadFromJson(path);  // 你稍后会实现这个函数
        }
    });



    resize(1920, 1080);


}

MainWindow::~MainWindow()
{
}

void MainWindow::on_actionInsertImage_triggered()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Select an image", "", "Images (*.png *.jpg *.bmp)");
    if (!filePath.isEmpty()) {
        auto *editor = qobject_cast<LayoutEditor*>(centralWidget());
        if (editor)
            editor->addImageItem(filePath);
    }
}

void MainWindow::on_actionExportHTML_triggered()
{
    QString filePath = QFileDialog::getSaveFileName(this, "Export HTML", "", "HTML Files (*.html)");
    if (!filePath.isEmpty()) {
        auto *editor = qobject_cast<LayoutEditor*>(centralWidget());
        if (editor) {
            QString html = editor->generateHTML();
            QFile file(filePath);
            if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QTextStream out(&file);
                out << html;
                file.close();
                QMessageBox::information(this, "Export", "HTML exported successfully.");
            }
        }
    }
}
