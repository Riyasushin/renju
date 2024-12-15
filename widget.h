#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QApplication>
#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QListWidget>
#include <QHBoxLayout>
#include <QDialog>
#include <QMessageBox>
#include "ut-files.h"
#include "fileopenui.h"
#include "board.h"


///// 登录界面和设计界面
///

QT_BEGIN_NAMESPACE
namespace Ui {
class Widget;
}
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:
    void on_pvp_clicked();

    void on_load_clicked();

private:
    Ui::Widget *ui;
    /// 为什么不是Ui::
    Board *board;
    fileOpenUI* fileopenboard;
    void openGame(std::string oldfiles);
    void openGameBoard();

public slots:
    void receiveStringSlot(const QString &receivedString);
};


#endif // WIDGET_H
