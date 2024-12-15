#ifndef FILEOPENUI_H
#define FILEOPENUI_H

#include <QWidget>
#include <vector>
#include <string>
#include <QString>
#include <QBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSignalMapper>
/**
 * @brief The ItemListWindow class
 * 用来加载文件列表的窗口，因为是根据文件多少动态的不能写死
 * 可视化就不好用了
 * 只能自己写
 */

namespace Ui {
class fileOpenUI;
}

class fileOpenUI : public QWidget
{
    Q_OBJECT

public:
    explicit fileOpenUI(std::vector<std::string> his, QWidget *parent = nullptr);
    ~fileOpenUI();

private:
    Ui::fileOpenUI *ui;
    std::vector<std::string> history;
public slots:
    void hideAndReturnChoice(const QString &labelText);

signals:
    void sendStringSignal(const QString &stringToSend);
};

#endif // FILEOPENUI_H
