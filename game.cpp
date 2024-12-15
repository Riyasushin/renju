#include "game.h"


std::string GAMEFILESUF = ".data";

game::game() {

    restart();
}



bool game::canMakeMove(const size_t x, const size_t y) const {
    return  !game::vised(x, y) &&
           (game::state == whiteToD || game::islegal(x, y) );

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
    return history[history.size() - 1].color;

}

bool game::hasDecideWinner() const {
    return !(state == whiteToD || state == blackToD);
}

void game::huiqi() {
    if (hasDecideWinner() || history.empty()) {
        return;
    }

    Move oo = history.front();
    history.pop_back();

    grid[oo.x][oo.y] = non;
    state = (oo.color == white ? whiteToD : blackToD);

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

bool game::islegal(const size_t x, const size_t y) const {
    /// TODO
    return true;
}

