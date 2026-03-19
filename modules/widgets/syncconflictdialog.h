#ifndef SYNCCONFLICTDIALOG_H
#define SYNCCONFLICTDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextEdit>
#include <QJsonObject>

class SyncConflictDialog : public QDialog
{
    Q_OBJECT

public:
    enum Resolution {
        UseLocal,
        UseCloud,
        KeepBoth
    };

    explicit SyncConflictDialog(const QString& taskId, const QString& taskTitle,
                                const QJsonObject& localData, const QJsonObject& cloudData,
                                QWidget *parent = nullptr);

    Resolution getResolution() const { return m_resolution; }

private slots:
    void onUseLocalClicked();
    void onUseCloudClicked();
    void onKeepBothClicked();

private:
    void setupUi(const QString& taskTitle, const QJsonObject& localData, const QJsonObject& cloudData);

    Resolution m_resolution;
    QPushButton* m_useLocalBtn;
    QPushButton* m_useCloudBtn;
    QPushButton* m_keepBothBtn;
    QTextEdit* m_localTextEdit;
    QTextEdit* m_cloudTextEdit;
};

#endif // SYNCCONFLICTDIALOG_H
