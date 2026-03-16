#ifndef USERWIDGET_H
#define USERWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QLineEdit>
#include <QJsonObject>
#include <QDebug>
#include <QTimer>
#include <QProgressBar>
#include <QTextEdit>
#include <QTableWidget>
#include <QDialog>
#include <QHeaderView>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include "modules/core/database.h"
#include "modules/user/userapi.h"
#include "modules/user/changepassworddialog.h"

class MainWindow;

class UserWidget : public QWidget
{
    Q_OBJECT
public:
    explicit UserWidget(Database *db, QWidget *parent = nullptr);
    ~UserWidget();

    void setMainWindow(MainWindow *mainWindow);
    void refreshUserStatus();

private slots:
    void onLoginClicked();
    void onLogoutClicked();
    void onRegisterClicked();
    void onChangePasswordClicked();
    void onViewVersionsClicked();
    void onRefreshClicked();

    void onLoginSuccess(const UserInfo& user);
    void onLoginFailed(const QString& error);
    void onLogoutComplete();
    void onPasswordChanged();
    void onSyncConfigLoaded(const QJsonObject& configs);
    void onSyncConfigSaved();
    void onSyncFailed(const QString& error);

private:
    void setupUI();
    void updateLoginStatus();
    void setLoggedInState(bool loggedIn);
    void showMessage(const QString& msg, bool isError = false);

    Database *m_db;
    MainWindow *m_mainWindow;

    // UI Components
    QLabel *m_avatarLabel;
    QLabel *m_usernameLabel;
    QLabel *m_emailLabel;
    QLabel *m_statusLabel;
    QLabel *m_lastLoginLabel;
    QLabel *m_memberSinceLabel;
    QLabel *m_syncStatusLabel;

    QPushButton *m_loginBtn;
    QPushButton *m_registerBtn;
    QPushButton *m_logoutBtn;
    QPushButton *m_changePasswordBtn;
    QPushButton *m_viewVersionsBtn;
    QPushButton *m_refreshBtn;

    QGroupBox *m_userInfoGroup;
    QGroupBox *m_syncGroup;

    QProgressBar *m_syncProgressBar;
    QTextEdit *m_syncLogText;

    bool m_isLoggedIn;
    bool m_isUploading;  // 标记当前是否是上传操作
    UserInfo m_currentUser;
};

class BackupVersionsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit BackupVersionsDialog(QWidget *parent = nullptr);
    ~BackupVersionsDialog();

    void setDatabase(Database *db);
    void setMainWindow(MainWindow *mainWindow);

private:
    void setupUI();
    void loadVersions();
    void downloadConfig(const QString &configName);
    void deleteConfig(const QString &configName);

private slots:
    void onRefreshClicked();
    void onDownloadClicked();
    void onUploadWithNameClicked();

private:
    Database *m_db;
    MainWindow *m_mainWindow;

    QTableWidget *m_versionsTable;
    QPushButton *m_downloadBtn;
    QPushButton *m_uploadWithNameBtn;
    QPushButton *m_refreshBtn;
    QLabel *m_statusLabel;
    QProgressBar *m_progressBar;
    QLineEdit *m_backupNameEdit;

    QList<QVariantMap> m_versions;
};

#endif // USERWIDGET_H
