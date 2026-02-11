#include "mainwindow.h"
#include "appmanagerwidget.h"
#include "fishmodewidget.h"
#include "shutdownwidget.h"
#include "settingswidget.h"
#include "collectionmanagerwidget.h"
#include "recommendedappswidget.h"
#include "updatedialog.h"
#include "updateprogressdialog.h"
#include "remotedesktopwidget.h"
#include <QApplication>
#include <QStyle>
#include <QStandardPaths>
#include <QIcon>
#include <QMessageBox>
#include <QPropertyAnimation>
#include <QProgressDialog>
#include <QDesktopServices>
#include <QUrl>
#include <QSettings>
#include <QTimer>
#include <QFile>
#include <QRegExp>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    db = new Database(this);
    if (!db->init()) {
        qWarning("Failed to initialize database!");
    }
    
    initPresetApps();
    
    updateManager = new UpdateManager(this);
    updateManager->setIgnoredVersion(db->getIgnoredVersion());
    
    connect(updateManager, &UpdateManager::checkForUpdatesStarted, this, [this]() {
        QString version = qApp->applicationVersion();
        setStatusText(QString("小马办公 v%1 - 正在检查更新...").arg(version));
    });
    connect(updateManager, &UpdateManager::updateAvailable, this, &MainWindow::onUpdateAvailable);
    connect(updateManager, &UpdateManager::noUpdateAvailable, this, &MainWindow::onNoUpdateAvailable);
    connect(updateManager, &UpdateManager::updateCheckFailed, this, &MainWindow::onUpdateCheckFailed);
    connect(updateManager, &UpdateManager::downloadProgress, this, &MainWindow::onDownloadProgress);
    connect(updateManager, &UpdateManager::downloadFinished, this, &MainWindow::onDownloadFinished);
    connect(updateManager, &UpdateManager::downloadFailed, this, &MainWindow::onDownloadFailed);
    connect(updateManager, &UpdateManager::extractProgress, this, &MainWindow::onExtractProgress);
    connect(updateManager, &UpdateManager::extractFinished, this, &MainWindow::onExtractFinished);
    connect(updateManager, &UpdateManager::extractFailed, this, &MainWindow::onExtractFailed);
    connect(updateManager, &UpdateManager::installProgress, this, &MainWindow::onInstallProgress);
    connect(updateManager, &UpdateManager::installFinished, this, &MainWindow::onInstallFinished);
    connect(updateManager, &UpdateManager::installFailed, this, &MainWindow::onInstallFailed);
    connect(updateManager, &UpdateManager::logMessage, this, &MainWindow::onLogMessage);
    
    setupUI();
    setupTrayIcon();
    
    if (db->getAutoCheckUpdate()) {
        updateManager->startPeriodicChecks();
    }
    
    QList<AppCollection> collections = db->getAllCollections();
    if (!collections.isEmpty()) {
        tabWidget->setCurrentWidget(collectionManagerWidget);
    }
    
    QString version = qApp->applicationVersion();
    setWindowTitle(QString("小马办公 - PonyWork v%1").arg(version));
    setMinimumSize(1000, 700);
    resize(1100, 750);
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUI()
{
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    
    tabWidget = new QTabWidget(this);
    
    appManagerWidget = new AppManagerWidget(db, this);
    collectionManagerWidget = new CollectionManagerWidget(db, this);
    remoteDesktopWidget = new RemoteDesktopWidget(db, this);
    fishModeWidget = new FishModeWidget(this);
    shutdownWidget = new ShutdownWidget(this);
    settingsWidget = new SettingsWidget(db, this);
    settingsWidget->setUpdateManager(updateManager);
    recommendedAppsWidget = new RecommendedAppsWidget(this);
    
    connect(appManagerWidget, &AppManagerWidget::resetAppsRequested, this, &MainWindow::resetApps);
    connect(remoteDesktopWidget, &RemoteDesktopWidget::appListNeedsRefresh, appManagerWidget, &AppManagerWidget::refreshAppList);
    connect(remoteDesktopWidget, &RemoteDesktopWidget::collectionNeedsRefresh, collectionManagerWidget, &CollectionManagerWidget::refreshCollectionList);
    connect(remoteDesktopWidget, &RemoteDesktopWidget::collectionNeedsRefresh, collectionManagerWidget, &CollectionManagerWidget::refreshCollectionApps);
    connect(collectionManagerWidget, &CollectionManagerWidget::statusMessageRequested, this, &MainWindow::setStatusText);
    connect(remoteDesktopWidget, &RemoteDesktopWidget::statusMessageRequested, this, &MainWindow::setStatusText);
    
    tabWidget->addTab(appManagerWidget, QApplication::style()->standardIcon(QStyle::SP_DesktopIcon), "应用管理");
    tabWidget->addTab(collectionManagerWidget, QApplication::style()->standardIcon(QStyle::SP_DirHomeIcon), "集合管理");
    tabWidget->addTab(remoteDesktopWidget, QApplication::style()->standardIcon(QStyle::SP_ComputerIcon), "远程桌面");
    tabWidget->addTab(recommendedAppsWidget, QApplication::style()->standardIcon(QStyle::SP_ArrowForward), "推荐应用");
    tabWidget->addTab(fishModeWidget, QApplication::style()->standardIcon(QStyle::SP_FileDialogDetailedView), "摸鱼模式");
    tabWidget->addTab(shutdownWidget, QApplication::style()->standardIcon(QStyle::SP_BrowserStop), "定时关机");
    tabWidget->addTab(settingsWidget, QApplication::style()->standardIcon(QStyle::SP_FileDialogInfoView), "设置");
 
    tabWidget->setIconSize(QSize(24, 24));
    
    connect(tabWidget, &QTabWidget::currentChanged, this, &MainWindow::onTabChanged);
    
    mainLayout->addWidget(tabWidget);
    
    statusLabel = new QLabel(this);
    statusBar()->addWidget(statusLabel);
    QString version = qApp->applicationVersion();
    statusLabel->setText(QString("小马办公 v%1 - 就绪").arg(version));
}

void MainWindow::initPresetApps()
{
    initPresetApps(false);
}

void MainWindow::initPresetApps(bool forceReset)
{
    QList<AppInfo> existingApps = db->getAllApps();
    if (!existingApps.isEmpty() && !forceReset) {
        return;
    }
    
    if (forceReset) {
        for (const AppInfo &app : existingApps) {
            db->deleteApp(app.id);
        }
    }
    
    QString winDir = qgetenv("WINDIR");
    QString systemDir = winDir + "/System32";
    
    QList<AppInfo> presetApps;
    QStringList detectionMessages;
    
    AppInfo notepad;
    notepad.name = "记事本";
    notepad.path = systemDir + "/notepad.exe";
    notepad.category = "系统工具";
    notepad.isFavorite = true;
    notepad.sortOrder = 0;
    presetApps.append(notepad);
    
    AppInfo calc;
    calc.name = "计算器";
    calc.path = systemDir + "/calc.exe";
    calc.category = "系统工具";
    calc.isFavorite = true;
    calc.sortOrder = 1;
    presetApps.append(calc);
    
    AppInfo cmd;
    cmd.name = "命令提示符";
    cmd.path = systemDir + "/cmd.exe";
    cmd.category = "系统工具";
    cmd.sortOrder = 2;
    presetApps.append(cmd);
    
    AppInfo powershell;
    powershell.name = "PowerShell";
    powershell.path = systemDir + "/WindowsPowerShell/v1.0/powershell.exe";
    powershell.category = "系统工具";
    powershell.sortOrder = 3;
    presetApps.append(powershell);
    
    AppInfo mspaint;
    mspaint.name = "画图";
    mspaint.path = systemDir + "/mspaint.exe";
    mspaint.category = "系统工具";
    mspaint.sortOrder = 4;
    presetApps.append(mspaint);
    
    AppInfo taskmgr;
    taskmgr.name = "任务管理器";
    taskmgr.path = systemDir + "/taskmgr.exe";
    taskmgr.category = "系统工具";
    taskmgr.sortOrder = 5;
    presetApps.append(taskmgr);
    
    QString officeVersion = getOfficeVersion();
    if (!officeVersion.isEmpty()) {
        detectionMessages << QString("检测到 Microsoft Office 版本: %1").arg(officeVersion);
    }
    
    QString wordPath = findOfficeAppPath("WINWORD.EXE");
    if (!wordPath.isEmpty() && QFile::exists(wordPath)) {
        AppInfo word;
        word.name = "Microsoft Word";
        word.path = wordPath;
        word.category = "Office办公";
        word.sortOrder = 6;
        presetApps.append(word);
        detectionMessages << "✓ 已检测到 Microsoft Word";
    } else {
        detectionMessages << "✗ 未检测到 Microsoft Word";
    }
    
    QString excelPath = findOfficeAppPath("EXCEL.EXE");
    if (!excelPath.isEmpty() && QFile::exists(excelPath)) {
        AppInfo excel;
        excel.name = "Microsoft Excel";
        excel.path = excelPath;
        excel.category = "Office办公";
        excel.sortOrder = 7;
        presetApps.append(excel);
        detectionMessages << "✓ 已检测到 Microsoft Excel";
    } else {
        detectionMessages << "✗ 未检测到 Microsoft Excel";
    }
    
    QString pptPath = findOfficeAppPath("POWERPNT.EXE");
    if (!pptPath.isEmpty() && QFile::exists(pptPath)) {
        AppInfo powerpoint;
        powerpoint.name = "Microsoft PowerPoint";
        powerpoint.path = pptPath;
        powerpoint.category = "Office办公";
        powerpoint.sortOrder = 8;
        presetApps.append(powerpoint);
        detectionMessages << "✓ 已检测到 Microsoft PowerPoint";
    } else {
        detectionMessages << "✗ 未检测到 Microsoft PowerPoint";
    }
    
    for (const AppInfo &app : presetApps) {
        db->addApp(app);
    }
    
    QTimer::singleShot(1000, this, [this, detectionMessages]() {
        QString message = "🔍 Office 应用检测完成\n\n" + detectionMessages.join("\n");
        QMessageBox::information(this, "Office 检测完成", message);
    });
}

void MainWindow::onTabChanged(int index)
{
    if (tabWidget->widget(index) == collectionManagerWidget) {
        collectionManagerWidget->selectFirstCollection();
    }
}

void MainWindow::setupTrayIcon()
{
    trayIcon = new QSystemTrayIcon(this);
    
    QIcon appIcon(":/img/icon.ico");
    if (appIcon.isNull()) {
        appIcon = qApp->style()->standardIcon(QStyle::SP_ComputerIcon);
    }
    trayIcon->setIcon(appIcon);
    
    QString version = qApp->applicationVersion();
    trayIcon->setToolTip(QString("小马办公 - PonyWork v%1\n正在运行中").arg(version));
    
    trayMenu = new QMenu(this);
    
    showWindowAction = new QAction("打开窗口", this);
    connect(showWindowAction, &QAction::triggered, this, &MainWindow::onShowWindow);
    trayMenu->addAction(showWindowAction);
    
    trayMenu->addSeparator();
    
    exitAppAction = new QAction("退出程序", this);
    connect(exitAppAction, &QAction::triggered, this, &MainWindow::onExitApp);
    trayMenu->addAction(exitAppAction);
    
    trayIcon->setContextMenu(trayMenu);
    
    connect(trayIcon, &QSystemTrayIcon::activated, this, &MainWindow::onTrayIconActivated);
    
    trayIcon->show();
}

void MainWindow::onTrayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::DoubleClick) {
        onShowWindow();
    }
}

void MainWindow::onShowWindow()
{
    if (isHidden()) {
        showNormal();
        activateWindow();
        raise();
    } else {
        showNormal();
        activateWindow();
        raise();
    }
}

void MainWindow::onExitApp()
{
    QApplication::quit();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    bool minimizeToTray = db->getMinimizeToTray();
    bool showPrompt = db->getShowClosePrompt();
    
    if (!showPrompt) {
        if (minimizeToTray) {
            hide();
            QString version = qApp->applicationVersion();
            trayIcon->showMessage(
                "小马办公",
                "已最小化到系统托盘\n双击托盘图标可重新打开窗口",
                QSystemTrayIcon::Information,
                4000
            );
            event->ignore();
        } else {
            event->accept();
        }
        return;
    }
    
    QMessageBox msgBox(this);
    
    if (minimizeToTray) {
        msgBox.setWindowTitle("关闭提示");
        msgBox.setText("关闭窗口将最小化到系统托盘");
        msgBox.setInformativeText("程序将在后台继续运行，您可以通过系统托盘图标重新打开窗口。\n\n勾选「不再显示此提示」可以跳过此确认。");
        msgBox.setIcon(QMessageBox::Information);
        
        QCheckBox *dontShowAgain = new QCheckBox("不再显示此提示", &msgBox);
        msgBox.setCheckBox(dontShowAgain);
        
        QPushButton *okButton = msgBox.addButton("确定", QMessageBox::AcceptRole);
        msgBox.addButton("取消", QMessageBox::RejectRole);
        msgBox.setDefaultButton(okButton);
        
        msgBox.exec();
        
        if (msgBox.clickedButton() == okButton) {
            if (dontShowAgain->isChecked()) {
                db->setShowClosePrompt(false);
            }
            hide();
            QString version = qApp->applicationVersion();
            trayIcon->showMessage(
                "小马办公",
                "已最小化到系统托盘\n双击托盘图标可重新打开窗口",
                QSystemTrayIcon::Information,
                4000
            );
            event->ignore();
        } else {
            event->ignore();
        }
    } else {
        msgBox.setWindowTitle("退出确认");
        msgBox.setText("确定要完全退出应用吗？");
        msgBox.setInformativeText("程序将完全退出，所有功能将停止运行。\n\n勾选「不再显示此提示」可以跳过此确认。");
        msgBox.setIcon(QMessageBox::Warning);
        
        QCheckBox *dontShowAgain = new QCheckBox("不再显示此提示", &msgBox);
        msgBox.setCheckBox(dontShowAgain);
        
        QPushButton *okButton = msgBox.addButton("退出", QMessageBox::AcceptRole);
        msgBox.addButton("取消", QMessageBox::RejectRole);
        msgBox.setDefaultButton(okButton);
        
        msgBox.exec();
        
        if (msgBox.clickedButton() == okButton) {
            if (dontShowAgain->isChecked()) {
                db->setShowClosePrompt(false);
            }
            event->accept();
        } else {
            event->ignore();
        }
    }
}

void MainWindow::setStatusText(const QString &text)
{
    if (statusLabel) {
        statusLabel->setText(text);
    }
}

void MainWindow::onUpdateAvailable(const UpdateInfo &info)
{
    setStatusText(QString("发现新版本 v%1").arg(info.version));
    updateDialog = new UpdateDialog(info, this);
    connect(updateDialog, &UpdateDialog::updateNow, this, &MainWindow::onUpdateNow);
    connect(updateDialog, &UpdateDialog::remindLater, this, &MainWindow::onRemindLater);
    connect(updateDialog, &UpdateDialog::skipThisVersion, this, &MainWindow::onSkipThisVersion);
    updateDialog->show();
}

void MainWindow::onNoUpdateAvailable()
{
    QString version = qApp->applicationVersion();
    setStatusText(QString("小马办公 v%1 - 已是最新版本").arg(version));
}

void MainWindow::onUpdateCheckFailed(const QString &error)
{
    qWarning() << "Update check failed:" << error;
    QString version = qApp->applicationVersion();
    setStatusText(QString("小马办公 v%1 - 检查更新失败").arg(version));
}

void MainWindow::onUpdateNow()
{
    updateProgressDialog = new UpdateProgressDialog(this);
    updateProgressDialog->setUpdateManager(updateManager);
    updateProgressDialog->show();
    
    updateManager->downloadUpdate();
}

void MainWindow::onRemindLater()
{
}

void MainWindow::onSkipThisVersion()
{
    updateManager->skipThisVersion();
    db->setIgnoredVersion(updateManager->ignoredVersion());
}

void MainWindow::onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
}

void MainWindow::onDownloadFinished(const QString &filePath)
{
    updateManager->installUpdate(filePath);
}

void MainWindow::onExtractProgress(int percent)
{
}

void MainWindow::onExtractFinished(const QString &extractPath)
{
    Q_UNUSED(extractPath);
}

void MainWindow::onExtractFailed(const QString &error)
{
    Q_UNUSED(error);
}

void MainWindow::onInstallProgress(int percent)
{
    Q_UNUSED(percent);
}

void MainWindow::onInstallFinished()
{
}

void MainWindow::onInstallFailed(const QString &error)
{
    Q_UNUSED(error);
}

void MainWindow::onLogMessage(const QString &message)
{
    Q_UNUSED(message);
}

void MainWindow::onDownloadFailed(const QString &error)
{
    Q_UNUSED(error);
}

void MainWindow::changeEvent(QEvent *event)
{
    QMainWindow::changeEvent(event);
    
    if (event->type() == QEvent::WindowStateChange) {
        if (isMinimized()) {
            bool minimizeToTray = db->getMinimizeToTray();
            if (minimizeToTray) {
                hide();
                QString version = qApp->applicationVersion();
                trayIcon->showMessage(
                    "小马办公",
                    "已最小化到系统托盘\n双击托盘图标可重新打开窗口",
                    QSystemTrayIcon::Information,
                    4000
                );
            }
        }
    }
}

QString MainWindow::findOfficeAppPath(const QString &appName)
{
    QSettings settings("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\" + appName, QSettings::NativeFormat);
    QString path = settings.value(".").toString();
    
    if (path.isEmpty() || !QFile::exists(path)) {
        QSettings settings32("HKEY_LOCAL_MACHINE\\SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\App Paths\\" + appName, QSettings::NativeFormat);
        path = settings32.value(".").toString();
    }
    
    return path;
}

QString MainWindow::getOfficeVersion()
{
    QSettings settings("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Office", QSettings::NativeFormat);
    QStringList versions = settings.childGroups();
    
    QString latestVersion = "";
    for (const QString &version : versions) {
        if (version.contains(QRegExp("^\\d+\\.\\d+$"))) {
            if (latestVersion.isEmpty() || version > latestVersion) {
                latestVersion = version;
            }
        }
    }
    
    return latestVersion;
}

void MainWindow::resetApps()
{
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("确认初始化");
    msgBox.setText("确定要初始化应用列表吗？");
    msgBox.setInformativeText("原应用列表可能会被覆盖，此操作将重新检测并添加所有预设应用。\n\n此操作不可撤销！");
    msgBox.setIcon(QMessageBox::Warning);
    
    QPushButton *confirmBtn = msgBox.addButton("确认", QMessageBox::AcceptRole);
    QPushButton *cancelBtn = msgBox.addButton("取消", QMessageBox::RejectRole);
    msgBox.setDefaultButton(cancelBtn);
    
    msgBox.exec();
    
    if (msgBox.clickedButton() == confirmBtn) {
        initPresetApps(true);
        if (appManagerWidget) {
            appManagerWidget->refreshAppList();
        }
        QMessageBox::information(this, "初始化完成", "应用列表已成功初始化！");
    }
}
