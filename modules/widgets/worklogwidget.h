#ifndef WORKLOGWIDGET_H
#define WORKLOGWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QStandardItemModel>
#include <QItemDelegate>
#include <QTextEdit>
#include <QTableWidget>
#include <QHeaderView>
#include <QListWidget>
#include <QDateTimeEdit>
#include <QSpinBox>
#include <QTabWidget>
#include <QSplitter>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QMenu>
#include <QAction>
#include <QProgressBar>
#include <QDialog>
#include <QFormLayout>
#include <QCheckBox>
#include <QGroupBox>
#include <QFileDialog>
#include <QMessageBox>
#include <QTimer>
#include <QtCharts>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>
#include <QProxyStyle>
#include <QPainter>
#include <QStyleOption>
#include <QStackedWidget>
#include <QButtonGroup>
#include "modules/core/database.h"

// 前向声明
class CalendarWidget;

class ComboBoxArrowStyle : public QProxyStyle
{
public:
    ComboBoxArrowStyle(QStyle *style = nullptr) : QProxyStyle(style) {}

    void drawComplexControl(ComplexControl control, const QStyleOptionComplex *option,
                           QPainter *painter, const QWidget *widget) const override
    {
        if (control == CC_ComboBox) {
            const QStyleOptionComboBox *comboBox = qstyleoption_cast<const QStyleOptionComboBox *>(option);
            if (comboBox) {
                QProxyStyle::drawComplexControl(control, option, painter, widget);
                
                QRect arrowRect = subControlRect(CC_ComboBox, option, SC_ComboBoxArrow, widget);
                if (arrowRect.isValid()) {
                    painter->setRenderHint(QPainter::Antialiasing);
                    painter->setPen(Qt::NoPen);
                    painter->setBrush(QColor(60, 60, 60));

                    QPolygonF triangle;
                    triangle << QPointF(arrowRect.center().x() - 5, arrowRect.top() + 4)
                             << QPointF(arrowRect.center().x() + 5, arrowRect.top() + 4)
                             << QPointF(arrowRect.center().x(), arrowRect.bottom() - 4);
                    painter->drawPolygon(triangle);
                }
                return;
            }
        }
        QProxyStyle::drawComplexControl(control, option, painter, widget);
    }

    void drawPrimitive(PrimitiveElement element, const QStyleOption *option,
                      QPainter *painter, const QWidget *widget) const override
    {
        if (element == PE_IndicatorArrowDown) {
            return;
        }
        QProxyStyle::drawPrimitive(element, option, painter, widget);
    }
};

// 自定义委托：实现复选框点击
class CheckBoxDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    CheckBoxDelegate(QObject *parent = nullptr) : QItemDelegate(parent) {}

    bool editorEvent(QEvent *event, QAbstractItemModel *model,
                     const QStyleOptionViewItem &option, const QModelIndex &index) override
    {
        if (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonRelease) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::LeftButton) {
                // 切换复选框状态
                Qt::CheckState state = index.model()->data(index, Qt::CheckStateRole).value<Qt::CheckState>();
                Qt::CheckState newState = (state == Qt::Checked) ? Qt::Unchecked : Qt::Checked;
                model->setData(index, newState, Qt::CheckStateRole);
                return true;
            }
        }
        return QItemDelegate::editorEvent(event, model, option, index);
    }

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        // 绘制复选框
        Qt::CheckState state = index.model()->data(index, Qt::CheckStateRole).value<Qt::CheckState>();
        QStyleOptionButton checkBoxOption;
        checkBoxOption.rect = option.rect;
        checkBoxOption.state = (state == Qt::Checked) ? QStyle::State_On : QStyle::State_Off;
        checkBoxOption.state |= QStyle::State_Enabled;
        checkBoxOption.text = index.model()->data(index, Qt::DisplayRole).toString();
        checkBoxOption.text += "                    "; // 增加间距

        QStyle *style = option.widget ? option.widget->style() : QApplication::style();
        style->drawControl(QStyle::CE_CheckBox, &checkBoxOption, painter);
    }
};

// 更新分类筛选下拉框显示文本（函数声明）
void updateCategoryFilterText(QComboBox *comboBox, QStandardItemModel *model);

// 自定义日历Widget - 5行7列显示
class CalendarWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CalendarWidget(QWidget *parent = nullptr);
    void setMonth(const QDate &date);
    QDate selectedDate() const { return m_selectedDate; }

signals:
    void dateSelected(const QDate &date);
    void dateDoubleClicked(const QDate &date);

public slots:
    void setSelectedDate(const QDate &date);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;

private:
    void drawCalendar(QPainter &painter);
    QDate m_currentMonth;
    QDate m_selectedDate;
    QVector<QVector<QDate>> m_days; // 5行7列的日期矩阵
    QVector<QPair<QString, QColor>> m_taskInfos; // 日期的任务信息

    struct DayCell {
        QDate date;
        bool isCurrentMonth;
        int taskCount;
        TaskStatus firstTaskStatus;
        QString firstTaskTitle;
    };
    QVector<QVector<DayCell>> m_cells;

    int cellWidth() const;
    int cellHeight() const;
    void updateCells();

public:
    void setTaskInfos(const QMap<QDate, QVector<QPair<QString, TaskStatus>>> &tasks);
};

// 工作日志主Widget
class WorkLogWidget : public QWidget
{
    Q_OBJECT

public:
    explicit WorkLogWidget(Database *database, QWidget *parent = nullptr);
    ~WorkLogWidget();

private slots:
    void onAddTask();
    void onEditTask();
    void onDeleteTask();
    void onCompleteTask();
    void onPauseTask();
    void onTaskSelectionChanged();
    void onTaskDoubleClicked(int row, int column);
    void onRefreshTasks();
    void onFilterChanged();
    void onGenerateReport();
    void onExportReport();
    void onShowStatistics();
    void onQuickAddTask();
    void onStartTask();
    void onStopTask();
    void onTaskContextMenu(const QPoint &pos);
    void onTimeFilterChanged(int index);
    void onViewDateChanged(const QDate &date);
    void onPrevDay();
    void onNextDay();
    void onToday();
    void onPieChartDoubleClick();
    void onCalendarDateSelected(const QDate &date);
    void onPrevMonth();
    void onMonthChanged(const QDate &date);
    void onNextMonth();
    void onGoToToday();
    void onViewModeChanged(int index);
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    void setupUI();
    void setupTaskTable();
    void setupToolbar();
    void setupStatisticsPanel();
    void setupCalendarView();
    void loadCategories();
    void loadTasks();
    void refreshTaskTable();
    void refreshCalendarView();
    void updateStatistics();
    void initDefaultCategories();
    Task getCurrentTask();
    Category getCurrentCategory();
    QString getPriorityString(TaskPriority priority);
    QString getStatusString(TaskStatus status);
    QString getDurationString(double hours);
    void showTaskDialog(Task *task = nullptr);
    void showCategoryDialog(Category *category = nullptr);
    QString generateWeeklyReport(const QDateTime &startDate, const QDateTime &endDate);
    QString generateMonthlyReport(const QDateTime &startDate, const QDateTime &endDate);
    QString generateQuarterlyReport(const QDateTime &startDate, const QDateTime &endDate);
    bool exportToMarkdown(const QString &content, const QString &fileName);
    bool exportToText(const QString &content, const QString &fileName);
    bool exportToPDF(const QString &content, const QString &fileName);
    bool exportToWord(const QString &content, const QString &fileName);
    QString markdownToHTML(const QString &markdown);
    QString getCachedReport(const QString &cacheKey);
    void cacheReport(const QString &cacheKey, const QString &report);
    bool isCacheValid(const QString &cacheKey, int maxAgeMinutes = 30);
    void clearReportCache();
    void logOperation(const QString &operation, const QString &details);
    bool checkPermission(const QString &permission);
    void updateTaskRow(int row, const Task &task);
    void analyzeTaskWithAI(const QString &title, QLineEdit *titleEdit, QTextEdit *descEdit, 
                           QComboBox *categoryCombo, QComboBox *priorityCombo, QDoubleSpinBox *durationSpin,
                           QLabel *aiStatusLabel, QPushButton *aiBtn, QLineEdit *tagsEdit = nullptr);
    void analyzeTaskWithAI(const QString &model, const QString &title, QLineEdit *titleEdit, QTextEdit *descEdit,
                           QComboBox *categoryCombo, QComboBox *priorityCombo, QDoubleSpinBox *durationSpin,
                           QLabel *aiStatusLabel, QPushButton *aiBtn, QLineEdit *tagsEdit = nullptr);
    void onAIAnalysisFinished(const QString &title, QLineEdit *titleEdit, QTextEdit *descEdit,
                               QComboBox *categoryCombo, QComboBox *priorityCombo, QDoubleSpinBox *durationSpin,
                               QLabel *aiStatusLabel, QPushButton *aiBtn);
    QString getExistingCategories();
    QString getAIServiceKey();
    void handleAIResponse(QPointer<QNetworkReply> reply, const QString &title, QLineEdit *titleEdit, QTextEdit *descEdit,
                          QComboBox *categoryCombo, QComboBox *priorityCombo, QDoubleSpinBox *durationSpin,
                          QLabel *aiStatusLabel, QPushButton *aiBtn, const QString &logFileName,
                          QLineEdit *tagsEdit = nullptr);
    void analyzeWithLocalAI(const QString &title, QLineEdit *titleEdit, QTextEdit *descEdit,
                             QComboBox *categoryCombo, QComboBox *priorityCombo, QDoubleSpinBox *durationSpin,
                             QLabel *aiStatusLabel, QLineEdit *tagsEdit = nullptr);
    void parseAIResponse(const QString &response, QLineEdit *titleEdit, QTextEdit *descEdit,
                         QComboBox *categoryCombo, QComboBox *priorityCombo, QDoubleSpinBox *durationSpin,
                         QLineEdit *tagsEdit);
    QString generateReportWithAI(const QString &reportType, const QString &reportData);
    void onAIGenerateReport(QTextEdit *reportEdit, const QString &reportType, const QString &reportData);
    void onAIGenerateReport(const QString &model, QTextEdit *reportEdit, const QString &reportType, const QString &reportData);
    
    void setSettingsWidget(void *settings);
    QString getCurrentAIModel();
    QString getAPIKey();
    QString getAPIKey(const QString &model);
    QString getAPIEndpoint();
    QString getDefaultEndpoint(const QString &model);
    QString getModelNameForAPI(const QString &model);
    void loadAIKeysToComboBox(QComboBox *comboBox);

    Database *db;
    
    QSplitter *mainSplitter;
    QSplitter *leftSplitter;
    
    QWidget *leftPanel;
    QWidget *rightPanel;

    QTableWidget *taskTable;
    QStackedWidget *viewStacker;
    QWidget *calendarViewContainer;
    CalendarWidget *calendarWidget;

    QButtonGroup *viewModeGroup;
    QPushButton *tableViewBtn;
    QPushButton *calendarViewBtn;
    QDateEdit *monthDateEdit;  // 日历视图的年份月份选择器

    // 视图导航布局（使用QWidget包装以控制可见性）
    QWidget *tableDateWidget;
    QWidget *calendarNavWidget;

    // 内部布局指针
    QHBoxLayout *tableDateLayout;
    QHBoxLayout *calendarNavLayout;

    // 日历导航按钮
    QPushButton *prevMonthBtn;
    QPushButton *nextMonthBtn;
    QPushButton *goTodayBtn;

    QLineEdit *searchEdit;
    QComboBox *statusFilter;
    QComboBox *priorityFilter;
    QComboBox *categoryFilter;
    QDateEdit *taskViewDate;
    
    // 新增统计相关控件
    QComboBox *timeFilterCombo;
    QDateEdit *customStartDate;
    QDateEdit *customEndDate;
    QListWidget *categoryStatsList;
    QLabel *taskStatsLabels[4];
    
    QPushButton *addTaskBtn;
    QPushButton *editTaskBtn;
    QPushButton *deleteTaskBtn;
    QPushButton *completeTaskBtn;
    QPushButton *refreshBtn;
    QPushButton *generateReportBtn;
    QPushButton *exportBtn;
    QPushButton *quickAddBtn;
    
    QLabel *totalTasksLabel;
    QLabel *completedTasksLabel;
    QLabel *totalHoursLabel;
    QLabel *currentCategoryLabel;
    
    QProgressBar *todayProgress;
    QProgressBar *weekProgress;
    QProgressBar *completionProgressBar;
    
    QChartView *pieChartView;
    QChart *pieChart;
    
    // 时间段枚举
    enum TimePeriod {
        Today,
        ThisWeek,
        ThisMonth,
        ThisYear,
        Custom
    };

    // 视图模式枚举
    enum ViewMode {
        TableView,
        CalendarView
    };

    // 当前日历月份
    QDate currentCalendarMonth;

    Task *currentRunningTask;
    QTimer *taskTimer;
    QDateTime taskStartTime;
    
    QNetworkAccessManager *networkManager;
    
    // 报告缓存
    QHash<QString, QString> reportCache;
    QDateTime lastCacheUpdate;
};

#endif
