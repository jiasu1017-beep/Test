#include "settingswidget.h"
#include "modules/user/userlogindialog.h"
#include "modules/user/userapi.h"
#include <QMessageBox>

void SettingsWidget::onCloudLoginClicked()
{
    if (UserManager::instance()->isLoggedIn()) {
        onCloudLogoutClicked();
        return;
    }

    UserLoginDialog *dialog = new UserLoginDialog(this);
    dialog->exec();
}

void SettingsWidget::onCloudSyncClicked()
{
    if (!UserManager::instance()->isLoggedIn()) {
        QMessageBox::warning(this, "提示", "请先登录云端账号");
        return;
    }

    cloudStatusLabel->setText("正在同步...");
    ConfigSync::instance()->fetchConfig();
}

void SettingsWidget::onCloudLoginSuccess(const UserInfo& user)
{
    cloudStatusLabel->setText("已登录: " + user.email);
    cloudStatusLabel->setStyleSheet("color: green;");
    cloudLoginBtn->setText("退出登录");

    QMessageBox::information(this, "登录成功",
        QString("欢迎回来，%1！\n\n用户 ID: %2\nVIP 等级：%3")
        .arg(user.username.isEmpty() ? user.email : user.username)
        .arg(user.id)
        .arg(user.vipLevel));

    // 不再自动同步，让用户手动选择是否同步
}

void SettingsWidget::onCloudLogoutClicked()
{
    UserManager::instance()->logout();
}

void SettingsWidget::onCloudChangePasswordClicked()
{
    if (!UserManager::instance()->isLoggedIn()) {
        QMessageBox::warning(this, "提示", "请先登录后再修改密码");
        return;
    }
    
    ChangePasswordDialog *changePwdDialog = new ChangePasswordDialog(this);
    changePwdDialog->exec();
}
