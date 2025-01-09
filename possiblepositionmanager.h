#ifndef POSSIBLEPOSITIONMANAGER_H
#define POSSIBLEPOSITIONMANAGER_H

#include "ChessAI.h"

class possiblepositionmanager
{
public:
    /**
     * 初始化，把上下左右一圈的方向存一下，不想用全局变量
     */
    PossiblePointManager();

    ~PossiblePointManager() = default;

    /**
     * 载入可能的棋局
     * @param board 当前棋局的状态
     * @param p 目前想下的棋子的位置，依据这个开始找
     */
    void AddPossiblePoints(char board[GRID_NUM][GRID_NUM], const Point &pos);

    /**
     * 撤销这一步搜索对ppm的改变
     */
    void Rollback();

    /**
     * 对外接口，返回当前局面下可能的下一步
     * @return 下一步落子的位置的所有可能（TODO 精准与剪枝）
     */
    const set<Point> &getCurrentPossiblePoints();

    /**
     * 清零~开启下一句时
     */
    void RemoveAll();

    void SetEvaluateFunc(int (*evaluateFunc)(char board[GRID_NUM][GRID_NUM], Point p));

private:
    set<Point> currentPossiblePositions;
    vector<HistoryItem> history;
    vector<pair<int, int>> directions;

    int (*evaluateFunc)(char board[GRID_NUM][GRID_NUM], Point p);
};

#endif // POSSIBLEPOSITIONMANAGER_H
