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
#include <QScrollArea>
#include "updatedialog.h"
#include "updateprogressdialog.h"

SettingsWidget::SettingsWidget(Database *db, QWidget *parent)
    : QWidget(parent), db(db), updateManager(nullptr), progressDialog(nullptr)
{
    setupUI();
}

void SettingsWidget::setUpdateManager(UpdateManager *manager)
{
    updateManager = manager;
    
    if (updateManager) {
        connect(updateManager, &UpdateManager::noUpdateAvailable, this, &SettingsWidget::onNoUpdateAvailable);
        connect(updateManager, &UpdateManager::updateCheckFailed, this, &SettingsWidget::onUpdateCheckFailed);
    }
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
    
    QGroupBox *closeBehaviorGroup = new QGroupBox("关闭行为", this);
    QVBoxLayout *closeBehaviorLayout = new QVBoxLayout();
    
    minimizeToTrayCheck = new QCheckBox("启用最小化到系统托盘", this);
    minimizeToTrayCheck->setChecked(db->getMinimizeToTray());
    connect(minimizeToTrayCheck, &QCheckBox::stateChanged, this, &SettingsWidget::onMinimizeToTrayToggled);
    
    showClosePromptCheck = new QCheckBox("关闭窗口时显示提示", this);
    showClosePromptCheck->setChecked(db->getShowClosePrompt());
    connect(showClosePromptCheck, &QCheckBox::stateChanged, this, &SettingsWidget::onShowClosePromptToggled);
    
    QLabel *closeBehaviorLabel = new QLabel("当前关闭行为: " + QString(db->getMinimizeToTray() ? "最小化到系统托盘" : "直接退出程序"), this);
    closeBehaviorLabel->setStyleSheet("padding: 5px; color: #2196f3;");
    closeBehaviorLabel->setObjectName("closeBehaviorLabel");
    
    closeBehaviorLayout->addWidget(minimizeToTrayCheck);
    closeBehaviorLayout->addWidget(showClosePromptCheck);
    closeBehaviorLayout->addWidget(closeBehaviorLabel);
    closeBehaviorGroup->setLayout(closeBehaviorLayout);
    mainLayout->addWidget(closeBehaviorGroup);
    
    QGroupBox *updateGroup = new QGroupBox("自动更新", this);
    QVBoxLayout *updateLayout = new QVBoxLayout();
    
    autoCheckUpdateCheck = new QCheckBox("自动检查更新", this);
    autoCheckUpdateCheck->setChecked(db->getAutoCheckUpdate());
    connect(autoCheckUpdateCheck, &QCheckBox::stateChanged, this, &SettingsWidget::onAutoCheckUpdateToggled);
    
    QLabel *updateInfoLabel = new QLabel("启用后，软件启动时和后台每24小时会自动检查更新。", this);
    updateInfoLabel->setStyleSheet("padding: 5px; color: #666; font-size: 12px;");
    updateInfoLabel->setWordWrap(true);
    
    checkUpdateButton = new QPushButton("检查更新", this);
    checkUpdateButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_BrowserReload));
    checkUpdateButton->setStyleSheet(
        "QPushButton { background-color: #2196f3; color: white; padding: 10px 20px; border-radius: 5px; font-weight: bold; } "
        "QPushButton:hover { background-color: #1976d2; } "
        "QPushButton:pressed { background-color: #1565c0; }"
    );
    connect(checkUpdateButton, &QPushButton::clicked, this, &SettingsWidget::onCheckUpdateClicked);
    
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(checkUpdateButton);
    buttonLayout->addStretch();
    
    updateLayout->addWidget(autoCheckUpdateCheck);
    updateLayout->addWidget(updateInfoLabel);
    updateLayout->addSpacing(10);
    updateLayout->addLayout(buttonLayout);
    updateGroup->setLayout(updateLayout);
    mainLayout->addWidget(updateGroup);
    
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
    
    QLabel *copyrightLabel = new QLabel("© 2026 小马办公. All rights reserved.", this);
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
    aboutDialog.setMinimumWidth(480);
    aboutDialog.setMinimumHeight(600);
    aboutDialog.setMaximumHeight(900);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(&aboutDialog);
    
    QScrollArea *scrollArea = new QScrollArea(&aboutDialog);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    
    QWidget *contentWidget = new QWidget();
    QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setSpacing(12);
    contentLayout->setContentsMargins(20, 20, 20, 20);
    
    QLabel *titleLabel = new QLabel("小马办公 v1.0", contentWidget);
    titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #6200ea; padding: 10px;");
    titleLabel->setAlignment(Qt::AlignCenter);
    contentLayout->addWidget(titleLabel);
    
    QLabel *picLabel = new QLabel(contentWidget);
    picLabel->setAlignment(Qt::AlignCenter);
    QPixmap picPixmap(":/img/pic.png");
    if (!picPixmap.isNull()) {
        picPixmap = picPixmap.scaled(220, 220, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        picLabel->setPixmap(picPixmap);
    } else {
        picLabel->setText("[插图加载失败]");
        picLabel->setStyleSheet(
            "background-color: #f5f5f5; "
            "border: 2px dashed #ccc; "
            "border-radius: 8px; "
            "padding: 50px; "
            "color: #999; "
            "font-size: 12px;"
        );
    }
    picLabel->setMinimumHeight(180);
    contentLayout->addWidget(picLabel);
    
    QLabel *descLabel = new QLabel("一个功能完善的桌面办公助手应用", contentWidget);
    descLabel->setStyleSheet("font-size: 14px; color: #666; padding: 5px;");
    descLabel->setAlignment(Qt::AlignCenter);
    contentLayout->addWidget(descLabel);
    
    QFrame *line1 = new QFrame(contentWidget);
    line1->setFrameShape(QFrame::HLine);
    line1->setFrameShadow(QFrame::Sunken);
    line1->setStyleSheet("color: #e0e0e0;");
    contentLayout->addWidget(line1);
    
    QLabel *featuresLabel = new QLabel("<b>主要功能:</b>", contentWidget);
    featuresLabel->setStyleSheet("font-size: 14px; padding: 10px 5px 5px;");
    contentLayout->addWidget(featuresLabel);
    
    QLabel *featuresContent = new QLabel(
        "• 应用管理 - 管理和快速启动常用应用<br>"
        "• 集合管理 - 自定义应用分组和批量启动<br>"
        "• 摸鱼模式 - 老板键和状态切换<br>"
        "• 定时关机 - 定时关机/重启/休眠<br>"
        "• 开机启动 - 设置开机自动运行", contentWidget);
    featuresContent->setStyleSheet("font-size: 13px; padding: 5px 20px; color: #555; line-height: 1.8;");
    contentLayout->addWidget(featuresContent);
    
    QFrame *line2 = new QFrame(contentWidget);
    line2->setFrameShape(QFrame::HLine);
    line2->setFrameShadow(QFrame::Sunken);
    line2->setStyleSheet("color: #e0e0e0;");
    contentLayout->addWidget(line2);
    
    QWidget *promoWidget = new QWidget(contentWidget);
    promoWidget->setStyleSheet(
        "background-color: #fff8e1; "
        "border: 2px solid #ffc107; "
        "border-radius: 10px; "
        "padding: 15px;"
    );
    QVBoxLayout *promoLayout = new QVBoxLayout(promoWidget);
    
    QLabel *promoTitle = new QLabel("📢 关注我们", promoWidget);
    promoTitle->setStyleSheet("font-size: 18px; font-weight: bold; color: #e65100;");
    promoTitle->setAlignment(Qt::AlignCenter);
    promoLayout->addWidget(promoTitle);
    
    QLabel *promoDesc = new QLabel(
        "欢迎关注微信公众号<br>"
        "<span style='font-size: 20px; font-weight: bold; color: #d32f2f;'>梁柱墙笔记</span><br><br>"
        "📚 获取更多办公效率技巧<br>"
        "💡 学习实用软件开发知识<br>"
        "🎁 不定期分享优质资源", promoWidget);
    promoDesc->setStyleSheet("font-size: 14px; color: #5d4037; line-height: 1.8;");
    promoDesc->setAlignment(Qt::AlignCenter);
    promoDesc->setWordWrap(true);
    promoLayout->addWidget(promoDesc);
    
    QLabel *qrLabel = new QLabel(promoWidget);
    qrLabel->setAlignment(Qt::AlignCenter);
    QPixmap qrPixmap(":/img/wechater.jpg");
    if (!qrPixmap.isNull()) {
        qrPixmap = qrPixmap.scaled(160, 160, Qt::KeepAspectRatio, Qt::SmoothTransformation);
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
    qrLabel->setMinimumHeight(160);
    promoLayout->addWidget(qrLabel);
    
    contentLayout->addWidget(promoWidget);
    
    QFrame *line3 = new QFrame(contentWidget);
    line3->setFrameShape(QFrame::HLine);
    line3->setFrameShadow(QFrame::Sunken);
    line3->setStyleSheet("color: #e0e0e0;");
    contentLayout->addWidget(line3);
    
    QLabel *techLabel = new QLabel("<b>技术栈:</b>", contentWidget);
    techLabel->setStyleSheet("font-size: 14px; padding: 10px 5px 5px;");
    contentLayout->addWidget(techLabel);
    
    QLabel *techContent = new QLabel(
        "• Qt 5.15.2<br>"
        "• JSON 数据存储<br>"
        "• MinGW 8.1.0 编译器", contentWidget);
    techContent->setStyleSheet("font-size: 13px; padding: 5px 20px; color: #555; line-height: 1.8;");
    contentLayout->addWidget(techContent);
    
    QLabel *copyrightLabel = new QLabel("© 2024 小马办公. All rights reserved.", contentWidget);
    copyrightLabel->setStyleSheet("color: #999; padding: 15px; font-size: 12px;");
    copyrightLabel->setAlignment(Qt::AlignCenter);
    contentLayout->addWidget(copyrightLabel);
    
    contentLayout->addStretch();
    
    scrollArea->setWidget(contentWidget);
    mainLayout->addWidget(scrollArea);
    
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

void SettingsWidget::onAutoCheckUpdateToggled(int state)
{
    bool enabled = (state == Qt::Checked);
    
    if (db->setAutoCheckUpdate(enabled)) {
        QMessageBox::information(this, "成功", QString("自动检查更新已%1！").arg(enabled ? "启用" : "禁用"));
    } else {
        QMessageBox::warning(this, "错误", "设置失败！");
        autoCheckUpdateCheck->setChecked(!enabled);
    }
}

void SettingsWidget::onMinimizeToTrayToggled(int state)
{
    bool enabled = (state == Qt::Checked);
    
    if (db->setMinimizeToTray(enabled)) {
        QLabel *label = findChild<QLabel*>("closeBehaviorLabel");
        if (label) {
            label->setText("当前关闭行为: " + QString(enabled ? "最小化到系统托盘" : "直接退出程序"));
        }
        QMessageBox::information(this, "成功", QString("最小化到系统托盘已%1！").arg(enabled ? "启用" : "禁用"));
    } else {
        QMessageBox::warning(this, "错误", "设置失败！");
        minimizeToTrayCheck->setChecked(!enabled);
    }
}

void SettingsWidget::onShowClosePromptToggled(int state)
{
    bool show = (state == Qt::Checked);
    
    if (db->setShowClosePrompt(show)) {
        QMessageBox::information(this, "成功", QString("关闭提示已%1！").arg(show ? "启用" : "禁用"));
    } else {
        QMessageBox::warning(this, "错误", "设置失败！");
        showClosePromptCheck->setChecked(!show);
    }
}

void SettingsWidget::onCheckUpdateClicked()
{
    if (!updateManager) {
        QMessageBox::warning(this, "错误", "更新管理器未初始化！");
        return;
    }
    
    QMessageBox::information(this, "调试", "开始检查更新...");
    
    progressDialog = new QProgressDialog(this);
    progressDialog->setWindowTitle("检查更新");
    progressDialog->setLabelText("正在检查更新...");
    progressDialog->setCancelButton(nullptr);
    progressDialog->setRange(0, 0);
    progressDialog->setWindowModality(Qt::WindowModal);
    progressDialog->show();
    
    checkUpdateButton->setEnabled(false);
    updateManager->checkForUpdates();
}

void SettingsWidget::onUpdateAvailable(const UpdateInfo &info)
{
    if (progressDialog) {
        progressDialog->close();
        progressDialog->deleteLater();
        progressDialog = nullptr;
    }
    
    checkUpdateButton->setEnabled(true);
    
    UpdateDialog *dialog = new UpdateDialog(info, this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    connect(dialog, &UpdateDialog::updateNow, [this, info]() {
        Q_UNUSED(info);
        if (updateManager) {
            updateProgressDialog = new UpdateProgressDialog(this);
            updateProgressDialog->setUpdateManager(updateManager);
            updateProgressDialog->show();
            
            connect(updateManager, &UpdateManager::downloadFinished, this, [this](const QString &filePath) {
                updateManager->installUpdate(filePath);
            });
            
            updateManager->downloadUpdate();
        }
    });
    connect(dialog, &UpdateDialog::remindLater, [this]() {
        if (updateManager) {
            updateManager->remindLater();
        }
    });
    connect(dialog, &UpdateDialog::skipThisVersion, [this]() {
        if (updateManager) {
            updateManager->skipThisVersion();
            db->setIgnoredVersion(updateManager->ignoredVersion());
        }
    });
    dialog->show();
}

void SettingsWidget::onNoUpdateAvailable()
{
    if (progressDialog) {
        progressDialog->close();
        progressDialog->deleteLater();
        progressDialog = nullptr;
    }
    
    checkUpdateButton->setEnabled(true);
    
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("检查更新");
    msgBox.setText("当前已是最新版本！");
    msgBox.setInformativeText(QString("当前版本: v%1").arg(qApp->applicationVersion()));
    msgBox.setIcon(QMessageBox::Information);
    msgBox.addButton("确定", QMessageBox::AcceptRole);
    msgBox.exec();
}

void SettingsWidget::onUpdateCheckFailed(const QString &error)
{
    if (progressDialog) {
        progressDialog->close();
        progressDialog->deleteLater();
        progressDialog = nullptr;
    }
    
    checkUpdateButton->setEnabled(true);
    
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("检查更新失败");
    msgBox.setText("无法检查更新");
    msgBox.setInformativeText(QString("错误信息: %1\n\n请检查网络连接后重试。").arg(error));
    msgBox.setIcon(QMessageBox::Warning);
    QPushButton *retryButton = msgBox.addButton("重试", QMessageBox::ActionRole);
    QPushButton *closeButton = msgBox.addButton("关闭", QMessageBox::RejectRole);
    msgBox.setDefaultButton(retryButton);
    
    msgBox.exec();
    
    if (msgBox.clickedButton() == retryButton) {
        onCheckUpdateClicked();
    }
}
