#include <cstdlib>
#include <set>
#include <vector>
#include <stack>
#include <list>
#include <iostream>
#include "jsoncpp/json.h"

#include <cstring>

using namespace std;

enum Piece {
    HUMAN = 1,
    COMPUTER = 2,
    EMPTY = 0,
};

static const int GRID_NUM = 15;
static const int MAX_SCORE = 10000000;
static const int MIN_SCORE = -10000000;
static const int UNKNOWN_SCORE = 10000001;
static const int DEPTH = 6;


struct Pattern {
    string type;
    int score;
};

static vector<Pattern> patterns = {
    {"11111", 50000},
    {"011110", 23200},
    {"011100", 720},
    {"001110", 720},
    {"011010", 720},
    {"010110", 720},
    {"11110", 720},
    {"01111", 720},
    {"11011", 720},
    {"10111", 720},
    {"11101", 720},
    {"001100", 120},
    {"001010", 120},
    {"010100", 120},
    {"000100", 20},
    {"001000", 20},
};


class Point {
public:
    int x, y;
    int score{}; // 棋局的估分

    Point(const int x, const int y) : x(x), y(y) {
    }

    Point() = default;

    Point(const int x, const int y, const int score) : x(x), y(y), score(score) {
    }


    bool operator==(const Point &other) const {
        return x == other.x && y == other.y;
    }

    bool operator!=(const Point &other) const {
        return !(*this == other);
    }

    bool operator<(const Point &other) const {
        return (score > other.score) ||
               (score == other.score && (x < other.x || (x == other.x && y < other.y)));
    }
};

#define HASH_ITEM_INDEX_MASK (0xffff)

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

class PossiblePositionManager {
public:
    PossiblePositionManager();

    ~PossiblePositionManager() = default;

    void AddPossiblePositions(char board[GRID_NUM][GRID_NUM], const Point &pos);

    void Rollback();

    const set<Point> &getCurrentPossiblePositions();

    void RemoveAll();

    //void SetEvaluateFunc(int (*evaluateFunc)(char board[GRID_NUM][GRID_NUM], Point p));

private:
    struct HistoryItem {
        set<Point> addedPositions;
        Point removedPosition;
    };

    set<Point> currentPossiblePositions;
    vector<HistoryItem> history; /// hash
    vector<pair<int, int> > directions;

    int (*evaluateFunc)(char board[GRID_NUM][GRID_NUM], Point p){};
};

bool isInBoard(const int x, const int y) {
    return x >= 0 && y >= 0 && x < GRID_NUM && y < GRID_NUM;
}

PossiblePositionManager::PossiblePositionManager() {
    directions.emplace_back(1, 1);
    directions.emplace_back(1, -1);
    directions.emplace_back(-1, 1);
    directions.emplace_back(-1, -1);
    directions.emplace_back(1, 0);
    directions.emplace_back(0, 1);
    directions.emplace_back(-1, 0);
    directions.emplace_back(0, -1);
}

void PossiblePositionManager::AddPossiblePositions(char board[GRID_NUM][GRID_NUM], const Point &p) {
    unsigned int i;
    set<Point> addedPositions;

    for (i = 0; i < 8; ++i) {
        if (!isInBoard(p.x + directions[i].first, p.y + directions[i].second)) {
            continue;
        }
        if (board[p.x + directions[i].first][p.y + directions[i].second] == EMPTY) {
            Point pos(p.x + directions[i].first, p.y + directions[i].second);
            pair<set<Point>::iterator, bool> insertedResult = currentPossiblePositions.insert(pos);

            if (insertedResult.second) {
                addedPositions.insert(pos);
            }
        }
    }

    HistoryItem hi;
    hi.addedPositions = addedPositions;
    if (currentPossiblePositions.find(p) != currentPossiblePositions.end()) {
        currentPossiblePositions.erase(p);
        hi.removedPosition = p;
    } else {
        hi.removedPosition = {-1, -1};
    }

    history.push_back(hi);
}

void PossiblePositionManager::Rollback() {
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

const set<Point> &PossiblePositionManager::getCurrentPossiblePositions() {
    return currentPossiblePositions;
}

void PossiblePositionManager::RemoveAll() {
    currentPossiblePositions.clear();
    history.clear();
}


/// AC自动机
class ACAutm {
public:
    ACAutm();

    ~ACAutm() = default;

    void LoadPattern(const vector<Pattern> &patterns);

    void BuildGotoTable();

    void BuildFailTable();

    vector<int> ACSearch(const string &text);

private:
    struct ACNode {
        ACNode(int p, char c) : parent(p), ch(c) {
            fail = -1;
        }

        map<char, int> sons;
        int parent;
        char ch;
        int fail;
        vector<int> output;
    };

    void AddState(int parent, const char ch);

    int maxState;
    vector<ACNode> nodes; /// 字典树
    vector<string> ACpatterns;
};

ACAutm::ACAutm(): maxState(0) {
    ACAutm::AddState(-1, 'a');
    nodes[0].fail = -1;
}

void ACAutm::LoadPattern(const vector<Pattern> &patterns) {
    this->ACpatterns.clear();
    for (auto pattern: patterns) {
        this->ACpatterns.push_back(pattern.type);
    }
}


void ACAutm::BuildGotoTable() {
    unsigned int i, j;
    for (i = 0; i < ACpatterns.size(); i++) {
        int currentIndex = 0;
        /// !
        for (j = 0; j < ACpatterns[i].size(); ++j) {
            if (nodes[currentIndex].sons.find(ACpatterns[i][j]) == nodes[currentIndex].sons.end()) {
                nodes[currentIndex].sons[ACpatterns[i][j]] = ++maxState;

                AddState(currentIndex, ACpatterns[i][j]);
                currentIndex = maxState;
            } else {
                currentIndex = nodes[currentIndex].sons[ACpatterns[i][j]];
            }
        }

        nodes[currentIndex].output.push_back(i);
    }
}

void ACAutm::BuildFailTable() {
    vector<int> midNodesIndex;
    const ACNode root = nodes[0];
    for (auto [fst, snd]: root.sons) {
        nodes[snd].fail = 0;
        ACNode &currentNode = nodes[snd];

        for (auto [fst, snd]: currentNode.sons) {
            midNodesIndex.push_back(snd);
        }
    }

    while (!midNodesIndex.empty()) {
        vector<int> newMidNodesIndex;

        unsigned int i;
        for (i = 0; i < midNodesIndex.size(); i++) {
            ACNode &currentNode = nodes[midNodesIndex[i]];

            //以下循环为寻找当前节点的fail值
            int currentFail = nodes[currentNode.parent].fail;
            while (true) {
                ACNode &currentFailNode = nodes[currentFail];

                if (currentFailNode.sons.find(currentNode.ch) != currentFailNode.sons.end()) {
                    //成功找到该节点的fail值
                    currentNode.fail = currentFailNode.sons.find(currentNode.ch)->second;

                    //后缀包含
                    if (nodes[currentNode.fail].output.size()) {
                        currentNode.output.insert(currentNode.output.end(), nodes[currentNode.fail].output.begin(),
                                                  nodes[currentNode.fail].output.end());
                    }

                    break;
                } else {
                    currentFail = currentFailNode.fail;
                }

                //如果是根节点
                if (currentFail == -1) {
                    currentNode.fail = 0;
                    break;
                }
            }

            /// 下一层
            for (auto item1: currentNode.sons) {
                newMidNodesIndex.push_back(item1.second);
            }
        }
        midNodesIndex = newMidNodesIndex;
    }
}

vector<int> ACAutm::ACSearch(const string &text) {
    vector<int> result;

    //初始化为根节点
    int currentIndex = 0;

    unsigned int i;
    map<char, int>::iterator tmpIter;
    for (i = 0; i < text.size();) {
        //顺着trie树查找
        if ((tmpIter = nodes[currentIndex].sons.find(text[i])) != nodes[currentIndex].sons.end()) {
            currentIndex = tmpIter->second;
            i++;
        } else {
            //失配的情况
            while (nodes[currentIndex].fail != -1 && nodes[currentIndex].sons.find(text[i]) == nodes[currentIndex].sons.
                   end()) {
                currentIndex = nodes[currentIndex].fail;
            }

            //如果没有成功找到合适的fail
            if (nodes[currentIndex].sons.find(text[i]) == nodes[currentIndex].sons.end()) {
                i++;
            }
        }

        if (nodes[currentIndex].output.empty()) {
            result.insert(result.end(), nodes[currentIndex].output.begin(), nodes[currentIndex].output.end());
        }
    }

    return result;
}

void ACAutm::AddState(int parent, char ch) {
    nodes.push_back(ACNode(parent, ch));
}


class AI {
public:
    AI();

    ~AI() = default;

    string nextStep(int x, int y);

    Point getLastPosition();

    void firstStep();

    char board[GRID_NUM][GRID_NUM];

    Point getGoodMove(char board[GRID_NUM][GRID_NUM]);

private:
    const int depth = 3;

    int winner = -1;

    /// 评分区，记忆化
    stack<Point> moves;
    int scores[2][72]; /// 棋局的分数 2色   72行 = 15 + 15 + 21 + 21
    int allScores[2]; // 局面总分
    ACAutm searcher;
    PossiblePositionManager ppm;

    /// Hash记忆话存储，虽然优化不了多少
    long long boardZobristValue[2][GRID_NUM][GRID_NUM];
    long long currentZobristValue;

    void randomBoardZobristValue();

    void initCurrentZobristValue();

    long long random64();

    HashItem hashItems[HASH_ITEM_INDEX_MASK];

    void recordHashItem(const int depth, const int score, const HashItem::Flag flag);

    int getHashItemScore(const int depth, const int alpha, const int beta);


    /// search
    char chessBoard[GRID_NUM][GRID_NUM];
    Point searchResult;

    int evaluatePoint(char board[GRID_NUM][GRID_NUM], Point p);

    int evaluate(char board[GRID_NUM][GRID_NUM], Piece p);

    void updateScore(char board[GRID_NUM][GRID_NUM], const Point p);

    int alphabetaSearch(char board[GRID_NUM][GRID_NUM], int depth, int alpha, int beta,
                        Piece currentSearchChessquer);
};

void AI::recordHashItem(const int depth, const int score, const HashItem::Flag flag) {
    int index = (int) (AI::currentZobristValue & HASH_ITEM_INDEX_MASK);

    HashItem *phashItem = &hashItems[index];

    /// 已经有了,不用存
    if (phashItem->flag != HashItem::EMPTY && phashItem->depth > depth) {
        return;
    }

    phashItem->checksum = currentZobristValue;
    phashItem->score = score;
    phashItem->flag = flag;
    phashItem->depth = depth;
}

/// 返回已经算好的表里存的得分
int AI::getHashItemScore(const int depth, const int alpha, const int beta) {
    int index = (int) (AI::currentZobristValue & HASH_ITEM_INDEX_MASK);
    HashItem *phashItem = &hashItems[index];

    if (phashItem->flag == HashItem::EMPTY) {
        return UNKNOWN_SCORE;
    }

    if (phashItem->checksum == currentZobristValue) {
        if (phashItem->depth >= depth) {
            ////// TODO understood
            if (phashItem->flag == HashItem::EXACT) {
                return phashItem->score;
            }
            if (phashItem->flag == HashItem::ALPHA) {
                return alpha;
            }
            if (phashItem->flag == HashItem::BETA) {
                return beta;
            }
        }
    }

    return UNKNOWN_SCORE;
}

long long AI::random64() {
    return ((long long) rand() << 15) | (long long) rand() | ((long long) rand() << 30) |
           ((long long) rand() << 45) |
           ((long long) rand() << 60);
}

/// 给每个位置赋随机权重
void AI::randomBoardZobristValue() {
    int i, j, k;
    for (i = 0; i < 2; ++i) {
        for (j = 0; j < GRID_NUM; ++j) {
            for (k = 0; k < GRID_NUM; ++k) {
                AI::boardZobristValue[i][j][k] = random64();
            }
        }
    }
}

/// 初始化，随机当前值
void AI::initCurrentZobristValue() {
    currentZobristValue = random64();
}

/// 根据当前位置评分,粗评，未来排序用
int AI::evaluatePoint(char board[GRID_NUM][GRID_NUM], Point p) {
    int result = 0;
    int i, j;
    Piece role = HUMAN;

    /// TODO why line line1
    string lines[4]; /// me
    string lines1[4]; /// he

    /// heng
    for (i = max(0, p.x - 5); i < (int) min(p.x + 6, GRID_NUM); ++i) {
        if (i != p.x) {
            lines[0].push_back(board[i][p.y] == role ? '1' : board[p.x][i] == 0 ? '0' : '2');
            lines1[0].push_back(board[i][p.y] == role ? '2' : board[i][p.y] == 0 ? '0' : '1');
        } else {
            lines[0].push_back('1');
            lines1[0].push_back('1');
        }
    }

    /// shu
    for (i = max(0, p.y - 5); i < (int) min(GRID_NUM, p.y + 6); i++) {
        if (i != p.y) {
            lines[1].push_back(board[p.x][i] == role ? '1' : board[p.x][i] == 0 ? '0' : '2');
            lines1[1].push_back(board[p.x][i] == role ? '2' : board[p.x][i] == 0 ? '0' : '1');
        } else {
            lines[1].push_back('1');
            lines1[1].push_back('1');
        }
    }

    /// zuo shang
    for (i = p.x - min(min(p.x, p.y), 5), j = p.y - min(min(p.x, p.y), 5);
         i < (int) min(GRID_NUM, p.x + 6) && j < (int) min(GRID_NUM, p.y + 6); i++, j++) {
        if (i != p.x) {
            lines[2].push_back(board[i][j] == role ? '1' : board[i][j] == 0 ? '0' : '2');
            lines1[2].push_back(board[i][j] == role ? '2' : board[i][j] == 0 ? '0' : '1');
        } else {
            lines[2].push_back('1');
            lines1[2].push_back('1');
        }
    }

    /// you shang
    for (i = p.x + min(min(p.y, GRID_NUM - 1 - p.x), 5), j = p.y - min(min(p.y, GRID_NUM - 1 - p.x), 5);
         i >= (int) max(0, p.x - 5) && j < (int) min(GRID_NUM, p.y + 6); i--, j++) {
        if (i != p.x) {
            lines[3].push_back(board[i][j] == role ? '1' : board[i][j] == 0 ? '0' : '2');
            lines1[3].push_back(board[i][j] == role ? '2' : board[i][j] == 0 ? '0' : '1');
        } else {
            lines[3].push_back('1');
            lines1[3].push_back('1');
        }
    }


    for (i = 0; i < 4; i++) {
        vector<int> tmp = AI::searcher.ACSearch(lines[i]);
        for (j = 0; j < int(tmp.size()); j++) {
            result += patterns[tmp[j]].score;
        }

        tmp = AI::searcher.ACSearch(lines1[i]);
        for (j = 0; j < int(tmp.size()); j++) {
            result += patterns[tmp[j]].score;
        }
    }

    return result;
}

int AI::evaluate(char board[GRID_NUM][GRID_NUM], Piece role) {
    if (role == COMPUTER) {
        return AI::allScores[1];
    } else if (role == HUMAN) {
        return AI::allScores[0];
    } else {
        cout << "error" << "evaluate() " << endl;
        return 0;
    }
}

void AI::updateScore(char board[GRID_NUM][GRID_NUM], const Point p) {
    string lines[4];
    string lines1[4];
    int i, j;
    int role = HUMAN;

    //竖
    for (i = 0; i < GRID_NUM; i++) {
        lines[0].push_back(board[i][p.y] == role ? '1' : board[i][p.y] == 0 ? '0' : '2');
        lines1[0].push_back(board[i][p.y] == role ? '2' : board[i][p.y] == 0 ? '0' : '1');
    }
    //横
    for (i = 0; i < GRID_NUM; i++) {
        lines[1].push_back(board[p.x][i] == role ? '1' : board[p.x][i] == 0 ? '0' : '2');
        lines1[1].push_back(board[p.x][i] == role ? '2' : board[p.x][i] == 0 ? '0' : '1');
    }
    //反斜杠
    for (i = p.x - min(p.x, p.y), j = p.y - min(p.x, p.y); i < GRID_NUM && j < GRID_NUM; i++, j++) {
        lines[2].push_back(board[i][j] == role ? '1' : board[i][j] == 0 ? '0' : '2');
        lines1[2].push_back(board[i][j] == role ? '2' : board[i][j] == 0 ? '0' : '1');
    }
    //斜杠
    for (i = p.x + min(p.y, GRID_NUM - 1 - p.x), j = p.y - min(p.y, GRID_NUM - 1 - p.x); i != 0 && j < GRID_NUM;
         i--, j++) {
        lines[3].push_back(board[i][j] == role ? '1' : board[i][j] == 0 ? '0' : '2');
        lines1[3].push_back(board[i][j] == role ? '2' : board[i][j] == 0 ? '0' : '1');
    }

    int lineScore[4];
    int line1Score[4];
    memset(lineScore, 0, sizeof(lineScore));
    memset(line1Score, 0, sizeof(line1Score));

    /// 算分
    for (i = 0; i < 4; ++i) {
        vector<int> result = AI::searcher.ACSearch(lines[i]);
        for (j = 0; j < int(result.size()); ++j) {
            lineScore[i] += patterns[result[j]].score;
        }

        result = AI::searcher.ACSearch(lines1[i]);
        for (j = 0; j < int(result.size()); ++j) {
            line1Score[i] += patterns[result[j]].score;
        }
    }

    ///  算x, y点在横竖撇捺中的ID
    int a = p.y, b = GRID_NUM + p.x;
    int c = 2 * GRID_NUM + (p.y - p.x + 10);
    int d = 2 * GRID_NUM + (p.x + p.y - 4);

    /// 减去原来的
    for (i = 0; i < 2; ++i) {
        allScores[i] -= scores[i][a];
        allScores[i] -= scores[i][b];
    }

    scores[0][a] = lineScore[0]; // 竖
    scores[1][a] = line1Score[0]; // 竖
    scores[0][b] = lineScore[1]; // 横
    scores[1][b] = line1Score[1]; // 横

    /// 加上下了这一子后的
    for (i = 0; i < 2; ++i) {
        AI::allScores[i] += scores[i][a];
        AI::allScores[i] += scores[i][b];
    }

    ///  ↖
    /// 能成5的
    if (p.y - p.x >= -10 && p.y - p.x <= 10) {
        for (i = 0; i < 2; ++i)
            AI::allScores[i] -= scores[i][c];


        scores[0][c] = lineScore[2];
        scores[1][c] = line1Score[2];

        for (i = 0; i < 2; i++)
            AI::allScores[i] += scores[i][c];
    }

    /// ↗
    if (p.x + p.y >= 4 && p.x + p.y <= 24) {
        for (i = 0; i < 2; i++)
            AI::allScores[i] -= scores[i][d];

        scores[0][d] = lineScore[3];
        scores[1][d] = line1Score[3];

        for (i = 0; i < 2; i++)
            AI::allScores[i] += scores[i][d];
    }
}

// ab剪枝的主函数
int AI::alphabetaSearch(char board[GRID_NUM][GRID_NUM], int depth, int alpha, int beta,
                        Piece currentSearchRole) {
    HashItem::Flag flag = HashItem::ALPHA;
    int score = getHashItemScore(depth, alpha, beta);
    if (score != UNKNOWN_SCORE && depth != DEPTH)
        return score;

    int score1 = AI::evaluate(board, currentSearchRole);
    int score2 = AI::evaluate(board, currentSearchRole == HUMAN ? COMPUTER : HUMAN);

    if (score1 >= 50000)
        return MAX_SCORE - 1000 - (DEPTH - depth);
    if (score2 >= 50000)
        return MIN_SCORE + 1000 + (DEPTH - depth);

    if (depth == 0) {
        recordHashItem(depth, score1 - score2, HashItem::EXACT);
        return score1 - score2;
    }

    int count = 0;
    set<Point> possiblePositions;
    /// TODO getCurrentPossiblePositions写
    const set<Point> &tmpPossiblePositions = ppm.getCurrentPossiblePositions();

    for (auto item: tmpPossiblePositions) {
        possiblePositions.insert(Point(item.x, item.y, evaluatePoint(board, item)));
    }

    while (!possiblePositions.empty()) {
        Point p = *possiblePositions.begin();
        possiblePositions.erase(p);

        board[p.x][p.y] = currentSearchRole;
        /// TODO 取^
        AI::currentZobristValue ^= boardZobristValue[currentSearchRole - 1][p.x][p.y];
        updateScore(board, p);

        /// 加可能出现的位置
        p.score = 0;
        /// TODO
        ppm.AddPossiblePositions(board, p);

        int val = -AI::alphabetaSearch(board, depth - 1, -beta, -alpha,
                                       currentSearchRole == HUMAN ? COMPUTER : HUMAN);

        /// back
        ppm.Rollback();

        /// 取消放置
        board[p.x][p.y] = 0;
        currentZobristValue ^= boardZobristValue[currentSearchRole - 1][p.x][p.y];
        updateScore(board, p);

        /// 剪枝与记忆
        if (val >= beta) {
            recordHashItem(depth, beta, HashItem::BETA);
            return beta;
        }
        if (val > alpha) {
            flag = HashItem::EXACT;
            alpha = val;
            if (depth == DEPTH) {
                searchResult = p;
            }
        }

        count++;
        if (count >= 9) {
            break;
        }
    }

    recordHashItem(depth, alpha, flag);
    return alpha;
}

Point AI::getGoodMove(char board[GRID_NUM][GRID_NUM]) {
    int score = AI::alphabetaSearch(board, DEPTH, MIN_SCORE, MAX_SCORE, COMPUTER);
    if (score >= MAX_SCORE - 1000 - 1)
        AI::winner = COMPUTER;
    else if (score <= MIN_SCORE + 1000 + 1)
        AI::winner = HUMAN;

    return AI::searchResult;
}

void AI::firstStep() {
    AI::currentZobristValue ^= boardZobristValue[COMPUTER - 1][7][7];
    AI::board[7][7] = COMPUTER;
    updateScore(board, Point(7, 7));

    Point p(7, 7);
    AI::moves.push(p);
    searchResult = p;

    ppm.AddPossiblePositions(board, p);
}

AI::AI() {
    memset(AI::chessBoard, Piece::EMPTY, sizeof(AI::chessBoard));
    memset(scores, 0, sizeof(scores));

    AI::searcher.LoadPattern(patterns);
    AI::searcher.BuildGotoTable();
    AI::searcher.BuildFailTable();

    randomBoardZobristValue();
    currentZobristValue = random64();
}

//输出棋盘
void printBoard(char board[GRID_NUM][GRID_NUM]) {
    int i, j;
    for (i = 0; i < GRID_NUM; i++) {
        for (j = 0; j < GRID_NUM; j++) {
            cout << (int) board[i][j] << " ";
        }
        cout << endl;
    }
}

//取得刚才电脑下得那一步棋子的位置
Point AI::getLastPosition() {
    return AI::searchResult;
}

//人类下棋，返回棋盘，传给界面
string AI::nextStep(int x, int y) {
    AI::moves.push(Point(x, y));

    AI::board[x][y] = HUMAN;
    AI::currentZobristValue ^= AI::boardZobristValue[HUMAN - 1][x][y];
    AI::updateScore(board, Point(x, y));

    //增加可能出现的位置
    ppm.AddPossiblePositions(board, Point(x, y));

    Point result = AI::getGoodMove(board);

    board[result.x][result.y] = COMPUTER;
    currentZobristValue ^= boardZobristValue[COMPUTER - 1][result.x][result.y];
    AI::updateScore(board, result);

    //增加可能出现的位置
    ppm.AddPossiblePositions(board, result);

    //若双方还未决出胜负，则把棋子位置加入到历史记录中
    if (winner == -1)
        moves.push(Point(result.x, result.y));

    // string resultStr;
    // int i, j;
    // for (i = 0; i < GRID_NUM; i++) {
    //     for (j = 0; j < GRID_NUM; j++) {
    //         resultStr.push_back(board[i][j] + 48);
    //     }
    // }

    /// 这里打印出下完后的结果 TODO
    //printBoard(board);
    //cout << "resultSTR" << endl << resultStr << endl;

    return "";
}


/// TODO 判断禁手


void TEST_TERMINAL() {
    Piece role = COMPUTER;
    bool isFirst = true;
    AI *ai = new AI();
    int humanx, humany;
    while (true) {
        if (isFirst && role == COMPUTER) {
            /// 电脑先手
            ai->firstStep();
            isFirst = false;
        } else {
            cin >> humanx >> humany;
            string s = ai->nextStep(humanx, humany);
            //cout << "AI: " <<ai->getLastPosition().x << " " << ai->getLastPosition().y << endl;
        }
    }
}

int main() {
    bool isFirst = true;
    const int ST = 0;
    AI *ai = new AI();

    // 读入 JSON

    string str;
    Json::Reader reader;
    Json::Value input;

    Json::FastWriter writer;
    // 输出决策 JSON
    Json::Value ret;
    Json::Value action;

    while (true) {
        // 读入 JSON
        getline(cin, str);
        reader.parse(str, input);



        //cout << writer.write(input) << endl;
         // 输出决策 JSON


         Point p;

         if (isFirst) {
             isFirst = false;
             if (input["requests"][ST]["x"].asInt() < 0) {
                 ai->firstStep();
                 action["x"] = 7;
                 action["y"] = 7;
             } else {
                ai->nextStep(input["requests"][ST]["x"].asInt(), input["requests"][ST]["y"].asInt());
                p = ai->getLastPosition();
                action["x"] = p.x;
                action["y"] = p.y;

             }
         } else {
             ai->nextStep(input["x"].asInt(), input["y"].asInt());
             p = ai->getLastPosition();
             action["x"] = p.x;
             action["y"] = p.y;

         }
         ret["response"] = action;
         cout << writer.write(ret) << endl;
         cout << ">>>BOTZONE_REQUEST_KEEP_RUNNING<<<" << endl;
    }
    return 0;
}