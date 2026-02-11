#include "logger.h"
#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QTextStream>
#include <QTimer>
#include <QDebug>

Logger* Logger::s_instance = nullptr;

Logger::Logger(QObject *parent)
    : QObject(parent)
    , m_logFileName("PonyWork-update.log")
    , m_maxFileSize(10 * 1024 * 1024)
    , m_maxBackupCount(5)
    , m_enableAsync(true)
    , m_isProcessing(false)
{
    m_logDirectory = QCoreApplication::applicationDirPath();
}

Logger::~Logger()
{
    QMutexLocker locker(&m_mutex);
    if (m_logFile.isOpen()) {
        m_logFile.flush();
        m_logFile.close();
    }
}

Logger* Logger::instance()
{
    if (!s_instance) {
        s_instance = new Logger();
    }
    return s_instance;
}

void Logger::setLogDirectory(const QString &dir)
{
    QMutexLocker locker(&m_mutex);
    m_logDirectory = dir;
    if (m_logFile.isOpen()) {
        m_logFile.close();
    }
}

void Logger::setLogFileName(const QString &name)
{
    QMutexLocker locker(&m_mutex);
    m_logFileName = name;
    if (m_logFile.isOpen()) {
        m_logFile.close();
    }
}

void Logger::setMaxFileSize(qint64 size)
{
    QMutexLocker locker(&m_mutex);
    m_maxFileSize = size;
}

void Logger::setMaxBackupCount(int count)
{
    QMutexLocker locker(&m_mutex);
    m_maxBackupCount = qMax(1, count);
}

void Logger::setEnableAsync(bool enable)
{
    QMutexLocker locker(&m_mutex);
    m_enableAsync = enable;
}

void Logger::log(const QString &message)
{
    log("INFO", message);
}

void Logger::log(const QString &level, const QString &message)
{
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
    QString formattedMessage = QString("[%1] [%2] %3").arg(timestamp, level, message);

    QMutexLocker locker(&m_mutex);

    if (m_enableAsync) {
        m_pendingLogs.append(formattedMessage);
        if (!m_isProcessing) {
            m_isProcessing = true;
            QTimer::singleShot(0, this, &Logger::processPendingLogs);
        }
    } else {
        rotateLogsIfNeeded();
        writeLogToFile(formattedMessage);
    }

    emit logWritten(formattedMessage);
    qDebug() << formattedMessage;
}

void Logger::processPendingLogs()
{
    QStringList logs;

    {
        QMutexLocker locker(&m_mutex);
        logs = m_pendingLogs;
        m_pendingLogs.clear();
    }

    if (!logs.isEmpty()) {
        rotateLogsIfNeeded();

        for (const QString &log : logs) {
            writeLogToFile(log);
        }
    }

    {
        QMutexLocker locker(&m_mutex);
        if (m_pendingLogs.isEmpty()) {
            m_isProcessing = false;
        } else {
            QTimer::singleShot(0, this, &Logger::processPendingLogs);
        }
    }
}

void Logger::rotateLogsIfNeeded()
{
    QString logPath = getLogFilePath();
    QFile logFile(logPath);

    if (logFile.exists() && logFile.size() >= m_maxFileSize) {
        performLogRotation();
    }
}

void Logger::performLogRotation()
{
    QDir logDir(m_logDirectory);

    for (int i = m_maxBackupCount - 1; i > 0; --i) {
        QString currentFile = getBackupFileName(i);
        QString nextFile = getBackupFileName(i + 1);

        if (logDir.exists(currentFile)) {
            if (logDir.exists(nextFile)) {
                logDir.remove(nextFile);
            }
            logDir.rename(currentFile, nextFile);
        }
    }

    QString mainLogPath = getLogFilePath();
    QString firstBackupPath = getBackupFileName(1);

    if (logDir.exists(mainLogPath)) {
        if (logDir.exists(firstBackupPath)) {
            logDir.remove(firstBackupPath);
        }
        logDir.rename(mainLogPath, firstBackupPath);
        emit logRotated(mainLogPath, firstBackupPath);
    }

    if (m_logFile.isOpen()) {
        m_logFile.close();
    }
}

QString Logger::getBackupFileName(int index) const
{
    QFileInfo fileInfo(m_logFileName);
    QString baseName = fileInfo.baseName();
    QString suffix = fileInfo.suffix();
    if (suffix.isEmpty()) {
        return QString("%1.%2").arg(baseName).arg(index);
    }
    return QString("%1.%2.%3").arg(baseName).arg(index).arg(suffix);
}

void Logger::writeLogToFile(const QString &message)
{
    QString logPath = getLogFilePath();

    if (!m_logFile.isOpen() || m_logFile.fileName() != logPath) {
        if (m_logFile.isOpen()) {
            m_logFile.close();
        }

        QDir().mkpath(m_logDirectory);
        m_logFile.setFileName(logPath);

        if (!m_logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
            qWarning() << "Failed to open log file:" << logPath;
            return;
        }
    }

    QTextStream out(&m_logFile);
    out << message << "\n";
    out.flush();
}

QString Logger::getLogFilePath() const
{
    return QDir(m_logDirectory).filePath(m_logFileName);
}

QStringList Logger::getBackupLogFiles() const
{
    QStringList files;
    QDir logDir(m_logDirectory);

    for (int i = 1; i <= m_maxBackupCount; ++i) {
        QString backupFile = getBackupFileName(i);
        QString fullPath = logDir.filePath(backupFile);
        if (QFile::exists(fullPath)) {
            files.append(fullPath);
        }
    }

    return files;
}
