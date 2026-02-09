/********************************************************************************
** Form generated from reading UI file 'settingswidget.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SETTINGSWIDGET_H
#define UI_SETTINGSWIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_SettingsWidget
{
public:
    QVBoxLayout *mainLayout;
    QLabel *titleLabel;
    QGroupBox *startupGroup;
    QVBoxLayout *startupLayout;
    QCheckBox *autoStartCheck;
    QLabel *hintLabel;
    QLabel *statusLabel;
    QSpacerItem *verticalSpacer;
    QGroupBox *aboutGroup;
    QVBoxLayout *aboutLayout;
    QLabel *aboutLabel;
    QPushButton *aboutButton;

    void setupUi(QWidget *SettingsWidget)
    {
        if (SettingsWidget->objectName().isEmpty())
            SettingsWidget->setObjectName(QString::fromUtf8("SettingsWidget"));
        SettingsWidget->resize(600, 400);
        mainLayout = new QVBoxLayout(SettingsWidget);
        mainLayout->setObjectName(QString::fromUtf8("mainLayout"));
        titleLabel = new QLabel(SettingsWidget);
        titleLabel->setObjectName(QString::fromUtf8("titleLabel"));
        titleLabel->setStyleSheet(QString::fromUtf8("font-size: 20px; font-weight: bold; padding: 10px;"));

        mainLayout->addWidget(titleLabel);

        startupGroup = new QGroupBox(SettingsWidget);
        startupGroup->setObjectName(QString::fromUtf8("startupGroup"));
        startupLayout = new QVBoxLayout(startupGroup);
        startupLayout->setObjectName(QString::fromUtf8("startupLayout"));
        autoStartCheck = new QCheckBox(startupGroup);
        autoStartCheck->setObjectName(QString::fromUtf8("autoStartCheck"));

        startupLayout->addWidget(autoStartCheck);

        hintLabel = new QLabel(startupGroup);
        hintLabel->setObjectName(QString::fromUtf8("hintLabel"));
        hintLabel->setStyleSheet(QString::fromUtf8("color: #666; padding: 5px;"));

        startupLayout->addWidget(hintLabel);


        mainLayout->addWidget(startupGroup);

        statusLabel = new QLabel(SettingsWidget);
        statusLabel->setObjectName(QString::fromUtf8("statusLabel"));
        statusLabel->setStyleSheet(QString::fromUtf8("font-size: 14px; padding: 10px; background-color: #e8f5e9; border-radius: 5px;"));

        mainLayout->addWidget(statusLabel);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        mainLayout->addItem(verticalSpacer);

        aboutGroup = new QGroupBox(SettingsWidget);
        aboutGroup->setObjectName(QString::fromUtf8("aboutGroup"));
        aboutLayout = new QVBoxLayout(aboutGroup);
        aboutLayout->setObjectName(QString::fromUtf8("aboutLayout"));
        aboutLabel = new QLabel(aboutGroup);
        aboutLabel->setObjectName(QString::fromUtf8("aboutLabel"));
        aboutLabel->setWordWrap(true);
        aboutLabel->setStyleSheet(QString::fromUtf8("padding: 10px;"));

        aboutLayout->addWidget(aboutLabel);

        aboutButton = new QPushButton(aboutGroup);
        aboutButton->setObjectName(QString::fromUtf8("aboutButton"));

        aboutLayout->addWidget(aboutButton);


        mainLayout->addWidget(aboutGroup);


        retranslateUi(SettingsWidget);

        QMetaObject::connectSlotsByName(SettingsWidget);
    } // setupUi

    void retranslateUi(QWidget *SettingsWidget)
    {
        SettingsWidget->setWindowTitle(QCoreApplication::translate("SettingsWidget", "Form", nullptr));
        titleLabel->setText(QCoreApplication::translate("SettingsWidget", "\350\256\276\347\275\256", nullptr));
        startupGroup->setTitle(QCoreApplication::translate("SettingsWidget", "\345\274\200\346\234\272\345\220\257\345\212\250", nullptr));
        autoStartCheck->setText(QCoreApplication::translate("SettingsWidget", "\345\274\200\346\234\272\350\207\252\345\212\250\345\220\257\345\212\250", nullptr));
        hintLabel->setText(QCoreApplication::translate("SettingsWidget", "\345\213\276\351\200\211\345\220\216\357\274\214\345\272\224\347\224\250\345\260\206\351\232\217\347\263\273\347\273\237\345\220\257\345\212\250\350\200\214\350\207\252\345\212\250\350\277\220\350\241\214", nullptr));
        statusLabel->setText(QCoreApplication::translate("SettingsWidget", "\347\212\266\346\200\201: \345\260\261\347\273\252", nullptr));
        aboutGroup->setTitle(QCoreApplication::translate("SettingsWidget", "\345\205\263\344\272\216", nullptr));
        aboutLabel->setText(QCoreApplication::translate("SettingsWidget", "\345\212\236\345\205\254\345\212\251\346\211\213 v1.0\n"
"\344\270\200\346\254\276\347\256\200\345\215\225\346\230\223\347\224\250\347\232\204\346\241\214\351\235\242\345\212\236\345\205\254\345\212\251\346\211\213\345\272\224\347\224\250\n"
"\n"
"\345\212\237\350\203\275\347\211\271\347\202\271\357\274\232\n"
"\342\200\242 \345\272\224\347\224\250\347\256\241\347\220\206 - \345\277\253\351\200\237\345\220\257\345\212\250\345\270\270\347\224\250\345\272\224\347\224\250\n"
"\342\200\242 \346\221\270\351\261\274\346\250\241\345\274\217 - \345\277\253\351\200\237\351\232\220\350\227\217\345\222\214\344\274\252\350\243\205\n"
"\342\200\242 \345\256\232\346\227\266\345\205\263\346\234\272 - \347\201\265\346\264\273\347\232\204\345\256\232\346\227\266\346\216\247\345\210\266", nullptr));
        aboutButton->setText(QCoreApplication::translate("SettingsWidget", "\345\205\263\344\272\216\346\234\254\350\275\257\344\273\266", nullptr));
    } // retranslateUi

};

namespace Ui {
    class SettingsWidget: public Ui_SettingsWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SETTINGSWIDGET_H
