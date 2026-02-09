#include "settingswidget.h"
#include <QApplication>
#include <QStyle>

SettingsWidget::SettingsWidget(Database *db, QWidget *parent)
    : QWidget(parent), db(db)
{
    setupUI();
}

void SettingsWidget::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    QLabel *titleLabel = new QLabel("设置", this);
    titleLabel->setStyleSheet("font-size: 20px; font-weight: bold; padding: 10px;");
    mainLayout->addWidget(titleLabel);
    
    QGroupBox *startupGroup = new QGroupBox("开机启动", this);
    QVBoxLayout *startupLayout = new QVBoxLayout();
    
    autoStartCheck = new QCheckBox("开机自动启动办公助手", this);
    autoStartCheck->setChecked(db->getAutoStart());
    connect(autoStartCheck, &QCheckBox::stateChanged, this, &SettingsWidget::onAutoStartToggled);
    
    statusLabel = new QLabel();
    statusLabel->setStyleSheet("padding: 5px;");
    if (db->getAutoStart()) {
        statusLabel->setText("当前状态: 已启用 ✓");
        statusLabel->setStyleSheet("padding: 5px; color: #4caf50;");
    } else {
        statusLabel->setText("当前状态: 已禁用");
        statusLabel->setStyleSheet("padding: 5px; color: #f44336;");
    }
    
    startupLayout->addWidget(autoStartCheck);
    startupLayout->addWidget(statusLabel);
    startupGroup->setLayout(startupLayout);
    mainLayout->addWidget(startupGroup);
    
    QGroupBox *aboutGroup = new QGroupBox("关于", this);
    QVBoxLayout *aboutLayout = new QVBoxLayout();
    
    QLabel *aboutLabel = new QLabel("办公助手 v1.0\n\n"
                                      "一个功能完善的桌面办公助手应用\n"
                                      "• 应用管理模块\n"
                                      "• 摸鱼模式模块\n"
                                      "• 定时关机模块\n"
                                      "• 开机自动启动\n\n"
                                      "使用 Qt 5.15.2 开发", this);
    aboutLabel->setStyleSheet("padding: 10px; line-height: 1.6;");
    aboutLabel->setWordWrap(true);
    
    QPushButton *aboutButton = new QPushButton("关于", this);
    aboutButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_FileDialogInfoView));
    connect(aboutButton, &QPushButton::clicked, this, &SettingsWidget::onAboutClicked);
    
    aboutLayout->addWidget(aboutLabel);
    aboutLayout->addWidget(aboutButton);
    aboutGroup->setLayout(aboutLayout);
    mainLayout->addWidget(aboutGroup);
    
    mainLayout->addStretch();
    
    QLabel *copyrightLabel = new QLabel("© 2024 办公助手. All rights reserved.", this);
    copyrightLabel->setStyleSheet("color: #999; padding: 10px; text-align: center;");
    copyrightLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(copyrightLabel);
}

void SettingsWidget::onAutoStartToggled(int state)
{
    bool enabled = (state == Qt::Checked);
    
    if (db->setAutoStart(enabled)) {
        if (enabled) {
            statusLabel->setText("当前状态: 已启用 ✓");
            statusLabel->setStyleSheet("padding: 5px; color: #4caf50;");
            QMessageBox::information(this, "成功", "开机自动启动已启用！");
        } else {
            statusLabel->setText("当前状态: 已禁用");
            statusLabel->setStyleSheet("padding: 5px; color: #f44336;");
            QMessageBox::information(this, "成功", "开机自动启动已禁用！");
        }
    } else {
        QMessageBox::warning(this, "错误", "设置开机自动启动失败！");
        autoStartCheck->setChecked(!enabled);
    }
}

void SettingsWidget::onAboutClicked()
{
    QMessageBox::about(this, "关于办公助手",
                      "办公助手 v1.0\n\n"
                      "一个功能完善的桌面办公助手应用\n\n"
                      "主要功能:\n"
                      "• 应用管理 - 管理和快速启动常用应用\n"
                      "• 摸鱼模式 - 老板键和状态切换\n"
                      "• 定时关机 - 定时关机/重启/休眠\n"
                      "• 开机启动 - 设置开机自动运行\n\n"
                      "技术栈:\n"
                      "• Qt 5.15.2\n"
                      "• SQLite 数据库\n"
                      "• MinGW 编译器\n\n"
                      "© 2024 办公助手");
}
