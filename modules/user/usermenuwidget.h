#ifndef USERMENUWIDGET_H
#define USERMENUWIDGET_H

#include <QObject>
#include <QToolButton>
#include <QMenu>
#include <QAction>
#include <QDateTime>

class ChangePasswordDialog;
class Database;
class MainWindow;

class UserMenuWidget : public QObject
{
    Q_OBJECT
public:
    explicit UserMenuWidget(QWidget *parent = nullptr);
    ~UserMenuWidget();

    QToolButton *getUserMenuButton() const { return m_userMenuBtn; }
    void setDatabase(Database *db);

signals:
    void statusMessageRequested(const QString &message, int durationMs = 3000);
    void showBackupVersionsDialogRequested();

public slots:
    void showLoginDialog();
    void showRegisterDialog();

private slots:
    void onLoginSuccess();
    void onLogoutComplete();
    void onManageConfig();
    void onChangePassword();
    void onLogout();
    void onTasksChanged();

private:
    void updateMenuState();
    void syncTasksToCloud();
    void onTasksSynced(const QJsonArray& tasks);
    void onTasksUploadComplete();
    void onTasksSyncFailed(const QString& error);
    void onSyncTimerTimeout();

    QWidget *m_parent;
    Database *m_db;
    QToolButton *m_userMenuBtn;
    QMenu *m_userMenu;
    QAction *m_loginAction;
    QAction *m_registerAction;
    QTimer *m_syncTimer;
    bool m_pendingSync;
    QDateTime m_lastSyncTime;
};

#endif // USERMENUWIDGET_H
