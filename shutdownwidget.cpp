#include "shutdownwidget.h"
#include <QApplication>
#include <QStyle>
#include <QDateTime>
#include <windows.h>
#include <shellapi.h>
#include <QDebug>

ShutdownWidget::ShutdownWidget(QWidget *parent)
    : QWidget(parent), remainingSeconds(0), currentAction(0)
{
    countdownTimer = new QTimer(this);
    connect(countdownTimer, &QTimer::timeout, this, &ShutdownWidget::onTimerTick);
    setupUI();
}

void ShutdownWidget::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    QLabel *titleLabel = new QLabel("定时关机", this);
    titleLabel->setStyleSheet("font-size: 20px; font-weight: bold; padding: 10px;");
    mainLayout->addWidget(titleLabel);
    
    statusLabel = new QLabel("当前状态: 空闲", this);
    statusLabel->setStyleSheet("font-size: 14px; padding: 10px; background-color: #e3f2fd; border-radius: 5px;");
    mainLayout->addWidget(statusLabel);
    
    countdownLabel = new QLabel("", this);
    countdownLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #f44336; padding: 20px; text-align: center;");
    countdownLabel->setAlignment(Qt::AlignCenter);
    countdownLabel->hide();
    mainLayout->addWidget(countdownLabel);
    
    QGroupBox *actionGroup = new QGroupBox("选择操作", this);
    QVBoxLayout *actionLayout = new QVBoxLayout();
    
    shutdownRadio = new QRadioButton("关机", this);
    restartRadio = new QRadioButton("重启", this);
    sleepRadio = new QRadioButton("休眠", this);
    shutdownRadio->setChecked(true);
    
    actionLayout->addWidget(shutdownRadio);
    actionLayout->addWidget(restartRadio);
    actionLayout->addWidget(sleepRadio);
    actionGroup->setLayout(actionLayout);
    mainLayout->addWidget(actionGroup);
    
    QGroupBox *timeGroup = new QGroupBox("设置时间", this);
    QVBoxLayout *timeLayout = new QVBoxLayout();
    
    countdownRadio = new QRadioButton("倒计时", this);
    timeRadio = new QRadioButton("指定时间", this);
    countdownRadio->setChecked(true);
    
    QHBoxLayout *presetLayout = new QHBoxLayout();
    presetLayout->addWidget(new QLabel("预设时间:", this));
    
    QPushButton *thirtyMinBtn = new QPushButton("30分钟", this);
    QPushButton *oneHourBtn = new QPushButton("1小时", this);
    QPushButton *twoHoursBtn = new QPushButton("2小时", this);
    QPushButton *fourHoursBtn = new QPushButton("4小时", this);
    QPushButton *sixHoursBtn = new QPushButton("6小时", this);
    
    presetLayout->addWidget(thirtyMinBtn);
    presetLayout->addWidget(oneHourBtn);
    presetLayout->addWidget(twoHoursBtn);
    presetLayout->addWidget(fourHoursBtn);
    presetLayout->addWidget(sixHoursBtn);
    
    QHBoxLayout *customLayout = new QHBoxLayout();
    customLayout->addWidget(new QLabel("自定义:", this));
    
    hourSpin = new QSpinBox(this);
    hourSpin->setRange(0, 23);
    hourSpin->setValue(0);
    customLayout->addWidget(hourSpin);
    customLayout->addWidget(new QLabel("小时", this));
    
    minuteSpin = new QSpinBox(this);
    minuteSpin->setRange(0, 59);
    minuteSpin->setValue(30);
    customLayout->addWidget(minuteSpin);
    customLayout->addWidget(new QLabel("分钟", this));
    
    secondSpin = new QSpinBox(this);
    secondSpin->setRange(0, 59);
    secondSpin->setValue(0);
    customLayout->addWidget(secondSpin);
    customLayout->addWidget(new QLabel("秒", this));
    
    // Connect preset buttons
    connect(thirtyMinBtn, &QPushButton::clicked, [this]() {
        hourSpin->setValue(0);
        minuteSpin->setValue(30);
        secondSpin->setValue(0);
    });
    
    connect(oneHourBtn, &QPushButton::clicked, [this]() {
        hourSpin->setValue(1);
        minuteSpin->setValue(0);
        secondSpin->setValue(0);
    });
    
    connect(twoHoursBtn, &QPushButton::clicked, [this]() {
        hourSpin->setValue(2);
        minuteSpin->setValue(0);
        secondSpin->setValue(0);
    });
    
    connect(fourHoursBtn, &QPushButton::clicked, [this]() {
        hourSpin->setValue(4);
        minuteSpin->setValue(0);
        secondSpin->setValue(0);
    });
    
    connect(sixHoursBtn, &QPushButton::clicked, [this]() {
        hourSpin->setValue(6);
        minuteSpin->setValue(0);
        secondSpin->setValue(0);
    });
    
    QHBoxLayout *timeEditLayout = new QHBoxLayout();
    timeEditLayout->addWidget(new QLabel("时间:", this));
    timeEdit = new QTimeEdit(this);
    timeEdit->setTime(QTime::currentTime().addSecs(1800));
    timeEditLayout->addWidget(timeEdit);
    
    timeLayout->addWidget(countdownRadio);
    timeLayout->addLayout(presetLayout);
    timeLayout->addLayout(customLayout);
    timeLayout->addWidget(timeRadio);
    timeLayout->addLayout(timeEditLayout);
    timeGroup->setLayout(timeLayout);
    mainLayout->addWidget(timeGroup);
    
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    
    startButton = new QPushButton("开始", this);
    startButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_MediaPlay));
    startButton->setStyleSheet("QPushButton { background-color: #4caf50; color: white; padding: 12px; font-size: 14px; border-radius: 5px; } "
                               "QPushButton:hover { background-color: #43a047; }");
    connect(startButton, &QPushButton::clicked, this, &ShutdownWidget::onStartTimer);
    
    cancelButton = new QPushButton("取消", this);
    cancelButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogCancelButton));
    cancelButton->setStyleSheet("QPushButton { background-color: #f44336; color: white; padding: 12px; font-size: 14px; border-radius: 5px; } "
                                "QPushButton:hover { background-color: #d32f2f; }");
    cancelButton->setEnabled(false);
    connect(cancelButton, &QPushButton::clicked, this, &ShutdownWidget::onCancelTimer);
    
    buttonLayout->addWidget(startButton);
    buttonLayout->addWidget(cancelButton);
    
    mainLayout->addLayout(buttonLayout);
    
    QGroupBox *instantGroup = new QGroupBox("立即执行", this);
    QHBoxLayout *instantLayout = new QHBoxLayout();
    
    QPushButton *shutdownNowBtn = new QPushButton("立即关机", this);
    shutdownNowBtn->setStyleSheet("QPushButton { background-color: #ff5722; color: white; padding: 10px; }");
    connect(shutdownNowBtn, &QPushButton::clicked, this, &ShutdownWidget::onShutdownNow);
    
    QPushButton *restartNowBtn = new QPushButton("立即重启", this);
    restartNowBtn->setStyleSheet("QPushButton { background-color: #ff9800; color: white; padding: 10px; }");
    connect(restartNowBtn, &QPushButton::clicked, this, &ShutdownWidget::onRestartNow);
    
    QPushButton *sleepNowBtn = new QPushButton("立即休眠", this);
    sleepNowBtn->setStyleSheet("QPushButton { background-color: #9c27b0; color: white; padding: 10px; }");
    connect(sleepNowBtn, &QPushButton::clicked, this, &ShutdownWidget::onSleepNow);
    
    instantLayout->addWidget(shutdownNowBtn);
    instantLayout->addWidget(restartNowBtn);
    instantLayout->addWidget(sleepNowBtn);
    instantGroup->setLayout(instantLayout);
    mainLayout->addWidget(instantGroup);
    
    mainLayout->addStretch();
}

void ShutdownWidget::onStartTimer()
{
    currentAction = 0;
    if (restartRadio->isChecked()) currentAction = 1;
    if (sleepRadio->isChecked()) currentAction = 2;
    
    if (countdownRadio->isChecked()) {
        remainingSeconds = hourSpin->value() * 3600 + minuteSpin->value() * 60 + secondSpin->value();
    } else {
        QTime targetTime = timeEdit->time();
        QTime currentTime = QTime::currentTime();
        int secsTo = currentTime.secsTo(targetTime);
        if (secsTo <= 0) {
            secsTo += 86400;
        }
        remainingSeconds = secsTo;
    }
    
    if (remainingSeconds <= 0) {
        QMessageBox::warning(this, "提示", "请设置有效的时间！");
        return;
    }
    
    countdownTimer->start(1000);
    countdownLabel->show();
    startButton->setEnabled(false);
    cancelButton->setEnabled(true);
    
    QString actionText = "关机";
    if (currentAction == 1) actionText = "重启";
    if (currentAction == 2) actionText = "休眠";
    statusLabel->setText(QString("当前状态: 倒计时中 - %1").arg(actionText));
    statusLabel->setStyleSheet("font-size: 14px; padding: 10px; background-color: #fff3e0; border-radius: 5px;");
}

void ShutdownWidget::onCancelTimer()
{
    countdownTimer->stop();
    countdownLabel->hide();
    startButton->setEnabled(true);
    cancelButton->setEnabled(false);
    statusLabel->setText("当前状态: 空闲");
    statusLabel->setStyleSheet("font-size: 14px; padding: 10px; background-color: #e3f2fd; border-radius: 5px;");
    QMessageBox::information(this, "提示", "任务已取消！");
}

void ShutdownWidget::onTimerTick()
{
    remainingSeconds--;
    
    int hours = remainingSeconds / 3600;
    int minutes = (remainingSeconds % 3600) / 60;
    int seconds = remainingSeconds % 60;
    
    countdownLabel->setText(QString("%1:%2:%3")
                            .arg(hours, 2, 10, QChar('0'))
                            .arg(minutes, 2, 10, QChar('0'))
                            .arg(seconds, 2, 10, QChar('0')));
    
    if (remainingSeconds <= 0) {
        countdownTimer->stop();
        executeAction(currentAction);
    }
}

void ShutdownWidget::executeAction(int action)
{
    QString command;
    
    switch (action) {
        case 0: // 关机
            command = "shutdown /s /t 0";
            break;
        case 1: // 重启
            command = "shutdown /r /t 0";
            break;
        case 2: // 休眠
            command = "rundll32.exe powrprof.dll,SetSuspendState 0,1,0";
            break;
    }
    
    QProcess process;
    process.setProgram("cmd.exe");
    process.setArguments(QStringList() << "/c" << command);
    process.startDetached();
}

void ShutdownWidget::onShutdownNow()
{
    QProcess process;
    process.setProgram("cmd.exe");
    process.setArguments(QStringList() << "/c" << "shutdown /s /t 0");
    process.startDetached();
}

void ShutdownWidget::onRestartNow()
{
    QProcess process;
    process.setProgram("cmd.exe");
    process.setArguments(QStringList() << "/c" << "shutdown /r /t 0");
    process.startDetached();
}

void ShutdownWidget::onSleepNow()
{
    QProcess process;
    process.setProgram("cmd.exe");
    process.setArguments(QStringList() << "/c" << "rundll32.exe powrprof.dll,SetSuspendState 0,1,0");
    process.startDetached();
}
