#include "chessai.h"



/// TODO 改成FALSE
constexpr bool DEBUG = false;
///////////////////////////////////////////////////////////////////////////////////////
/// 有关的常量
///////////////////////////////////////////////////////////////////////////////////////

constexpr int MAX_SCORE = 10000000; ///
constexpr int MIN_SCORE = -10000000;
#define HASH_ITEM_INDEX_MASK (0xffff)
constexpr int UNKNOWN_SCORE = 10000001;

constexpr int DEPTH = 4; /// 搜索深度

/// 旗型的评分函数 TODO 优化
const vector<Pattern> patterns = { /// not const safe
    {"11111", 50000}, /// 成五
    {"011110", 4320}, /// 活4
    {"011100", 720}, /// 活3
    {"001110", 720}, /// 活3
    {"011010", 720}, /// huo 3
    {"010110", 720}, /// huo 3
    {"11110", 720},/// si 4
    {"01111", 720},/// si 4
    {"11011", 720},/// si 4
    {"10111", 720},/// si 4
    {"11101", 720},/// si 4
    {"001100", 120}, /// huo2
    {"001010", 120}, /// huo2
    {"010100", 120}, /// huo2
    {"000100", 20},  /// 1
    {"001000", 20},  /// huo 1
};

extern int FIRST_ROLE = HUMAN; /// 存第一手棋是谁，即谁是黑子

Point::Point(const int x, const int y) : x(x), y(y) { score = 0; }

Point::Point(const int x, const int y, const Role curRule, const int score) : x(x), y(y), score(score) {
    color = (FIRST_ROLE == HUMAN ? (curRule == HUMAN ? Black : White) : (curRule == HUMAN ? White : Black));
}

/// 比较，用来在set中排序
bool Point::operator<(const Point &other) const {
    return (score > other.score) || (score == other.score && (x < other.x || (x == other.x && y < other.y)));
}


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

const set<Point> &PossiblePointManager::getCurrentPossiblePoints() { return currentPossiblePositions; }

void PossiblePointManager::RemoveAll() {
    currentPossiblePositions.clear();
    history.clear();
}



///////////////////////////////////////////////////////////////////////////////////////
/// AC自动机
///////////////////////////////////////////////////////////////////////////////////////

ACAutm::ACAutm() {
    /// TODO
    if (DEBUG == true) {
        cout << "initAC" << endl;
    }
    memset(times, 0, sizeof(times));
    memset(fail, 0, sizeof(fail));
    memset(visited, 0, sizeof(visited));
    memset(Ends, 0, sizeof(Ends));
    memset(head, 0, sizeof(head));
    memset(tree, 0, sizeof(tree));
    memset(box, 0, sizeof(box));
}

void ACAutm::insert(const int i, const string str) {
    int u = 0;
    for (int j = 0, len = str.length(); j < len; ++j) {
        /// !!!!
        const int id = int(str[j] - '0');

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

/// TODO
vector<int> ACAutm::ACSearch(const string &text) {
    vector<int> result(17, 0);
    memset(times, 0, sizeof(times));


    for (int u = 0, i = 0, len = text.length(); i < len; ++i) {
        int id = int(text[i] - '0');

        u = tree[u][id];

        ACAutm::times[u]++;
    }


    ACAutm::runBFS(0);

    /// TODO 要不要改成常量
    for (int i = 0; i < 16; ++i) {
        result[i] = times[Ends[i + 1]];
    }

    return result;
}

void ACAutm::addEdge(const int u, const int v) {
    Next[++edge] = head[u];
    head[u] = edge;
    to[edge] = v;
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
            for (int i = head[cur]; i > 0; i = Next[i]) {
                box[r++] = to[i];
            }
        } else {
            r--;
            for (int i = head[cur]; i > 0; i = Next[i]) {
                times[cur] += times[to[i]];
            }
        }
    }
}



///////////////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////////////

namespace ChessAI {



 long long boardZobristValue[2][GRID_NUM][GRID_NUM];
 long long currentZobristValue;
 HashItem hashItems[HASH_ITEM_INDEX_MASK];
 char board[GRID_NUM][GRID_NUM];
 int winner;

 stack<Point> moves; /// 历史记录
 int scores[2][72]; /// 15 + 15 + 21 + 21
 int allScores[2];

 ACAutm acseacher;
 PossiblePointManager ppm;

/// the position of the result of alpha-beta search,the point AI want to move
 Point searchResult;

/**
     * 记录当前局面的置换表
     * @param depth
     * @param score
     * @param flag
     */
void recordHashItem(const int depth, const int score, const HashItem::Flag flag) {
    int index = (int) (currentZobristValue & HASH_ITEM_INDEX_MASK);

    HashItem *phashItem = &hashItems[index];

    /// 已经有了
    if (phashItem->flag != HashItem::EMPTY && phashItem->depth > depth) {
        return;
    }

    phashItem->checksum = currentZobristValue;
    phashItem->score = score;
    phashItem->flag = flag;
    phashItem->depth = depth;
}

/**
     * 查表取分
     * @param depth
     * @param alpha
     * @param beta
     * @return
     */
int getHashItemScore(const int depth, const int alpha, const int beta) {
    int index = (int) (currentZobristValue & HASH_ITEM_INDEX_MASK);
    HashItem *phashItem = &hashItems[index];

    if (phashItem->flag == HashItem::EMPTY)
        return UNKNOWN_SCORE;

    if (phashItem->checksum == currentZobristValue) {
        if (phashItem->depth >= depth) {
            if (phashItem->flag == HashItem::EXACT) {
                return phashItem->score;
            }
            if (phashItem->flag == HashItem::ALPHA && phashItem->score <= alpha) {
                return alpha;
            }
            if (phashItem->flag == HashItem::BETA && phashItem->score >= beta) {
                return beta;
            }
        }
    }

    return UNKNOWN_SCORE;
}

/**
     * 生成64位随机数
     * @return 一个64位的随机数
     */
long long random64() {
    return (long long) rand() | ((long long) rand() << 15) | ((long long) rand() << 30) |
           ((long long) rand() << 45) | ((long long) rand() << 60);
}

/**
     * @brief produce every point`s hashKey in the bord
     */
void randomBoardHashValue() {
    int i, j, k;
    for (i = 0; i < 2; ++i) {
        for (j = 0; j < GRID_NUM; ++j) {
            for (k = 0; k < GRID_NUM; ++k) {
                boardZobristValue[i][j][k] = random64();
            }
        }
    }
}

/**
     * 初始化置换表的权值
     */
void initCurrentZobristValue() { currentZobristValue = random64(); }


////////////////////////////////////////////////////////////////////////////////

/**
 * 根据位置对棋盘board 评分
 * 忽视禁手的情况下，黑白评估近似相同，先写TODO
 * 现住想办法加上禁手TODO
 * 如果不是人类方的话把line1 \ line 的地位交换
 * @param board 当前棋盘
 * @param p 位置
 */
int evaluatePoint(char board[GRID_NUM][GRID_NUM], Point p) {
    int role = HUMAN, result = 0;

    unsigned int i, j;

    ///
    /// attack
    string lines[4];
    /// defend
    string lines1[4];
    /// none
    string lines2[4];

    for (i = max(0, p.x - 5); i < (unsigned int) min(GRID_NUM, p.x + 6); i++) {
        if (i != (unsigned int) p.x) {
            lines[0].push_back(board[i][p.y] == role ? '1' : (board[i][p.y] == 0 ? '0' : '2'));
            lines1[0].push_back(board[i][p.y] == role ? '2' : (board[i][p.y] == 0 ? '0' : '1'));
        } else {
            /// TODO
            lines[0].push_back('1');
            lines1[0].push_back('1');
        }
        /// TODO
        lines2[0].push_back(board[i][p.y] == role ? '1' : (board[i][p.y] == 0 ? '0' : '2'));
    }
    for (i = max(0, p.y - 5); i < (unsigned int) min(GRID_NUM, p.y + 6); i++) {
        if (i != (unsigned int) p.y) { ///// !
            lines[1].push_back(board[p.x][i] == role ? '1' : (board[p.x][i] == 0 ? '0' : '2'));
            lines1[1].push_back(board[p.x][i] == role ? '2' : (board[p.x][i] == 0 ? '0' : '1'));
        } else {
            lines[1].push_back('1');
            lines1[1].push_back('1');
        }
    }
    for (i = p.x - min(min(p.x, p.y), 5), j = p.y - min(min(p.x, p.y), 5);
         i < (unsigned int) min(GRID_NUM, p.x + 6) && j < (unsigned int) min(GRID_NUM, p.y + 6); i++, j++) {
        if (i != (unsigned int) p.x) {
            lines[2].push_back(board[i][j] == role ? '1' : (board[i][j] == 0 ? '0' : '2'));
            lines1[2].push_back(board[i][j] == role ? '2' : (board[i][j] == 0 ? '0' : '1'));
        } else {
            lines[2].push_back('1');
            lines1[2].push_back('1');
        }
    }
    /// 这里用了unsigned int 溢出后变成的大数!!!
    for (i = p.x + min(min(p.y, GRID_NUM - 1 - p.x), 5), j = p.y - min(min(p.y, GRID_NUM - 1 - p.x), 5);
         i >= (unsigned int) max(0, p.x - 5) && i < 1000 && j < (unsigned int) min(GRID_NUM, p.y + 6); i--, j++) {
        if (i != (unsigned int) p.x) {
            lines[3].push_back(board[i][j] == role ? '1' : (board[i][j] == 0 ? '0' : '2'));
            lines1[3].push_back(board[i][j] == role ? '2' : (board[i][j] == 0 ? '0' : '1'));
        } else {
            lines[3].push_back('1');
            lines1[3].push_back('1');
        }
    }


    for (i = 0; i < 4; i++) {
        vector<int> tmp = acseacher.ACSearch(lines[i]);
        for (j = 0; j < tmp.size(); j++) {
            result += tmp[j] * patterns[j].score;
        }


        tmp = acseacher.ACSearch(lines1[i]);
        for (j = 0; j < tmp.size(); j++) {
            result += tmp[j] * patterns[j].score;
        }
    }

    return result;
}

/**
     * 对局面估计分数
     * @param board
     * @param role
     * @return
     */
int evaluate(char board[GRID_NUM][GRID_NUM], Role role) {
    if (role == HUMAN) {
        return allScores[0];
    } else if (role == COMPUTER) {
        return allScores[1];
    }
    if (DEBUG == true) {
        cout << "error: invalid role in Func evaluate" << endl;
    }

    return 0;
}

/**
 * 更新人类下棋后目前棋局下得分，根据算出某点的得分和历史总得分的记录,
 * TODO
 * @param board 1 ME; 2 HIM; 0 EMPTY
 * @param p
 */
void updateScore(char board[GRID_NUM][GRID_NUM], Point p) {
    string lines[4]; /// HUMAN
    string lines1[4]; /// COMPUTER
    unsigned int i, j;
    int role = HUMAN;

    /// 竖
    for (i = 0; i < GRID_NUM; i++) {
        lines[0].push_back(board[i][p.y] == role ? '1' : board[i][p.y] == 0 ? '0' : '2');
        lines1[0].push_back(board[i][p.y] == role ? '2' : board[i][p.y] == 0 ? '0' : '1');
    }
    /// 横
    for (i = 0; i < GRID_NUM; i++) {
        lines[1].push_back(board[p.x][i] == role ? '1' : board[p.x][i] == 0 ? '0' : '2');
        lines1[1].push_back(board[p.x][i] == role ? '2' : board[p.x][i] == 0 ? '0' : '1');
    }
    /// 反斜杠
    for (i = p.x - min(p.x, p.y), j = p.y - min(p.x, p.y); i < GRID_NUM && j < GRID_NUM; i++, j++) {
        lines[2].push_back(board[i][j] == role ? '1' : board[i][j] == 0 ? '0' : '2');
        lines1[2].push_back(board[i][j] == role ? '2' : board[i][j] == 0 ? '0' : '1');
    }
    /// 斜杠
    /// 同样，用到了insigned int的溢出
    for (i = p.x + min(p.y, GRID_NUM - 1 - p.x), j = p.y - min(p.y, GRID_NUM - 1 - p.x);
         i >= 0 && i < 1000 && j < GRID_NUM; i--, j++) {
        lines[3].push_back(board[i][j] == role ? '1' : board[i][j] == 0 ? '0' : '2');
        lines1[3].push_back(board[i][j] == role ? '2' : board[i][j] == 0 ? '0' : '1');
    }

    int lineScore[4];
    int line1Score[4];
    memset(lineScore, 0, sizeof(lineScore));
    memset(line1Score, 0, sizeof(line1Score));

    /// estimate the point score
    for (i = 0; i < 4; i++) {
        /// attack
        vector<int> result = acseacher.ACSearch(lines[i]);
        for (j = 0; j < result.size(); j++) {
            lineScore[i] += result[j] * patterns[j].score;
        }

        /// defend
        result = acseacher.ACSearch(lines1[i]);
        for (j = 0; j < result.size(); j++) {
            line1Score[i] += result[j] * patterns[j].score;
        }
    }

    /// update the whole board score
    int a = p.y;
    int b = GRID_NUM + p.x;
    int c = 2 * GRID_NUM + (p.y - p.x + 10);
    int d = 2 * GRID_NUM + 21 + (p.x + p.y - 4);

    /// 减去以前的
    for (i = 0; i < 2; i++) {
        allScores[i] -= scores[i][a];
        allScores[i] -= scores[i][b];
    }

    /// 竖、横
    scores[0][a] = lineScore[0];
    scores[1][a] = line1Score[0];
    scores[0][b] = lineScore[1];
    scores[1][b] = line1Score[1];

    /// 加上新的
    for (i = 0; i < 2; i++) {
        allScores[i] += scores[i][a];
        allScores[i] += scores[i][b];
    }

    if (p.y - p.x >= -10 && p.y - p.x <= 10) {
        /// 减
        for (i = 0; i < 2; i++)
            allScores[i] -= scores[i][c];


        scores[0][c] = lineScore[2];
        scores[1][c] = line1Score[2];

        /// 加
        for (i = 0; i < 2; i++)
            allScores[i] += scores[i][c];
    }

    if (p.x + p.y >= 4 && p.x + p.y <= 24) {
        for (i = 0; i < 2; i++)
            allScores[i] -= scores[i][d];

        scores[0][d] = lineScore[3];
        scores[1][d] = line1Score[3];

        for (i = 0; i < 2; i++)
            allScores[i] += scores[i][d];
    }
}

/**
     * alpha-beta 剪枝
     * @param board 目前的棋局
     * @param depth 目前的深度,倒着走
     * @param alpha a值
     * @param beta  b值
     * @param currentSearchRole 现在以什么角色进行评分
     * @return 返回分数
     */
int alphaBetaSearch(char board[GRID_NUM][GRID_NUM], int depth, int alpha, int beta, Role currentSearchRole) {
    /// 置换表 TODO
    HashItem::Flag flag = HashItem::ALPHA;
    int score = getHashItemScore(depth, alpha, beta);
    if (score != UNKNOWN_SCORE && depth != DEPTH) {
        return score;
    }

    /// 评分-点
    int score1 = evaluate(board, currentSearchRole);
    int score2 = evaluate(board, currentSearchRole == HUMAN ? COMPUTER : HUMAN);

    if (score1 >= 50000) {
        /// 不然后面赢也被算赢了
        /// 直接赢赢赢
        return MAX_SCORE - 1000 - (DEPTH - depth);
    }
    if (score2 >= 50000) {
        /// 已经输麻了
        return MIN_SCORE + 1000 + (DEPTH - depth);
    }


    if (depth == 0) {
        /// record
        recordHashItem(depth, score1 - score2, HashItem::EXACT);
        return score1 - score2;
    }

    /// 之后的可能情况
    set<Point> possiblePositions;
    const set<Point> &tmpPossiblePositions = ppm.getCurrentPossiblePoints();

    /// 粗略评分当前选中的点
    for (auto item: tmpPossiblePositions) {
        possiblePositions.insert(Point(item.x, item.y, currentSearchRole, evaluatePoint(board, item)));
    }

    /// 搜索主要部分
    int count = 0;
    while (!possiblePositions.empty()) {
        Point p = *possiblePositions.begin();

        possiblePositions.erase(possiblePositions.begin());

        /// put chess
        board[p.x][p.y] = currentSearchRole;
        currentZobristValue ^= boardZobristValue[currentSearchRole - 1][p.x][p.y];
        updateScore(board, p);

        /// 增加可能出现的位置
        p.score = 0;
        ppm.AddPossiblePoints(board, p);

        int val = -alphaBetaSearch(board, depth - 1, -beta, -alpha, currentSearchRole == HUMAN ? COMPUTER : HUMAN);
        if (depth == DEPTH) {
            if (DEBUG == true) {
                cout << "score(" << p.x << "," << p.y << "):" << val << endl;
            }
        }
        /// backtracing
        ppm.Rollback();
        board[p.x][p.y] = 0;
        currentZobristValue ^= boardZobristValue[currentSearchRole - 1][p.x][p.y];
        updateScore(board, p);

        /// ab剪枝
        if (val >= beta) {
            recordHashItem(depth, beta, HashItem::BETA);
            return beta;
        }
        if (val > alpha) {
            flag = HashItem::EXACT;
            alpha = val;
            if (depth == DEPTH) {
                /// 更新搜索到的
                searchResult = p;
            }
        }

        /// 减少搜索，卡时间 TODO 改成什么更好
        count++;
        if (count >= 9) {
            break;
        }
    }

    /// TODO
    recordHashItem(depth, alpha, flag);
    return alpha;
}

/**
     * @brief 接口;进行ab搜索，判断是否出现五子连珠（max/min）;调用，返回最佳点
     * @param board 当前棋局
     * @return 下一步走法
     */
Point getAGoodMove(char board[GRID_NUM][GRID_NUM]) {
    int score = alphaBetaSearch(board, DEPTH, MIN_SCORE, MAX_SCORE, COMPUTER);
    if (score >= MAX_SCORE - 1000 - 1) {
        winner = COMPUTER;
    } else if (score <= MIN_SCORE + 1000 + 1) {
        winner = HUMAN;
    }
    return searchResult;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
     * @brief init:  acsearcher, hashboard
     */
void init() {
    acseacher.LoadPattern(patterns);
    randomBoardHashValue();
    currentZobristValue = random64();
}

/**
     * @brief init the whole AI part
     */
void beforeStart() {
    /// TODO
    if (DEBUG == true) {
        cout << "beforeStart" << endl;
    }
    memset(board, Role::EMPTY, sizeof(board));
    memset(scores, 0, sizeof(scores));
    init();
}

/**
     * 返回胜利者
     * @return  0 : human win
     *          1 : computer win
     *         -1 : not have winner
     */
int isSomeOneWin() {
    if (winner == HUMAN) {
        return 0;
    } else if (winner == COMPUTER) {
        return 1;
    } else {
        return -1;
    }
}

/**
 * 悔棋, 只能是人悔棋
 */
void takeBack() {
    if (moves.size() < 2) {
        cout << "error : just 2 or less than tow moves, cannot rollback" <<endl;
    }

    Point previousPosition = moves.top();
    moves.pop();
    currentZobristValue ^= boardZobristValue[COMPUTER - 1][previousPosition.x][previousPosition.y];
    board[previousPosition.x][previousPosition.y] = EMPTY;
    updateScore(board, previousPosition);

    previousPosition = moves.top();
    moves.pop();
    currentZobristValue ^= boardZobristValue[HUMAN - 1][previousPosition.x][previousPosition.y];
    board[previousPosition.x][previousPosition.y] = EMPTY;
    updateScore(board, previousPosition);

    /// 把电脑的棋也悔掉
    ppm.Rollback();
    ppm.Rollback();

    winner = -1;
}

/**
     * 初始化AI的工作
     * 清楚记录，重新开局
     * @param role 谁是黑手 1 电脑黑手，0电脑白色
     */
void reset(int role) {

    memset(scores, 0, sizeof(scores));
    memset(allScores, 0, sizeof(allScores));

    winner = -1;
    // 初始化局面总分数为0

    while (!moves.empty()) {
        moves.pop();
    }

    /// hash
    int i;
    for (i = 0; i < HASH_ITEM_INDEX_MASK + 1; i++) {
        hashItems[i].flag = HashItem::EMPTY;
    }

    // INIT BOARD
    memset(board, EMPTY, GRID_NUM * GRID_NUM * sizeof(char));

    /// ppm
    ppm.RemoveAll();

    // 用户先走
    if (role == 0) {
        // do nothing
        FIRST_ROLE = HUMAN;
    }
    // 电脑先走
    else if (role == 1) {
        FIRST_ROLE = COMPUTER;

        /// tianyuan
        currentZobristValue ^= boardZobristValue[COMPUTER - 1][7][7];
        board[7][7] = COMPUTER;
        updateScore(board, Point(7, 7));

        moves.push(Point(7, 7));
        ///  记得改serachresult
        searchResult = Point(7, 7);

        ppm.AddPossiblePoints(board, Point(7, 7));
    }

    winner = -1;
}


/**
     * @return 返回电脑下的那一步的棋子的位置
     */
Point getLastPoint() { return ChessAI::searchResult; }

/**
     * 人类下棋
     * @param x
     * @param y
     */
void nextStep(const int x, const int y) {

    moves.push(Point(x, y));

    board[x][y] = HUMAN;
    currentZobristValue ^= boardZobristValue[HUMAN - 1][x][y];
    updateScore(board, Point(x, y));

    /// 增加可能出现的位置
    ppm.AddPossiblePoints(board, Point(x, y));

    Point result = getAGoodMove(board);

    board[result.x][result.y] = COMPUTER;
    currentZobristValue ^= boardZobristValue[COMPUTER - 1][result.x][result.y];
    updateScore(board, result);

    /// 增加可能出现的位置
    ppm.AddPossiblePoints(board, result);

    /// 还未决出胜负，加入到历史记录中
    if (winner == -1)
        moves.push(Point(result.x, result.y));
}
};



/**
 * 输入人下的位置，返回AI 下的位置
 * @param x 人x
 * @param y 人y
 * @return AI下的位置
 */
Point calculate(const int x, const int y) {
    ChessAI::nextStep(x, y);
    return ChessAI::getLastPoint();
}


void testACAutm() {
    ACAutm ac;
    ac.LoadPattern(patterns);
    vector<int> res = ac.ACSearch("111112011110");
    for (int i = 0; i < patterns.size(); ++i) {
        cout << patterns[i].type << " : " << res[i] << endl;
    }

}


