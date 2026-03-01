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
#include <QMessageBox>
#include <QPushButton>
#include <QHBoxLayout>
#include "modules/core/database.h"
#include "modules/update/updatemanager.h"

// Forward declarations
struct ShortcutStat;

class AppManagerWidget;
class FishModeWidget;
class ShutdownWidget;
class SettingsWidget;
class CollectionManagerWidget;
class RecommendedAppsWidget;
class UpdateDialog;
class UpdateProgressDialog;
class RemoteDesktopWidget;
class WorkLogWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void setStatusText(const QString &text);
    void resetApps();
    void refreshGlobalShortcut();
    
private slots:
    void onTabChanged(int index);
    void onTrayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void onShowWindow();
    void onExitApp();
    void onShortcutActivated();
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
    
#ifdef _WIN32
    bool nativeEvent(const QByteArray &eventType, void *message, long *result) override;
#endif
    
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
    RemoteDesktopWidget *remoteDesktopWidget;
    WorkLogWidget *workLogWidget;
    QLabel *statusLabel;
    
    void setupGlobalShortcut();
    void toggleWindow();
    void showShortcutTips();
};

#endif
