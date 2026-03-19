#include "syncconflictdialog.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextEdit>
#include <QGroupBox>
#include <QDebug>

SyncConflictDialog::SyncConflictDialog(const QString& taskId, const QString& taskTitle,
                                     const QJsonObject& localData, const QJsonObject& cloudData,
                                     QWidget *parent)
    : QDialog(parent)
    , m_resolution(UseLocal)
{
    setWindowTitle("任务冲突解决");
    setMinimumSize(700, 500);

    setupUi(taskTitle, localData, cloudData);
}

void SyncConflictDialog::setupUi(const QString& taskTitle, const QJsonObject& localData, const QJsonObject& cloudData)
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // 标题
    QLabel* titleLabel = new QLabel("任务冲突: " + taskTitle, this);
    titleLabel->setStyleSheet("font-size: 16px; font-weight: bold;");
    mainLayout->addWidget(titleLabel);

    // 说明
    QLabel* descLabel = new QLabel("检测到此任务在本地和云端都有修改，请选择保留哪个版本：", this);
    mainLayout->addWidget(descLabel);

    // 本地版本和云端版本对比
    QHBoxLayout* compareLayout = new QHBoxLayout();

    // 本地版本
    QGroupBox* localGroup = new QGroupBox("本地版本", this);
    QVBoxLayout* localLayout = new QVBoxLayout();
    m_localTextEdit = new QTextEdit(this);
    m_localTextEdit->setReadOnly(true);
    m_localTextEdit->setPlainText(QJsonDocument(localData).toJson(QJsonDocument::Indented));
    localLayout->addWidget(m_localTextEdit);
    localGroup->setLayout(localLayout);
    compareLayout->addWidget(localGroup);

    // 云端版本
    QGroupBox* cloudGroup = new QGroupBox("云端版本", this);
    QVBoxLayout* cloudLayout = new QVBoxLayout();
    m_cloudTextEdit = new QTextEdit(this);
    m_cloudTextEdit->setReadOnly(true);
    m_cloudTextEdit->setPlainText(QJsonDocument(cloudData).toJson(QJsonDocument::Indented));
    cloudLayout->addWidget(m_cloudTextEdit);
    cloudGroup->setLayout(cloudLayout);
    compareLayout->addWidget(cloudGroup);

    mainLayout->addLayout(compareLayout);

    // 按钮区域
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    m_useLocalBtn = new QPushButton("保留本地版本", this);
    m_useCloudBtn = new QPushButton("保留云端版本", this);
    m_keepBothBtn = new QPushButton("保留两个版本", this);

    connect(m_useLocalBtn, &QPushButton::clicked, this, &SyncConflictDialog::onUseLocalClicked);
    connect(m_useCloudBtn, &QPushButton::clicked, this, &SyncConflictDialog::onUseCloudClicked);
    connect(m_keepBothBtn, &QPushButton::clicked, this, &SyncConflictDialog::onKeepBothClicked);

    buttonLayout->addWidget(m_useLocalBtn);
    buttonLayout->addWidget(m_useCloudBtn);
    buttonLayout->addWidget(m_keepBothBtn);

    mainLayout->addLayout(buttonLayout);
}

void SyncConflictDialog::onUseLocalClicked()
{
    m_resolution = UseLocal;
    accept();
}

void SyncConflictDialog::onUseCloudClicked()
{
    m_resolution = UseCloud;
    accept();
}

void SyncConflictDialog::onKeepBothClicked()
{
    m_resolution = KeepBoth;
    accept();
}
