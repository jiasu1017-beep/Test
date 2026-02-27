#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QString>
#include <QFile>
#include <QMutex>

class Logger : public QObject
{
    Q_OBJECT

public:
    explicit Logger(QObject *parent = nullptr);
    ~Logger();

    static Logger* instance();

    void setLogDirectory(const QString &dir);
    void setLogFileName(const QString &name);
    void setMaxFileSize(qint64 size);
    void setMaxBackupCount(int count);
    void setEnableAsync(bool enable);

    void log(const QString &message);
    void log(const QString &level, const QString &message);

    QString getLogFilePath() const;
    QStringList getBackupLogFiles() const;

signals:
    void logWritten(const QString &message);
    void logRotated(const QString &oldFile, const QString &newFile);

private slots:
    void processPendingLogs();

private:
    void rotateLogsIfNeeded();
    void performLogRotation();
    QString getBackupFileName(int index) const;
    void writeLogToFile(const QString &message);

    static Logger* s_instance;

    QString m_logDirectory;
    QString m_logFileName;
    qint64 m_maxFileSize;
    int m_maxBackupCount;
    bool m_enableAsync;

    QFile m_logFile;
    QMutex m_mutex;
    QStringList m_pendingLogs;
    bool m_isProcessing;
};

#endif
