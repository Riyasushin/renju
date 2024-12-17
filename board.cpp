#include "board.h"
#include "ui_board.h"
#include "game.h"


Board::Board(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Board)
{
    ui->setupUi(this);
    this->setWindowTitle("五子棋");

    Board::nextMove = {-1, -1, game::Chessquer::non};

    /// START THE GAME
    Game = new game;

    //game->MakeMove(true, 0, 0);
    //game->MakeMove(false, 0, 1);

}

Board::~Board()
{
    delete ui;
}

/// 绘制棋盘
void Board::paintEvent(QPaintEvent *event){



    painter = new QPainter;
    painter->begin(this);

    /// draw lines
    for (int i = 0; i <= LINE; ++i) {
        painter->drawLine(x, y + WIDTH * i, x + WIDTH * LINE, y + WIDTH * i);
    }
    for (int i = 0; i <= LINE; ++i) {
        painter->drawLine(x + WIDTH * i, y, x + WIDTH * i, y + WIDTH * LINE);
    }

    /// draw chequers
    const std::vector<game::Move> s = Game->getGameData();

    for (auto elem : s) {
        int x1 = elem.x, y1 = elem.y;

         //// TODO 把画棋子封装成小函数
        if (elem.color == game::white) { /// BAO LOU JIE KOU LE
            painter->setBrush(QBrush(Qt::white, Qt::SolidPattern));
            painter->drawEllipse(x + WIDTH * x1 - R, y + WIDTH * y1 - R, R * 2, R * 2);

        } else {
            painter->setBrush(QBrush(Qt::black, Qt::SolidPattern));
            painter->drawEllipse(x + WIDTH * x1 - R, y + WIDTH * y1 - R, R * 2, R * 2);
        }
    }

    /// TODO 画虚拟的棋子
    if (Board::nextMove.x != -1) {
        switch(Board::nextMove.type) {
        case game::Chessquer::black:
            painter->setBrush(QBrush(Qt::black, Qt::SolidPattern));
            break;
        case game::Chessquer::white:
            painter->setBrush(QBrush(Qt::white, Qt::SolidPattern));
            break;
        case game::Chessquer::invalid:
            painter->setBrush(QBrush(Qt::red, Qt::SolidPattern));
            break;
        }
        painter->drawEllipse(x + WIDTH * Board::nextMove.x - R / 2, y + WIDTH * Board::nextMove.y - R / 2, R, R);
    }

    painter->end();
}


/// 判断鼠标移动，预先画
/// TODO
void Board::mouseMoveEvent(QMouseEvent *event) {

    if (Game->hasDecideWinner()) {
        Board::nextMove.x = -1;
        Board::nextMove.y = -1;
        return;
    }

    int mouseX = event->x(), mouseY = event->y();

    if (mouseX >= MARGEIN / 2 && mouseX <= MARGEIN + WIDTH * LINE + MARGEIN / 2
        && mouseY >= MARGEIN / 2 && mouseY <= MARGEIN + WIDTH * LINE + MARGEIN / 2) {

        qDebug() << mouseX << " " << mouseY;
        auto [absX, absY] = Board::getPointAbsLocation(mouseX, mouseY);

        bool canMove = Game->canMakeMove(absX, absY);

        /// TODO bug!!!
        if (canMove) {
            /// 能下棋，写成可行的
            /// 已经排除了棋局结束的情况，所有只有白瞎或黑瞎的情况
            Board::nextMove.type = (Game->getGameState() == game::gameState::whiteToD ? game::Chessquer::white : game::Chessquer::black);

        } else {
            /// 不能下棋,写成红色的叉号
            Board::nextMove.type = game::Chessquer::invalid;
        }

        Board::nextMove.x = absX;
        Board::nextMove.y = absY;

        update();

    }


}

/// 鼠标释放，画实心的
void Board::mouseReleaseEvent(QMouseEvent *event) {

    if (Game->hasDecideWinner())
        return;


    /// 已经被弃用 怎么改 TODO
    int mouseX = event->x(), mouseY = event->y();
    /// around the board ,in play
    if (mouseX >= MARGEIN / 2 && mouseX <= MARGEIN + WIDTH * LINE + MARGEIN / 2
        && mouseY >= MARGEIN / 2 && mouseY <= MARGEIN + WIDTH * LINE + MARGEIN / 2) {

        qDebug() << mouseX << " " << mouseY;
        auto [absX, absY] = Board::getPointAbsLocation(mouseX, mouseY);

        bool canMove = Game->canMakeMove(absX, absY);
        if (canMove) {

            Board::nextMove = {-1, -1, game::Chessquer::non};
            Game->MakeMoveHelper(absX, absY);
            Board::NotSave();

            /// 文本修改，提示轮到谁下
            QString stateT;
            switch (Game->getGameState()) {
            case game::gameState::blackToD:
                stateT = (Board::isPVE ? (Board::AIMode == Board::Mode::ComputerBlack ? "人机下" : "该你了")  : "轮到黑子");
                break;
            case game::gameState::whiteToD:
                stateT = (Board::isPVE ? (Board::AIMode == Board::Mode::ComputerWhite? "人机下" : "该你了")  : "轮到白子");
                break;
              case game::gameState::whiteWin:
                stateT = (Board::isPVE ? (Board::AIMode == Board::Mode::ComputerWhite ? "人机赢了" : "你赢了") :"白子胜利");
                break;
               case game::gameState::blackWin:
                stateT = (Board::isPVE ? (Board::AIMode == Board::Mode::ComputerBlack ? "人机赢了" : "你赢了") :"黑子胜利");
                break;
           default:
                stateT = "出BUG了！";
                break;
            }
            Board::ui->label->setText(stateT);

            update();

            if (Game->hasDecideWinner()) {
                //// todo 有BUG
                QString winnertr;
                QString title = "CONGRATULATION!!!";
                game::Chessquer winner = Game->Winner();
                if (winner == game::Chessquer::white) {
                    if (Board::isPVE) {
                        /// TODO
                        winnertr = (Board::AIMode == Board::Mode::ComputerWhite ? "你赢了" : "你输了");
                    } else {
                        winnertr = "白子胜";
                    }
                } else if (winner == game::Chessquer::black){
                    if (Board::isPVE) {
                        winnertr = (Board::AIMode == Board::Mode::ComputerBlack? "你赢了" : "你输了");
                    } else {
                        winnertr = "黑子胜";
                    }
                }
                QMessageBox::information(this, title, winnertr, QMessageBox::Ok, QMessageBox::NoButton);
            } else {
                /// 还可以继续挣扎

                if (Board::isPVE) {
                    /// 装模作样暂停一秒 TODO


                    ChessEngine::nextStep(absX, absY);

                    ChessEngine::Position p = ChessEngine::getLastPosition();

                    /// TODO 复制粘贴不好
                    Game->MakeMoveHelper(p.x, p.y);

                    if (Game->hasDecideWinner()) {
                        //// todo 有BUG
                        QString winnertr;
                        QString title = "CONGRATULATION!!!";
                        game::Chessquer winner = Game->Winner();
                        if (winner == game::Chessquer::white) {
                            if (Board::isPVE) {
                                /// TODO
                            } else {
                                winnertr = "白子胜";
                            }
                        } else if (winner == game::Chessquer::black){
                            if (Board::isPVE) {
                                /// TODO
                            } else {
                                winnertr = "黑子胜";
                            }
                        }
                        QMessageBox::information(this, title, winnertr, QMessageBox::Ok, QMessageBox::NoButton);
                    }

                    switch (Game->getGameState()) {
                    case game::gameState::blackToD:
                        stateT = (Board::isPVE ? (Board::AIMode == Board::Mode::ComputerBlack ? "人机下" : "该你了")  : "轮到黑子");
                        break;
                    case game::gameState::whiteToD:
                        stateT = (Board::isPVE ? (Board::AIMode == Board::Mode::ComputerWhite? "人机下" : "该你了")  : "轮到白子");
                        break;
                    case game::gameState::whiteWin:
                        stateT = (Board::isPVE ? (Board::AIMode == Board::Mode::ComputerWhite ? "人机赢了" : "你赢了") :"白子胜利");
                        break;
                    case game::gameState::blackWin:
                        stateT = (Board::isPVE ? (Board::AIMode == Board::Mode::ComputerBlack ? "人机赢了" : "你赢了") :"黑子胜利");
                        break;
                    default:
                        stateT = "出BUG了！";
                        break;
                    }
                    Board::ui->label->setText(stateT);

                }
            }

            update();


        }

    }
}

//// TODO 自定义内容
void Board::closeEvent(QCloseEvent *event) {
    if (hasSaved()) {

    } else {
        /// 没有被保存，弹出提示框进行保存
        QString dialtitle = "Warning";
        QString strInfo = "你的对局还没有保存，是否确定保存并退出";
        QMessageBox::StandardButton result=QMessageBox::question(this, dialtitle, strInfo,
                                                                   QMessageBox::Yes|QMessageBox::No );
        if (result == QMessageBox::Yes) {
            /// TODO
            /// SAVE IT!!!
            Board::saveFile();
        }
        event->accept();
    }
}


std::pair<size_t, size_t> Board::getPointAbsLocation(int mouseX, int mouseY) {
    size_t x, y;
    x = ( mouseX - MARGEIN + EDGE  - 1) / WIDTH;
    y = ( mouseY - MARGEIN + EDGE  - 1) / WIDTH;
    return std::make_pair(x, y);
}

/// 重新开始对局
void Board::on_restart_clicked()
{
    /// 友好提示框
    QString dialtitle = "Warning";
    QString strInfo = "本操作不可撤销，确定重新开始吗";
    QMessageBox::StandardButton result=QMessageBox::question(this, dialtitle, strInfo,
                                                               QMessageBox::Yes|QMessageBox::No );
    if (result == QMessageBox::Yes) {
        Game->restart();
        update();
        if (Board::isPVE) {
            ChessEngine::reset(Board::AIMode == Board::Mode::ComputerBlack ? 0 : 1);
        }
    }

}


void Board::on_huiqi_clicked()
{
    /// TODO 怎么禁止AI 悔棋， 好像不用，因为AI 不会悔棋
    Game->huiqi();
    if (Board::isPVE) {
        /// TODO
    }
    update();
}

/// 按键保存
void Board::on_save_clicked()
{
    Board::saveFile();
}


/// TODO : 怎么吧文件名传过来
void Board::openGameFile(const std::string filename) {

    std::vector<game::Move> his;

    if (fileutils::readFromFile(filename, his)) {
        Game->loadDataToGame(his);
        update();
        this->filename = filename;
    } else {
        qDebug() << "读取文件中数据失败";
    }

}


void Board::on_end_clicked()
{
    if (!hasSaved()) {
        /// 没有被保存，弹出提示框进行保存
        QString dialtitle = "Warning";
        QString strInfo = "你的对局还没有保存，是否确定保存并退出";
        QMessageBox::StandardButton result=QMessageBox::question(this, dialtitle, strInfo,
                                                                   QMessageBox::Yes|QMessageBox::No);
        if (result == QMessageBox::Yes) {
            /// TODO
            /// SAVE IT!!!
            Board::saveFile();
        }
    }

    emit goBackStart();
}

/// 保存文件
void Board::saveFile() {
    //// TODO
    /// 如果处于已保存状态不用理会
    /// 更改是否保存过这一变量
    if (isSaved)
        return;

    /// 代表没有文件名在加载中，还没有保存过
    if (filename.size() == 0) {
        std::stringstream oss;
        /// TODO 怎么把这个.data设置成常量
        oss << __DATE__ << " " << __TIME__ << ".data";
        filename = oss.str();
        /// 起名字好烦
        std::replace(filename.begin(), filename.end(), ':', '-');
    }

    /// 有了文件名了，向里面保存就可以
    if (fileutils::writeTo(Game->getGameData(),  filename ) ) {
        qDebug() << "succeed saving the game";
        YesSaved();
        /// TODO: GUI it!
    }
}

bool Board::hasSaved() {
    return isSaved;
}

void Board::NotSave() {
    isSaved = false;
}

void Board::YesSaved() {
    isSaved = true;
}


void Board::on_label_linkHovered(const QString &link)
{
   ///hahaha
}

void Board::useAIMode(const Board::Mode mode) {
    Board::isPVE = true;
    Board::AIMode = mode;

    ui->label->setText((mode == Board::ComputerBlack ? "电脑下" : "轮到你了"));

    /// init AI
    qDebug() << "初始化AI:" << (mode == Board::ComputerBlack ? "AI黑子" : "AI白子");
    ChessEngine::beforeStart();
    ChessEngine::reset(mode);

    /// AI先手，下在天元
    if (mode == Board::Mode::ComputerBlack) {
        qDebug() << "AI 先手";
        Game->MakeMoveHelper(7, 7);
        Board::NotSave();

        update();
    }

}
