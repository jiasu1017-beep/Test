#include "aisettingsdialog.h"
#include "modules/dialogs/chattestdialog.h"
#include <QMessageBox>

AISettingsDialog::AISettingsDialog(QWidget *parent)
    : QDialog(parent), networkManager(nullptr)
{
    setupUI();
    loadAISettings();
}

AISettingsDialog::~AISettingsDialog()
{
}

void AISettingsDialog::setupUI()
{
    setWindowTitle("AI设置");
    setMinimumSize(500, 400);
    resize(600, 500);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QGroupBox *aiGroup = new QGroupBox("🤖 AI模型配置", this);
    QVBoxLayout *aiLayout = new QVBoxLayout(aiGroup);

    QLabel *aiDescLabel = new QLabel("配置AI模型以启用智能任务分析功能", this);
    aiDescLabel->setStyleSheet("padding: 5px; color: #666; font-size: 12px;");
    aiLayout->addWidget(aiDescLabel);

    QHBoxLayout *modelLayout = new QHBoxLayout();
    QLabel *modelLabel = new QLabel("AI模型:", this);
    modelLabel->setMinimumWidth(80);
    aiModelCombo = new QComboBox(this);
    aiModelCombo->addItem("🤖 MiniMax", "minimax");
    aiModelCombo->addItem("🔵 OpenAI GPT-3.5", "gpt35");
    aiModelCombo->addItem("🔷 OpenAI GPT-4", "gpt4");
    aiModelCombo->addItem("🦊 Anthropic Claude-3", "claude");
    aiModelCombo->addItem("💎 Google Gemini", "gemini");
    aiModelCombo->addItem("🟢 通义千问", "qwen");
    aiModelCombo->addItem("🔶 讯飞星火", "spark");
    aiModelCombo->addItem("🚀 DeepSeek (硅基流动)", "deepseek");
    aiModelCombo->addItem("💻 本地关键词匹配", "local");
    connect(aiModelCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AISettingsDialog::onAISettingsChanged);
    modelLayout->addWidget(modelLabel);
    modelLayout->addWidget(aiModelCombo);
    aiLayout->addLayout(modelLayout);

    QHBoxLayout *apiKeyLayout = new QHBoxLayout();
    QLabel *apiKeyLabel = new QLabel("API Key:", this);
    apiKeyLabel->setMinimumWidth(80);
    apiKeyEdit = new QLineEdit(this);
    apiKeyEdit->setPlaceholderText("请输入API Key");
    apiKeyEdit->setEchoMode(QLineEdit::Password);
    apiKeyLayout->addWidget(apiKeyLabel);
    apiKeyLayout->addWidget(apiKeyEdit);
    aiLayout->addLayout(apiKeyLayout);

    QHBoxLayout *endpointLayout = new QHBoxLayout();
    QLabel *endpointLabel = new QLabel("API地址:", this);
    endpointLabel->setMinimumWidth(80);
    apiEndpointEdit = new QLineEdit(this);
    apiEndpointEdit->setPlaceholderText("留空使用默认地址");
    endpointLayout->addWidget(endpointLabel);
    endpointLayout->addWidget(apiEndpointEdit);
    aiLayout->addLayout(endpointLayout);

    QHBoxLayout *buttonLayout2 = new QHBoxLayout();
    testAiBtn = new QPushButton("测试连接", this);
    testAiBtn->setStyleSheet(
        "QPushButton { background-color: #9b59b6; color: white; padding: 8px 15px; border-radius: 4px; } "
        "QPushButton:hover { background-color: #8e44ad; }"
    );
    connect(testAiBtn, &QPushButton::clicked, this, &AISettingsDialog::onTestAIConnection);

    saveAiBtn = new QPushButton("保存配置", this);
    saveAiBtn->setStyleSheet(
        "QPushButton { background-color: #27ae60; color: white; padding: 8px 15px; border-radius: 4px; } "
        "QPushButton:hover { background-color: #229954; }"
    );
    connect(saveAiBtn, &QPushButton::clicked, this, &AISettingsDialog::onSaveAIConfig);

    chatTestBtn = new QPushButton("💬 Chat测试", this);
    chatTestBtn->setStyleSheet(
        "QPushButton { background-color: #e67e22; color: white; padding: 8px 15px; border-radius: 4px; } "
        "QPushButton:hover { background-color: #d35400; }"
    );
    connect(chatTestBtn, &QPushButton::clicked, this, &AISettingsDialog::onChatTestClicked);

    aiStatusLabel = new QLabel("", this);
    aiStatusLabel->setStyleSheet("padding: 5px;");

    buttonLayout2->addWidget(testAiBtn);
    buttonLayout2->addWidget(saveAiBtn);
    buttonLayout2->addWidget(chatTestBtn);
    buttonLayout2->addStretch();
    aiLayout->addLayout(buttonLayout2);
    aiLayout->addWidget(aiStatusLabel);

    QLabel *aiHelpLabel = new QLabel(
        "📖 API Key获取指南:\n"
        "• MiniMax: https://platform.minimaxi.com\n"
        "• DeepSeek: https://siliconflow.cn (推荐，免费100万tokens)\n"
        "• 通义千问: https://dashscope.aliyun.com\n"
        "• 讯飞星火: https://console.xfyun.cn\n"
        "• OpenAI: https://platform.openai.com (需代理)\n"
        "• Claude: https://console.anthropic.com (需代理)\n"
        "• Gemini: https://aistudio.google.com/app/apikey (需代理)\n\n"
        "💡 安全提示: API Key将加密存储在本地配置文件中"
    );
    aiHelpLabel->setStyleSheet("padding: 10px; color: #666; font-size: 11px; background-color: #f5f5f5; border-radius: 4px;");
    aiHelpLabel->setWordWrap(true);
    aiLayout->addWidget(aiHelpLabel);

    mainLayout->addWidget(aiGroup);

    QHBoxLayout *bottomLayout = new QHBoxLayout();
    QPushButton *closeBtn = new QPushButton("关闭", this);
    closeBtn->setStyleSheet(
        "QPushButton { background-color: #95a5a6; color: white; padding: 8px 20px; border-radius: 4px; } "
        "QPushButton:hover { background-color: #7f8c8d; }"
    );
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    bottomLayout->addStretch();
    bottomLayout->addWidget(closeBtn);
    mainLayout->addLayout(bottomLayout);
}

void AISettingsDialog::loadAISettings()
{
    QSettings settings("PonyWork", "WorkLog");
    QString model = settings.value("ai_model", "qwen").toString();
    QString endpoint = settings.value("ai_endpoint", "").toString();

    int index = aiModelCombo->findData(model);
    if (index >= 0) {
        aiModelCombo->setCurrentIndex(index);
    }

    apiEndpointEdit->setText(endpoint);

    QString apiKey = loadSavedAPIKey();
    if (!apiKey.isEmpty()) {
        apiKeyEdit->setText(apiKey);
    }
}

QString AISettingsDialog::loadSavedAPIKey()
{
    QSettings settings("PonyWork", "WorkLog");
    QString encryptedKey = settings.value("ai_api_key", "").toString();
    if (encryptedKey.isEmpty()) {
        return "";
    }
    QByteArray data = QByteArray::fromBase64(encryptedKey.toUtf8());
    QByteArray key = QCryptographicHash::hash(QByteArray("PonyWorkAI").append(QHostInfo::localHostName().toUtf8()), QCryptographicHash::Sha256);
    QByteArray decrypted;
    for (int i = 0; i < data.size(); ++i) {
        decrypted.append(data.at(i) ^ key.at(i % key.size()));
    }
    return QString::fromUtf8(decrypted);
}

QString AISettingsDialog::getCurrentModel() const
{
    return aiModelCombo->currentData().toString();
}

QString AISettingsDialog::getAPIKey() const
{
    return apiKeyEdit->text().trimmed();
}

QString AISettingsDialog::getAPIEndpoint() const
{
    return apiEndpointEdit->text().trimmed();
}

void AISettingsDialog::onAISettingsChanged()
{
    QString model = aiModelCombo->currentData().toString();
    QString endpoint = getDefaultEndpoint(model);
    apiEndpointEdit->setPlaceholderText(endpoint.isEmpty() ? "留空使用默认地址" : endpoint);
}

void AISettingsDialog::onSaveAIConfig()
{
    QString model = aiModelCombo->currentData().toString();
    QString apiKey = apiKeyEdit->text().trimmed();
    QString endpoint = apiEndpointEdit->text().trimmed();

    if (apiKey.isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入API Key");
        return;
    }

    QSettings settings("PonyWork", "WorkLog");
    settings.setValue("ai_model", model);

    QByteArray key = QCryptographicHash::hash(QByteArray("PonyWorkAI").append(QHostInfo::localHostName().toUtf8()), QCryptographicHash::Sha256);
    QByteArray data = apiKey.toUtf8();
    QByteArray encrypted;
    for (int i = 0; i < data.size(); ++i) {
        encrypted.append(data.at(i) ^ key.at(i % key.size()));
    }
    settings.setValue("ai_api_key", QString::fromUtf8(encrypted.toBase64()));

    settings.setValue("ai_endpoint", endpoint);

    aiStatusLabel->setText("✅ 配置已保存");
    aiStatusLabel->setStyleSheet("padding: 5px; color: #27ae60;");
    QTimer::singleShot(2000, this, [this]() {
        aiStatusLabel->setText("");
    });
}

void AISettingsDialog::onTestAIConnection()
{
    QString model = aiModelCombo->currentData().toString();
    QString apiKey = apiKeyEdit->text().trimmed();
    QString endpoint = apiEndpointEdit->text().trimmed();

    if (apiKey.isEmpty()) {
        aiStatusLabel->setText("❌ 请先输入API Key");
        aiStatusLabel->setStyleSheet("padding: 5px; color: #e74c3c;");
        return;
    }

    if (model == "local") {
        aiStatusLabel->setText("ℹ️ 使用本地关键词匹配，无需测试");
        aiStatusLabel->setStyleSheet("padding: 5px; color: #3498db;");
        return;
    }

    if (endpoint.isEmpty()) {
        endpoint = getDefaultEndpoint(model);
    }

    if (endpoint.isEmpty()) {
        aiStatusLabel->setText("❌ 未配置API地址");
        aiStatusLabel->setStyleSheet("padding: 5px; color: #e74c3c;");
        return;
    }

    testAiBtn->setEnabled(false);
    aiStatusLabel->setText("🔄 测试连接中...");
    aiStatusLabel->setStyleSheet("padding: 5px; color: #3498db;");

    if (!networkManager) {
        networkManager = new QNetworkAccessManager(this);
    }

    QNetworkRequest request;
    QUrl url(endpoint);
    if (!url.isValid()) {
        aiStatusLabel->setText("❌ API地址无效");
        aiStatusLabel->setStyleSheet("padding: 5px; color: #e74c3c;");
        testAiBtn->setEnabled(true);
        return;
    }
    request.setUrl(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", QString("Bearer %1").arg(apiKey).toUtf8());

    QJsonObject json;
    json["model"] = getModelName(model);
    QJsonArray messages;
    QJsonObject msg;
    msg["role"] = "user";
    msg["content"] = "Hello";
    messages.append(msg);
    json["messages"] = messages;
    json["max_tokens"] = 10;
    json["stream"] = false;

    if (model == "qwen") {
        QJsonObject input;
        input["messages"] = json.take("messages");
        json["input"] = input;
        json.remove("max_tokens");
        json.remove("stream");
        json["parameters"] = QJsonObject({
            {"temperature", 0.7},
            {"max_tokens", 10},
            {"result_format", "message"}
        });
    }

    QJsonDocument doc(json);
    qDebug() << "Request JSON:" << doc.toJson(QJsonDocument::Indented);

    QPointer<QNetworkReply> reply = networkManager->post(request, doc.toJson());

    connect(reply, &QNetworkReply::finished, this, [this, reply, endpoint, apiKey]() {
        if (!reply) return;

        testAiBtn->setEnabled(true);

        qDebug() << "Test connection response:";
        qDebug() << "  URL:" << endpoint;
        qDebug() << "  Error:" << reply->error();
        qDebug() << "  ErrorString:" << reply->errorString();

        if (reply->error() != QNetworkReply::NoError) {
            QString errorMsg = reply->errorString();
            aiStatusLabel->setText(QString("❌ 连接失败: %1").arg(errorMsg));
            aiStatusLabel->setStyleSheet("padding: 5px; color: #e74c3c;");
        } else {
            int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            qDebug() << "  StatusCode:" << statusCode;

            if (statusCode == 200) {
                aiStatusLabel->setText("✅ 连接成功!");
                aiStatusLabel->setStyleSheet("padding: 5px; color: #27ae60;");
            } else {
                QByteArray data = reply->readAll();
                qDebug() << "  Response:" << data;
                aiStatusLabel->setText(QString("❌ 错误码: %1").arg(statusCode));
                aiStatusLabel->setStyleSheet("padding: 5px; color: #e74c3c;");
            }
        }
        reply->deleteLater();
    });

    QTimer::singleShot(10000, this, [this, reply]() {
        if (reply && reply->isRunning()) {
            reply->abort();
            testAiBtn->setEnabled(true);
            aiStatusLabel->setText("❌ 连接超时");
            aiStatusLabel->setStyleSheet("padding: 5px; color: #e74c3c;");
        }
    });
}

void AISettingsDialog::onChatTestClicked()
{
    QString apiKey = apiKeyEdit->text().trimmed();
    if (apiKey.isEmpty()) {
        QMessageBox::warning(this, "提示", "请先配置API Key");
        return;
    }

    ChatTestDialog *chatDialog = new ChatTestDialog(this);
    chatDialog->exec();
    delete chatDialog;
}

QString AISettingsDialog::getDefaultEndpoint(const QString &model)
{
    static QMap<QString, QString> endpoints = {
        {"minimax", "https://api.minimax.chat/v1/text/chatcompletion_v2"},
        {"gpt35", "https://api.openai.com/v1/chat/completions"},
        {"gpt4", "https://api.openai.com/v1/chat/completions"},
        {"claude", "https://api.anthropic.com/v1/messages"},
        {"gemini", "https://generativelanguage.googleapis.com/v1beta/models/gemini-pro:generateContent"},
        {"qwen", "https://dashscope.aliyuncs.com/api/v1/services/aigc/text-generation/generation"},
        {"spark", "https://spark-api.xfyun.com/v3.5/chat"},
        {"deepseek", "https://api.siliconflow.cn/v1/chat/completions"}
    };
    return endpoints.value(model, "");
}

QString AISettingsDialog::getModelName(const QString &model)
{
    static QMap<QString, QString> models = {
        {"minimax", "abab6.5s-chat"},
        {"gpt35", "gpt-3.5-turbo"},
        {"gpt4", "gpt-4"},
        {"claude", "claude-3-opus-20240229"},
        {"gemini", "gemini-pro"},
        {"qwen", "qwen-turbo"},
        {"spark", "generalv3.5"},
        {"deepseek", "deepseek-ai/DeepSeek-V3.2"}
    };
    return models.value(model, "");
}
