#ifndef RECOMMENDEDAPPSWIDGET_H
#define RECOMMENDEDAPPSWIDGET_H

#include <QWidget>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QScrollArea>
#include <QGroupBox>
#include <QUrl>
#include <QDesktopServices>
#include <QVector>
#include <QPair>

struct RecommendedAppInfo {
    QString name;
    QString url;
    QString description;
};

struct CategoryInfo {
    QString name;
    QVector<RecommendedAppInfo> apps;
};

class RecommendedAppsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit RecommendedAppsWidget(QWidget *parent = nullptr);

private slots:
    void openAppUrl(const QString &url);

private:
    void setupUI();
    void loadAppData();
    QWidget* createCategoryWidget(const CategoryInfo &category);

    QVector<CategoryInfo> categories;
    QTabWidget *tabWidget;
};

#endif // RECOMMENDEDAPPSWIDGET_H
