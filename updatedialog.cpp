#include "updatedialog.h"
#include <QStyle>
#include <QIcon>
#include <QScrollArea>
#include <QTextEdit>
#include <QApplication>

UpdateDialog::UpdateDialog(const UpdateInfo &info, QWidget *parent)
    : QDialog(parent)
{
    setupUI(info);
    
    setWindowTitle("发现新版本");
    setMinimumWidth(500);
    setMaximumHeight(600);
}

void UpdateDialog::setupUI(const UpdateInfo &info)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    QHBoxLayout *headerLayout = new QHBoxLayout();
    
    QLabel *iconLabel = new QLabel(this);
    iconLabel->setPixmap(style()->standardIcon(QStyle::SP_MessageBoxInformation).pixmap(64, 64));
    headerLayout->addWidget(iconLabel);
    
    QVBoxLayout *infoLayout = new QVBoxLayout();
    
    versionLabel = new QLabel(QString("<b>新版本可用</b><br>当前版本: %1<br>最新版本: v%2")
                                     .arg(qApp->applicationVersion(), info.version), this);
    versionLabel->setStyleSheet("font-size: 14px;");
    
    dateLabel = new QLabel(QString("发布日期: %1").arg(info.releaseDate.left(10)), this);
    dateLabel->setStyleSheet("color: #666;");
    
    if (info.fileSize > 0) {
        QString sizeStr;
        qint64 size = info.fileSize;
        if (size >= 1024 * 1024 * 1024) {
            sizeStr = QString::number(size / (1024.0 * 1024.0 * 1024.0), 'f', 2) + " GB";
        } else if (size >= 1024 * 1024) {
            sizeStr = QString::number(size / (1024.0 * 1024.0), 'f', 2) + " MB";
        } else if (size >= 1024) {
            sizeStr = QString::number(size / 1024.0, 'f', 2) + " KB";
        } else {
            sizeStr = QString::number(size) + " B";
        }
        sizeLabel = new QLabel(QString("更新大小: %1").arg(sizeStr), this);
        sizeLabel->setStyleSheet("color: #666;");
        infoLayout->addWidget(versionLabel);
        infoLayout->addWidget(dateLabel);
        infoLayout->addWidget(sizeLabel);
    } else {
        infoLayout->addWidget(versionLabel);
        infoLayout->addWidget(dateLabel);
    }
    
    headerLayout->addLayout(infoLayout);
    headerLayout->addStretch();
    mainLayout->addLayout(headerLayout);
    
    mainLayout->addSpacing(20);
    
    QLabel *changelogTitle = new QLabel("<b>更新内容:</b>", this);
    mainLayout->addWidget(changelogTitle);
    
    QTextEdit *changelogEdit = new QTextEdit(this);
    changelogEdit->setReadOnly(true);
    changelogEdit->setMaximumHeight(200);
    changelogEdit->setHtml(info.changelog);
    changelogEdit->setStyleSheet("background-color: #f5f5f5; border: 1px solid #ddd; border-radius: 4px; padding: 10px;");
    mainLayout->addWidget(changelogEdit);
    
    mainLayout->addSpacing(20);
    
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    skipButton = new QPushButton("忽略此版本", this);
    skipButton->setStyleSheet("QPushButton { padding: 8px 20px; }");
    connect(skipButton, &QPushButton::clicked, this, &UpdateDialog::onSkipThisVersion);
    buttonLayout->addWidget(skipButton);
    
    remindLaterButton = new QPushButton("稍后提醒", this);
    remindLaterButton->setStyleSheet("QPushButton { padding: 8px 20px; }");
    connect(remindLaterButton, &QPushButton::clicked, this, &UpdateDialog::onRemindLater);
    buttonLayout->addWidget(remindLaterButton);
    
    updateNowButton = new QPushButton("立即更新", this);
    updateNowButton->setStyleSheet("QPushButton { background-color: #1976d2; color: white; padding: 8px 25px; border-radius: 4px; font-weight: bold; } QPushButton:hover { background-color: #1565c0; }");
    updateNowButton->setDefault(true);
    connect(updateNowButton, &QPushButton::clicked, this, &UpdateDialog::onUpdateNow);
    buttonLayout->addWidget(updateNowButton);
    
    mainLayout->addLayout(buttonLayout);
}

void UpdateDialog::onUpdateNow()
{
    emit updateNow();
    accept();
}

void UpdateDialog::onRemindLater()
{
    emit remindLater();
    accept();
}

void UpdateDialog::onSkipThisVersion()
{
    emit skipThisVersion();
    accept();
}
