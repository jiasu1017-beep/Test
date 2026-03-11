#ifndef BATCHIMPORTDIALOG_H
#define BATCHIMPORTDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPushButton>
#include <QCheckBox>
#include <QTimer>
#include <QMap>
#include <QIcon>
#include <QPixmap>
#include <QStyle>
#include <QApplication>
#include <QMessageBox>
#include <QSizePolicy>

#include "modules/core/database.h"

class BatchImportListItemWidget : public QWidget
{
    Q_OBJECT
public:
    explicit BatchImportListItemWidget(const AppInfo &app, bool showIcon = true,
                                       QWidget *parent = nullptr);

    QCheckBox *checkBox;
    QLabel *iconLabel;
    QLabel *nameLabel;
    AppInfo appInfo;

private:
    void setupUI(const AppInfo &app, bool showIcon);
};

class BatchImportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BatchImportDialog(const QString &title, const QString &labelText,
                               const QList<AppInfo> &items, Database *db,
                               QWidget *parent = nullptr);
    ~BatchImportDialog();

private:
    void setupUI();
    void setupConnections();
    void performSearch();
    void selectAllVisible();
    void deselectAll();
    void invertSelections();
    void importSelected();

    Database *db;
    QString m_labelText;
    QList<AppInfo> allItems;
    QMap<int, BatchImportListItemWidget*> itemWidgets;

    QLabel *label;
    QLineEdit *searchBox;
    QListWidget *listWidget;
    QPushButton *btnSelectAll;
    QPushButton *btnDeselectAll;
    QPushButton *btnInvertSelect;
    QPushButton *btnImport;
    QPushButton *btnCancel;
    QTimer *searchTimer;
};

#endif // BATCHIMPORTDIALOG_H
