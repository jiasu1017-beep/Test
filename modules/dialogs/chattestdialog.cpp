#include "chattestdialog.h"
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
    setWindowTitle("AI模型测试");
    setMinimumSize(600, 500);
    resize(800, 600);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QLabel *titleLabel = new QLabel("AI模型测试 - 验证配置是否正常", this);
    titleLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: #2c3e50; padding: 5px;");
    mainLayout->addWidget(titleLabel);

    QGroupBox *chatGroup = new QGroupBox("对话测试", this);
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
    messageInput->setPlaceholderText("输入消息进行测试...");
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

    appendMessage("欢迎使用AI模型测试！\n\n请输入消息测试您的AI配置是否正常工作。", false);
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

    appendMessage(message, true);
    messageInput->clear();

    callAI(message);
}

void ChatTestDialog::onClearButtonClicked()
{
    chatDisplay->clear();
    appendMessage("对话已清空。", false);
}

void ChatTestDialog::appendMessage(const QString &message, bool isUser)
{
    QString color = isUser ? "#3498db" : "#27ae60";
    QString prefix = isUser ? "👤 您" : "🤖 AI";
    
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
    if (currentModel == "local") {
        appendMessage("本地关键词模式不支持对话测试。", false);
        return;
    }

    isProcessing = true;
    sendButton->setEnabled(false);
    statusLabel->setText("🔄 AI思考中...");

    QString endpoint = getAPIEndpoint();
    if (endpoint.isEmpty()) {
        endpoint = getDefaultEndpoint(currentModel);
    }

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

    if (currentModel == "qwen") {
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

    if (reply->error() != QNetworkReply::NoError) {
        QString errorMsg = reply->errorString();
        statusLabel->setText(QString("❌ 调用失败: %1").arg(errorMsg));
        appendMessage(QString("错误: %1").arg(errorMsg), false);
        reply->deleteLater();
        return;
    }

    QByteArray data = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
        statusLabel->setText("❌ 响应格式错误");
        appendMessage("错误: 响应格式不正确", false);
        reply->deleteLater();
        return;
    }

    QJsonObject rootObj = doc.object();

    if (rootObj.contains("base_resp")) {
        int statusCode = rootObj["base_resp"].toObject()["status_code"].toInt();
        if (statusCode != 0) {
            QString errorMsg = rootObj["base_resp"].toObject()["status_msg"].toString();
            statusLabel->setText(QString("❌ API错误: %1").arg(errorMsg));
            appendMessage(QString("API错误: %1").arg(errorMsg), false);
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
        appendMessage("错误: 无法解析AI响应", false);
    } else {
        statusLabel->setText("✅ 调用成功");
        appendMessage(responseText, false);
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

QString ChatTestDialog::getCurrentModel()
{
    QSettings settings("PonyWork", "WorkLog");
    return settings.value("ai_model", "qwen").toString();
}

QString ChatTestDialog::getAPIEndpoint()
{
    QSettings settings("PonyWork", "WorkLog");
    return settings.value("ai_endpoint", "").toString();
}

QString ChatTestDialog::getDefaultEndpoint(const QString &model)
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

QString ChatTestDialog::getModelName(const QString &model)
{
    static QMap<QString, QString> models = {
        {"minimax", "abab6.5s-chat"},
        {"gpt35", "gpt-3.5-turbo"},
        {"gpt4", "gpt-4"},
        {"claude", "claude-3-opus-20240229"},
        {"gemini", "gemini-pro"},
        {"qwen", "qwen-turbo"},
        {"spark", "generalv3.5"},
        {"deepseek", "deepseek-ai/DeepSeek-V2-Chat"}
    };
    return models.value(model, "");
}
