#ifndef REMOTEDESKTOPWIDGET_H
#define REMOTEDESKTOPWIDGET_H

#include <QWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QTextEdit>
#include <QSplitter>
#include <QLabel>
#include <QFormLayout>
#include <QDialog>
#include <QDialogButtonBox>
#include <QMenu>
#include <QAction>
#include "database.h"

class RemoteDesktopWidget : public QWidget
{
    Q_OBJECT

public:
    explicit RemoteDesktopWidget(Database *db, QWidget *parent = nullptr);

signals:
    void collectionNeedsRefresh();
    void appListNeedsRefresh();
    void statusMessageRequested(const QString &message);

private slots:
    void refreshConnectionList();
    void onSearchTextChanged(const QString &text);
    void onAddConnection();
    void onEditConnection();
    void onDeleteConnection();
    void onConnect();
    void onTestConnection();
    void onImportConnections();
    void onExportConnections();
    void onConnectionSelectionChanged();
    void onToggleFavorite();
    void onTableContextMenuRequested(const QPoint &pos);
    void onAddToAppList();
    void onAddToCollection();

private:
    void setupUI();
    void loadConnections(const QList<RemoteDesktopConnection> &connections);
    RemoteDesktopConnection getSelectedConnection();
    void updateConnectionButtons();

    Database *db;

    QTableWidget *connectionTable;
    QLineEdit *searchEdit;
    QPushButton *addButton;
    QPushButton *editButton;
    QPushButton *deleteButton;
    QPushButton *connectButton;
    QPushButton *testButton;
    QPushButton *favoriteButton;
    QPushButton *importButton;
    QPushButton *exportButton;
    QComboBox *categoryFilter;
};

class RemoteDesktopDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RemoteDesktopDialog(Database *db, QWidget *parent = nullptr);
    explicit RemoteDesktopDialog(Database *db, const RemoteDesktopConnection &connection, QWidget *parent = nullptr);

    RemoteDesktopConnection getConnection() const;

private slots:
    void validateForm();
    void onSave();

private:
    void setupUI();
    void loadConnection(const RemoteDesktopConnection &connection);

    Database *db;
    RemoteDesktopConnection m_connection;
    bool m_isEditMode;

    QLineEdit *nameEdit;
    QLineEdit *hostEdit;
    QSpinBox *portSpin;
    QLineEdit *usernameEdit;
    QLineEdit *passwordEdit;
    QLineEdit *domainEdit;
    QSpinBox *widthSpin;
    QSpinBox *heightSpin;
    QCheckBox *fullScreenCheck;
    QCheckBox *allMonitorsCheck;
    QCheckBox *audioCheck;
    QCheckBox *clipboardCheck;
    QCheckBox *printerCheck;
    QCheckBox *driveCheck;
    QComboBox *categoryCombo;
    QTextEdit *notesEdit;
    QDialogButtonBox *buttonBox;
};

#endif
