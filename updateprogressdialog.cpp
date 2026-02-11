#include "updateprogressdialog.h"
#include <QDateTime>
#include <QStyle>
#include <QScrollBar>
#include <QMessageBox>

UpdateProgressDialog::UpdateProgressDialog(QWidget *parent)
    : QDialog(parent)
    , m_bytesReceived(0)
    , m_bytesTotal(0)
    , m_startTime(0)
    , m_downloading(false)
{
    setupUI();
    
    m_estimationTimer = new QTimer(this);
    connect(m_estimationTimer, &QTimer::timeout, this, &UpdateProgressDialog::updateTimeEstimation);
    
    setWindowTitle("正在更新");
    setMinimumWidth(500);
    setMaximumWidth(600);
    setModal(true);
}

void UpdateProgressDialog::setUpdateManager(UpdateManager *manager)
{
    connect(manager, &UpdateManager::downloadProgress, this, &UpdateProgressDialog::onDownloadProgress);
    connect(manager, &UpdateManager::downloadFinished, this, &UpdateProgressDialog::onDownloadFinished);
    connect(manager, &UpdateManager::downloadFailed, this, &UpdateProgressDialog::onDownloadFailed);
    connect(manager, &UpdateManager::extractProgress, this, &UpdateProgressDialog::onExtractProgress);
    connect(manager, &UpdateManager::extractFinished, this, &UpdateProgressDialog::onExtractFinished);
    connect(manager, &UpdateManager::extractFailed, this, &UpdateProgressDialog::onExtractFailed);
    connect(manager, &UpdateManager::installProgress, this, &UpdateProgressDialog::onInstallProgress);
    connect(manager, &UpdateManager::installFinished, this, &UpdateProgressDialog::onInstallFinished);
    connect(manager, &UpdateManager::installFailed, this, &UpdateProgressDialog::onInstallFailed);
    connect(manager, &UpdateManager::logMessage, this, &UpdateProgressDialog::onLogMessage);
}

void UpdateProgressDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    QHBoxLayout *headerLayout = new QHBoxLayout();
    
    QLabel *iconLabel = new QLabel(this);
    iconLabel->setPixmap(style()->standardIcon(QStyle::SP_ComputerIcon).pixmap(48, 48));
    headerLayout->addWidget(iconLabel);
    
    QVBoxLayout *infoLayout = new QVBoxLayout();
    
    statusLabel = new QLabel("准备更新...", this);
    statusLabel->setStyleSheet("font-size: 14px; font-weight: bold;");
    
    progressLabel = new QLabel("", this);
    
    timeLabel = new QLabel("", this);
    timeLabel->setStyleSheet("color: #666;");
    
    sizeLabel = new QLabel("", this);
    sizeLabel->setStyleSheet("color: #666;");
    
    infoLayout->addWidget(statusLabel);
    infoLayout->addWidget(progressLabel);
    infoLayout->addWidget(timeLabel);
    infoLayout->addWidget(sizeLabel);
    
    headerLayout->addLayout(infoLayout);
    headerLayout->addStretch();
    mainLayout->addLayout(headerLayout);
    
    mainLayout->addSpacing(15);
    
    progressBar = new QProgressBar(this);
    progressBar->setMinimum(0);
    progressBar->setMaximum(100);
    progressBar->setValue(0);
    progressBar->setTextVisible(true);
    mainLayout->addWidget(progressBar);
    
    mainLayout->addSpacing(10);
    
    QLabel *logTitle = new QLabel("<b>详细日志:</b>", this);
    mainLayout->addWidget(logTitle);
    
    logEdit = new QTextEdit(this);
    logEdit->setReadOnly(true);
    logEdit->setMaximumHeight(200);
    logEdit->setStyleSheet("background-color: #f5f5f5; border: 1px solid #ddd; border-radius: 4px; padding: 8px; font-family: Consolas, monospace; font-size: 9pt;");
    mainLayout->addWidget(logEdit);
    
    mainLayout->addSpacing(15);
    
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    cancelButton = new QPushButton("取消", this);
    cancelButton->setStyleSheet("QPushButton { padding: 8px 25px; }");
    connect(cancelButton, &QPushButton::clicked, this, &UpdateProgressDialog::onCancel);
    buttonLayout->addWidget(cancelButton);
    
    mainLayout->addLayout(buttonLayout);
}

void UpdateProgressDialog::updateStatus(const QString &status)
{
    statusLabel->setText(status);
}

void UpdateProgressDialog::onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    m_bytesReceived = bytesReceived;
    m_bytesTotal = bytesTotal;
    
    if (bytesTotal > 0) {
        int percent = static_cast<int>((bytesReceived * 100) / bytesTotal);
        progressBar->setValue(percent);
        progressLabel->setText(QString("进度: %1%").arg(percent));
    }
    
    sizeLabel->setText(QString("已下载: %1 / %2").arg(formatFileSize(bytesReceived)).arg(formatFileSize(bytesTotal)));
}

void UpdateProgressDialog::onDownloadFinished(const QString &filePath)
{
    m_downloading = false;
    m_estimationTimer->stop();
    
    progressBar->setValue(100);
    updateStatus("下载完成");
    progressLabel->setText("进度: 100%");
    timeLabel->clear();
    
    Q_UNUSED(filePath);
}

void UpdateProgressDialog::onDownloadFailed(const QString &error)
{
    m_downloading = false;
    m_estimationTimer->stop();
    
    updateStatus("下载失败");
    progressLabel->setText("下载失败");
    
    QMessageBox::critical(this, "更新失败", error);
    reject();
}

void UpdateProgressDialog::onExtractProgress(int percent)
{
    progressBar->setValue(percent);
    updateStatus("正在解压...");
    progressLabel->setText(QString("进度: %1%").arg(percent));
    sizeLabel->clear();
    timeLabel->clear();
}

void UpdateProgressDialog::onExtractFinished(const QString &extractPath)
{
    progressBar->setValue(100);
    updateStatus("解压完成");
    progressLabel->setText("进度: 100%");
    
    Q_UNUSED(extractPath);
}

void UpdateProgressDialog::onExtractFailed(const QString &error)
{
    updateStatus("解压失败");
    progressLabel->setText("解压失败");
    
    QMessageBox::critical(this, "更新失败", error);
    reject();
}

void UpdateProgressDialog::onInstallProgress(int percent)
{
    progressBar->setValue(percent);
    updateStatus("正在安装...");
    progressLabel->setText(QString("进度: %1%").arg(percent));
}

void UpdateProgressDialog::onInstallFinished()
{
    progressBar->setValue(100);
    updateStatus("安装完成");
    progressLabel->setText("进度: 100%");
    cancelButton->setEnabled(false);
    
    QTimer::singleShot(1500, this, [this]() {
        accept();
    });
}

void UpdateProgressDialog::onInstallFailed(const QString &error)
{
    updateStatus("安装失败");
    progressLabel->setText("安装失败");
    
    QMessageBox::critical(this, "更新失败", error);
    reject();
}

void UpdateProgressDialog::onLogMessage(const QString &message)
{
    logEdit->append(message);
    
    QScrollBar *bar = logEdit->verticalScrollBar();
    bar->setValue(bar->maximum());
}

void UpdateProgressDialog::onCancel()
{
    if (QMessageBox::question(this, "确认取消", "确定要取消更新吗？", 
                               QMessageBox::Yes | QMessageBox::No, 
                               QMessageBox::No) == QMessageBox::Yes) {
        emit cancelRequested();
        reject();
    }
}

void UpdateProgressDialog::updateTimeEstimation()
{
    if (!m_downloading || m_bytesReceived <= 0 || m_bytesTotal <= 0) {
        return;
    }
    
    qint64 elapsed = QDateTime::currentMSecsSinceEpoch() - m_startTime;
    if (elapsed > 0) {
        qint64 remainingBytes = m_bytesTotal - m_bytesReceived;
        qint64 bytesPerMs = m_bytesReceived / elapsed;
        if (bytesPerMs > 0) {
            qint64 remainingMs = remainingBytes / bytesPerMs;
            qint64 remainingSeconds = remainingMs / 1000;
            
            timeLabel->setText(QString("预计剩余: %1").arg(formatTime(remainingSeconds)));
        }
    }
}

QString UpdateProgressDialog::formatTime(qint64 seconds)
{
    if (seconds < 60) {
        return QString("%1 秒").arg(seconds);
    } else if (seconds < 3600) {
        return QString("%1 分 %2 秒").arg(seconds / 60).arg(seconds % 60);
    } else {
        return QString("%1 小时 %2 分").arg(seconds / 3600).arg((seconds % 3600) / 60);
    }
}

QString UpdateProgressDialog::formatFileSize(qint64 bytes)
{
    const qint64 KB = 1024;
    const qint64 MB = 1024 * KB;
    const qint64 GB = 1024 * MB;
    
    if (bytes >= GB) {
        return QString::number(bytes / (double)GB, 'f', 2) + " GB";
    } else if (bytes >= MB) {
        return QString::number(bytes / (double)MB, 'f', 2) + " MB";
    } else if (bytes >= KB) {
        return QString::number(bytes / (double)KB, 'f', 2) + " KB";
    } else {
        return QString::number(bytes) + " B";
    }
}
