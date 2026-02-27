#ifndef SHORTCUTDIALOG_H
#define SHORTCUTDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QKeySequenceEdit>
#include <QKeySequence>
#include <QPushButton>
#include <QMessageBox>
#include <QFormLayout>

class Database;

class ShortcutDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ShortcutDialog(Database *db, QWidget *parent = nullptr);
    ~ShortcutDialog();

    QKeySequence getShortcut() const;
    void setShortcut(const QKeySequence &shortcut);

private slots:
    void onKeySequenceChanged(const QKeySequence &sequence);
    void onResetButtonClicked();
    void onOkButtonClicked();
    void onCancelButtonClicked();

private:
    void setupUI();
    bool isShortcutConflict(const QString &shortcut);
    void updateStatusMessage(const QString &message, bool isError = false);

    Database *db;
    QKeySequenceEdit *shortcutEdit;
    QLabel *statusLabel;
    QPushButton *okButton;
    QKeySequence originalShortcut;
};

#endif // SHORTCUTDIALOG_H
