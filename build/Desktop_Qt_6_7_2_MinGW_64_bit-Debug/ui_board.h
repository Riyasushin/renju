/********************************************************************************
** Form generated from reading UI file 'board.ui'
**
** Created by: Qt User Interface Compiler version 6.7.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_BOARD_H
#define UI_BOARD_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Board
{
public:
    QWidget *layoutWidget;
    QVBoxLayout *verticalLayout;
    QPushButton *huiqi;
    QPushButton *restart;
    QPushButton *save;
    QPushButton *end;
    QWidget *IMAGE;
    QLabel *label;

    void setupUi(QWidget *Board)
    {
        if (Board->objectName().isEmpty())
            Board->setObjectName("Board");
        Board->resize(939, 700);
        Board->setMinimumSize(QSize(600, 700));
        Board->setMaximumSize(QSize(1000, 700));
        QFont font;
        font.setPointSize(20);
        Board->setFont(font);
        layoutWidget = new QWidget(Board);
        layoutWidget->setObjectName("layoutWidget");
        layoutWidget->setGeometry(QRect(710, 200, 151, 191));
        verticalLayout = new QVBoxLayout(layoutWidget);
        verticalLayout->setObjectName("verticalLayout");
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        huiqi = new QPushButton(layoutWidget);
        huiqi->setObjectName("huiqi");

        verticalLayout->addWidget(huiqi);

        restart = new QPushButton(layoutWidget);
        restart->setObjectName("restart");

        verticalLayout->addWidget(restart);

        save = new QPushButton(layoutWidget);
        save->setObjectName("save");

        verticalLayout->addWidget(save);

        end = new QPushButton(layoutWidget);
        end->setObjectName("end");

        verticalLayout->addWidget(end);

        IMAGE = new QWidget(Board);
        IMAGE->setObjectName("IMAGE");
        IMAGE->setGeometry(QRect(710, 70, 131, 81));
        label = new QLabel(Board);
        label->setObjectName("label");
        label->setGeometry(QRect(720, 160, 141, 31));
        QFont font1;
        font1.setFamilies({QString::fromUtf8("\345\215\216\346\226\207\350\241\214\346\245\267")});
        font1.setPointSize(20);
        label->setFont(font1);

        retranslateUi(Board);

        QMetaObject::connectSlotsByName(Board);
    } // setupUi

    void retranslateUi(QWidget *Board)
    {
        Board->setWindowTitle(QCoreApplication::translate("Board", "Form", nullptr));
        huiqi->setText(QCoreApplication::translate("Board", "\346\202\224\346\243\213", nullptr));
        restart->setText(QCoreApplication::translate("Board", "\351\207\215\346\226\260\345\274\200\345\247\213", nullptr));
        save->setText(QCoreApplication::translate("Board", "\344\277\235\345\255\230\345\257\271\345\261\200", nullptr));
        end->setText(QCoreApplication::translate("Board", "\347\273\223\346\235\237\346\270\270\346\210\217", nullptr));
        label->setText(QCoreApplication::translate("Board", "\350\275\256\345\210\260\351\273\221\345\255\220", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Board: public Ui_Board {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_BOARD_H
