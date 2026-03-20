#ifndef APPMANAGERWIDGET_H
#define APPMANAGERWIDGET_H

#include <QWidget>
#include <QListView>
#include <QPushButton>
#include <QToolButton>
#include <QLabel>
#include <QComboBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QMenu>
#include <QAction>
#include <QFileIconProvider>
#include <QFileInfo>
#include <QInputDialog>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QFont>
#include <QPainterPath>
#include <QtMath>
#include <QHash>
#include <QPixmap>
#include "modules/core/database.h"
#include "modules/core/applicationmanager.h"
#include "modules/dialogs/batchimportdialog.h"
#include "modules/dialogs/iconselectordialog.h"

QT_BEGIN_NAMESPACE
namespace Ui { class AppManagerWidget; }
QT_END_NAMESPACE

// 自定义角色，用于存储和获取数据
enum AppManagerRoles {
    Role_IconPath = Qt::UserRole + 2
};

class AppIconDelegate : public QStyledItemDelegate
{
public:
    explicit AppIconDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    // 缓存pixmap，使用iconPath作为key（稳定key）
    mutable QHash<QString, QPixmap> pixmapCache;
};

class AppListDelegate : public QStyledItemDelegate
{
public:
    explicit AppListDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    // 缓存pixmap，使用iconPath作为key（稳定key）
    mutable QHash<QString, QPixmap> pixmapCache;
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
    void onViewModeChanged(QAction *action);
    void onMoveUp();
    void onMoveDown();
    void onInitApps();
    void onSortModeChanged(QAction *action);

private:
    void setupUI();
    void loadCategories();
    void filterAppsByCategory(const QString &category);
    void launchApp(const AppInfo &app);
    QIcon getAppIcon(const AppInfo &app);
    void saveAppOrder();
    void addExecutableApp();
    void addWebsiteApp();
    void addFolderApp();
    void addDocumentApp();
    void addAppsFromRegistry();
    void addBookmarksFromBrowsers();
    void addRunningApps();
    
    Database *db;
    ApplicationManager *appManager;
    Ui::AppManagerWidget *ui;
    QStandardItemModel *appModel;
    QFileIconProvider iconProvider;
    AppIconDelegate *iconDelegate;
    AppListDelegate *listDelegate;
    QString currentCategory;
    int currentType;
    QList<AppInfo> allApps;
    AppSortMode currentSortMode;
    QMenu *sortModeMenu;
    QToolButton *sortModeButton;
    QToolButton *viewModeButton;
};

#endif
