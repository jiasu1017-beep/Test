#ifndef CHANGEPASSWORDDIALOG_H
#define CHANGEPASSWORDDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include "userapi.h"

class ChangePasswordDialog : public QDialog {
    Q_OBJECT
public:
    explicit ChangePasswordDialog(QWidget *parent = nullptr);
    ~ChangePasswordDialog();

private slots:
    void onChangePasswordClicked();
    void onPasswordChanged();
    void onPasswordChangeFailed(const QString& error);
    void toggleOldPasswordVisibility();
    void toggleNewPasswordVisibility();
    void toggleConfirmPasswordVisibility();
    void onNewPasswordTextChanged(const QString& text);
    void onConfirmPasswordTextChanged(const QString& text);

private:
    void setupUI();
    void setUIEnabled(bool enabled);
    bool validatePassword(const QString& password);

private:
    QLineEdit *m_oldPasswordEdit;
    QPushButton *m_oldPasswordToggleBtn;
    
    QLineEdit *m_newPasswordEdit;
    QPushButton *m_newPasswordToggleBtn;
    QLabel *m_passwordStrengthLabel;
    
    QLineEdit *m_confirmPasswordEdit;
    QPushButton *m_confirmPasswordToggleBtn;
    QLabel *m_confirmStatusLabel;
    
    QPushButton *m_changePasswordBtn;
    QPushButton *m_cancelBtn;
    QLabel *m_statusLabel;
};

#endif // CHANGEPASSWORDDIALOG_H
