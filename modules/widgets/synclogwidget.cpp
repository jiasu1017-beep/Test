#include "synclogwidget.h"
#include "../core/database.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableWidget>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QHeaderView>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>

SyncLogWidget::SyncLogWidget(Database* db, QWidget *parent)
    : QWidget(parent)
    , m_db(db)
{
    setupUi();
    loadLogs();
}

SyncLogWidget::~SyncLogWidget()
{
}

void SyncLogWidget::setupUi()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // 工具栏
    QHBoxLayout* toolbarLayout = new QHBoxLayout();

    QLabel* filterLabel = new QLabel("类型筛选:", this);
    m_entityTypeCombo = new QComboBox(this);
    m_entityTypeCombo->addItem("全部", "");
    m_entityTypeCombo->addItem("任务", "task");
    m_entityTypeCombo->addItem("配置", "config");
    m_entityTypeCombo->addItem("分类", "category");

    m_refreshBtn = new QPushButton("刷新", this);
    m_clearBtn = new QPushButton("清空日志", this);
    m_countLabel = new QLabel("", this);

    toolbarLayout->addWidget(filterLabel);
    toolbarLayout->addWidget(m_entityTypeCombo);
    toolbarLayout->addStretch();
    toolbarLayout->addWidget(m_countLabel);
    toolbarLayout->addWidget(m_refreshBtn);
    toolbarLayout->addWidget(m_clearBtn);

    mainLayout->addLayout(toolbarLayout);

    // 日志表格
    m_table = new QTableWidget(this);
    m_table->setColumnCount(6);
    m_table->setHorizontalHeaderLabels({"时间", "类型", "操作", "实体ID", "解决方式", "详情"});
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setAlternatingRowColors(true);

    mainLayout->addWidget(m_table);

    // 信号连接
    connect(m_refreshBtn, &QPushButton::clicked, this, &SyncLogWidget::loadLogs);
    connect(m_clearBtn, &QPushButton::clicked, this, &SyncLogWidget::onClearClicked);
    connect(m_entityTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SyncLogWidget::onEntityTypeChanged);
    connect(m_table, &QTableWidget::cellDoubleClicked,
            this, &SyncLogWidget::onRowDoubleClicked);
}

void SyncLogWidget::loadLogs()
{
    if (!m_db) return;

    QString entityType = m_entityTypeCombo->currentData().toString();
    QList<SyncLog> logs = m_db->getSyncLogs(entityType, 100);

    m_table->setRowCount(logs.size());
    m_countLabel->setText(QString("共 %1 条").arg(logs.size()));

    for (int i = 0; i < logs.size(); ++i) {
        const SyncLog& log = logs.at(i);

        m_table->setItem(i, 0, new QTableWidgetItem(log.timestamp.toString("yyyy-MM-dd HH:mm:ss")));
        m_table->setItem(i, 1, new QTableWidgetItem(log.entityType));
        m_table->setItem(i, 2, new QTableWidgetItem(getActionText(log.action)));
        m_table->setItem(i, 3, new QTableWidgetItem(log.entityId));
        m_table->setItem(i, 4, new QTableWidgetItem(getResolutionText(log.resolution)));
        m_table->setItem(i, 5, new QTableWidgetItem(log.beforeData.isEmpty() ? log.afterData : log.beforeData));
    }
}

void SyncLogWidget::onRefreshClicked()
{
    loadLogs();
}

void SyncLogWidget::onClearClicked()
{
    int ret = QMessageBox::question(this, "确认清空",
                                     "确定要清空所有同步日志吗？",
                                     QMessageBox::Yes | QMessageBox::No);
    if (ret == QMessageBox::Yes && m_db) {
        m_db->clearSyncLogs();
        loadLogs();
    }
}

void SyncLogWidget::onEntityTypeChanged(int index)
{
    Q_UNUSED(index);
    loadLogs();
}

void SyncLogWidget::onRowDoubleClicked(int row, int column)
{
    Q_UNUSED(column);
    QString beforeData = m_table->item(row, 5)->text();

    // 尝试解析JSON并格式化显示
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(beforeData.toUtf8(), &parseError);

    QString displayText;
    if (parseError.error == QJsonParseError::NoError && doc.isObject()) {
        displayText = doc.toJson(QJsonDocument::Indented);
    } else {
        displayText = beforeData;
    }

    QMessageBox::information(this, "同步详情", displayText);
}

QString SyncLogWidget::getActionText(const QString& action)
{
    if (action == "upload") return "上传";
    if (action == "download") return "下载";
    if (action == "conflict_resolved") return "冲突解决";
    return action;
}

QString SyncLogWidget::getResolutionText(const QString& resolution)
{
    if (resolution == "local_wins") return "本地优先";
    if (resolution == "cloud_wins") return "云端优先";
    if (resolution == "manual") return "手动解决";
    return resolution;
}
