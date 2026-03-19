#ifndef SYNCLOGWIDGET_H
#define SYNCLOGWIDGET_H

#include <QWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>

class Database;
struct SyncLog;

class SyncLogWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SyncLogWidget(Database* db, QWidget *parent = nullptr);
    ~SyncLogWidget();

    void loadLogs();

private slots:
    void onRefreshClicked();
    void onClearClicked();
    void onEntityTypeChanged(int index);
    void onRowDoubleClicked(int row, int column);

private:
    void setupUi();
    QString getActionText(const QString& action);
    QString getResolutionText(const QString& resolution);

    Database* m_db;
    QTableWidget* m_table;
    QPushButton* m_refreshBtn;
    QPushButton* m_clearBtn;
    QComboBox* m_entityTypeCombo;
    QLabel* m_countLabel;
};

#endif // SYNCLOGWIDGET_H
