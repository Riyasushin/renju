#include "game.h"


std::string GAMEFILESUF = ".data";

/*
 * 1.三三禁手 黑方一子落下同时形成两个或两个以上的活三(或嵌四),此步为三三三禁手。注意:这里一定要 两个都是"活"三才能算。
 * 2.四四禁手 黑方一子落下同时形成两个或两个以上的四,活四、冲四、嵌五之四四,包括在此四之内。此步为四四禁手。注意:只要是两个"四"即为禁手,无论是哪种四,活四,跳四,冲四都算。
 * 3.四三三禁手 黑方一步使一个四,两个活三同时形成。
 * 4.四四三禁手 黑方一步使两个四,一个活三同时形成。
 * 5.长连禁手 黑方一子落下形成连续六子或六子以上相连。注意:白棋出现长连与连五同等作用,即白棋出现长连也将获胜。
 * 禁手规定:
 * 1. 黑棋禁手判负、白棋无禁手。黑棋禁手有"三、三"、"四、四"和"长连",包括"四、三、三" 和"四、四、三",即黑棋只能以"四、三"取胜; 2.五连与禁手同时形成,判胜;
 */

/// 旗型的评分函数 TODO 优化
const vector<Pattern> AC_patterns = { /// not const safe
    {"11111", 50000}, /// 成五     0
    {"011110", 4320}, /// 活4      1
    {"011100", 720}, /// 活3       2
    {"001110", 720}, /// 活3       3
    {"011010", 720}, /// huo 3     4
    {"010110", 720}, /// huo 3     5
    {"11110", 720},/// si 4        6
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

game::game() {

    ac.LoadPattern(AC_patterns);
    restart();
}



bool game::canMakeMove(const size_t x, const size_t y) const {
    return  !game::vised(x, y);
}


void game::MakeMoveHelper(const size_t x, const size_t y) {
    // 已经打完，禁止下棋
    if (hasDecideWinner())
        return;

    game::Chessquer qiqi = (game::state == game::whiteToD ? game::white : game::black);

    game::MakeMove(x, y, qiqi);
}

void game::MakeMove(const size_t x, const size_t y, const game::Chessquer color) {



    history.push_back({x, y, color, PUSHINGARBAGE});
    size_t len = history.size();

    game::setHasVised(x, y, color);

    this->state = (color == white ? blackToD : whiteToD);

    /// 不用扫描一遍矩阵了，拿空间换时间
    for (size_t i = 0; i< len - 1; ++i) {
        if (history[i].color != history[len - 1].color)
            continue;
        int x1 = history[i].x, y1 = history[i].y;
        int dx = x1 - x, dy = y1 - y;

        if (dx == 1 && dy == 0) {
            /// YOU
            history[len-1].allDirections[0] = i;
            history[i].allDirections[7] = len - 1;
        }  else if (dx == -1 && dy == 0) {
            /// ZUO
            history[len-1].allDirections[7] = i;
            history[i].allDirections[0] = len - 1;
        }  else if (dx == 0 && dy == 1) {
            /// xia
            history[len-1].allDirections[1] = i;
            history[i].allDirections[6] = len - 1;
        }  else if (dx == 0 && dy == -1) {
            /// shang
            history[len-1].allDirections[6] = i;
            history[i].allDirections[1] = len - 1;
        }  else if (dx == 1 && dy == 1) {
            /// youxiaq
            history[len-1].allDirections[2] = i;
            history[i].allDirections[5] = len - 1;
        }  else if (dx == 1 && dy == -1) {
            /// youshang
            history[len-1].allDirections[3] = i;
            history[i].allDirections[4] = len - 1;
        }  else if (dx == -1 && dy == 1) {
            /// zuoixia
            history[len-1].allDirections[4] = i;
            history[i].allDirections[3] = len - 1;
        }  else if (dx == -1 && dy == -1) {
            /// zuoshang
            history[len-1].allDirections[5] = i;
            history[i].allDirections[2] = len - 1;
        }
    }

    if (color == black && reachInValid(x, y)) {
        this->state = whiteWin;
        return;
    }

    if ( tryGetWinner(len - 1, color)){
        /// 判断输赢
        state = (color == white ? whiteWin : blackWin);
    }
    if ( !hasDecideWinner() && len == 225) {
        /// 平局
        state = tie;
    }

}

game::Chessquer game::Winner() {
    if (state == whiteWin) {
        return white;
    } else if (state == blackWin) {
        return black;
    } else {
        return non;
    }

}

bool game::hasDecideWinner() const {
    return !(state == whiteToD || state == blackToD);
}

void game::huiqi() {
    if (hasDecideWinner() || history.empty()) {
        return;
    }

    Move oo = history.back();
    history.pop_back();

    grid[oo.x][oo.y] = non;

    if (oo.color == Chessquer::white) {
        state = gameState::whiteToD;
    } else {
        state = gameState::blackToD;
    }


}

void game::restart() {
    history.clear();
    state = blackToD;
    for (size_t i = 0; i < 15; ++i) {
        for (size_t j = 0; j < 15; ++j) {
            grid[i][j] = non;
        }
    }

}

const std::vector<game::Move>& game::getGameData() {
    return history;
}

bool game::loadDataToGame(const std::vector<Move>& his) {
    /// 这个restart 有必要吗！！！ 似乎没有，调用这个函数只会在初始界面
    if (history.size() > 0)
        restart();

    for (size_t i = 0, len = his.size(); i < len; ++i) {
        const Move* temp = &his[i];
        if (canMakeMove(temp->x, temp->y)) {
            qDebug() << "load move :" << temp->x << " " << temp->y << " " << temp->color;
            MakeMove(temp->x, temp->y, temp->color);
        } else {
            restart();
            return false;
        }
    }
    return true;
}

const game::gameState game::getGameState() {
    return game::state;
}

bool game::tryGetWinner(const size_t id, Chessquer color) {

    if (id < 8)
        return false;
    if (state == whiteWin || state == blackWin || state == tie)
        return true;
    for (int i = 0; i < 4; ++i) {
        int tot = 0;
        int temp = history[id].allDirections[i];
        while (temp != -1) {
            tot++;
            temp = history[temp].allDirections[i];
        }
        temp = history[id].allDirections[7 - i];
        while (temp != -1) {
            tot++;
            temp = history[temp].allDirections[7 - i];
        }

        if (tot >= 4) {/// todo 更合理的判断赢的方式
            state = (color == white ? whiteWin : blackWin);
            return true;
        }
    }
    return false;
}

bool game::vised(const size_t x, const size_t y) const {
    return grid[x][y] != non;
}

void game::setHasVised(const size_t x, const size_t y, const Chessquer color) {
    grid[x][y] = color;
}

bool game::inbound(const int x, const int y) {
    return x >= 0 && y >= 0 && x < game::WID && y < game::WID;
}



 bool game::reachInValid(const int x, const int y) {
    /// old
    std::string lines1[4];
    /// new
    std::string lines2[4];

    /// 1 Black ; 2 White; 0 NONE
    unsigned int i, j;
    /// TODO 写死了！！
    for (i = std::max(0, x - 5); i < (unsigned int) std::min(15, x + 6); i++) {
        if (i != (unsigned int) x) {
            lines1[0].push_back(grid[i][y] ==  black ? '1' : (grid[i][y] == non ? '0' : '2'));
            lines2[0].push_back(grid[i][y] == black ? '1' : (grid[i][y] == non ? '0' : '2'));
        } else {
            /// TODO
            lines1[0].push_back('0');
            lines2[0].push_back('1');
        }

    }
    for (i = max(0, y - 5); i < (unsigned int) min(15, y + 6); i++) {
        if (i != (unsigned int) y) { ///// !
            lines1[1].push_back(grid[x][i] == black ? '1' : (grid[x][i] == non ? '0' : '2'));
            lines2[1].push_back(grid[x][i] == black ? '1' : (grid[x][i] == non ? '0' : '2'));
        } else {
            lines1[1].push_back('0');
            lines2[1].push_back('1');
        }
    }
    for (i = x - std::min(std::min(x, y), 5), j = y - std::min(std::min(x, y), 5);
         i < (unsigned int) std::min(15, x + 6) && j < (unsigned int) std::min(15, y + 6); i++, j++) {
        if (i != (unsigned int) x) {
            lines1[2].push_back(grid[i][j] == black ? '1' : (grid[i][j] == non ? '0' : '2'));
            lines2[2].push_back(grid[i][j] == black ? '1' : (grid[i][j] == non ? '0' : '2'));
        } else {
            lines1[2].push_back('0');
            lines2[2].push_back('1');
        }
    }
    /// 这里用了unsigned int 溢出后变成的大数!!!
    for (i = x + std::min(std::min(y, 15 - 1 - x), 5), j = y - std::min(std::min(y, 15 - 1 - x), 5);
         i >= (unsigned int) std::max(0, x - 5) && i < 1000 && j < (unsigned int) std::min(15, y + 6); i--, j++) {
        if (i != (unsigned int) x) {
            lines1[3].push_back(grid[i][j] == black ? '1' : (grid[i][j] == non ? '0' : '2'));
            lines2[3].push_back(grid[i][j] == black ? '1' : (grid[i][j] == non ? '0' : '2'));
        } else {
            lines1[3].push_back('0');
            lines2[3].push_back('1');
        }
    }


    int huo_3_old = 0, chneg_4_old = 0;
    int huo_3_new = 0, chneg_4_new = 0;
    for (i = 0; i < 4; i++) {
        std::vector<int> tmp = ac.ACSearch(lines1[i]);
        huo_3_old += tmp[2] + tmp[3] + tmp[4] + tmp[5];
        chneg_4_old += tmp[1] +tmp[6] + tmp[7] + tmp[8] + tmp[9] + tmp[10];

        tmp = ac.ACSearch(lines2[i]);
        huo_3_new += tmp[2] + tmp[3] + tmp[4] + tmp[5];
        chneg_4_new += tmp[6] + tmp[7] + tmp[8] + tmp[9] + tmp[10];

    }
    qDebug() << "Find huo_3" << " " << huo_3_old << " -> " << huo_3_new;
    qDebug() << "Find cheng_4" << " " << chneg_4_old << " -> " << chneg_4_new;

    /// TODO delete
    huo_3_new = huo_3_old;

    /// 后两个一定用不到
    return (huo_3_new - huo_3_old >= 2) ||
           (chneg_4_new - chneg_4_old >= 2) ||
           (chneg_4_new - chneg_4_old == 1 && huo_3_new - huo_3_old == 2) ||
           (chneg_4_new - chneg_4_old == 2 && huo_3_new - huo_3_old == 1);

 }

