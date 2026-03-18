#include "worklogwidget.h"
#include "modules/core/aiconfig.h"
#include <QDateTime>

// 更新分类筛选下拉框显示文本（函数实现）
void updateCategoryFilterText(QComboBox *comboBox, QStandardItemModel *model)
{
    int checkedCount = 0;
    int totalCount = 0;
    for (int i = 0; i < model->rowCount(); ++i) {
        QStandardItem *item = model->item(i);
        if (item->isCheckable()) {
            totalCount++;
            if (item->checkState() == Qt::Checked) {
                checkedCount++;
            }
        }
    }

    if (checkedCount == 0) {
        comboBox->setCurrentText("无分类");
    } else if (checkedCount == totalCount) {
        comboBox->setCurrentText("全选 (" + QString::number(checkedCount) + ")");
    } else {
        comboBox->setCurrentText("已选 " + QString::number(checkedCount) + " 个分类");
    }
}
#include <QTimer>
#include <QMessageBox>
#include <QInputDialog>
#include <QTextStream>
#include <QTextCodec>
#include <QDesktopServices>
#include <QUrl>
#include <QDebug>
#include <QAxObject>
#include <QAxWidget>
#include <QTextDocument>
#include <QTextCursor>
#include <QTextBlock>
#include <QTextTable>
#include <QGraphicsDropShadowEffect>
#include <QPropertyAnimation>
#include <QPainter>
#include <QStyleOption>
#include <QSet>
#include <algorithm>
#include <QtCharts>
#include <QHostInfo>
#include <QCryptographicHash>
#include <QSettings>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QPointer>
#include <QFile>
#include <QDir>
#include <QPrinter>
#include <QTextDocument>
#include <QMarginsF>
#include <QPageSize>
#include <QPageLayout>
#include <QRegularExpression>
#include <QCoreApplication>
using namespace QtCharts;

WorkLogWidget::WorkLogWidget(Database *database, QWidget *parent)
    : QWidget(parent), db(database), currentRunningTask(nullptr)
{
    // 初始化所有指针成员
    mainSplitter = nullptr;
    leftSplitter = nullptr;
    leftPanel = nullptr;
    rightPanel = nullptr;
    taskTable = nullptr;
    viewStacker = nullptr;
    calendarViewContainer = nullptr;
    calendarWidget = nullptr;
    viewModeGroup = nullptr;
    tableViewBtn = nullptr;
    calendarViewBtn = nullptr;
    monthDateEdit = nullptr;
    tableDateWidget = nullptr;
    calendarNavWidget = nullptr;
    tableDateLayout = nullptr;
    calendarNavLayout = nullptr;
    calendarNavLayout = nullptr;
    prevMonthBtn = nullptr;
    nextMonthBtn = nullptr;
    goTodayBtn = nullptr;
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
    networkManager = nullptr;

    // 初始化当前日历月份
    currentCalendarMonth = QDate::currentDate();

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
    networkManager = new QNetworkAccessManager(this);
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

        QComboBox {
            padding: 6px 28px 6px 10px;
            border: 1px solid #dde4eb;
            border-radius: 6px;
            background-color: #ffffff;
            color: #2d3436;
            min-width: 130px;
        }

        QComboBox:focus {
            border-color: #3498db;
            background-color: #fdfefe;
        }

        QComboBox:hover {
            border-color: #b2bec3;
        }

        QComboBox::drop-down {
            subcontrol-origin: padding;
            subcontrol-position: right center;
            width: 26px;
            border-left: none;
            background: transparent;
            margin-right: 4px;
        }

        QComboBox::down-arrow {
            image: none;
            width: 0;
            height: 0;
            border-left: 5px solid transparent;
            border-right: 5px solid transparent;
            border-top: 6px solid #7f8c8d;
            margin-right: 8px;
        }

        QComboBox QAbstractItemView {
            border: 1px solid #dde4eb;
            background-color: #ffffff;
            selection-background-color: #3498db;
            border-radius: 4px;
        }

        QComboBox QAbstractItemView::item {
            min-height: 28px;
            padding: 4px 10px;
        }

        QComboBox QAbstractItemView::item:selected {
            background-color: #3498db;
            color: #ffffff;
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
            color: #2c3e50;
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

    // 左侧：工作汇总统计面板
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
    timeFilterCombo->setStyle(new ComboBoxArrowStyle(timeFilterCombo->style()));
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
    pieChart->setTitle("分类时间分布");
    pieChart->setAnimationOptions(QChart::SeriesAnimations);
    pieChart->setTheme(QChart::ChartThemeLight);

    // 设置图例显示在下方
    pieChart->legend()->setAlignment(Qt::AlignBottom);

    // 调整饼图大小，留出下方空间给图例
    pieChart->setBackgroundBrush(QBrush(Qt::white));

    pieChartView = new QChartView(pieChart, statisticsPanel);
    pieChartView->setRenderHint(QPainter::Antialiasing);
    pieChartView->setMinimumHeight(300);
    pieChartView->setInteractive(true);  // 启用交互
    pieChartView->setMouseTracking(true);
    pieChartView->installEventFilter(this);  // 安装事件过滤器
    chartLayout->addWidget(pieChartView);

    chartLayout->addStretch();
    statisticsLayout->addWidget(chartGroup);

    leftMainLayout->addWidget(statisticsPanel);

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
    filterLayout->addWidget(searchEdit, 3);

    // 分类筛选下拉框
    categoryFilter = new QComboBox(this);
    categoryFilter->setStyle(new ComboBoxArrowStyle(categoryFilter->style()));
    categoryFilter->setFixedWidth(60);

    filterLayout->addWidget(new QLabel("分类:", this));
    filterLayout->addWidget(categoryFilter);

    statusFilter = new QComboBox(this);
    statusFilter->setStyle(new ComboBoxArrowStyle(statusFilter->style()));
    statusFilter->setFixedWidth(60);
    statusFilter->addItem("全部状态", -1);
    statusFilter->addItem("📝 待办", static_cast<int>(TaskStatus_Todo));
    statusFilter->addItem("🔄 进行中", static_cast<int>(TaskStatus_InProgress));
    statusFilter->addItem("⏸️ 暂停", static_cast<int>(TaskStatus_Paused));
    statusFilter->addItem("✅ 已完成", static_cast<int>(TaskStatus_Completed));
    filterLayout->addWidget(new QLabel("状态:", this));
    filterLayout->addWidget(statusFilter);

    priorityFilter = new QComboBox(this);
    priorityFilter->setStyle(new ComboBoxArrowStyle(priorityFilter->style()));
    priorityFilter->setFixedWidth(60);
    priorityFilter->addItem("全部优先级", -1);
    priorityFilter->addItem("🔵 低", static_cast<int>(TaskPriority_Low));
    priorityFilter->addItem("🟡 中", static_cast<int>(TaskPriority_Medium));
    priorityFilter->addItem("🔴 高", static_cast<int>(TaskPriority_High));
    filterLayout->addWidget(new QLabel("优先级:", this));
    filterLayout->addWidget(priorityFilter);

    rightMainLayout->addLayout(filterLayout);

    // 表格视图日期选择器（用QWidget包装以控制可见性）
    tableDateWidget = new QWidget(this);
    tableDateLayout = new QHBoxLayout(tableDateWidget);
    tableDateLayout->setSpacing(8);
    tableDateLayout->setContentsMargins(10, 5, 10, 5);
    tableDateLayout->addWidget(new QLabel("📅 查看月份:", tableDateWidget));

    taskViewDate = new QDateEdit(tableDateWidget);
    taskViewDate->setCalendarPopup(true);
    taskViewDate->setDate(QDate::currentDate());
    taskViewDate->setDisplayFormat("yyyy-MM");
    taskViewDate->setMinimumWidth(120);
    tableDateLayout->addWidget(taskViewDate);

    QPushButton *prevDayBtn = new QPushButton("◀ 上月", tableDateWidget);
    prevDayBtn->setStyleSheet("padding: 6px 12px;");
    tableDateLayout->addWidget(prevDayBtn);

    QPushButton *nextDayBtn = new QPushButton("下月 ▶", tableDateWidget);
    nextDayBtn->setStyleSheet("padding: 6px 12px;");
    tableDateLayout->addWidget(nextDayBtn);

    QPushButton *todayBtn = new QPushButton("今天", tableDateWidget);
    todayBtn->setStyleSheet("padding: 6px 12px;");
    tableDateLayout->addWidget(todayBtn);

    tableDateLayout->addStretch();
    rightMainLayout->addWidget(tableDateWidget);

    // 日历视图月份导航（放在tableDateWidget同一位置，通过可见性切换）
    calendarNavWidget = new QWidget(this);
    calendarNavLayout = new QHBoxLayout(calendarNavWidget);
    calendarNavLayout->setSpacing(8);
    calendarNavLayout->setContentsMargins(10, 5, 10, 5);

    // 年月选择器 - 样式与表格视图日期选择器一致
    monthDateEdit = new QDateEdit(calendarNavWidget);
    monthDateEdit->setCalendarPopup(true);
    monthDateEdit->setDate(QDate::currentDate());
    monthDateEdit->setDisplayFormat("yyyy-MM");
    monthDateEdit->setMinimumWidth(120);
    connect(monthDateEdit, &QDateEdit::dateChanged, this, &WorkLogWidget::onMonthChanged);

    // 上月按钮 - 样式与表格视图"前一天"按钮一致
    prevMonthBtn = new QPushButton("◀ 上月", calendarNavWidget);
    prevMonthBtn->setStyleSheet("padding: 6px 12px;");
    connect(prevMonthBtn, &QPushButton::clicked, this, &WorkLogWidget::onPrevMonth);

    // 下月按钮 - 样式与表格视图"后一天"按钮一致
    nextMonthBtn = new QPushButton("下月 ▶", calendarNavWidget);
    nextMonthBtn->setStyleSheet("padding: 6px 12px;");
    connect(nextMonthBtn, &QPushButton::clicked, this, &WorkLogWidget::onNextMonth);

    // 今天按钮 - 样式与表格视图"今天"按钮一致
    goTodayBtn = new QPushButton("今天", calendarNavWidget);
    goTodayBtn->setStyleSheet("padding: 6px 12px;");
    connect(goTodayBtn, &QPushButton::clicked, this, &WorkLogWidget::onGoToToday);

    calendarNavLayout->addWidget(monthDateEdit);
    calendarNavLayout->addWidget(prevMonthBtn);
    calendarNavLayout->addWidget(nextMonthBtn);
    calendarNavLayout->addWidget(goTodayBtn);
    calendarNavLayout->addStretch();
    calendarNavWidget->setVisible(false);  // 初始隐藏

    rightMainLayout->addWidget(calendarNavWidget);  // 与tableDateWidget同一位置

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

    // 添加视图切换按钮
    QLabel *viewModeLabel = new QLabel("视图:", this);
    viewModeLabel->setStyleSheet("font-weight: bold;");
    taskBtnLayout->addWidget(viewModeLabel);

    viewModeGroup = new QButtonGroup(this);
    tableViewBtn = new QPushButton("📊 表格", this);
    tableViewBtn->setCheckable(true);
    tableViewBtn->setChecked(true);
    tableViewBtn->setStyleSheet("QPushButton { padding: 4px 12px; }");
    calendarViewBtn = new QPushButton("📅 日历", this);
    calendarViewBtn->setCheckable(true);
    calendarViewBtn->setStyleSheet("QPushButton { padding: 4px 12px; }");

    viewModeGroup->addButton(tableViewBtn, 0);
    viewModeGroup->addButton(calendarViewBtn, 1);
    taskBtnLayout->addWidget(tableViewBtn);
    taskBtnLayout->addWidget(calendarViewBtn);

    taskBtnLayout->addStretch();

    rightMainLayout->addLayout(taskBtnLayout);

    // 创建视图切换器
    setupTaskTable();
    setupCalendarView();

    viewStacker = new QStackedWidget(this);
    viewStacker->addWidget(taskTable);
    viewStacker->addWidget(calendarViewContainer);

    rightMainLayout->addWidget(viewStacker);

    // 初始刷新日历视图
    refreshCalendarView();

    // 连接视图切换信号槽
    connect(viewModeGroup, QOverload<int>::of(&QButtonGroup::buttonClicked),
            this, &WorkLogWidget::onViewModeChanged);

    // 报告按钮
    QHBoxLayout *reportBtnLayout = new QHBoxLayout();
    reportBtnLayout->setSpacing(6);
    generateReportBtn = new QPushButton("📊 生成报告", this);
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

    connect(taskViewDate, &QDateEdit::dateChanged, this, &WorkLogWidget::onViewDateChanged);
    connect(prevDayBtn, &QPushButton::clicked, this, &WorkLogWidget::onPrevDay);
    connect(nextDayBtn, &QPushButton::clicked, this, &WorkLogWidget::onNextDay);
    connect(todayBtn, &QPushButton::clicked, this, &WorkLogWidget::onToday);

    connect(taskTable, &QTableWidget::itemSelectionChanged, this, &WorkLogWidget::onTaskSelectionChanged);
    connect(taskTable, &QTableWidget::cellDoubleClicked, this, &WorkLogWidget::onTaskDoubleClicked);

    connect(searchEdit, &QLineEdit::textChanged, this, &WorkLogWidget::onFilterChanged);
    connect(statusFilter, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &WorkLogWidget::onFilterChanged);
    connect(priorityFilter, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &WorkLogWidget::onFilterChanged);
    connect(categoryFilter, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &WorkLogWidget::onFilterChanged);
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
    taskTable->setColumnCount(8);
    taskTable->setHorizontalHeaderLabels({"🔢 序号", "📝 标题", "📁 分类", "🎯 优先级", "📊 状态", "⏱️ 工时", "🏷️ 标签", "📅 完成时间"});

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
    taskTable->setColumnWidth(7, 160);

    // 启用自动换行
    taskTable->setWordWrap(true);

    // 设置图标大小
    taskTable->setIconSize(QSize(16, 16));

    // 设置单元格对齐方式
    taskTable->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
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

void WorkLogWidget::setupCalendarView()
{
    // 创建日历视图容器
    calendarViewContainer = new QWidget(this);
    QVBoxLayout *calendarLayout = new QVBoxLayout(calendarViewContainer);
    calendarLayout->setContentsMargins(5, 5, 5, 5);
    calendarLayout->setSpacing(5);

    // 创建日历Widget
    calendarWidget = new CalendarWidget(calendarViewContainer);
    calendarLayout->addWidget(calendarWidget);

    // 连接日期双击信号（跳转到表格视图）
    connect(calendarWidget, &CalendarWidget::dateDoubleClicked,
            this, &WorkLogWidget::onCalendarDateSelected);

    // 初始刷新日历
    refreshCalendarView();
}

void WorkLogWidget::refreshCalendarView()
{
    if (!calendarWidget || !monthDateEdit) return;

    // 更新年月选择器
    monthDateEdit->setDate(currentCalendarMonth);

    // 设置日历月份
    calendarWidget->setMonth(currentCalendarMonth);

    // 获取当前月份的任务
    QDate firstDayOfMonth(currentCalendarMonth.year(), currentCalendarMonth.month(), 1);
    QDate lastDayOfMonth = firstDayOfMonth.addMonths(1).addDays(-1);

    QList<Task> monthTasks = db->getTasksByDateRange(
        QDateTime(firstDayOfMonth, QTime(0, 0, 0)),
        QDateTime(lastDayOfMonth, QTime(23, 59, 59)));

    // 按日期组织任务
    QMap<QDate, QVector<QPair<QString, TaskStatus>>> tasksByDate;
    for (const Task &task : monthTasks) {
        if (task.completionTime.isValid()) {
            QDate taskDate = task.completionTime.date();
            tasksByDate[taskDate].append(qMakePair(task.title, task.status));
        }
    }

    // 设置任务信息到日历
    calendarWidget->setTaskInfos(tasksByDate);
}

void WorkLogWidget::onPrevMonth()
{
    currentCalendarMonth = currentCalendarMonth.addMonths(-1);
    refreshCalendarView();
}

void WorkLogWidget::onNextMonth()
{
    currentCalendarMonth = currentCalendarMonth.addMonths(1);
    refreshCalendarView();
}

void WorkLogWidget::onMonthChanged(const QDate &date)
{
    currentCalendarMonth = QDate(date.year(), date.month(), 1);
    calendarWidget->setMonth(currentCalendarMonth);

    // 刷新任务数据
    QDate firstDayOfMonth = currentCalendarMonth;
    QDate lastDayOfMonth = firstDayOfMonth.addMonths(1).addDays(-1);

    QList<Task> monthTasks = db->getTasksByDateRange(
        QDateTime(firstDayOfMonth, QTime(0, 0, 0)),
        QDateTime(lastDayOfMonth, QTime(23, 59, 59)));

    QMap<QDate, QVector<QPair<QString, TaskStatus>>> tasksByDate;
    for (const Task &task : monthTasks) {
        if (task.completionTime.isValid()) {
            QDate taskDate = task.completionTime.date();
            tasksByDate[taskDate].append(qMakePair(task.title, task.status));
        }
    }
    calendarWidget->setTaskInfos(tasksByDate);
}

void WorkLogWidget::onGoToToday()
{
    currentCalendarMonth = QDate::currentDate();
    calendarWidget->setSelectedDate(QDate::currentDate());
    refreshCalendarView();
}

void WorkLogWidget::onCalendarDateSelected(const QDate &date)
{
    // 双击日历日期跳转到表格视图并设置对应日期
    if (taskViewDate) {
        taskViewDate->setDate(date);
    }
    // 切换到表格视图
    if (viewStacker) {
        viewStacker->setCurrentIndex(0);
    }
    if (tableViewBtn) {
        tableViewBtn->setChecked(true);
    }
    if (calendarViewBtn) {
        calendarViewBtn->setChecked(false);
    }
    // 显示表格视图的日期控件
    if (tableDateWidget && calendarNavWidget) {
        tableDateWidget->setVisible(true);
        calendarNavWidget->setVisible(false);
    }
    // 刷新任务列表
    loadTasks();
}

void WorkLogWidget::onViewModeChanged(int index)
{
    if (viewStacker) {
        viewStacker->setCurrentIndex(index);
    }

    // 根据视图模式显示/隐藏对应的导航控件
    if (tableDateWidget && calendarNavWidget) {
        if (index == 0) {
            // 表格视图：显示日期选择器，隐藏月份导航
            tableDateWidget->setVisible(true);
            calendarNavWidget->setVisible(false);
        } else {
            // 日历视图：隐藏日期选择器，显示月份导航
            tableDateWidget->setVisible(false);
            calendarNavWidget->setVisible(true);
        }
    }
}

void WorkLogWidget::loadCategories()
{
    // 加载分类到下拉框
    categoryFilter->clear();
    categoryFilter->addItem("全部分类", -1);

    QList<Category> categories = db->getAllCategories();
    for (const Category &category : categories) {
        categoryFilter->addItem(category.name, category.id);
    }
}

void WorkLogWidget::loadTasks()
{
    refreshTaskTable();
}

void WorkLogWidget::refreshTaskTable()
{
    taskTable->setRowCount(0);

    QDate viewDate = taskViewDate->date();
    QList<Task> allTasks = db->getTasksForDate(viewDate);

    QString searchText = searchEdit->text().toLower();
    int statusValue = statusFilter->currentData().toInt();
    int priorityValue = priorityFilter->currentData().toInt();

    QString viewDateStr = viewDate.toString("yyyyMMdd");

    // 获取选中的分类ID
    int selectedCategoryId = categoryFilter->currentData().toInt();

    QSet<QString> displayedTaskIds;

    for (const Task &task : allTasks) {
        QString taskDateStr = task.id.left(8);

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

        // 分类筛选：如果选中了分类（不为-1），则只显示该分类
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
    
    QString completionText;
    if (task.completionTime.isValid()) {
        completionText = task.completionTime.toString("yyyy-MM-dd hh:mm:ss");
    } else {
        completionText = "-";
    }
    taskTable->setItem(row, 7, new QTableWidgetItem(completionText));

    for (int col = 0; col < taskTable->columnCount(); ++col) {
        if (taskTable->item(row, col)) {
            taskTable->item(row, col)->setData(Qt::UserRole, task.id);
        }
    }
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
                pieChart->legend()->setAlignment(Qt::AlignBottom);
                pieChart->legend()->setFont(QFont("Microsoft YaHei", 9));
                pieChart->legend()->setContentsMargins(5, 5, 5, 5);
                pieChart->legend()->setBackgroundVisible(false);

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
        todayProgress->setFormat(QString("本日 %1 %2 小时 (%3%)").arg(statusText).arg(todayHours, 0, 'f', 1).arg(percentage));
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
        weekProgress->setFormat(QString("本周 %1 %2 小时 (%3%)").arg(statusText).arg(weekHours, 0, 'f', 1).arg(percentage));
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

void WorkLogWidget::onGenerateReport()
{
    if (!checkPermission("generate_report")) {
        QMessageBox::warning(this, "权限不足", "您没有生成报告的权限");
        return;
    }
    
    bool ok;
    QString reportType = QInputDialog::getItem(this, "选择报告类型", "请选择要生成的报告类型:",
                                               {"周报", "月报", "季报"}, 0, false, &ok);
    if (!ok) return;

    QDate startDate, endDate;
    if (reportType == "周报") {
        QDate today = QDate::currentDate();
        int dayOfWeek = today.dayOfWeek();
        startDate = today.addDays(-(dayOfWeek - 1));
        endDate = startDate.addDays(6);
    } else if (reportType == "月报") {
        QDate today = QDate::currentDate();
        startDate = QDate(today.year(), today.month(), 1);
        endDate = QDate(today.year(), today.month(), today.daysInMonth());
    } else {
        QDate today = QDate::currentDate();
        int currentMonth = today.month();
        int quarter = (currentMonth - 1) / 3 + 1;
        int startMonth = (quarter - 1) * 3 + 1;
        startDate = QDate(today.year(), startMonth, 1);
        endDate = QDate(today.year(), startMonth + 2, 1).addDays(-1);
    }

    QString cacheKey = QString("%1_%2_%3").arg(reportType).arg(startDate.toString("yyyyMMdd")).arg(endDate.toString("yyyyMMdd"));
    QString reportContent = getCachedReport(cacheKey);
    
    bool fromCache = !reportContent.isEmpty();
    
    if (!fromCache) {
        if (reportType == "周报") {
            reportContent = generateWeeklyReport(QDateTime(startDate), QDateTime(endDate));
        } else if (reportType == "月报") {
            reportContent = generateMonthlyReport(QDateTime(startDate), QDateTime(endDate));
        } else {
            reportContent = generateQuarterlyReport(QDateTime(startDate), QDateTime(endDate));
        }
        cacheReport(cacheKey, reportContent);
        logOperation("generate_report", QString("类型: %1, 时间范围: %2 - %3, 来源: %4")
                    .arg(reportType)
                    .arg(startDate.toString("yyyy-MM-dd"))
                    .arg(endDate.toString("yyyy-MM-dd"))
                    .arg(fromCache ? "缓存" : "新生成"));
    }

    QDialog reportDialog(this);
    reportDialog.setWindowTitle(reportType);
    reportDialog.resize(900, 700);

    QVBoxLayout *layout = new QVBoxLayout(&reportDialog);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *aiGenerateBtn = new QPushButton("🤖 AI生成分析", &reportDialog);

    QComboBox *aiModelCombo = new QComboBox(&reportDialog);
    aiModelCombo->setStyle(new ComboBoxArrowStyle(aiModelCombo->style()));
    aiModelCombo->setToolTip("选择AI模型");
    aiModelCombo->setStyleSheet(R"(
        QComboBox {
            min-width: 150px;
            padding: 4px;
        }
    )");
    loadAIKeysToComboBox(aiModelCombo);

    QPushButton *exportPDFBtn = new QPushButton("📄 导出PDF", &reportDialog);
    QPushButton *exportWordBtn = new QPushButton("📝 导出Word", &reportDialog);
    QPushButton *exportMarkdownBtn = new QPushButton("📋 导出Markdown", &reportDialog);
    QPushButton *copyBtn = new QPushButton("复制", &reportDialog);
    QPushButton *closeBtn = new QPushButton("关闭", &reportDialog);
    btnLayout->addWidget(aiGenerateBtn);
    btnLayout->addWidget(aiModelCombo);
    btnLayout->addWidget(exportPDFBtn);
    btnLayout->addWidget(exportWordBtn);
    btnLayout->addWidget(exportMarkdownBtn);
    btnLayout->addWidget(copyBtn);
    btnLayout->addWidget(closeBtn);
    layout->addLayout(btnLayout);

    QLabel *cacheLabel = new QLabel(&reportDialog);
    if (fromCache) {
        cacheLabel->setText("💾 从缓存加载");
        cacheLabel->setStyleSheet("color: #666; font-size: 11px;");
    }
    layout->addWidget(cacheLabel);

    QTextEdit *reportEdit = new QTextEdit(&reportDialog);
    reportEdit->setPlainText(reportContent);
    layout->addWidget(reportEdit);

    connect(reportEdit, &QTextEdit::textChanged, [this, cacheLabel, cacheKey]() {
        clearReportCache();
        if (cacheLabel) {
            cacheLabel->setText("✏️ 已编辑");
        }
    });

    connect(aiGenerateBtn, &QPushButton::clicked, [this, reportEdit, reportType, reportContent, aiModelCombo]() {
        QString currentModel = aiModelCombo->currentData().toString();
        onAIGenerateReport(currentModel, reportEdit, reportType, reportContent);
    });

    connect(exportPDFBtn, &QPushButton::clicked, [this, reportEdit, reportType]() {
        QString defaultName = reportType + "_" + QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss") + ".pdf";
        QString fileName = QFileDialog::getSaveFileName(this, "导出PDF", defaultName, "PDF文件 (*.pdf)");
        if (!fileName.isEmpty()) {
            if (exportToPDF(reportEdit->toPlainText(), fileName)) {
                QDesktopServices::openUrl(QUrl::fromLocalFile(fileName));
            }
        }
    });

    connect(exportWordBtn, &QPushButton::clicked, [this, reportEdit, reportType]() {
        QString defaultName = reportType + "_" + QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss") + ".docx";
        QString fileName = QFileDialog::getSaveFileName(this, "导出Word", defaultName, "Word文件 (*.docx)");
        if (!fileName.isEmpty()) {
            if (exportToWord(reportEdit->toPlainText(), fileName)) {
                QDesktopServices::openUrl(QUrl::fromLocalFile(fileName));
            }
        }
    });

    connect(exportMarkdownBtn, &QPushButton::clicked, [this, reportEdit, reportType]() {
        QString defaultName = reportType + "_" + QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss") + ".md";
        QString fileName = QFileDialog::getSaveFileName(this, "导出Markdown", defaultName, "Markdown文件 (*.md)");
        if (!fileName.isEmpty()) {
            exportToMarkdown(reportEdit->toPlainText(), fileName);
        }
    });

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
                                               {"周报", "月报", "季报"}, 0, false, &ok);
    if (!ok) return;

    QDate startDate, endDate;
    if (reportType == "周报") {
        QDate today = QDate::currentDate();
        int dayOfWeek = today.dayOfWeek();
        startDate = today.addDays(-(dayOfWeek - 1));
        endDate = startDate.addDays(6);
    } else if (reportType == "月报") {
        QDate today = QDate::currentDate();
        startDate = QDate(today.year(), today.month(), 1);
        endDate = QDate(today.year(), today.month(), today.daysInMonth());
    } else {
        QDate today = QDate::currentDate();
        int currentMonth = today.month();
        int quarter = (currentMonth - 1) / 3 + 1;
        int startMonth = (quarter - 1) * 3 + 1;
        startDate = QDate(today.year(), startMonth, 1);
        endDate = QDate(today.year(), startMonth + 2, 1).addDays(-1);
    }

    QString reportContent;
    if (reportType == "周报") {
        reportContent = generateWeeklyReport(QDateTime(startDate), QDateTime(endDate));
    } else if (reportType == "月报") {
        reportContent = generateMonthlyReport(QDateTime(startDate), QDateTime(endDate));
    } else {
        reportContent = generateQuarterlyReport(QDateTime(startDate), QDateTime(endDate));
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
                    //QMessageBox::information(this, "成功", "任务已删除");
                } else {
                    //QMessageBox::warning(this, "错误", "删除任务失败");
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
    // 从 categoryFilter 中获取选中的分类
    int categoryId = categoryFilter->currentData().toInt();
    if (categoryId > 0) {
        return db->getCategoryById(categoryId);
    }

    Category category;
    category.id = -1;
    return category;
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
    
    QHBoxLayout *titleLayout = new QHBoxLayout();
    titleLayout->addWidget(titleEdit);
    
    QPushButton *aiBtn = new QPushButton("🤖 AI", &dialog);
    aiBtn->setToolTip("点击使用AI分析任务标题");
    aiBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #9b59b6;
            color: white;
            font-weight: bold;
            padding: 6px 12px;
            border-radius: 4px;
            min-width: 50px;
        }
        QPushButton:hover {
            background-color: #8e44ad;
        }
        QPushButton:pressed {
            background-color: #7d3c98;
        }
        QPushButton:disabled {
            background-color: #bdc3c7;
        }
    )");
    titleLayout->addWidget(aiBtn);
    
    QLabel *aiStatusLabel = new QLabel("", &dialog);
    aiStatusLabel->setStyleSheet("font-size: 11px; color: #7f8c8d;");
    titleLayout->addWidget(aiStatusLabel);

    QComboBox *aiModelCombo = new QComboBox(&dialog);
    aiModelCombo->setStyle(new ComboBoxArrowStyle(aiModelCombo->style()));
    aiModelCombo->setToolTip("选择AI模型");
    aiModelCombo->setStyleSheet(R"(
        QComboBox {
            min-width: 150px;
            padding: 4px;
        }
    )");
    loadAIKeysToComboBox(aiModelCombo);
    titleLayout->addWidget(aiModelCombo);
    
    QTextEdit *descEdit = new QTextEdit(&dialog);
    descEdit->setPlaceholderText("请输入任务描述");

    basicLayout->addRow("标题:", titleLayout);
    basicLayout->addRow("描述:", descEdit);

    mainLayout->addWidget(basicGroup);

    // 详细信息分组
    QGroupBox *detailGroup = new QGroupBox("📊 详细信息", &dialog);
    QFormLayout *detailLayout = new QFormLayout(detailGroup);
    detailLayout->setSpacing(10);

    QComboBox *categoryCombo = new QComboBox(&dialog);
    categoryCombo->setStyle(new ComboBoxArrowStyle(categoryCombo->style()));
    QComboBox *priorityCombo = new QComboBox(&dialog);
    priorityCombo->setStyle(new ComboBoxArrowStyle(priorityCombo->style()));
    QComboBox *statusCombo = new QComboBox(&dialog);
    statusCombo->setStyle(new ComboBoxArrowStyle(statusCombo->style()));
    QDoubleSpinBox *durationSpin = new QDoubleSpinBox(&dialog);
    QLineEdit *tagsEdit = new QLineEdit(&dialog);
    QDateEdit *completionTimeEdit = new QDateEdit(&dialog);
    completionTimeEdit->setDisplayFormat("yyyy-MM-dd");
    completionTimeEdit->setCalendarPopup(true);
    completionTimeEdit->setStyleSheet(R"(
        QDateEdit {
            padding: 8px;
            border: 1px solid #dfe6e9;
            border-radius: 4px;
            background-color: #ffffff;
            font-size: 13px;
        }
        QDateEdit:focus {
            border: 1px solid #3498db;
        }
        QDateEdit:hover {
            border: 1px solid #95a5a6;
        }
        QDateEdit:disabled {
            background-color: #f5f6fa;
            color: #bdc3c7;
        }
    )");

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
    detailLayout->addRow("完成时间:", completionTimeEdit);
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
        if (task->completionTime.isValid()) {
            completionTimeEdit->setDate(task->completionTime.date());
        } else {
            completionTimeEdit->setDate(QDate::currentDate());
        }
    } else {
        completionTimeEdit->setDate(QDate::currentDate());
    }

    auto updateCompletionTimeVisibility = [completionTimeEdit, statusCombo]() {
        bool isCompleted = statusCombo->currentData().toInt() == static_cast<int>(TaskStatus_Completed);
        completionTimeEdit->setEnabled(isCompleted);
    };
    updateCompletionTimeVisibility();
    QObject::connect(statusCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), &dialog, [updateCompletionTimeVisibility]() {
        updateCompletionTimeVisibility();
    });

    connect(aiBtn, &QPushButton::clicked, this, [this, titleEdit, descEdit, categoryCombo, priorityCombo, durationSpin, aiStatusLabel, aiBtn, tagsEdit, aiModelCombo, &dialog]() {
        QString title = titleEdit->text().trimmed();
        if (title.isEmpty()) {
            QMessageBox::warning(&dialog, "提示", "请先输入任务标题");
            return;
        }
        QString selectedModel = aiModelCombo->currentData().toString();
        if (selectedModel.isEmpty()) {
            QMessageBox::warning(&dialog, "提示", "请先配置AI API Key");
            return;
        }
        analyzeTaskWithAI(selectedModel, title, titleEdit, descEdit, categoryCombo, priorityCombo, durationSpin, aiStatusLabel, aiBtn, tagsEdit);
    });
    
    connect(okBtn, &QPushButton::clicked, &dialog, &QDialog::accept);
    connect(cancelBtn, &QPushButton::clicked, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        TaskStatus oldStatus = task ? task->status : TaskStatus_Todo;
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

        if (!task) {
            newTask.completionTime = QDateTime();
        }

        if (newTask.status == TaskStatus_Completed) {
            QDate selectedCompletionDate = completionTimeEdit->date();
            if (selectedCompletionDate > QDate::currentDate()) {
                QMessageBox::warning(&dialog, "提示", "完成时间不能晚于当前日期");
                return;
            }
            newTask.completionTime = QDateTime(selectedCompletionDate, QTime(0, 0, 0));
        } else if (oldStatus == TaskStatus_Completed && newTask.status != TaskStatus_Completed) {
            newTask.completionTime = QDateTime();
        }

        if (task) {
            db->updateTask(newTask);
            logOperation("update_task", QString("任务ID: %1, 标题: %2, 状态: %3, 完成时间: %4")
                         .arg(newTask.id)
                         .arg(newTask.title)
                         .arg(getStatusString(newTask.status))
                         .arg(newTask.completionTime.isValid() ? newTask.completionTime.toString("yyyy-MM-dd hh:mm:ss") : "无"));
            //QMessageBox::information(this, "成功", "任务已更新");
        } else {
            db->addTask(newTask);
            logOperation("add_task", QString("标题: %1, 状态: %2, 完成时间: %3")
                         .arg(newTask.title)
                         .arg(getStatusString(newTask.status))
                         .arg(newTask.completionTime.isValid() ? newTask.completionTime.toString("yyyy-MM-dd hh:mm:ss") : "无"));
            //QMessageBox::information(this, "成功", "任务已创建");
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

        loadCategories();
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

    report += "## 三、本周工作亮点\n\n";
    report += "（待补充）\n\n";

    report += "## 四、存在问题与改进\n\n";
    report += "（待补充）\n\n";

    report += "## 五、下周计划\n\n";

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

    report += "## 三、月度工作亮点\n\n";
    report += "（待补充）\n\n";

    report += "## 四、存在问题与改进\n\n";
    report += "（待补充）\n\n";

    report += "## 五、下月计划\n\n";

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

QString WorkLogWidget::generateQuarterlyReport(const QDateTime &startDate, const QDateTime &endDate)
{
    QString report;

    QDate start = startDate.date();
    QDate end = endDate.date();

    int quarter = (start.month() - 1) / 3 + 1;
    QString quarterName = QString("第%1季度").arg(quarter);

    report += QString("# %1工作总结（%2-%3）\n\n")
        .arg(quarterName)
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

    report += "## 一、季度完成工作\n\n";

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

    report += "## 二、季度工作统计\n\n";

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

    report += "## 四、季度工作亮点\n\n";
    report += "（待补充）\n\n";

    report += "## 五、存在问题与改进\n\n";
    report += "（待补充）\n\n";

    report += "## 六、下季度计划\n\n";

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
            if (index > 15) break;
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

QString WorkLogWidget::generateReportWithAI(const QString &reportType, const QString &reportData)
{
    QString prompt;
    
    if (reportType == "季报") {
        prompt = QString("你是一个专业的项目管理助手。请根据以下季度工作报告数据，生成专业的季度总结分析。\n\n"
                        "报告数据：\n%1\n\n"
                        "请生成以下内容（只返回内容，不要有任何其他文字）：\n"
                        "1. 季度工作亮点：总结本季度的重要成果、突破性进展、关键项目完成情况等（3-5点）\n"
                        "2. 存在问题与改进：分析工作中遇到的问题、挑战，提出具体的改进措施和建议（3-5点）\n"
                        "3. 季度总结：对本季度整体工作进行评价，包括工作质量、效率、团队协作等方面（2-3段）\n\n"
                        "要求：\n"
                        "- 内容要具体、有数据支撑\n"
                        "- 语言要专业、简洁\n"
                        "- 突出重点，避免空泛\n"
                        "- 每个要点都要有实际意义\n\n"
                        "只返回内容，不要其他文字。")
                        .arg(reportData);
    } else if (reportType == "月报") {
        prompt = QString("你是一个专业的项目管理助手。请根据以下月度工作报告数据，生成专业的月度总结分析。\n\n"
                        "报告数据：\n%1\n\n"
                        "请生成以下内容（只返回内容，不要有任何其他文字）：\n"
                        "1. 月度工作亮点：总结本月的重要成果、突破性进展、关键项目完成情况等（3-5点）\n"
                        "2. 存在问题与改进：分析工作中遇到的问题、挑战，提出具体的改进措施和建议（3-5点）\n"
                        "3. 月度总结：对本月整体工作进行评价，包括工作质量、效率、团队协作等方面（2-3段）\n\n"
                        "要求：\n"
                        "- 内容要具体、有数据支撑\n"
                        "- 语言要专业、简洁\n"
                        "- 突出重点，避免空泛\n"
                        "- 每个要点都要有实际意义\n\n"
                        "只返回内容，不要其他文字。")
                        .arg(reportData);
    } else {
        prompt = QString("你是一个专业的项目管理助手。请根据以下周工作报告数据，生成专业的周总结分析。\n\n"
                        "报告数据：\n%1\n\n"
                        "请生成以下内容（只返回内容，不要有任何其他文字）：\n"
                        "1. 本周工作亮点：总结本周的重要成果、突破性进展、关键项目完成情况等（3-5点）\n"
                        "2. 存在问题与改进：分析工作中遇到的问题、挑战，提出具体的改进措施和建议（3-5点）\n"
                        "3. 本周总结：对本周整体工作进行评价，包括工作质量、效率、团队协作等方面（2-3段）\n\n"
                        "要求：\n"
                        "- 内容要具体、有数据支撑\n"
                        "- 语言要专业、简洁\n"
                        "- 突出重点，避免空泛\n"
                        "- 每个要点都要有实际意义\n\n"
                        "只返回内容，不要其他文字。")
                        .arg(reportData);
    }
    
    return prompt;
}

void WorkLogWidget::onAIGenerateReport(QTextEdit *reportEdit, const QString &reportType, const QString &reportData)
{
    QString currentModel = getCurrentAIModel();
    onAIGenerateReport(currentModel, reportEdit, reportType, reportData);
}

void WorkLogWidget::onAIGenerateReport(const QString &model, QTextEdit *reportEdit, const QString &reportType, const QString &reportData)
{
    if (!reportEdit) {
        return;
    }

    QString currentModel = model;

    if (currentModel == "local") {
        QMessageBox::information(this, "提示", "本地AI模式不支持报告生成，请配置在线AI服务");
        return;
    }

    if (!networkManager) {
        networkManager = new QNetworkAccessManager(this);
    }

    AIKeyConfig keyConfig = AIConfig::instance().getKeyByModelName(currentModel);
    if (keyConfig.apiKey.isEmpty()) {
        QMessageBox::warning(this, "提示", "请先在设置中配置AI API Key");
        return;
    }

    QString endpoint = keyConfig.endpoint;
    if (endpoint.isEmpty()) {
        endpoint = getDefaultEndpoint(currentModel);
    }

    QString modelName = currentModel;

    QString prompt = generateReportWithAI(reportType, reportData);

    QJsonObject json;
    json["model"] = modelName;
    json["stream"] = false;

    QJsonArray messages;
    QJsonObject msg;
    msg["role"] = "user";
    msg["content"] = prompt;
    messages.append(msg);
    json["messages"] = messages;

    if (currentModel == "minimax" || currentModel.startsWith("qwen") || currentModel.startsWith("spark") || currentModel.startsWith("deepseek")) {
        json["max_tokens"] = 1024;
    }

    if (currentModel.startsWith("qwen")) {
        QJsonObject input;
        input["messages"] = json.take("messages");
        json["input"] = input;
        json.remove("max_tokens");
        json.remove("stream");
        json["parameters"] = QJsonObject({
            {"temperature", 0.7},
            {"max_tokens", 1024},
            {"result_format", "message"}
        });
    }

    QJsonDocument doc(json);
    QByteArray postData = doc.toJson();

    QString logDir = QCoreApplication::applicationDirPath() + "/logs";
    QDir dir(logDir);
    if (!dir.exists()) {
        dir.mkpath(logDir);
    }
    
    QString logFileName = logDir + "/ai_report_" + QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss") + ".log";
    QFile logFile(logFileName);
    if (logFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&logFile);
        out.setCodec("UTF-8");
        out << "=== AI Report Request ===" << "\n";
        out << "Time: " << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << "\n";
        out << "Model: " << currentModel << "\n";
        out << "Endpoint: " << endpoint << "\n";
        out << "Request JSON:\n" << doc.toJson(QJsonDocument::Indented) << "\n\n";
        logFile.close();
    }
    
    QNetworkRequest request;
    request.setUrl(QUrl(endpoint));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", QString("Bearer %1").arg(keyConfig.apiKey).toUtf8());

    QProgressDialog *progressDialog = new QProgressDialog("AI正在生成报告分析...", "取消", 0, 0, this);
    progressDialog->setWindowModality(Qt::WindowModal);
    progressDialog->setCancelButton(nullptr);
    progressDialog->setAttribute(Qt::WA_DeleteOnClose);
    progressDialog->show();

    QPointer<QProgressDialog> progressDialogPtr(progressDialog);
    QNetworkReply *reply = networkManager->post(request, postData);
    QPointer<QNetworkReply> replyPtr(reply);

    QTimer *timeoutTimer = new QTimer(this);
    timeoutTimer->setSingleShot(true);
    timeoutTimer->setInterval(AIConfig::instance().getTimeout() * 1000);

    connect(timeoutTimer, &QTimer::timeout, this, [replyPtr]() {
        if (replyPtr && replyPtr->isRunning()) {
            replyPtr->abort();
        }
    });

    connect(reply, &QNetworkReply::finished, this, [this, replyPtr, reportEdit, reportType, progressDialogPtr, currentModel, logFileName, timeoutTimer]() {
        if (timeoutTimer) {
            timeoutTimer->stop();
            timeoutTimer->deleteLater();
        }

        if (progressDialogPtr) {
            progressDialogPtr->close();
        }

        if (!replyPtr) {
            return;
        }

        if (replyPtr->error() != QNetworkReply::NoError) {
            QString errorMsg = replyPtr->errorString();
            if (replyPtr->error() == QNetworkReply::OperationCanceledError) {
                errorMsg = QString("请求超时（%1秒），请检查网络连接或增加超时时间").arg(AIConfig::instance().getTimeout());
            }
            QByteArray responseData = replyPtr->readAll();

            QFile logFile(logFileName);
            if (logFile.open(QIODevice::Append | QIODevice::Text)) {
                QTextStream out(&logFile);
                out.setCodec("UTF-8");
                out << "\n=== AI Report Response (ERROR) ===" << "\n";
                out << "Error Code: " << replyPtr->error() << "\n";
                out << "Error: " << errorMsg << "\n";
                out << "Response:\n" << responseData << "\n\n";
                logFile.close();
            }
            
            QMessageBox::warning(this, "错误", "AI生成失败：" + errorMsg);
            return;
        }
        
        QByteArray responseData = replyPtr->readAll();
        
        {
            QFile logFile(logFileName);
            if (logFile.open(QIODevice::Append | QIODevice::Text)) {
                QTextStream out(&logFile);
                out.setCodec("UTF-8");
                out << "\n=== AI Report Response ===" << "\n";
                out << "Response:\n" << responseData << "\n\n";
                logFile.close();
            }
        }
        
        QJsonDocument doc = QJsonDocument::fromJson(responseData);
        if (doc.isNull()) {
            QMessageBox::warning(this, "错误", "AI响应解析失败");
            return;
        }
        
        QJsonObject rootObj = doc.object();
        
        QString content;
        
        if (currentModel.startsWith("qwen")) {
            if (rootObj.contains("output") && rootObj["output"].toObject().contains("choices")) {
                QJsonArray choices = rootObj["output"].toObject()["choices"].toArray();
                if (!choices.isEmpty()) {
                    content = choices[0].toObject()["message"].toObject()["content"].toString();
                }
            }
        } else {
            QJsonObject choicesObj;
            if (rootObj.contains("output") && rootObj["output"].toObject().contains("choices")) {
                choicesObj = rootObj["output"].toObject()["choices"].toArray()[0].toObject();
            } else if (rootObj.contains("choices")) {
                choicesObj = rootObj["choices"].toArray()[0].toObject();
            } else {
                QMessageBox::warning(this, "错误", "AI返回内容格式错误");
                return;
            }
            
            content = choicesObj["message"].toObject()["content"].toString();
        }
        
        if (content.isEmpty()) {
            QMessageBox::warning(this, "错误", "AI返回内容为空");
            return;
        }
        
        QString currentReport = reportEdit->toPlainText();
        QString updatedReport;
        
        if (reportType == "季报") {
            int highlightsPos = currentReport.indexOf("## 四、季度工作亮点");
            if (highlightsPos > 0) {
                int problemsPos = currentReport.indexOf("## 五、存在问题与改进");
                if (problemsPos > highlightsPos) {
                    QString beforeHighlights = currentReport.left(highlightsPos);
                    QString afterProblems = currentReport.mid(problemsPos);
                    updatedReport = beforeHighlights + "## 四、季度工作亮点\n\n" + content + "\n\n" + afterProblems;
                }
            }
        } else if (reportType == "月报") {
            int highlightsPos = currentReport.indexOf("## 三、月度工作亮点");
            if (highlightsPos > 0) {
                int problemsPos = currentReport.indexOf("## 四、存在问题与改进");
                if (problemsPos > highlightsPos) {
                    QString beforeHighlights = currentReport.left(highlightsPos);
                    QString afterProblems = currentReport.mid(problemsPos);
                    updatedReport = beforeHighlights + "## 三、月度工作亮点\n\n" + content + "\n\n" + afterProblems;
                }
            }
        } else {
            int highlightsPos = currentReport.indexOf("## 三、本周工作亮点");
            if (highlightsPos > 0) {
                int problemsPos = currentReport.indexOf("## 四、存在问题与改进");
                if (problemsPos > highlightsPos) {
                    QString beforeHighlights = currentReport.left(highlightsPos);
                    QString afterProblems = currentReport.mid(problemsPos);
                    updatedReport = beforeHighlights + "## 三、本周工作亮点\n\n" + content + "\n\n" + afterProblems;
                }
            }
        }
        
        if (!updatedReport.isEmpty()) {
            reportEdit->setPlainText(updatedReport);
            logOperation("ai_generate_report", QString("报告类型: %1, 成功生成AI分析内容").arg(reportType));
            QMessageBox::information(this, "成功", "AI报告分析已生成，您可以在此基础上进行编辑修改");
        } else {
            logOperation("ai_generate_report", QString("报告类型: %1, 插入内容失败").arg(reportType));
            QMessageBox::warning(this, "警告", "无法将AI内容插入报告，请手动编辑");
        }
        
        replyPtr->deleteLater();
    });
}

void WorkLogWidget::analyzeTaskWithAI(const QString &title, QLineEdit *titleEdit, QTextEdit *descEdit,
                                       QComboBox *categoryCombo, QComboBox *priorityCombo, QDoubleSpinBox *durationSpin,
                                       QLabel *aiStatusLabel, QPushButton *aiBtn, QLineEdit *tagsEdit)
{
    QString currentModel = getCurrentAIModel();
    analyzeTaskWithAI(currentModel, title, titleEdit, descEdit, categoryCombo, priorityCombo, durationSpin, aiStatusLabel, aiBtn, tagsEdit);
}

void WorkLogWidget::analyzeTaskWithAI(const QString &model, const QString &title, QLineEdit *titleEdit, QTextEdit *descEdit,
                                       QComboBox *categoryCombo, QComboBox *priorityCombo, QDoubleSpinBox *durationSpin,
                                       QLabel *aiStatusLabel, QPushButton *aiBtn, QLineEdit *tagsEdit)
{
    if (!titleEdit || !descEdit || !categoryCombo || !priorityCombo || !durationSpin) {
        return;
    }

    QString currentModel = model;

    if (currentModel == "local") {
        analyzeWithLocalAI(title, titleEdit, descEdit, categoryCombo, priorityCombo, durationSpin, aiStatusLabel, tagsEdit);
        return;
    }
    
    if (!networkManager) {
        networkManager = new QNetworkAccessManager(this);
    }
    
    if (aiBtn) {
        aiBtn->setEnabled(false);
    }
    if (aiStatusLabel) {
        aiStatusLabel->setText("🤖 AI分析中...");
    }
    
    QString prompt = QString("你是一个专业的任务管理助手。请根据任务标题进行深度分析，生成详细的任务信息。\n\n"
                            "任务标题：%1\n\n"
                            "现有分类：%2\n\n"
                            "请返回以下JSON格式的分析结果（只返回JSON，不要有任何其他内容）：\n"
                            "{\n"
                            "  \"description\": \"详细的任务描述（50-100字），说明任务的目标、背景和重要性\",\n"
                            "  \"priority\": \"高或中或低，根据任务紧急程度和重要性判断\",\n"
                            "  \"estimated_hours\": \"预计工时（数字，如0.5,1,2,4,8），根据任务复杂度估算\",\n"
                            "  \"category\": \"从现有分类中选择最匹配的分类名称\",\n"
                            "  \"subtasks\": [\n"
                            "    \"子任务1：具体的执行步骤\",\n"
                            "    \"子任务2：具体的执行步骤\",\n"
                            "    \"子任务3：具体的执行步骤\"\n"
                            "  ],\n"
                            "  \"tags\": [\"标签1\", \"标签2\", \"标签3\"],\n"
                            "  \"notes\": \"补充说明、注意事项或建议\"\n"
                            "}\n\n"
                            "判断规则：\n"
                            "1. priority=高: 包含'紧急'、'重要'、'urgent'、'critical'、'asap'、'bug'、'修复'等关键词\n"
                            "2. priority=低: 包含'简单'、'快速'、'easy'、'minor'、'整理'、'学习'等关键词\n"
                            "3. estimated_hours: 根据任务复杂度估算，简单任务0.5-1小时，中等任务2-4小时，复杂任务4-8小时\n"
                            "4. subtasks: 将任务分解为3-5个具体的、可执行的子任务，每个子任务应该清晰明确\n"
                            "5. tags: 根据任务内容提取3-5个相关标签，如技术栈、项目名称、功能模块等\n"
                            "6. category: 根据任务主题从现有分类中选择最匹配的一个\n\n"
                            "要求：\n"
                            "- 描述要具体、有价值，不要只是重复标题\n"
                            "- 子任务要可执行、可衡量、有时间限制\n"
                            "- 标签要准确反映任务特征\n"
                            "- 确保返回的JSON格式正确，可以被解析\n\n"
                            "只返回JSON，不要其他文字。")
                            .arg(title)
                            .arg(getExistingCategories());

    QString apiKey = getAPIKey(currentModel);
    if (apiKey.isEmpty()) {
        if (aiStatusLabel) {
            aiStatusLabel->setText("⚠️ 请先配置API Key");
        }
        if (aiBtn) {
            aiBtn->setEnabled(true);
        }
        QMessageBox::warning(nullptr, "提示", "请先在设置中配置AI API Key");
        return;
    }

    AIKeyConfig keyConfig = AIConfig::instance().getKeyByModelName(currentModel);
    if (keyConfig.apiKey.isEmpty()) {
        if (aiStatusLabel) {
            aiStatusLabel->setText("⚠️ 请先配置API Key");
        }
        if (aiBtn) {
            aiBtn->setEnabled(true);
        }
        QMessageBox::warning(nullptr, "提示", "请先在设置中配置AI API Key");
        return;
    }

    QString endpoint = keyConfig.endpoint;
    if (endpoint.isEmpty()) {
        endpoint = getDefaultEndpoint(currentModel);
    }

    QString modelName = currentModel;

    QJsonObject json;
    json["model"] = modelName;
    json["stream"] = false;
    
    QJsonArray messages;
    QJsonObject msg;
    msg["role"] = "user";
    msg["content"] = prompt;
    messages.append(msg);
    json["messages"] = messages;
    
    if (currentModel == "minimax" || currentModel.startsWith("qwen") || currentModel.startsWith("spark") || currentModel.startsWith("deepseek")) {
        json["max_tokens"] = 1024;
    }

    if (currentModel.startsWith("qwen")) {
        QJsonObject input;
        input["messages"] = json.take("messages");
        json["input"] = input;
        json.remove("max_tokens");
        json.remove("stream");
        json["parameters"] = QJsonObject({
            {"temperature", 0.7},
            {"max_tokens", 1024},
            {"result_format", "message"}
        });
    }

    QJsonDocument doc(json);
    QByteArray postData = doc.toJson();

    QString logDir = QCoreApplication::applicationDirPath() + "/logs";
    QDir dir(logDir);
    if (!dir.exists()) {
        dir.mkpath(logDir);
    }
    
    QString logFileName = logDir + "/ai_request_" + QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss") + ".log";
    QFile logFile(logFileName);
    if (logFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&logFile);
        out.setCodec("UTF-8");
        out << "=== AI Request ===" << "\n";
        out << "Time: " << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << "\n";
        out << "Model: " << currentModel << "\n";
        out << "Endpoint: " << endpoint << "\n";
        out << "Request JSON:\n" << doc.toJson(QJsonDocument::Indented) << "\n\n";
        logFile.close();
    }
    
    QNetworkRequest request;
    request.setUrl(QUrl(endpoint));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", QString("Bearer %1").arg(keyConfig.apiKey).toUtf8());

    QNetworkReply *reply = networkManager->post(request, postData);
    QPointer<QNetworkReply> replyPtr(reply);

    QTimer *timeoutTimer = new QTimer(this);
    timeoutTimer->setSingleShot(true);
    timeoutTimer->setInterval(AIConfig::instance().getTimeout() * 1000);

    connect(timeoutTimer, &QTimer::timeout, this, [replyPtr]() {
        if (replyPtr && replyPtr->isRunning()) {
            replyPtr->abort();
        }
    });

    connect(reply, &QNetworkReply::finished, this, [this, replyPtr, title, titleEdit, descEdit, categoryCombo, priorityCombo, durationSpin, aiStatusLabel, aiBtn, logFileName, tagsEdit, timeoutTimer]() {
        if (timeoutTimer) {
            timeoutTimer->stop();
            timeoutTimer->deleteLater();
        }

        if (replyPtr) {
            handleAIResponse(replyPtr, title, titleEdit, descEdit, categoryCombo, priorityCombo, durationSpin, aiStatusLabel, aiBtn, logFileName, tagsEdit);
        }
    });
}

QString WorkLogWidget::getExistingCategories()
{
    QStringList categories;
    QList<Category> allCategories = db->getAllCategories();
    for (const Category &cat : allCategories) {
        categories.append(cat.name);
    }
    if (categories.isEmpty()) {
        categories << "工作" << "学习" << "生活" << "其他";
    }
    return categories.join("、");
}

QString WorkLogWidget::getAIServiceKey()
{
    AIKeyConfig key = AIConfig::instance().getDefaultKey();
    return key.apiKey;
}

QString WorkLogWidget::getAPIKey()
{
    return getAIServiceKey();
}

QString WorkLogWidget::getAPIKey(const QString &model)
{
    if (model.isEmpty()) {
        return getAPIKey();
    }
    
    AIKeyConfig key = AIConfig::instance().getKeyByModel(model);
    if (!key.apiKey.isEmpty()) {
        return key.apiKey;
    }
    
    return getAPIKey();
}

QString WorkLogWidget::getCurrentAIModel()
{
    AIKeyConfig key = AIConfig::instance().getDefaultKey();
    return key.model;
}

QString WorkLogWidget::getAPIEndpoint()
{
    AIKeyConfig key = AIConfig::instance().getDefaultKey();
    if (key.endpoint.isEmpty()) {
        return getDefaultEndpoint(key.model);
    }
    return key.endpoint;
}

void WorkLogWidget::setSettingsWidget(void *settings)
{
    Q_UNUSED(settings);
}

QString WorkLogWidget::getDefaultEndpoint(const QString &model)
{
    AIModelInfo modelInfo = AIConfig::instance().getModelInfo(model);
    if (!modelInfo.defaultEndpoint.isEmpty()) {
        return modelInfo.defaultEndpoint;
    }
    for (const AIModelInfo &m : AIConfig::instance().getAllModels()) {
        if (m.name == model) {
            return m.defaultEndpoint;
        }
    }
    return "";
}

QString WorkLogWidget::getModelNameForAPI(const QString &model)
{
    static QMap<QString, QString> models = {
        {"minimax", "abab6.5s-chat"},
        {"abab6.5s-chat", "abab6.5s-chat"},
        {"abab6.5g-chat", "abab6.5g-chat"},
        {"gpt35", "gpt-3.5-turbo"},
        {"gpt-3.5-turbo", "gpt-3.5-turbo"},
        {"gpt4", "gpt-4"},
        {"gpt-4", "gpt-4"},
        {"gpt4o", "gpt-4o"},
        {"gpt4turbo", "gpt-4-turbo"},
        {"claude", "claude-3-opus-20240229"},
        {"claude3haiku", "claude-3-haiku-20240307"},
        {"claude3sonnet", "claude-3-sonnet-20240229"},
        {"claude3opus", "claude-3-opus-20240229"},
        {"claude3.5sonnet", "claude-3-5-sonnet-20240620"},
        {"gemini", "gemini-pro"},
        {"gemini-pro", "gemini-pro"},
        {"gemini-1.5-pro", "gemini-1.5-pro"},
        {"qwen", "qwen-turbo"},
        {"qwen-turbo", "qwen-turbo"},
        {"qwen-plus", "qwen-plus"},
        {"qwen-max", "qwen-max"},
        {"qwen-long", "qwen-long"},
        {"spark", "generalv3.5"},
        {"spark-v3.5", "generalv3.5"},
        {"spark-v3", "generalv3"},
        {"deepseek", "deepseek-ai/DeepSeek-V3.2"},
        {"deepseek-v2", "deepseek-ai/DeepSeek-V2-Chat"},
        {"deepseek-v2.5", "deepseek-ai/DeepSeek-V2.5"},
        {"deepseek-v3", "deepseek-ai/DeepSeek-V3"},
        {"deepseek-v3.2", "deepseek-ai/DeepSeek-V3.2"},
        {"deepseek-coder", "deepseek-ai/DeepSeek-Coder-V2"},
        {"qwen-coder", "qwen-coder-7b-instruct"}
    };
    return models.value(model, model);
}

void WorkLogWidget::loadAIKeysToComboBox(QComboBox *comboBox)
{
    if (!comboBox) {
        return;
    }
    
    comboBox->clear();
    
    QList<AIKeyConfig> allKeys = AIConfig::instance().getAllKeys();
    
    if (allKeys.isEmpty()) {
        comboBox->addItem("⚙️ 请先配置AI Key", "");
        return;
    }
    
    AIKeyConfig defaultKey = AIConfig::instance().getDefaultKey();
    int defaultIndex = 0;
    
    for (int i = 0; i < allKeys.size(); ++i) {
        const AIKeyConfig &key = allKeys[i];
        QString displayName = key.name.isEmpty() ? key.model : key.name;

        if (!key.provider.isEmpty()) {
            AIProviderInfo providerInfo = AIConfig::instance().getProviderInfo(key.provider);
            if (!providerInfo.displayName.isEmpty()) {
                displayName = QString("%1 - %2").arg(providerInfo.displayName).arg(displayName);
            }
        }

        QString modelId = key.model;
        AIModelInfo modelInfo = AIConfig::instance().getModelInfo(modelId);
        QString modelName = modelInfo.name.isEmpty() ? modelId : modelInfo.name;

        comboBox->addItem(displayName, modelName);

        if (key.isDefault || key.model == defaultKey.model) {
            defaultIndex = i;
        }
    }
    
    if (comboBox->count() > 0) {
        comboBox->setCurrentIndex(defaultIndex);
    }
}

void WorkLogWidget::handleAIResponse(QPointer<QNetworkReply> reply, const QString &title, QLineEdit *titleEdit, 
                                      QTextEdit *descEdit, QComboBox *categoryCombo, QComboBox *priorityCombo,
                                      QDoubleSpinBox *durationSpin, QLabel *aiStatusLabel, QPushButton *aiBtn,
                                      const QString &logFileName, QLineEdit *tagsEdit)
{
    if (!reply) {
        return;
    }
    
    if (aiBtn) {
        aiBtn->setEnabled(true);
    }
    
    if (reply->error() != QNetworkReply::NoError) {
        QString errorMsg = reply->errorString();
        if (reply->error() == QNetworkReply::OperationCanceledError) {
            errorMsg = QString("请求超时（%1秒），请检查网络连接或增加超时时间").arg(AIConfig::instance().getTimeout());
        }
        if (aiStatusLabel) {
            aiStatusLabel->setText("❌ 调用失败");
        }
        qDebug() << "AI API Error:" << errorMsg;

        QFile logFile(logFileName);
        if (logFile.open(QIODevice::Append | QIODevice::Text)) {
            QTextStream out(&logFile);
            out.setCodec("UTF-8");
            out << "=== AI Response ===" << "\n";
            out << "Time: " << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << "\n";
            out << "HTTP Status: " << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() << "\n";
            out << "Error Code: " << reply->error() << "\n";
            out << "Error: " << errorMsg << "\n";
            out << "\n========================================\n\n";
            logFile.close();
        }

        if (reply->error() != QNetworkReply::OperationCanceledError) {
            analyzeWithLocalAI(title, titleEdit, descEdit, categoryCombo, priorityCombo, durationSpin, aiStatusLabel, tagsEdit);
        }
        return;
    }
    
    QByteArray responseData = reply->readAll();
    qDebug() << "AI Response:" << responseData;
    
    QFile logFile(logFileName);
    if (logFile.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&logFile);
        out.setCodec("UTF-8");
        out << "=== AI Response ===" << "\n";
        out << "Time: " << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << "\n";
        out << "HTTP Status: " << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() << "\n";
        out << "Response Data:\n" << responseData << "\n";
        out << "Response JSON:\n" << QJsonDocument::fromJson(responseData).toJson(QJsonDocument::Indented) << "\n";
        out << "\n========================================\n\n";
        logFile.close();
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(responseData);
    
    if (doc.isNull()) {
        if (aiStatusLabel) {
            aiStatusLabel->setText("⚠️ 解析失败");
        }
        analyzeWithLocalAI(title, titleEdit, descEdit, categoryCombo, priorityCombo, durationSpin, aiStatusLabel, tagsEdit);
        return;
    }
    
    QJsonObject rootObj = doc.object();
    
    if (rootObj.contains("base_resp")) {
        int statusCode = rootObj["base_resp"].toObject()["status_code"].toInt();
        if (statusCode != 0) {
            QString errorMsg = rootObj["base_resp"].toObject()["status_msg"].toString();
            if (aiStatusLabel) {
                aiStatusLabel->setText("❌ API错误: " + errorMsg);
            }
            qDebug() << "AI API Error:" << errorMsg;
            analyzeWithLocalAI(title, titleEdit, descEdit, categoryCombo, priorityCombo, durationSpin, aiStatusLabel, tagsEdit);
            return;
        }
    }
    
    QJsonObject choicesObj;
    
    if (rootObj.contains("output") && rootObj["output"].toObject().contains("choices")) {
        choicesObj = rootObj["output"].toObject()["choices"].toArray()[0].toObject();
    } else if (rootObj.contains("choices")) {
        choicesObj = rootObj["choices"].toArray()[0].toObject();
    } else {
        if (aiStatusLabel) {
            aiStatusLabel->setText("⚠️ 无返回内容");
        }
        analyzeWithLocalAI(title, titleEdit, descEdit, categoryCombo, priorityCombo, durationSpin, aiStatusLabel, tagsEdit);
        return;
    }
    
    QString content = choicesObj["message"].toObject()["content"].toString();
    
    if (content.isEmpty()) {
        if (aiStatusLabel) {
            aiStatusLabel->setText("⚠️ 内容为空");
        }
        analyzeWithLocalAI(title, titleEdit, descEdit, categoryCombo, priorityCombo, durationSpin, aiStatusLabel, tagsEdit);
        return;
    }
    
    parseAIResponse(content, titleEdit, descEdit, categoryCombo, priorityCombo, durationSpin, tagsEdit);
    
    if (aiStatusLabel) {
        aiStatusLabel->setText("✅ 已填充");
        QTimer::singleShot(2000, aiStatusLabel, [aiStatusLabel]() {
            if (aiStatusLabel) {
                aiStatusLabel->setText("");
            }
        });
    }
    
    reply->deleteLater();
}

void WorkLogWidget::analyzeWithLocalAI(const QString &title, QLineEdit *titleEdit, QTextEdit *descEdit,
                                         QComboBox *categoryCombo, QComboBox *priorityCombo, 
                                         QDoubleSpinBox *durationSpin, QLabel *aiStatusLabel, QLineEdit *tagsEdit)
{
    if (!descEdit || !categoryCombo || !priorityCombo || !durationSpin) {
        return;
    }
    
    QString lowerTitle = title.toLower();
    QString description;
    QString priorityStr = "中";
    double hours = 1.0;
    QString categoryName = "其他";
    QStringList subtasks;
    QStringList tags;
    QString notes;
    
    if (lowerTitle.contains("会议") || lowerTitle.contains("meeting")) {
        categoryName = "会议";
        description = "参加相关会议，讨论项目进展和问题";
        hours = 1.0;
        subtasks << "准备会议材料" << "参加会议讨论" << "整理会议纪要";
        tags << "会议" << "沟通" << "协作";
    } else if (lowerTitle.contains("代码") || lowerTitle.contains("开发") || lowerTitle.contains("bug") || 
               lowerTitle.contains("debug") || lowerTitle.contains("code") || lowerTitle.contains("编程")) {
        categoryName = "开发";
        description = "进行代码开发工作，实现功能需求或修复问题";
        hours = 2.0;
        if (lowerTitle.contains("紧急") || lowerTitle.contains("重要") || lowerTitle.contains("critical")) {
            priorityStr = "高";
        }
        if (lowerTitle.contains("bug") || lowerTitle.contains("修复")) {
            subtasks << "分析问题原因" << "定位代码位置" << "修复bug" << "测试验证";
            tags << "bug" << "修复" << "调试";
            notes = "修复后需要进行回归测试";
        } else {
            subtasks << "分析需求" << "编写代码" << "单元测试" << "代码审查";
            tags << "开发" << "编码" << "测试";
        }
    } else if (lowerTitle.contains("学习") || lowerTitle.contains("培训") || lowerTitle.contains("study") || 
               lowerTitle.contains("课程") || lowerTitle.contains("读书")) {
        categoryName = "学习";
        description = "进行学习培训，提升专业技能和知识储备";
        hours = 1.5;
        subtasks << "阅读资料" << "做笔记" << "实践练习" << "总结心得";
        tags << "学习" << "培训" << "提升";
        notes = "建议定期复习巩固";
    } else if (lowerTitle.contains("文档") || lowerTitle.contains("报告") || lowerTitle.contains("write") || 
               lowerTitle.contains("撰写") || lowerTitle.contains("整理")) {
        categoryName = "文档";
        description = "撰写文档报告，记录项目信息和技术细节";
        hours = 2.0;
        subtasks << "收集资料" << "编写大纲" << "撰写内容" << "校对修改";
        tags << "文档" << "写作" << "整理";
    } else if (lowerTitle.contains("测试") || lowerTitle.contains("test") || lowerTitle.contains("验证")) {
        categoryName = "测试";
        description = "进行测试验证工作，确保功能正常和质量达标";
        hours = 1.5;
        subtasks << "编写测试用例" << "执行测试" << "记录结果" << "提交bug";
        tags << "测试" << "验证" << "质量";
    } else if (lowerTitle.contains("维护") || lowerTitle.contains("部署") || lowerTitle.contains("运维") || 
               lowerTitle.contains("deploy") || lowerTitle.contains("maintain")) {
        categoryName = "运维";
        description = "系统维护部署工作，保障系统稳定运行";
        hours = 1.0;
        subtasks << "检查系统状态" << "执行维护操作" << "监控运行情况" << "记录日志";
        tags << "运维" << "维护" << "部署";
    } else if (lowerTitle.contains("设计") || lowerTitle.contains("design") || lowerTitle.contains("规划")) {
        categoryName = "设计";
        description = "进行设计规划工作，制定技术方案和架构";
        hours = 2.0;
        subtasks << "需求分析" << "方案设计" << "原型制作" << "评审确认";
        tags << "设计" << "规划" << "架构";
    } else if (lowerTitle.contains("review") || lowerTitle.contains("评审") || lowerTitle.contains("检查")) {
        categoryName = "评审";
        description = "进行代码评审或检查，确保代码质量";
        hours = 1.0;
        subtasks << "阅读代码" << "检查规范" << "提出建议" << "确认修改";
        tags << "评审" << "检查" << "质量";
    } else if (lowerTitle.contains("紧急") || lowerTitle.contains("urgent") || lowerTitle.contains("重要")) {
        priorityStr = "高";
        hours = 1.0;
        description = "紧急重要任务，需要优先处理";
        subtasks << "评估影响" << "制定计划" << "执行处理" << "跟踪结果";
        tags << "紧急" << "重要" << "优先";
        notes = "需要密切关注进展";
    } else if (lowerTitle.contains("简单") || lowerTitle.contains("快速") || lowerTitle.contains("easy")) {
        hours = 0.5;
        description = "简单快速任务，可以快速完成";
        subtasks << "执行任务" << "验证结果";
        tags << "简单" << "快速";
    } else if (lowerTitle.contains("复杂") || lowerTitle.contains("困难") || lowerTitle.contains("hard")) {
        hours = 4.0;
        priorityStr = "高";
        description = "复杂困难任务，需要仔细规划和执行";
        subtasks << "分析需求" << "制定方案" << "分步实施" << "测试验证" << "总结优化";
        tags << "复杂" << "困难" << "重要";
        notes = "建议分阶段完成";
    } else {
        subtasks << "明确目标" << "执行任务" << "检查结果";
        tags << "任务" << "执行";
    }
    
    if (lowerTitle.contains("早上") || lowerTitle.contains("上午")) {
        notes += "（建议上午完成）";
    } else if (lowerTitle.contains("下午") || lowerTitle.contains("下班")) {
        notes += "（建议下午完成）";
    }
    
    if (lowerTitle.contains("2小时") || lowerTitle.contains("两小时")) {
        hours = 2.0;
    } else if (lowerTitle.contains("3小时") || lowerTitle.contains("三小时")) {
        hours = 3.0;
    } else if (lowerTitle.contains("半天")) {
        hours = 4.0;
    } else if (lowerTitle.contains("一天")) {
        hours = 8.0;
    }
    
    QString fullDescription = description;
    if (!subtasks.isEmpty()) {
        fullDescription += "\n\n子任务：\n";
        for (const QString &subtask : subtasks) {
            fullDescription += "• " + subtask + "\n";
        }
    }
    
    if (!notes.isEmpty()) {
        fullDescription += "\n备注：\n" + notes;
    }
    
    descEdit->setPlainText(fullDescription.trimmed());
    
    if (priorityStr == "高") {
        priorityCombo->setCurrentIndex(priorityCombo->findData(static_cast<int>(TaskPriority_High)));
    } else if (priorityStr == "中") {
        priorityCombo->setCurrentIndex(priorityCombo->findData(static_cast<int>(TaskPriority_Medium)));
    } else {
        priorityCombo->setCurrentIndex(priorityCombo->findData(static_cast<int>(TaskPriority_Low)));
    }
    
    durationSpin->setValue(hours);
    
    int categoryIndex = categoryCombo->findText("📁 " + categoryName);
    if (categoryIndex < 0) {
        categoryIndex = categoryCombo->findText(categoryName);
    }
    if (categoryIndex >= 0) {
        categoryCombo->setCurrentIndex(categoryIndex);
    }
    
    if (tagsEdit && !tags.isEmpty()) {
        tagsEdit->setText(tags.join(", "));
    }
    
    if (aiStatusLabel) {
        aiStatusLabel->setText("✅ 已填充");
        QTimer::singleShot(2000, aiStatusLabel, [aiStatusLabel]() {
            if (aiStatusLabel) {
                aiStatusLabel->setText("");
            }
        });
    }
}

void WorkLogWidget::parseAIResponse(const QString &response, QLineEdit *titleEdit, QTextEdit *descEdit,
                                    QComboBox *categoryCombo, QComboBox *priorityCombo, QDoubleSpinBox *durationSpin,
                                    QLineEdit *tagsEdit)
{
    if (!descEdit || !categoryCombo || !priorityCombo || !durationSpin) {
        return;
    }
    
    QString jsonStr = response;
    jsonStr = jsonStr.replace("```json", "").replace("```", "").trimmed();
    
    QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toUtf8());
    if (doc.isNull() || !doc.isObject()) {
        analyzeWithLocalAI(titleEdit ? titleEdit->text() : QString(), titleEdit, descEdit, categoryCombo, priorityCombo, durationSpin, nullptr, tagsEdit);
        return;
    }
    
    QJsonObject obj = doc.object();
    
    QString description = "";
    if (obj.contains("description")) {
        description = obj["description"].toString();
    }
    
    if (obj.contains("subtasks") && obj["subtasks"].isArray()) {
        QJsonArray subtasks = obj["subtasks"].toArray();
        if (!subtasks.isEmpty()) {
            description += "\n\n子任务：\n";
            for (int i = 0; i < subtasks.size(); ++i) {
                description += QString("• %1\n").arg(subtasks[i].toString());
            }
        }
    }
    
    if (obj.contains("notes")) {
        QString notes = obj["notes"].toString();
        if (!notes.isEmpty()) {
            description += "\n备注：\n" + notes;
        }
    }
    
    if (!description.isEmpty()) {
        descEdit->setPlainText(description.trimmed());
    }
    
    if (obj.contains("priority")) {
        QString priority = obj["priority"].toString();
        if (priority.contains("高") || priority.contains("high")) {
            priorityCombo->setCurrentIndex(priorityCombo->findData(static_cast<int>(TaskPriority_High)));
        } else if (priority.contains("低") || priority.contains("low")) {
            priorityCombo->setCurrentIndex(priorityCombo->findData(static_cast<int>(TaskPriority_Low)));
        } else {
            priorityCombo->setCurrentIndex(priorityCombo->findData(static_cast<int>(TaskPriority_Medium)));
        }
    }
    
    if (obj.contains("estimated_hours")) {
        double hours = obj["estimated_hours"].toDouble();
        if (hours > 0) {
            durationSpin->setValue(hours);
        }
    }
    
    if (obj.contains("category")) {
        QString category = obj["category"].toString();
        int index = categoryCombo->findText("📁 " + category);
        if (index < 0) {
            index = categoryCombo->findText(category);
        }
        if (index >= 0) {
            categoryCombo->setCurrentIndex(index);
        }
    }
    
    if (obj.contains("tags") && obj["tags"].isArray() && tagsEdit) {
        QJsonArray tags = obj["tags"].toArray();
        QStringList tagList;
        for (const QJsonValue &tag : tags) {
            tagList << tag.toString();
        }
        if (!tagList.isEmpty()) {
            tagsEdit->setText(tagList.join(", "));
        }
    }
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

bool WorkLogWidget::exportToPDF(const QString &content, const QString &fileName)
{
    if (!checkPermission("export_report")) {
        QMessageBox::warning(this, "权限不足", "您没有导出报告的权限");
        return false;
    }
    
    QTextDocument document;
    document.setMarkdown(content);
    
    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(fileName);
    printer.setPageSize(QPageSize(QPageSize::A4));
    printer.setPageMargins(QMarginsF(20, 20, 20, 20), QPageLayout::Millimeter);
    
    document.print(&printer);
    
    logOperation("export_report", QString("格式: PDF, 文件: %1").arg(fileName));
    QMessageBox::information(this, "成功", "报告已成功导出为PDF");
    return true;
}

bool WorkLogWidget::exportToWord(const QString &content, const QString &fileName)
{
    if (!checkPermission("export_report")) {
        QMessageBox::warning(this, "权限不足", "您没有导出报告的权限");
        return false;
    }
    
    QAxObject *word = nullptr;
    QAxObject *documents = nullptr;
    QAxObject *document = nullptr;
    QAxObject *selection = nullptr;
    
    try {
        word = new QAxObject("Word.Application", this);
        if (!word->isNull()) {
            word->setProperty("Visible", false);
            
            documents = word->querySubObject("Documents");
            if (documents) {
                document = documents->querySubObject("Add()");
                if (document) {
                    selection = word->querySubObject("Selection");
                    if (selection) {
                        QTextDocument textDoc;
                        textDoc.setMarkdown(content);
                        QTextCursor cursor(&textDoc);
                        
                        QAxObject *font = selection->querySubObject("Font");
                        if (font) {
                            font->setProperty("Name", "Microsoft YaHei");
                            font->setProperty("Size", 12);
                            font->setProperty("NameFarEast", "Microsoft YaHei");
                        }
                        
                        for (QTextBlock block = textDoc.begin(); block != textDoc.end(); block = block.next()) {
                            QTextBlockFormat blockFormat = block.blockFormat();
                            QTextCharFormat charFormat = block.charFormat();
                            
                            if (block.text().startsWith("## ")) {
                                selection->querySubObject("ParagraphFormat")->setProperty("Alignment", 0);
                                QAxObject *headingFont = selection->querySubObject("Font");
                                if (headingFont) {
                                    headingFont->setProperty("Size", 16);
                                    headingFont->setProperty("Bold", true);
                                }
                                selection->dynamicCall("TypeText(const QString&)", block.text().mid(3));
                                selection->dynamicCall("TypeParagraph()");
                                QAxObject *normalFont = selection->querySubObject("Font");
                                if (normalFont) {
                                    normalFont->setProperty("Size", 12);
                                    normalFont->setProperty("Bold", false);
                                }
                            } else if (block.text().startsWith("### ")) {
                                selection->querySubObject("ParagraphFormat")->setProperty("Alignment", 0);
                                QAxObject *subHeadingFont = selection->querySubObject("Font");
                                if (subHeadingFont) {
                                    subHeadingFont->setProperty("Size", 14);
                                    subHeadingFont->setProperty("Bold", true);
                                }
                                selection->dynamicCall("TypeText(const QString&)", block.text().mid(4));
                                selection->dynamicCall("TypeParagraph()");
                                QAxObject *normalFont = selection->querySubObject("Font");
                                if (normalFont) {
                                    normalFont->setProperty("Size", 12);
                                    normalFont->setProperty("Bold", false);
                                }
                            } else if (block.text().startsWith("# ")) {
                                selection->querySubObject("ParagraphFormat")->setProperty("Alignment", 1);
                                QAxObject *titleFont = selection->querySubObject("Font");
                                if (titleFont) {
                                    titleFont->setProperty("Size", 18);
                                    titleFont->setProperty("Bold", true);
                                }
                                selection->dynamicCall("TypeText(const QString&)", block.text().mid(2));
                                selection->dynamicCall("TypeParagraph()");
                                QAxObject *normalFont = selection->querySubObject("Font");
                                if (normalFont) {
                                    normalFont->setProperty("Size", 12);
                                    normalFont->setProperty("Bold", false);
                                }
                            } else if (block.text().startsWith("- ") || block.text().startsWith("* ")) {
                                selection->querySubObject("ParagraphFormat")->setProperty("Alignment", 0);
                                selection->dynamicCall("TypeText(const QString&)", "• " + block.text().mid(2));
                                selection->dynamicCall("TypeParagraph()");
                            } else if (!block.text().isEmpty()) {
                                selection->querySubObject("ParagraphFormat")->setProperty("Alignment", 0);
                                selection->dynamicCall("TypeText(const QString&)", block.text());
                                selection->dynamicCall("TypeParagraph()");
                            }
                        }
                        
                        document->dynamicCall("SaveAs(const QString&, int)", fileName, 16);
                        
                        document->dynamicCall("Close(bool)", false);
                        word->dynamicCall("Quit()");
                        
                        logOperation("export_report", QString("格式: Word, 文件: %1").arg(fileName));
                        QMessageBox::information(this, "成功", "报告已成功导出为Word");
                        return true;
                    }
                }
            }
        }
        
        if (document) {
            document->dynamicCall("Close(bool)", false);
        }
        if (word) {
            word->dynamicCall("Quit()");
        }
        
        QMessageBox::warning(this, "错误", "无法创建Word文档，请确保已安装Microsoft Word");
        return false;
        
    } catch (...) {
        if (document) {
            document->dynamicCall("Close(bool)", false);
        }
        if (word) {
            word->dynamicCall("Quit()");
        }
        
        QMessageBox::warning(this, "错误", "导出Word文档时发生错误");
        return false;
    }
}

QString WorkLogWidget::markdownToHTML(const QString &markdown)
{
    QString html = markdown;
    
    html.replace(QRegularExpression("^# (.+)$"), "<h1>\\1</h1>");
    html.replace(QRegularExpression("^## (.+)$"), "<h2>\\1</h2>");
    html.replace(QRegularExpression("^### (.+)$"), "<h3>\\1</h3>");
    html.replace(QRegularExpression("^#### (.+)$"), "<h4>\\1</h4>");
    
    html.replace(QRegularExpression("\\*\\*(.+?)\\*\\*"), "<strong>\\1</strong>");
    html.replace(QRegularExpression("\\*(.+?)\\*"), "<em>\\1</em>");
    
    html.replace(QRegularExpression("^- (.+)$"), "<li>\\1</li>");
    html.replace(QRegularExpression("^\\d+\\. (.+)$"), "<li>\\1</li>");
    
    html.replace(QRegularExpression("```"), "<pre>");
    
    html.replace(QRegularExpression("\n\n"), "</p><p>");
    html.prepend("<p>");
    html.append("</p>");
    
    html.replace(QRegularExpression("<p>(<h[1-6]>)"), "\\1");
    html.replace(QRegularExpression("(</h[1-6]>)</p>"), "\\1");
    html.replace(QRegularExpression("<p>(<li>)"), "<ul>\\1");
    html.replace(QRegularExpression("(</li>)</p>"), "\\1</ul>");
    
    html.replace(QRegularExpression("<p></p>"), "");
    html.replace(QRegularExpression("<ul></ul>"), "");
    
    html.replace("\n", "<br>");
    
    return html;
}

QString WorkLogWidget::getCachedReport(const QString &cacheKey)
{
    if (reportCache.contains(cacheKey) && isCacheValid(cacheKey)) {
        return reportCache[cacheKey];
    }
    return QString();
}

void WorkLogWidget::cacheReport(const QString &cacheKey, const QString &report)
{
    reportCache[cacheKey] = report;
    lastCacheUpdate = QDateTime::currentDateTime();
}

bool WorkLogWidget::isCacheValid(const QString &cacheKey, int maxAgeMinutes)
{
    if (!reportCache.contains(cacheKey)) {
        return false;
    }
    
    if (lastCacheUpdate.isNull()) {
        return false;
    }
    
    QDateTime now = QDateTime::currentDateTime();
    qint64 secondsDiff = lastCacheUpdate.secsTo(now);
    qint64 minutesDiff = secondsDiff / 60;
    
    return minutesDiff < maxAgeMinutes;
}

void WorkLogWidget::clearReportCache()
{
    reportCache.clear();
    lastCacheUpdate = QDateTime();
}

void WorkLogWidget::logOperation(const QString &operation, const QString &details)
{
    QString logDir = QCoreApplication::applicationDirPath() + "/logs";
    QDir dir(logDir);
    if (!dir.exists()) {
        dir.mkpath(logDir);
    }
    
    QString logFileName = logDir + "/operations_" + QDateTime::currentDateTime().toString("yyyy_MM_dd") + ".log";
    QFile logFile(logFileName);
    
    if (logFile.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&logFile);
        out.setCodec("UTF-8");
        out << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << " | "
            << operation << " | " << details << "\n";
        logFile.close();
    }
}

bool WorkLogWidget::checkPermission(const QString &permission)
{
    QSettings settings("PonyWork", "WorkLog");
    QStringList permissions = settings.value("permissions", QStringList()).toStringList();

    if (permissions.isEmpty()) {
        return true;
    }

    return permissions.contains(permission);
}

bool WorkLogWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == pieChartView) {
        if (event->type() == QEvent::MouseButtonDblClick) {
            onPieChartDoubleClick();
            return true;
        }
    }
    return QWidget::eventFilter(obj, event);
}

void WorkLogWidget::onPieChartDoubleClick()
{
    // 创建对话框
    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle("饼图详情 - 分类时间分布");
    dialog->setMinimumSize(800, 600);
    dialog->setStyleSheet("QDialog { background-color: #ffffff; }");

    QVBoxLayout *layout = new QVBoxLayout(dialog);

    // 创建新的图表视图（放大版）
    QChart *largeChart = new QChart();
    largeChart->setTitle("分类时间分布（双击查看详情）");
    largeChart->setAnimationOptions(QChart::SeriesAnimations);
    largeChart->legend()->setVisible(true);
    largeChart->legend()->setAlignment(Qt::AlignBottom);

    // 复制当前饼图的数据到新图表
    if (!pieChart->series().isEmpty()) {
        QPieSeries *originalSeries = qobject_cast<QPieSeries*>(pieChart->series().first());
        if (originalSeries) {
            QPieSeries *newSeries = new QPieSeries();
            for (QPieSlice *slice : originalSeries->slices()) {
                newSeries->append(slice->label(), slice->value());
            }
            largeChart->addSeries(newSeries);
        }
    }

    QChartView *chartView = new QChartView(largeChart, dialog);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setMinimumSize(700, 500);
    layout->addWidget(chartView);

    // 添加关闭按钮
    QPushButton *closeBtn = new QPushButton("关闭", dialog);
    closeBtn->setStyleSheet("padding: 8px 20px; background-color: #3498db; color: white; border: none; border-radius: 4px;");
    connect(closeBtn, &QPushButton::clicked, dialog, &QDialog::accept);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    btnLayout->addWidget(closeBtn);
    layout->addLayout(btnLayout);

    // 显示对话框
    dialog->exec();
    dialog->deleteLater();
}

// ========== CalendarWidget 实现 ==========

CalendarWidget::CalendarWidget(QWidget *parent)
    : QWidget(parent)
{
    m_currentMonth = QDate::currentDate();
    m_selectedDate = QDate::currentDate();
    setMinimumHeight(400);
    setMinimumWidth(600);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    // 移除背景色和边框，由paintEvent统一绘制
    setAttribute(Qt::WA_StyledBackground, false);
    updateCells();
}

void CalendarWidget::setMonth(const QDate &date)
{
    m_currentMonth = QDate(date.year(), date.month(), 1);
    updateCells();
    update();
}

void CalendarWidget::setSelectedDate(const QDate &date)
{
    m_selectedDate = date;
    update();
}

void CalendarWidget::setTaskInfos(const QMap<QDate, QVector<QPair<QString, TaskStatus>>> &tasks)
{
    m_taskInfos.clear();
    // 将任务信息转换为日期矩阵格式
    updateCells();

    // 为每个日期设置任务信息
    for (auto it = tasks.constBegin(); it != tasks.constEnd(); ++ it) {
        QDate date = it.key();
        const QVector<QPair<QString, TaskStatus>> &taskList = it.value();

        // 找到对应的单元格并设置信息
        for (int row = 0; row < m_cells.size(); ++row) {
            for (int col = 0; col < m_cells[row].size(); ++col) {
                if (m_cells[row][col].date == date && !taskList.isEmpty()) {
                    m_cells[row][col].taskCount = taskList.size();
                    m_cells[row][col].firstTaskStatus = taskList.first().second;
                    m_cells[row][col].firstTaskTitle = taskList.first().first;
                }
            }
        }
    }
    update();
}

void CalendarWidget::updateCells()
{
    m_cells.clear();
    m_cells.resize(6); // 最多6行

    // 获取当月第一天是星期几（周一 = 0， Sunday = 6）
    int firstDayOfWeek = m_currentMonth.dayOfWeek() - 1; // 转换为0-6

    // 日历应该从上月最后firstDayOfWeek天开始显示
    // 获取日历第一个日期（可能来自上月）
    QDate firstDate = m_currentMonth.addDays(-firstDayOfWeek);

    // 获取下月的开始几天
    int daysInMonth = m_currentMonth.daysInMonth();
    int totalCells = 42; // 6行 x 7列
    int nextMonthDay = 1;

    for (int i = 0; i < 6; ++i) {
        m_cells[i].resize(7);
        for (int j = 0; j < 7; ++j) {
            int dayOffset = i * 7 + j;
            QDate cellDate = firstDate.addDays(dayOffset);

            // 判断是否为本月
            bool isCurrentMonth = (cellDate.month() == m_currentMonth.month());

            if (isCurrentMonth) {
                m_cells[i][j] = {cellDate, true, 0, TaskStatus_Todo, ""};
            } else if (cellDate < m_currentMonth) {
                // 上月
                m_cells[i][j] = {cellDate, false, 0, TaskStatus_Todo, ""};
            } else {
                // 下月
                m_cells[i][j] = {cellDate, false, 0, TaskStatus_Todo, ""};
            }
        }
    }
}

int CalendarWidget::cellWidth() const
{
    return width() / 7;
}

int CalendarWidget::cellHeight() const
{
    // 减去星期标题行的高度
    return (height() - 30) / 6;
}

void CalendarWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    int w = cellWidth();
    int h = cellHeight();

    // 绘制外边框和背景 - 强制白色背景
    QRect outerRect = rect();
    outerRect.adjust(0, 0, -1, -1);
    painter.setPen(QColor("#AED6F1"));  // 浅蓝色边框
    painter.setBrush(QBrush(QColor("#FFFFFF"), Qt::SolidPattern));
    painter.drawRoundedRect(outerRect, 8, 8);

    // 强制填充整个区域为白色
    painter.fillRect(rect(), QColor("#FFFFFF"));

    // 绘制星期标题栏 - 清新浅蓝色
    QRect headerRect(0, 0, width(), 30);
    QLinearGradient headerGradient(headerRect.topLeft(), headerRect.bottomRight());
    headerGradient.setColorAt(0, QColor("#AED6F1"));  // 浅蓝
    headerGradient.setColorAt(1, QColor("#D4E6F1"));  // 更浅的蓝
    painter.setBrush(headerGradient);
    painter.setPen(Qt::NoPen);
    painter.drawRect(headerRect);

    // 绘制星期标题
    QStringList weekDays = {"周一", "周二", "周三", "周四", "周五", "周六", "周日"};
    painter.setFont(QFont("Microsoft YaHei", 10, QFont::Bold));
    for (int i = 0; i < 7; ++i) {
        QRect weekRect(i * w + 1, 1, w - 2, 28);
        painter.setPen(QColor("#2C3E50"));  // 深蓝灰文字
        painter.drawText(weekRect, Qt::AlignCenter, weekDays[i]);
    }

    // 绘制日期格子
    QDate today = QDate::currentDate();  // 循环外获取今天日期
    painter.setFont(QFont("Microsoft YaHei", 10));
    painter.setBrush(QBrush());  // 重置画刷，避免继承星期标题的渐变
    for (int i = 0; i < 6; ++i) {
        for (int j = 0; j < 7; ++j) {
            QRect cellRect(j * w + 1, 30 + i * h + 1, w - 2, h - 2);
            const DayCell &cell = m_cells[i][j];

            // 背景色 - 使用明确的条件判断
            QColor bgColor;
            QColor textColor;
            QColor borderColor = QColor("#E8E8E8");

            // 先设置默认白色背景
            bgColor = QColor("#FFFFFF");
            textColor = QColor("#2C3E50");

            // 选中日期 - 蓝色渐变（优先）
            bool isSelected = (cell.isCurrentMonth && cell.date == m_selectedDate);
            if (isSelected) {
                QLinearGradient selectedGradient(cellRect.topLeft(), cellRect.bottomRight());
                selectedGradient.setColorAt(0, QColor("#5DADE2"));
                selectedGradient.setColorAt(1, QColor("#3498DB"));
                painter.setBrush(selectedGradient);
                painter.setPen(Qt::NoPen);
                painter.drawRoundedRect(cellRect, 6, 6);
                painter.setBrush(QBrush());
                textColor = QColor("#FFFFFF");
            }
            // 今日之前 - 薄荷绿（仅当未选中时）
            else if (cell.isCurrentMonth && cell.date < today) {
                bgColor = QColor("#E8F8F5");
                textColor = QColor("#1D8348");
                painter.fillRect(cellRect, bgColor);
            }
            // 今日之后 - 浅蓝色（仅当未选中时）
            else if (cell.isCurrentMonth && cell.date > today) {
                bgColor = QColor("#EBF5FB");
                textColor = QColor("#2874A6");
                painter.fillRect(cellRect, bgColor);
            }
            // 今天 - 浅蓝色（仅当未选中时）
            else if (cell.isCurrentMonth && cell.date == today) {
                bgColor = QColor("#D4E6F1");
                textColor = QColor("#1A5276");
                painter.fillRect(cellRect, bgColor);
            }
            // 非本月日期 - 浅灰色
            else if (!cell.isCurrentMonth) {
                bgColor = QColor("#F4F6F6");
                textColor = QColor("#BDC3C7");
                painter.fillRect(cellRect, bgColor);
            }

            // 绘制网格线（选中日期跳过，避免覆盖圆角效果）
            if (!isSelected) {
                painter.setPen(borderColor);
                painter.drawRect(cellRect);
            }

            // 绘制日期数字
            painter.setFont(QFont("Microsoft YaHei", 10, QFont::Bold));
            painter.setPen(textColor);

            QRect dateRect = cellRect.adjusted(5, 3, -5, -3);
            painter.drawText(dateRect, Qt::AlignTop | Qt::AlignLeft, QString::number(cell.date.day()));

            // 绘制今日标记（橙色圆圈）- 未选中时显示
            if (cell.isCurrentMonth && cell.date == today && cell.date != m_selectedDate) {
                painter.setRenderHint(QPainter::Antialiasing);
                painter.setPen(QColor("#F39C12"));  // 橙色
                painter.setBrush(Qt::NoBrush);
                painter.drawEllipse(cellRect.center().x() - 9, cellRect.top() + 3, 18, 18);
            }

            // 绘制任务信息
            if (cell.isCurrentMonth && cell.taskCount > 0) {
                // 任务状态颜色点（圆形）
                QColor statusColor;
                switch (cell.firstTaskStatus) {
                    case TaskStatus_Completed: statusColor = QColor("#27AE60"); break;  // 绿色
                    case TaskStatus_InProgress: statusColor = QColor("#3498DB"); break;  // 蓝色
                    case TaskStatus_Paused: statusColor = QColor("#F39C12"); break;  // 橙色
                    default: statusColor = QColor("#F39C12"); break;
                }

                painter.setRenderHint(QPainter::Antialiasing);
                //painter.setBrush(statusColor);
                painter.setPen(Qt::NoPen);
                painter.drawEllipse(cellRect.left() + 6, cellRect.bottom() - 14, 6, 6);

                // 任务标题（截断）
                QString title = cell.firstTaskTitle;
                int maxChars = (cell.taskCount > 1) ? 6 : 8;
                if (title.length() > maxChars) {
                    title = title.left(maxChars) + "..";
                }

                painter.setRenderHint(QPainter::Antialiasing, false);
                painter.setFont(QFont("Microsoft YaHei", 8));
                // 任务文字颜色
                if (cell.isCurrentMonth && cell.date == m_selectedDate) {
                    painter.setPen(QColor("#FFFFFF"));  // 选中时白色
                } else if (cell.isCurrentMonth && cell.date == today) {
                    painter.setPen(QColor("#1A5276"));  // 今天深蓝色
                } else if (!cell.isCurrentMonth) {
                    painter.setPen(QColor("#BDC3C7"));  // 跨月灰色
                } else {
                    painter.setPen(QColor("#2C3E50"));  // 普通日期深蓝灰
                }

                QRect taskRect(cellRect.left() + 14, cellRect.bottom() - 17, w - 20, 14);
                painter.drawText(taskRect, Qt::AlignLeft | Qt::AlignVCenter, title);

                // 如果有更多任务，显示数量
                if (cell.taskCount > 1) {
                    QString moreText = QString("+%1").arg(cell.taskCount - 1);
                    painter.setFont(QFont("Microsoft YaHei", 7));
                    QRect moreRect(cellRect.right() - 28, cellRect.bottom() - 15, 24, 12);
                    painter.drawText(moreRect, Qt::AlignRight | Qt::AlignVCenter, moreText);
                }
            }
        }
    }
}

void CalendarWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        int w = cellWidth();
        int h = cellHeight();

        int col = event->x() / w;
        int row = (event->y() - 30) / h;

        if (row >= 0 && row < 6 && col >= 0 && col < 7) {
            m_selectedDate = m_cells[row][col].date;
            update();
            emit dateSelected(m_selectedDate);
        }
    }
    QWidget::mousePressEvent(event);
}

void CalendarWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        int w = cellWidth();
        int h = cellHeight();

        int col = event->x() / w;
        int row = (event->y() - 30) / h;

        if (row >= 0 && row < 6 && col >= 0 && col < 7) {
            m_selectedDate = m_cells[row][col].date;
            update();
            emit dateDoubleClicked(m_selectedDate);
        }
    }
    QWidget::mouseDoubleClickEvent(event);
}
