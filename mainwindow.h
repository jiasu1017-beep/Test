#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QCloseEvent>
#include <QEvent>
#include <QLabel>
#include <QStatusBar>
#include "database.h"
#include "updatemanager.h"

class AppManagerWidget;
class FishModeWidget;
class ShutdownWidget;
class SettingsWidget;
class CollectionManagerWidget;
class RecommendedAppsWidget;
class UpdateDialog;
class UpdateProgressDialog;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void setStatusText(const QString &text);
    void resetApps();
    
private slots:
    void onTabChanged(int index);
    void onTrayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void onShowWindow();
    void onExitApp();
    void onUpdateAvailable(const UpdateInfo &info);
    void onNoUpdateAvailable();
    void onUpdateCheckFailed(const QString &error);
    void onUpdateNow();
    void onRemindLater();
    void onSkipThisVersion();
    void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void onDownloadFinished(const QString &filePath);
    void onDownloadFailed(const QString &error);
    void onExtractProgress(int percent);
    void onExtractFinished(const QString &extractPath);
    void onExtractFailed(const QString &error);
    void onInstallProgress(int percent);
    void onInstallFinished();
    void onInstallFailed(const QString &error);
    void onLogMessage(const QString &message);
    
protected:
    void closeEvent(QCloseEvent *event) override;
    void changeEvent(QEvent *event) override;
    
private:
    void setupUI();
    void setupTrayIcon();
    void initPresetApps();
    void initPresetApps(bool forceReset);
    QString findOfficeAppPath(const QString &appName);
    QString getOfficeVersion();
    
    Database *db;
    QTabWidget *tabWidget;
    AppManagerWidget *appManagerWidget;
    FishModeWidget *fishModeWidget;
    ShutdownWidget *shutdownWidget;
    SettingsWidget *settingsWidget;
    CollectionManagerWidget *collectionManagerWidget;
    RecommendedAppsWidget *recommendedAppsWidget;
    QSystemTrayIcon *trayIcon;
    QMenu *trayMenu;
    QAction *showWindowAction;
    QAction *exitAppAction;
    UpdateManager *updateManager;
    UpdateDialog *updateDialog;
    UpdateProgressDialog *updateProgressDialog;
    QLabel *statusLabel;
};

#endif
