#include "settingswidget.h"
#include <QApplication>
#include <QStyle>
#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QPixmap>

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
    
    autoStartCheck = new QCheckBox("开机自动启动小马办公", this);
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
    
    QLabel *aboutLabel = new QLabel("小马办公 v1.0\n\n"
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
    
    QLabel *copyrightLabel = new QLabel("© 2024 小马办公. All rights reserved.", this);
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
    QDialog aboutDialog(this);
    aboutDialog.setWindowTitle("关于小马办公");
    aboutDialog.setMinimumWidth(450);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(&aboutDialog);
    
    QLabel *titleLabel = new QLabel("小马办公 v1.0", &aboutDialog);
    titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #6200ea; padding: 10px;");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);
    
    QLabel *descLabel = new QLabel("一个功能完善的桌面办公助手应用", &aboutDialog);
    descLabel->setStyleSheet("font-size: 14px; color: #666; padding: 5px;");
    descLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(descLabel);
    
    QFrame *line1 = new QFrame(&aboutDialog);
    line1->setFrameShape(QFrame::HLine);
    line1->setFrameShadow(QFrame::Sunken);
    line1->setStyleSheet("color: #e0e0e0;");
    mainLayout->addWidget(line1);
    
    QLabel *featuresLabel = new QLabel("<b>主要功能:</b>", &aboutDialog);
    featuresLabel->setStyleSheet("font-size: 14px; padding: 10px 5px 5px;");
    mainLayout->addWidget(featuresLabel);
    
    QLabel *featuresContent = new QLabel(
        "• 应用管理 - 管理和快速启动常用应用<br>"
        "• 集合管理 - 自定义应用分组和批量启动<br>"
        "• 摸鱼模式 - 老板键和状态切换<br>"
        "• 定时关机 - 定时关机/重启/休眠<br>"
        "• 开机启动 - 设置开机自动运行", &aboutDialog);
    featuresContent->setStyleSheet("font-size: 13px; padding: 5px 20px; color: #555; line-height: 1.8;");
    mainLayout->addWidget(featuresContent);
    
    QFrame *line2 = new QFrame(&aboutDialog);
    line2->setFrameShape(QFrame::HLine);
    line2->setFrameShadow(QFrame::Sunken);
    line2->setStyleSheet("color: #e0e0e0;");
    mainLayout->addWidget(line2);
    
    QWidget *promoWidget = new QWidget(&aboutDialog);
    promoWidget->setStyleSheet(
        "background-color: #fff8e1; "
        "border: 2px solid #ffc107; "
        "border-radius: 10px; "
        "padding: 15px;"
    );
    QVBoxLayout *promoLayout = new QVBoxLayout(promoWidget);
    
    QLabel *promoTitle = new QLabel("📢 关注我们", &aboutDialog);
    promoTitle->setStyleSheet("font-size: 18px; font-weight: bold; color: #e65100;");
    promoTitle->setAlignment(Qt::AlignCenter);
    promoLayout->addWidget(promoTitle);
    
    QLabel *promoDesc = new QLabel(
        "欢迎关注微信公众号<br>"
        "<span style='font-size: 20px; font-weight: bold; color: #d32f2f;'>梁柱墙笔记</span><br><br>"
        "📚 获取更多办公效率技巧<br>"
        "💡 学习实用软件开发知识<br>"
        "🎁 不定期分享优质资源", &aboutDialog);
    promoDesc->setStyleSheet("font-size: 14px; color: #5d4037; line-height: 1.8;");
    promoDesc->setAlignment(Qt::AlignCenter);
    promoDesc->setWordWrap(true);
    promoLayout->addWidget(promoDesc);
    
    QLabel *qrLabel = new QLabel(&aboutDialog);
    qrLabel->setAlignment(Qt::AlignCenter);
    QPixmap qrPixmap(":/img/wechater.jpg");
    if (!qrPixmap.isNull()) {
        qrPixmap = qrPixmap.scaled(180, 180, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        qrLabel->setPixmap(qrPixmap);
    } else {
        qrLabel->setText("[二维码加载失败]");
        qrLabel->setStyleSheet(
            "background-color: #fff; "
            "border: 2px dashed #ffc107; "
            "border-radius: 8px; "
            "padding: 30px; "
            "color: #999; "
            "font-size: 12px;"
        );
    }
    qrLabel->setMinimumHeight(180);
    promoLayout->addWidget(qrLabel);
    
    mainLayout->addWidget(promoWidget);
    
    QFrame *line3 = new QFrame(&aboutDialog);
    line3->setFrameShape(QFrame::HLine);
    line3->setFrameShadow(QFrame::Sunken);
    line3->setStyleSheet("color: #e0e0e0;");
    mainLayout->addWidget(line3);
    
    QLabel *techLabel = new QLabel("<b>技术栈:</b>", &aboutDialog);
    techLabel->setStyleSheet("font-size: 14px; padding: 10px 5px 5px;");
    mainLayout->addWidget(techLabel);
    
    QLabel *techContent = new QLabel(
        "• Qt 5.15.2<br>"
        "• JSON 数据存储<br>"
        "• MinGW 8.1.0 编译器", &aboutDialog);
    techContent->setStyleSheet("font-size: 13px; padding: 5px 20px; color: #555; line-height: 1.8;");
    mainLayout->addWidget(techContent);
    
    QLabel *copyrightLabel = new QLabel("© 2024 小马办公. All rights reserved.", &aboutDialog);
    copyrightLabel->setStyleSheet("color: #999; padding: 15px; font-size: 12px;");
    copyrightLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(copyrightLabel);
    
    QPushButton *closeButton = new QPushButton("关闭", &aboutDialog);
    closeButton->setStyleSheet(
        "QPushButton { background-color: #6200ea; color: white; padding: 10px 30px; border-radius: 5px; font-weight: bold; } "
        "QPushButton:hover { background-color: #7c43bd; }"
    );
    connect(closeButton, &QPushButton::clicked, &aboutDialog, &QDialog::accept);
    
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(closeButton);
    buttonLayout->addStretch();
    mainLayout->addLayout(buttonLayout);
    
    aboutDialog.exec();
}
