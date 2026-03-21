#include "modules/widgets/userwidget.h"
#include "mainwindow.h"
#include "modules/core/database.h"
#include "modules/user/userapi.h"
#include <QStackedWidget>
#include <QFormLayout>
#include <QFrame>
#include <QScrollArea>
#include <QDateTime>
#include <QList>
#include <QTableWidget>
#include <QHeaderView>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonArray>
#include <QSettings>
#include <QAbstractItemView>
#include <QRadioButton>
#include <QDialog>
#include <QUrl>

UserWidget::UserWidget(Database *db, QWidget *parent)
    : QWidget(parent)
    , m_db(db)
    , m_mainWindow(nullptr)
    , m_isLoggedIn(false)
    , m_isUploading(false)
{
    setupUI();

    // 监听用户登录/登出状态
    connect(UserManager::instance(), &UserManager::loginSuccess, this, &UserWidget::onLoginSuccess);
    connect(UserManager::instance(), &UserManager::logoutComplete, this, &UserWidget::onLogoutComplete);

    // 监听同步状态
    connect(ConfigSync::instance(), &ConfigSync::configLoaded, this, &UserWidget::onSyncConfigLoaded);
    connect(ConfigSync::instance(), &ConfigSync::configSaved, this, &UserWidget::onSyncConfigSaved);
    connect(ConfigSync::instance(), &ConfigSync::syncFailed, this, &UserWidget::onSyncFailed);

    // 初始状态更新
    refreshUserStatus();
}

UserWidget::~UserWidget()
{
}

void UserWidget::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(15);

    // 标题
    QLabel *titleLabel = new QLabel("用户中心", this);
    titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #333;");
    mainLayout->addWidget(titleLabel);

    // 用户信息卡片
    m_userInfoGroup = new QGroupBox("用户信息", this);
    QVBoxLayout *userInfoLayout = new QVBoxLayout(m_userInfoGroup);

    // 头像和基本信息
    QHBoxLayout *profileLayout = new QHBoxLayout();

    // 头像
    m_avatarLabel = new QLabel(this);
    m_avatarLabel->setFixedSize(80, 80);
    m_avatarLabel->setStyleSheet("QLabel { background-color: #4a90d9; border-radius: 40px; font-size: 36px; color: white; qproperty-alignment: AlignCenter; }");
    m_avatarLabel->setText("👤");
    profileLayout->addWidget(m_avatarLabel);

    // 用户详细信息
    QVBoxLayout *infoLayout = new QVBoxLayout();
    infoLayout->setSpacing(8);

    m_usernameLabel = new QLabel("未登录", this);
    m_usernameLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #333;");

    m_emailLabel = new QLabel("", this);
    m_emailLabel->setStyleSheet("font-size: 14px; color: #666;");

    m_statusLabel = new QLabel("", this);
    m_statusLabel->setStyleSheet("font-size: 14px; color: #888;");

    infoLayout->addWidget(m_usernameLabel);
    infoLayout->addWidget(m_emailLabel);
    infoLayout->addWidget(m_statusLabel);

    profileLayout->addLayout(infoLayout);
    profileLayout->addStretch();

    userInfoLayout->addLayout(profileLayout);

    // 详细信息行
    QFormLayout *detailForm = new QFormLayout();
    detailForm->setSpacing(10);

    m_lastLoginLabel = new QLabel("从未登录", this);
    m_lastLoginLabel->setStyleSheet("color: #666;");

    m_memberSinceLabel = new QLabel("", this);
    m_memberSinceLabel->setStyleSheet("color: #666;");

    detailForm->addRow("最后登录:", m_lastLoginLabel);
    detailForm->addRow("注册时间:", m_memberSinceLabel);

    userInfoLayout->addLayout(detailForm);

    // 按钮行
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(10);

    m_loginBtn = new QPushButton("登录", this);
    m_loginBtn->setStyleSheet("QPushButton { background-color: #4a90d9; color: white; padding: 10px 25px; border-radius: 5px; font-size: 14px; } QPushButton:hover { background-color: #357abd; }");

    m_registerBtn = new QPushButton("注册", this);
    m_registerBtn->setStyleSheet("QPushButton { background-color: #28a745; color: white; padding: 10px 25px; border-radius: 5px; font-size: 14px; } QPushButton:hover { background-color: #218838; }");

    m_logoutBtn = new QPushButton("退出登录", this);
    m_logoutBtn->setStyleSheet("QPushButton { background-color: #dc3545; color: white; padding: 10px 25px; border-radius: 5px; font-size: 14px; } QPushButton:hover { background-color: #c82333; }");
    m_logoutBtn->setVisible(false);

    m_changePasswordBtn = new QPushButton("修改密码", this);
    m_changePasswordBtn->setStyleSheet("QPushButton { background-color: #ffc107; color: #333; padding: 10px 25px; border-radius: 5px; font-size: 14px; } QPushButton:hover { background-color: #e0a800; }");
    m_changePasswordBtn->setVisible(false);

    m_refreshBtn = new QPushButton("刷新状态", this);
    m_refreshBtn->setStyleSheet("QPushButton { background-color: #6c757d; color: white; padding: 10px 20px; border-radius: 5px; font-size: 14px; } QPushButton:hover { background-color: #5a6268; }");

    btnLayout->addWidget(m_loginBtn);
    btnLayout->addWidget(m_registerBtn);
    btnLayout->addWidget(m_logoutBtn);
    btnLayout->addWidget(m_changePasswordBtn);
    btnLayout->addWidget(m_refreshBtn);
    btnLayout->addStretch();

    userInfoLayout->addLayout(btnLayout);

    mainLayout->addWidget(m_userInfoGroup);

    // 云端同步卡片
    m_syncGroup = new QGroupBox("云端配置", this);
    QVBoxLayout *syncLayout = new QVBoxLayout(m_syncGroup);

    // 说明
    QLabel *syncDescLabel = new QLabel("管理云端配置，可上传本地配置或下载云端配置到本地", this);
    syncDescLabel->setStyleSheet("color: #666; font-size: 12px; margin-bottom: 10px;");
    syncLayout->addWidget(syncDescLabel);

    // 状态
    m_syncStatusLabel = new QLabel("未同步", this);
    m_syncStatusLabel->setStyleSheet("color: #888;");
    syncLayout->addWidget(m_syncStatusLabel);

    // 进度条
    m_syncProgressBar = new QProgressBar(this);
    m_syncProgressBar->setVisible(false);
    m_syncProgressBar->setRange(0, 100);
    syncLayout->addWidget(m_syncProgressBar);

    // 打开管理对话框按钮
    QHBoxLayout *manageBtnLayout = new QHBoxLayout();
    manageBtnLayout->setSpacing(15);

    m_viewVersionsBtn = new QPushButton("打开配置管理", this);
    m_viewVersionsBtn->setStyleSheet("QPushButton { background-color: #6f42c1; color: white; padding: 15px 40px; border-radius: 5px; font-size: 16px; } QPushButton:hover { background-color: #5a32a3; }");
    m_viewVersionsBtn->setEnabled(false);
    m_viewVersionsBtn->setToolTip("打开配置管理对话框，上传或下载配置");

    manageBtnLayout->addWidget(m_viewVersionsBtn);
    manageBtnLayout->addStretch();

    syncLayout->addLayout(manageBtnLayout);

    // 绑定按钮信号
    connect(m_viewVersionsBtn, &QPushButton::clicked, this, &UserWidget::onViewVersionsClicked);

    // 同步日志
    m_syncLogText = new QTextEdit(this);
    m_syncLogText->setReadOnly(true);
    m_syncLogText->setMaximumHeight(120);
    m_syncLogText->setStyleSheet("QTextEdit { background-color: #f8f9fa; border: 1px solid #dee2e6; border-radius: 5px; padding: 10px; }");
    syncLayout->addWidget(m_syncLogText);

    mainLayout->addWidget(m_syncGroup);

    mainLayout->addStretch();

    // 信号连接
    connect(m_loginBtn, &QPushButton::clicked, this, &UserWidget::onLoginClicked);
    connect(m_registerBtn, &QPushButton::clicked, this, &UserWidget::onRegisterClicked);
    connect(m_logoutBtn, &QPushButton::clicked, this, &UserWidget::onLogoutClicked);
    connect(m_changePasswordBtn, &QPushButton::clicked, this, &UserWidget::onChangePasswordClicked);
    connect(m_refreshBtn, &QPushButton::clicked, this, &UserWidget::onRefreshClicked);

    updateLoginStatus();
}

void UserWidget::setMainWindow(MainWindow *mainWindow)
{
    m_mainWindow = mainWindow;
}

void UserWidget::refreshUserStatus()
{
    updateLoginStatus();

    if (UserManager::instance()->isLoggedIn()) {
        m_currentUser = UserManager::instance()->currentUser();
        onLoginSuccess(m_currentUser);
    }
}

void UserWidget::showBackupVersionsDialog()
{
    onViewVersionsClicked();
}

void UserWidget::updateLoginStatus()
{
    bool loggedIn = UserManager::instance()->isLoggedIn();
    setLoggedInState(loggedIn);
}

void UserWidget::setLoggedInState(bool loggedIn)
{
    m_isLoggedIn = loggedIn;

    if (loggedIn) {
        m_loginBtn->setVisible(false);
        m_registerBtn->setVisible(false);
        m_logoutBtn->setVisible(true);
        m_changePasswordBtn->setVisible(true);
        if (m_viewVersionsBtn) {
            m_viewVersionsBtn->setEnabled(true);
        }

        const UserInfo &user = UserManager::instance()->currentUser();
        m_usernameLabel->setText(user.username.isEmpty() ? user.email : user.username);
        m_emailLabel->setText(user.email);
        m_statusLabel->setText("已登录");
        m_statusLabel->setStyleSheet("font-size: 14px; color: #28a745; font-weight: bold;");

        if (!user.lastLogin.isEmpty()) {
            // 格式化 ISO 日期为本地可读格式
            QString formatted = user.lastLogin;
            formatted.replace("T", " ");
            formatted.replace("Z", "");
            if (formatted.contains(".")) {
                formatted = formatted.section(".", 0, 0);
            }
            m_lastLoginLabel->setText(formatted);
        }

        if (!user.createdAt.isEmpty()) {
            // 格式化 ISO 日期为本地可读格式
            QString formatted = user.createdAt;
            formatted.replace("T", " ");
            formatted.replace("Z", "");
            if (formatted.contains(".")) {
                formatted = formatted.section(".", 0, 0);
            }
            m_memberSinceLabel->setText(formatted);
        }
    } else {
        m_loginBtn->setVisible(true);
        m_registerBtn->setVisible(true);
        m_logoutBtn->setVisible(false);
        m_changePasswordBtn->setVisible(false);
        if (m_viewVersionsBtn) {
            m_viewVersionsBtn->setEnabled(false);
        }

        m_usernameLabel->setText("未登录");
        m_emailLabel->setText("请登录以使用云端配置功能");
        m_statusLabel->setText("未登录");
        m_statusLabel->setStyleSheet("font-size: 14px; color: #888;");
        m_lastLoginLabel->setText("从未登录");
        m_memberSinceLabel->setText("");
    }
}

void UserWidget::onLoginClicked()
{
    if (m_mainWindow) {
        m_mainWindow->showLoginDialog();
    }
}

void UserWidget::onRegisterClicked()
{
    if (m_mainWindow) {
        m_mainWindow->showRegisterDialog();
    }
}

void UserWidget::onLogoutClicked()
{
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "确认退出",
        "确定要退出登录吗？",
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        UserManager::instance()->logout();
    }
}

void UserWidget::onChangePasswordClicked()
{
    ChangePasswordDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        showMessage("密码修改成功，请重新登录", false);
        UserManager::instance()->logout();
    }
}

void UserWidget::onRefreshClicked()
{
    refreshUserStatus();
    if (m_isLoggedIn) {
        UserManager::instance()->fetchProfile();
    }
    showMessage("已刷新", false);
}

void UserWidget::onLoginSuccess(const UserInfo &user)
{
    m_currentUser = user;
    setLoggedInState(true);

    // 更新云端同步状态
    m_syncStatusLabel->setText("已登录");
    m_syncStatusLabel->setStyleSheet("color: #28a745;");

    QString msg = QString("登录成功！欢迎 %1").arg(user.username.isEmpty() ? user.email : user.username);
    //showMessage(msg, false);
    m_syncLogText->append(QString("[%1] %2").arg(QDateTime::currentDateTime().toString("HH:mm:ss"), msg));
}

void UserWidget::onLoginFailed(const QString &error)
{
    showMessage(QString("登录失败: %1").arg(error), true);
    m_syncLogText->append(QString("[%1] 登录失败: %2").arg(QDateTime::currentDateTime().toString("HH:mm:ss"), error));
}

void UserWidget::onLogoutComplete()
{
    setLoggedInState(false);
    m_syncStatusLabel->setText("未同步");
    m_syncStatusLabel->setStyleSheet("color: #888;");
    m_syncLogText->append(QString("[%1] 已退出登录").arg(QDateTime::currentDateTime().toString("HH:mm:ss")));
    showMessage("已退出登录", false);
}

void UserWidget::onPasswordChanged()
{
    showMessage("密码修改成功", false);
}

void UserWidget::onSyncConfigLoaded(const QJsonObject &configs)
{
    m_syncProgressBar->setValue(80);
    m_syncLogText->append(QString("[%1] 配置加载完成，正在合并到本地...").arg(QDateTime::currentDateTime().toString("HH:mm:ss")));

    int addedCount = 0;

    // 处理应用列表
    if (configs.contains("apps") && m_db) {
        QJsonArray cloudApps = configs["apps"].toArray();
        QList<AppInfo> localApps = m_db->getAllApps();
        QSet<int> localAppIds;
        for (const AppInfo &app : localApps) {
            localAppIds.insert(app.id);
        }

        for (const QJsonValue &appVal : cloudApps) {
            QJsonObject appObj = appVal.toObject();
            int appId = appObj["id"].toInt();

            // 只添加本地不存在的应用
            if (!localAppIds.contains(appId)) {
                AppInfo app;
                app.id = appId;
                app.name = appObj["name"].toString();
                app.path = appObj["path"].toString();
                app.iconPath = appObj["iconPath"].toString();
                app.category = appObj["category"].toString();
                app.sortOrder = appObj["sortOrder"].toInt();
                app.isFavorite = appObj["isFavorite"].toBool();
                app.useCount = 0;
                app.type = AppType_Executable;
                app.remoteDesktopId = 0;
                app.arguments = "";

                if (m_db->addApp(app)) {
                    addedCount++;
                    m_syncLogText->append(QString("[%1] 添加应用: %2").arg(QDateTime::currentDateTime().toString("HH:mm:ss"), app.name));
                }
            }
        }
    }

    // 处理收藏列表
    if (configs.contains("collections") && m_db) {
        QJsonArray cloudCollections = configs["collections"].toArray();
        QList<AppCollection> localCollections = m_db->getAllCollections();
        QSet<QString> localCollectionNames;
        for (const AppCollection &col : localCollections) {
            localCollectionNames.insert(col.name);
        }

        for (const QJsonValue &colVal : cloudCollections) {
            QJsonObject colObj = colVal.toObject();
            QString colName = colObj["name"].toString();

            // 只添加本地不存在的收藏
            if (!localCollectionNames.contains(colName)) {
                AppCollection col;
                col.name = colName;
                col.description = colObj["description"].toString();
                col.tag = "";
                col.sortPriority = 0;

                // 读取 appIds
                QJsonArray appIdsArray = colObj["appIds"].toArray();
                for (const QJsonValue &idVal : appIdsArray) {
                    col.appIds.append(idVal.toInt());
                }

                if (m_db->addCollection(col)) {
                    addedCount++;
                    m_syncLogText->append(QString("[%1] 添加收藏: %2").arg(QDateTime::currentDateTime().toString("HH:mm:ss"), colName));
                }
            }
        }
    }

    // 处理远程桌面配置
    if (configs.contains("remoteDesktops") && m_db) {
        // 支持两种格式：直接是数组，或嵌套在对象中
        QJsonValue rdValue = configs["remoteDesktops"];
        QJsonArray cloudDesktops;
        if (rdValue.isArray()) {
            cloudDesktops = rdValue.toArray();
        } else if (rdValue.isObject()) {
            // 可能是 { "remoteDesktops": { "remoteDesktops": [...] } }
            QJsonObject rdObj = rdValue.toObject();
            if (rdObj.contains("remoteDesktops")) {
                cloudDesktops = rdObj["remoteDesktops"].toArray();
            }
        }

        QList<RemoteDesktopConnection> localDesktops = m_db->getAllRemoteDesktops();
        QSet<QString> localDesktopNames;
        for (const RemoteDesktopConnection &rd : localDesktops) {
            localDesktopNames.insert(rd.name);
        }

        for (const QJsonValue &rdVal : cloudDesktops) {
            QJsonObject rdObj = rdVal.toObject();
            QString rdName = rdObj["name"].toString();

            // 只添加本地不存在的远程桌面
            if (!localDesktopNames.contains(rdName)) {
                RemoteDesktopConnection rd;
                rd.name = rdName;
                rd.hostAddress = rdObj["hostAddress"].toString();
                rd.port = rdObj["port"].toInt();
                rd.username = rdObj["username"].toString();
                rd.password = "";
                rd.domain = "";
                rd.displayName = rdName;
                rd.screenWidth = 0;
                rd.screenHeight = 0;
                rd.fullScreen = false;
                rd.useAllMonitors = false;
                rd.enableAudio = false;
                rd.enableClipboard = false;
                rd.enablePrinter = false;
                rd.enableDrive = false;

                if (m_db->addRemoteDesktop(rd)) {
                    addedCount++;
                    m_syncLogText->append(QString("[%1] 添加远程桌面: %2").arg(QDateTime::currentDateTime().toString("HH:mm:ss"), rdName));
                }
            }
        }
    }

    // 处理其他配置保存到 QSettings
    QSettings settings;
    for (auto it = configs.begin(); it != configs.end(); ++it) {
        QString key = it.key();
        // 跳过已经在数据库中处理的配置
        if (key == "apps" || key == "collections" || key == "remoteDesktops" || key == "tasks") {
            continue;
        }

        QJsonValue value = it.value();

        // 检查本地是否已存在
        if (!settings.contains(key)) {
            if (value.isObject()) {
                settings.setValue(key, QVariant::fromValue(value.toObject()));
            } else if (value.isArray()) {
                settings.setValue(key, QVariant::fromValue(value.toArray()));
            } else if (value.isString()) {
                settings.setValue(key, value.toString());
            } else if (value.isBool()) {
                settings.setValue(key, value.toBool());
            } else if (value.isDouble()) {
                settings.setValue(key, value.toDouble());
            }
            m_syncLogText->append(QString("[%1] 添加配置: %2").arg(QDateTime::currentDateTime().toString("HH:mm:ss"), key));
        }
    }

    m_syncLogText->append(QString("[%1] 合并完成，共添加 %2 项新数据").arg(QDateTime::currentDateTime().toString("HH:mm:ss")).arg(addedCount));

    // 完成后更新状态
    m_syncProgressBar->setValue(100);
    m_syncStatusLabel->setText("合并完成");
    m_syncStatusLabel->setStyleSheet("color: #28a745;");
    m_syncLogText->append(QString("[%1] 同步完成！").arg(QDateTime::currentDateTime().toString("HH:mm:ss")));

    // 刷新所有窗口
    if (m_mainWindow) {
        m_mainWindow->refreshAllWidgets();
    }

    showMessage(QString("配置同步成功，添加了 %1 项新数据").arg(addedCount), false);

    QTimer::singleShot(1000, this, [this]() {
        m_syncProgressBar->setVisible(false);
    });
}

void UserWidget::onSyncConfigSaved()
{
    m_syncProgressBar->setValue(100);

    if (m_isUploading) {
        // 上传完成
        m_syncStatusLabel->setText("上传完成");
        m_syncStatusLabel->setStyleSheet("color: #28a745;");
        m_syncLogText->append(QString("[%1] 上传完成！").arg(QDateTime::currentDateTime().toString("HH:mm:ss")));
        showMessage("配置上传成功", false);
        m_isUploading = false;
    } else {
        // 下载完成（实际上现在下载是onSyncConfigLoaded处理的）
        m_syncStatusLabel->setText("同步完成");
        m_syncStatusLabel->setStyleSheet("color: #28a745;");
        m_syncLogText->append(QString("[%1] 同步完成！").arg(QDateTime::currentDateTime().toString("HH:mm:ss")));
        showMessage("配置同步成功", false);
    }

    QTimer::singleShot(1000, this, [this]() {
        m_syncProgressBar->setVisible(false);
    });
}

void UserWidget::onSyncFailed(const QString &error)
{
    m_syncProgressBar->setVisible(false);
    m_syncStatusLabel->setText("同步失败");
    m_syncStatusLabel->setStyleSheet("color: #dc3545;");
    m_syncLogText->append(QString("[%1] 同步失败: %2").arg(QDateTime::currentDateTime().toString("HH:mm:ss"), error));

    showMessage(QString("同步失败: %1").arg(error), true);
}

void UserWidget::showMessage(const QString &msg, bool isError)
{
    if (isError) {
        QMessageBox::warning(this, "提示", msg);
    } else {
        QMessageBox::information(this, "提示", msg);
    }
}

void UserWidget::onViewVersionsClicked()
{
    if (!m_isLoggedIn) {
        showMessage("请先登录", true);
        return;
    }

    // 弹出备份版本选择对话框
    BackupVersionsDialog *dialog = new BackupVersionsDialog(this);
    dialog->setDatabase(m_db);
    dialog->setMainWindow(m_mainWindow);
    dialog->exec();
}

// BackupVersionsDialog implementation
BackupVersionsDialog::BackupVersionsDialog(QWidget *parent)
    : QDialog(parent)
    , m_db(nullptr)
    , m_mainWindow(nullptr)
{
    setWindowTitle("云端配置管理");
    setMinimumSize(600, 450);
    setupUI();
    loadVersions();
}

BackupVersionsDialog::~BackupVersionsDialog()
{
}

void BackupVersionsDialog::setDatabase(Database *db)
{
    m_db = db;
}

void BackupVersionsDialog::setMainWindow(MainWindow *mainWindow)
{
    m_mainWindow = mainWindow;
}

void BackupVersionsDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    mainLayout->setSpacing(10);

    // 标题
    QLabel *titleLabel = new QLabel("云端备份版本", this);
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold;");
    mainLayout->addWidget(titleLabel);

    // 说明
    QLabel *descLabel = new QLabel("您可以上传不同设备的配置（如台式机、笔记本），或从云端下载已有配置", this);
    descLabel->setStyleSheet("color: #666;");
    mainLayout->addWidget(descLabel);

    // 版本表格
    m_versionsTable = new QTableWidget(this);
    m_versionsTable->setColumnCount(5);
    m_versionsTable->setHorizontalHeaderLabels({"配置名称", "创建时间", "更新时间", "下载", "删除"});
    m_versionsTable->horizontalHeader()->setStretchLastSection(true);
    m_versionsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_versionsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_versionsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    mainLayout->addWidget(m_versionsTable);

    // 状态和进度
    m_statusLabel = new QLabel("", this);
    mainLayout->addWidget(m_statusLabel);

    m_progressBar = new QProgressBar(this);
    m_progressBar->setVisible(false);
    mainLayout->addWidget(m_progressBar);

    // 按钮区域
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(10);

    // 上传新配置（需要名称）
    QHBoxLayout *uploadLayout = new QHBoxLayout();
    m_backupNameEdit = new QLineEdit(this);
    m_backupNameEdit->setPlaceholderText("输入配置名称（如：台式机、笔记本）");
    m_backupNameEdit->setMinimumWidth(250);
    uploadLayout->addWidget(new QLabel("配置名称:"));
    uploadLayout->addWidget(m_backupNameEdit);

    // 数据类型选择（仅上传时使用）
    m_includeAppsCheck = new QCheckBox("应用", this);
    m_includeAppsCheck->setChecked(true);
    uploadLayout->addWidget(m_includeAppsCheck);

    m_includeCollectionsCheck = new QCheckBox("收藏", this);
    m_includeCollectionsCheck->setChecked(true);
    uploadLayout->addWidget(m_includeCollectionsCheck);

    m_includeRemoteDesktopsCheck = new QCheckBox("远程桌面", this);
    m_includeRemoteDesktopsCheck->setChecked(true);
    uploadLayout->addWidget(m_includeRemoteDesktopsCheck);

    m_includeSettingsCheck = new QCheckBox("设置", this);
    m_includeSettingsCheck->setChecked(true);
    uploadLayout->addWidget(m_includeSettingsCheck);

    m_uploadWithNameBtn = new QPushButton("上传配置", this);
    m_uploadWithNameBtn->setStyleSheet("QPushButton { background-color: #28a745; color: white; padding: 8px 16px; }");
    uploadLayout->addWidget(m_uploadWithNameBtn);

    m_refreshBtn = new QPushButton("刷新", this);
    m_refreshBtn->setStyleSheet("QPushButton { background-color: #6c757d; color: white; padding: 8px 16px; }");
    uploadLayout->addWidget(m_refreshBtn);

    btnLayout->addLayout(uploadLayout);
    btnLayout->addStretch();

    m_downloadBtn = new QPushButton("下载选中配置", this);
    m_downloadBtn->setStyleSheet("QPushButton { background-color: #17a2b8; color: white; padding: 8px 16px; }");
    m_downloadBtn->setEnabled(false);
    btnLayout->addWidget(m_downloadBtn);

    mainLayout->addLayout(btnLayout);

    // 关闭按钮
    QPushButton *closeBtn = new QPushButton("关闭", this);
    closeBtn->setStyleSheet("QPushButton { padding: 8px 20px; }");
    mainLayout->addWidget(closeBtn);

    // 信号连接
    connect(m_refreshBtn, &QPushButton::clicked, this, &BackupVersionsDialog::onRefreshClicked);
    connect(m_downloadBtn, &QPushButton::clicked, this, &BackupVersionsDialog::onDownloadClicked);
    connect(m_uploadWithNameBtn, &QPushButton::clicked, this, &BackupVersionsDialog::onUploadWithNameClicked);
    connect(m_versionsTable, &QTableWidget::itemSelectionChanged, this, [this]() {
        m_downloadBtn->setEnabled(m_versionsTable->currentRow() >= 0);
    });
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
}

void BackupVersionsDialog::loadVersions()
{
    m_versionsTable->setRowCount(0);
    m_statusLabel->setText("正在加载云端配置...");
    m_statusLabel->setStyleSheet("color: #666;");

    QString token = UserManager::instance()->getToken();
    if (token.isEmpty()) {
        m_statusLabel->setText("请先登录");
        m_statusLabel->setStyleSheet("color: red;");
        return;
    }

    QUrl url(QString(CLOUD_API_URL) + "/api/config/profiles");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", ("Bearer " + token).toUtf8());

    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    QNetworkReply *reply = manager->get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        m_statusLabel->setText("");

        if (reply->error() != QNetworkReply::NoError) {
            m_statusLabel->setText("加载失败: " + reply->errorString());
            m_statusLabel->setStyleSheet("color: red;");
            return;
        }

        QByteArray responseData = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(responseData);
        QJsonObject obj = doc.object();

        if (!obj["success"].toBool()) {
            m_statusLabel->setText("加载失败: " + obj["error"].toString());
            m_statusLabel->setStyleSheet("color: red;");
            return;
        }

        QJsonArray profiles = obj["profiles"].toArray();
        m_versions.clear();

        for (int i = 0; i < profiles.size(); ++i) {
            QJsonObject p = profiles[i].toObject();
            QVariantMap map;
            map["config_name"] = p["config_name"].toString();
            map["created_at"] = p["created_at"].toString();
            map["updated_at"] = p["updated_at"].toString();
            m_versions.append(map);

            int row = m_versionsTable->rowCount();
            m_versionsTable->insertRow(row);

            m_versionsTable->setItem(row, 0, new QTableWidgetItem(p["config_name"].toString()));
            m_versionsTable->setItem(row, 1, new QTableWidgetItem(p["created_at"].toString()));
            m_versionsTable->setItem(row, 2, new QTableWidgetItem(p["updated_at"].toString()));

            // 下载按钮
            QPushButton *downloadBtn = new QPushButton("下载");
            downloadBtn->setProperty("config_name", p["config_name"].toString());
            downloadBtn->setStyleSheet("QPushButton { background-color: #17a2b8; color: white; padding: 4px 8px; font-size: 11px; }");
            connect(downloadBtn, &QPushButton::clicked, this, [this, p]() {
                downloadConfig(p["config_name"].toString());
            });
            m_versionsTable->setCellWidget(row, 3, downloadBtn);

            // 删除按钮
            QPushButton *deleteBtn = new QPushButton("删除");
            deleteBtn->setProperty("config_name", p["config_name"].toString());
            deleteBtn->setStyleSheet("QPushButton { background-color: #dc3545; color: white; padding: 4px 8px; font-size: 11px; }");
            connect(deleteBtn, &QPushButton::clicked, this, [this, p]() {
                QString configName = p["config_name"].toString();
                int ret = QMessageBox::question(this, "确认删除",
                    QString("确定要删除配置 \"%1\" 吗？此操作不可恢复。").arg(configName),
                    QMessageBox::Yes | QMessageBox::No);
                if (ret == QMessageBox::Yes) {
                    deleteConfig(configName);
                }
            });
            m_versionsTable->setCellWidget(row, 4, deleteBtn);
        }

        m_statusLabel->setText(QString("共 %1 个配置（如：台式机、笔记本等）").arg(profiles.size()));
        m_statusLabel->setStyleSheet("color: #28a745;");
    });
}

void BackupVersionsDialog::onRefreshClicked()
{
    loadVersions();
}

void BackupVersionsDialog::onDownloadClicked()
{
    int currentRow = m_versionsTable->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, "提示", "请选择一个配置");
        return;
    }

    QString configName = m_versions[currentRow]["config_name"].toString();
    downloadConfig(configName);
}

void BackupVersionsDialog::downloadConfig(const QString &configName)
{
    m_progressBar->setVisible(true);
    m_progressBar->setValue(30);
    m_statusLabel->setText(QString("正在下载配置 [%1]...").arg(configName));
    m_statusLabel->setStyleSheet("color: #666;");

    QString token = UserManager::instance()->getToken();
    if (token.isEmpty()) {
        QMessageBox::warning(this, "提示", "请先登录");
        return;
    }

    QUrl url(QString(CLOUD_API_URL) + "/api/config/profiles/" + QUrl::toPercentEncoding(configName));
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", ("Bearer " + token).toUtf8());

    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    QNetworkReply *reply = manager->get(request);

    connect(reply, &QNetworkReply::finished, this, [this, configName, reply]() {
        m_progressBar->setVisible(false);

        if (reply->error() != QNetworkReply::NoError) {
            m_statusLabel->setText("下载失败: " + reply->errorString());
            m_statusLabel->setStyleSheet("color: red;");
            reply->deleteLater();
            return;
        }

        QByteArray responseData = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(responseData);
        QJsonObject obj = doc.object();

        if (!obj["success"].toBool()) {
            m_statusLabel->setText("下载失败: " + obj["error"].toString());
            m_statusLabel->setStyleSheet("color: red;");
            reply->deleteLater();
            return;
        }

        QJsonObject configs = obj["configs"].toObject();

        // 合并到本地数据库
        if (m_db) {
            // 处理应用列表
            if (configs.contains("apps")) {
                QJsonArray cloudApps = configs["apps"].toArray();
                QList<AppInfo> localApps = m_db->getAllApps();
                QSet<int> localAppIds;
                for (const AppInfo &app : localApps) {
                    localAppIds.insert(app.id);
                }

                for (const QJsonValue &appVal : cloudApps) {
                    QJsonObject appObj = appVal.toObject();
                    int appId = appObj["id"].toInt();

                    if (!localAppIds.contains(appId)) {
                        AppInfo app;
                        app.id = appId;
                        app.name = appObj["name"].toString();
                        app.path = appObj["path"].toString();
                        app.iconPath = appObj["iconPath"].toString();
                        app.category = appObj["category"].toString();
                        app.sortOrder = appObj["sortOrder"].toInt();
                        app.isFavorite = appObj["isFavorite"].toBool();
                        app.useCount = 0;
                        app.type = AppType_Executable;
                        app.remoteDesktopId = 0;
                        app.arguments = "";

                        m_db->addApp(app);
                    }
                }
            }

            // 处理收藏列表
            if (configs.contains("collections")) {
                QJsonArray cloudCollections = configs["collections"].toArray();
                QList<AppCollection> localCollections = m_db->getAllCollections();
                QSet<QString> localCollectionNames;
                for (const AppCollection &col : localCollections) {
                    localCollectionNames.insert(col.name);
                }

                for (const QJsonValue &colVal : cloudCollections) {
                    QJsonObject colObj = colVal.toObject();
                    QString colName = colObj["name"].toString();

                    if (!localCollectionNames.contains(colName)) {
                        AppCollection col;
                        col.name = colName;
                        col.description = colObj["description"].toString();
                        col.tag = "";
                        col.sortPriority = 0;

                        QJsonArray appIdsArray = colObj["appIds"].toArray();
                        for (const QJsonValue &idVal : appIdsArray) {
                            col.appIds.append(idVal.toInt());
                        }

                        m_db->addCollection(col);
                    }
                }
            }

            // 处理远程桌面配置
            if (configs.contains("remoteDesktops")) {
                QJsonArray cloudDesktops = configs["remoteDesktops"].toArray();
                QList<RemoteDesktopConnection> localDesktops = m_db->getAllRemoteDesktops();
                QSet<QString> localDesktopNames;
                for (const RemoteDesktopConnection &rd : localDesktops) {
                    localDesktopNames.insert(rd.name);
                }

                for (const QJsonValue &rdVal : cloudDesktops) {
                    QJsonObject rdObj = rdVal.toObject();
                    QString rdName = rdObj["name"].toString();

                    if (!localDesktopNames.contains(rdName)) {
                        RemoteDesktopConnection rd;
                        rd.name = rdName;
                        rd.hostAddress = rdObj["hostAddress"].toString();
                        rd.port = rdObj["port"].toInt();
                        rd.username = rdObj["username"].toString();
                        rd.password = "";
                        rd.domain = "";
                        rd.displayName = rdName;
                        rd.screenWidth = 0;
                        rd.screenHeight = 0;
                        rd.fullScreen = false;
                        rd.useAllMonitors = false;
                        rd.enableAudio = false;
                        rd.enableClipboard = false;
                        rd.enablePrinter = false;
                        rd.enableDrive = false;

                        m_db->addRemoteDesktop(rd);
                    }
                }
            }

            // 处理其他配置保存到 QSettings
            QSettings settings;
            for (auto it = configs.begin(); it != configs.end(); ++it) {
                QString key = it.key();
                if (key == "apps" || key == "collections" || key == "remoteDesktops" || key == "tasks") {
                    continue;
                }

                QJsonValue value = it.value();
                if (!settings.contains(key)) {
                    if (value.isObject()) {
                        settings.setValue(key, QVariant::fromValue(value.toObject()));
                    } else if (value.isArray()) {
                        settings.setValue(key, QVariant::fromValue(value.toArray()));
                    } else if (value.isString()) {
                        settings.setValue(key, value.toString());
                    } else if (value.isBool()) {
                        settings.setValue(key, value.toBool());
                    } else if (value.isDouble()) {
                        settings.setValue(key, value.toDouble());
                    }
                }
            }
        }

        m_statusLabel->setText(QString("配置 [%1] 下载并合并成功！").arg(configName));
        m_statusLabel->setStyleSheet("color: #28a745;");

        QMessageBox::information(this, "成功",
            QString("配置 [%1] 已下载并合并到本地！").arg(configName));

        // 刷新窗口
        if (m_mainWindow) {
            m_mainWindow->refreshAllWidgets();
        }

        reply->deleteLater();
    });
}

void BackupVersionsDialog::deleteConfig(const QString &configName)
{
    m_statusLabel->setText(QString("正在删除配置 [%1]...").arg(configName));
    m_statusLabel->setStyleSheet("color: #666;");

    QString token = UserManager::instance()->getToken();
    if (token.isEmpty()) {
        QMessageBox::warning(this, "提示", "请先登录");
        return;
    }

    QUrl url(QString(CLOUD_API_URL) + "/api/config/profiles/" + QUrl::toPercentEncoding(configName));
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", ("Bearer " + token).toUtf8());

    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    QNetworkReply *reply = manager->deleteResource(request);

    connect(reply, &QNetworkReply::finished, this, [this, configName, reply]() {
        if (reply->error() != QNetworkReply::NoError) {
            m_statusLabel->setText("删除失败: " + reply->errorString());
            m_statusLabel->setStyleSheet("color: red;");
            reply->deleteLater();
            return;
        }

        QByteArray responseData = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(responseData);
        QJsonObject obj = doc.object();

        if (!obj["success"].toBool()) {
            m_statusLabel->setText("删除失败: " + obj["error"].toString());
            m_statusLabel->setStyleSheet("color: red;");
            reply->deleteLater();
            return;
        }

        m_statusLabel->setText(QString("配置 [%1] 已删除").arg(configName));
        m_statusLabel->setStyleSheet("color: #28a745;");

        // 刷新列表
        loadVersions();

        reply->deleteLater();
    });
}

void BackupVersionsDialog::onUploadWithNameClicked()
{
    // 先读取本地配置
    QJsonObject localConfigs;

    if (m_db) {
        // 获取应用列表（根据用户选择）
        if (m_includeAppsCheck->isChecked()) {
            QList<AppInfo> apps = m_db->getAllApps();
            QJsonArray appsArray;
            for (const AppInfo &app : apps) {
                QJsonObject appObj;
                appObj["id"] = app.id;
                appObj["name"] = app.name;
                appObj["path"] = app.path;
                appObj["iconPath"] = app.iconPath;
                appObj["category"] = app.category;
                appObj["sortOrder"] = app.sortOrder;
                appObj["isFavorite"] = app.isFavorite;
                appsArray.append(appObj);
            }
            localConfigs["apps"] = appsArray;
        }

        // 获取收藏列表（根据用户选择）
        if (m_includeCollectionsCheck->isChecked()) {
            QList<AppCollection> collections = m_db->getAllCollections();
            QJsonArray collectionsArray;
            for (const AppCollection &col : collections) {
                QJsonObject colObj;
                colObj["id"] = col.id;
                colObj["name"] = col.name;
                colObj["description"] = col.description;
                QJsonArray appIdsArray;
                for (int appId : col.appIds) {
                    appIdsArray.append(appId);
                }
                colObj["appIds"] = appIdsArray;
                collectionsArray.append(colObj);
            }
            localConfigs["collections"] = collectionsArray;
        }

        // 获取远程桌面配置（根据用户选择）
        if (m_includeRemoteDesktopsCheck->isChecked()) {
            QList<RemoteDesktopConnection> desktops = m_db->getAllRemoteDesktops();
            QJsonArray desktopsArray;
            for (const RemoteDesktopConnection &rd : desktops) {
                QJsonObject rdObj;
                rdObj["id"] = rd.id;
                rdObj["name"] = rd.name;
                rdObj["hostAddress"] = rd.hostAddress;
                rdObj["port"] = rd.port;
                rdObj["username"] = rd.username;
                desktopsArray.append(rdObj);
            }
            localConfigs["remoteDesktops"] = desktopsArray;
        }

        // 注意：工作日志(tasks)已改为自动同步，不再通过手动备份上传
    }

    // 读取QSettings中的用户配置（根据用户选择）
    if (m_includeSettingsCheck->isChecked()) {
        QSettings settings;
        QStringList allKeys = settings.allKeys();
        QJsonObject appSettings;
        for (const QString &key : allKeys) {
            if (key.startsWith("app/") || key.startsWith("ui/") || key.startsWith("shortcut/")) {
                appSettings[key] = settings.value(key).toString();
            }
        }
        if (!appSettings.isEmpty()) {
            localConfigs["appSettings"] = appSettings;
        }
    }

    QString configName = m_backupNameEdit->text().trimmed();

    // 配置名称是必需的
    if (configName.isEmpty()) {
        QMessageBox::warning(this, "提示", "请输入配置名称（如：台式机、笔记本、工作电脑等）");
        return;
    }

    m_progressBar->setVisible(true);
    m_progressBar->setValue(30);
    m_statusLabel->setText(QString("正在上传配置 [%1]...").arg(configName));
    m_statusLabel->setStyleSheet("color: #666;");

    QString token = UserManager::instance()->getToken();
    if (token.isEmpty()) {
        QMessageBox::warning(this, "提示", "请先登录");
        return;
    }

    QUrl url(QString(CLOUD_API_URL) + "/api/config/upload");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", ("Bearer " + token).toUtf8());

    QJsonObject body;
    body["configs"] = localConfigs;
    body["config_name"] = configName;

    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    QNetworkReply *reply = manager->post(request, QJsonDocument(body).toJson());

    connect(reply, &QNetworkReply::finished, this, [this, configName, reply]() {
        m_progressBar->setVisible(false);

        if (reply->error() != QNetworkReply::NoError) {
            m_statusLabel->setText("上传失败: " + reply->errorString());
            m_statusLabel->setStyleSheet("color: red;");
            reply->deleteLater();
            return;
        }

        QByteArray responseData = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(responseData);
        QJsonObject obj = doc.object();

        if (!obj["success"].toBool()) {
            m_statusLabel->setText("上传失败: " + obj["error"].toString());
            m_statusLabel->setStyleSheet("color: red;");
            reply->deleteLater();
            return;
        }

        m_statusLabel->setText(QString("上传成功！配置 [%1]").arg(configName));
        m_statusLabel->setStyleSheet("color: #28a745;");

        QMessageBox::information(this, "成功",
            QString("配置 [%1] 上传成功！\n（可用于不同设备/用途）").arg(configName));

        // 刷新配置列表
        loadVersions();

        reply->deleteLater();
    });
}
