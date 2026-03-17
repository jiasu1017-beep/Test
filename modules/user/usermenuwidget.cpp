#include "usermenuwidget.h"
#include "userapi.h"
#include "userlogindialog.h"
#include "changepassworddialog.h"
#include "../core/database.h"
#include <QApplication>
#include <QStyle>
#include <QIcon>
#include <QJsonArray>
#include <QJsonObject>
#include <QSet>
#include <QMap>
#include <QTimer>

UserMenuWidget::UserMenuWidget(QWidget *parent)
    : QObject(parent)
    , m_parent(parent)
    , m_db(nullptr)
    , m_userMenuBtn(nullptr)
    , m_userMenu(nullptr)
    , m_loginAction(nullptr)
    , m_registerAction(nullptr)
    , m_syncTimer(nullptr)
    , m_pendingSync(false)
    , m_lastSyncTime(QDateTime::fromSecsSinceEpoch(0))
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

void UserMenuWidget::setDatabase(Database *db)
{
    m_db = db;
    if (m_db) {
        // 创建防抖定时器（60秒延迟）
        m_syncTimer = new QTimer(this);
        m_syncTimer->setSingleShot(true);
        m_syncTimer->setInterval(60000); // 60秒防抖
        connect(m_syncTimer, &QTimer::timeout, this, &UserMenuWidget::onSyncTimerTimeout);

        // 连接任务变化信号，实现自动同步（带防抖）
        connect(m_db, &Database::tasksChanged, this, &UserMenuWidget::onTasksChanged);
    }
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

    // 自动同步工作日志到云端
    syncTasksToCloud();
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

void UserMenuWidget::syncTasksToCloud()
{
    // 连接信号
    connect(TaskSync::instance(), &TaskSync::tasksSynced, this, &UserMenuWidget::onTasksSynced);
    connect(TaskSync::instance(), &TaskSync::tasksUploadComplete, this, &UserMenuWidget::onTasksUploadComplete);
    connect(TaskSync::instance(), &TaskSync::syncFailed, this, &UserMenuWidget::onTasksSyncFailed);

    // 设置同步时间，登录时完整上传所有任务
    m_lastSyncTime = QDateTime::fromSecsSinceEpoch(0);

    // 先获取本地工作日志并上传，然后拉取云端工作日志
    if (m_db) {
        QList<Task> localTasks = m_db->getAllTasks();
        QJsonArray tasksArray;
        for (const Task &task : localTasks) {
            QJsonObject taskObj;
            taskObj["id"] = task.id;
            taskObj["title"] = task.title;
            taskObj["description"] = task.description;
            taskObj["categoryId"] = task.categoryId;
            taskObj["priority"] = task.priority;
            taskObj["status"] = task.status;
            taskObj["workDuration"] = task.workDuration;
            taskObj["completionTime"] = task.completionTime.toString(Qt::ISODate);
            taskObj["tags"] = QJsonArray::fromStringList(task.tags);
            tasksArray.append(taskObj);
        }
        TaskSync::instance()->uploadTasks(tasksArray);
    }
}

void UserMenuWidget::onTasksSynced(const QJsonArray& tasks)
{
    // 收到云端工作日志，合并到本地数据库
    if (!m_db) return;

    // 获取本地已有的任务
    QList<Task> localTasks = m_db->getAllTasks();
    QMap<QString, Task> localTaskMap;
    for (const Task &task : localTasks) {
        localTaskMap[task.id] = task;
    }

    // 处理云端任务：添加新任务或更新已有任务
    for (const QJsonValue &taskVal : tasks) {
        QJsonObject taskObj = taskVal.toObject();
        QString taskId = taskObj["id"].toString();

        Task task;
        task.id = taskId;
        task.title = taskObj["title"].toString();
        task.description = taskObj["description"].toString();
        task.categoryId = taskObj["categoryId"].toInt();
        task.priority = static_cast<TaskPriority>(taskObj["priority"].toInt());
        task.status = static_cast<TaskStatus>(taskObj["status"].toInt());
        task.workDuration = taskObj["workDuration"].toDouble();
        task.completionTime = QDateTime::fromString(taskObj["completionTime"].toString(), Qt::ISODate);

        // 读取云端任务的更新时间
        task.updatedAt = QDateTime::fromString(taskObj["updatedAt"].toString(), Qt::ISODate);
        if (!task.updatedAt.isValid()) {
            task.updatedAt = QDateTime::currentDateTime();
        }

        QJsonArray tagsArray = taskObj["tags"].toArray();
        for (const QJsonValue &tagVal : tagsArray) {
            task.tags.append(tagVal.toString());
        }

        // 如果本地不存在，直接添加
        if (!localTaskMap.contains(taskId)) {
            m_db->addTask(task);
        } else {
            // 如果本地存在，比较更新时间，保留最新的
            Task localTask = localTaskMap[taskId];
            // 这里简单处理：直接更新本地任务为云端版本
            // 可以根据 updatedAt 字段做更精细的比较
            m_db->updateTask(task);
        }
    }

    // 更新同步时间
    m_lastSyncTime = QDateTime::currentDateTime();

    qDebug() << "Tasks synced from cloud, count:" << tasks.size();
}

void UserMenuWidget::onTasksUploadComplete()
{
    // 上传完成后，下载云端最新数据
    TaskSync::instance()->downloadTasks();
}

void UserMenuWidget::onTasksSyncFailed(const QString& error)
{
    qDebug() << "Tasks sync failed:" << error;
}

void UserMenuWidget::onTasksChanged()
{
    // 当本地任务发生变化时，触发防抖同步
    if (UserManager::instance()->isLoggedIn() && m_db) {
        // 标记有待同步的数据
        m_pendingSync = true;

        // 如果定时器没有运行，启动防抖定时器
        if (!m_syncTimer->isActive()) {
            m_syncTimer->start();
            qDebug() << "Tasks changed, waiting 2s before sync...";
        } else {
            // 定时器已在运行，重置计时
            m_syncTimer->stop();
            m_syncTimer->start();
            qDebug() << "Tasks changed again, reset sync timer...";
        }
    }
}

void UserMenuWidget::onSyncTimerTimeout()
{
    if (m_pendingSync && UserManager::instance()->isLoggedIn() && m_db) {
        m_pendingSync = false;
        qDebug() << "Sync timer timeout, uploading changed tasks to cloud...";

        // 连接信号（如果尚未连接）
        static bool connected = false;
        if (!connected) {
            connected = true;
            connect(TaskSync::instance(), &TaskSync::tasksSynced, this, &UserMenuWidget::onTasksSynced);
            connect(TaskSync::instance(), &TaskSync::tasksUploadComplete, this, &UserMenuWidget::onTasksUploadComplete);
            connect(TaskSync::instance(), &TaskSync::syncFailed, this, &UserMenuWidget::onTasksSyncFailed);
        }

        // 增量上传：只上传自上次同步以来变化的任务
        QList<Task> localTasks = m_db->getAllTasks();
        QJsonArray tasksArray;
        for (const Task &task : localTasks) {
            // 检查任务是否在上次同步后有更新
            if (task.updatedAt.isValid() && task.updatedAt > m_lastSyncTime) {
                QJsonObject taskObj;
                taskObj["id"] = task.id;
                taskObj["title"] = task.title;
                taskObj["description"] = task.description;
                taskObj["categoryId"] = task.categoryId;
                taskObj["priority"] = task.priority;
                taskObj["status"] = task.status;
                taskObj["workDuration"] = task.workDuration;
                taskObj["completionTime"] = task.completionTime.toString(Qt::ISODate);
                taskObj["tags"] = QJsonArray::fromStringList(task.tags);
                tasksArray.append(taskObj);
                qDebug() << "Uploading changed task:" << task.title;
            }
        }

        if (tasksArray.size() > 0) {
            TaskSync::instance()->uploadTasks(tasksArray);
            // 更新同步时间（等上传完成后再更新，这里先更新）
            m_lastSyncTime = QDateTime::currentDateTime();
        } else {
            qDebug() << "No changed tasks to upload";
        }
    }
}
