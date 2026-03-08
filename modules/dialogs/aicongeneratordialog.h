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

private slots:
    void onGenerateClicked();
    void onRegenerateClicked();
    void onDownloadFinished(QNetworkReply *reply);
    void onMethodChanged();
    void onColorChanged(const QString &color);

private:
    void setupUI();
    bool tryTemplateMatch();
    bool searchIconLibraries();
    bool useSiliconFlowAPI();
    bool useDALLE3();
    bool useStabilityAPI();
    void callImageAPI(const QString &provider, const QString &model, const QString &prompt, const QString &endpoint, const QString &apiKey);
    void downloadImage(const QString &url);
    void saveGeneratedIcon(const QByteArray &imageData);
    void generateTemplateIcon(const QString &iconType, const QString &color);
    
    QLineEdit *promptEdit;
    QTextEdit *styleEdit;
    QComboBox *methodCombo;
    QComboBox *colorCombo;
    QComboBox *templateCombo;
    QPushButton *generateBtn;
    QPushButton *regenerateBtn;
    QPushButton *okBtn;
    QLabel *previewLabel;
    QLabel *statusLabel;
    QNetworkAccessManager *networkManager;
    
    QString m_generatedIconPath;
    QByteArray m_lastImageData;
    QString m_lastPrompt;
};

#endif
