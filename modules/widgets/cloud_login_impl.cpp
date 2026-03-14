#include "settingswidget.h"
#include "modules/user/userlogindialog.h"
#include "modules/user/userapi.h"

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

    ConfigSync::instance()->fetchConfig();
}

void SettingsWidget::onCloudLogoutClicked()
{
    UserManager::instance()->logout();
}
