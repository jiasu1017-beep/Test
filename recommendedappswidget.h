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
#include <QLineEdit>
#include <QCheckBox>
#include <QSet>
#include <QToolButton>
#include <QPropertyAnimation>

struct RecommendedAppInfo {
    QString name;
    QString url;
    QString description;
    QString iconEmoji;
    QString category;
    bool isFavorite;
};

struct CategoryInfo {
    QString name;
    QString iconEmoji;
    QVector<RecommendedAppInfo> apps;
};

class RecommendedAppsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit RecommendedAppsWidget(QWidget *parent = nullptr);

private slots:
    void openAppUrl(const QString &url);
    void toggleFavorite(const QString &appName);
    void onSearchTextChanged(const QString &text);
    void onShowFavoritesChanged(int state);
    void refreshAllViews();

private:
    void setupUI();
    void loadAppData();
    QWidget* createCategoryWidget(const CategoryInfo &category);
    QWidget* createAppCard(const RecommendedAppInfo &app);
    void applyFilter();

    QVector<CategoryInfo> categories;
    QVector<RecommendedAppInfo> allApps;
    QSet<QString> favoriteApps;
    
    QTabWidget *tabWidget;
    QLineEdit *searchEdit;
    QCheckBox *showFavoritesCheck;
    QWidget *allAppsWidget;
    QScrollArea *allAppsScrollArea;
};

#endif // RECOMMENDEDAPPSWIDGET_H
