#include "settingswidget.h"
#include "mainwindow.h"
#include "modules/dialogs/shortcutdialog.h"
#include "modules/dialogs/aisettingsdialog.h"
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
#include <QMessageBox>
#include <QListWidget>
#include <QStackedWidget>
#include <QTextEdit>
#include <QFontComboBox>
#include <QSpinBox>
#include <QSlider>
#include <QRadioButton>
#include "modules/update/updatedialog.h"
#include "modules/update/updateprogressdialog.h"

SettingsWidget::SettingsWidget(Database *db, QWidget *parent)
    : QWidget(parent), db(db), mainWindow(nullptr), updateManager(nullptr), progressDialog(nullptr)
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

void SettingsWidget::setMainWindow(MainWindow *mw)
{
    mainWindow = mw;
}

void SettingsWidget::setupUI()
{
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    QWidget *leftWidget = new QWidget(this);
    leftWidget->setFixedWidth(220);
    leftWidget->setStyleSheet("background-color: #f5f5f5;");
    QVBoxLayout *leftLayout = new QVBoxLayout(leftWidget);
    leftLayout->setContentsMargins(10, 20, 10, 20);
    leftLayout->setSpacing(5);

    QLabel *titleLabel = new QLabel("设置", leftWidget);
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; padding: 10px 15px; color: #333;");
    leftLayout->addWidget(titleLabel);

    QListWidget *listWidget = new QListWidget(leftWidget);
    listWidget->setFrameShape(QListWidget::NoFrame);
    listWidget->setBackgroundRole(QPalette::NoRole);
    listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    listWidget->setStyleSheet(
        "QListWidget { border: none; background: transparent; } "
        "QListWidget::item { padding: 12px 15px; border-radius: 6px; margin: 2px 5px; color: #555; } "
        "QListWidget::item:selected { background-color: #e3f2fd; color: #1976d2; font-weight: bold; } "
        "QListWidget::item:hover { background-color: #eeeeee; }"
    );

    QListWidgetItem *generalItem = new QListWidgetItem("🔧 通用设置", listWidget);
    generalItem->setData(Qt::UserRole, 0);
    
    QListWidgetItem *shortcutItem = new QListWidgetItem("⌨️ 快捷键", listWidget);
    shortcutItem->setData(Qt::UserRole, 1);
    
    QListWidgetItem *aiItem = new QListWidgetItem("🤖 AI设置", listWidget);
    aiItem->setData(Qt::UserRole, 2);
    
    QListWidgetItem *startupItem = new QListWidgetItem("🚀 开机启动", listWidget);
    startupItem->setData(Qt::UserRole, 3);
    
    QListWidgetItem *updateItem = new QListWidgetItem("🔄 检查更新", listWidget);
    updateItem->setData(Qt::UserRole, 4);
    
    QListWidgetItem *aboutItem = new QListWidgetItem("ℹ️ 关于", listWidget);
    aboutItem->setData(Qt::UserRole, 5);

    listWidget->setCurrentRow(0);
    leftLayout->addWidget(listWidget);
    
    leftLayout->addStretch();

    QStackedWidget *stackWidget = new QStackedWidget(this);
    stackWidget->setStyleSheet("background-color: white;");

    QWidget *generalPage = createGeneralPage();
    QWidget *shortcutPage = createShortcutPage();
    QWidget *aiPage = createAIPage();
    QWidget *startupPage = createStartupPage();
    QWidget *updatePage = createUpdatePage();
    QWidget *aboutPage = createAboutPage();

    stackWidget->addWidget(generalPage);
    stackWidget->addWidget(shortcutPage);
    stackWidget->addWidget(aiPage);
    stackWidget->addWidget(startupPage);
    stackWidget->addWidget(updatePage);
    stackWidget->addWidget(aboutPage);

    connect(listWidget, &QListWidget::currentRowChanged, stackWidget, &QStackedWidget::setCurrentIndex);

    mainLayout->addWidget(leftWidget);
    mainLayout->addWidget(stackWidget, 1);

    QFrame *splitter = new QFrame(this);
    splitter->setFrameShape(QFrame::VLine);
    splitter->setStyleSheet("color: #e0e0e0;");
    mainLayout->addWidget(splitter);
}

QWidget *SettingsWidget::createGeneralPage()
{
    QWidget *page = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setContentsMargins(30, 30, 30, 30);
    layout->setSpacing(20);

    QLabel *title = new QLabel("通用设置", page);
    title->setStyleSheet("font-size: 24px; font-weight: bold; color: #333;");
    layout->addWidget(title);

    QFrame *line = new QFrame(page);
    line->setFrameShape(QFrame::HLine);
    line->setStyleSheet("color: #e0e0e0;");
    layout->addWidget(line);

    QGroupBox *closeGroup = new QGroupBox("关闭行为", page);
    QVBoxLayout *closeLayout = new QVBoxLayout(closeGroup);
    closeLayout->setSpacing(10);

    minimizeToTrayCheck = new QCheckBox("启用最小化到系统托盘", page);
    minimizeToTrayCheck->setChecked(db->getMinimizeToTray());
    connect(minimizeToTrayCheck, &QCheckBox::stateChanged, this, &SettingsWidget::onMinimizeToTrayToggled);
    closeLayout->addWidget(minimizeToTrayCheck);

    showClosePromptCheck = new QCheckBox("关闭窗口时显示提示", page);
    showClosePromptCheck->setChecked(db->getShowClosePrompt());
    connect(showClosePromptCheck, &QCheckBox::stateChanged, this, &SettingsWidget::onShowClosePromptToggled);
    closeLayout->addWidget(showClosePromptCheck);

    QLabel *closeHint = new QLabel("当前关闭行为: " + QString(db->getMinimizeToTray() ? "最小化到系统托盘" : "直接退出程序"), page);
    closeHint->setStyleSheet("color: #666; font-size: 12px; padding: 5px;");
    closeLayout->addWidget(closeHint);

    layout->addWidget(closeGroup);

    QGroupBox *appearanceGroup = new QGroupBox("外观", page);
    QVBoxLayout *appearLayout = new QVBoxLayout(appearanceGroup);
    appearLayout->setSpacing(10);

    QHBoxLayout *themeLayout = new QHBoxLayout();
    QLabel *themeLabel = new QLabel("主题:", page);
    QComboBox *themeCombo = new QComboBox(page);
    themeCombo->addItems({"浅色", "深色", "跟随系统"});
    themeCombo->setCurrentText("浅色");
    themeLayout->addWidget(themeLabel);
    themeLayout->addWidget(themeCombo);
    themeLayout->addStretch();
    appearLayout->addLayout(themeLayout);

    QLabel *themeHint = new QLabel("主题切换功能开发中...", page);
    themeHint->setStyleSheet("color: #999; font-size: 12px; padding: 5px;");
    appearLayout->addWidget(themeHint);

    layout->addWidget(appearanceGroup);

    QGroupBox *trayGroup = new QGroupBox("系统托盘", page);
    QVBoxLayout *trayLayout = new QVBoxLayout(trayGroup);
    trayLayout->setSpacing(10);

    QCheckBox *showTrayIcon = new QCheckBox("显示托盘图标", trayGroup);
    showTrayIcon->setChecked(true);
    trayLayout->addWidget(showTrayIcon);

    QCheckBox *showTrayTooltip = new QCheckBox("显示托盘提示", trayGroup);
    showTrayTooltip->setChecked(true);
    trayLayout->addWidget(showTrayTooltip);

    layout->addWidget(trayGroup);

    layout->addStretch();
    return page;
}

QWidget *SettingsWidget::createShortcutPage()
{
    QWidget *page = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setContentsMargins(30, 30, 30, 30);
    layout->setSpacing(20);

    QLabel *title = new QLabel("快捷键设置", page);
    title->setStyleSheet("font-size: 24px; font-weight: bold; color: #333;");
    layout->addWidget(title);

    QFrame *line = new QFrame(page);
    line->setFrameShape(QFrame::HLine);
    line->setStyleSheet("color: #e0e0e0;");
    layout->addWidget(line);

    QGroupBox *globalShortcutGroup = new QGroupBox("全局快捷键", page);
    QVBoxLayout *shortcutLayout = new QVBoxLayout(globalShortcutGroup);
    shortcutLayout->setSpacing(15);

    QLabel *descLabel = new QLabel("设置用于调出窗口的全局快捷键，软件在后台运行时也能响应。", page);
    descLabel->setStyleSheet("color: #666; font-size: 13px; padding: 5px;");
    descLabel->setWordWrap(true);
    shortcutLayout->addWidget(descLabel);

    QHBoxLayout *shortcutInputLayout = new QHBoxLayout();
    QLabel *shortcutLabel = new QLabel("快捷键:", page);
    QLabel *currentShortcutLabel = new QLabel(db->getShortcutKey(), page);
    currentShortcutLabel->setStyleSheet(
        "padding: 8px 15px; border: 1px solid #ddd; border-radius: 5px; "
        "background-color: #f9f9f9; font-weight: bold; min-width: 100px;"
    );

    QPushButton *changeShortcutBtn = new QPushButton("修改", page);
    changeShortcutBtn->setStyleSheet(
        "QPushButton { background-color: #2196f3; color: white; padding: 8px 20px; border-radius: 5px; } "
        "QPushButton:hover { background-color: #1976d2; }"
    );
    connect(changeShortcutBtn, &QPushButton::clicked, this, [this, currentShortcutLabel]() {
        ShortcutDialog dialog(db, this);
        dialog.setShortcut(QKeySequence(db->getShortcutKey()));
        
        if (mainWindow && !mainWindow->isVisible()) {
            mainWindow->show();
            mainWindow->activateWindow();
        }
        
        if (dialog.exec() == QDialog::Accepted) {
            QKeySequence newShortcut = dialog.getShortcut();
            if (!newShortcut.isEmpty()) {
                QString shortcutStr = newShortcut.toString();
                if (db->setShortcutKey(shortcutStr)) {
                    currentShortcutLabel->setText(shortcutStr);
                    QMessageBox::information(this, "成功", "快捷键已更新: " + shortcutStr);
                    
                    if (mainWindow) {
                        mainWindow->refreshGlobalShortcut();
                    }
                } else {
                    QMessageBox::warning(this, "错误", "保存快捷键失败！");
                }
            }
        }
    });

    QPushButton *resetShortcutBtn = new QPushButton("重置为默认", page);
    resetShortcutBtn->setStyleSheet(
        "QPushButton { background-color: #9e9e9e; color: white; padding: 8px 20px; border-radius: 5px; } "
        "QPushButton:hover { background-color: #757575; }"
    );
    connect(resetShortcutBtn, &QPushButton::clicked, this, [this, currentShortcutLabel]() {
        if (db->setShortcutKey("Ctrl+W")) {
            currentShortcutLabel->setText("Ctrl+W");
            QMessageBox::information(this, "成功", "快捷键已重置为默认: Ctrl+W");
            
            if (mainWindow) {
                mainWindow->refreshGlobalShortcut();
            }
        }
    });

    shortcutInputLayout->addWidget(shortcutLabel);
    shortcutInputLayout->addWidget(currentShortcutLabel);
    shortcutInputLayout->addWidget(changeShortcutBtn);
    shortcutInputLayout->addWidget(resetShortcutBtn);
    shortcutInputLayout->addStretch();
    shortcutLayout->addLayout(shortcutInputLayout);

    QLabel *hintLabel = new QLabel("💡 提示: 快捷键在软件最小化或处于后台时仍然有效。窗口激活时按快捷键会最小化窗口。", page);
    hintLabel->setStyleSheet("color: #888; font-size: 12px; padding: 10px; background-color: #f5f5f5; border-radius: 5px;");
    hintLabel->setWordWrap(true);
    shortcutLayout->addWidget(hintLabel);

    layout->addWidget(globalShortcutGroup);

    layout->addStretch();
    return page;
}

QWidget *SettingsWidget::createAIPage()
{
    QWidget *page = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setContentsMargins(30, 30, 30, 30);
    layout->setSpacing(20);

    QLabel *title = new QLabel("AI设置", page);
    title->setStyleSheet("font-size: 24px; font-weight: bold; color: #333;");
    layout->addWidget(title);

    QFrame *line = new QFrame(page);
    line->setFrameShape(QFrame::HLine);
    line->setStyleSheet("color: #e0e0e0;");
    layout->addWidget(line);

    QGroupBox *aiConfigGroup = new QGroupBox("AI配置", page);
    QVBoxLayout *aiConfigLayout = new QVBoxLayout(aiConfigGroup);
    aiConfigLayout->setSpacing(15);

    QLabel *aiDescLabel = new QLabel("配置AI模型以启用智能任务分析和报告生成功能", page);
    aiDescLabel->setStyleSheet("color: #666; font-size: 13px; padding: 5px;");
    aiDescLabel->setWordWrap(true);
    aiConfigLayout->addWidget(aiDescLabel);

    QPushButton *openAISettingsBtn = new QPushButton("🔧 打开AI设置", page);
    openAISettingsBtn->setStyleSheet(
        "QPushButton { background-color: #3498db; color: white; padding: 12px 24px; border-radius: 5px; font-size: 14px; } "
        "QPushButton:hover { background-color: #2980b9; }"
    );
    connect(openAISettingsBtn, &QPushButton::clicked, this, &SettingsWidget::onOpenAISettings);
    aiConfigLayout->addWidget(openAISettingsBtn);

    layout->addWidget(aiConfigGroup);

    QGroupBox *aiFeaturesGroup = new QGroupBox("AI功能", page);
    QVBoxLayout *aiFeaturesLayout = new QVBoxLayout(aiFeaturesGroup);
    aiFeaturesLayout->setSpacing(10);

    QCheckBox *taskAnalysisCheck = new QCheckBox("启用智能任务分析", aiFeaturesGroup);
    taskAnalysisCheck->setChecked(true);
    aiFeaturesLayout->addWidget(taskAnalysisCheck);

    QCheckBox *reportGenCheck = new QCheckBox("启用AI报告生成", aiFeaturesGroup);
    reportGenCheck->setChecked(true);
    aiFeaturesLayout->addWidget(reportGenCheck);

    QCheckBox *autoSuggestCheck = new QCheckBox("启用智能建议", aiFeaturesGroup);
    autoSuggestCheck->setChecked(false);
    aiFeaturesLayout->addWidget(autoSuggestCheck);

    layout->addWidget(aiFeaturesGroup);

    QGroupBox *aiPromptGroup = new QGroupBox("提示词模板", page);
    QVBoxLayout *promptLayout = new QVBoxLayout(aiPromptGroup);
    promptLayout->setSpacing(10);

    QLabel *promptHint = new QLabel("自定义AI提示词模板", page);
    promptHint->setStyleSheet("color: #666; font-size: 12px;");
    promptLayout->addWidget(promptHint);

    QTextEdit *promptEdit = new QTextEdit(page);
    promptEdit->setPlaceholderText("在此输入自定义提示词模板...");
    promptEdit->setMaximumHeight(150);
    promptLayout->addWidget(promptEdit);

    QPushButton *savePromptBtn = new QPushButton("保存模板", page);
    savePromptBtn->setStyleSheet(
        "QPushButton { background-color: #4caf50; color: white; padding: 8px 20px; border-radius: 5px; } "
        "QPushButton:hover { background-color: #43a047; }"
    );
    promptLayout->addWidget(savePromptBtn, 0, Qt::AlignRight);

    layout->addWidget(aiPromptGroup);

    layout->addStretch();
    return page;
}

QWidget *SettingsWidget::createStartupPage()
{
    QWidget *page = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setContentsMargins(30, 30, 30, 30);
    layout->setSpacing(20);

    QLabel *title = new QLabel("开机启动", page);
    title->setStyleSheet("font-size: 24px; font-weight: bold; color: #333;");
    layout->addWidget(title);

    QFrame *line = new QFrame(page);
    line->setFrameShape(QFrame::HLine);
    line->setStyleSheet("color: #e0e0e0;");
    layout->addWidget(line);

    QGroupBox *startupGroup = new QGroupBox("开机启动设置", page);
    QVBoxLayout *startupLayout = new QVBoxLayout(startupGroup);
    startupLayout->setSpacing(15);

    autoStartCheck = new QCheckBox("开机自动启动小马办公", page);
    autoStartCheck->setChecked(db->getAutoStart());
    connect(autoStartCheck, &QCheckBox::stateChanged, this, &SettingsWidget::onAutoStartToggled);
    startupLayout->addWidget(autoStartCheck);

    statusLabel = new QLabel();
    if (db->getAutoStart()) {
        statusLabel->setText("✓ 当前状态: 已启用");
        statusLabel->setStyleSheet("color: #4caf50; font-size: 13px; padding: 5px;");
    } else {
        statusLabel->setText("✗ 当前状态: 已禁用");
        statusLabel->setStyleSheet("color: #f44336; font-size: 13px; padding: 5px;");
    }
    startupLayout->addWidget(statusLabel);

    QLabel *startupHint = new QLabel("启用后，每次电脑开机时小马办公将自动启动并运行在系统托盘区域。", page);
    startupHint->setStyleSheet("color: #666; font-size: 12px; padding: 10px; background-color: #f5f5f5; border-radius: 5px;");
    startupHint->setWordWrap(true);
    startupLayout->addWidget(startupHint);

    layout->addWidget(startupGroup);

    QGroupBox *startupBehaviorGroup = new QGroupBox("启动行为", page);
    QVBoxLayout *behaviorLayout = new QVBoxLayout(startupBehaviorGroup);
    behaviorLayout->setSpacing(10);

    QRadioButton *startMinimized = new QRadioButton("启动时最小化到托盘", startupBehaviorGroup);
    startMinimized->setChecked(true);
    behaviorLayout->addWidget(startMinimized);

    QRadioButton *startNormal = new QRadioButton("启动时正常显示窗口", startupBehaviorGroup);
    behaviorLayout->addWidget(startNormal);

    layout->addWidget(startupBehaviorGroup);

    layout->addStretch();
    return page;
}

QWidget *SettingsWidget::createUpdatePage()
{
    QWidget *page = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setContentsMargins(30, 30, 30, 30);
    layout->setSpacing(20);

    QLabel *title = new QLabel("检查更新", page);
    title->setStyleSheet("font-size: 24px; font-weight: bold; color: #333;");
    layout->addWidget(title);

    QFrame *line = new QFrame(page);
    line->setFrameShape(QFrame::HLine);
    line->setStyleSheet("color: #e0e0e0;");
    layout->addWidget(line);

    QGroupBox *autoUpdateGroup = new QGroupBox("自动更新", page);
    QVBoxLayout *autoUpdateLayout = new QVBoxLayout(autoUpdateGroup);
    autoUpdateLayout->setSpacing(10);

    autoCheckUpdateCheck = new QCheckBox("自动检查更新", page);
    autoCheckUpdateCheck->setChecked(db->getAutoCheckUpdate());
    connect(autoCheckUpdateCheck, &QCheckBox::stateChanged, this, &SettingsWidget::onAutoCheckUpdateToggled);
    autoUpdateLayout->addWidget(autoCheckUpdateCheck);

    QLabel *updateInfoLabel = new QLabel("启用后，软件启动时和后台每24小时会自动检查更新。", page);
    updateInfoLabel->setStyleSheet("color: #666; font-size: 12px; padding: 5px;");
    updateInfoLabel->setWordWrap(true);
    autoUpdateLayout->addWidget(updateInfoLabel);

    layout->addWidget(autoUpdateGroup);

    QGroupBox *checkUpdateGroup = new QGroupBox("手动检查更新", page);
    QVBoxLayout *checkLayout = new QVBoxLayout(checkUpdateGroup);
    checkLayout->setSpacing(15);

    QLabel *versionLabel = new QLabel("当前版本: v0.0.6", page);
    versionLabel->setStyleSheet("font-size: 14px; color: #333; padding: 5px;");
    checkLayout->addWidget(versionLabel);

    checkUpdateButton = new QPushButton("🔄 检查更新", page);
    checkUpdateButton->setStyleSheet(
        "QPushButton { background-color: #2196f3; color: white; padding: 12px 30px; border-radius: 5px; font-size: 14px; font-weight: bold; } "
        "QPushButton:hover { background-color: #1976d2; } "
        "QPushButton:pressed { background-color: #1565c0; }"
    );
    connect(checkUpdateButton, &QPushButton::clicked, this, &SettingsWidget::onCheckUpdateClicked);
    checkLayout->addWidget(checkUpdateButton);

    layout->addWidget(checkUpdateGroup);

    QGroupBox *updateChannelGroup = new QGroupBox("更新通道", page);
    QVBoxLayout *channelLayout = new QVBoxLayout(updateChannelGroup);
    channelLayout->setSpacing(10);

    QRadioButton *stableChannel = new QRadioButton("稳定版 (推荐)", updateChannelGroup);
    stableChannel->setChecked(true);
    channelLayout->addWidget(stableChannel);

    QRadioButton *betaChannel = new QRadioButton("测试版", updateChannelGroup);
    betaChannel->setChecked(false);
    channelLayout->addWidget(betaChannel);

    QLabel *channelHint = new QLabel("测试版可能包含最新功能但可能不稳定", updateChannelGroup);
    channelHint->setStyleSheet("color: #999; font-size: 11px; padding: 5px;");
    channelLayout->addWidget(channelHint);

    layout->addWidget(updateChannelGroup);

    layout->addStretch();
    return page;
}

QWidget *SettingsWidget::createAboutPage()
{
    QWidget *page = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setContentsMargins(30, 30, 30, 30);
    layout->setSpacing(20);

    QLabel *title = new QLabel("关于", page);
    title->setStyleSheet("font-size: 24px; font-weight: bold; color: #333;");
    layout->addWidget(title);

    QFrame *line = new QFrame(page);
    line->setFrameShape(QFrame::HLine);
    line->setStyleSheet("color: #e0e0e0;");
    layout->addWidget(line);

    QGroupBox *appInfoGroup = new QGroupBox("应用信息", page);
    QVBoxLayout *infoLayout = new QVBoxLayout(appInfoGroup);
    infoLayout->setSpacing(15);

    QLabel *appNameLabel = new QLabel("小马办公", page);
    appNameLabel->setStyleSheet("font-size: 28px; font-weight: bold; color: #6200ea;");
    infoLayout->addWidget(appNameLabel);

    QLabel *versionLabel = new QLabel("版本: v0.0.6", page);
    versionLabel->setStyleSheet("font-size: 14px; color: #666;");
    infoLayout->addWidget(versionLabel);

    QLabel *descLabel = new QLabel("一个功能完善的桌面办公助手应用", page);
    descLabel->setStyleSheet("font-size: 13px; color: #888; padding: 10px 0;");
    infoLayout->addWidget(descLabel);

    layout->addWidget(appInfoGroup);

    QGroupBox *featuresGroup = new QGroupBox("主要功能", page);
    QVBoxLayout *featuresLayout = new QVBoxLayout(featuresGroup);
    featuresLayout->setSpacing(8);

    QStringList features = {
        "📱 应用管理 - 管理和快速启动常用应用",
        "📁 集合管理 - 自定义应用分组和批量启动",
        "🐟 摸鱼模式 - 老板键和状态切换",
        "⏰ 定时关机 - 定时关机/重启/休眠",
        "🤖 AI智能 - 任务分析和报告生成",
        "📊 工作日志 - 记录和分析工作情况"
    };

    for (const QString &feature : features) {
        QLabel *featureLabel = new QLabel(feature, page);
        featureLabel->setStyleSheet("font-size: 13px; color: #555; padding: 5px;");
        featuresLayout->addWidget(featureLabel);
    }

    layout->addWidget(featuresGroup);

    QGroupBox *techGroup = new QGroupBox("技术信息", page);
    QVBoxLayout *techLayout = new QVBoxLayout(techGroup);
    techLayout->setSpacing(8);

    QLabel *techLabel1 = new QLabel("• Qt 5.15.2", techGroup);
    techLabel1->setStyleSheet("font-size: 12px; color: #666;");
    techLayout->addWidget(techLabel1);

    QLabel *techLabel2 = new QLabel("• MinGW 8.1.0", techGroup);
    techLabel2->setStyleSheet("font-size: 12px; color: #666;");
    techLayout->addWidget(techLabel2);

    QLabel *techLabel3 = new QLabel("• Windows COM 自动化", techGroup);
    techLabel3->setStyleSheet("font-size: 12px; color: #666;");
    techLayout->addWidget(techLabel3);

    layout->addWidget(techGroup);

    layout->addStretch();

    QLabel *copyrightLabel = new QLabel("© 2026 小马办公. All rights reserved.", page);
    copyrightLabel->setStyleSheet("color: #bbb; font-size: 12px; padding: 20px; text-align: center;");
    copyrightLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(copyrightLabel);

    return page;
}

void SettingsWidget::onAutoStartToggled(int state)
{
    bool enabled = (state == Qt::Checked);
    
    if (db->setAutoStart(enabled)) {
        if (enabled) {
            statusLabel->setText("✓ 当前状态: 已启用");
            statusLabel->setStyleSheet("color: #4caf50; font-size: 13px; padding: 5px;");
            QMessageBox::information(this, "成功", "开机自动启动已启用！");
        } else {
            statusLabel->setText("✗ 当前状态: 已禁用");
            statusLabel->setStyleSheet("color: #f44336; font-size: 13px; padding: 5px;");
            QMessageBox::information(this, "成功", "开机自动启动已禁用！");
        }
    } else {
        QMessageBox::warning(this, "错误", "设置开机自动启动失败！");
        autoStartCheck->setChecked(!enabled);
    }
}

void SettingsWidget::onMinimizeToTrayToggled(int state)
{
    bool enabled = (state == Qt::Checked);
    
    if (db->setMinimizeToTray(enabled)) {
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

void SettingsWidget::onAboutClicked()
{
    // 关于页面已集成到左侧导航中，无需额外处理
}

void SettingsWidget::onCheckUpdateClicked()
{
    if (!updateManager) {
        QMessageBox::warning(this, "错误", "更新管理器未初始化");
        return;
    }
    
    checkUpdateButton->setEnabled(false);
    checkUpdateButton->setText("检查中...");
    updateManager->checkForUpdates();
}

void SettingsWidget::onUpdateAvailable(const UpdateInfo &info)
{
    Q_UNUSED(info);
    checkUpdateButton->setEnabled(true);
    checkUpdateButton->setText("🔄 检查更新");
}

void SettingsWidget::onNoUpdateAvailable()
{
    checkUpdateButton->setEnabled(true);
    checkUpdateButton->setText("🔄 检查更新");
    QMessageBox::information(this, "检查更新", "当前已是最新版本！");
}

void SettingsWidget::onUpdateCheckFailed(const QString &error)
{
    Q_UNUSED(error);
    checkUpdateButton->setEnabled(true);
    checkUpdateButton->setText("🔄 检查更新");
    QMessageBox::warning(this, "检查更新", "检查更新失败，请检查网络连接！");
}

void SettingsWidget::onOpenAISettings()
{
    AISettingsDialog dialog(this);
    dialog.exec();
}

bool SettingsWidget::isShortcutConflict(const QString &shortcut)
{
    QStringList conflictShortcuts = {
        "Ctrl+A", "Ctrl+Z", "Ctrl+Y", "F1", "F12", "Win+Tab"
    };
    
    QString normalizedShortcut = shortcut.toUpper().replace(" ", "");
    for (const QString &conflict : conflictShortcuts) {
        if (normalizedShortcut == conflict.toUpper().replace(" ", "")) {
            return true;
        }
    }
    
    if (shortcut.contains("Ctrl+Alt+", Qt::CaseInsensitive) || 
        shortcut.contains("Ctrl+Shift+Alt+", Qt::CaseInsensitive) ||
        shortcut.contains("Win+Ctrl", Qt::CaseInsensitive) ||
        shortcut.contains("Win+Alt", Qt::CaseInsensitive)) {
        return true;
    }
    
    if (shortcut.contains("Ctrl+Shift", Qt::CaseInsensitive) && 
        (shortcut.endsWith("T", Qt::CaseInsensitive) || 
         shortcut.endsWith("N", Qt::CaseInsensitive) ||
         shortcut.endsWith("W", Qt::CaseInsensitive))) {
        return true;
    }
    
    return false;
}
