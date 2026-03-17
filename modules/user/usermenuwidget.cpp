#include "usermenuwidget.h"
#include "userapi.h"
#include "userlogindialog.h"
#include "changepassworddialog.h"
#include <QApplication>
#include <QStyle>
#include <QIcon>

UserMenuWidget::UserMenuWidget(QWidget *parent)
    : QObject(parent)
    , m_parent(parent)
    , m_userMenuBtn(nullptr)
    , m_userMenu(nullptr)
    , m_loginAction(nullptr)
    , m_registerAction(nullptr)
{
    // 创建用户菜单按钮
    m_userMenuBtn = new QToolButton(m_parent);
    m_userMenuBtn->setPopupMode(QToolButton::InstantPopup);
    m_userMenuBtn->setToolButtonStyle(Qt::ToolButtonIconOnly);
    m_userMenuBtn->setCursor(Qt::PointingHandCursor);
    m_userMenuBtn->setIcon(QIcon(":/img/icon0.png"));
    m_userMenuBtn->setFixedSize(24, 24);
    m_userMenuBtn->setToolTip("用户");
    m_userMenuBtn->setStyleSheet("QToolButton::menu-indicator { image: none; }");

    // 创建用户菜单
    m_userMenu = new QMenu(m_parent);
    m_loginAction = new QAction("登录", m_userMenu);
    m_registerAction = new QAction("注册", m_userMenu);
    m_userMenu->addAction(m_loginAction);
    m_userMenu->addAction(m_registerAction);

    m_userMenuBtn->setMenu(m_userMenu);

    // 连接菜单动作
    connect(m_loginAction, &QAction::triggered, this, &UserMenuWidget::showLoginDialog);
    connect(m_registerAction, &QAction::triggered, this, &UserMenuWidget::showRegisterDialog);

    // 连接用户管理器信号
    connect(UserManager::instance(), &UserManager::loginSuccess, this, &UserMenuWidget::onLoginSuccess);
    connect(UserManager::instance(), &UserManager::logoutComplete, this, &UserMenuWidget::onLogoutComplete);

    // 检查当前登录状态
    updateMenuState();
}

UserMenuWidget::~UserMenuWidget()
{
    // m_userMenuBtn 和 m_userMenu 的父对象是 m_parent，会自动清理
    // 断开与 UserManager 的信号连接
    disconnect(UserManager::instance(), nullptr, this, nullptr);
}

void UserMenuWidget::showLoginDialog()
{
    UserLoginDialog dialog(m_parent);
    dialog.exec();
}

void UserMenuWidget::showRegisterDialog()
{
    UserLoginDialog dialog(m_parent);
    dialog.switchToRegister();
    dialog.exec();
}

void UserMenuWidget::onLoginSuccess()
{
    // 清理旧action
    m_loginAction = nullptr;
    m_registerAction = nullptr;

    m_userMenu->clear();
    UserInfo user = UserManager::instance()->currentUser();
    QString displayName = user.username.isEmpty() ? user.email : user.username;

    QAction *userInfo = new QAction("当前用户: " + displayName, m_userMenu);
    userInfo->setEnabled(false);
    m_userMenu->addAction(userInfo);
    m_userMenu->addSeparator();

    QAction *manageConfigAction = new QAction("云端配置管理", m_userMenu);
    connect(manageConfigAction, &QAction::triggered, this, &UserMenuWidget::onManageConfig);
    m_userMenu->addAction(manageConfigAction);

    QAction *changePwdAction = new QAction("修改密码", m_userMenu);
    connect(changePwdAction, &QAction::triggered, this, &UserMenuWidget::onChangePassword);
    m_userMenu->addAction(changePwdAction);

    m_userMenu->addSeparator();

    QAction *logoutAction = new QAction("退出登录", m_userMenu);
    connect(logoutAction, &QAction::triggered, this, &UserMenuWidget::onLogout);
    m_userMenu->addAction(logoutAction);

    m_userMenuBtn->setToolTip("用户: " + displayName);
    m_userMenuBtn->setIcon(QIcon(":/img/icon.png"));

    emit statusMessageRequested("登录成功", 3000);
}

void UserMenuWidget::onLogoutComplete()
{
    m_userMenu->clear();

    // 重新创建登录和注册action
    m_loginAction = new QAction("登录", m_userMenu);
    m_registerAction = new QAction("注册", m_userMenu);
    connect(m_loginAction, &QAction::triggered, this, &UserMenuWidget::showLoginDialog);
    connect(m_registerAction, &QAction::triggered, this, &UserMenuWidget::showRegisterDialog);

    m_userMenu->addAction(m_loginAction);
    m_userMenu->addAction(m_registerAction);
    m_userMenuBtn->setToolTip("用户");
    m_userMenuBtn->setIcon(QIcon(":/img/icon0.png"));

    emit statusMessageRequested("已退出登录", 3000);
}

void UserMenuWidget::onManageConfig()
{
    emit showBackupVersionsDialogRequested();
}

void UserMenuWidget::onChangePassword()
{
    ChangePasswordDialog dlg(nullptr);
    dlg.exec();
}

void UserMenuWidget::onLogout()
{
    UserManager::instance()->logout();
}

void UserMenuWidget::updateMenuState()
{
    // 检查当前登录状态并更新菜单
    UserInfo user = UserManager::instance()->currentUser();
    if (!user.email.isEmpty()) {
        onLoginSuccess();
    }
}
