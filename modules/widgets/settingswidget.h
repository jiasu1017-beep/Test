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
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QTabWidget>
#include <QLabel>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSettings>
#include <QCryptographicHash>
#include <QHostInfo>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDebug>
#include "modules/core/database.h"
#include "modules/update/updatemanager.h"

class UpdateProgressDialog;
class MainWindow;
class ChatTestDialog;

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
    void onOpenAISettings();
    void onAISettingsChanged();
    void onSaveAIConfig();
    void onTestAIConnection();
    void onChatTestClicked();

private:
    void setupUI();
    QWidget* createGeneralPage();
    QWidget* createShortcutPage();
    QWidget* createAIPage();
    QWidget* createStartupPage();
    QWidget* createUpdatePage();
    QWidget* createAboutPage();
    bool isShortcutConflict(const QString &shortcut);
    void loadAISettings();
    QString loadSavedAPIKey();
    QString getDefaultEndpoint(const QString &model);
    QString getModelName(const QString &model);
    
    Database *db;
    MainWindow *mainWindow;
    UpdateManager *updateManager;
    QNetworkAccessManager *networkManager;
    
    QCheckBox *autoStartCheck;
    QCheckBox *minimizeToTrayCheck;
    QCheckBox *showClosePromptCheck;
    QCheckBox *autoCheckUpdateCheck;
    QPushButton *checkUpdateButton;
    QLabel *statusLabel;
    QProgressDialog *progressDialog;
    UpdateProgressDialog *updateProgressDialog;
    QLabel *shortcutStatusLabel;
    
    QComboBox *aiModelCombo;
    QLineEdit *apiKeyEdit;
    QLineEdit *apiEndpointEdit;
    QPushButton *testAiBtn;
    QPushButton *saveAiBtn;
    QPushButton *chatTestBtn;
    QLabel *aiStatusLabel;
};

#endif
