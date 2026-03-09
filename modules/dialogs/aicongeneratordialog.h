#ifndef AICONGENERATORDIALOG_H
#define AICONGENERATORDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QRadioButton>
#include <QNetworkAccessManager>
#include <QPixmap>
#include <QString>

#include "modules/core/aiconfig.h"

enum IconGenMethod {
    METHOD_TEMPLATE,
    METHOD_ICONFINDER,
    METHOD_FLATICON,
    METHOD_SILICONFLOW,
    METHOD_DALLE3,
    METHOD_STABILITY
};

class AIIconGeneratorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AIIconGeneratorDialog(QWidget *parent = nullptr);
    ~AIIconGeneratorDialog();
    
    QString getGeneratedIconPath() const;
    void setTemplateIcon(const QString &iconPath);

private slots:
    void onGenerateClicked();
    void onRegenerateClicked();
    void onSaveIconClicked();
    void onUseIconClicked();
    void onDownloadFinished(QNetworkReply *reply);
    void onMethodChanged();
    void onColorChanged(const QString &color);

private:
    void setupUI();
    bool tryTemplateMatch();
    bool searchIconLibraries();
    void callImageAPI(const QString &provider, const QString &model, const QString &prompt, const QString &endpoint, const QString &apiKey);
    void downloadImage(const QString &url);
    bool saveGeneratedIcon(const QByteArray &imageData);
    void generateTemplateIcon(const QString &iconType, const QString &color);
    QString decryptApiKey(const QString &encryptedKey);
    QString buildPrompt(const QString &basePrompt);
    void getMethodAndProviderFromKeyId(const QString &keyId, int &method, QString &provider, AIImageConfig &config);
    
    QLineEdit *promptEdit;
    QTextEdit *styleEdit;
    QComboBox *methodCombo;
    QComboBox *colorCombo;
    QComboBox *templateCombo;
    QComboBox *iconTypeCombo;
    QComboBox *designStyleCombo;
    QComboBox *sizeCombo;
    QComboBox *backgroundCombo;
    QPushButton *generateBtn;
    QPushButton *regenerateBtn;
    QPushButton *saveBtn;
    QPushButton *okBtn;
    QLabel *previewLabel;
    QLabel *statusLabel;
    QNetworkAccessManager *networkManager;
    QNetworkReply *m_currentReply;
    
    QString m_generatedIconPath;
    QByteArray m_lastImageData;
    QString m_lastPrompt;
    QString m_logFileName;
    bool m_iconSaved;
};

#endif
