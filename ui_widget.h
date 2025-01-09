/********************************************************************************
** Form generated from reading UI file 'widget.ui'
**
** Created by: Qt User Interface Compiler version 6.7.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_WIDGET_H
#define UI_WIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Widget
{
public:
    QLabel *label;
    QWidget *widget;
    QVBoxLayout *verticalLayout;
    QPushButton *load;
    QPushButton *pve;
    QPushButton *pvp;

    void setupUi(QWidget *Widget)
    {
        if (Widget->objectName().isEmpty())
            Widget->setObjectName("Widget");
        Widget->resize(433, 413);
        label = new QLabel(Widget);
        label->setObjectName("label");
        label->setGeometry(QRect(140, 20, 241, 61));
        QFont font;
        font.setFamilies({QString::fromUtf8("\345\215\216\346\226\207\346\226\260\351\255\217")});
        font.setPointSize(48);
        label->setFont(font);
        widget = new QWidget(Widget);
        widget->setObjectName("widget");
        widget->setGeometry(QRect(140, 100, 161, 181));
        verticalLayout = new QVBoxLayout(widget);
        verticalLayout->setObjectName("verticalLayout");
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        load = new QPushButton(widget);
        load->setObjectName("load");
        QFont font1;
        font1.setPointSize(15);
        load->setFont(font1);

        verticalLayout->addWidget(load);

        pve = new QPushButton(widget);
        pve->setObjectName("pve");
        pve->setFont(font1);

        verticalLayout->addWidget(pve);

        pvp = new QPushButton(widget);
        pvp->setObjectName("pvp");
        pvp->setFont(font1);

        verticalLayout->addWidget(pvp);


        retranslateUi(Widget);

        QMetaObject::connectSlotsByName(Widget);
    } // setupUi

    void retranslateUi(QWidget *Widget)
    {
        Widget->setWindowTitle(QCoreApplication::translate("Widget", "Widget", nullptr));
        label->setText(QCoreApplication::translate("Widget", "\344\272\224\345\255\220\346\243\213", nullptr));
        load->setText(QCoreApplication::translate("Widget", "\350\275\275\345\205\245\346\256\213\345\261\200", nullptr));
        pve->setText(QCoreApplication::translate("Widget", "\344\272\272\346\234\272\345\257\271\345\261\200", nullptr));
        pvp->setText(QCoreApplication::translate("Widget", "\345\245\275\345\217\213\345\257\271\345\206\263", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Widget: public Ui_Widget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_WIDGET_H
