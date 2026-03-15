#ifndef USERLOGINDIALOG_H
#define USERLOGINDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QCheckBox>
#include <QStackedWidget>
#include <QFrame>
#include "userapi.h"

class UserLoginDialog : public QDialog {
    Q_OBJECT
public:
    explicit UserLoginDialog(QWidget *parent = nullptr);
    ~UserLoginDialog();

private slots:
            void onLoginClicked();
            void onLoginSuccess(const UserInfo& user);
            void onLoginFailed(const QString& error);
            
            void onRegisterClicked();
            void onRegisterSuccess();
            void onRegisterFailed(const QString& error);
            
            void onEmailCheckResult(bool exists);
            
            void switchToRegister();
            void switchToLogin();
            
            void onIdentifierTextChanged(const QString& text);
            void onLoginPasswordTextChanged(const QString& text);
            void onRegisterPasswordTextChanged(const QString& text);
            void onConfirmPasswordTextChanged(const QString& text);
            void onUsernameTextChanged(const QString& text);
            void loadSavedCredentials();
            void saveCredentials();

private:
    void setupUI();
    void setupLoginPage();
    void setupRegisterPage();
    void setUIEnabled(bool enabled);
    bool validateEmail(const QString& email);
    bool validatePassword(const QString& password);
    bool validateUsername(const QString& username);
    void showStatusMessage(const QString& message, bool isError = false);

private:
            QStackedWidget *m_stackedWidget;
            
            QWidget *m_loginPage;
            QLineEdit *m_loginIdentifierEdit;
            QLineEdit *m_loginPasswordEdit;
            QCheckBox *m_rememberMeCheck;
            QPushButton *m_loginBtn;
            QLabel *m_loginStatusLabel;
    
    QWidget *m_registerPage;
    QLineEdit *m_registerUsernameEdit;
    QLineEdit *m_registerEmailEdit;
    QLineEdit *m_registerPasswordEdit;
    QLineEdit *m_registerConfirmPasswordEdit;
    QPushButton *m_registerBtn;
    QLabel *m_registerStatusLabel;
    
    QPushButton *m_cancelBtn;
    
    bool m_isValidating;
};

#endif // USERLOGINDIALOG_H
