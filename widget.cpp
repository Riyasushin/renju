#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{

    ui->setupUi(this);
    this->setWindowTitle("五子棋v1.12.0");

    board = new Board;

    connect(board, &Board::goBackStart, this, [=](){
        this->show();
        board->hide();
    });


}

Widget::~Widget()
{
    delete ui;
}

void Widget::on_pvp_clicked()
{
    /// Widget::openGameBoard();

    Widget::openGame("");
}

void Widget::receiveStringSlot(const QString &receivedString)
{
    // 在这里对接收到的字符串进行处理
    qDebug() << "接收到的文件名：" << receivedString;
    Widget::openGame(receivedString.toStdString().append(".data"));
}

/// 准备载入残局
/// TODO
void Widget::on_load_clicked()
{



    std::vector<std::string> filelist = fileutils::readFileList(".", ".data");

    if (filelist.empty()) {
        /// 找不到可行的棋局
        /// 提示
        QString dialtitle = "prompt";
        QString strInfo = "没有找到可选择的对局文件，是否新建一个对局";
        QMessageBox::StandardButton result=QMessageBox::question(this, dialtitle, strInfo,
                                                                   QMessageBox::Yes|QMessageBox::No );
        if (result == QMessageBox::Yes) {
            Widget::openGame("");
        } else {
            /// TODO
        }
    } else {
        // 创建并显示新窗口
        fileopenboard = new fileOpenUI(filelist);
        fileopenboard->show();
        connect(fileopenboard, &fileOpenUI::sendStringSignal, this, &Widget::receiveStringSlot);
    }

}

void Widget::openGame(std::string oldfiles){

    if (oldfiles.size() > 0) {
        board->openGameFile(oldfiles);
    } else {
        delete board;
        board = new Board();
        connect(board, &Board::goBackStart, this, [=](){
            this->show();
            board->hide();
        });

    }

    Widget::openGameBoard();

}

void Widget::openGameBoard() {
    this->hide();

    board->show();
}

