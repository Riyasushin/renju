#pragma once

#ifndef CHESSAI_H
#define CHESSAI_H
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <list>
#include <set>
#include <stack>
#include <vector>

#include <cstring>

using namespace std;

/// 棋盘大小
#define GRID_NUM  15

struct Pattern {
    string type;
    int score;
};





enum Role { HUMAN = 1, COMPUTER = 2, EMPTY = 0 };
enum Type {Black = 1, White = 2};



///////////////////////////////////////////////////////////////////////////////////////
/// 棋子位置类，存储得分
///////////////////////////////////////////////////////////////////////////////////////
class Point {
public:
    int x, y; /// 目前下的位置
    int score; // 棋局的估分
    Type color; // 棋子的TYPE，用来判断需不需要禁手

    Point(const int x, const int y) ;

    Point() = default;


    Point(const int x, const int y, const Role curRule, const int score) ;

    /// 比较，用来在set中排序
    bool operator<(const Point &other) const;
};

///////////////////////////////////////////////////////////////////////////////////////
///  记录历史的哈希表
///  棋局的数据
///  置换表
///////////////////////////////////////////////////////////////////////////////////////
struct HistoryItem {
    set<Point> addedPositions;
    Point removedPosition;
};

struct HashItem {
    long long checksum;
    int depth;
    int score;

    enum Flag { ALPHA = 0, BETA = 1, EXACT = 2, EMPTY = 3 } flag;
};

///////////////////////////////////////////////////////////////////////////////////////
/// PossiblePointManager
/// restore posible chessquer`s position and their score
///////////////////////////////////////////////////////////////////////////////////////
class PossiblePointManager {
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


///////////////////////////////////////////////////////////////////////////////////////
/// AC自动机实现字符串匹配
///////////////////////////////////////////////////////////////////////////////////////

// TODO 重写AC自动机，这个还不是足够优化
constexpr size_t MAX_AC = 200; /// 49 + 3cache
class ACAutm {
public:
    ACAutm();

    ~ACAutm() = default;

    /**
     * 加载要检查模式到字典树上
     * @param patterns
     */
    void LoadPattern(const vector<Pattern> &patterns);

    /**
     * TODO
     * @param text
     * @return 返回各种模式都有几个，按照给出模式的数列的顺序返回
     */
    vector<int> ACSearch(const string &text);

private:
    /// 字典树
    int Ends[MAX_AC];
    int tree[MAX_AC][3] = {};
    int fail[MAX_AC] = {};
    int cnt = 0;

    void insert(const int i, const string str);

    void setFail();

    int box[MAX_AC];
    bool visited[MAX_AC] = {};
    int times[MAX_AC];

    int head[MAX_AC] = {};
    int Next[MAX_AC] = {};
    int to[MAX_AC] = {};
    int edge = 0;


    void addEdge(const int u, const int v);

    void runBFS(const int i);
};


///////////////////////////////////////////////////////////////////////////////////////
/// AI核心实现与对外接口
///////////////////////////////////////////////////////////////////////////////////////

namespace ChessAI {




////////////////////////////////////////////////////////////////////////////////

/**
     * 记录当前局面的置换表
     * @param depth
     * @param score
     * @param flag
     */
void recordHashItem(const int depth, const int score, const HashItem::Flag flag);

    /**
     * 查表取分
     * @param depth
     * @param alpha
     * @param beta
     * @return
     */
int getHashItemScore(const int depth, const int alpha, const int beta);

/**
     * 生成64位随机数
     * @return 一个64位的随机数
     */
long long random64() ;

/**
     * @brief produce every point`s hashKey in the bord
     */
void randomBoardHashValue() ;

/**
     * 初始化置换表的权值
     */
void initCurrentZobristValue();


////////////////////////////////////////////////////////////////////////////////

/**
 * 根据位置对棋盘board 评分
 * 忽视禁手的情况下，黑白评估近似相同，先写TODO
 * 现住想办法加上禁手TODO
 * 如果不是人类方的话把line1 \ line 的地位交换
 * @param board 当前棋盘
 * @param p 位置
 */
int evaluatePoint(char board[GRID_NUM][GRID_NUM], Point p) ;
/**
     * 对局面估计分数
     * @param board
     * @param role
     * @return
     */
int evaluate(char board[GRID_NUM][GRID_NUM], Role role);
/**
 * 更新人类下棋后目前棋局下得分，根据算出某点的得分和历史总得分的记录,
 * TODO
 * @param board 1 ME; 2 HIM; 0 EMPTY
 * @param p
 */
void updateScore(char board[GRID_NUM][GRID_NUM], Point p);

/**
     * alpha-beta 剪枝
     * @param board 目前的棋局
     * @param depth 目前的深度,倒着走
     * @param alpha a值
     * @param beta  b值
     * @param currentSearchRole 现在以什么角色进行评分
     * @return 返回分数
     */
int alphaBetaSearch(char board[GRID_NUM][GRID_NUM], int depth, int alpha, int beta, Role currentSearchRole);
/**
     * @brief 接口;进行ab搜索，判断是否出现五子连珠（max/min）;调用，返回最佳点
     * @param board 当前棋局
     * @return 下一步走法
     */
Point getAGoodMove(char board[GRID_NUM][GRID_NUM]);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
     * @brief init:  acsearcher, hashboard
     */
void init() ;

/**
     * @brief init the whole AI part
     */
void beforeStart();

/**
     * 返回胜利者
     * @return  0 : human win
     *          1 : computer win
     *         -1 : not have winner
     */
int isSomeOneWin() ;

/**
 * 悔棋, 只能是人悔棋
 */
void takeBack();

/**
     * 初始化AI的工作
     * 清楚记录，重新开局
     * @param role 谁是黑手 1 电脑黑手，0电脑白色
     */
void reset(int role);


/**
     * @return 返回电脑下的那一步的棋子的位置
     */
Point getLastPoint() ;
/**
     * 人类下棋
     * @param x
     * @param y
     */
void nextStep(const int x, const int y) ;

}; // namespace ChessAI



Point calculate(const int x, const int y);
void testACAutm();



#endif // CHESSAI_H
