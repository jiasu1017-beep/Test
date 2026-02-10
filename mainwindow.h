#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QCloseEvent>
#include "database.h"

class AppManagerWidget;
class FishModeWidget;
class ShutdownWidget;
class SettingsWidget;
class CollectionManagerWidget;
class RecommendedAppsWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onTabChanged(int index);
    void onTrayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void onShowWindow();
    void onExitApp();
    
protected:
    void closeEvent(QCloseEvent *event) override;
    
private:
    void setupUI();
    void setupTrayIcon();
    void initPresetApps();
    
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
};

#endif
