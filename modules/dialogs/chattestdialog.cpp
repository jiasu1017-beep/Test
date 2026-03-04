#include "chattestdialog.h"
#include "modules/core/aiconfig.h"
#include <QSplitter>
#include <QGroupBox>
#include <QScrollBar>

ChatTestDialog::ChatTestDialog(QWidget *parent)
    : QDialog(parent), networkManager(nullptr), isProcessing(false)
{
    setupUI();
}

ChatTestDialog::~ChatTestDialog()
{
}

void ChatTestDialog::setupUI()
{
    setWindowTitle("AI对话");
    setMinimumSize(600, 500);
    resize(800, 600);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QLabel *titleLabel = new QLabel("AI对话", this);
    titleLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: #2c3e50; padding: 5px;");
    mainLayout->addWidget(titleLabel);

    QGroupBox *chatGroup = new QGroupBox("对话", this);
    QVBoxLayout *chatLayout = new QVBoxLayout(chatGroup);

    chatDisplay = new QTextEdit(this);
    chatDisplay->setReadOnly(true);
    chatDisplay->setStyleSheet(
        "QTextEdit {"
        "  background-color: #f8f9fa;"
        "  border: 1px solid #dee2e6;"
        "  border-radius: 8px;"
        "  padding: 10px;"
        "  font-size: 12px;"
        "}"
    );
    chatLayout->addWidget(chatDisplay);

    QHBoxLayout *inputLayout = new QHBoxLayout();
    messageInput = new QLineEdit(this);
    messageInput->setPlaceholderText("输入消息...");
    messageInput->setStyleSheet(
        "QLineEdit {"
        "  border: 1px solid #ced4da;"
        "  border-radius: 4px;"
        "  padding: 8px;"
        "  font-size: 12px;"
        "}"
        "QLineEdit:focus {"
        "  border: 2px solid #3498db;"
        "}"
    );
    inputLayout->addWidget(messageInput);

    sendButton = new QPushButton("发送", this);
    sendButton->setStyleSheet(
        "QPushButton {"
        "  background-color: #3498db;"
        "  color: white;"
        "  padding: 8px 20px;"
        "  border-radius: 4px;"
        "  font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "  background-color: #2980b9;"
        "}"
        "QPushButton:disabled {"
        "  background-color: #bdc3c7;"
        "}"
    );
    connect(sendButton, &QPushButton::clicked, this, &ChatTestDialog::onSendButtonClicked);
    inputLayout->addWidget(sendButton);

    clearButton = new QPushButton("清空", this);
    clearButton->setStyleSheet(
        "QPushButton {"
        "  background-color: #95a5a6;"
        "  color: white;"
        "  padding: 8px 20px;"
        "  border-radius: 4px;"
        "}"
        "QPushButton:hover {"
        "  background-color: #7f8c8d;"
        "}"
    );
    connect(clearButton, &QPushButton::clicked, this, &ChatTestDialog::onClearButtonClicked);
    inputLayout->addWidget(clearButton);

    chatLayout->addLayout(inputLayout);
    mainLayout->addWidget(chatGroup);

    statusLabel = new QLabel("", this);
    statusLabel->setStyleSheet("padding: 5px; color: #666; font-size: 11px;");
    mainLayout->addWidget(statusLabel);

    connect(messageInput, &QLineEdit::returnPressed, this, &ChatTestDialog::onSendButtonClicked);

    QString currentModel = getCurrentModel();
    QString aiName = getModelDisplayName(currentModel);
    appendMessage("欢迎使用AI对话！\n\n请输入消息验证您的AI配置是否正常工作。", false, aiName);
}

void ChatTestDialog::onSendButtonClicked()
{
    QString message = messageInput->text().trimmed();
    if (message.isEmpty()) {
        return;
    }

    if (isProcessing) {
        return;
    }

    QString currentModel = getCurrentModel();
    QString aiName = getModelDisplayName(currentModel);
    appendMessage(message, true, aiName);
    messageInput->clear();

    callAI(message);
}

void ChatTestDialog::onClearButtonClicked()
{
    chatDisplay->clear();
    QString currentModel = getCurrentModel();
    QString aiName = getModelDisplayName(currentModel);
    appendMessage("对话已清空。", false, aiName);
}

void ChatTestDialog::appendMessage(const QString &message, bool isUser, const QString &aiName)
{
    QString color = isUser ? "#3498db" : "#27ae60";
    QString displayName = isUser ? "您" : aiName;
    QString prefix = isUser ? "👤 " + displayName : "🤖 " + displayName;
    
    QString formatted = QString("<div style='margin: 8px 0;'>"
                           "<span style='color: %1; font-weight: bold;'>%2:</span><br/>"
                           "<span style='color: #333; margin-left: 10px;'>%3</span>"
                           "</div>")
                           .arg(color)
                           .arg(prefix)
                           .arg(message.toHtmlEscaped());

    chatDisplay->append(formatted);
    QScrollBar *scrollBar = chatDisplay->verticalScrollBar();
    scrollBar->setValue(scrollBar->maximum());
}

void ChatTestDialog::callAI(const QString &message)
{
    QString apiKey = getAPIKey();
    if (apiKey.isEmpty()) {
        statusLabel->setText("❌ 请先配置API Key");
        return;
    }

    QString currentModel = getCurrentModel();
    QString aiName = getModelDisplayName(currentModel);
    if (currentModel == "local") {
        appendMessage("本地关键词模式不支持对话测试。", false, aiName);
        return;
    }

    isProcessing = true;
    sendButton->setEnabled(false);

    QString endpoint = getAPIEndpoint();
    if (endpoint.isEmpty()) {
        endpoint = getDefaultEndpoint(currentModel);
    }

    statusLabel->setText("🔄 " + aiName + " 思考中...\nEndpoint: " + endpoint + "\nModel: " + getModelName(currentModel) + "\nAPI Key: " + apiKey.mid(0, 8) + "...");

    if (endpoint.isEmpty()) {
        statusLabel->setText("❌ 未配置API地址");
        isProcessing = false;
        sendButton->setEnabled(true);
        return;
    }

    if (!networkManager) {
        networkManager = new QNetworkAccessManager(this);
    }

    QJsonObject json;
    json["model"] = getModelName(currentModel);
    json["stream"] = false;

    QJsonArray messages;
    QJsonObject msg;
    msg["role"] = "user";
    msg["content"] = message;
    messages.append(msg);
    json["messages"] = messages;

    if (currentModel.startsWith("qwen")) {
        QJsonObject input;
        input["messages"] = json.take("messages");
        json["input"] = input;
        json.remove("stream");
        json["parameters"] = QJsonObject({
            {"temperature", 0.7},
            {"max_tokens", 1024},
            {"result_format", "message"}
        });
    }

    QJsonDocument doc(json);
    QByteArray postData = doc.toJson();

    QNetworkRequest request;
    QUrl url(endpoint);
    if (!url.isValid()) {
        statusLabel->setText("❌ API地址无效");
        isProcessing = false;
        sendButton->setEnabled(true);
        return;
    }
    request.setUrl(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", QString("Bearer %1").arg(apiKey).toUtf8());

    currentReply = networkManager->post(request, postData);

    connect(currentReply, &QNetworkReply::finished, this, [this]() {
        onAIResponse(currentReply);
    });

    QTimer::singleShot(30000, this, [this]() {
        if (currentReply && currentReply->isRunning()) {
            currentReply->abort();
            statusLabel->setText("❌ 请求超时");
            isProcessing = false;
            sendButton->setEnabled(true);
        }
    });
}

void ChatTestDialog::onAIResponse(QPointer<QNetworkReply> reply)
{
    if (!reply) return;

    isProcessing = false;
    sendButton->setEnabled(true);

    QString currentModel = getCurrentModel();
    QString aiName = getModelDisplayName(currentModel);

    if (reply->error() != QNetworkReply::NoError) {
        QString errorMsg = reply->errorString();
        int httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        QByteArray responseData = reply->readAll();
        QString responseText = QString::fromUtf8(responseData);
        
        QString detailedMsg = QString("错误: %1 (HTTP %2)\n响应: %3").arg(errorMsg).arg(httpStatus).arg(responseText.left(200));
        statusLabel->setText(QString("❌ 调用失败: %1").arg(detailedMsg));
        appendMessage(detailedMsg, false, aiName);
        reply->deleteLater();
        return;
    }

    QByteArray data = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
        statusLabel->setText("❌ 响应格式错误");
        appendMessage("错误: 响应格式不正确", false, aiName);
        reply->deleteLater();
        return;
    }

    QJsonObject rootObj = doc.object();

    if (rootObj.contains("base_resp")) {
        int statusCode = rootObj["base_resp"].toObject()["status_code"].toInt();
        if (statusCode != 0) {
            QString errorMsg = rootObj["base_resp"].toObject()["status_msg"].toString();
            statusLabel->setText(QString("❌ API错误: %1").arg(errorMsg));
            appendMessage(QString("API错误: %1").arg(errorMsg), false, aiName);
            reply->deleteLater();
            return;
        }
    }

    QString responseText;
    if (rootObj.contains("choices") && rootObj["choices"].isArray()) {
        QJsonArray choices = rootObj["choices"].toArray();
        if (!choices.isEmpty()) {
            QJsonObject choice = choices.first().toObject();
            if (choice.contains("message")) {
                responseText = choice["message"].toObject()["content"].toString();
            }
        }
    } else if (rootObj.contains("output")) {
        QJsonObject output = rootObj["output"].toObject();
        if (output.contains("choices") && output["choices"].isArray()) {
            QJsonArray choices = output["choices"].toArray();
            if (!choices.isEmpty()) {
                QJsonObject choice = choices.first().toObject();
                if (choice.contains("message")) {
                    responseText = choice["message"].toObject()["content"].toString();
                }
            }
        }
    }

    if (responseText.isEmpty()) {
        statusLabel->setText("❌ 无法解析响应");
        appendMessage("错误: 无法解析AI响应", false, aiName);
    } else {
        statusLabel->setText("✅ 调用成功");
        appendMessage(responseText, false, aiName);
    }

    reply->deleteLater();
}

void ChatTestDialog::onTimeout()
{
    if (currentReply && currentReply->isRunning()) {
        currentReply->abort();
        statusLabel->setText("❌ 请求超时");
        isProcessing = false;
        sendButton->setEnabled(true);
    }
}

QString ChatTestDialog::getAPIKey()
{
    AIKeyConfig key = AIConfig::instance().getDefaultKey();
    return key.apiKey;
}

QString ChatTestDialog::getCurrentModel()
{
    AIKeyConfig key = AIConfig::instance().getDefaultKey();
    return key.model;
}

QString ChatTestDialog::getAPIEndpoint()
{
    AIKeyConfig key = AIConfig::instance().getDefaultKey();
    if (key.endpoint.isEmpty()) {
        return getDefaultEndpoint(key.model);
    }
    return key.endpoint;
}

QString ChatTestDialog::getDefaultEndpoint(const QString &model)
{
    AIModelInfo modelInfo = AIConfig::instance().getModelInfo(model);
    if (!modelInfo.defaultEndpoint.isEmpty()) {
        return modelInfo.defaultEndpoint;
    }
    for (const AIModelInfo &m : AIConfig::instance().getAllModels()) {
        if (m.name == model) {
            return m.defaultEndpoint;
        }
    }
    return "";
}

QString ChatTestDialog::getModelName(const QString &model)
{
    static QMap<QString, QString> models = {
        {"minimax", "abab6.5s-chat"},
        {"abab6.5s-chat", "abab6.5s-chat"},
        {"abab6.5g-chat", "abab6.5g-chat"},
        {"gpt35", "gpt-3.5-turbo"},
        {"gpt-3.5-turbo", "gpt-3.5-turbo"},
        {"gpt4", "gpt-4"},
        {"gpt-4", "gpt-4"},
        {"gpt4o", "gpt-4o"},
        {"gpt4turbo", "gpt-4-turbo"},
        {"claude", "claude-3-opus-20240229"},
        {"claude3haiku", "claude-3-haiku-20240307"},
        {"claude3sonnet", "claude-3-sonnet-20240229"},
        {"claude3opus", "claude-3-opus-20240229"},
        {"claude3.5sonnet", "claude-3-5-sonnet-20240620"},
        {"gemini", "gemini-pro"},
        {"gemini-pro", "gemini-pro"},
        {"gemini-1.5-pro", "gemini-1.5-pro"},
        {"qwen", "qwen-turbo"},
        {"qwen-turbo", "qwen-turbo"},
        {"qwen-plus", "qwen-plus"},
        {"qwen-max", "qwen-max"},
        {"qwen-long", "qwen-long"},
        {"spark", "generalv3.5"},
        {"spark-v3.5", "generalv3.5"},
        {"spark-v3", "generalv3"},
        {"deepseek", "deepseek-ai/DeepSeek-V3.2"},
        {"deepseek-v2", "deepseek-ai/DeepSeek-V2-Chat"},
        {"deepseek-v2.5", "deepseek-ai/DeepSeek-V2.5"},
        {"deepseek-v3", "deepseek-ai/DeepSeek-V3"},
        {"deepseek-v3.2", "deepseek-ai/DeepSeek-V3.2"},
        {"deepseek-coder", "deepseek-ai/DeepSeek-Coder-V2"},
        {"qwen-coder", "qwen-coder-7b-instruct"}
    };
    
    if (models.contains(model)) {
        return models.value(model);
    }
    
    if (model.contains("deepseek", Qt::CaseInsensitive)) {
        return "deepseek-ai/DeepSeek-V3.2";
    } else if (model.contains("qwen", Qt::CaseInsensitive)) {
        return "qwen-turbo";
    } else if (model.contains("gpt", Qt::CaseInsensitive)) {
        return "gpt-3.5-turbo";
    } else if (model.contains("claude", Qt::CaseInsensitive)) {
        return "claude-3-sonnet-20240229";
    } else if (model.contains("gemini", Qt::CaseInsensitive)) {
        return "gemini-pro";
    } else if (model.contains("spark", Qt::CaseInsensitive)) {
        return "generalv3.5";
    } else if (model.contains("minimax", Qt::CaseInsensitive) || model.contains("abab", Qt::CaseInsensitive)) {
        return "abab6.5s-chat";
    }
    
    return model;
}

QString ChatTestDialog::getModelDisplayName(const QString &model)
{
    static QMap<QString, QString> displayNames = {
        {"minimax", "MiniMax"},
        {"abab6.5s-chat", "MiniMax ABAB 6.5s"},
        {"abab6.5g-chat", "MiniMax ABAB 6.5g"},
        {"gpt35", "OpenAI GPT-3.5"},
        {"gpt-3.5-turbo", "OpenAI GPT-3.5"},
        {"gpt4", "OpenAI GPT-4"},
        {"gpt-4", "OpenAI GPT-4"},
        {"gpt4o", "OpenAI GPT-4o"},
        {"gpt4turbo", "OpenAI GPT-4 Turbo"},
        {"claude", "Claude-3"},
        {"claude3haiku", "Claude-3 Haiku"},
        {"claude3sonnet", "Claude-3 Sonnet"},
        {"claude3opus", "Claude-3 Opus"},
        {"claude3.5sonnet", "Claude-3.5 Sonnet"},
        {"gemini", "Google Gemini"},
        {"gemini-pro", "Google Gemini Pro"},
        {"gemini-1.5-pro", "Google Gemini 1.5 Pro"},
        {"qwen", "通义千问"},
        {"qwen-turbo", "通义千问 Turbo"},
        {"qwen-plus", "通义千问 Plus"},
        {"qwen-max", "通义千问 Max"},
        {"qwen-long", "通义千问 Long"},
        {"spark", "讯飞星火"},
        {"spark-v3.5", "讯飞星火 V3.5"},
        {"spark-v3", "讯飞星火 V3"},
        {"deepseek", "DeepSeek"},
        {"deepseek-v2", "DeepSeek V2"},
        {"deepseek-v2.5", "DeepSeek V2.5"},
        {"deepseek-v3", "DeepSeek V3"},
        {"deepseek-v3.2", "DeepSeek V3.2"},
        {"deepseek-coder", "DeepSeek Coder V2"},
        {"qwen-coder", "Qwen Coder"},
        {"local", "本地关键词匹配"}
    };
    
    if (model.startsWith("qwen")) {
        return "通义千问";
    } else if (model.startsWith("spark")) {
        return "讯飞星火";
    } else if (model.startsWith("deepseek")) {
        return "DeepSeek";
    } else if (model.startsWith("gpt")) {
        return "OpenAI";
    } else if (model.startsWith("claude")) {
        return "Claude";
    } else if (model.startsWith("gemini")) {
        return "Google Gemini";
    } else if (model.startsWith("minimax") || model.startsWith("abab")) {
        return "MiniMax";
    }
    
    return displayNames.value(model, "AI");
}
