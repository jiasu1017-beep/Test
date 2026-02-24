#ifndef DESKTOPSNAPSHOTDIALOG_H
#define DESKTOPSNAPSHOTDIALOG_H

#include <QDialog>
#include <QTableWidget>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include "database.h"

struct SnapshotWindowInfo {
    QString title;
    QString processName;
    QString processPath;
    HWND hwnd;
    bool isSelected;
    AppType appType;
};

struct AppTypeDetection {
    AppType type;
    QString path;
    QString name;
};

class DesktopSnapshotDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DesktopSnapshotDialog(Database *db, QWidget *parent = nullptr);

private slots:
    void onCaptureSnapshot();
    void onAddToCollection();
    void onSelectAll();
    void onDeselectAll();
    void onTableItemChanged(QTableWidgetItem *item);
    void onFilterChanged();

private:
    void setupUI();
    QList<SnapshotWindowInfo> captureOpenWindows();
    void populateTable(const QList<SnapshotWindowInfo> &windows);
    QString getProcessPathFromWindow(HWND hwnd);
    QString getProcessNameFromPath(const QString &path);
    QString extractWindowTitle(const QString &title);
    QString getExplorerFolderPath(HWND hwnd);
    QString getBrowserURL(HWND hwnd);
    QString getDocumentPath(HWND hwnd);
    AppTypeDetection detectAppType(const SnapshotWindowInfo &window);
    void applyFilters();
    bool shouldShowWindow(const SnapshotWindowInfo &window);

    Database *db;
    QTableWidget *windowTable;
    QPushButton *captureButton;
    QList<SnapshotWindowInfo> currentWindows;
    QPushButton *addToCollectionButton;
    QPushButton *selectAllButton;
    QPushButton *deselectAllButton;
    QComboBox *collectionComboBox;
    QLabel *statusLabel;
    QCheckBox *filterFolderCheckBox;
    QCheckBox *filterDocumentCheckBox;
    QCheckBox *filterWebsiteCheckBox;
    QCheckBox *filterProgramCheckBox;
    QWidget *filterContainer;
};

#endif
