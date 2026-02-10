#include "mainwindow.h"
#include "appmanagerwidget.h"
#include "fishmodewidget.h"
#include "shutdownwidget.h"
#include "settingswidget.h"
#include "collectionmanagerwidget.h"
#include "recommendedappswidget.h"
#include "updatedialog.h"
#include <QApplication>
#include <QStyle>
#include <QStandardPaths>
#include <QIcon>
#include <QMessageBox>
#include <QPropertyAnimation>
#include <QProgressDialog>
#include <QDesktopServices>
#include <QUrl>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    db = new Database(this);
    if (!db->init()) {
        qWarning("Failed to initialize database!");
    }
    
    initPresetApps();
    setupUI();
    setupTrayIcon();
    
    updateManager = new UpdateManager(this);
    updateManager->setIgnoredVersion(db->getIgnoredVersion());
    
    connect(updateManager, &UpdateManager::updateAvailable, this, &MainWindow::onUpdateAvailable);
    connect(updateManager, &UpdateManager::noUpdateAvailable, this, &MainWindow::onNoUpdateAvailable);
    connect(updateManager, &UpdateManager::updateCheckFailed, this, &MainWindow::onUpdateCheckFailed);
    connect(updateManager, &UpdateManager::downloadProgress, this, &MainWindow::onDownloadProgress);
    connect(updateManager, &UpdateManager::downloadFinished, this, &MainWindow::onDownloadFinished);
    connect(updateManager, &UpdateManager::downloadFailed, this, &MainWindow::onDownloadFailed);
    
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
    fishModeWidget = new FishModeWidget(this);
    shutdownWidget = new ShutdownWidget(this);
    settingsWidget = new SettingsWidget(db, this);
    recommendedAppsWidget = new RecommendedAppsWidget(this);
    
    tabWidget->addTab(appManagerWidget, QApplication::style()->standardIcon(QStyle::SP_ComputerIcon), "应用管理");
    tabWidget->addTab(collectionManagerWidget, QApplication::style()->standardIcon(QStyle::SP_DirHomeIcon), "集合管理");
    tabWidget->addTab(recommendedAppsWidget, QApplication::style()->standardIcon(QStyle::SP_ArrowForward), "推荐应用");
    tabWidget->addTab(fishModeWidget, QApplication::style()->standardIcon(QStyle::SP_FileDialogDetailedView), "摸鱼模式");
    tabWidget->addTab(shutdownWidget, QApplication::style()->standardIcon(QStyle::SP_BrowserStop), "定时关机");
    tabWidget->addTab(settingsWidget, QApplication::style()->standardIcon(QStyle::SP_FileDialogInfoView), "设置");
    
    tabWidget->setIconSize(QSize(24, 24));
    
    connect(tabWidget, &QTabWidget::currentChanged, this, &MainWindow::onTabChanged);
    
    mainLayout->addWidget(tabWidget);
}

void MainWindow::initPresetApps()
{
    QList<AppInfo> existingApps = db->getAllApps();
    if (!existingApps.isEmpty()) {
        return;
    }
    
    QString winDir = qgetenv("WINDIR");
    QString systemDir = winDir + "/System32";
    
    QList<AppInfo> presetApps;
    
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
    
    AppInfo mspaint;
    mspaint.name = "画图";
    mspaint.path = systemDir + "/mspaint.exe";
    mspaint.category = "系统工具";
    mspaint.sortOrder = 3;
    presetApps.append(mspaint);
    
    AppInfo taskmgr;
    taskmgr.name = "任务管理器";
    taskmgr.path = systemDir + "/taskmgr.exe";
    taskmgr.category = "系统工具";
    taskmgr.sortOrder = 4;
    presetApps.append(taskmgr);
    
    for (const AppInfo &app : presetApps) {
        db->addApp(app);
    }
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

void MainWindow::onUpdateAvailable(const UpdateInfo &info)
{
    updateDialog = new UpdateDialog(info, this);
    connect(updateDialog, &UpdateDialog::updateNow, this, &MainWindow::onUpdateNow);
    connect(updateDialog, &UpdateDialog::remindLater, this, &MainWindow::onRemindLater);
    connect(updateDialog, &UpdateDialog::skipThisVersion, this, &MainWindow::onSkipThisVersion);
    updateDialog->show();
}

void MainWindow::onNoUpdateAvailable()
{
}

void MainWindow::onUpdateCheckFailed(const QString &error)
{
    qWarning() << "Update check failed:" << error;
}

void MainWindow::onUpdateNow()
{
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
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("下载完成");
    msgBox.setText("更新包已下载完成！");
    msgBox.setInformativeText(QString("文件位置: %1\n\n是否立即打开文件位置？").arg(filePath));
    msgBox.setIcon(QMessageBox::Information);
    
    QPushButton *openButton = msgBox.addButton("打开文件位置", QMessageBox::ActionRole);
    QPushButton *closeButton = msgBox.addButton("关闭", QMessageBox::RejectRole);
    msgBox.setDefaultButton(openButton);
    
    msgBox.exec();
    
    if (msgBox.clickedButton() == openButton) {
        QFileInfo info(filePath);
        QDesktopServices::openUrl(QUrl::fromLocalFile(info.absolutePath()));
    }
}

void MainWindow::onDownloadFailed(const QString &error)
{
    QMessageBox::warning(this, "下载失败", QString("更新下载失败:\n%1").arg(error));
}

void MainWindow::changeEvent(QEvent *event)
{
    QMainWindow::changeEvent(event);
    
    if (event->type() == QEvent::WindowStateChange) {
        if (isMinimized()) {
            bool minimizeToTray = db->getMinimizeToTray();
            if (minimizeToTray) {
                hide();
            }
        }
    }
}
