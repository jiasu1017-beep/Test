#ifndef AISETTINGSDIALOG_H
#define AISETTINGSDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QGroupBox>
#include <QNetworkAccessManager>
#include <QPointer>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QSettings>
#include <QHostInfo>
#include <QCryptographicHash>
#include <QTimer>

class AISettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AISettingsDialog(QWidget *parent = nullptr);
    ~AISettingsDialog();

    QString getCurrentModel() const;
    QString getAPIKey() const;
    QString getAPIEndpoint() const;

private slots:
    void onAISettingsChanged();
    void onSaveAIConfig();
    void onTestAIConnection();
    void onChatTestClicked();

private:
    void setupUI();
    void loadAISettings();
    QString loadSavedAPIKey();
    QString getDefaultEndpoint(const QString &model);
    QString getModelName(const QString &model);

    QComboBox *aiModelCombo;
    QLineEdit *apiKeyEdit;
    QLineEdit *apiEndpointEdit;
    QPushButton *testAiBtn;
    QPushButton *saveAiBtn;
    QPushButton *chatTestBtn;
    QLabel *aiStatusLabel;
    QNetworkAccessManager *networkManager;
};

#endif // AISETTINGSDIALOG_H
