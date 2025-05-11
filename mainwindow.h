#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QToolBar>
#include <QFontComboBox>
#include <QSpinBox>
#include <QColorDialog>
#include <QToolButton>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_actionInsertImage_triggered();
    void on_actionExportHTML_triggered();


private:
    QToolBar *styleToolbar;
    QFontComboBox *fontCombo;
    QSpinBox *fontSizeBox;
    QToolButton *boldButton;
    QToolButton *colorButton;
    QSpinBox *posXBox;
    QSpinBox *posYBox;



};

#endif // MAINWINDOW_H
