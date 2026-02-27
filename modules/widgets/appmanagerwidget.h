#ifndef APPMANAGERWIDGET_H
#define APPMANAGERWIDGET_H

#include <QWidget>
#include <QListView>
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
#include <QStandardItemModel>
#include "modules/core/database.h"
#include "modules/dialogs/iconselectordialog.h"

QT_BEGIN_NAMESPACE
namespace Ui { class AppManagerWidget; }
QT_END_NAMESPACE

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
    ~AppManagerWidget();

signals:
    void resetAppsRequested();

public slots:
    void refreshAppList();
    
private slots:
    void onAddApp();
    void onDeleteApp();
    void onLaunchApp();
    void onAppItemDoubleClicked(const QModelIndex &index);
    void onShowContextMenu(const QPoint &pos);
    void onRenameApp();
    void onChangeIcon();
    void onChangePath();
    void onChangeArguments();
    void onIconViewMode();
    void onListViewMode();
    void onMoveUp();
    void onMoveDown();
    void onInitApps();

private:
    void setupUI();
    void launchApp(const AppInfo &app);
    QIcon getAppIcon(const AppInfo &app);
    void saveAppOrder();
    void addExecutableApp();
    void addWebsiteApp();
    void addFolderApp();
    void addDocumentApp();
    
    Database *db;
    Ui::AppManagerWidget *ui;
    QStandardItemModel *appModel;
    QFileIconProvider iconProvider;
    AppIconDelegate *iconDelegate;
};

#endif
