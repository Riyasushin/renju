#ifndef GAME_H
#define GAME_H

/// 这里实现游戏内容
/// 判断能不能落子
/// 存落子的数据
/// 判断胜负
/// 提供接口，供悔棋操作
#include<vector>
#include <string>
#include "grid.h"
#include <QDebug>

/// fill the move
#define PUSHINGARBAGE {-1, -1, -1, -1, -1, -1, -1, -1}

/// 在头文件中直接定义了变量
/// 当这个头文件被多个源文件包含时，变量就会在多个编译单元中被重复定义
/// multiple definition
/// extern 的作用是告诉编译器：这个变量的定义在其他地方，只在一个源文件中真正分配内存。
extern std::string GAMEFILESUF;

class game
{
public:
    enum gameState {
        whiteWin,
        blackWin,
        tie,
        whiteToD,
        blackToD
    };

    enum Chessquer {
        white,
        black,
        non,
        invalid
    };

    /// maybe private
    enum Mode {
        PVP,
        PVE
    };

    struct Move {
        size_t x, y;
        Chessquer color;
        int allDirections[8];
        /// 代表上下左右等八个方向，为了判断有没有五子连珠   -1 为初始值
    };

     static const size_t WID = 16;

    game();

     /**
     * @brief canMakeMove
     * 		判断能不能走这一步棋
     * 		需要：走过没有、黑旗的话是不是禁手
     * 		不用判断出界,XY一定给的不出界的
     * @param x
     * @param y
     * @return
     */
    bool canMakeMove(const size_t x, const size_t y) const;

    /**
     * @brief MakeMoveHelper
     * @param x
     * @param y
     * 不传入棋子状态，仅仅是一个接口，棋子状态是GAME 内部的事情
     */
    void MakeMoveHelper(const size_t x, const size_t y);

    /**
     * @brief Winner
     *      一定在判定游戏结束后才能调用,这里没有判定游戏打没打完
     * @return 返回胜者，即HISTORY最后一个加进去的数据
     */
    Chessquer Winner();

    /**
     * @brief hasDecideWinner
     * 	判断是不是有胜利者了
     */
    bool hasDecideWinner() const;

    /**
     * @brief huiqi
     * 如果没有分出胜负
     * 		HISTORY头出列，改状态
     */
    void huiqi();

    /**TODO
     * @brief restart
     * 春秋蝉，带我重走一次时间长河吧
     */
    void restart();

    /**
     * @brief getGameData
     * 返回HISTORY这个数组
     */
    const std::vector<Move>& getGameData();

    /**
     * @brief loadDataToGame
     * 加载读到的信息
     * 有非法信息判断，注意！！遇到非法信息直接返回FALSE，并且clear()
     * 从文件中读取写到下棋UI界面去，这里仅仅和下棋的数据、状态有关
     * @param his : 传入的信息
     */
    bool loadDataToGame(const std::vector<Move>& his);

    /**
     * @brief getGameState
     * @return 返回当前游戏的状态
     */
    const game::gameState getGameState();

private:
    gameState state;
    std::vector<Move> history;

    /**
     * @brief MakeMove
     * 如果没有获胜
     * 	写入数据到 数据库HISTORY中
     *  更新GRID,STATE
     *  判断获胜了没有
     * @param color 棋子的颜色
     */
    void MakeMove(const size_t x, const size_t y, const Chessquer color);

    Chessquer grid[16][16];
    bool vised(const size_t x, const size_t y) const;
    void setHasVised(const size_t x, const size_t y, const Chessquer color);

    bool tryGetWinner(const size_t id, Chessquer color);


    /**
     * @brief inbound
     * 判定边界
     * 这里不能size_t
     */
    bool inbound(const int x, const int y);
    /**
     * @brief islegal
     * 禁手判断
     */
    bool islegal(const size_t x, const size_t y) const;

    const int steps[4][2] = { {0, 1}, {1, 1}, {1, 0}, {1, -1}};


};


#endif // GAME_H
