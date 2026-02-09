#ifndef SHUTDOWNWIDGET_H
#define SHUTDOWNWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QRadioButton>
#include <QSpinBox>
#include <QTimeEdit>
#include <QTimer>
#include <QProcess>
#include <QMessageBox>
#include <QButtonGroup>
#include <QGroupBox>

class ShutdownWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ShutdownWidget(QWidget *parent = nullptr);

private slots:
    void onStartTimer();
    void onCancelTimer();
    void onTimerTick();
    void onShutdownNow();
    void onRestartNow();
    void onSleepNow();

private:
    void setupUI();
    void executeAction(int action);
    
    QTimer *countdownTimer;
    int remainingSeconds;
    int currentAction;
    
    QRadioButton *shutdownRadio;
    QRadioButton *restartRadio;
    QRadioButton *sleepRadio;
    QRadioButton *countdownRadio;
    QRadioButton *timeRadio;
    
    QSpinBox *hourSpin;
    QSpinBox *minuteSpin;
    QSpinBox *secondSpin;
    QTimeEdit *timeEdit;
    
    QPushButton *startButton;
    QPushButton *cancelButton;
    QLabel *statusLabel;
    QLabel *countdownLabel;
};

#endif
