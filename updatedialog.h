#ifndef UPDATEDIALOG_H
#define UPDATEDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "updatemanager.h"

class UpdateDialog : public QDialog
{
    Q_OBJECT
public:
    explicit UpdateDialog(const UpdateInfo &info, QWidget *parent = nullptr);
    
private slots:
    void onUpdateNow();
    void onRemindLater();
    void onSkipThisVersion();
    
signals:
    void updateNow();
    void remindLater();
    void skipThisVersion();
    
private:
    void setupUI(const UpdateInfo &info);
    
    QLabel *versionLabel;
    QLabel *dateLabel;
    QLabel *sizeLabel;
    QLabel *changelogLabel;
    QPushButton *updateNowButton;
    QPushButton *remindLaterButton;
    QPushButton *skipButton;
};

#endif
