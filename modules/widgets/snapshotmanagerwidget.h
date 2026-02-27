#ifndef SNAPSHOTMANAGERWIDGET_H
#define SNAPSHOTMANAGERWIDGET_H

#include <QWidget>
#include <QListView>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QProcess>
#include <QMenu>
#include <QAction>
#include <QFileInfo>
#include <QDir>
#include <QInputDialog>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QStandardItemModel>
#include <QDesktopServices>
#include <QUrl>
#include "modules/core/database.h"

class SnapshotIconDelegate : public QStyledItemDelegate
{
public:
    explicit SnapshotIconDelegate(QObject *parent = nullptr);
    
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

class SnapshotManagerWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SnapshotManagerWidget(Database *db, QWidget *parent = nullptr);
    ~SnapshotManagerWidget();

signals:
    void statusMessageRequested(const QString &message);

public slots:
    void refreshSnapshotList();
    
private slots:
    void onAddFolderSnapshot();
    void onAddWebsiteSnapshot();
    void onAddDocumentSnapshot();
    void onDeleteSnapshot();
    void onOpenSnapshot();
    void onSnapshotItemDoubleClicked(const QModelIndex &index);
    void onShowContextMenu(const QPoint &pos);
    void onRenameSnapshot();
    void onToggleFavorite();
    void onSearchSnapshot(const QString &text);
    void onFilterByType(int index);
    void onShowAll();
    void onShowFavorites();
    void onSnapshotDetails();

private:
    void setupUI();
    void openSnapshot(const SnapshotInfo &snapshot);
    QIcon getSnapshotIcon(const SnapshotInfo &snapshot);
    QString formatSize(qint64 size);
    void saveSnapshotOrder();
    
    Database *db;
    QStandardItemModel *snapshotModel;
    SnapshotIconDelegate *iconDelegate;
    QListView *snapshotListView;
    QLineEdit *searchEdit;
    QComboBox *typeFilterCombo;
    QPushButton *showAllButton;
    QPushButton *showFavoritesButton;
    QLabel *statusLabel;
    
    bool showingFavorites;
};

#endif
