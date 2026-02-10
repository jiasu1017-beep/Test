#ifndef SETTINGSWIDGET_H
#define SETTINGSWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QGroupBox>
#include <QMessageBox>
#include "database.h"

class SettingsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SettingsWidget(Database *db, QWidget *parent = nullptr);

private slots:
    void onAutoStartToggled(int state);
    void onMinimizeToTrayToggled(int state);
    void onShowClosePromptToggled(int state);
    void onAboutClicked();

private:
    void setupUI();
    
    Database *db;
    QCheckBox *autoStartCheck;
    QCheckBox *minimizeToTrayCheck;
    QCheckBox *showClosePromptCheck;
    QLabel *statusLabel;
};

#endif
