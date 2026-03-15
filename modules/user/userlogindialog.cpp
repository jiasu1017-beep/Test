#include "userlogindialog.h"
#include <QFormLayout>
#include <QGroupBox>
#include <QGraphicsDropShadowEffect>
#include <QStyle>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QSettings>

UserLoginDialog::UserLoginDialog(QWidget *parent)
    : QDialog(parent)
    , m_isValidating(false)
{
    setupUI();
    setWindowTitle("PonyWork 用户登录");
    setModal(true);
    resize(450, 550);
    
    setStyleSheet(R"(
        UserLoginDialog {
            background-color: #f5f6fa;
        }
        QLabel[cssClass="titleLabel"] {
            font-size: 24px;
            font-weight: bold;
            color: #2d3436;
            padding: 15px;
        }
        QLabel[cssClass="statusLabel"] {
            padding: 8px;
            border-radius: 4px;
            font-size: 13px;
        }
        QLabel[cssClass="statusLabel"][isError="true"] {
            color: #d63031;
            background-color: #ffeaa7;
        }
        QLabel[cssClass="statusLabel"][isError="false"] {
            color: #00b894;
            background-color: #55efc4;
        }
        QLineEdit {
            padding: 10px;
            border: 2px solid #dfe6e9;
            border-radius: 6px;
            font-size: 14px;
            background-color: white;
        }
        QLineEdit:focus {
            border-color: #0984e3;
        }
        QLineEdit:disabled {
            background-color: #f0f0f0;
        }
        QPushButton[cssClass="primaryBtn"] {
            background-color: #0984e3;
            color: white;
            font-size: 16px;
            font-weight: bold;
            padding: 12px;
            border: none;
            border-radius: 6px;
        }
        QPushButton[cssClass="primaryBtn"]:hover {
            background-color: #74b9ff;
        }
        QPushButton[cssClass="primaryBtn"]:pressed {
            background-color: #0984e3;
        }
        QPushButton[cssClass="primaryBtn"]:disabled {
            background-color: #b2bec3;
        }
        QPushButton[cssClass="secondaryBtn"] {
            background-color: #636e72;
            color: white;
            font-size: 14px;
            padding: 10px;
            border: none;
            border-radius: 6px;
        }
        QPushButton[cssClass="secondaryBtn"]:hover {
            background-color: #b2bec3;
        }
        QPushButton[cssClass="linkBtn"] {
            background-color: transparent;
            color: #0984e3;
            font-size: 13px;
            border: none;
            text-decoration: underline;
        }
        QPushButton[cssClass="linkBtn"]:hover {
            color: #74b9ff;
        }
        QCheckBox {
            font-size: 13px;
            color: #2d3436;
            spacing: 8px;
        }
        QGroupBox {
            font-weight: bold;
            border: 2px solid #dfe6e9;
            border-radius: 8px;
            margin-top: 12px;
            padding-top: 10px;
            font-size: 14px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 15px;
            padding: 0 8px;
            color: #2d3436;
        }
    )");

    connect(UserManager::instance(), &UserManager::loginSuccess, this, &UserLoginDialog::onLoginSuccess);
    connect(UserManager::instance(), &UserManager::loginFailed, this, &UserLoginDialog::onLoginFailed);
    connect(UserManager::instance(), &UserManager::registerSuccess, this, &UserLoginDialog::onRegisterSuccess);
    connect(UserManager::instance(), &UserManager::registerFailed, this, &UserLoginDialog::onRegisterFailed);
    connect(UserManager::instance(), &UserManager::emailCheckResult, this, &UserLoginDialog::onEmailCheckResult);
}

UserLoginDialog::~UserLoginDialog() {
}

void UserLoginDialog::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(30, 30, 30, 30);

    QLabel *titleLabel = new QLabel("☁️ PonyWork 云端账户", this);
    titleLabel->setProperty("cssClass", "titleLabel");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    mainLayout->addSpacing(10);

    m_stackedWidget = new QStackedWidget(this);
    
    setupLoginPage();
    setupRegisterPage();
    
    m_stackedWidget->addWidget(m_loginPage);
    m_stackedWidget->addWidget(m_registerPage);
    
    mainLayout->addWidget(m_stackedWidget);

    m_cancelBtn = new QPushButton("关闭", this);
    m_cancelBtn->setProperty("cssClass", "secondaryBtn");
    m_cancelBtn->setFixedHeight(45);
    connect(m_cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    
    mainLayout->addWidget(m_cancelBtn);

    m_stackedWidget->setCurrentIndex(0);
}

void UserLoginDialog::setupLoginPage() {
    m_loginPage = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(m_loginPage);
    layout->setSpacing(12);
    layout->setContentsMargins(0, 0, 0, 0);

    QLabel *identifierLabel = new QLabel("📧 邮箱 / 👤 用户名", this);
    identifierLabel->setStyleSheet("font-weight: bold; font-size: 14px;");
    layout->addWidget(identifierLabel);
    
    m_loginIdentifierEdit = new QLineEdit(this);
    m_loginIdentifierEdit->setPlaceholderText("请输入邮箱或用户名");
    m_loginIdentifierEdit->setFixedHeight(45);
    m_loginIdentifierEdit->setClearButtonEnabled(true);
    connect(m_loginIdentifierEdit, &QLineEdit::textChanged, this, &UserLoginDialog::onIdentifierTextChanged);
    layout->addWidget(m_loginIdentifierEdit);

    QLabel *passwordLabel = new QLabel("🔒 密码", this);
    passwordLabel->setStyleSheet("font-weight: bold; font-size: 14px;");
    layout->addWidget(passwordLabel);
    
    m_loginPasswordEdit = new QLineEdit(this);
    m_loginPasswordEdit->setPlaceholderText("请输入密码");
    m_loginPasswordEdit->setEchoMode(QLineEdit::Password);
    m_loginPasswordEdit->setFixedHeight(45);
    m_loginPasswordEdit->setClearButtonEnabled(true);
    connect(m_loginPasswordEdit, &QLineEdit::textChanged, this, &UserLoginDialog::onLoginPasswordTextChanged);
    layout->addWidget(m_loginPasswordEdit);

    m_rememberMeCheck = new QCheckBox("✓ 记住我（7 天内自动登录）", this);
    m_rememberMeCheck->setChecked(true);
    layout->addWidget(m_rememberMeCheck);

    m_loginStatusLabel = new QLabel(this);
    m_loginStatusLabel->setProperty("cssClass", "statusLabel");
    m_loginStatusLabel->setAlignment(Qt::AlignCenter);
    m_loginStatusLabel->setWordWrap(true);
    layout->addWidget(m_loginStatusLabel);

    layout->addSpacing(10);

    m_loginBtn = new QPushButton("🔑 立即登录", this);
    m_loginBtn->setProperty("cssClass", "primaryBtn");
    m_loginBtn->setFixedHeight(50);
    m_loginBtn->setCursor(Qt::PointingHandCursor);
    connect(m_loginBtn, &QPushButton::clicked, this, &UserLoginDialog::onLoginClicked);
    layout->addWidget(m_loginBtn);

    QHBoxLayout *switchLayout = new QHBoxLayout();
    switchLayout->setAlignment(Qt::AlignCenter);
    QLabel *hintLabel = new QLabel("还没有账户？", this);
    hintLabel->setStyleSheet("color: #636e72; font-size: 13px;");
    
    QPushButton *switchBtn = new QPushButton("立即注册 →", this);
    switchBtn->setProperty("cssClass", "linkBtn");
    switchBtn->setCursor(Qt::PointingHandCursor);
    connect(switchBtn, &QPushButton::clicked, this, &UserLoginDialog::switchToRegister);
    
    switchLayout->addWidget(hintLabel);
    switchLayout->addWidget(switchBtn);
    layout->addLayout(switchLayout);

    m_loginPage->setLayout(layout);
    
    loadSavedCredentials();
}

void UserLoginDialog::setupRegisterPage() {
    m_registerPage = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(m_registerPage);
    layout->setSpacing(12);
    layout->setContentsMargins(0, 0, 0, 0);

    QLabel *usernameLabel = new QLabel("👤 用户名", this);
    usernameLabel->setStyleSheet("font-weight: bold; font-size: 14px;");
    layout->addWidget(usernameLabel);
    
    m_registerUsernameEdit = new QLineEdit(this);
    m_registerUsernameEdit->setPlaceholderText("3-20 位字母、数字或下划线");
    m_registerUsernameEdit->setFixedHeight(45);
    m_registerUsernameEdit->setClearButtonEnabled(true);
    connect(m_registerUsernameEdit, &QLineEdit::textChanged, this, &UserLoginDialog::onUsernameTextChanged);
    layout->addWidget(m_registerUsernameEdit);

    QLabel *emailLabel = new QLabel("📧 邮箱地址", this);
    emailLabel->setStyleSheet("font-weight: bold; font-size: 14px;");
    layout->addWidget(emailLabel);
    
    m_registerEmailEdit = new QLineEdit(this);
    m_registerEmailEdit->setPlaceholderText("用于登录和找回密码");
    m_registerEmailEdit->setFixedHeight(45);
    m_registerEmailEdit->setClearButtonEnabled(true);
    connect(m_registerEmailEdit, &QLineEdit::textChanged, this, [=](const QString& text) {
        if (!text.isEmpty() && !text.contains("@")) {
            m_registerEmailEdit->setStyleSheet("border: 2px solid #fdcb6e;");
        } else {
            m_registerEmailEdit->setStyleSheet("");
        }
    });
    layout->addWidget(m_registerEmailEdit);

    QLabel *passwordLabel = new QLabel("🔒 设置密码", this);
    passwordLabel->setStyleSheet("font-weight: bold; font-size: 14px;");
    layout->addWidget(passwordLabel);
    
    m_registerPasswordEdit = new QLineEdit(this);
    m_registerPasswordEdit->setPlaceholderText("6-20 位字符");
    m_registerPasswordEdit->setEchoMode(QLineEdit::Password);
    m_registerPasswordEdit->setFixedHeight(45);
    m_registerPasswordEdit->setClearButtonEnabled(true);
    connect(m_registerPasswordEdit, &QLineEdit::textChanged, this, &UserLoginDialog::onRegisterPasswordTextChanged);
    layout->addWidget(m_registerPasswordEdit);

    QLabel *confirmLabel = new QLabel("🔒 确认密码", this);
    confirmLabel->setStyleSheet("font-weight: bold; font-size: 14px;");
    layout->addWidget(confirmLabel);
    
    m_registerConfirmPasswordEdit = new QLineEdit(this);
    m_registerConfirmPasswordEdit->setPlaceholderText("再次输入密码");
    m_registerConfirmPasswordEdit->setEchoMode(QLineEdit::Password);
    m_registerConfirmPasswordEdit->setFixedHeight(45);
    m_registerConfirmPasswordEdit->setClearButtonEnabled(true);
    connect(m_registerConfirmPasswordEdit, &QLineEdit::textChanged, this, &UserLoginDialog::onConfirmPasswordTextChanged);
    layout->addWidget(m_registerConfirmPasswordEdit);

    m_registerStatusLabel = new QLabel(this);
    m_registerStatusLabel->setProperty("cssClass", "statusLabel");
    m_registerStatusLabel->setAlignment(Qt::AlignCenter);
    m_registerStatusLabel->setWordWrap(true);
    layout->addWidget(m_registerStatusLabel);

    layout->addSpacing(10);

    m_registerBtn = new QPushButton("🎉 免费注册", this);
    m_registerBtn->setProperty("cssClass", "primaryBtn");
    m_registerBtn->setFixedHeight(50);
    m_registerBtn->setCursor(Qt::PointingHandCursor);
    connect(m_registerBtn, &QPushButton::clicked, this, &UserLoginDialog::onRegisterClicked);
    layout->addWidget(m_registerBtn);

    QHBoxLayout *switchLayout = new QHBoxLayout();
    switchLayout->setAlignment(Qt::AlignCenter);
    QLabel *hintLabel = new QLabel("已有账户？", this);
    hintLabel->setStyleSheet("color: #636e72; font-size: 13px;");
    
    QPushButton *switchBtn = new QPushButton("立即登录 →", this);
    switchBtn->setProperty("cssClass", "linkBtn");
    switchBtn->setCursor(Qt::PointingHandCursor);
    connect(switchBtn, &QPushButton::clicked, this, &UserLoginDialog::switchToLogin);
    
    switchLayout->addWidget(hintLabel);
    switchLayout->addWidget(switchBtn);
    layout->addLayout(switchLayout);

    m_registerPage->setLayout(layout);
}

void UserLoginDialog::onLoginClicked() {
    QString identifier = m_loginIdentifierEdit->text().trimmed();
    QString password = m_loginPasswordEdit->text().trimmed();  // 修复：去除密码首尾空格
    
    qDebug() << "[登录调试] 标识符:" << identifier << "密码长度:" << password.length() << "密码内容:" << password;
    
    if (identifier.isEmpty()) {
        showStatusMessage("请输入邮箱或用户名", true);
        return;
    }
    
    if (password.isEmpty()) {
        showStatusMessage("请输入密码", true);
        return;
    }
    
    setUIEnabled(false);
    showStatusMessage("正在登录...", false);
    
    // 自动判断是邮箱还是用户名
    QRegularExpression emailRe("^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\\.[A-Z|a-z]{2,}$");
    bool isEmail = emailRe.match(identifier).hasMatch();
    
    qDebug() << "[登录] 标识符：" << identifier << "类型：" << (isEmail ? "邮箱" : "用户名");
    
    UserManager::instance()->loginAuto(identifier, password);
}

void UserLoginDialog::onRegisterClicked() {
    QString username = m_registerUsernameEdit->text().trimmed();
    QString email = m_registerEmailEdit->text().trimmed();
    QString password = m_registerPasswordEdit->text();
    QString confirmPassword = m_registerConfirmPasswordEdit->text();

    if (!validateUsername(username)) {
        showStatusMessage("用户名格式不正确（3-20 位字母、数字或下划线）", true);
        return;
    }

    if (!validateEmail(email)) {
        showStatusMessage("请输入有效的邮箱地址", true);
        return;
    }

    if (!validatePassword(password)) {
        showStatusMessage("密码长度至少 6 位", true);
        return;
    }

    if (password != confirmPassword) {
        showStatusMessage("两次输入的密码不一致", true);
        return;
    }

    setUIEnabled(false);
    showStatusMessage("正在检查邮箱...", false);
    UserManager::instance()->checkEmailExists(email);
}

void UserLoginDialog::onEmailCheckResult(bool exists) {
    if (exists) {
        setUIEnabled(true);
        showStatusMessage("该邮箱已被注册，请直接登录", true);
        QMessageBox::information(this, "提示", "该邮箱已被注册，请直接登录");
        switchToLogin();
        return;
    }

    QString username = m_registerUsernameEdit->text().trimmed();
    QString email = m_registerEmailEdit->text().trimmed();
    QString password = m_registerPasswordEdit->text();
    
    showStatusMessage("正在注册...", false);
    qDebug() << "[UserLoginDialog] 开始注册:" << username << email;
    UserManager::instance()->registerUser(username, email, password);
}

void UserLoginDialog::onLoginSuccess(const UserInfo& user) {
    setUIEnabled(true);
    showStatusMessage("登录成功！", false);
    
    saveCredentials();
    
    QMessageBox::information(this, "登录成功",
        QString("欢迎回来，%1！\n\n用户 ID: %2\nVIP 等级：%3")
        .arg(user.username.isEmpty() ? user.email : user.username)
        .arg(user.id)
        .arg(user.vipLevel));

    accept();
}

void UserLoginDialog::onLoginFailed(const QString& error) {
    setUIEnabled(true);
    showStatusMessage(error, true);
}

void UserLoginDialog::onRegisterSuccess() {
    setUIEnabled(true);
    showStatusMessage("注册成功！请登录", false);
    QMessageBox::information(this, "注册成功", "恭喜您注册成功！请登录您的账户。");
    switchToLogin();
}

void UserLoginDialog::onRegisterFailed(const QString& error) {
    setUIEnabled(true);
    showStatusMessage(error, true);
}

void UserLoginDialog::switchToRegister() {
    m_stackedWidget->setCurrentIndex(1);
    showStatusMessage("");
}

void UserLoginDialog::switchToLogin() {
    m_stackedWidget->setCurrentIndex(0);
    showStatusMessage("");
}

void UserLoginDialog::onIdentifierTextChanged(const QString& text) {
    if (!m_loginIdentifierEdit) return;
    
    if (text.isEmpty()) {
        m_loginIdentifierEdit->setStyleSheet("");
        return;
    }
    
    QRegularExpression emailRe("^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\\.[A-Z|a-z]{2,}$");
    QRegularExpression usernameRe("^[a-zA-Z0-9_]{3,20}$");
    
    bool isValidEmail = emailRe.match(text).hasMatch();
    bool isValidUsername = usernameRe.match(text).hasMatch();
    
    if (isValidEmail || isValidUsername) {
        m_loginIdentifierEdit->setStyleSheet("");
    } else {
        m_loginIdentifierEdit->setStyleSheet("border: 2px solid #fdcb6e;");
    }
}

void UserLoginDialog::onLoginPasswordTextChanged(const QString& text) {
    if (!m_loginPasswordEdit) {
        return;
    }
    
    if (!text.isEmpty() && text.length() < 6) {
        m_loginPasswordEdit->setStyleSheet("border: 2px solid #fdcb6e;");
    } else {
        m_loginPasswordEdit->setStyleSheet("");
    }
}

void UserLoginDialog::onRegisterPasswordTextChanged(const QString& text) {
    if (!m_registerPasswordEdit) {
        return;
    }
    
    if (!text.isEmpty() && text.length() < 6) {
        m_registerPasswordEdit->setStyleSheet("border: 2px solid #fdcb6e;");
    } else {
        m_registerPasswordEdit->setStyleSheet("");
    }
}

void UserLoginDialog::onUsernameTextChanged(const QString& text) {
    if (!m_registerUsernameEdit) return;
    
    QRegularExpression re("^[a-zA-Z0-9_]{3,20}$");
    if (!text.isEmpty() && !re.match(text).hasMatch()) {
        m_registerUsernameEdit->setStyleSheet("border: 2px solid #fdcb6e;");
    } else {
        m_registerUsernameEdit->setStyleSheet("");
    }
}

void UserLoginDialog::onConfirmPasswordTextChanged(const QString& text) {
    if (!m_registerConfirmPasswordEdit || !m_registerPasswordEdit) return;
    
    QString password = m_registerPasswordEdit->text();
    
    if (text.isEmpty()) {
        m_registerConfirmPasswordEdit->setStyleSheet("");
        return;
    }
    
    if (text != password && !password.isEmpty()) {
        m_registerConfirmPasswordEdit->setStyleSheet("border: 2px solid #fdcb6e;");
    } else {
        m_registerConfirmPasswordEdit->setStyleSheet("");
    }
}

void UserLoginDialog::setUIEnabled(bool enabled) {
    if (m_loginIdentifierEdit) m_loginIdentifierEdit->setEnabled(enabled);
    if (m_loginPasswordEdit) m_loginPasswordEdit->setEnabled(enabled);
    if (m_loginBtn) m_loginBtn->setEnabled(enabled);
    
    if (m_registerUsernameEdit) m_registerUsernameEdit->setEnabled(enabled);
    if (m_registerEmailEdit) m_registerEmailEdit->setEnabled(enabled);
    if (m_registerPasswordEdit) m_registerPasswordEdit->setEnabled(enabled);
    if (m_registerConfirmPasswordEdit) m_registerConfirmPasswordEdit->setEnabled(enabled);
    if (m_registerBtn) m_registerBtn->setEnabled(enabled);
    
    if (m_cancelBtn) m_cancelBtn->setEnabled(enabled);
}

bool UserLoginDialog::validateEmail(const QString& email) {
    QRegularExpression re("^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\\.[A-Z|a-z]{2,}$");
    return re.match(email).hasMatch();
}

bool UserLoginDialog::validatePassword(const QString& password) {
    return password.length() >= 6;
}

bool UserLoginDialog::validateUsername(const QString& username) {
    QRegularExpression re("^[a-zA-Z0-9_]{3,20}$");
    return re.match(username).hasMatch();
}

void UserLoginDialog::showStatusMessage(const QString& message, bool isError) {
    QLabel *label = nullptr;
    
    if (m_stackedWidget->currentIndex() == 0) {
        label = m_loginStatusLabel;
    } else {
        label = m_registerStatusLabel;
    }
    
    if (label) {
        label->setText(message);
        label->setProperty("isError", isError);
        label->style()->unpolish(label);
        label->style()->polish(label);
    }
}

void UserLoginDialog::loadSavedCredentials() {
    QSettings settings;
    QString savedIdentifier = settings.value("login_identifier").toString();
    QString savedPassword = settings.value("login_password").toString();
    bool rememberMe = settings.value("login_remember", false).toBool();
    
    m_rememberMeCheck->setChecked(rememberMe);
    
    if (rememberMe) {
        if (!savedIdentifier.isEmpty()) {
            m_loginIdentifierEdit->setText(savedIdentifier);
        }
        if (!savedPassword.isEmpty()) {
            m_loginPasswordEdit->setText(savedPassword);
        }
    }
}

void UserLoginDialog::saveCredentials() {
    QSettings settings;
    QString identifier = m_loginIdentifierEdit->text().trimmed();
    QString password = m_loginPasswordEdit->text();
    bool rememberMe = m_rememberMeCheck->isChecked();
    
    settings.setValue("login_remember", rememberMe);
    
    if (rememberMe) {
        if (!identifier.isEmpty()) {
            settings.setValue("login_identifier", identifier);
        } else {
            settings.remove("login_identifier");
        }
        
        if (!password.isEmpty()) {
            settings.setValue("login_password", password);
        } else {
            settings.remove("login_password");
        }
    } else {
        settings.remove("login_identifier");
        settings.remove("login_password");
    }
}
