#ifndef WORKLOGWIDGET_H
#define WORKLOGWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
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
#include <QtCharts>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>
#include "modules/core/database.h"

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
    void onAddCategory();
    void onEditCategory();
    void onDeleteCategory();
    void onCategorySelectionChanged();
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

private:
    void setupUI();
    void setupTaskTable();
    void setupCategoryTree();
    void setupToolbar();
    void setupStatisticsPanel();
    void loadCategories();
    void loadTasks();
    void refreshTaskTable();
    void refreshCategoryTree();
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
    
    void setSettingsWidget(void *settings);
    QString getCurrentAIModel();
    QString getAPIKey();
    QString getAPIEndpoint();
    QString getDefaultEndpoint(const QString &model);
    QString getModelNameForAPI(const QString &model);

    Database *db;
    
    QSplitter *mainSplitter;
    QSplitter *leftSplitter;
    
    QWidget *leftPanel;
    QWidget *rightPanel;
    
    QTreeWidget *categoryTree;
    QTableWidget *taskTable;
    
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
    QPushButton *addCategoryBtn;
    QPushButton *editCategoryBtn;
    QPushButton *deleteCategoryBtn;
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
    
    Task *currentRunningTask;
    QTimer *taskTimer;
    QDateTime taskStartTime;
    
    QNetworkAccessManager *networkManager;
    
    // 报告缓存
    QHash<QString, QString> reportCache;
    QDateTime lastCacheUpdate;
};

#endif
