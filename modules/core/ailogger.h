#ifndef AILOGGER_H
#define AILOGGER_H

#include <QString>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QCoreApplication>

/**
 * @brief AI日志工具类 - 统一管理AI相关日志输出
 * @details 提供结构化的日志记录功能，包括请求、响应、错误等信息
 *          支持文本AI和图像AI的验证与调用日志
 */
class AILogger {
public:
    /**
     * @brief 日志类型枚举
     */
    enum LogType {
        TextVerify,    // 文本AI验证
        ImageVerify,   // 图像AI验证
        TextCall,      // 文本AI调用
        ImageCall      // 图像AI调用
    };

    /**
     * @brief 初始化日志文件
     * @param type 日志类型
     * @return 日志文件路径
     */
    static QString initLogFile(LogType type) {
        QString prefix;
        switch (type) {
            case TextVerify: prefix = "ai_text_verify"; break;
            case ImageVerify: prefix = "ai_image_verify"; break;
            case TextCall: prefix = "ai_text_call"; break;
            case ImageCall: prefix = "ai_image_call"; break;
        }

        QString logDir = QCoreApplication::applicationDirPath() + "/logs";
        QDir dir(logDir);
        if (!dir.exists()) {
            dir.mkpath(logDir);
        }
        return logDir + "/" + prefix + "_" + QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss") + ".log";
    }

    /**
     * @brief 写入日志内容
     * @param fileName 日志文件路径
     * @param content 日志内容
     */
    static void writeLog(const QString &fileName, const QString &content) {
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out.setCodec("UTF-8");
            out << content;
            file.close();
        }
    }

    /**
     * @brief 追加日志内容
     * @param fileName 日志文件路径
     * @param content 日志内容
     */
    static void appendLog(const QString &fileName, const QString &content) {
        QFile file(fileName);
        if (file.open(QIODevice::Append | QIODevice::Text)) {
            QTextStream out(&file);
            out.setCodec("UTF-8");
            out << content;
            file.close();
        }
    }

    /**
     * @brief 记录验证请求日志
     * @param fileName 日志文件路径
     * @param provider 提供商ID
     * @param model 模型ID
     * @param endpoint API端点
     * @param requestJson 请求JSON
     */
    static void logVerifyRequest(const QString &fileName, const QString &provider,
                                 const QString &model, const QString &endpoint,
                                 const QJsonObject &requestJson) {
        QString log;
        QTextStream(&log) << "=== AI Verify Request ===" << "\n"
                         << "Time: " << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << "\n"
                         << "Provider: " << provider << "\n"
                         << "Model: " << model << "\n"
                         << "Endpoint: " << endpoint << "\n"
                         << "Request JSON:\n" << QJsonDocument(requestJson).toJson(QJsonDocument::Indented) << "\n\n";
        writeLog(fileName, log);
    }

    /**
     * @brief 记录验证响应日志
     * @param fileName 日志文件路径
     * @param statusCode HTTP状态码
     * @param responseJson 响应JSON
     */
    static void logVerifyResponse(const QString &fileName, int statusCode, const QJsonObject &responseJson) {
        QString log;
        QTextStream(&log) << "\n=== AI Verify Response ===" << "\n"
                         << "Status Code: " << statusCode << "\n"
                         << "Response:\n" << QJsonDocument(responseJson).toJson(QJsonDocument::Indented) << "\n\n";
        appendLog(fileName, log);
    }

    /**
     * @brief 记录验证错误日志
     * @param fileName 日志文件路径
     * @param error 错误信息
     * @param responseData 响应数据
     */
    static void logVerifyError(const QString &fileName, const QString &error, const QString &responseData) {
        QString log;
        QTextStream(&log) << "\n=== AI Verify Error ===" << "\n"
                         << "Error: " << error << "\n"
                         << "Response:\n" << responseData << "\n\n";
        appendLog(fileName, log);
    }

    /**
     * @brief 记录验证成功日志
     * @param fileName 日志文件路径
     * @param message 成功消息
     */
    static void logVerifySuccess(const QString &fileName, const QString &message) {
        QString log;
        QTextStream(&log) << "\n=== AI Verify Success ===" << "\n"
                         << "Message: " << message << "\n\n";
        appendLog(fileName, log);
    }
};

#endif // AILOGGER_H
