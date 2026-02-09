#ifndef APPMANAGERWIDGET_H
#define APPMANAGERWIDGET_H

#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QProcess>
#include <QMenu>
#include <QAction>
#include <QFileIconProvider>
#include <QFileInfo>
#include <QInputDialog>
#include <QStyledItemDelegate>
#include <QPainter>
#include "database.h"

class AppIconDelegate : public QStyledItemDelegate
{
public:
    explicit AppIconDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}
    
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

class AppManagerWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AppManagerWidget(Database *db, QWidget *parent = nullptr);

private slots:
    void onAddApp();
    void onDeleteApp();
    void onLaunchApp();
    void onAppItemDoubleClicked(QListWidgetItem *item);
    void onShowContextMenu(const QPoint &pos);
    void onRenameApp();
    void onChangeIcon();
    void onChangePath();
    void onChangeArguments();
    void refreshAppList();
    void onIconViewMode();
    void onListViewMode();
    void onRowsMoved(const QModelIndex &parent, int start, int end, const QModelIndex &destination, int row);

private:
    void setupUI();
    void launchApp(const AppInfo &app);
    QIcon getAppIcon(const AppInfo &app);
    void saveAppOrder();
    
    Database *db;
    QListWidget *appListWidget;
    QPushButton *addButton;
    QPushButton *deleteButton;
    QPushButton *launchButton;
    QPushButton *refreshButton;
    QPushButton *iconViewButton;
    QPushButton *listViewButton;
    QFileIconProvider iconProvider;
    AppIconDelegate *iconDelegate;
};

#endif
