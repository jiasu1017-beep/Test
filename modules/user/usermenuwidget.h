#ifndef USERMENUWIDGET_H
#define USERMENUWIDGET_H

#include <QObject>
#include <QToolButton>
#include <QMenu>
#include <QAction>

class UserWidget;
class ChangePasswordDialog;

class UserMenuWidget : public QObject
{
    Q_OBJECT
public:
    explicit UserMenuWidget(QWidget *parent = nullptr);
    ~UserMenuWidget();

    QToolButton *getUserMenuButton() const { return m_userMenuBtn; }

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

private:
    void setupMenu();
    void updateMenuState();

    QWidget *m_parent;
    QToolButton *m_userMenuBtn;
    QMenu *m_userMenu;
    QAction *m_loginAction;
    QAction *m_registerAction;
};

#endif // USERMENUWIDGET_H
