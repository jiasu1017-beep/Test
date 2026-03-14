#include "userapi.h"
#include <QCryptographicHash>
#include <QDateTime>
#include <QNetworkCookie>
#include <QUrlQuery>
#include <QSettings>

ApiClient* ApiClient::instance() {
    static ApiClient client;
    return &client;
}

ApiClient::ApiClient() {
    m_manager = new QNetworkAccessManager(this);
    m_baseUrl = CLOUD_API_URL;
    m_authToken = "";
}

ApiClient::~ApiClient() {
}

void ApiClient::setBaseUrl(const QString& url) {
    m_baseUrl = url;
}

void ApiClient::setAuthToken(const QString& token) {
    m_authToken = token;
    qDebug() << "Auth token set:" << token.left(20) << "...";
}

void ApiClient::get(const QString& endpoint, const QJsonObject& params) {
    QString url = m_baseUrl + endpoint;
    if (!params.isEmpty()) {
        QUrlQuery query;
        for (auto it = params.begin(); it != params.end(); ++it) {
            query.addQueryItem(it.key(), it.value().toString());
        }
        url += "?" + query.toString();
    }
    
    QNetworkRequest request;
    request.setUrl(QUrl(url));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    if (!m_authToken.isEmpty()) {
        request.setRawHeader("Authorization", ("Bearer " + m_authToken).toUtf8());
    }
    
    QNetworkReply* reply = m_manager->get(request);
    reply->setProperty("endpoint", endpoint);
    connect(reply, &QNetworkReply::finished, this, &ApiClient::onReplyFinished);
}

void ApiClient::post(const QString& endpoint, const QJsonObject& data) {
    post(endpoint, QJsonDocument(data).toJson(QJsonDocument::Compact));
}

void ApiClient::post(const QString& endpoint, const QString& jsonStr) {
    QNetworkRequest request;
    request.setUrl(QUrl(m_baseUrl + endpoint));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    if (!m_authToken.isEmpty()) {
        request.setRawHeader("Authorization", ("Bearer " + m_authToken).toUtf8());
    }
    
    QNetworkReply* reply = m_manager->post(request, jsonStr.toUtf8());
    reply->setProperty("endpoint", endpoint);
    connect(reply, &QNetworkReply::finished, this, &ApiClient::onReplyFinished);
}

void ApiClient::onReplyFinished() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;

    QString endpoint = reply->property("endpoint").toString();
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    qDebug() << "API Response:" << endpoint << "status:" << statusCode;
    QByteArray data = reply->readAll();
    
    if (statusCode >= 200 && statusCode < 300) {
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (!doc.isNull()) {
            emit requestSuccess(endpoint, doc);
        } else {
            emit requestFailed(endpoint, statusCode, "Invalid JSON response");
        }
    } else {
        QString error = data;
        if (error.isEmpty()) {
            error = "HTTP " + QString::number(statusCode);
        }
        emit requestFailed(endpoint, statusCode, error);
    }
    
    reply->deleteLater();
}

UserManager* UserManager::instance() {
    static UserManager manager;
    return &manager;
}

UserManager::UserManager() : QObject(), m_isLoggedIn(false) {
    ApiClient* client = ApiClient::instance();
    connect(client, &ApiClient::requestSuccess, this, &UserManager::onLoginResponse);
    connect(client, &ApiClient::requestSuccess, this, &UserManager::onRegisterResponse);
    connect(client, &ApiClient::requestSuccess, this, &UserManager::onProfileResponse);
    connect(client, &ApiClient::requestSuccess, this, &UserManager::onEmailCheckResponse);
    connect(client, &ApiClient::requestSuccess, this, &UserManager::onUpdateProfileResponse);
    connect(client, &ApiClient::requestSuccess, this, &UserManager::onChangePasswordResponse);
    connect(client, &ApiClient::requestFailed, this, &UserManager::onRequestFailed);
}

void UserManager::login(const QString& email, const QString& password) {
    qDebug() << "[登录请求]" << QTime::currentTime().toString();
    qDebug() << "  - Email:" << email;
    
    QJsonObject data;
    data["email"] = email;
    data["password"] = password;
    
    ApiClient::instance()->post("/api/auth/login", data);
}

void UserManager::loginByUsername(const QString& username, const QString& password) {
    qDebug() << "[登录请求 - 用户名]" << QTime::currentTime().toString();
    qDebug() << "  - Username:" << username;
    
    QJsonObject data;
    data["username"] = username;
    data["password"] = password;
    
    ApiClient::instance()->post("/api/auth/login", data);
}

void UserManager::loginAuto(const QString& identifier, const QString& password) {
    qDebug() << "[登录请求 - 自动判断]" << QTime::currentTime().toString();
    qDebug() << "  - Identifier:" << identifier;
    
    QRegularExpression emailRe("^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\\.[A-Z|a-z]{2,}$");
    bool isEmail = emailRe.match(identifier).hasMatch();
    
    if (isEmail) {
        login(identifier, password);
    } else {
        loginByUsername(identifier, password);
    }
}

void UserManager::registerUser(const QString& username, const QString& email, const QString& password) {
    qDebug() << "[注册请求]" << QTime::currentTime().toString();
    qDebug() << "  - Username:" << username;
    qDebug() << "  - Email:" << email;
    
    QJsonObject data;
    data["username"] = username;
    data["email"] = email;
    data["password"] = password;
    
    ApiClient::instance()->post("/api/auth/register", data);
}

void UserManager::checkEmailExists(const QString& email) {
    qDebug() << "Checking email:" << email;
    ApiClient::instance()->get("/api/auth/check-email?email=" + QUrl::toPercentEncoding(email));
}

void UserManager::logout() {
    m_token.clear();
    m_currentUser = {};
    m_isLoggedIn = false;
    ApiClient::instance()->setAuthToken("");
    
    QSettings settings;
    settings.remove("cloud_user_token");
    settings.remove("cloud_user_email");
    
    emit logoutComplete();
}

void UserManager::fetchProfile() {
    if (m_token.isEmpty()) {
        emit loginFailed("Not logged in");
        return;
    }
    
    ApiClient::instance()->get("/api/auth/profile");
}

void UserManager::autoLogin() {
    QSettings settings;
    QString token = settings.value("cloud_user_token").toString();
    QString email = settings.value("cloud_user_email").toString();
    
    if (!token.isEmpty() && !email.isEmpty()) {
        m_token = token;
        ApiClient::instance()->setAuthToken(token);
        fetchProfile();
    }
}

void UserManager::updateProfile(const QString& username, const QString& avatar) {
    if (!m_isLoggedIn) {
        emit loginFailed("Not logged in");
        return;
    }
    
    QJsonObject data;
    if (!username.isEmpty()) {
        data["username"] = username;
    }
    if (!avatar.isEmpty()) {
        data["avatar"] = avatar;
    }
    
    ApiClient::instance()->post("/api/user/update-profile", data);
}

void UserManager::changePassword(const QString& oldPassword, const QString& newPassword) {
    if (!m_isLoggedIn) {
        emit loginFailed("Not logged in");
        return;
    }
    
    QJsonObject data;
    data["old_password"] = oldPassword;
    data["new_password"] = newPassword;
    
    ApiClient::instance()->post("/api/user/change-password", data);
}

void UserManager::onLoginResponse(const QString& endpoint, const QJsonDocument& response) {
    if (endpoint != "/api/auth/login") return;
    
    QJsonObject obj = response.object();
    if (obj["success"].toBool()) {
        m_token = obj["token"].toString();
        ApiClient::instance()->setAuthToken(m_token);
        
        QJsonObject userObj = obj["user"].toObject();
        m_currentUser.id = userObj["id"].toInt();
        m_currentUser.email = userObj["email"].toString();
        m_currentUser.username = userObj["username"].toString();
        m_currentUser.vipLevel = userObj["vipLevel"].toInt();
        m_isLoggedIn = true;
        
        QSettings settings;
        settings.setValue("cloud_user_token", m_token);
        settings.setValue("cloud_user_email", m_currentUser.email);
        
        emit loginSuccess(m_currentUser);
        qDebug() << "Login successful:" << m_currentUser.email;
    } else {
        QString error = obj["error"].toString();
        emit loginFailed(error);
        qDebug() << "Login failed:" << error;
    }
}

void UserManager::onRegisterResponse(const QString& endpoint, const QJsonDocument& response) {
    if (endpoint != "/api/auth/register") return;
    
    QJsonObject obj = response.object();
    if (obj["success"].toBool()) {
        emit registerSuccess();
        qDebug() << "Registration successful, userId:" << obj["userId"].toInt();
    } else {
        QString error = obj["error"].toString();
        emit registerFailed(error);
        qDebug() << "Registration failed:" << error;
    }
}

void UserManager::onEmailCheckResponse(const QString& endpoint, const QJsonDocument& response) {
    if (!endpoint.contains("/api/auth/check-email")) return;
    
    QJsonObject obj = response.object();
    bool exists = obj["exists"].toBool();
    qDebug() << "Email exists:" << exists;
    emit emailCheckResult(exists);
}

void UserManager::onProfileResponse(const QString& endpoint, const QJsonDocument& response) {
    if (endpoint != "/api/auth/profile") return;
    
    QJsonObject obj = response.object();
    if (obj.contains("user")) {
        QJsonObject userObj = obj["user"].toObject();
        m_currentUser.id = userObj["id"].toInt();
        m_currentUser.email = userObj["email"].toString();
        m_currentUser.username = userObj["username"].toString();
        m_currentUser.vipLevel = userObj["vip_level"].toInt();
        m_currentUser.createdAt = userObj["created_at"].toString();
        m_currentUser.lastLogin = userObj["last_login"].toString();
        m_isLoggedIn = true;
        
        emit profileLoaded(m_currentUser);
        qDebug() << "Profile loaded:" << m_currentUser.email;
    }
}

void UserManager::onUpdateProfileResponse(const QString& endpoint, const QJsonDocument& response) {
    if (endpoint != "/api/user/update-profile") return;
    
    QJsonObject obj = response.object();
    if (obj["success"].toBool()) {
        emit profileUpdated();
        qDebug() << "Profile updated successfully";
    } else {
        QString error = obj["error"].toString();
        emit loginFailed(error);
        qDebug() << "Profile update failed:" << error;
    }
}

void UserManager::onChangePasswordResponse(const QString& endpoint, const QJsonDocument& response) {
    if (endpoint != "/api/user/change-password") return;
    
    QJsonObject obj = response.object();
    if (obj["success"].toBool()) {
        emit passwordChanged();
        qDebug() << "Password changed successfully";
    } else {
        QString error = obj["error"].toString();
        emit loginFailed(error);
        qDebug() << "Password change failed:" << error;
    }
}

void UserManager::onRequestFailed(const QString& endpoint, int errorCode, const QString& error) {
    qDebug() << "Request failed:" << endpoint << "error:" << errorCode << error;
    
    if (endpoint == "/api/auth/login") {
        emit loginFailed(error);
    } else if (endpoint == "/api/auth/register") {
        emit registerFailed(error);
    } else if (endpoint == "/api/user/update-profile") {
        emit loginFailed(error);
    } else if (endpoint == "/api/user/change-password") {
        emit loginFailed(error);
    }
}

ConfigSync* ConfigSync::instance() {
    static ConfigSync sync;
    return &sync;
}

ConfigSync::ConfigSync() : QObject(), m_isSyncing(false) {
    ApiClient* client = ApiClient::instance();
    connect(client, &ApiClient::requestSuccess, this, &ConfigSync::onConfigLoaded);
    connect(client, &ApiClient::requestSuccess, this, &ConfigSync::onConfigSaved);
    connect(client, &ApiClient::requestFailed, this, &ConfigSync::onRequestFailed);
}

void ConfigSync::fetchConfig() {
    if (!UserManager::instance()->isLoggedIn()) {
        emit syncFailed("Not logged in");
        return;
    }
    
    if (m_isSyncing) return;
    m_isSyncing = true;
    
    ApiClient::instance()->get("/api/config/get");
}

void ConfigSync::saveConfig(const QString& key, const QJsonObject& config) {
    QJsonObject configs;
    configs[key] = config;
    saveAllConfig(configs);
}

void ConfigSync::saveAllConfig(const QJsonObject& configs) {
    if (!UserManager::instance()->isLoggedIn()) {
        emit syncFailed("Not logged in");
        return;
    }
    
    if (m_isSyncing) return;
    m_isSyncing = true;
    
    QJsonObject data;
    data["configs"] = configs;
    
    ApiClient::instance()->post("/api/config/save", data);
}

void ConfigSync::onConfigLoaded(const QString& endpoint, const QJsonDocument& response) {
    if (endpoint != "/api/config/get") return;
    
    m_isSyncing = false;
    
    QJsonObject obj = response.object();
    if (obj["success"].toBool()) {
        QJsonObject configs = obj["configs"].toObject();
        emit configLoaded(configs);
        qDebug() << "Config loaded successfully";
    } else {
        QString error = obj["error"].toString();
        emit syncFailed(error);
        qDebug() << "Config load failed:" << error;
    }
}

void ConfigSync::onConfigSaved(const QString& endpoint, const QJsonDocument& response) {
    if (endpoint != "/api/config/save") return;
    
    m_isSyncing = false;
    
    QJsonObject obj = response.object();
    if (obj["success"].toBool()) {
        emit configSaved();
        qDebug() << "Config saved successfully";
    } else {
        QString error = obj["error"].toString();
        emit syncFailed(error);
        qDebug() << "Config save failed:" << error;
    }
}

void ConfigSync::onRequestFailed(const QString& endpoint, int errorCode, const QString& error) {
    if (endpoint == "/api/config/get" || endpoint == "/api/config/save") {
        m_isSyncing = false;
        emit syncFailed(error);
        qDebug() << "Config sync failed:" << error;
    }
}
