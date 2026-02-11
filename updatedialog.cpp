#include "updatedialog.h"
#include <QStyle>
#include <QIcon>
#include <QScrollArea>
#include <QTextEdit>
#include <QApplication>
#include <QMessageBox>

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
    
    versionLabel = new QLabel(QString("<h2>发现新版本！</h2><p style='font-size: 14px; color: #333;'><b>当前版本:</b> %1<br><b>最新版本:</b> v%2</p>")
                                     .arg(qApp->applicationVersion(), info.version), this);
    versionLabel->setStyleSheet("font-size: 14px;");
    
    dateLabel = new QLabel(QString("<p style='color: #666;'>📅 发布日期: %1</p>").arg(info.releaseDate.left(10)), this);
    
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
        sizeLabel = new QLabel(QString("<p style='color: #666;'>📦 更新大小: %1</p>").arg(sizeStr), this);
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
    
    QLabel *changelogTitle = new QLabel("<h3>📋 更新内容</h3>", this);
    changelogTitle->setStyleSheet("border-bottom: 2px solid #1976d2; padding-bottom: 10px;");
    mainLayout->addWidget(changelogTitle);
    
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setMaximumHeight(280);
    scrollArea->setStyleSheet("QScrollArea { border: 1px solid #e0e0e0; border-radius: 6px; background-color: #fafafa; }");
    
    QWidget *scrollContent = new QWidget();
    QVBoxLayout *scrollLayout = new QVBoxLayout(scrollContent);
    scrollLayout->setSpacing(12);
    scrollLayout->setContentsMargins(12, 12, 12, 12);
    
    QStringList changelogLines = info.changelog.split("\n", Qt::SkipEmptyParts);
    
    QString currentSection = "";
    QStringList newFeatures;
    QStringList improvements;
    QStringList bugFixes;
    QStringList performance;
    
    for (const QString &line : changelogLines) {
        QString trimmed = line.trimmed();
        if (trimmed.startsWith("- ")) {
            QString content = trimmed.mid(2).trimmed();
            if (currentSection == "新增功能" || currentSection.contains("新功能")) {
                newFeatures << content;
            } else if (currentSection == "功能改进" || currentSection.contains("改进")) {
                improvements << content;
            } else if (currentSection == "问题修复" || currentSection.contains("修复")) {
                bugFixes << content;
            } else if (currentSection == "性能优化" || currentSection.contains("性能")) {
                performance << content;
            } else {
                improvements << content;
            }
        } else if (!trimmed.isEmpty()) {
            currentSection = trimmed;
        }
    }
    
    auto addSection = [&](const QString &title, const QStringList &items, const QString &color, const QString &icon) {
        if (items.isEmpty()) return;
        
        QWidget *sectionWidget = new QWidget();
        sectionWidget->setStyleSheet("background-color: white; border-radius: 6px; padding: 8px;");
        QVBoxLayout *sectionLayout = new QVBoxLayout(sectionWidget);
        sectionLayout->setSpacing(6);
        sectionLayout->setContentsMargins(10, 10, 10, 10);
        
        QLabel *sectionTitle = new QLabel(QString("<span style='color: %1; font-weight: bold; font-size: 14px;'>%2 %3</span>").arg(color, icon, title), sectionWidget);
        sectionLayout->addWidget(sectionTitle);
        
        for (const QString &item : items) {
            QLabel *itemLabel = new QLabel(QString("<span style='color: #444; font-size: 13px;'>• %1</span>").arg(item), sectionWidget);
            itemLabel->setWordWrap(true);
            sectionLayout->addWidget(itemLabel);
        }
        
        scrollLayout->addWidget(sectionWidget);
    };
    
    addSection("新增功能", newFeatures, "#2e7d32", "✨");
    addSection("功能改进", improvements, "#1976d2", "🔧");
    addSection("问题修复", bugFixes, "#d32f2f", "🐛");
    addSection("性能优化", performance, "#f57c00", "⚡");
    
    if (newFeatures.isEmpty() && improvements.isEmpty() && bugFixes.isEmpty() && performance.isEmpty()) {
        QString changelogHtml = info.changelog;
        changelogHtml.replace("\n", "<br>");
        QLabel *fallbackLabel = new QLabel("<span style='color: #444; font-size: 13px;'>" + changelogHtml + "</span>", scrollContent);
        fallbackLabel->setWordWrap(true);
        fallbackLabel->setStyleSheet("background-color: white; border-radius: 6px; padding: 12px;");
        scrollLayout->addWidget(fallbackLabel);
    }
    
    scrollLayout->addStretch();
    scrollArea->setWidget(scrollContent);
    mainLayout->addWidget(scrollArea);
    
    mainLayout->addSpacing(20);
    
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    skipButton = new QPushButton("忽略此版本", this);
    skipButton->setStyleSheet("QPushButton { padding: 10px 24px; border: 1px solid #ccc; border-radius: 6px; background-color: white; color: #666; } QPushButton:hover { background-color: #f5f5f5; }");
    connect(skipButton, &QPushButton::clicked, this, &UpdateDialog::onSkipThisVersion);
    buttonLayout->addWidget(skipButton);
    
    remindLaterButton = new QPushButton("稍后提醒", this);
    remindLaterButton->setStyleSheet("QPushButton { padding: 10px 24px; border: 1px solid #1976d2; border-radius: 6px; background-color: white; color: #1976d2; } QPushButton:hover { background-color: #e3f2fd; }");
    connect(remindLaterButton, &QPushButton::clicked, this, &UpdateDialog::onRemindLater);
    buttonLayout->addWidget(remindLaterButton);
    
    updateNowButton = new QPushButton("立即更新", this);
    updateNowButton->setStyleSheet("QPushButton { background-color: #1976d2; color: white; padding: 10px 30px; border-radius: 6px; font-weight: bold; font-size: 14px; } QPushButton:hover { background-color: #1565c0; } QPushButton:pressed { background-color: #0d47a1; }");
    updateNowButton->setDefault(true);
    connect(updateNowButton, &QPushButton::clicked, this, &UpdateDialog::onUpdateNow);
    buttonLayout->addWidget(updateNowButton);
    
    mainLayout->addLayout(buttonLayout);
}

void UpdateDialog::onUpdateNow()
{
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("确认更新");
    msgBox.setText("更新需要重启程序");
    msgBox.setInformativeText("下载并安装更新后，程序将自动重启以应用新版本。\n\n确定要继续吗？");
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::Yes);
    
    QPushButton *yesButton = qobject_cast<QPushButton*>(msgBox.button(QMessageBox::Yes));
    if (yesButton) {
        yesButton->setText("继续更新");
    }
    QPushButton *noButton = qobject_cast<QPushButton*>(msgBox.button(QMessageBox::No));
    if (noButton) {
        noButton->setText("取消");
    }
    
    if (msgBox.exec() == QMessageBox::Yes) {
        emit updateNow();
        accept();
    }
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
