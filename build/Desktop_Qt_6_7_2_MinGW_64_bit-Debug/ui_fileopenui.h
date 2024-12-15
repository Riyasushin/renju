/********************************************************************************
** Form generated from reading UI file 'fileopenui.ui'
**
** Created by: Qt User Interface Compiler version 6.7.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_FILEOPENUI_H
#define UI_FILEOPENUI_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_fileOpenUI
{
public:

    void setupUi(QWidget *fileOpenUI)
    {
        if (fileOpenUI->objectName().isEmpty())
            fileOpenUI->setObjectName("fileOpenUI");
        fileOpenUI->resize(351, 277);

        retranslateUi(fileOpenUI);

        QMetaObject::connectSlotsByName(fileOpenUI);
    } // setupUi

    void retranslateUi(QWidget *fileOpenUI)
    {
        fileOpenUI->setWindowTitle(QCoreApplication::translate("fileOpenUI", "Form", nullptr));
    } // retranslateUi

};

namespace Ui {
    class fileOpenUI: public Ui_fileOpenUI {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_FILEOPENUI_H
