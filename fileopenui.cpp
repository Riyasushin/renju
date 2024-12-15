#include "fileopenui.h"
#include "ui_fileopenui.h"

fileOpenUI::fileOpenUI( std::vector<std::string> his, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::fileOpenUI)
{
    ui->setupUi(this);

    // 创建垂直布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // 创建并添加上层文本提示
    QLabel *helloLabel = new QLabel("loading...");
    mainLayout->addWidget(helloLabel);

    // 创建列表容器部件
    QWidget *listWidget = new QWidget;
    QVBoxLayout *listLayout = new QVBoxLayout(listWidget);

    // 创建QSignalMapper对象
    QSignalMapper *signalMapper = new QSignalMapper(this);

    // 创建并添加列表中的每一行（左右布局）
    for (size_t i = 0, len = his.size(); i < len; ++i) { // 这里假设添加5行，可根据需求整
        qDebug() << "reaching load filelist";

        QHBoxLayout *rowLayout = new QHBoxLayout;

        // 创建左侧文本
        QLabel *testLabel = new QLabel(QString::fromStdString(his[i]));
        rowLayout->addWidget(testLabel);

        // 创建右侧按钮
        QPushButton *button = new QPushButton("load");
        // 将按钮点击信号映射到signalMapper的map()信号
        connect(button, &QPushButton::clicked, signalMapper, qOverload<>(&QSignalMapper::map));
        // 设置映射关系，将按钮和QLabel的文本内容关联起来
        signalMapper->setMapping(button, QString::fromStdString(his[i]));



        rowLayout->addWidget(button);
        listLayout->addLayout(rowLayout);
    }

    // 将QSignalMapper的mapped信号连接到接收槽函数
    connect(signalMapper, &QSignalMapper::mappedString, this, &fileOpenUI::hideAndReturnChoice);

    // 将列表容器部件添加到垂直布局
    mainLayout->addWidget(listWidget);
}

void fileOpenUI::hideAndReturnChoice(const QString &labelText) {
    hide();
    qDebug() << "receive file to load:" << labelText;

    emit sendStringSignal(labelText);
}

fileOpenUI::~fileOpenUI()
{
    delete ui;
}
