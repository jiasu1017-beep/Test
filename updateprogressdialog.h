#ifndef UPDATEPROGRESSDIALOG_H
#define UPDATEPROGRESSDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTimer>
#include <QTextEdit>
#include "updatemanager.h"

class UpdateProgressDialog : public QDialog
{
    Q_OBJECT
public:
    explicit UpdateProgressDialog(QWidget *parent = nullptr);
    
    void setUpdateManager(UpdateManager *manager);
    
public slots:
    void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void onDownloadFinished(const QString &filePath);
    void onDownloadFailed(const QString &error);
    void onExtractProgress(int percent);
    void onExtractFinished(const QString &extractPath);
    void onExtractFailed(const QString &error);
    void onInstallProgress(int percent);
    void onInstallFinished();
    void onInstallFailed(const QString &error);
    void onLogMessage(const QString &message);
    
private slots:
    void onCancel();
    void updateTimeEstimation();
    
signals:
    void cancelRequested();
    
private:
    void setupUI();
    void updateStatus(const QString &status);
    QString formatTime(qint64 seconds);
    QString formatFileSize(qint64 bytes);
    
    QLabel *statusLabel;
    QLabel *progressLabel;
    QLabel *timeLabel;
    QLabel *sizeLabel;
    QProgressBar *progressBar;
    QPushButton *cancelButton;
    QTextEdit *logEdit;
    
    qint64 m_bytesReceived;
    qint64 m_bytesTotal;
    qint64 m_startTime;
    QTimer *m_estimationTimer;
    bool m_downloading;
};

#endif
