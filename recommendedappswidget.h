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
#include <QProgressBar>
#include <QTimer>
#include "appcollectiontypes.h"
#include "appcollectionupdater.h"

class RecommendedAppsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit RecommendedAppsWidget(QWidget *parent = nullptr);
    
    void checkForUpdates();

protected:
    void showEvent(QShowEvent *event) override;

private slots:
    void openAppUrl(const QString &url);
    void toggleFavorite(const QString &appName);
    void onSearchTextChanged(const QString &text);
    void onShowFavoritesChanged(int state);
    void refreshAllViews();
    void onUpdateCheckStarted();
    void onUpdateAvailable(int appCount);
    void onNoUpdateAvailable();
    void onUpdateCheckFailed(const QString &error);
    void onUpdateFinished();
    void onUpdateFailed(const QString &error);
    void onLogMessage(const QString &message);
    void onInitialLoad();

private:
    void setupUI();
    void loadAppData();
    QWidget* createCategoryWidget(const CategoryInfo &category);
    QWidget* createAppCard(const RecommendedAppInfo &app);
    void applyFilter();
    void updateTabs();

    QVector<CategoryInfo> categories;
    QVector<RecommendedAppInfo> allApps;
    QSet<QString> favoriteApps;
    
    QTabWidget *tabWidget;
    QLineEdit *searchEdit;
    QCheckBox *showFavoritesCheck;
    QPushButton *updateButton;
    QWidget *allAppsWidget;
    QScrollArea *allAppsScrollArea;
    QLabel *statusLabel;
    QProgressBar *updateProgressBar;
    
    AppCollectionUpdater *updater;
    bool m_hasOpened;
};

#endif // RECOMMENDEDAPPSWIDGET_H
