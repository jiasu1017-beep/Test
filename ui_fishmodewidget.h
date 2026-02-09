/********************************************************************************
** Form generated from reading UI file 'fishmodewidget.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_FISHMODEWIDGET_H
#define UI_FISHMODEWIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_FishModeWidget
{
public:
    QVBoxLayout *mainLayout;
    QLabel *titleLabel;
    QLabel *statusLabel;
    QLabel *hintLabel;
    QVBoxLayout *buttonLayout;
    QPushButton *bossKeyButton;
    QPushButton *toggleButton;
    QPushButton *fakeWindowButton;
    QSpacerItem *verticalSpacer;
    QLabel *infoLabel;

    void setupUi(QWidget *FishModeWidget)
    {
        if (FishModeWidget->objectName().isEmpty())
            FishModeWidget->setObjectName(QString::fromUtf8("FishModeWidget"));
        FishModeWidget->resize(600, 500);
        mainLayout = new QVBoxLayout(FishModeWidget);
        mainLayout->setObjectName(QString::fromUtf8("mainLayout"));
        titleLabel = new QLabel(FishModeWidget);
        titleLabel->setObjectName(QString::fromUtf8("titleLabel"));
        titleLabel->setStyleSheet(QString::fromUtf8("font-size: 20px; font-weight: bold; padding: 10px;"));

        mainLayout->addWidget(titleLabel);

        statusLabel = new QLabel(FishModeWidget);
        statusLabel->setObjectName(QString::fromUtf8("statusLabel"));
        statusLabel->setStyleSheet(QString::fromUtf8("font-size: 14px; padding: 10px; background-color: #e8f5e9; border-radius: 5px;"));

        mainLayout->addWidget(statusLabel);

        hintLabel = new QLabel(FishModeWidget);
        hintLabel->setObjectName(QString::fromUtf8("hintLabel"));
        hintLabel->setStyleSheet(QString::fromUtf8("color: #666; padding: 5px;"));

        mainLayout->addWidget(hintLabel);

        buttonLayout = new QVBoxLayout();
        buttonLayout->setObjectName(QString::fromUtf8("buttonLayout"));
        bossKeyButton = new QPushButton(FishModeWidget);
        bossKeyButton->setObjectName(QString::fromUtf8("bossKeyButton"));

        buttonLayout->addWidget(bossKeyButton);

        toggleButton = new QPushButton(FishModeWidget);
        toggleButton->setObjectName(QString::fromUtf8("toggleButton"));

        buttonLayout->addWidget(toggleButton);

        fakeWindowButton = new QPushButton(FishModeWidget);
        fakeWindowButton->setObjectName(QString::fromUtf8("fakeWindowButton"));

        buttonLayout->addWidget(fakeWindowButton);


        mainLayout->addLayout(buttonLayout);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        mainLayout->addItem(verticalSpacer);

        infoLabel = new QLabel(FishModeWidget);
        infoLabel->setObjectName(QString::fromUtf8("infoLabel"));
        infoLabel->setStyleSheet(QString::fromUtf8("color: #666; padding: 10px; background-color: #f5f5f5; border-radius: 5px;"));
        infoLabel->setWordWrap(true);

        mainLayout->addWidget(infoLabel);


        retranslateUi(FishModeWidget);

        QMetaObject::connectSlotsByName(FishModeWidget);
    } // setupUi

    void retranslateUi(QWidget *FishModeWidget)
    {
        FishModeWidget->setWindowTitle(QCoreApplication::translate("FishModeWidget", "Form", nullptr));
        titleLabel->setText(QCoreApplication::translate("FishModeWidget", "\346\221\270\351\261\274\346\250\241\345\274\217", nullptr));
        statusLabel->setText(QCoreApplication::translate("FishModeWidget", "\345\275\223\345\211\215\347\212\266\346\200\201: \345\267\245\344\275\234\346\250\241\345\274\217", nullptr));
        hintLabel->setText(QCoreApplication::translate("FishModeWidget", "\345\277\253\346\215\267\351\224\256: F12 - \350\200\201\346\235\277\351\224\256", nullptr));
        bossKeyButton->setText(QCoreApplication::translate("FishModeWidget", "\350\200\201\346\235\277\351\224\256 (F12)", nullptr));
        toggleButton->setText(QCoreApplication::translate("FishModeWidget", "\345\210\207\346\215\242\346\221\270\351\261\274/\345\267\245\344\275\234\346\250\241\345\274\217", nullptr));
        fakeWindowButton->setText(QCoreApplication::translate("FishModeWidget", "\346\211\223\345\274\200\344\274\252\350\243\205\347\252\227\345\217\243", nullptr));
        infoLabel->setText(QCoreApplication::translate("FishModeWidget", "\345\212\237\350\203\275\350\257\264\346\230\216:\n"
"\342\200\242 \350\200\201\346\235\277\351\224\256(F12): \345\277\253\351\200\237\351\232\220\350\227\217\346\211\200\346\234\211\347\252\227\345\217\243\n"
"\342\200\242 \346\221\270\351\261\274\346\250\241\345\274\217: \345\210\207\346\215\242\345\267\245\344\275\234/\344\274\221\351\227\262\347\212\266\346\200\201\n"
"\342\200\242 \344\274\252\350\243\205\347\252\227\345\217\243: \346\211\223\345\274\200\347\263\273\347\273\237\345\267\245\345\205\267\344\274\252\350\243\205\345\267\245\344\275\234\347\212\266\346\200\201", nullptr));
    } // retranslateUi

};

namespace Ui {
    class FishModeWidget: public Ui_FishModeWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_FISHMODEWIDGET_H
