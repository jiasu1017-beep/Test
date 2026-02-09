/********************************************************************************
** Form generated from reading UI file 'shutdownwidget.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SHUTDOWNWIDGET_H
#define UI_SHUTDOWNWIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QTimeEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_ShutdownWidget
{
public:
    QVBoxLayout *mainLayout;
    QLabel *titleLabel;
    QGroupBox *actionGroup;
    QHBoxLayout *actionLayout;
    QRadioButton *shutdownRadio;
    QRadioButton *restartRadio;
    QRadioButton *sleepRadio;
    QGroupBox *timeGroup;
    QVBoxLayout *timeLayout;
    QRadioButton *countdownRadio;
    QHBoxLayout *countdownLayout;
    QLabel *hourLabel;
    QSpinBox *hourSpin;
    QLabel *minuteLabel;
    QSpinBox *minuteSpin;
    QLabel *secondLabel;
    QSpinBox *secondSpin;
    QRadioButton *timeRadio;
    QTimeEdit *timeEdit;
    QLabel *statusLabel;
    QLabel *countdownLabel;
    QHBoxLayout *buttonLayout;
    QPushButton *startButton;
    QPushButton *cancelButton;
    QGroupBox *instantGroup;
    QHBoxLayout *instantLayout;
    QPushButton *shutdownNowButton;
    QPushButton *restartNowButton;
    QPushButton *sleepNowButton;

    void setupUi(QWidget *ShutdownWidget)
    {
        if (ShutdownWidget->objectName().isEmpty())
            ShutdownWidget->setObjectName(QString::fromUtf8("ShutdownWidget"));
        ShutdownWidget->resize(700, 600);
        mainLayout = new QVBoxLayout(ShutdownWidget);
        mainLayout->setObjectName(QString::fromUtf8("mainLayout"));
        titleLabel = new QLabel(ShutdownWidget);
        titleLabel->setObjectName(QString::fromUtf8("titleLabel"));
        titleLabel->setStyleSheet(QString::fromUtf8("font-size: 20px; font-weight: bold; padding: 10px;"));

        mainLayout->addWidget(titleLabel);

        actionGroup = new QGroupBox(ShutdownWidget);
        actionGroup->setObjectName(QString::fromUtf8("actionGroup"));
        actionLayout = new QHBoxLayout(actionGroup);
        actionLayout->setObjectName(QString::fromUtf8("actionLayout"));
        shutdownRadio = new QRadioButton(actionGroup);
        shutdownRadio->setObjectName(QString::fromUtf8("shutdownRadio"));
        shutdownRadio->setChecked(true);

        actionLayout->addWidget(shutdownRadio);

        restartRadio = new QRadioButton(actionGroup);
        restartRadio->setObjectName(QString::fromUtf8("restartRadio"));

        actionLayout->addWidget(restartRadio);

        sleepRadio = new QRadioButton(actionGroup);
        sleepRadio->setObjectName(QString::fromUtf8("sleepRadio"));

        actionLayout->addWidget(sleepRadio);


        mainLayout->addWidget(actionGroup);

        timeGroup = new QGroupBox(ShutdownWidget);
        timeGroup->setObjectName(QString::fromUtf8("timeGroup"));
        timeLayout = new QVBoxLayout(timeGroup);
        timeLayout->setObjectName(QString::fromUtf8("timeLayout"));
        countdownRadio = new QRadioButton(timeGroup);
        countdownRadio->setObjectName(QString::fromUtf8("countdownRadio"));
        countdownRadio->setChecked(true);

        timeLayout->addWidget(countdownRadio);

        countdownLayout = new QHBoxLayout();
        countdownLayout->setObjectName(QString::fromUtf8("countdownLayout"));
        hourLabel = new QLabel(timeGroup);
        hourLabel->setObjectName(QString::fromUtf8("hourLabel"));

        countdownLayout->addWidget(hourLabel);

        hourSpin = new QSpinBox(timeGroup);
        hourSpin->setObjectName(QString::fromUtf8("hourSpin"));
        hourSpin->setMaximum(99);

        countdownLayout->addWidget(hourSpin);

        minuteLabel = new QLabel(timeGroup);
        minuteLabel->setObjectName(QString::fromUtf8("minuteLabel"));

        countdownLayout->addWidget(minuteLabel);

        minuteSpin = new QSpinBox(timeGroup);
        minuteSpin->setObjectName(QString::fromUtf8("minuteSpin"));
        minuteSpin->setMaximum(59);
        minuteSpin->setValue(10);

        countdownLayout->addWidget(minuteSpin);

        secondLabel = new QLabel(timeGroup);
        secondLabel->setObjectName(QString::fromUtf8("secondLabel"));

        countdownLayout->addWidget(secondLabel);

        secondSpin = new QSpinBox(timeGroup);
        secondSpin->setObjectName(QString::fromUtf8("secondSpin"));
        secondSpin->setMaximum(59);

        countdownLayout->addWidget(secondSpin);


        timeLayout->addLayout(countdownLayout);

        timeRadio = new QRadioButton(timeGroup);
        timeRadio->setObjectName(QString::fromUtf8("timeRadio"));

        timeLayout->addWidget(timeRadio);

        timeEdit = new QTimeEdit(timeGroup);
        timeEdit->setObjectName(QString::fromUtf8("timeEdit"));

        timeLayout->addWidget(timeEdit);


        mainLayout->addWidget(timeGroup);

        statusLabel = new QLabel(ShutdownWidget);
        statusLabel->setObjectName(QString::fromUtf8("statusLabel"));
        statusLabel->setStyleSheet(QString::fromUtf8("font-size: 14px; padding: 10px; background-color: #e3f2fd; border-radius: 5px;"));

        mainLayout->addWidget(statusLabel);

        countdownLabel = new QLabel(ShutdownWidget);
        countdownLabel->setObjectName(QString::fromUtf8("countdownLabel"));
        countdownLabel->setStyleSheet(QString::fromUtf8("font-size: 24px; font-weight: bold; padding: 10px; text-align: center;"));
        countdownLabel->setAlignment(Qt::AlignCenter);

        mainLayout->addWidget(countdownLabel);

        buttonLayout = new QHBoxLayout();
        buttonLayout->setObjectName(QString::fromUtf8("buttonLayout"));
        startButton = new QPushButton(ShutdownWidget);
        startButton->setObjectName(QString::fromUtf8("startButton"));

        buttonLayout->addWidget(startButton);

        cancelButton = new QPushButton(ShutdownWidget);
        cancelButton->setObjectName(QString::fromUtf8("cancelButton"));
        cancelButton->setEnabled(false);

        buttonLayout->addWidget(cancelButton);


        mainLayout->addLayout(buttonLayout);

        instantGroup = new QGroupBox(ShutdownWidget);
        instantGroup->setObjectName(QString::fromUtf8("instantGroup"));
        instantLayout = new QHBoxLayout(instantGroup);
        instantLayout->setObjectName(QString::fromUtf8("instantLayout"));
        shutdownNowButton = new QPushButton(instantGroup);
        shutdownNowButton->setObjectName(QString::fromUtf8("shutdownNowButton"));

        instantLayout->addWidget(shutdownNowButton);

        restartNowButton = new QPushButton(instantGroup);
        restartNowButton->setObjectName(QString::fromUtf8("restartNowButton"));

        instantLayout->addWidget(restartNowButton);

        sleepNowButton = new QPushButton(instantGroup);
        sleepNowButton->setObjectName(QString::fromUtf8("sleepNowButton"));

        instantLayout->addWidget(sleepNowButton);


        mainLayout->addWidget(instantGroup);


        retranslateUi(ShutdownWidget);

        QMetaObject::connectSlotsByName(ShutdownWidget);
    } // setupUi

    void retranslateUi(QWidget *ShutdownWidget)
    {
        ShutdownWidget->setWindowTitle(QCoreApplication::translate("ShutdownWidget", "Form", nullptr));
        titleLabel->setText(QCoreApplication::translate("ShutdownWidget", "\345\256\232\346\227\266\345\205\263\346\234\272", nullptr));
        actionGroup->setTitle(QCoreApplication::translate("ShutdownWidget", "\351\200\211\346\213\251\346\223\215\344\275\234", nullptr));
        shutdownRadio->setText(QCoreApplication::translate("ShutdownWidget", "\345\205\263\346\234\272", nullptr));
        restartRadio->setText(QCoreApplication::translate("ShutdownWidget", "\351\207\215\345\220\257", nullptr));
        sleepRadio->setText(QCoreApplication::translate("ShutdownWidget", "\347\235\241\347\234\240", nullptr));
        timeGroup->setTitle(QCoreApplication::translate("ShutdownWidget", "\345\256\232\346\227\266\346\226\271\345\274\217", nullptr));
        countdownRadio->setText(QCoreApplication::translate("ShutdownWidget", "\345\200\222\350\256\241\346\227\266", nullptr));
        hourLabel->setText(QCoreApplication::translate("ShutdownWidget", "\346\227\266:", nullptr));
        minuteLabel->setText(QCoreApplication::translate("ShutdownWidget", "\345\210\206:", nullptr));
        secondLabel->setText(QCoreApplication::translate("ShutdownWidget", "\347\247\222:", nullptr));
        timeRadio->setText(QCoreApplication::translate("ShutdownWidget", "\346\214\207\345\256\232\346\227\266\351\227\264", nullptr));
        timeEdit->setDisplayFormat(QCoreApplication::translate("ShutdownWidget", "HH:mm:ss", nullptr));
        statusLabel->setText(QCoreApplication::translate("ShutdownWidget", "\347\212\266\346\200\201: \347\255\211\345\276\205\346\223\215\344\275\234", nullptr));
        countdownLabel->setText(QCoreApplication::translate("ShutdownWidget", "\345\200\222\350\256\241\346\227\266: --:--:--", nullptr));
        startButton->setText(QCoreApplication::translate("ShutdownWidget", "\345\274\200\345\247\213", nullptr));
        cancelButton->setText(QCoreApplication::translate("ShutdownWidget", "\345\217\226\346\266\210", nullptr));
        instantGroup->setTitle(QCoreApplication::translate("ShutdownWidget", "\347\253\213\345\215\263\346\211\247\350\241\214", nullptr));
        shutdownNowButton->setText(QCoreApplication::translate("ShutdownWidget", "\347\253\213\345\215\263\345\205\263\346\234\272", nullptr));
        restartNowButton->setText(QCoreApplication::translate("ShutdownWidget", "\347\253\213\345\215\263\351\207\215\345\220\257", nullptr));
        sleepNowButton->setText(QCoreApplication::translate("ShutdownWidget", "\347\253\213\345\215\263\347\235\241\347\234\240", nullptr));
    } // retranslateUi

};

namespace Ui {
    class ShutdownWidget: public Ui_ShutdownWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SHUTDOWNWIDGET_H
