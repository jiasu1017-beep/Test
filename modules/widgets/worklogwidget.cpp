#include "worklogwidget.h"
#include <QDateTime>
#include <QTimer>
#include <QMessageBox>
#include <QInputDialog>
#include <QTextStream>
#include <QTextCodec>
#include <QDesktopServices>
#include <QUrl>
#include <QDebug>
#include <QGraphicsDropShadowEffect>
#include <QPropertyAnimation>
#include <QPainter>
#include <QStyleOption>
#include <QSet>
#include <algorithm>
#include <QtCharts>
using namespace QtCharts;

WorkLogWidget::WorkLogWidget(Database *database, QWidget *parent)
    : QWidget(parent), db(database), currentRunningTask(nullptr)
{
    // 初始化所有指针成员
    mainSplitter = nullptr;
    leftSplitter = nullptr;
    leftPanel = nullptr;
    rightPanel = nullptr;
    categoryTree = nullptr;
    taskTable = nullptr;
    searchEdit = nullptr;
    statusFilter = nullptr;
    priorityFilter = nullptr;
    categoryFilter = nullptr;
    taskViewDate = nullptr;
    timeFilterCombo = nullptr;
    customStartDate = nullptr;
    customEndDate = nullptr;
    categoryStatsList = nullptr;
    addTaskBtn = nullptr;
    editTaskBtn = nullptr;
    deleteTaskBtn = nullptr;
    completeTaskBtn = nullptr;
    refreshBtn = nullptr;
    generateReportBtn = nullptr;
    exportBtn = nullptr;
    addCategoryBtn = nullptr;
    editCategoryBtn = nullptr;
    deleteCategoryBtn = nullptr;
    quickAddBtn = nullptr;
    totalTasksLabel = nullptr;
    completedTasksLabel = nullptr;
    totalHoursLabel = nullptr;
    currentCategoryLabel = nullptr;
    todayProgress = nullptr;
    weekProgress = nullptr;
    completionProgressBar = nullptr;
    pieChartView = nullptr;
    pieChart = nullptr;
    taskTimer = nullptr;

    // 初始化统计标签数组
    for (int i = 0; i < 4; i++) {
        taskStatsLabels[i] = nullptr;
    }

    setupUI();
    initDefaultCategories();
    loadCategories();
    loadTasks();
    updateStatistics();

    taskTimer = new QTimer(this);
    connect(taskTimer, &QTimer::timeout, this, [this]() {
        if (currentRunningTask) {
            QDateTime now = QDateTime::currentDateTime();
            double hours = taskStartTime.secsTo(now) / 3600.0;
            currentRunningTask->workDuration = hours;
            refreshTaskTable();
        }
    });
}

WorkLogWidget::~WorkLogWidget()
{
}

void WorkLogWidget::setupUI()
{
    // 设置主窗口样式
    this->setStyleSheet(R"(
        QWidget {
            background-color: #f5f6fa;
            color: #2d3436;
            font-family: "Microsoft YaHei", "Segoe UI", sans-serif;
            font-size: 13px;
        }

        /* 主分割线样式 */
        QSplitter::handle {
            background-color: #dfe6e9;
            width: 1px;
        }

        /* 标签样式 */
        QLabel {
            color: #2d3436;
            background: transparent;
        }

        QLabel[cssClass="section-title"] {
            font-size: 16px;
            font-weight: bold;
            color: #2c3e50;
            padding: 8px;
            border-left: 4px solid #3498db;
            background-color: #ffffff;
        }

        /* 输入框样式 */
        QLineEdit {
            padding: 6px 12px;
            border: 1px solid #dfe6e9;
            border-radius: 4px;
            background-color: #ffffff;
            selection-background-color: #3498db;
        }

        QLineEdit:focus {
            border: 1px solid #3498db;
        }

        QLineEdit:hover {
            border: 1px solid #95a5a6;
        }

        /* 下拉框样式 */
        QComboBox {
            padding: 6px 12px;
            border: 1px solid #dfe6e9;
            border-radius: 4px;
            background-color: #ffffff;
            min-width: 120px;
        }

        QComboBox:focus {
            border: 1px solid #3498db;
        }

        QComboBox:hover {
            border: 1px solid #95a5a6;
        }

        QComboBox::drop-down {
            border: none;
            width: 30px;
        }

        QComboBox::down-arrow {
            image: none;
            border-left: 5px solid transparent;
            border-right: 5px solid transparent;
            border-top: 6px solid #7f8c8d;
            margin-right: 10px;
        }

        QComboBox QAbstractItemView {
            border: 1px solid #dfe6e9;
            background-color: #ffffff;
            selection-background-color: #3498db;
            border-radius: 4px;
        }

        QComboBox QAbstractItemView::item {
            min-height: 30px;
            padding: 4px 8px;
        }

        QComboBox QAbstractItemView::item:selected {
            background-color: #3498db;
            color: white;
        }

        /* 按钮样式 */
        QPushButton {
            padding: 8px 16px;
            border: none;
            border-radius: 5px;
            background-color: #3498db;
            color: white;
            font-weight: bold;
        }

        QPushButton:hover {
            background-color: #2980b9;
        }

        QPushButton:pressed {
            background-color: #21618c;
        }

        QPushButton:disabled {
            background-color: #bdc3c7;
            color: #7f8c8d;
        }

        QPushButton[cssClass="btn-success"] {
            background-color: #27ae60;
        }

        QPushButton[cssClass="btn-success"]:hover {
            background-color: #229954;
        }

        QPushButton[cssClass="btn-success"]:pressed {
            background-color: #1e8449;
        }

        QPushButton[cssClass="btn-warning"] {
            background-color: #f39c12;
        }

        QPushButton[cssClass="btn-warning"]:hover {
            background-color: #d68910;
        }

        QPushButton[cssClass="btn-warning"]:pressed {
            background-color: #b9770e;
        }

        QPushButton[cssClass="btn-danger"] {
            background-color: #e74c3c;
        }

        QPushButton[cssClass="btn-danger"]:hover {
            background-color: #c0392b;
        }

        QPushButton[cssClass="btn-danger"]:pressed {
            background-color: #a93226;
        }

        QPushButton[cssClass="btn-secondary"] {
            background-color: #95a5a6;
        }

        QPushButton[cssClass="btn-secondary"]:hover {
            background-color: #7f8c8d;
        }

        QPushButton[cssClass="btn-secondary"]:pressed {
            background-color: #6c7a7d;
        }

        QPushButton[cssClass="btn-primary"] {
            background-color: #8e44ad;
        }

        QPushButton[cssClass="btn-primary"]:hover {
            background-color: #7d3c98;
        }

        QPushButton[cssClass="btn-primary"]:pressed {
            background-color: #6c3483;
        }
            background-color: #2980b9;
        }

        QPushButton:pressed {
            background-color: #21618c;
        }

        QPushButton:disabled {
            background-color: #bdc3c7;
            color: #7f8c8d;
        }

        QPushButton[cssClass="btn-success"] {
            background-color: #27ae60;
        }

        QPushButton[cssClass="btn-success"]:hover {
            background-color: #229954;
        }

        QPushButton[cssClass="btn-warning"] {
            background-color: #f39c12;
        }

        QPushButton[cssClass="btn-warning"]:hover {
            background-color: #d68910;
        }

        QPushButton[cssClass="btn-danger"] {
            background-color: #e74c3c;
        }

        QPushButton[cssClass="btn-danger"]:hover {
            background-color: #c0392b;
        }

        QPushButton[cssClass="btn-secondary"] {
            background-color: #95a5a6;
        }

        QPushButton[cssClass="btn-secondary"]:hover {
            background-color: #7f8c8d;
        }

        /* 表格样式 */
        QTableWidget {
            border: 1px solid #dfe6e9;
            border-radius: 4px;
            background-color: #ffffff;
            gridline-color: #ecf0f1;
            selection-background-color: #3498db;
            selection-color: white;
        }

        QTableWidget::item {
            padding: 8px;
            border: none;
        }

        QTableWidget::item:selected {
            background-color: #3498db;
            color: white;
        }

        QHeaderView::section {
            background-color: #2c3e50;
            color: white;
            padding: 8px;
            border: none;
            font-weight: bold;
            text-transform: uppercase;
            font-size: 12px;
        }

        QHeaderView::section:first {
            border-top-left-radius: 4px;
        }

        QHeaderView::section:last {
            border-top-right-radius: 4px;
        }

        QHeaderView::section:hover {
            background-color: #34495e;
        }

        QTableCornerButton::section {
            background-color: #2c3e50;
            border: none;
        }

        /* 树形组件样式 */
        QTreeWidget {
            border: 1px solid #dfe6e9;
            border-radius: 4px;
            background-color: #ffffff;
            alternate-background-color: #f9f9f9;
        }

        QTreeWidget::item {
            padding: 6px 4px;
            border-radius: 3px;
        }

        QTreeWidget::item:selected {
            background-color: #3498db;
            color: white;
        }

        QTreeWidget::branch {
            background-color: transparent;
        }

        QTreeWidget::branch:has-siblings:!adjoins-item {
            border-left: 1px solid #bdc3c7;
        }

        QTreeWidget::branch:has-children:!has-siblings:closed,
        QTreeWidget::branch:closed:has-children:has-siblings {
            image: none;
        }

        QTreeWidget::branch:open:has-children:!has-siblings,
        QTreeWidget::branch:open:has-children:has-siblings {
            image: none;
        }

        /* 分组框样式 */
        QGroupBox {
            margin-top: 10px;
            padding-top: 10px;
            border: 1px solid #dfe6e9;
            border-radius: 4px;
            background-color: #ffffff;
            font-weight: bold;
            color: #2c3e50;
        }

        QGroupBox::title {
            subcontrol-origin: margin;
            subcontrol-position: top left;
            left: 10px;
            padding: 0 5px;
            color: #3498db;
        }

        /* 进度条样式 */
        QProgressBar {
            border: none;
            border-radius: 10px;
            background-color: #ecf0f1;
            text-align: center;
            height: 20px;
        }

        QProgressBar::chunk {
            background-color: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                              stop:0 #3498db, stop:1 #2ecc71);
            border-radius: 10px;
        }

        /* 日期编辑器样式 */
        QDateEdit {
            padding: 6px 12px;
            border: 1px solid #dfe6e9;
            border-radius: 4px;
            background-color: #ffffff;
        }

        QDateEdit:focus {
            border: 1px solid #3498db;
        }

        QDateEdit::drop-down {
            border: none;
            width: 30px;
        }

        QDateEdit::down-arrow {
            image: none;
            border-left: 5px solid transparent;
            border-right: 5px solid transparent;
            border-top: 6px solid #7f8c8d;
            margin-right: 10px;
        }

        /* 滚动条样式 */
        QScrollBar:vertical {
            background-color: #f5f6fa;
            width: 10px;
            border-radius: 5px;
        }

        QScrollBar::handle:vertical {
            background-color: #bdc3c7;
            min-height: 30px;
            border-radius: 5px;
        }

        QScrollBar::handle:vertical:hover {
            background-color: #95a5a6;
        }

        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            height: 0px;
        }

        QScrollBar:horizontal {
            background-color: #f5f6fa;
            height: 10px;
            border-radius: 5px;
        }

        QScrollBar::handle:horizontal {
            background-color: #bdc3c7;
            min-width: 30px;
            border-radius: 5px;
        }

        QScrollBar::handle:horizontal:hover {
            background-color: #95a5a6;
        }

        QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {
            width: 0px;
        }
    )");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);

    setupToolbar();

    mainSplitter = new QSplitter(Qt::Horizontal, this);
    mainSplitter->setHandleWidth(1);

    // 左侧面板（包含任务管理和统计）
    QWidget *leftPanel = new QWidget(this);
    QVBoxLayout *leftMainLayout = new QVBoxLayout(leftPanel);
    leftMainLayout->setContentsMargins(0, 0, 0, 0);
    leftMainLayout->setSpacing(8);

    // 左侧分割器（上下两部分：分类管理 + 统计）
    QSplitter *leftSplitter = new QSplitter(Qt::Vertical, leftPanel);
    leftSplitter->setHandleWidth(1);

    // 左上：分类管理面板
    QWidget *categoryPanel = new QWidget(leftPanel);
    QVBoxLayout *categoryLayout = new QVBoxLayout(categoryPanel);
    categoryLayout->setContentsMargins(0, 0, 0, 0);
    categoryLayout->setSpacing(6);

    // 分类管理按钮
    QHBoxLayout *categoryBtnLayout = new QHBoxLayout();
    categoryBtnLayout->setSpacing(4);
    addCategoryBtn = new QPushButton("➕ 添加", categoryPanel);
    addCategoryBtn->setProperty("cssClass", "btn-success");
    addCategoryBtn->setMaximumWidth(80);
    editCategoryBtn = new QPushButton("✏️ 编辑", categoryPanel);
    editCategoryBtn->setProperty("cssClass", "btn-warning");
    editCategoryBtn->setMaximumWidth(80);
    deleteCategoryBtn = new QPushButton("🗑️ 删除", categoryPanel);
    deleteCategoryBtn->setProperty("cssClass", "btn-danger");
    deleteCategoryBtn->setMaximumWidth(80);
    categoryBtnLayout->addWidget(addCategoryBtn);
    categoryBtnLayout->addWidget(editCategoryBtn);
    categoryBtnLayout->addWidget(deleteCategoryBtn);
    categoryLayout->addLayout(categoryBtnLayout);

    // 分类树
    setupCategoryTree();
    categoryLayout->addWidget(categoryTree);

    leftSplitter->addWidget(categoryPanel);

    // 左下：工作汇总统计面板
    QWidget *statisticsPanel = new QWidget(leftPanel);
    QVBoxLayout *statisticsLayout = new QVBoxLayout(statisticsPanel);
    statisticsLayout->setContentsMargins(0, 0, 0, 0);
    statisticsLayout->setSpacing(6);

    QLabel *statisticsLabel = new QLabel("📊 工作汇总", statisticsPanel);
    statisticsLabel->setObjectName("sectionTitle");
    statisticsLabel->setProperty("cssClass", "section-title");
    statisticsLayout->addWidget(statisticsLabel);

    // 时间段选择
    QHBoxLayout *timeFilterLayout = new QHBoxLayout();
    timeFilterLayout->setSpacing(4);
    timeFilterCombo = new QComboBox(statisticsPanel);
    timeFilterCombo->addItem("📅 今日", Today);
    timeFilterCombo->addItem("📆 本周", ThisWeek);
    timeFilterCombo->addItem("📆 本月", ThisMonth);
    timeFilterCombo->addItem("📆 本年", ThisYear);
    timeFilterCombo->addItem("🔍 自定义", Custom);
    timeFilterCombo->setMaximumWidth(120);
    timeFilterLayout->addWidget(timeFilterCombo);
    timeFilterLayout->addStretch();
    statisticsLayout->addLayout(timeFilterLayout);

    // 自定义日期范围（默认隐藏）
    QHBoxLayout *customDateLayout = new QHBoxLayout();
    customDateLayout->setSpacing(4);
    customDateLayout->addWidget(new QLabel("从:", statisticsPanel));
    customStartDate = new QDateEdit(statisticsPanel);
    customStartDate->setCalendarPopup(true);
    customStartDate->setDate(QDate::currentDate().addDays(-7));
    customStartDate->setDisplayFormat("MM-dd");
    customStartDate->setMinimumWidth(100);
    customDateLayout->addWidget(customStartDate);
    customDateLayout->addWidget(new QLabel("至:", statisticsPanel));
    customEndDate = new QDateEdit(statisticsPanel);
    customEndDate->setCalendarPopup(true);
    customEndDate->setDate(QDate::currentDate());
    customEndDate->setDisplayFormat("MM-dd");
    customEndDate->setMinimumWidth(100);
    customDateLayout->addWidget(customEndDate);
    customDateLayout->addStretch();
    statisticsLayout->addLayout(customDateLayout);

    // 统计图表区域（使用 Widget 占位，后续可集成图表库）
    QGroupBox *chartGroup = new QGroupBox("📈 工时统计", statisticsPanel);
    QVBoxLayout *chartLayout = new QVBoxLayout(chartGroup);
    chartLayout->setSpacing(4);

    // 今日统计
    todayProgress = new QProgressBar(statisticsPanel);
    todayProgress->setRange(0, 100);
    todayProgress->setValue(0);
    todayProgress->setFormat("📊 今日工时：%v 小时");
    todayProgress->setMinimumHeight(25);
    chartLayout->addWidget(todayProgress);

    // 本周统计
    weekProgress = new QProgressBar(statisticsPanel);
    weekProgress->setRange(0, 100);
    weekProgress->setValue(0);
    weekProgress->setFormat("📊 本周工时：%v 小时");
    weekProgress->setMinimumHeight(25);
    chartLayout->addWidget(weekProgress);

    // 饼图统计（合并任务完成情况和分类时间分布）
    pieChart = new QChart();
    pieChart->setTitle("� 分类时间分布");
    pieChart->setAnimationOptions(QChart::SeriesAnimations);

    pieChartView = new QChartView(pieChart, statisticsPanel);
    pieChartView->setRenderHint(QPainter::Antialiasing);
    pieChartView->setMinimumHeight(300);
    chartLayout->addWidget(pieChartView);

    chartLayout->addStretch();
    statisticsLayout->addWidget(chartGroup);

    leftSplitter->addWidget(statisticsPanel);

    // 设置左侧分割器大小（任务管理占 40%，统计占 60%）
    QList<int> leftSizes;
    leftSizes << 250 << 400;
    leftSplitter->setSizes(leftSizes);

    leftMainLayout->addWidget(leftSplitter);

    // 右侧：任务列表（主视图）
    rightPanel = new QWidget(this);
    QVBoxLayout *rightMainLayout = new QVBoxLayout(rightPanel);
    rightMainLayout->setContentsMargins(0, 0, 0, 0);
    rightMainLayout->setSpacing(8);

    QLabel *taskListLabel = new QLabel("📋 任务列表", rightPanel);
    taskListLabel->setObjectName("sectionTitle");
    taskListLabel->setProperty("cssClass", "section-title");
    rightMainLayout->addWidget(taskListLabel);

    // 第一行筛选器
    QHBoxLayout *filterLayout = new QHBoxLayout();
    filterLayout->setSpacing(8);

    searchEdit = new QLineEdit(this);
    searchEdit->setPlaceholderText("🔍 搜索任务...");
    filterLayout->addWidget(new QLabel("搜索:", this));
    filterLayout->addWidget(searchEdit, 1);

    statusFilter = new QComboBox(this);
    statusFilter->addItem("全部状态", -1);
    statusFilter->addItem("📝 待办", static_cast<int>(TaskStatus_Todo));
    statusFilter->addItem("🔄 进行中", static_cast<int>(TaskStatus_InProgress));
    statusFilter->addItem("⏸️ 暂停", static_cast<int>(TaskStatus_Paused));
    statusFilter->addItem("✅ 已完成", static_cast<int>(TaskStatus_Completed));
    filterLayout->addWidget(new QLabel("状态:", this));
    filterLayout->addWidget(statusFilter);

    priorityFilter = new QComboBox(this);
    priorityFilter->addItem("全部优先级", -1);
    priorityFilter->addItem("🔵 低", static_cast<int>(TaskPriority_Low));
    priorityFilter->addItem("🟡 中", static_cast<int>(TaskPriority_Medium));
    priorityFilter->addItem("🔴 高", static_cast<int>(TaskPriority_High));
    filterLayout->addWidget(new QLabel("优先级:", this));
    filterLayout->addWidget(priorityFilter);

    rightMainLayout->addLayout(filterLayout);

    // 日期选择器（用于查看往日任务）
    QHBoxLayout *dateViewLayout = new QHBoxLayout();
    dateViewLayout->setSpacing(8);
    dateViewLayout->addWidget(new QLabel("📅 查看日期:", this));

    taskViewDate = new QDateEdit(this);
    taskViewDate->setCalendarPopup(true);
    taskViewDate->setDate(QDate::currentDate());
    taskViewDate->setDisplayFormat("yyyy-MM-dd");
    taskViewDate->setMinimumWidth(150);
    dateViewLayout->addWidget(taskViewDate);

    QPushButton *prevDayBtn = new QPushButton("◀ 前一天", this);
    prevDayBtn->setStyleSheet("padding: 6px 12px;");
    dateViewLayout->addWidget(prevDayBtn);

    QPushButton *nextDayBtn = new QPushButton("后一天 ▶", this);
    nextDayBtn->setStyleSheet("padding: 6px 12px;");
    dateViewLayout->addWidget(nextDayBtn);

    QPushButton *todayBtn = new QPushButton("今天", this);
    todayBtn->setStyleSheet("padding: 6px 12px;");
    dateViewLayout->addWidget(todayBtn);

    dateViewLayout->addStretch();
    rightMainLayout->addLayout(dateViewLayout);

    // 工具栏按钮
    QHBoxLayout *taskBtnLayout = new QHBoxLayout();
    taskBtnLayout->setSpacing(6);
    addTaskBtn = new QPushButton("➕ 新建任务", this);
    addTaskBtn->setProperty("cssClass", "btn-success");
    editTaskBtn = new QPushButton("✏️ 编辑任务", this);
    editTaskBtn->setProperty("cssClass", "btn-warning");
    deleteTaskBtn = new QPushButton("🗑️ 删除任务", this);
    deleteTaskBtn->setProperty("cssClass", "btn-danger");
    completeTaskBtn = new QPushButton("✅ 完成任务", this);
    completeTaskBtn->setProperty("cssClass", "btn-success");
    refreshBtn = new QPushButton("🔄 刷新", this);
    refreshBtn->setProperty("cssClass", "btn-secondary");
    quickAddBtn = new QPushButton("⚡ 快速添加", this);
    quickAddBtn->setProperty("cssClass", "btn-warning");
    taskBtnLayout->addWidget(addTaskBtn);
    taskBtnLayout->addWidget(editTaskBtn);
    taskBtnLayout->addWidget(deleteTaskBtn);
    taskBtnLayout->addWidget(completeTaskBtn);
    taskBtnLayout->addWidget(refreshBtn);
    taskBtnLayout->addWidget(quickAddBtn);
    rightMainLayout->addLayout(taskBtnLayout);

    setupTaskTable();
    rightMainLayout->addWidget(taskTable);

    // 报告按钮
    QHBoxLayout *reportBtnLayout = new QHBoxLayout();
    reportBtnLayout->setSpacing(6);
    generateReportBtn = new QPushButton("📊 生成周报/月报", this);
    generateReportBtn->setProperty("cssClass", "btn-primary");
    exportBtn = new QPushButton("💾 导出报告", this);
    exportBtn->setProperty("cssClass", "btn-secondary");
    reportBtnLayout->addWidget(generateReportBtn);
    reportBtnLayout->addWidget(exportBtn);
    reportBtnLayout->addStretch();
    rightMainLayout->addLayout(reportBtnLayout);

    leftMainLayout->addWidget(leftPanel);
    rightMainLayout->addWidget(rightPanel);

    mainSplitter->addWidget(leftPanel);
    mainSplitter->addWidget(rightPanel);

    mainLayout->addWidget(mainSplitter);

    // 设置分割器初始大小（左侧 350px，右侧占据剩余空间）
    QList<int> sizes;
    sizes << 350 << 1200;
    mainSplitter->setSizes(sizes);

    connect(addTaskBtn, &QPushButton::clicked, this, &WorkLogWidget::onAddTask);
    connect(editTaskBtn, &QPushButton::clicked, this, &WorkLogWidget::onEditTask);
    connect(deleteTaskBtn, &QPushButton::clicked, this, &WorkLogWidget::onDeleteTask);
    connect(completeTaskBtn, &QPushButton::clicked, this, &WorkLogWidget::onCompleteTask);
    connect(refreshBtn, &QPushButton::clicked, this, &WorkLogWidget::onRefreshTasks);
    connect(quickAddBtn, &QPushButton::clicked, this, &WorkLogWidget::onQuickAddTask);
    connect(generateReportBtn, &QPushButton::clicked, this, &WorkLogWidget::onGenerateReport);
    connect(exportBtn, &QPushButton::clicked, this, &WorkLogWidget::onExportReport);

    connect(addCategoryBtn, &QPushButton::clicked, this, &WorkLogWidget::onAddCategory);
    connect(editCategoryBtn, &QPushButton::clicked, this, &WorkLogWidget::onEditCategory);
    connect(deleteCategoryBtn, &QPushButton::clicked, this, &WorkLogWidget::onDeleteCategory);

    connect(taskViewDate, &QDateEdit::dateChanged, this, &WorkLogWidget::onViewDateChanged);
    connect(prevDayBtn, &QPushButton::clicked, this, &WorkLogWidget::onPrevDay);
    connect(nextDayBtn, &QPushButton::clicked, this, &WorkLogWidget::onNextDay);
    connect(todayBtn, &QPushButton::clicked, this, &WorkLogWidget::onToday);

    connect(categoryTree, &QTreeWidget::itemSelectionChanged, this, &WorkLogWidget::onCategorySelectionChanged);
    connect(taskTable, &QTableWidget::itemSelectionChanged, this, &WorkLogWidget::onTaskSelectionChanged);
    connect(taskTable, &QTableWidget::cellDoubleClicked, this, &WorkLogWidget::onTaskDoubleClicked);

    connect(searchEdit, &QLineEdit::textChanged, this, &WorkLogWidget::onFilterChanged);
    connect(statusFilter, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &WorkLogWidget::onFilterChanged);
    connect(priorityFilter, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &WorkLogWidget::onFilterChanged);
    connect(timeFilterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &WorkLogWidget::onTimeFilterChanged);
    connect(customStartDate, &QDateEdit::dateChanged, this, &WorkLogWidget::updateStatistics);
    connect(customEndDate, &QDateEdit::dateChanged, this, &WorkLogWidget::updateStatistics);

    taskTable->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(taskTable, &QTableWidget::customContextMenuRequested, this, &WorkLogWidget::onTaskContextMenu);
}

void WorkLogWidget::setupToolbar()
{
}

void WorkLogWidget::setupTaskTable()
{
    taskTable = new QTableWidget();
    taskTable->setColumnCount(7);
    taskTable->setHorizontalHeaderLabels({"🔢 序号", "📝 标题", "📁 分类", "🎯 优先级", "📊 状态", "⏱️ 工时", "🏷️ 标签"});

    taskTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    taskTable->setSelectionMode(QAbstractItemView::SingleSelection);
    taskTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    taskTable->setAlternatingRowColors(true);
    taskTable->horizontalHeader()->setStretchLastSection(true);
    taskTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    taskTable->verticalHeader()->setVisible(false);
    taskTable->setShowGrid(true);
    taskTable->setSortingEnabled(false);

    // 设置列宽
    taskTable->setColumnWidth(0, 60);
    taskTable->setColumnWidth(1, 280);
    taskTable->setColumnWidth(2, 100);
    taskTable->setColumnWidth(3, 80);
    taskTable->setColumnWidth(4, 90);
    taskTable->setColumnWidth(5, 80);
    taskTable->setColumnWidth(6, 150);

    // 启用自动换行
    taskTable->setWordWrap(true);

    // 设置图标大小
    taskTable->setIconSize(QSize(16, 16));

    // 设置单元格对齐方式
    taskTable->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
}

void WorkLogWidget::setupCategoryTree()
{
    categoryTree = new QTreeWidget();
    categoryTree->setHeaderLabel("📂 分类列表");
    categoryTree->setSelectionMode(QAbstractItemView::SingleSelection);
    categoryTree->setEditTriggers(QAbstractItemView::NoEditTriggers);
    categoryTree->setAnimated(true);
    categoryTree->setIndentation(20);
    categoryTree->setColumnWidth(0, 200);
    categoryTree->setUniformRowHeights(true);
    categoryTree->setHeaderHidden(false);
    categoryTree->header()->setStretchLastSection(true);
}

void WorkLogWidget::setupStatisticsPanel()
{
    QHBoxLayout *statsLayout = new QHBoxLayout();

    QGroupBox *statsGroup = new QGroupBox("统计信息", this);
    QHBoxLayout *groupLayout = new QHBoxLayout(statsGroup);

    totalTasksLabel = new QLabel("总任务数: 0", statsGroup);
    completedTasksLabel = new QLabel("已完成: 0", statsGroup);
    totalHoursLabel = new QLabel("总工时: 0小时", statsGroup);
    currentCategoryLabel = new QLabel("当前分类: 全部", statsGroup);

    groupLayout->addWidget(totalTasksLabel);
    groupLayout->addWidget(completedTasksLabel);
    groupLayout->addWidget(totalHoursLabel);
    groupLayout->addWidget(currentCategoryLabel);

    statsLayout->addWidget(statsGroup);

    QVBoxLayout *vLayout = qobject_cast<QVBoxLayout*>(rightPanel->layout());
    if (vLayout) {
        vLayout->insertLayout(vLayout->count() - 2, statsLayout);
    }
}

void WorkLogWidget::initDefaultCategories()
{
    QList<Category> existingCategories = db->getAllCategories();
    if (existingCategories.isEmpty()) {
        Category work;
        work.name = "工作";
        work.description = "工作相关任务";
        work.parentId = -1;
        work.color = "#3498db";
        db->addCategory(work);

        Category dev;
        dev.name = "研发";
        dev.description = "研发相关任务";
        dev.parentId = -1;
        dev.color = "#e74c3c";
        db->addCategory(dev);

        Category meeting;
        meeting.name = "会议";
        meeting.description = "会议相关任务";
        meeting.parentId = -1;
        meeting.color = "#f39c12";
        db->addCategory(meeting);

        Category doc;
        doc.name = "文档";
        doc.description = "文档相关任务";
        doc.parentId = -1;
        doc.color = "#9b59b6";
        db->addCategory(doc);

        Category support;
        support.name = "客户支持";
        support.description = "客户支持相关任务";
        support.parentId = -1;
        support.color = "#1abc9c";
        db->addCategory(support);

        Category ops;
        ops.name = "运维";
        ops.description = "运维相关任务";
        ops.parentId = -1;
        ops.color = "#34495e";
        db->addCategory(ops);
    }
}

void WorkLogWidget::loadCategories()
{
    categoryTree->clear();

    QTreeWidgetItem *allItem = new QTreeWidgetItem(categoryTree);
    allItem->setText(0, "全部");
    allItem->setData(0, Qt::UserRole, -1);
    allItem->setExpanded(true);

    QList<Category> categories = db->getAllCategories();
    for (const Category &category : categories) {
        QTreeWidgetItem *item = new QTreeWidgetItem(allItem);
        item->setText(0, category.name);
        item->setData(0, Qt::UserRole, category.id);
        item->setForeground(0, QBrush(QColor(category.color)));
    }

    categoryTree->expandAll();
}

void WorkLogWidget::loadTasks()
{
    refreshTaskTable();
}

void WorkLogWidget::refreshTaskTable()
{
    taskTable->setRowCount(0);

    QList<Task> allTasks = db->getAllTasks();

    QString searchText = searchEdit->text().toLower();
    int statusValue = statusFilter->currentData().toInt();
    int priorityValue = priorityFilter->currentData().toInt();

    QDate viewDate = taskViewDate->date();
    QString viewDateStr = viewDate.toString("yyyyMMdd");

    int selectedCategoryId = -1;
    if (categoryTree) {
        QList<QTreeWidgetItem*> selectedItems = categoryTree->selectedItems();
        if (!selectedItems.isEmpty()) {
            selectedCategoryId = selectedItems.first()->data(0, Qt::UserRole).toInt();
        }
    }

    QSet<QString> displayedTaskIds;

    for (const Task &task : allTasks) {
        QString taskDateStr = task.id.left(8);

        bool isViewDateTask = (taskDateStr == viewDateStr);
        bool isUncompletedTask = (task.status != TaskStatus_Completed) && (taskDateStr < viewDateStr);

        if (!isViewDateTask && !isUncompletedTask) {
            continue;
        }

        if (!searchText.isEmpty()) {
            if (!task.title.toLower().contains(searchText) &&
                !task.description.toLower().contains(searchText)) {
                continue;
            }
        }

        if (statusValue != -1 && static_cast<int>(task.status) != statusValue) {
            continue;
        }

        if (priorityValue != -1 && static_cast<int>(task.priority) != priorityValue) {
            continue;
        }

        if (selectedCategoryId != -1 && task.categoryId != selectedCategoryId) {
            continue;
        }

        if (displayedTaskIds.contains(task.id)) {
            continue;
        }

        int row = taskTable->rowCount();
        taskTable->insertRow(row);
        updateTaskRow(row, task);
        displayedTaskIds.insert(task.id);
    }
}

void WorkLogWidget::updateTaskRow(int row, const Task &task)
{
    taskTable->setItem(row, 0, new QTableWidgetItem(QString::number(row + 1)));
    taskTable->item(row, 0)->setTextAlignment(Qt::AlignCenter);

    taskTable->setItem(row, 1, new QTableWidgetItem(task.title));

    Category category = db->getCategoryById(task.categoryId);
    QString categoryName = category.id != -1 ? category.name : "未分类";
    taskTable->setItem(row, 2, new QTableWidgetItem(categoryName));

    taskTable->setItem(row, 3, new QTableWidgetItem(getPriorityString(task.priority)));
    taskTable->setItem(row, 4, new QTableWidgetItem(getStatusString(task.status)));
    taskTable->setItem(row, 5, new QTableWidgetItem(getDurationString(task.workDuration)));

    QString tags = task.tags.join(", ");
    taskTable->setItem(row, 6, new QTableWidgetItem(tags));

    for (int col = 0; col < taskTable->columnCount(); ++col) {
        if (taskTable->item(row, col)) {
            taskTable->item(row, col)->setData(Qt::UserRole, task.id);
        }
    }
}

void WorkLogWidget::refreshCategoryTree()
{
    loadCategories();
}

void WorkLogWidget::updateStatistics()
{
    if (!db) return;

    QList<Task> allTasks = db->getAllTasks();

    int totalTasks = allTasks.size();
    int completedTasks = 0;
    double totalHours = 0.0;

    for (const Task &task : allTasks) {
        if (task.status == TaskStatus_Completed) {
            completedTasks++;
            totalHours += task.workDuration;
        }
    }

    if (totalTasksLabel) {
        totalTasksLabel->setText(QString("总任务数: %1").arg(totalTasks));
    }
    if (completedTasksLabel) {
        completedTasksLabel->setText(QString("已完成: %1").arg(completedTasks));
    }
    if (totalHoursLabel) {
        totalHoursLabel->setText(QString("总工时: %1小时").arg(totalHours, 0, 'f', 1));
    }

    if (timeFilterCombo) {
        TimePeriod selectedPeriod = static_cast<TimePeriod>(timeFilterCombo->currentData().toInt());

        QDate startDate, endDate;
        switch (selectedPeriod) {
            case Today:
                startDate = QDate::currentDate();
                endDate = QDate::currentDate();
                break;
            case ThisWeek:
                startDate = QDate::currentDate().addDays(-(QDate::currentDate().dayOfWeek() - 1));
                endDate = QDate::currentDate();
                break;
            case ThisMonth:
                startDate = QDate::currentDate().addDays(-(QDate::currentDate().day() - 1));
                endDate = QDate::currentDate();
                break;
            case ThisYear:
                startDate = QDate::currentDate().addDays(-(QDate::currentDate().dayOfYear() - 1));
                endDate = QDate::currentDate();
                break;
            case Custom:
                if (customStartDate && customEndDate) {
                    startDate = customStartDate->date();
                    endDate = customEndDate->date();
                } else {
                    startDate = QDate::currentDate();
                    endDate = QDate::currentDate();
                }
                break;
        }

        QList<Task> filteredTasks;
        for (const Task &task : allTasks) {
            if (task.id.length() >= 8) {
                QString dateStr = task.id.left(8);
                QDate taskDate = QDate::fromString(dateStr, "yyyyMMdd");
                if (taskDate.isValid() && taskDate >= startDate && taskDate <= endDate) {
                    filteredTasks.append(task);
                }
            }
        }

        int periodTotalTasks = filteredTasks.size();
        int periodCompletedTasks = 0;
        double periodHours = 0.0;

        for (const Task &task : filteredTasks) {
            if (task.status == TaskStatus_Completed) {
                periodCompletedTasks++;
                periodHours += task.workDuration;
            }
        }

        // 更新饼图显示分类时间分布
        if (pieChart) {
            pieChart->removeAllSeries();

            QMap<QString, double> categoryHours;
            for (const Task &task : filteredTasks) {
                if (task.status == TaskStatus_Completed) {
                    Category cat = db->getCategoryById(task.categoryId);
                    QString catName = cat.name.isEmpty() ? "未分类" : cat.name;
                    categoryHours[catName] += task.workDuration;
                }
            }

            if (categoryHours.isEmpty()) {
                pieChart->setTitle("📊 分类时间分布（暂无数据）");
            } else {
                pieChart->setTitle(QString("📊 分类时间分布（总计：%1 小时）").arg(periodHours, 0, 'f', 1));

                QPieSeries *series = new QPieSeries();
                series->setPieStartAngle(90);
                series->setPieEndAngle(450);
                series->setLabelsVisible(true);

                QList<QString> categories = categoryHours.keys();
                std::sort(categories.begin(), categories.end(), [&categoryHours](const QString &a, const QString &b) {
                    return categoryHours[a] > categoryHours[b];
                });

                QColor colors[] = {QColor("#3498db"), QColor("#27ae60"), QColor("#f39c12"),
                                   QColor("#e74c3c"), QColor("#9b59b6"), QColor("#1abc9c"),
                                   QColor("#34495e"), QColor("#e67e22"), QColor("#2ecc71"),
                                   QColor("#95a5a6")};

                int count = 0;
                for (const QString &catName : categories) {
                    if (count >= 8) break;
                    double hours = categoryHours[catName];
                    double percentage = hours * 100 / (periodHours > 0 ? periodHours : 1);

                    QPieSlice *slice = series->append(catName, hours);

                    QColor color = colors[count % 8];
                    slice->setColor(color);

                    QString labelText = QString("%1%").arg(percentage, 0, 'f', 1);
                    slice->setLabel(labelText);
                    slice->setLabelColor(Qt::black);
                    slice->setLabelFont(QFont("Microsoft YaHei", 9, QFont::Bold));
                    slice->setLabelVisible(true);

                    if (percentage >= 12) {
                        slice->setLabelPosition(QPieSlice::LabelInsideHorizontal);
                    } else {
                        slice->setLabelPosition(QPieSlice::LabelOutside);
                        slice->setLabelArmLengthFactor(0.3);
                    }

                    count++;
                }

                pieChart->addSeries(series);

                pieChart->legend()->setVisible(true);
                pieChart->legend()->setAlignment(Qt::AlignRight);
                pieChart->legend()->setFont(QFont("Microsoft YaHei", 9));
                pieChart->legend()->setMaximumWidth(180);
                pieChart->legend()->setContentsMargins(5, 5, 5, 5);
                pieChart->legend()->setBackgroundVisible(true);
                pieChart->legend()->setBrush(QBrush(QColor(249, 250, 251, 245)));
                pieChart->legend()->setPen(QPen(QColor(200, 200, 200)));

                QList<QLegendMarker*> markers = pieChart->legend()->markers();
                 for (int i = 0; i < markers.count(); ++i) {
                     QLegendMarker *marker = markers.at(i);
                     if (i < categories.size()) {
                         QString catName = categories.at(i);
                         double hours = categoryHours[catName];
                         double percentage = hours * 100 / (periodHours > 0 ? periodHours : 1);
                         QString legendLabel = QString("%1: %2小时").arg(catName).arg(hours, 0, 'f', 1);
                         marker->setLabel(legendLabel);
                     }
                 }

                pieChart->setTheme(QChart::ChartThemeLight);
                pieChart->setBackgroundBrush(QBrush(Qt::white));
            }
        }
    }

    // 更新今日工时
    QDate today = QDate::currentDate();
    double todayHours = 0.0;
    for (const Task &task : allTasks) {
        if (task.id.length() >= 8) {
            QString dateStr = task.id.left(8);
            QDate taskDate = QDate::fromString(dateStr, "yyyyMMdd");
            if (task.status == TaskStatus_Completed && taskDate.isValid() && taskDate == today) {
                todayHours += task.workDuration;
            }
        }
    }
    if (todayProgress) {
        int targetHours = 8;
        int percentage = static_cast<int>((todayHours / targetHours) * 100);
        if (percentage > 100) percentage = 100;
        todayProgress->setValue(percentage);

        QString statusText;
        QString color;
        if (todayHours >= targetHours) {
            statusText = "✅ 已达标";
            color = "#27ae60";
        } else if (todayHours >= targetHours * 0.8) {
            statusText = "⏳ 进行中";
            color = "#f39c12";
        } else {
            statusText = "📋 待完成";
            color = "#3498db";
        }
        todayProgress->setFormat(QString("%1 %2 小时 (%3%)").arg(statusText).arg(todayHours, 0, 'f', 1).arg(percentage));
        todayProgress->update();
    }

    // 更新本周工时
    QDate weekStart = QDate::currentDate().addDays(-(QDate::currentDate().dayOfWeek() - 1));
    double weekHours = 0.0;
    for (const Task &task : allTasks) {
        if (task.id.length() >= 8) {
            QString dateStr = task.id.left(8);
            QDate taskDate = QDate::fromString(dateStr, "yyyyMMdd");
            if (task.status == TaskStatus_Completed && taskDate.isValid() && taskDate >= weekStart && taskDate <= today) {
                weekHours += task.workDuration;
            }
        }
    }
    if (weekProgress) {
        int targetHours = 40;
        int percentage = static_cast<int>((weekHours / targetHours) * 100);
        if (percentage > 100) percentage = 100;
        weekProgress->setValue(percentage);

        QString statusText;
        if (weekHours >= targetHours) {
            statusText = "✅ 已达标";
        } else if (weekHours >= targetHours * 0.8) {
            statusText = "⏳ 进行中";
        } else {
            statusText = "📋 待完成";
        }
        weekProgress->setFormat(QString("%1 %2 小时 (%3%)").arg(statusText).arg(weekHours, 0, 'f', 1).arg(percentage));
        weekProgress->update();
    }
}

void WorkLogWidget::onAddTask()
{
    showTaskDialog();
}

void WorkLogWidget::onEditTask()
{
    Task task = getCurrentTask();
    if (task.id.isEmpty()) {
        QMessageBox::warning(this, "提示", "请先选择一个任务");
        return;
    }

    showTaskDialog(&task);
}

void WorkLogWidget::onDeleteTask()
{
    Task task = getCurrentTask();
    if (task.id.isEmpty()) {
        QMessageBox::warning(this, "提示", "请先选择一个任务");
        return;
    }

    int ret = QMessageBox::question(this, "确认删除", "确定要删除这个任务吗？",
                                     QMessageBox::Yes | QMessageBox::No);
    if (ret == QMessageBox::Yes) {
        db->deleteTask(task.id);
        refreshTaskTable();
        updateStatistics();
    }
}

void WorkLogWidget::onCompleteTask()
{
    Task task = getCurrentTask();
    if (task.id.isEmpty()) {
        QMessageBox::warning(this, "提示", "请先选择一个任务");
        return;
    }

    if (task.status == TaskStatus_Completed) {
        QMessageBox::information(this, "提示", "该任务已经完成");
        return;
    }

    db->updateTaskStatus(task.id, TaskStatus_Completed);

    refreshTaskTable();
    updateStatistics();
}

void WorkLogWidget::onPauseTask()
{
    Task task = getCurrentTask();
    if (task.id.isEmpty()) {
        QMessageBox::warning(this, "提示", "请先选择一个任务");
        return;
    }

    if (task.status != TaskStatus_InProgress) {
        QMessageBox::information(this, "提示", "只能暂停正在进行的任务");
        return;
    }

    db->updateTaskStatus(task.id, TaskStatus_Paused);

    refreshTaskTable();
    updateStatistics();
}

void WorkLogWidget::onTaskSelectionChanged()
{
}

void WorkLogWidget::onTaskDoubleClicked(int row, int column)
{
    QTableWidgetItem *item = taskTable->item(row, 0);
    if (item) {
        QString taskId = item->data(Qt::UserRole).toString();
        Task task = db->getTaskById(taskId);
        if (!task.id.isEmpty()) {
            showTaskDialog(&task);
        }
    }
}

void WorkLogWidget::onRefreshTasks()
{
    refreshTaskTable();
    updateStatistics();
}

void WorkLogWidget::onFilterChanged()
{
    refreshTaskTable();
}

void WorkLogWidget::onAddCategory()
{
    showCategoryDialog();
}

void WorkLogWidget::onEditCategory()
{
    Category category = getCurrentCategory();
    if (category.id == -1) {
        QMessageBox::warning(this, "提示", "请先选择一个分类");
        return;
    }

    showCategoryDialog(&category);
}

void WorkLogWidget::onDeleteCategory()
{
    Category category = getCurrentCategory();
    if (category.id == -1) {
        QMessageBox::warning(this, "提示", "请先选择一个分类");
        return;
    }

    int ret = QMessageBox::question(this, "确认删除", "确定要删除这个分类吗？该分类下的任务将变为未分类。",
                                     QMessageBox::Yes | QMessageBox::No);
    if (ret == QMessageBox::Yes) {
        db->deleteCategory(category.id);
        refreshCategoryTree();
        refreshTaskTable();
    }
}

void WorkLogWidget::onCategorySelectionChanged()
{
    if (!categoryTree) return;

    QList<QTreeWidgetItem*> selectedItems = categoryTree->selectedItems();
    if (!selectedItems.isEmpty()) {
        int categoryId = selectedItems.first()->data(0, Qt::UserRole).toInt();
        if (currentCategoryLabel) {
            if (categoryId == -1) {
                currentCategoryLabel->setText("当前分类: 全部");
            } else {
                Category category = db->getCategoryById(categoryId);
                if (category.id != -1) {
                    currentCategoryLabel->setText(QString("当前分类: %1").arg(category.name));
                }
            }
        }
    }

    refreshTaskTable();
}

void WorkLogWidget::onGenerateReport()
{
    bool ok;
    QString reportType = QInputDialog::getItem(this, "选择报告类型", "请选择要生成的报告类型:",
                                               {"周报", "月报"}, 0, false, &ok);
    if (!ok) return;

    QDate startDate, endDate;
    if (reportType == "周报") {
        QDate today = QDate::currentDate();
        int dayOfWeek = today.dayOfWeek();
        startDate = today.addDays(-(dayOfWeek - 1));
        endDate = startDate.addDays(6);
    } else {
        QDate today = QDate::currentDate();
        startDate = QDate(today.year(), today.month(), 1);
        endDate = QDate(today.year(), today.month(), today.daysInMonth());
    }

    QString reportContent;
    if (reportType == "周报") {
        reportContent = generateWeeklyReport(QDateTime(startDate), QDateTime(endDate));
    } else {
        reportContent = generateMonthlyReport(QDateTime(startDate), QDateTime(endDate));
    }

    QDialog reportDialog(this);
    reportDialog.setWindowTitle(reportType);
    reportDialog.resize(800, 600);

    QVBoxLayout *layout = new QVBoxLayout(&reportDialog);

    QTextEdit *reportEdit = new QTextEdit(&reportDialog);
    reportEdit->setPlainText(reportContent);
    reportEdit->setReadOnly(true);
    layout->addWidget(reportEdit);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *copyBtn = new QPushButton("复制", &reportDialog);
    QPushButton *closeBtn = new QPushButton("关闭", &reportDialog);
    btnLayout->addWidget(copyBtn);
    btnLayout->addWidget(closeBtn);
    layout->addLayout(btnLayout);

    connect(copyBtn, &QPushButton::clicked, [reportEdit, this]() {
        reportEdit->selectAll();
        reportEdit->copy();
        QMessageBox::information(this, "提示", "已复制到剪贴板");
    });

    connect(closeBtn, &QPushButton::clicked, &reportDialog, &QDialog::accept);

    reportDialog.exec();
}

void WorkLogWidget::onExportReport()
{
    bool ok;
    QString reportType = QInputDialog::getItem(this, "选择报告类型", "请选择要导出的报告类型:",
                                               {"周报", "月报"}, 0, false, &ok);
    if (!ok) return;

    QDate startDate, endDate;
    if (reportType == "周报") {
        QDate today = QDate::currentDate();
        int dayOfWeek = today.dayOfWeek();
        startDate = today.addDays(-(dayOfWeek - 1));
        endDate = startDate.addDays(6);
    } else {
        QDate today = QDate::currentDate();
        startDate = QDate(today.year(), today.month(), 1);
        endDate = QDate(today.year(), today.month(), today.daysInMonth());
    }

    QString reportContent;
    if (reportType == "周报") {
        reportContent = generateWeeklyReport(QDateTime(startDate), QDateTime(endDate));
    } else {
        reportContent = generateMonthlyReport(QDateTime(startDate), QDateTime(endDate));
    }

    QString fileName = QFileDialog::getSaveFileName(this, "保存报告",
                                                     QString("%1_%2.md").arg(reportType).arg(QDate::currentDate().toString("yyyyMMdd")),
                                                     "Markdown文件 (*.md);;文本文件 (*.txt)");
    if (fileName.isEmpty()) return;

    if (fileName.endsWith(".md")) {
        exportToMarkdown(reportContent, fileName);
    } else {
        exportToText(reportContent, fileName);
    }
}

void WorkLogWidget::onShowStatistics()
{
}

void WorkLogWidget::onQuickAddTask()
{
    bool ok;
    QString title = QInputDialog::getText(this, "快速添加任务", "请输入任务标题:", QLineEdit::Normal, "", &ok);
    if (!ok || title.isEmpty()) return;

    Task task;
    task.title = title;
    task.description = "";
    task.categoryId = -1;
    task.priority = TaskPriority_Medium;
    task.status = TaskStatus_Todo;
    task.workDuration = 0.0;

    db->addTask(task);
    refreshTaskTable();
    updateStatistics();
}

void WorkLogWidget::onStartTask()
{
}

void WorkLogWidget::onStopTask()
{
}

void WorkLogWidget::onTaskContextMenu(const QPoint &pos)
{
    QTableWidgetItem *item = taskTable->itemAt(pos);
    if (!item) {
        return;
    }

    int row = item->row();
    QString taskId = taskTable->item(row, 0)->data(Qt::UserRole).toString();

    QMenu menu(this);

    menu.addAction("▶️ 开始任务", this, [this, taskId]() {
        db->updateTaskStatus(taskId, TaskStatus_InProgress);
        refreshTaskTable();
        updateStatistics();
    });

    menu.addAction("⏸️ 暂停任务", this, [this, taskId]() {
        db->updateTaskStatus(taskId, TaskStatus_Paused);
        refreshTaskTable();
        updateStatistics();
    });

    menu.addAction("✅ 完成任务", this, [this, taskId]() {
        db->updateTaskStatus(taskId, TaskStatus_Completed);
        refreshTaskTable();
        updateStatistics();
    });

    menu.addSeparator();

    menu.addAction("✏️ 编辑任务", this, [this, taskId]() {
        Task task = db->getTaskById(taskId);
        if (!task.id.isEmpty()) {
            showTaskDialog(&task);
            refreshTaskTable();
            updateStatistics();
        }
    });

    menu.addAction("🗑️ 删除任务", this, [this, taskId]() {
        Task task = db->getTaskById(taskId);
        if (!task.id.isEmpty()) {
            int ret = QMessageBox::question(this, "确认删除",
                QString("确定要删除任务「%1」吗？").arg(task.title),
                QMessageBox::Yes | QMessageBox::No);

            if (ret == QMessageBox::Yes) {
                if (db->deleteTask(taskId)) {
                    refreshTaskTable();
                    updateStatistics();
                    QMessageBox::information(this, "成功", "任务已删除");
                } else {
                    QMessageBox::warning(this, "错误", "删除任务失败");
                }
            }
        }
    });

    menu.exec(taskTable->mapToGlobal(pos));
}

Task WorkLogWidget::getCurrentTask()
{
    QList<QTableWidgetItem*> selectedItems = taskTable->selectedItems();
    if (selectedItems.isEmpty()) {
        Task task;
        task.id = "";
        return task;
    }

    int row = selectedItems.first()->row();
    QTableWidgetItem *item = taskTable->item(row, 0);
    if (item) {
        QString taskId = item->data(Qt::UserRole).toString();
        return db->getTaskById(taskId);
    }

    Task task;
    task.id = "";
    return task;
}

Category WorkLogWidget::getCurrentCategory()
{
    if (!categoryTree) {
        Category category;
        category.id = -1;
        return category;
    }

    QList<QTreeWidgetItem*> selectedItems = categoryTree->selectedItems();
    if (selectedItems.isEmpty()) {
        Category category;
        category.id = -1;
        return category;
    }

    int categoryId = selectedItems.first()->data(0, Qt::UserRole).toInt();
    return db->getCategoryById(categoryId);
}

QString WorkLogWidget::getPriorityString(TaskPriority priority)
{
    switch (priority) {
        case TaskPriority_Low: return "低";
        case TaskPriority_Medium: return "中";
        case TaskPriority_High: return "高";
        default: return "中";
    }
}

QString WorkLogWidget::getStatusString(TaskStatus status)
{
    switch (status) {
        case TaskStatus_Todo: return "待办";
        case TaskStatus_InProgress: return "进行中";
        case TaskStatus_Paused: return "暂停";
        case TaskStatus_Completed: return "已完成";
        default: return "待办";
    }
}

QString WorkLogWidget::getDurationString(double hours)
{
    if (hours < 1.0) {
        int minutes = static_cast<int>(hours * 60);
        return QString("%1分钟").arg(minutes);
    } else {
        return QString("%1小时").arg(hours, 0, 'f', 1);
    }
}

void WorkLogWidget::showTaskDialog(Task *task)
{
    QDialog dialog(this);
    dialog.setWindowTitle(task ? "✏️ 编辑任务" : "➕ 新建任务");
    dialog.setMinimumSize(600, 500);

    // 设置对话框样式
    dialog.setStyleSheet(R"(
        QDialog {
            background-color: #f5f6fa;
        }

        QLabel {
            font-size: 13px;
            color: #2c3e50;
            font-weight: bold;
            min-width: 80px;
        }

        QLineEdit, QTextEdit, QComboBox, QSpinBox, QDoubleSpinBox {
            padding: 8px;
            border: 1px solid #dfe6e9;
            border-radius: 4px;
            background-color: #ffffff;
            font-size: 13px;
        }

        QLineEdit:focus, QTextEdit:focus, QComboBox:focus, QSpinBox:focus, QDoubleSpinBox:focus {
            border: 1px solid #3498db;
        }

        QLineEdit:hover, QTextEdit:hover, QComboBox:hover, QSpinBox:hover, QDoubleSpinBox:hover {
            border: 1px solid #95a5a6;
        }

        QTextEdit {
            min-height: 100px;
        }

        QGroupBox {
            margin-top: 10px;
            padding: 10px;
            border: 1px solid #dfe6e9;
            border-radius: 4px;
            background-color: #ffffff;
            font-weight: bold;
            color: #2c3e50;
        }

        QGroupBox::title {
            subcontrol-origin: margin;
            subcontrol-position: top left;
            left: 10px;
            padding: 0 5px;
            color: #3498db;
        }
    )");

    QVBoxLayout *mainLayout = new QVBoxLayout(&dialog);
    mainLayout->setSpacing(15);

    // 基本信息分组
    QGroupBox *basicGroup = new QGroupBox("📝 基本信息", &dialog);
    QFormLayout *basicLayout = new QFormLayout(basicGroup);
    basicLayout->setSpacing(10);

    QLineEdit *titleEdit = new QLineEdit(&dialog);
    titleEdit->setPlaceholderText("请输入任务标题");
    QTextEdit *descEdit = new QTextEdit(&dialog);
    descEdit->setPlaceholderText("请输入任务描述");

    basicLayout->addRow("标题:", titleEdit);
    basicLayout->addRow("描述:", descEdit);

    mainLayout->addWidget(basicGroup);

    // 详细信息分组
    QGroupBox *detailGroup = new QGroupBox("📊 详细信息", &dialog);
    QFormLayout *detailLayout = new QFormLayout(detailGroup);
    detailLayout->setSpacing(10);

    QComboBox *categoryCombo = new QComboBox(&dialog);
    QComboBox *priorityCombo = new QComboBox(&dialog);
    QComboBox *statusCombo = new QComboBox(&dialog);
    QDoubleSpinBox *durationSpin = new QDoubleSpinBox(&dialog);
    QLineEdit *tagsEdit = new QLineEdit(&dialog);

    categoryCombo->addItem("📁 未分类", -1);
    QList<Category> categories = db->getAllCategories();
    for (const Category &cat : categories) {
        categoryCombo->addItem("📁 " + cat.name, cat.id);
    }

    priorityCombo->addItem("🔵 低", static_cast<int>(TaskPriority_Low));
    priorityCombo->addItem("🟡 中", static_cast<int>(TaskPriority_Medium));
    priorityCombo->addItem("🔴 高", static_cast<int>(TaskPriority_High));

    statusCombo->addItem("📝 待办", static_cast<int>(TaskStatus_Todo));
    statusCombo->addItem("🔄 进行中", static_cast<int>(TaskStatus_InProgress));
    statusCombo->addItem("✅ 已完成", static_cast<int>(TaskStatus_Completed));

    durationSpin->setRange(0, 9999);
    durationSpin->setSingleStep(0.5);
    durationSpin->setDecimals(1);
    durationSpin->setSuffix(" 小时");

    tagsEdit->setPlaceholderText("多个标签用逗号分隔，例如：重要，紧急");

    detailLayout->addRow("分类:", categoryCombo);
    detailLayout->addRow("优先级:", priorityCombo);
    detailLayout->addRow("状态:", statusCombo);
    detailLayout->addRow("工时:", durationSpin);
    detailLayout->addRow("标签:", tagsEdit);

    mainLayout->addWidget(detailGroup);

    // 按钮布局
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(10);

    QPushButton *okBtn = new QPushButton("✅ 确定", &dialog);
    okBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #27ae60;
            color: white;
            font-weight: bold;
            padding: 10px 30px;
            border-radius: 4px;
            min-width: 100px;
        }
        QPushButton:hover {
            background-color: #229954;
        }
        QPushButton:pressed {
            background-color: #1e8449;
        }
    )");

    QPushButton *cancelBtn = new QPushButton("❌ 取消", &dialog);
    cancelBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #95a5a6;
            color: white;
            font-weight: bold;
            padding: 10px 30px;
            border-radius: 4px;
            min-width: 100px;
        }
        QPushButton:hover {
            background-color: #7f8c8d;
        }
        QPushButton:pressed {
            background-color: #6c7a7d;
        }
    )");

    btnLayout->addStretch();
    btnLayout->addWidget(okBtn);
    btnLayout->addWidget(cancelBtn);
    btnLayout->addStretch();

    mainLayout->addLayout(btnLayout);

    // 填充现有数据
    if (task) {
        titleEdit->setText(task->title);
        descEdit->setPlainText(task->description);
        categoryCombo->setCurrentIndex(categoryCombo->findData(task->categoryId));
        priorityCombo->setCurrentIndex(priorityCombo->findData(static_cast<int>(task->priority)));
        statusCombo->setCurrentIndex(statusCombo->findData(static_cast<int>(task->status)));
        durationSpin->setValue(task->workDuration);
        tagsEdit->setText(task->tags.join(", "));
    }

    connect(okBtn, &QPushButton::clicked, &dialog, &QDialog::accept);
    connect(cancelBtn, &QPushButton::clicked, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        Task newTask;
        if (task) {
            newTask = *task;
        }

        newTask.title = titleEdit->text();
        newTask.description = descEdit->toPlainText();
        newTask.categoryId = categoryCombo->currentData().toInt();
        newTask.priority = static_cast<TaskPriority>(priorityCombo->currentData().toInt());
        newTask.status = static_cast<TaskStatus>(statusCombo->currentData().toInt());
        newTask.workDuration = durationSpin->value();
        newTask.tags = tagsEdit->text().split(",", Qt::SkipEmptyParts);

        if (task) {
            db->updateTask(newTask);
        } else {
            db->addTask(newTask);
        }

        refreshTaskTable();
        updateStatistics();
    }
}

void WorkLogWidget::showCategoryDialog(Category *category)
{
    QDialog dialog(this);
    dialog.setWindowTitle(category ? "编辑分类" : "新建分类");
    dialog.resize(400, 300);

    QFormLayout *formLayout = new QFormLayout(&dialog);

    QLineEdit *nameEdit = new QLineEdit(&dialog);
    QTextEdit *descEdit = new QTextEdit(&dialog);
    QLineEdit *colorEdit = new QLineEdit(&dialog);

    colorEdit->setPlaceholderText("例如: #3498db");

    formLayout->addRow("名称:", nameEdit);
    formLayout->addRow("描述:", descEdit);
    formLayout->addRow("颜色:", colorEdit);

    if (category) {
        nameEdit->setText(category->name);
        descEdit->setPlainText(category->description);
        colorEdit->setText(category->color);
    } else {
        colorEdit->setText("#3498db");
    }

    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *okBtn = new QPushButton("确定", &dialog);
    QPushButton *cancelBtn = new QPushButton("取消", &dialog);
    btnLayout->addWidget(okBtn);
    btnLayout->addWidget(cancelBtn);
    formLayout->addRow(btnLayout);

    connect(okBtn, &QPushButton::clicked, &dialog, &QDialog::accept);
    connect(cancelBtn, &QPushButton::clicked, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        Category newCategory;
        if (category) {
            newCategory = *category;
        }

        newCategory.name = nameEdit->text();
        newCategory.description = descEdit->toPlainText();
        newCategory.color = colorEdit->text();
        newCategory.parentId = -1;

        if (category) {
            db->updateCategory(newCategory);
        } else {
            newCategory.sortOrder = db->getAllCategories().size();
            db->addCategory(newCategory);
        }

        refreshCategoryTree();
    }
}

QString WorkLogWidget::generateWeeklyReport(const QDateTime &startDate, const QDateTime &endDate)
{
    QString report;

    QDate start = startDate.date();
    QDate end = endDate.date();

    int weekNumber = start.weekNumber();

    report += QString("# 第%1周工作总结（%2-%3）\n\n")
        .arg(weekNumber)
        .arg(start.toString("yyyy.MM.dd"))
        .arg(end.toString("yyyy.MM.dd"));

    QList<Task> tasks = db->getTasksByDateRange(startDate, endDate);

    QHash<QString, QList<Task>> categoryTasks;
    for (const Task &task : tasks) {
        if (task.status == TaskStatus_Completed) {
            Category cat = db->getCategoryById(task.categoryId);
            QString categoryName = cat.id != -1 ? cat.name : "未分类";
            categoryTasks[categoryName].append(task);
        }
    }

    report += "## 一、本周完成工作\n\n";

    QStringList categoryNames = categoryTasks.keys();
    std::sort(categoryNames.begin(), categoryNames.end());

    for (const QString &categoryName : categoryNames) {
        report += QString("### 【%1】\n").arg(categoryName);
        int index = 1;
        for (const Task &task : categoryTasks[categoryName]) {
            report += QString("%1. %2").arg(index).arg(task.title);
            if (!task.description.isEmpty()) {
                report += QString(" - %1").arg(task.description);
            }
            report += QString("（%1）\n").arg(getDurationString(task.workDuration));
            index++;
        }
        report += "\n";
    }

    report += "## 二、本周工作统计\n\n";

    int totalTasks = 0;
    double totalHours = 0.0;

    for (const QString &categoryName : categoryNames) {
        int count = categoryTasks[categoryName].size();
        double hours = 0.0;
        for (const Task &task : categoryTasks[categoryName]) {
            hours += task.workDuration;
        }

        report += QString("- %1类：%2个任务，%3\n")
            .arg(categoryName)
            .arg(count)
            .arg(getDurationString(hours));

        totalTasks += count;
        totalHours += hours;
    }

    report += QString("\n- 完成任务总数：%1\n").arg(totalTasks);
    report += QString("- 总工作时长：%1\n\n").arg(getDurationString(totalHours));

    report += "## 三、下周计划\n\n";

    QList<Task> todoTasks = db->getTasksByStatus(TaskStatus_Todo);
    if (todoTasks.isEmpty()) {
        report += "暂无待办任务\n";
    } else {
        int index = 1;
        for (const Task &task : todoTasks) {
            report += QString("%1. %2").arg(index).arg(task.title);
            if (!task.description.isEmpty()) {
                report += QString(" - %1").arg(task.description);
            }
            report += "\n";
            index++;
            if (index > 10) break;
        }
    }

    return report;
}

QString WorkLogWidget::generateMonthlyReport(const QDateTime &startDate, const QDateTime &endDate)
{
    QString report;

    QDate start = startDate.date();
    QDate end = endDate.date();

    report += QString("# %1工作总结（%2-%3）\n\n")
        .arg(start.toString("yyyy年MM月"))
        .arg(start.toString("yyyy.MM.dd"))
        .arg(end.toString("yyyy.MM.dd"));

    QList<Task> tasks = db->getTasksByDateRange(startDate, endDate);

    QHash<QString, QList<Task>> categoryTasks;
    for (const Task &task : tasks) {
        if (task.status == TaskStatus_Completed) {
            Category cat = db->getCategoryById(task.categoryId);
            QString categoryName = cat.id != -1 ? cat.name : "未分类";
            categoryTasks[categoryName].append(task);
        }
    }

    report += "## 一、本月完成工作\n\n";

    QStringList categoryNames = categoryTasks.keys();
    std::sort(categoryNames.begin(), categoryNames.end());

    for (const QString &categoryName : categoryNames) {
        report += QString("### 【%1】\n").arg(categoryName);
        int index = 1;
        for (const Task &task : categoryTasks[categoryName]) {
            report += QString("%1. %2").arg(index).arg(task.title);
            if (!task.description.isEmpty()) {
                report += QString(" - %1").arg(task.description);
            }
            report += QString("（%1）\n").arg(getDurationString(task.workDuration));
            index++;
        }
        report += "\n";
    }

    report += "## 二、本月工作统计\n\n";

    int totalTasks = 0;
    double totalHours = 0.0;

    for (const QString &categoryName : categoryNames) {
        int count = categoryTasks[categoryName].size();
        double hours = 0.0;
        for (const Task &task : categoryTasks[categoryName]) {
            hours += task.workDuration;
        }

        report += QString("- %1类：%2个任务，%3\n")
            .arg(categoryName)
            .arg(count)
            .arg(getDurationString(hours));

        totalTasks += count;
        totalHours += hours;
    }

    report += QString("\n- 完成任务总数：%1\n").arg(totalTasks);
    report += QString("- 总工作时长：%1\n\n").arg(getDurationString(totalHours));

    report += "## 三、下月计划\n\n";

    QList<Task> todoTasks = db->getTasksByStatus(TaskStatus_Todo);
    if (todoTasks.isEmpty()) {
        report += "暂无待办任务\n";
    } else {
        int index = 1;
        for (const Task &task : todoTasks) {
            report += QString("%1. %2").arg(index).arg(task.title);
            if (!task.description.isEmpty()) {
                report += QString(" - %1").arg(task.description);
            }
            report += "\n";
            index++;
            if (index > 10) break;
        }
    }

    return report;
}

bool WorkLogWidget::exportToMarkdown(const QString &content, const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, "错误", "无法保存文件");
        return false;
    }

    QByteArray utf8Data = content.toUtf8();
    file.write(utf8Data);
    file.close();

    QMessageBox::information(this, "成功", "报告已成功导出");
    return true;
}

void WorkLogWidget::onTimeFilterChanged(int index)
{
    if (!timeFilterCombo || !customStartDate || !customEndDate) {
        return;
    }

    TimePeriod selectedPeriod = static_cast<TimePeriod>(timeFilterCombo->itemData(index).toInt());

    int dayOfWeek, dayOfMonth, dayOfYear;

    switch (selectedPeriod) {
        case Today:
            customStartDate->setDate(QDate::currentDate());
            customEndDate->setDate(QDate::currentDate());
            break;
        case ThisWeek:
            dayOfWeek = QDate::currentDate().dayOfWeek();
            customStartDate->setDate(QDate::currentDate().addDays(-(dayOfWeek - 1)));
            customEndDate->setDate(QDate::currentDate());
            break;
        case ThisMonth:
            dayOfMonth = QDate::currentDate().day();
            customStartDate->setDate(QDate::currentDate().addDays(-(dayOfMonth - 1)));
            customEndDate->setDate(QDate::currentDate());
            break;
        case ThisYear:
            dayOfYear = QDate::currentDate().dayOfYear();
            customStartDate->setDate(QDate::currentDate().addDays(-(dayOfYear - 1)));
            customEndDate->setDate(QDate::currentDate());
            break;
        case Custom:
            break;
    }

    updateStatistics();
}

void WorkLogWidget::onViewDateChanged(const QDate &date)
{
    refreshTaskTable();
}

void WorkLogWidget::onPrevDay()
{
    if (taskViewDate) {
        taskViewDate->setDate(taskViewDate->date().addDays(-1));
    }
}

void WorkLogWidget::onNextDay()
{
    if (taskViewDate) {
        taskViewDate->setDate(taskViewDate->date().addDays(1));
    }
}

void WorkLogWidget::onToday()
{
    if (taskViewDate) {
        taskViewDate->setDate(QDate::currentDate());
    }
}

bool WorkLogWidget::exportToText(const QString &content, const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, "错误", "无法保存文件");
        return false;
    }

    QByteArray utf8Data = content.toUtf8();
    file.write(utf8Data);
    file.close();

    QMessageBox::information(this, "成功", "报告已成功导出");
    return true;
}
