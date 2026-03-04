#ifndef AICONFIG_H
#define AICONFIG_H

#include <QString>
#include <QList>
#include <QMap>

struct AIKeyConfig {
    QString id;
    QString name;
    QString provider;
    QString model;
    QString apiKey;
    QString endpoint;
    bool isDefault;
    QString createdAt;
    QString updatedAt;
};

struct AIModelInfo {
    QString id;
    QString name;
    QString provider;
    QString defaultEndpoint;
};

struct AIProviderInfo {
    QString id;
    QString name;
    QString displayName;
};

class AIConfig
{
public:
    static AIConfig& instance();

    QList<AIKeyConfig> getAllKeys();
    AIKeyConfig getKey(const QString &id);
    AIKeyConfig getKeyByModel(const QString &model);
    AIKeyConfig getKeyByModelName(const QString &modelName);
    AIKeyConfig getDefaultKey();

    void addKey(const AIKeyConfig &config);
    void updateKey(const AIKeyConfig &config);
    void deleteKey(const QString &id);

    void setDefaultKey(const QString &id);
    QString getDefaultModel();
    void setDefaultModel(const QString &model);

    int getTimeout();
    void setTimeout(int timeout);

    bool isEnabled(const QString &feature);
    void setEnabled(const QString &feature, bool enabled);

    QString encryptAPIKey(const QString &apiKey);
    QString decryptAPIKey(const QString &encryptedKey);

    QList<AIProviderInfo> getProviders();
    AIProviderInfo getProviderInfo(const QString &providerId);
    QList<AIModelInfo> getAllModels();
    QList<AIModelInfo> getModelsByProvider(const QString &providerId);
    AIModelInfo getModelInfo(const QString &modelId);

    QString maskAPIKey(const QString &apiKey);

private:
    AIConfig();
    ~AIConfig();
    AIConfig(const AIConfig&) = delete;
    AIConfig& operator=(const AIConfig&) = delete;

    void load();
    void save();

    QString hashKey(const QString &input);
    QString generateId();

    QList<AIKeyConfig> m_keys;
    QMap<QString, bool> m_features;
    QString m_defaultModel;
    QString m_defaultKeyId;
    int m_timeout;

    QList<AIProviderInfo> m_providers;
    QList<AIModelInfo> m_models;
    void initProvidersAndModels();
};

#endif // AICONFIG_H
