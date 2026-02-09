#ifndef FISHMODEWIDGET_H
#define FISHMODEWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QLineEdit>
#include <QTimer>
#include <QProcess>
#include <QMessageBox>

class FishModeWidget : public QWidget
{
    Q_OBJECT
public:
    explicit FishModeWidget(QWidget *parent = nullptr);

private slots:
    void onBossKeyPressed();
    void onToggleFishMode();
    void onOpenFakeWindow();

private:
    void setupUI();
    
    bool isFishModeActive;
    QPushButton *bossKeyButton;
    QPushButton *toggleButton;
    QPushButton *fakeWindowButton;
    QLabel *statusLabel;
    QTimer *hideTimer;
};

#endif
