#ifndef CHATTESTDIALOG_H
#define CHATTESTDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QPointer>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QSettings>
#include <QHostInfo>
#include <QCryptographicHash>
#include <QTimer>

class ChatTestDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ChatTestDialog(QWidget *parent = nullptr);
    ~ChatTestDialog();

private slots:
    void onSendButtonClicked();
    void onClearButtonClicked();
    void onAIResponse(QPointer<QNetworkReply> reply);
    void onTimeout();

private:
    void setupUI();
    void appendMessage(const QString &message, bool isUser, const QString &aiName = "AI");
    void callAI(const QString &message);
    QString getAPIKey();
    QString getCurrentModel();
    QString getAPIEndpoint();
    QString getDefaultEndpoint(const QString &model);
    QString getModelName(const QString &model);
    QString getModelDisplayName(const QString &model);

    QTextEdit *chatDisplay;
    QLineEdit *messageInput;
    QPushButton *sendButton;
    QPushButton *clearButton;
    QLabel *statusLabel;
    QNetworkAccessManager *networkManager;
    QPointer<QNetworkReply> currentReply;
    bool isProcessing;
};

#endif // CHATTESTDIALOG_H
