#ifndef SETTINGSWIDGET_H
#define SETTINGSWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QGroupBox>
#include <QMessageBox>
#include <QProgressDialog>
#include <QKeySequenceEdit>
#include <QListWidget>
#include "modules/core/database.h"
#include "modules/update/updatemanager.h"

class UpdateProgressDialog;
class MainWindow;

class SettingsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SettingsWidget(Database *db, QWidget *parent = nullptr);
    
    void setUpdateManager(UpdateManager *manager);
    void setMainWindow(MainWindow *mainWindow);

private slots:
    void onAutoStartToggled(int state);
    void onMinimizeToTrayToggled(int state);
    void onShowClosePromptToggled(int state);
    void onAutoCheckUpdateToggled(int state);
    void onAboutClicked();
    void onCheckUpdateClicked();
    void onUpdateAvailable(const UpdateInfo &info);
    void onNoUpdateAvailable();
    void onUpdateCheckFailed(const QString &error);

private:
    void setupUI();
    bool isShortcutConflict(const QString &shortcut);
    
    Database *db;
    MainWindow *mainWindow;
    UpdateManager *updateManager;
    QCheckBox *autoStartCheck;
    QCheckBox *minimizeToTrayCheck;
    QCheckBox *showClosePromptCheck;
    QCheckBox *autoCheckUpdateCheck;
    QPushButton *checkUpdateButton;
    QLabel *statusLabel;
    QProgressDialog *progressDialog;
    UpdateProgressDialog *updateProgressDialog;

    QLabel *shortcutStatusLabel;
};

#endif
