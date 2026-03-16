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
#include <QTableWidget>
#include <QHeaderView>
#include "modules/core/aiconfig.h"
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
#include "modules/user/userapi.h"

class UpdateProgressDialog;
class MainWindow;
class ChatTestDialog;

class SettingsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SettingsWidget(Database *db, QWidget *parent = nullptr);
    ~SettingsWidget();
    
    void setUpdateManager(UpdateManager *manager);
    void setMainWindow(MainWindow *mainWindow);
    void updateCloudLoginStatus(const UserInfo& user);
    
private slots:
    void onAutoStartToggled(int state);
    void onMinimizeToTrayToggled(int state);
    void onShowClosePromptToggled(int state);
    void onShowBottomAppBarToggled(int state);
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
    void onIconGenTestClicked();
    void onAddAIKey();
    void onEditAIKey();
    void onDeleteAIKey();
    void onSetDefaultAIKey();
    void onAIKeyTableSelectionChanged();

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
    QString getModelDisplayName(const QString &model);
    
    Database *db;
    MainWindow *mainWindow;
    UpdateManager *updateManager;
    QNetworkAccessManager *networkManager;
    
    QCheckBox *autoStartCheck;
    QCheckBox *minimizeToTrayCheck;
    QCheckBox *showClosePromptCheck;
    QCheckBox *showBottomAppBarCheck;
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
    QTableWidget *aiKeysTable;
    QTableWidget *aiImageKeysTable;
    QPushButton *addAIKeyBtn;
    QPushButton *editAIKeyBtn;
    QPushButton *deleteAIKeyBtn;
    QPushButton *setDefaultAIKeyBtn;
    QPushButton *addImageKeyBtn;
    QPushButton *editImageKeyBtn;
    QPushButton *deleteImageKeyBtn;
    QPushButton *setDefaultImageKeyBtn;
    QPushButton *cloudLoginBtn;
    QPushButton *cloudSyncBtn;
    QLabel *cloudStatusLabel;

private slots:
    void onAIImageKeyTableSelectionChanged();
    void onAddImageKey();
    void onEditImageKey();
    void onDeleteImageKey();
    void onSetDefaultImageKey();
    void onCloudLoginClicked();
    void onCloudSyncClicked();
    void onCloudLoginSuccess(const UserInfo& user);
    void onCloudLogoutClicked();
};

#endif
