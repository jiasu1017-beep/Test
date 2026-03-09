#include "aiconfig.h"
#include <QSettings>
#include <QCryptographicHash>
#include <QHostInfo>
#include <QDateTime>
#include <QUuid>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

AIConfig::AIConfig()
{
    initProvidersAndModels();
    initImageProvidersAndModels();
    load();
}

AIConfig::~AIConfig()
{
    save();
}

AIConfig& AIConfig::instance()
{
    static AIConfig instance;
    return instance;
}

void AIConfig::load()
{
    QSettings settings("PonyWork", "WorkLog");

    m_defaultModel = settings.value("ai_default_model", "qwen").toString();
    m_defaultKeyId = settings.value("ai_default_key_id", "").toString();
    m_timeout = settings.value("ai_timeout", 60).toInt();

    QString featuresJson = settings.value("ai_features", "{}").toString();
    QJsonDocument featuresDoc = QJsonDocument::fromJson(featuresJson.toUtf8());
    if (featuresDoc.isObject()) {
        QJsonObject obj = featuresDoc.object();
        for (auto it = obj.begin(); it != obj.end(); ++it) {
            m_features[it.key()] = it.value().toBool();
        }
    }

    QString keysJson = settings.value("ai_keys", "[]").toString();
    QJsonDocument keysDoc = QJsonDocument::fromJson(keysJson.toUtf8());
    if (keysDoc.isArray()) {
        QJsonArray keysArray = keysDoc.array();
        for (int i = 0; i < keysArray.size(); ++i) {
            QJsonObject obj = keysArray[i].toObject();
            AIKeyConfig key;
            key.id = obj["id"].toString();
            key.name = obj["name"].toString();
            key.provider = obj["provider"].toString();
            key.model = obj["model"].toString();
            key.apiKey = decryptAPIKey(obj["encryptedKey"].toString());
            key.endpoint = obj["endpoint"].toString();
            key.isDefault = obj["isDefault"].toBool();
            key.createdAt = obj["createdAt"].toString();
            key.updatedAt = obj["updatedAt"].toString();
            m_keys.append(key);
        }
    }

    if (m_keys.isEmpty()) {
        AIKeyConfig defaultKey;
        defaultKey.id = generateId();
        defaultKey.name = "默认配置";
        defaultKey.provider = "aliyun";
        defaultKey.model = "qwen-turbo";
        defaultKey.apiKey = "";
        defaultKey.endpoint = "";
        defaultKey.isDefault = true;
        defaultKey.createdAt = QDateTime::currentDateTime().toString(Qt::ISODate);
        defaultKey.updatedAt = defaultKey.createdAt;
        m_keys.append(defaultKey);
        m_defaultKeyId = defaultKey.id;
        save();
    }

    m_defaultImageKeyId = settings.value("ai_default_image_key_id", "").toString();

    QString imageKeysJson = settings.value("ai_image_keys", "[]").toString();
    QJsonDocument imageKeysDoc = QJsonDocument::fromJson(imageKeysJson.toUtf8());
    if (imageKeysDoc.isArray()) {
        QJsonArray keysArray = imageKeysDoc.array();
        for (int i = 0; i < keysArray.size(); ++i) {
            QJsonObject obj = keysArray[i].toObject();
            AIImageConfig key;
            key.id = obj["id"].toString();
            key.name = obj["name"].toString();
            key.provider = obj["provider"].toString();
            key.model = obj["model"].toString();
            key.apiKey = decryptAPIKey(obj["encryptedKey"].toString());
            key.endpoint = obj["endpoint"].toString();
            key.isDefault = obj["isDefault"].toBool();
            key.createdAt = obj["createdAt"].toString();
            key.updatedAt = obj["updatedAt"].toString();
            m_imageKeys.append(key);
        }
    }

    if (m_imageKeys.isEmpty()) {
        AIImageConfig defaultKey;
        defaultKey.id = generateId();
        defaultKey.name = "默认图像配置";
        defaultKey.provider = "siliconflow";
        defaultKey.model = "black-forest-labs/FLUX.1-schnell";
        defaultKey.apiKey = "";
        defaultKey.endpoint = "";
        defaultKey.isDefault = true;
        defaultKey.createdAt = QDateTime::currentDateTime().toString(Qt::ISODate);
        defaultKey.updatedAt = defaultKey.createdAt;
        m_imageKeys.append(defaultKey);
        m_defaultImageKeyId = defaultKey.id;
        save();
    }
}

void AIConfig::save()
{
    QSettings settings("PonyWork", "WorkLog");

    settings.setValue("ai_default_model", m_defaultModel);
    settings.setValue("ai_default_key_id", m_defaultKeyId);
    settings.setValue("ai_timeout", m_timeout);

    QJsonObject featuresObj;
    for (auto it = m_features.begin(); it != m_features.end(); ++it) {
        featuresObj[it.key()] = it.value();
    }
    settings.setValue("ai_features", QJsonDocument(featuresObj).toJson(QJsonDocument::Compact));

    QJsonArray keysArray;
    for (const AIKeyConfig &key : m_keys) {
        QJsonObject obj;
        obj["id"] = key.id;
        obj["name"] = key.name;
        obj["provider"] = key.provider;
        obj["model"] = key.model;
        obj["encryptedKey"] = encryptAPIKey(key.apiKey);
        obj["endpoint"] = key.endpoint;
        obj["isDefault"] = key.isDefault;
        obj["createdAt"] = key.createdAt;
        obj["updatedAt"] = key.updatedAt;
        keysArray.append(obj);
    }
    settings.setValue("ai_keys", QJsonDocument(keysArray).toJson(QJsonDocument::Compact));

    settings.setValue("ai_default_image_key_id", m_defaultImageKeyId);

    QJsonArray imageKeysArray;
    for (const AIImageConfig &key : m_imageKeys) {
        QJsonObject obj;
        obj["id"] = key.id;
        obj["name"] = key.name;
        obj["provider"] = key.provider;
        obj["model"] = key.model;
        obj["encryptedKey"] = encryptAPIKey(key.apiKey);
        obj["endpoint"] = key.endpoint;
        obj["isDefault"] = key.isDefault;
        obj["createdAt"] = key.createdAt;
        obj["updatedAt"] = key.updatedAt;
        imageKeysArray.append(obj);
    }
    settings.setValue("ai_image_keys", QJsonDocument(imageKeysArray).toJson(QJsonDocument::Compact));
}

QList<AIKeyConfig> AIConfig::getAllKeys()
{
    return m_keys;
}

AIKeyConfig AIConfig::getKey(const QString &id)
{
    for (const AIKeyConfig &key : m_keys) {
        if (key.id == id) {
            return key;
        }
    }
    return AIKeyConfig();
}

AIKeyConfig AIConfig::getKeyByModel(const QString &model)
{
    for (const AIKeyConfig &key : m_keys) {
        if (key.model == model) {
            return key;
        }
    }
    return getDefaultKey();
}

AIKeyConfig AIConfig::getKeyByModelName(const QString &modelName)
{
    for (const AIKeyConfig &key : m_keys) {
        AIModelInfo modelInfo = getModelInfo(key.model);
        if (modelInfo.name == modelName) {
            return key;
        }
    }
    return getDefaultKey();
}

AIKeyConfig AIConfig::getDefaultKey()
{
    if (!m_defaultKeyId.isEmpty()) {
        for (const AIKeyConfig &key : m_keys) {
            if (key.id == m_defaultKeyId) {
                return key;
            }
        }
    }
    if (!m_keys.isEmpty()) {
        return m_keys.first();
    }
    return AIKeyConfig();
}

void AIConfig::addKey(const AIKeyConfig &config)
{
    AIKeyConfig newKey = config;
    newKey.id = generateId();
    newKey.createdAt = QDateTime::currentDateTime().toString(Qt::ISODate);
    newKey.updatedAt = newKey.createdAt;

    if (newKey.isDefault) {
        for (AIKeyConfig &key : m_keys) {
            key.isDefault = false;
        }
        m_defaultKeyId = newKey.id;
    }

    m_keys.append(newKey);
    save();
}

void AIConfig::updateKey(const AIKeyConfig &config)
{
    for (int i = 0; i < m_keys.size(); ++i) {
        if (m_keys[i].id == config.id) {
            if (config.isDefault) {
                for (int j = 0; j < m_keys.size(); ++j) {
                    m_keys[j].isDefault = (j == i);
                }
                m_defaultKeyId = config.id;
            }
            m_keys[i] = config;
            m_keys[i].updatedAt = QDateTime::currentDateTime().toString(Qt::ISODate);
            save();
            return;
        }
    }
}

void AIConfig::deleteKey(const QString &id)
{
    for (int i = 0; i < m_keys.size(); ++i) {
        if (m_keys[i].id == id) {
            m_keys.removeAt(i);
            if (m_defaultKeyId == id) {
                if (!m_keys.isEmpty()) {
                    m_keys.first().isDefault = true;
                    m_defaultKeyId = m_keys.first().id;
                } else {
                    m_defaultKeyId = "";
                }
            }
            save();
            return;
        }
    }
}

void AIConfig::setDefaultKey(const QString &id)
{
    m_defaultKeyId = id;
    for (AIKeyConfig &key : m_keys) {
        key.isDefault = (key.id == id);
    }
    save();
}

QString AIConfig::getDefaultModel()
{
    return m_defaultModel;
}

void AIConfig::setDefaultModel(const QString &model)
{
    m_defaultModel = model;
    save();
}

int AIConfig::getTimeout()
{
    return m_timeout;
}

void AIConfig::setTimeout(int timeout)
{
    m_timeout = timeout;
    save();
}

bool AIConfig::isEnabled(const QString &feature)
{
    return m_features.value(feature, false);
}

void AIConfig::setEnabled(const QString &feature, bool enabled)
{
    m_features[feature] = enabled;
    save();
}

QString AIConfig::encryptAPIKey(const QString &apiKey)
{
    if (apiKey.isEmpty()) {
        return "";
    }
    QByteArray key = hashKey("PonyWorkAI").toUtf8();
    QByteArray data = apiKey.toUtf8();
    QByteArray encrypted;
    for (int i = 0; i < data.size(); ++i) {
        encrypted.append(data.at(i) ^ key.at(i % key.size()));
    }
    return QString::fromUtf8(encrypted.toBase64());
}

QString AIConfig::decryptAPIKey(const QString &encryptedKey)
{
    if (encryptedKey.isEmpty()) {
        return "";
    }
    QByteArray key = hashKey("PonyWorkAI").toUtf8();
    QByteArray data = QByteArray::fromBase64(encryptedKey.toUtf8());
    QByteArray decrypted;
    for (int i = 0; i < data.size(); ++i) {
        decrypted.append(data.at(i) ^ key.at(i % key.size()));
    }
    return QString::fromUtf8(decrypted);
}

QString AIConfig::hashKey(const QString &input)
{
    return QCryptographicHash::hash(
        QByteArray("PonyWorkAI").append(QHostInfo::localHostName().toUtf8()).append(input.toUtf8()),
        QCryptographicHash::Sha256
    ).toHex();
}

QString AIConfig::generateId()
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

void AIConfig::initProvidersAndModels()
{
    m_providers = {
        {"openai", "OpenAI", "OpenAI"},
        {"anthropic", "Anthropic", "Claude"},
        {"google", "Google", "Gemini"},
        {"minimax", "MiniMax", "MiniMax"},
        {"aliyun", "Aliyun", "通义千问"},
        {"iflytek", "iFlytek", "讯飞星火"},
        {"siliconflow", "SiliconFlow", "硅基流动"},
        {"deepseek", "DeepSeek", "DeepSeek"},
        {"stability", "Stability AI", "Stability AI"},
        {"local", "Local", "本地模式"}
    };

    m_models = {
        {"gpt35", "gpt-3.5-turbo", "openai", "https://api.openai.com/v1/chat/completions"},
        {"gpt4", "gpt-4", "openai", "https://api.openai.com/v1/chat/completions"},
        {"gpt4o", "gpt-4o", "openai", "https://api.openai.com/v1/chat/completions"},
        {"gpt4turbo", "gpt-4-turbo", "openai", "https://api.openai.com/v1/chat/completions"},
        {"claude3haiku", "claude-3-haiku-20240307", "anthropic", "https://api.anthropic.com/v1/messages"},
        {"claude3sonnet", "claude-3-sonnet-20240229", "anthropic", "https://api.anthropic.com/v1/messages"},
        {"claude3opus", "claude-3-opus-20240229", "anthropic", "https://api.anthropic.com/v1/messages"},
        {"claude3.5sonnet", "claude-3-5-sonnet-20240620", "anthropic", "https://api.anthropic.com/v1/messages"},
        {"gemini-pro", "gemini-pro", "google", "https://generativelanguage.googleapis.com/v1beta/models/gemini-pro:generateContent"},
        {"gemini-1.5-pro", "gemini-1.5-pro", "google", "https://generativelanguage.googleapis.com/v1beta/models/gemini-1.5-pro:generateContent"},
        {"abab6.5s-chat", "abab6.5s-chat", "minimax", "https://api.minimax.chat/v1/text/chatcompletion_v2"},
        {"abab6.5g-chat", "abab6.5g-chat", "minimax", "https://api.minimax.chat/v1/text/chatcompletion_v2"},
        {"qwen-turbo", "qwen-turbo", "aliyun", "https://dashscope.aliyuncs.com/api/v1/services/aigc/text-generation/generation"},
        {"qwen-plus", "qwen-plus", "aliyun", "https://dashscope.aliyuncs.com/api/v1/services/aigc/text-generation/generation"},
        {"qwen-max", "qwen-max", "aliyun", "https://dashscope.aliyuncs.com/api/v1/services/aigc/text-generation/generation"},
        {"qwen-long", "qwen-long", "aliyun", "https://dashscope.aliyuncs.com/api/v1/services/aigc/text-generation/generation"},
        {"spark-v3.5", "generalv3.5", "iflytek", "https://spark-api.xfyun.com/v3.5/chat"},
        {"spark-v3", "generalv3", "iflytek", "https://spark-api.xfyun.com/v3.1/chat"},
        {"deepseek-v2", "deepseek-ai/DeepSeek-V2-Chat", "siliconflow", "https://api.siliconflow.cn/v1/chat/completions"},
        {"deepseek-v2.5", "deepseek-ai/DeepSeek-V2.5", "siliconflow", "https://api.siliconflow.cn/v1/chat/completions"},
        {"deepseek-v3", "deepseek-ai/DeepSeek-V3", "siliconflow", "https://api.siliconflow.cn/v1/chat/completions"},
        {"deepseek-v3.2", "deepseek-ai/DeepSeek-V3.2", "siliconflow", "https://api.siliconflow.cn/v1/chat/completions"},
        {"deepseek-v3.1", "deepseek-ai/DeepSeek-V3.1-Terminus", "siliconflow", "https://api.siliconflow.cn/v1/chat/completions"},
        {"deepseek-r1", "deepseek-ai/DeepSeek-R1", "siliconflow", "https://api.siliconflow.cn/v1/chat/completions"},
        {"deepseek-coder", "deepseek-ai/DeepSeek-Coder-V2", "siliconflow", "https://api.siliconflow.cn/v1/chat/completions"},
        {"qwen-coder", "qwen-coder-7b-instruct", "siliconflow", "https://api.siliconflow.cn/v1/chat/completions"},
        {"minimax-m2.5", "MiniMaxAI/MiniMax-M2.5", "siliconflow", "https://api.siliconflow.cn/v1/chat/completions"},
        {"minimax-m2.1", "MiniMaxAI/MiniMax-M2.1", "siliconflow", "https://api.siliconflow.cn/v1/chat/completions"},
        {"local", "local", "local", ""}
    };
}

QList<AIProviderInfo> AIConfig::getProviders()
{
    return m_providers;
}

AIProviderInfo AIConfig::getProviderInfo(const QString &providerId)
{
    for (const AIProviderInfo &provider : m_providers) {
        if (provider.id == providerId) {
            return provider;
        }
    }
    return AIProviderInfo();
}

QList<AIModelInfo> AIConfig::getAllModels()
{
    return m_models;
}

QList<AIModelInfo> AIConfig::getModelsByProvider(const QString &providerId)
{
    QList<AIModelInfo> result;
    for (const AIModelInfo &model : m_models) {
        if (model.provider == providerId) {
            result.append(model);
        }
    }
    return result;
}

AIModelInfo AIConfig::getModelInfo(const QString &modelId)
{
    for (const AIModelInfo &model : m_models) {
        if (model.id == modelId) {
            return model;
        }
    }
    return AIModelInfo();
}

QString AIConfig::maskAPIKey(const QString &apiKey)
{
    if (apiKey.length() <= 8) {
        return "****";
    }
    return apiKey.left(4) + "****" + apiKey.right(4);
}

QList<AIImageConfig> AIConfig::getAllImageKeys()
{
    return m_imageKeys;
}

AIImageConfig AIConfig::getImageKey(const QString &id)
{
    for (const AIImageConfig &key : m_imageKeys) {
        if (key.id == id) {
            return key;
        }
    }
    return AIImageConfig();
}

AIImageConfig AIConfig::getImageKeyByProvider(const QString &provider)
{
    for (const AIImageConfig &key : m_imageKeys) {
        if (key.provider == provider) {
            return key;
        }
    }
    return AIImageConfig();
}

AIImageConfig AIConfig::getDefaultImageKey()
{
    if (!m_defaultImageKeyId.isEmpty()) {
        for (const AIImageConfig &key : m_imageKeys) {
            if (key.id == m_defaultImageKeyId) {
                return key;
            }
        }
    }
    if (!m_imageKeys.isEmpty()) {
        return m_imageKeys.first();
    }
    return AIImageConfig();
}

void AIConfig::addImageKey(const AIImageConfig &config)
{
    AIImageConfig newKey = config;
    newKey.id = generateId();
    newKey.createdAt = QDateTime::currentDateTime().toString(Qt::ISODate);
    newKey.updatedAt = newKey.createdAt;

    if (newKey.isDefault) {
        for (AIImageConfig &key : m_imageKeys) {
            key.isDefault = false;
        }
        m_defaultImageKeyId = newKey.id;
    }

    m_imageKeys.append(newKey);
    save();
}

void AIConfig::updateImageKey(const AIImageConfig &config)
{
    for (int i = 0; i < m_imageKeys.size(); ++i) {
        if (m_imageKeys[i].id == config.id) {
            if (config.isDefault) {
                for (int j = 0; j < m_imageKeys.size(); ++j) {
                    m_imageKeys[j].isDefault = (j == i);
                }
                m_defaultImageKeyId = config.id;
            }
            m_imageKeys[i] = config;
            m_imageKeys[i].updatedAt = QDateTime::currentDateTime().toString(Qt::ISODate);
            save();
            return;
        }
    }
}

void AIConfig::deleteImageKey(const QString &id)
{
    for (int i = 0; i < m_imageKeys.size(); ++i) {
        if (m_imageKeys[i].id == id) {
            m_imageKeys.removeAt(i);
            if (m_defaultImageKeyId == id) {
                if (!m_imageKeys.isEmpty()) {
                    m_imageKeys.first().isDefault = true;
                    m_defaultImageKeyId = m_imageKeys.first().id;
                } else {
                    m_defaultImageKeyId = "";
                }
            }
            save();
            return;
        }
    }
}

void AIConfig::setDefaultImageKey(const QString &id)
{
    m_defaultImageKeyId = id;
    for (AIImageConfig &key : m_imageKeys) {
        key.isDefault = (key.id == id);
    }
    save();
}

QList<AIImageModelInfo> AIConfig::getAllImageModels()
{
    return m_imageModels;
}

QList<AIImageModelInfo> AIConfig::getImageModelsByProvider(const QString &providerId)
{
    QList<AIImageModelInfo> result;
    for (const AIImageModelInfo &model : m_imageModels) {
        if (model.provider == providerId) {
            result.append(model);
        }
    }
    return result;
}

AIImageModelInfo AIConfig::getImageModelInfo(const QString &modelId)
{
    for (const AIImageModelInfo &model : m_imageModels) {
        if (model.id == modelId) {
            return model;
        }
    }
    return AIImageModelInfo();
}

void AIConfig::initImageProvidersAndModels()
{
    m_imageModels = {
        {"dalle3", "DALL-E 3", "openai", "https://api.openai.com/v1/images/generations"},
        {"stable-diffusion", "Stable Diffusion", "stability", "https://api.stability.ai/v1/generation/stable-diffusion-xl-1024-v1-0/text-to-image"},
        {"black-forest-labs/flux.1-schnell", "Flux Schnell", "siliconflow", "https://api.siliconflow.cn/v1/images/generations"},
        {"black-forest-labs/flux.1-pro", "Flux Pro", "siliconflow", "https://api.siliconflow.cn/v1/images/generations"},
        {"sd3-medium", "Stable Diffusion 3 Medium", "siliconflow", "https://api.siliconflow.cn/v1/images/generations"},
        {"sd3-large", "Stable Diffusion 3 Large", "siliconflow", "https://api.siliconflow.cn/v1/images/generations"}
    };
}
