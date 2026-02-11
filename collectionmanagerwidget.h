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
#include <QComboBox>
#include <QSpinBox>
#include "database.h"
#include "appmanagerwidget.h"

struct TagInfo {
    QString name;
    QString color;
    QString displayName;
    QString iconSymbol;
};

class CollectionItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit CollectionItemDelegate(QObject *parent = nullptr);
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    
    void setCollections(const QList<AppCollection> &cols) { collections = cols; }

private:
    QList<AppCollection> collections;
    QMap<QString, TagInfo> tagColors;
};

class CollectionManagerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CollectionManagerWidget(Database *db, QWidget *parent = nullptr);
    ~CollectionManagerWidget();
    
    void selectFirstCollection();

public slots:
    void refreshCollectionList();
    void refreshCollectionApps();

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
    void onShowCollectionContextMenu(const QPoint &pos);
    void onLaunchAppFromCollection();
    void onRenameCollection();
    void onEditCollectionProperties();
    void onExportCollection();

private:
    void setupUI();
    void runApp(const AppInfo &app);
    QIcon getAppIcon(const AppInfo &app);
    void launchApp(const AppInfo &app);
    void initTagColors();
    QMap<QString, TagInfo> getTagColors() const { return tagColors; }
    void showCollectionPropertiesDialog(AppCollection &collection, bool isNew = false);

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
    QLabel *collectionTagLabel;
    QLabel *collectionAppsLabel;
    
    AppIconDelegate *iconDelegate;
    CollectionItemDelegate *collectionDelegate;
    QFileIconProvider iconProvider;
    
    int currentCollectionId;
    QMap<QString, TagInfo> tagColors;
};

#endif
