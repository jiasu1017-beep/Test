#ifndef COLLECTIONMANAGERWIDGET_H
#define COLLECTIONMANAGERWIDGET_H

#include <QWidget>
#include <QListWidget>
#include <QListView>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QStandardItemModel>
#include <QMenu>
#include <QAction>
#include <QFileIconProvider>
#include <QStyledItemDelegate>
#include <QPainter>
#include "database.h"
#include "appmanagerwidget.h"

class CollectionManagerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CollectionManagerWidget(Database *db, QWidget *parent = nullptr);
    ~CollectionManagerWidget();

private slots:
    void onCreateCollection();
    void onEditCollection();
    void onDeleteCollection();
    void onCollectionSelected(QListWidgetItem *item);
    void onAddAppToCollection();
    void onRemoveAppFromCollection();
    void onRunCollection();
    void onAppItemDoubleClicked(const QModelIndex &index);
    void onShowContextMenu(const QPoint &pos);
    void onLaunchAppFromCollection();

private:
    void setupUI();
    void refreshCollectionList();
    void refreshCollectionApps();
    void runApp(const AppInfo &app);
    QIcon getAppIcon(const AppInfo &app);
    void launchApp(const AppInfo &app);

    Database *db;
    
    QListWidget *collectionListWidget;
    QListView *appsListView;
    QStandardItemModel *appsModel;
    
    QPushButton *createCollectionButton;
    QPushButton *editCollectionButton;
    QPushButton *deleteCollectionButton;
    QPushButton *addAppButton;
    QPushButton *removeAppButton;
    QPushButton *runCollectionButton;
    
    QLabel *collectionNameLabel;
    QLabel *collectionDescLabel;
    QLabel *collectionAppsLabel;
    
    AppIconDelegate *iconDelegate;
    QFileIconProvider iconProvider;
    
    int currentCollectionId;
};

#endif
