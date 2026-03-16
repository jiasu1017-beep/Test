#include "changepassworddialog.h"
#include <QFormLayout>
#include <QGraphicsDropShadowEffect>
#include <QStyle>
#include <QTimer>

ChangePasswordDialog::ChangePasswordDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUI();
    setWindowTitle("修改密码");
    setModal(true);
    resize(400, 450);
    
    setStyleSheet(R"(
        ChangePasswordDialog {
            background-color: #f5f6fa;
        }
        QLabel[cssClass="titleLabel"] {
            font-size: 20px;
            font-weight: bold;
            color: #2d3436;
            padding: 10px;
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
            font-size: 15px;
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
    )");

    connect(UserManager::instance(), &UserManager::passwordChanged, this, &ChangePasswordDialog::onPasswordChanged);
    connect(UserManager::instance(), &UserManager::loginFailed, this, &ChangePasswordDialog::onPasswordChangeFailed);
}

ChangePasswordDialog::~ChangePasswordDialog() {
}

void ChangePasswordDialog::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(30, 30, 30, 30);

    QLabel *titleLabel = new QLabel("🔐 修改密码", this);
    titleLabel->setProperty("cssClass", "titleLabel");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    mainLayout->addSpacing(10);

    QLabel *oldPasswordLabel = new QLabel("🔒 当前密码", this);
    oldPasswordLabel->setStyleSheet("font-weight: bold; font-size: 14px;");
    mainLayout->addWidget(oldPasswordLabel);
    
    QWidget *oldPasswordWidget = new QWidget(this);
    QHBoxLayout *oldPasswordLayout = new QHBoxLayout(oldPasswordWidget);
    oldPasswordLayout->setContentsMargins(0, 0, 0, 0);
    oldPasswordLayout->setSpacing(5);
    
    m_oldPasswordEdit = new QLineEdit(this);
    m_oldPasswordEdit->setPlaceholderText("请输入当前密码");
    m_oldPasswordEdit->setEchoMode(QLineEdit::Password);
    m_oldPasswordEdit->setFixedHeight(45);
    m_oldPasswordEdit->setClearButtonEnabled(true);
    oldPasswordLayout->addWidget(m_oldPasswordEdit);
    
    m_oldPasswordToggleBtn = new QPushButton("👁️", this);
    m_oldPasswordToggleBtn->setFixedSize(45, 45);
    m_oldPasswordToggleBtn->setCursor(Qt::PointingHandCursor);
    m_oldPasswordToggleBtn->setStyleSheet(R"(
        QPushButton {
            background-color: transparent;
            border: none;
            font-size: 18px;
        }
        QPushButton:hover {
            background-color: #e0e0e0;
            border-radius: 4px;
        }
    )");
    connect(m_oldPasswordToggleBtn, &QPushButton::clicked, this, &ChangePasswordDialog::toggleOldPasswordVisibility);
    oldPasswordLayout->addWidget(m_oldPasswordToggleBtn);
    
    mainLayout->addWidget(oldPasswordWidget);

    QLabel *newPasswordLabel = new QLabel("🔑 新密码", this);
    newPasswordLabel->setStyleSheet("font-weight: bold; font-size: 14px;");
    mainLayout->addWidget(newPasswordLabel);
    
    QWidget *newPasswordWidget = new QWidget(this);
    QHBoxLayout *newPasswordLayout = new QHBoxLayout(newPasswordWidget);
    newPasswordLayout->setContentsMargins(0, 0, 0, 0);
    newPasswordLayout->setSpacing(5);
    
    m_newPasswordEdit = new QLineEdit(this);
    m_newPasswordEdit->setPlaceholderText("请输入新密码（至少6位）");
    m_newPasswordEdit->setEchoMode(QLineEdit::Password);
    m_newPasswordEdit->setFixedHeight(45);
    m_newPasswordEdit->setClearButtonEnabled(true);
    connect(m_newPasswordEdit, &QLineEdit::textChanged, this, &ChangePasswordDialog::onNewPasswordTextChanged);
    newPasswordLayout->addWidget(m_newPasswordEdit);
    
    m_newPasswordToggleBtn = new QPushButton("👁️", this);
    m_newPasswordToggleBtn->setFixedSize(45, 45);
    m_newPasswordToggleBtn->setCursor(Qt::PointingHandCursor);
    m_newPasswordToggleBtn->setStyleSheet(R"(
        QPushButton {
            background-color: transparent;
            border: none;
            font-size: 18px;
        }
        QPushButton:hover {
            background-color: #e0e0e0;
            border-radius: 4px;
        }
    )");
    connect(m_newPasswordToggleBtn, &QPushButton::clicked, this, &ChangePasswordDialog::toggleNewPasswordVisibility);
    newPasswordLayout->addWidget(m_newPasswordToggleBtn);
    
    mainLayout->addWidget(newPasswordWidget);
    
    m_passwordStrengthLabel = new QLabel(this);
    m_passwordStrengthLabel->setStyleSheet("font-size: 12px; color: #636e72;");
    mainLayout->addWidget(m_passwordStrengthLabel);

    QLabel *confirmPasswordLabel = new QLabel("🔑 确认新密码", this);
    confirmPasswordLabel->setStyleSheet("font-weight: bold; font-size: 14px;");
    mainLayout->addWidget(confirmPasswordLabel);
    
    QWidget *confirmPasswordWidget = new QWidget(this);
    QHBoxLayout *confirmPasswordLayout = new QHBoxLayout(confirmPasswordWidget);
    confirmPasswordLayout->setContentsMargins(0, 0, 0, 0);
    confirmPasswordLayout->setSpacing(5);
    
    m_confirmPasswordEdit = new QLineEdit(this);
    m_confirmPasswordEdit->setPlaceholderText("请再次输入新密码");
    m_confirmPasswordEdit->setEchoMode(QLineEdit::Password);
    m_confirmPasswordEdit->setFixedHeight(45);
    m_confirmPasswordEdit->setClearButtonEnabled(true);
    connect(m_confirmPasswordEdit, &QLineEdit::textChanged, this, &ChangePasswordDialog::onConfirmPasswordTextChanged);
    confirmPasswordLayout->addWidget(m_confirmPasswordEdit);
    
    m_confirmPasswordToggleBtn = new QPushButton("👁️", this);
    m_confirmPasswordToggleBtn->setFixedSize(45, 45);
    m_confirmPasswordToggleBtn->setCursor(Qt::PointingHandCursor);
    m_confirmPasswordToggleBtn->setStyleSheet(R"(
        QPushButton {
            background-color: transparent;
            border: none;
            font-size: 18px;
        }
        QPushButton:hover {
            background-color: #e0e0e0;
            border-radius: 4px;
        }
    )");
    connect(m_confirmPasswordToggleBtn, &QPushButton::clicked, this, &ChangePasswordDialog::toggleConfirmPasswordVisibility);
    confirmPasswordLayout->addWidget(m_confirmPasswordToggleBtn);
    
    mainLayout->addWidget(confirmPasswordWidget);
    
    m_confirmStatusLabel = new QLabel(this);
    m_confirmStatusLabel->setStyleSheet("font-size: 12px;");
    mainLayout->addWidget(m_confirmStatusLabel);

    m_statusLabel = new QLabel(this);
    m_statusLabel->setProperty("cssClass", "statusLabel");
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setWordWrap(true);
    mainLayout->addWidget(m_statusLabel);

    mainLayout->addSpacing(10);

    m_changePasswordBtn = new QPushButton("🔐 确认修改", this);
    m_changePasswordBtn->setProperty("cssClass", "primaryBtn");
    m_changePasswordBtn->setFixedHeight(50);
    m_changePasswordBtn->setCursor(Qt::PointingHandCursor);
    connect(m_changePasswordBtn, &QPushButton::clicked, this, &ChangePasswordDialog::onChangePasswordClicked);
    mainLayout->addWidget(m_changePasswordBtn);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(10);
    
    m_cancelBtn = new QPushButton("取消", this);
    m_cancelBtn->setProperty("cssClass", "secondaryBtn");
    m_cancelBtn->setFixedHeight(40);
    connect(m_cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    buttonLayout->addWidget(m_cancelBtn);
    
    mainLayout->addLayout(buttonLayout);
}

void ChangePasswordDialog::setUIEnabled(bool enabled) {
    m_oldPasswordEdit->setEnabled(enabled);
    m_newPasswordEdit->setEnabled(enabled);
    m_confirmPasswordEdit->setEnabled(enabled);
    m_changePasswordBtn->setEnabled(enabled);
    m_cancelBtn->setEnabled(enabled);
}

bool ChangePasswordDialog::validatePassword(const QString& password) {
    if (password.length() < 6) {
        return false;
    }
    return true;
}

void ChangePasswordDialog::onChangePasswordClicked() {
    QString oldPassword = m_oldPasswordEdit->text();
    QString newPassword = m_newPasswordEdit->text();
    QString confirmPassword = m_confirmPasswordEdit->text();
    
    if (oldPassword.isEmpty()) {
        m_statusLabel->setText("请输入当前密码");
        m_statusLabel->setProperty("isError", true);
        m_statusLabel->style()->unpolish(m_statusLabel);
        m_statusLabel->style()->polish(m_statusLabel);
        return;
    }
    
    if (newPassword.isEmpty()) {
        m_statusLabel->setText("请输入新密码");
        m_statusLabel->setProperty("isError", true);
        m_statusLabel->style()->unpolish(m_statusLabel);
        m_statusLabel->style()->polish(m_statusLabel);
        return;
    }
    
    if (!validatePassword(newPassword)) {
        m_statusLabel->setText("新密码长度不能少于6位");
        m_statusLabel->setProperty("isError", true);
        m_statusLabel->style()->unpolish(m_statusLabel);
        m_statusLabel->style()->polish(m_statusLabel);
        return;
    }
    
    if (newPassword != confirmPassword) {
        m_statusLabel->setText("两次输入的新密码不一致");
        m_statusLabel->setProperty("isError", true);
        m_statusLabel->style()->unpolish(m_statusLabel);
        m_statusLabel->style()->polish(m_statusLabel);
        return;
    }
    
    if (oldPassword == newPassword) {
        m_statusLabel->setText("新密码不能与当前密码相同");
        m_statusLabel->setProperty("isError", true);
        m_statusLabel->style()->unpolish(m_statusLabel);
        m_statusLabel->style()->polish(m_statusLabel);
        return;
    }
    
    setUIEnabled(false);
    m_statusLabel->clear();
    
    UserManager::instance()->changePassword(oldPassword, newPassword);
}

void ChangePasswordDialog::onPasswordChanged() {
    setUIEnabled(true);
    
    m_statusLabel->setText("✓ 密码修改成功！请使用新密码重新登录");
    m_statusLabel->setProperty("isError", false);
    m_statusLabel->style()->unpolish(m_statusLabel);
    m_statusLabel->style()->polish(m_statusLabel);
    
    m_oldPasswordEdit->clear();
    m_newPasswordEdit->clear();
    m_confirmPasswordEdit->clear();
    
    QTimer::singleShot(3000, this, [this]() {
        accept();
    });
}

void ChangePasswordDialog::onPasswordChangeFailed(const QString& error) {
    setUIEnabled(true);
    
    m_statusLabel->setText(error);
    m_statusLabel->setProperty("isError", true);
    m_statusLabel->style()->unpolish(m_statusLabel);
    m_statusLabel->style()->polish(m_statusLabel);
}

void ChangePasswordDialog::toggleOldPasswordVisibility() {
    if (m_oldPasswordEdit->echoMode() == QLineEdit::Password) {
        m_oldPasswordEdit->setEchoMode(QLineEdit::Normal);
        m_oldPasswordToggleBtn->setText("🔒");
    } else {
        m_oldPasswordEdit->setEchoMode(QLineEdit::Password);
        m_oldPasswordToggleBtn->setText("👁️");
    }
}

void ChangePasswordDialog::toggleNewPasswordVisibility() {
    if (m_newPasswordEdit->echoMode() == QLineEdit::Password) {
        m_newPasswordEdit->setEchoMode(QLineEdit::Normal);
        m_newPasswordToggleBtn->setText("🔒");
    } else {
        m_newPasswordEdit->setEchoMode(QLineEdit::Password);
        m_newPasswordToggleBtn->setText("👁️");
    }
}

void ChangePasswordDialog::toggleConfirmPasswordVisibility() {
    if (m_confirmPasswordEdit->echoMode() == QLineEdit::Password) {
        m_confirmPasswordEdit->setEchoMode(QLineEdit::Normal);
        m_confirmPasswordToggleBtn->setText("🔒");
    } else {
        m_confirmPasswordEdit->setEchoMode(QLineEdit::Password);
        m_confirmPasswordToggleBtn->setText("👁️");
    }
}

void ChangePasswordDialog::onNewPasswordTextChanged(const QString& text) {
    if (text.isEmpty()) {
        m_passwordStrengthLabel->clear();
        return;
    }
    
    if (text.length() < 6) {
        m_passwordStrengthLabel->setText("⚠️ 密码长度不足6位");
        m_passwordStrengthLabel->setStyleSheet("font-size: 12px; color: #d63031;");
    } else if (text.length() < 8) {
        m_passwordStrengthLabel->setText("🔴 密码强度：弱");
        m_passwordStrengthLabel->setStyleSheet("font-size: 12px; color: #e17055;");
    } else if (text.length() < 12) {
        m_passwordStrengthLabel->setText("🟡 密码强度：中等");
        m_passwordStrengthLabel->setStyleSheet("font-size: 12px; color: #fdcb6e;");
    } else {
        m_passwordStrengthLabel->setText("🟢 密码强度：强");
        m_passwordStrengthLabel->setStyleSheet("font-size: 12px; color: #00b894;");
    }
    
    onConfirmPasswordTextChanged(m_confirmPasswordEdit->text());
}

void ChangePasswordDialog::onConfirmPasswordTextChanged(const QString& text) {
    QString newPassword = m_newPasswordEdit->text();
    
    if (text.isEmpty() || newPassword.isEmpty()) {
        m_confirmStatusLabel->clear();
        return;
    }
    
    if (text == newPassword) {
        m_confirmStatusLabel->setText("✓ 密码匹配");
        m_confirmStatusLabel->setStyleSheet("font-size: 12px; color: #00b894;");
    } else {
        m_confirmStatusLabel->setText("✗ 密码不匹配");
        m_confirmStatusLabel->setStyleSheet("font-size: 12px; color: #d63031;");
    }
}
