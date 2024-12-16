#include <cstdlib>
#include <set>
#include <vector>
#include <stack>
#include <list>
#include <iostream>
#include "jsoncpp/json.h"
#include <iomanip>

#include <cstring>

using namespace std;


///////////////////////////////////////////////////////////////////////////////////////
/// 有关的常量
///////////////////////////////////////////////////////////////////////////////////////
constexpr int GRID_NUM = 15; /// 棋盘大小
constexpr int MAX_SCORE = 10000000; ///
constexpr int MIN_SCORE = -10000000;
#define HASH_ITEM_INDEX_MASK (0xffff)
constexpr int UNKNOWN_SCORE = 10000001;

constexpr int DEPTH = 6; /// 搜索深度

struct Pattern {
    string type;
    int score;
};


/// 旗型的评分函数 TODO 优化
static vector<Pattern> patterns = {
    {"11111", 50000}, /// 成五
    {"011110", 4320}, /// 活4
    {"011100", 720}, /// 活3
    {"001110", 720}, /// 活3
    {"011010", 720}, ///
    {"010110", 720},
    {"11110", 720},
    {"01111", 720},
    {"11011", 720},
    {"10111", 720},
    {"11101", 720},
    {"001100", 120}, ///
    {"001010", 120},
    {"010100", 120},
    {"000100", 20},
    {"001000", 20},
};


///////////////////////////////////////////////////////////////////////////////////////
/// 棋子位置类，存储得分
///////////////////////////////////////////////////////////////////////////////////////
class Point {
public:
    int x, y; /// 目前下的位置
    int score; // 棋局的估分

    Point(const int x, const int y) : x(x), y(y) {
        score = 0;
    }

    Point() {
    };

    Point(const int x, const int y, const int score) : x(x), y(y), score(score) {
    }

    /// 比较，用来在可能棋局中排序
    bool operator<(const Point &other) const {
        return (score > other.score) ||
               (score == other.score && (x < other.x || (x == other.x && y < other.y)));
    }
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

    ~PossiblePointManager() {
    } ;

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
    vector<pair<int, int> > directions;

    int (*evaluateFunc)(char board[GRID_NUM][GRID_NUM], Point p);
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

    enum Flag {
        ALPHA = 0,
        BETA = 1,
        EXACT = 2,
        EMPTY = 3
    } flag;
};

///////////////////////////////////////////////////////////////////////////////////////
/// AC自动机实现字符串匹配
///////////////////////////////////////////////////////////////////////////////////////

// TODO 重写AC自动机，这个还不是足够优化
constexpr size_t MAX_AC = 52; /// 49 + 3cache
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
    int tree[MAX_AC][2] = {};
    int fail[MAX_AC] = {};
    int cnt = 0;

    void insert(const int i, const string str);

    void setFail();

    int box[MAX_AC];
    bool visited[MAX_AC];
    int times[MAX_AC];

    /// 链式前向星
    int head[MAX_AC];
    int edge = 0;

    struct treeNode {
        int next, to;
    } failGrape[MAX_AC];

    void addEdge(const int u, const int v);

    void runBFS(const int i);
};


///////////////////////////////////////////////////////////////////////////////////////
/// AI核心实现与对外接口
///////////////////////////////////////////////////////////////////////////////////////

namespace ChessAI {
    enum Role { HUMAN = 1, COMPUTER = 2, EMPTY = 0 };

    long long boardZobristValue[2][GRID_NUM][GRID_NUM];
    long long currentZobristValue;
    HashItem hashItems[HASH_ITEM_INDEX_MASK];
    char board[GRID_NUM][GRID_NUM];
    int winner;

    stack<Point> moves;
    int scores[2][72]; /// 15 + 15 + 21 + 21
    int allScores[2][72];

    ACAutm acseacher;
    PossiblePointManager ppm;

    Point searchResult;

    void init();

    /**
     * 初始化
     */
    void beforeStart();

    /**
     *
     * @return 返回胜利者
     */
    int isSomeOneWin();

    /**
     * 悔棋
     * @return
     */
    string takeBack();

    /**
     * 清楚记录，重新开局
     * @param role 谁是黑手
     * @return
     */
    string reset(int role);

    /**
     * 设置难度
     * @param level
     */
    void setLevel(const int level);

    /**
     *
     * @return 返回电脑下的那一步的棋子的位置
     */
    Point getLastPoint();

    /**
     * 人类下棋
     * @param x
     * @param y
     */
    void nextStep(const int x, const int y);

    ////////////////////////////////////////////////////////////////////////////////
    /**
     * 记录置换表
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
     * @return
     */
    long long random64();

    /**
     * 初始化置换表的权值
     */
    void initCurrentZobristValue();


    ////////////////////////////////////////////////////////////////////////////////

    /**
     * 根据位置对棋盘board 评分
     */
    int evaluatePoint(char board[GRID_NUM][GRID_NUM], Point p);

    /**
     * 对局面估计分数
     * @param board
     * @param role
     * @return
     */
    int evaluate(char board[GRID_NUM][GRID_NUM], Role role);

    /**
     * 更新目前棋局下某点的得分
     * @param board
     * @param p
     */
    void updateScore(char board[GRID_NUM][GRID_NUM], Point p);

    /**
     * alpha-beta 剪枝
     * @param board 目前的棋局
     * @param depth 目前的深度
     * @param alpha a值
     * @param beta  b值
     * @param currentSearchRole 现在以什么角色进行评分
     * @return 返回分数
     */
    int abSearch(char board[GRID_NUM][GRID_NUM], int depth, int alpha, int beta, Role currentSearchRole);
};


///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
/// 功能的实现部分，仍然分类写
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////////
/// PossiblePointManager类的实现
///////////////////////////////////////////////////////////////////////////////////////
bool isInBoard(const int x, const int y) {
    return x >= 0 && y >= 0 && x < GRID_NUM && y < GRID_NUM;
}

PossiblePointManager::PossiblePointManager() {
    directions.emplace_back(1, 1);
    directions.emplace_back(1, -1);
    directions.emplace_back(-1, 1);
    directions.emplace_back(-1, -1);
    directions.emplace_back(1, 0);
    directions.emplace_back(0, 1);
    directions.emplace_back(-1, 0);
    directions.emplace_back(0, -1);
}

void PossiblePointManager::AddPossiblePoints(char board[GRID_NUM][GRID_NUM], const Point &p) {
    unsigned int i;
    set<Point> addedPositions;

    for (i = 0; i < 8; ++i) {
        if (!isInBoard(p.x + directions[i].first, p.y + directions[i].second)) {
            continue;
        }

        if (board[p.x + directions[i].first][p.y + directions[i].second] == ChessAI::EMPTY) {
            Point pos(p.x + directions[i].first, p.y + directions[i].second);
            pair<set<Point>::iterator, bool> insertedResult = currentPossiblePositions.insert(pos);

            if (insertedResult.second) {
                addedPositions.insert(pos);
            }
        }
    }

    HistoryItem hi;
    hi.addedPositions = addedPositions;

    if (currentPossiblePositions.contains(p)) {
        currentPossiblePositions.erase(p);
        hi.removedPosition = p;
    } else {
        hi.removedPosition.x = -1;
    }

    history.push_back(hi);
}

void PossiblePointManager::Rollback() {
    if (currentPossiblePositions.empty()) {
        return;
    }

    HistoryItem hi = history[history.size() - 1];
    history.pop_back();

    for (auto item: hi.addedPositions) {
        currentPossiblePositions.erase(item);
    }

    if (hi.removedPosition.x != -1) {
        currentPossiblePositions.insert(hi.removedPosition);
    }
}

const set<Point> &PossiblePointManager::getCurrentPossiblePoints() {
    return currentPossiblePositions;
}

void PossiblePointManager::RemoveAll() {
    currentPossiblePositions.clear();
    history.clear();
}


///////////////////////////////////////////////////////////////////////////////////////
/// AC自动机
///////////////////////////////////////////////////////////////////////////////////////

ACAutm::ACAutm() {
    memset(times, 0, sizeof(times));
    memset(fail, 0, sizeof(fail));
    memset(failGrape, 0, sizeof(failGrape));
    memset(visited, 0, sizeof(visited));
    memset(Ends, 0, sizeof(Ends));
    memset(head, 0, sizeof(head));
    memset(tree, 0, sizeof(tree));
    memset(box, 0, sizeof(box));
}

void ACAutm::insert(const int i, const string str) {
    int u = 0;
    for (int j = 0, len = str.length(); j < len; ++j) {
        int id = 2;;
        switch (str[j]) {
            /// TODO
            case '0':
                id = 0;
                break;
            case '1':
                id = 1;
                break;
        }

        if (tree[u][id] == 0) {
            tree[u][id] = ++ACAutm::cnt;
        }
        u = tree[u][id];
    }

    ACAutm::Ends[i] = u;
}

void ACAutm::LoadPattern(const vector<Pattern> &patterns) {
    int i = 1;
    for (auto pattern: patterns) {
        const string &s = pattern.type;
        ACAutm::insert(i, s);
        i++;
    }

    ACAutm::setFail();

    for (int i = 1; i <= ACAutm::cnt; ++i) {
        addEdge(ACAutm::fail[i], i);
    }
}

void ACAutm::setFail() {
    int l = 0, r = 0;
    for (int i = 0; i <= 2; ++i) {
        if (tree[0][i] > 0) {
            ACAutm::box[r++] = tree[0][i];
        }
    }

    while (l < r) {
        int u = box[l++];
        for (int i = 0; i <= 2; ++i) {
            if (tree[u][i] == 0) {
                tree[u][i] = tree[fail[u]][i]; /// TODO
            } else {
                fail[tree[u][i]] = tree[fail[u]][i];
                box[r++] = tree[u][i];
            }
        }
    }
}

vector<int> ACAutm::ACSearch(const string &text) {
    vector<int> result;
    memset(times, 0, sizeof(times));


    for (int u = 0, i = 0, len = text.length(); i < len; ++i) {
        int id = 2;
        // TODO 改成合规的
        switch (text[i]) {
            case '0':
                id = 0;
                break;
            case '1':
                id = 1;
                break;
        }
        u = tree[u][id];

        ACAutm::times[u]++;
    }


    ACAutm::runBFS(0);

    /// TODO 要不要改成常量
    result.resize(16);
    for (int i = 1; i <= 16; ++i) {
        result[i] = times[Ends[i]];
    }

    return result;
}

void ACAutm::addEdge(const int u, const int v) {
    ACAutm::failGrape[++edge].next = head[u];
    ACAutm::failGrape[edge].to = v;
    head[u] = edge;
}

void ACAutm::runBFS(const int u) {
    memset(visited, 0, sizeof(visited));

    int r = 0;
    box[r++] = u;
    int cur;
    while (r > 0) {
        cur = box[r - 1];
        if (!ACAutm::visited[cur]) {
            ACAutm::visited[cur] = true;
            for (int i = ACAutm::head[cur]; i > 0; i = ACAutm::failGrape[i].next) {
                box[r++] = ACAutm::failGrape[i].to;
            }
        } else {
            r--;
            for (int i = ACAutm::head[cur]; i > 0; i = ACAutm::failGrape[i].next) {
                ACAutm::times[cur] += times[ACAutm::failGrape[i].to];
            }
        }
    }
}



///////////////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////////////












































//
//
// class AI {
// public:
//     AI();
//
//     ~AI() = default;
//
//     void nextStep(int x, int y);
//
//     Point getLastPosition();
//
//     void firstStep();
//
//     char board[GRID_NUM][GRID_NUM];
//
//     Point getGoodMove(char board[GRID_NUM][GRID_NUM]);
//
// private:
//     const int depth = 3;
//
//     int winner = -1;
//
//     /// 评分区，记忆化
//     stack<Point> moves;
//     ACAutm searcher;
//     PossiblePositionManager ppm;
//
//
//     void randomBoardZobristValue();
//
//     void initCurrentZobristValue();
//
//     long long random64();
//
//
//     /// search
//     char chessBoard[GRID_NUM][GRID_NUM];
//     Point searchResult;
//
//     int evaluatePoint(char board[GRID_NUM][GRID_NUM], const Point p);
//
//     int evaluate(char board[GRID_NUM][GRID_NUM], Piece p);
//
//     void updateScore(char board[GRID_NUM][GRID_NUM], const Point p);
//
//     int alphabetaSearch(char board[GRID_NUM][GRID_NUM], int depth, int alpha, int beta,
//                         Piece currentSearchChessquer);
// };
//
// /**
//  * 记录哈希表可能的产物
//  * @param depth
//  * @param score
//  * @param flag
//  */
// void AI::recordHashItem(const int depth, const int score, const HashItem::Flag flag) {
//     int index = (int) (AI::currentZobristValue & HASH_ITEM_INDEX_MASK);
//
//     HashItem *phashItem = &hashItems[index];
//
//     /// 已经有了,不用存
//     if (phashItem->flag != HashItem::EMPTY && phashItem->depth > depth) {
//         return;
//     }
//
//     phashItem->checksum = currentZobristValue;
//     phashItem->score = score;
//     phashItem->flag = flag;
//     phashItem->depth = depth;
// }
//
// /**
//  * 返回已经算好的表里存的得分
// */
// int AI::getHashItemScore(const int depth, const int alpha, const int beta) {
//     int index = (int) (AI::currentZobristValue & HASH_ITEM_INDEX_MASK);
//     HashItem *phashItem = &hashItems[index];
//
//     if (phashItem->flag == HashItem::EMPTY) {
//         return UNKNOWN_SCORE;
//     }
//
//     if (phashItem->checksum == currentZobristValue) {
//         if (phashItem->depth >= depth) {
//             ////// TODO understood
//             if (phashItem->flag == HashItem::EXACT) {
//                 return phashItem->score;
//             }
//             if (phashItem->flag == HashItem::ALPHA) {
//                 return alpha;
//             }
//             if (phashItem->flag == HashItem::BETA) {
//                 return beta;
//             }
//         }
//     }
//
//     return UNKNOWN_SCORE;
// }
//
// /**
//  * @return 返回一个64位随机数
//  */
// long long AI::random64() {
//     return ((long long) rand() << 15) | (long long) rand() | ((long long) rand() << 30) |
//            ((long long) rand() << 45) |
//            ((long long) rand() << 60);
// }
//
// void test_random64() {
//     long long a = ((long long) rand() << 15) | (long long) rand() | ((long long) rand() << 30) |
//                   ((long long) rand() << 45) |
//                   ((long long) rand() << 60);
//     cout << hex << a << endl << endl;
//     a = ((long long) rand() << 15) | (long long) rand() | ((long long) rand() << 30) |
//         ((long long) rand() << 45) |
//         ((long long) rand() << 60);
//     cout << hex << a << endl << endl;
//     a = ((long long) rand() << 15) | (long long) rand() | ((long long) rand() << 30) |
//         ((long long) rand() << 45) |
//         ((long long) rand() << 60);
//     cout << hex << a << endl << endl;
// }
//
// /**
//  * @brief 给每个位置赋随机权重
//  */
// void AI::randomBoardZobristValue() {
//     int i, j, k;
//     for (i = 0; i < 2; ++i) {
//         for (j = 0; j < GRID_NUM; ++j) {
//             for (k = 0; k < GRID_NUM; ++k) {
//                 AI::boardZobristValue[i][j][k] = random64();
//             }
//         }
//     }
// }
//
// /// 初始化，随机当前值
// void AI::initCurrentZobristValue() {
//     currentZobristValue = random64();
// }
//
//
// /**
//  * @brief
//  * @return 返回该点的局面估计值
//  */
// int AI::evaluatePoint(char board[GRID_NUM][GRID_NUM], const Point p) {
//     int result = 0;
//     int i, j;
//     Piece role = HUMAN;
//
//     /// TODO why line line1
//     string lines[4]; /// me
//     string lines1[4]; /// he
//
//     /// heng
//     for (i = max(0, p.x - 5); i < (int) min(p.x + 6, GRID_NUM); ++i) {
//         if (i != p.x) {
//             lines[0].push_back(board[i][p.y] == role ? '1' : board[p.x][i] == 0 ? '0' : '2');
//             lines1[0].push_back(board[i][p.y] == role ? '2' : board[i][p.y] == 0 ? '0' : '1');
//         } else {
//             lines[0].push_back('1');
//             lines1[0].push_back('1');
//         }
//     }
//
//     /// shu
//     for (i = max(0, p.y - 5); i < (int) min(GRID_NUM, p.y + 6); i++) {
//         if (i != p.y) {
//             lines[1].push_back(board[p.x][i] == role ? '1' : board[p.x][i] == 0 ? '0' : '2');
//             lines1[1].push_back(board[p.x][i] == role ? '2' : board[p.x][i] == 0 ? '0' : '1');
//         } else {
//             lines[1].push_back('1');
//             lines1[1].push_back('1');
//         }
//     }
//
//     /// zuo shang
//     for (i = p.x - min(min(p.x, p.y), 5), j = p.y - min(min(p.x, p.y), 5);
//          i < (int) min(GRID_NUM, p.x + 6) && j < (int) min(GRID_NUM, p.y + 6); i++, j++) {
//         if (i != p.x) {
//             lines[2].push_back(board[i][j] == role ? '1' : board[i][j] == 0 ? '0' : '2');
//             lines1[2].push_back(board[i][j] == role ? '2' : board[i][j] == 0 ? '0' : '1');
//         } else {
//             lines[2].push_back('1');
//             lines1[2].push_back('1');
//         }
//     }
//
//     /// you shang
//     for (i = p.x + min(min(p.y, GRID_NUM - 1 - p.x), 5), j = p.y - min(min(p.y, GRID_NUM - 1 - p.x), 5);
//          i >= (int) max(0, p.x - 5) && j < (int) min(GRID_NUM, p.y + 6); i--, j++) {
//         if (i != p.x) {
//             lines[3].push_back(board[i][j] == role ? '1' : board[i][j] == 0 ? '0' : '2');
//             lines1[3].push_back(board[i][j] == role ? '2' : board[i][j] == 0 ? '0' : '1');
//         } else {
//             lines[3].push_back('1');
//             lines1[3].push_back('1');
//         }
//     }
//
//
//     for (i = 0; i < 4; i++) {
//         vector<int> tmp = AI::searcher.ACSearch(lines[i]);
//         for (j = 0; j < int(tmp.size()); j++) {
//             result += patterns[tmp[j]].score;
//         }
//
//         tmp = AI::searcher.ACSearch(lines1[i]);
//         for (j = 0; j < int(tmp.size()); j++) {
//             result += patterns[tmp[j]].score;
//         }
//     }
//
//     return result;
// }
//
// int AI::evaluate(char board[GRID_NUM][GRID_NUM], Piece role) {
//     if (role == COMPUTER) {
//         return AI::allScores[1];
//     } else if (role == HUMAN) {
//         return AI::allScores[0];
//     } else {
//         cout << "error" << "evaluate() " << endl;
//         return 0;
//     }
// }
//
// void AI::updateScore(char board[GRID_NUM][GRID_NUM], const Point p) {
//     string lines[4];
//     string lines1[4];
//     int i, j;
//     int role = HUMAN;
//
//     //竖
//     for (i = 0; i < GRID_NUM; i++) {
//         lines[0].push_back(board[i][p.y] == role ? '1' : board[i][p.y] == 0 ? '0' : '2');
//         lines1[0].push_back(board[i][p.y] == role ? '2' : board[i][p.y] == 0 ? '0' : '1');
//     }
//     //横
//     for (i = 0; i < GRID_NUM; i++) {
//         lines[1].push_back(board[p.x][i] == role ? '1' : board[p.x][i] == 0 ? '0' : '2');
//         lines1[1].push_back(board[p.x][i] == role ? '2' : board[p.x][i] == 0 ? '0' : '1');
//     }
//     //反斜杠
//     for (i = p.x - min(p.x, p.y), j = p.y - min(p.x, p.y); i < GRID_NUM && j < GRID_NUM; i++, j++) {
//         lines[2].push_back(board[i][j] == role ? '1' : board[i][j] == 0 ? '0' : '2');
//         lines1[2].push_back(board[i][j] == role ? '2' : board[i][j] == 0 ? '0' : '1');
//     }
//     //斜杠
//     for (i = p.x + min(p.y, GRID_NUM - 1 - p.x), j = p.y - min(p.y, GRID_NUM - 1 - p.x); i != 0 && j < GRID_NUM;
//          i--, j++) {
//         lines[3].push_back(board[i][j] == role ? '1' : board[i][j] == 0 ? '0' : '2');
//         lines1[3].push_back(board[i][j] == role ? '2' : board[i][j] == 0 ? '0' : '1');
//     }
//
//     int lineScore[4];
//     int line1Score[4];
//     memset(lineScore, 0, sizeof(lineScore));
//     memset(line1Score, 0, sizeof(line1Score));
//
//     /// 算分
//     for (i = 0; i < 4; ++i) {
//         vector<int> result = AI::searcher.ACSearch(lines[i]);
//         for (j = 0; j < int(result.size()); ++j) {
//             lineScore[i] += patterns[result[j]].score;
//         }
//
//         result = AI::searcher.ACSearch(lines1[i]);
//         for (j = 0; j < int(result.size()); ++j) {
//             line1Score[i] += patterns[result[j]].score;
//         }
//     }
//
//     ///  算x, y点在横竖撇捺中的ID
//     int a = p.y, b = GRID_NUM + p.x;
//     int c = 2 * GRID_NUM + (p.y - p.x + 10);
//     int d = 2 * GRID_NUM + (p.x + p.y - 4);
//
//     /// 减去原来的
//     for (i = 0; i < 2; ++i) {
//         allScores[i] -= scores[i][a];
//         allScores[i] -= scores[i][b];
//     }
//
//     scores[0][a] = lineScore[0]; // 竖
//     scores[1][a] = line1Score[0]; // 竖
//     scores[0][b] = lineScore[1]; // 横
//     scores[1][b] = line1Score[1]; // 横
//
//     /// 加上下了这一子后的
//     for (i = 0; i < 2; ++i) {
//         AI::allScores[i] += scores[i][a];
//         AI::allScores[i] += scores[i][b];
//     }
//
//     ///  ↖
//     /// 能成5的
//     if (p.y - p.x >= -10 && p.y - p.x <= 10) {
//         for (i = 0; i < 2; ++i)
//             AI::allScores[i] -= scores[i][c];
//
//
//         scores[0][c] = lineScore[2];
//         scores[1][c] = line1Score[2];
//
//         for (i = 0; i < 2; i++)
//             AI::allScores[i] += scores[i][c];
//     }
//
//     /// ↗
//     if (p.x + p.y >= 4 && p.x + p.y <= 24) {
//         for (i = 0; i < 2; i++)
//             AI::allScores[i] -= scores[i][d];
//
//         scores[0][d] = lineScore[3];
//         scores[1][d] = line1Score[3];
//
//         for (i = 0; i < 2; i++)
//             AI::allScores[i] += scores[i][d];
//     }
// }
//
// /**
//  * alpha-beta剪枝的主函数
//  * @param board
//  * @param depth
//  * @param alpha
//  * @param beta
//  * @param currentSearchRole
//  * @return
//  */
// int AI::alphabetaSearch(char board[GRID_NUM][GRID_NUM], int depth, int alpha, int beta,
//                         Piece currentSearchRole) {
//     HashItem::Flag flag = HashItem::ALPHA;
//     int score = getHashItemScore(depth, alpha, beta);
//     if (score != UNKNOWN_SCORE && depth != DEPTH)
//         return score;
//
//     int score1 = AI::evaluate(board, currentSearchRole);
//     int score2 = AI::evaluate(board, currentSearchRole == HUMAN ? COMPUTER : HUMAN);
//
//     if (score1 >= 50000)
//         return MAX_SCORE - 1000 - (DEPTH - depth);
//     if (score2 >= 50000)
//         return MIN_SCORE + 1000 + (DEPTH - depth);
//
//     if (depth == 0) {
//         recordHashItem(depth, score1 - score2, HashItem::EXACT);
//         return score1 - score2;
//     }
//
//     int count = 0;
//     set<Point> possiblePositions;
//     /// TODO getCurrentPossiblePositions写
//     const set<Point> &tmpPossiblePositions = ppm.getCurrentPossiblePositions();
//
//     for (auto item: tmpPossiblePositions) {
//         possiblePositions.insert(Point(item.x, item.y, evaluatePoint(board, item)));
//     }
//
//     while (!possiblePositions.empty()) {
//         Point p = *possiblePositions.begin();
//         possiblePositions.erase(p);
//
//         board[p.x][p.y] = currentSearchRole;
//         /// TODO 取^
//         AI::currentZobristValue ^= boardZobristValue[currentSearchRole - 1][p.x][p.y];
//         updateScore(board, p);
//
//         /// 加可能出现的位置
//         p.score = 0;
//         /// TODO
//         ppm.AddPossiblePositions(board, p);
//
//         int val = -AI::alphabetaSearch(board, depth - 1, -beta, -alpha,
//                                        currentSearchRole == HUMAN ? COMPUTER : HUMAN);
//
//         /// back
//         ppm.Rollback();
//
//         /// 取消放置
//         board[p.x][p.y] = 0;
//         currentZobristValue ^= boardZobristValue[currentSearchRole - 1][p.x][p.y];
//         updateScore(board, p);
//
//         /// 剪枝与记忆
//         if (val >= beta) {
//             recordHashItem(depth, beta, HashItem::BETA);
//             return beta;
//         }
//         if (val > alpha) {
//             flag = HashItem::EXACT;
//             alpha = val;
//             if (depth == DEPTH) {
//                 searchResult = p;
//             }
//         }
//
//         count++;
//         if (count >= 9) {
//             break;
//         }
//     }
//
//     recordHashItem(depth, alpha, flag);
//     return alpha;
// }
//
// /**
//  * 对外接口
//  * 搜索时更新searchResult，最后返回能不能赢，TODO这几个参数1000要不要改掉
//  * @param board
//  * @return
//  */
// Point AI::getGoodMove(char board[GRID_NUM][GRID_NUM]) {
//     int score = AI::alphabetaSearch(board, DEPTH, MIN_SCORE, MAX_SCORE, COMPUTER);
//     if (score >= MAX_SCORE - 1000 - 1)
//         AI::winner = COMPUTER;
//     else if (score <= MIN_SCORE + 1000 + 1)
//         AI::winner = HUMAN;
//
//     return AI::searchResult;
// }
//
// /**
//  * 对外接口，由AI走第一步
//  */
// void AI::firstStep() {
//     AI::currentZobristValue ^= boardZobristValue[COMPUTER - 1][7][7];
//     AI::board[7][7] = COMPUTER;
//     updateScore(board, Point(7, 7));
//
//     Point p(7, 7);
//     AI::moves.push(p);
//     searchResult = p;
//
//     ppm.AddPossiblePositions(board, p);
// }
//
// AI::AI() {
//     memset(AI::chessBoard, Piece::EMPTY, sizeof(AI::chessBoard));
//     memset(scores, 0, sizeof(scores));
//
//     AI::searcher.LoadPattern(patterns);
//
//     randomBoardZobristValue();
//     currentZobristValue = random64();
// }
//
// //输出棋盘
// void printBoard(char board[GRID_NUM][GRID_NUM]) {
//     int i, j;
//     for (i = 0; i < GRID_NUM; i++) {
//         for (j = 0; j < GRID_NUM; j++) {
//             cout << (int) board[i][j] << " ";
//         }
//         cout << endl;
//     }
// }
//
//
// /**
//  * @return 返回AI 走的那一步应该是什么
//  */
// Point AI::getLastPosition() {
//     return AI::searchResult;
// }
//
// /**
//  *
//  * @param x 人下的棋子的X
//  * @param y Y
//  * @return 啥也不返回
//  */
// void AI::nextStep(int x, int y) {
//     AI::moves.push(Point(x, y));
//
//     AI::board[x][y] = HUMAN;
//     AI::currentZobristValue ^= AI::boardZobristValue[HUMAN - 1][x][y];
//     AI::updateScore(board, Point(x, y));
//
//     //增加可能出现的位置
//     ppm.AddPossiblePositions(board, Point(x, y));
//
//     Point result = AI::getGoodMove(board);
//
//     board[result.x][result.y] = COMPUTER;
//     currentZobristValue ^= boardZobristValue[COMPUTER - 1][result.x][result.y];
//     AI::updateScore(board, result);
//
//     //增加可能出现的位置
//     ppm.AddPossiblePositions(board, result);
//
//     //若双方还未决出胜负，则把棋子位置加入到历史记录中
//     if (winner == -1)
//         moves.push(Point(result.x, result.y));
//
//     // string resultStr;
//     // int i, j;
//     // for (i = 0; i < GRID_NUM; i++) {
//     //     for (j = 0; j < GRID_NUM; j++) {
//     //         resultStr.push_back(board[i][j] + 48);
//     //     }
//     // }
//
//     /// 这里打印出下完后的结果 TODO
//     //printBoard(board);
//     //cout << "resultSTR" << endl << resultStr << endl;
// }
//
// /// TODO 判断禁手
//
//
// void TEST_TERMINAL() {
//     Piece role = COMPUTER;
//     bool isFirst = true;
//     AI *ai = new AI();
//     int humanx, humany;
//     while (true) {
//         if (isFirst && role == COMPUTER) {
//             /// 电脑先手
//             ai->firstStep();
//             isFirst = false;
//         } else {
//             cin >> humanx >> humany;
//             string s = ai->nextStep(humanx, humany);
//             //cout << "AI: " <<ai->getLastPosition().x << " " << ai->getLastPosition().y << endl;
//         }
//     }
// }
//
// void BOTZONE() {
//     bool isFirst = true;
//     const int ST = 0;
//     AI *ai = new AI();
//
//     // 读入 JSON
//
//     string str;
//     Json::Reader reader;
//     Json::Value input;
//
//     Json::FastWriter writer;
//     // 输出决策 JSON
//     Json::Value ret;
//     Json::Value action;
//
//     while (true) {
//         // 读入 JSON
//         getline(cin, str);
//         reader.parse(str, input);
//
//
//         //cout << writer.write(input) << endl;
//         // 输出决策 JSON
//
//
//         Point p;
//
//         if (isFirst) {
//             isFirst = false;
//             if (input["requests"][ST]["x"].asInt() < 0) {
//                 ai->firstStep();
//                 action["x"] = 7;
//                 action["y"] = 7;
//             } else {
//                 ai->nextStep(input["requests"][ST]["x"].asInt(), input["requests"][ST]["y"].asInt());
//                 p = ai->getLastPosition();
//                 action["x"] = p.x;
//                 action["y"] = p.y;
//             }
//         } else {
//             ai->nextStep(input["x"].asInt(), input["y"].asInt());
//             p = ai->getLastPosition();
//             action["x"] = p.x;
//             action["y"] = p.y;
//         }
//         ret["response"] = action;
//         cout << writer.write(ret) << endl;
//         cout << ">>>BOTZONE_REQUEST_KEEP_RUNNING<<<" << endl;
//     }
// }
//
// int main() {
//     // BOTZONE();
//     //test_random64();
//     return 0;
// }
