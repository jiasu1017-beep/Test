#include "fishmodewidget.h"
#include <QApplication>
#include <QStyle>
#include <QShortcut>
#include <QKeySequence>

FishModeWidget::FishModeWidget(QWidget *parent)
    : QWidget(parent), isFishModeActive(false)
{
    setupUI();
    
    QShortcut *bossKeyShortcut = new QShortcut(QKeySequence(Qt::Key_F12), this);
    connect(bossKeyShortcut, &QShortcut::activated, this, &FishModeWidget::onBossKeyPressed);
}

void FishModeWidget::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    QLabel *titleLabel = new QLabel("摸鱼模式", this);
    titleLabel->setStyleSheet("font-size: 20px; font-weight: bold; padding: 10px;");
    mainLayout->addWidget(titleLabel);
    
    statusLabel = new QLabel("当前状态: 工作模式", this);
    statusLabel->setStyleSheet("font-size: 14px; padding: 10px; background-color: #e8f5e9; border-radius: 5px;");
    mainLayout->addWidget(statusLabel);
    
    QLabel *hintLabel = new QLabel("快捷键: F12 - 老板键", this);
    hintLabel->setStyleSheet("color: #666; padding: 5px;");
    mainLayout->addWidget(hintLabel);
    
    QVBoxLayout *buttonLayout = new QVBoxLayout();
    
    bossKeyButton = new QPushButton("老板键 (F12)", this);
    bossKeyButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_BrowserStop));
    bossKeyButton->setStyleSheet("QPushButton { background-color: #ff5722; color: white; padding: 15px; font-size: 16px; border-radius: 8px; } "
                                 "QPushButton:hover { background-color: #f4511e; }");
    connect(bossKeyButton, &QPushButton::clicked, this, &FishModeWidget::onBossKeyPressed);
    
    toggleButton = new QPushButton("切换摸鱼/工作模式", this);
    toggleButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogApplyButton));
    toggleButton->setStyleSheet("QPushButton { background-color: #2196f3; color: white; padding: 15px; font-size: 16px; border-radius: 8px; } "
                                 "QPushButton:hover { background-color: #1976d2; }");
    connect(toggleButton, &QPushButton::clicked, this, &FishModeWidget::onToggleFishMode);
    
    fakeWindowButton = new QPushButton("打开伪装窗口", this);
    fakeWindowButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_FileDialogContentsView));
    fakeWindowButton->setStyleSheet("QPushButton { background-color: #4caf50; color: white; padding: 15px; font-size: 16px; border-radius: 8px; } "
                                      "QPushButton:hover { background-color: #43a047; }");
    connect(fakeWindowButton, &QPushButton::clicked, this, &FishModeWidget::onOpenFakeWindow);
    
    buttonLayout->addWidget(bossKeyButton);
    buttonLayout->addSpacing(10);
    buttonLayout->addWidget(toggleButton);
    buttonLayout->addSpacing(10);
    buttonLayout->addWidget(fakeWindowButton);
    
    mainLayout->addLayout(buttonLayout);
    mainLayout->addStretch();
    
    QLabel *infoLabel = new QLabel("功能说明:\n"
                                    "• 老板键(F12): 快速隐藏所有窗口\n"
                                    "• 摸鱼模式: 切换工作/休闲状态\n"
                                    "• 伪装窗口: 打开系统工具伪装工作状态", this);
    infoLabel->setStyleSheet("color: #666; padding: 10px; background-color: #f5f5f5; border-radius: 5px;");
    mainLayout->addWidget(infoLabel);
}

void FishModeWidget::onBossKeyPressed()
{
    QMessageBox::information(this, "老板键", "老板键已触发！\n\n注意：完整的窗口隐藏功能需要调用Windows API来实现。\n当前是演示版本，实际使用建议添加更多窗口管理功能。");
}

void FishModeWidget::onToggleFishMode()
{
    isFishModeActive = !isFishModeActive;
    
    if (isFishModeActive) {
        statusLabel->setText("当前状态: 摸鱼模式 🏖️");
        statusLabel->setStyleSheet("font-size: 14px; padding: 10px; background-color: #fff3e0; border-radius: 5px;");
        QMessageBox::information(this, "摸鱼模式", "已切换到摸鱼模式！\n祝您摸鱼愉快！🐟");
    } else {
        statusLabel->setText("当前状态: 工作模式 💼");
        statusLabel->setStyleSheet("font-size: 14px; padding: 10px; background-color: #e8f5e9; border-radius: 5px;");
        QMessageBox::information(this, "工作模式", "已切换到工作模式！\n认真工作！💪");
    }
}

void FishModeWidget::onOpenFakeWindow()
{
    QProcess::startDetached("notepad.exe");
    QMessageBox::information(this, "伪装窗口", "已打开记事本作为伪装窗口！");
}
