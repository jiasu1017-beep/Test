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

private:
    void setupUI();
    QList<SnapshotWindowInfo> captureOpenWindows();
    void populateTable(const QList<SnapshotWindowInfo> &windows);
    QString getProcessPathFromWindow(HWND hwnd);
    QString getProcessNameFromPath(const QString &path);
    QString extractWindowTitle(const QString &title);

    Database *db;
    QTableWidget *windowTable;
    QPushButton *captureButton;
    QPushButton *addToCollectionButton;
    QPushButton *selectAllButton;
    QPushButton *deselectAllButton;
    QComboBox *collectionComboBox;
    QLabel *statusLabel;
};

#endif
