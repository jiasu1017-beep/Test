#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include <QVBoxLayout>
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

private:
    void setupUI();
    void initPresetApps();
    
    Database *db;
    QTabWidget *tabWidget;
    AppManagerWidget *appManagerWidget;
    FishModeWidget *fishModeWidget;
    ShutdownWidget *shutdownWidget;
    SettingsWidget *settingsWidget;
    CollectionManagerWidget *collectionManagerWidget;
    RecommendedAppsWidget *recommendedAppsWidget;
};

#endif
