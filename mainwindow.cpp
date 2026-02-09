#include "mainwindow.h"
#include "appmanagerwidget.h"
#include "fishmodewidget.h"
#include "shutdownwidget.h"
#include "settingswidget.h"
#include "collectionmanagerwidget.h"
#include <QApplication>
#include <QStyle>
#include <QStandardPaths>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    db = new Database(this);
    if (!db->init()) {
        qWarning("Failed to initialize database!");
    }
    
    initPresetApps();
    setupUI();
    
    setWindowTitle("办公助手 - Office Assistant");
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
    
    tabWidget->addTab(appManagerWidget, QApplication::style()->standardIcon(QStyle::SP_ComputerIcon), "应用管理");
    tabWidget->addTab(collectionManagerWidget, QApplication::style()->standardIcon(QStyle::SP_DirHomeIcon), "集合管理");
    tabWidget->addTab(fishModeWidget, QApplication::style()->standardIcon(QStyle::SP_FileDialogDetailedView), "摸鱼模式");
    tabWidget->addTab(shutdownWidget, QApplication::style()->standardIcon(QStyle::SP_BrowserStop), "定时关机");
    tabWidget->addTab(settingsWidget, QApplication::style()->standardIcon(QStyle::SP_FileDialogInfoView), "设置");
    
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
