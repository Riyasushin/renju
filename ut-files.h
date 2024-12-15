#ifndef   FILES_H
#define   FILES_H

/// 文件处理的各种静态方法放到这里
/// 包括：
/// 	读取文件夹下所有特定后缀的文件，并且返回一个vector<string>
/// 	读取数据文件
/// 	写入数据文件

#include "game.h"
#include <string>
#include <vector>
#include <fstream>
#include <filesystem>
#include <QDebug>






class fileutils {

public:
    /// TODO HEADCODE
    ///static std::string HEADCODE;

    /**
     * @brief readFromFile
     * @param filename 文件名
     * @param history  传一个引用，给里面加上读取到的数据
     * @return  返回读取成功了没有，不抛出异常，之后文件界面进行处理TODO
     */
    static bool readFromFile(std::string filename, std::vector<game::Move>& history) {
        std::ifstream  oss(filename.c_str());
        if (oss.is_open()) {
            std::string headcode;
            getline(oss, headcode);
            if (headcode == "#66ccff#ee0000#ffff00#006666#0080ff	(238,0,0)") {
                size_t x, y;
                char qizi; /// 不想重写操作符了，W是白，B是黑
                while (oss >> x >> y >> qizi) {
                    game::Chessquer qiqi;
                    if (qizi == 'W') {
                        qiqi = game::Chessquer::white;
                    } else if (qizi == 'B') {
                        qiqi = game::Chessquer::black;
                    } else {
                        qDebug() << "读取文件数据时出错， 非法的棋子类型";
                        return false;
                    }
                    /// 调用了GAME.H中定义的一个宏，注意
                    history.push_back({x, y, qiqi, PUSHINGARBAGE});
                }
            } else {
                qDebug() << "读取文件数据时发现头文件损坏";
                return false;
            }
        } else {
            qDebug() << "fail to open " << filename.c_str();
            return false;
        }
        return true;
    }

    /**
     * @brief readFileList
     * 读取程序根目录下的所有以sufname为后缀的文件，并把文件名除掉拓展名传回去，用来构造可选文件列表
     * TODO： 如果文件夹中的文件非常多，调用 std::filesystem::directory_iterator 会比较消耗资源。可以考虑在文件数量非常大时引入分页或多线程处理。
     * @return
     */
    static std::vector<std::string> readFileList(const std::string path, const std::string& extension) {
        std::vector<std::string> res;

        try {
            // 遍历指定目录下的所有文件
            for (const auto& entry : std::filesystem::directory_iterator(path)) {
                // 如果是普通文件，并且文件的扩展名与给定的匹配
                if (entry.is_regular_file() && entry.path().extension() == extension) {
                    // 将符合条件的文件名加入到结果向量中
                    res.push_back(entry.path().stem().string());
                }
            }
        } catch (const std::filesystem::filesystem_error& e) {
            qDebug() << "Error: " << e.what();
        }
        return res;
    }

    /**
     * @brief writeTo
     * @param history 需要被加载进去的数据
     * @param name    要生成的文件的名字
     * @return      写成功了就返回true
     */
    static bool writeTo(const std::vector<game::Move>& history, const std::string name) {
        std::ofstream oss(name.c_str());

        if (oss.is_open()) {
            qDebug() << "writing:" << std::filesystem::current_path().c_str() << " " << name << "opened";
            oss << "#66ccff#ee0000#ffff00#006666#0080ff	(238,0,0)" << std::endl;
            for (auto it : history) {
                oss << " " << it.x << " " << it.y << " " << (it.color == game::white ? "W" : "B") << ' ';
                qDebug() << " " << it.x << " " << it.y << " " << (it.color == game::white ? "W" : "B");
            }
            oss.close();
            return true;
        } else {
            return false;
        }
    }

private:
    fileutils();

};

/// static 定义与命名分开写 WRONG!!!
///std::string fileutils::HEADCODE = "#66ccff#ee0000#ffff00#006666#0080ff	(238,0,0)";

#endif // UT-FILES_H
