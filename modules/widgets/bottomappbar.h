#ifndef BOTTOMAPPBAR_H
#define BOTTOMAPPBAR_H

#include <QWidget>
#include <QScrollArea>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QGraphicsDropShadowEffect>
#include <QPropertyAnimation>
#include <QTimer>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QParallelAnimationGroup>
#include "modules/core/database.h"

class BottomAppBarItem : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal scale READ scale WRITE setScale)

public:
    explicit BottomAppBarItem(const AppInfo &app, QWidget *parent = nullptr);
    
    static constexpr int DEFAULT_ICON_SIZE = 56;
    static constexpr int ICON_PADDING = 20;
    static constexpr qreal HOVER_SCALE = 1.12;
    static constexpr qreal PRESS_SCALE = 0.92;
    
    AppInfo appInfo() const { return m_app; }
    void setIconSize(int size);
    qreal scale() const;
    void setScale(qreal scale);

protected:
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

signals:
    void clicked(const AppInfo &app);

private:
    void loadIcon();
    QIcon getDefaultIcon();
    
    AppInfo m_app;
    int m_iconSize;
    qreal m_scale;
    qreal m_opacity;
    bool m_isHovered;
    QPixmap m_cachedIcon;
    bool m_iconLoaded;
    QGraphicsDropShadowEffect *m_shadowEffect;
    QPropertyAnimation *m_scaleAnimation;
};

class BottomAppBar : public QWidget
{
    Q_OBJECT

public:
    explicit BottomAppBar(Database *db, QWidget *parent = nullptr);
    
    void setApps(const QList<AppInfo> &apps);
    void refreshApps();
    void setIconSize(int size);
    void setHeight(int height);
    
    enum Theme {
        Light,
        Dark
    };
    void setTheme(Theme theme);
    
    static constexpr int DEFAULT_HEIGHT = 72;
    static constexpr int DEFAULT_ICON_SIZE = 56;
    static constexpr int LOAD_DELAY_MS = 600;
    
    struct Colors {
        static constexpr const char* LIGHT_BG = "#fafafa";
        static constexpr const char* LIGHT_BORDER = "#eeeeee";
        static constexpr const char* LIGHT_HOVER = "#f5f5f5";
        static constexpr const char* LIGHT_ICON_BASE = "#fafafa";
        
        static constexpr const char* DARK_BG = "#212121";
        static constexpr const char* DARK_BORDER = "#424242";
        static constexpr const char* DARK_HOVER = "#303030";
        static constexpr const char* DARK_ICON_BASE = "#2d2d2d";
        
        static constexpr int BG_NORMAL = 250;
        static constexpr int BG_HOVER = 255;
        static constexpr int BORDER_COLOR = 230;
        static constexpr int BORDER_COLOR_ALT = 235;
    };
    
protected:
    void wheelEvent(QWheelEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

signals:
    void appLaunched(const AppInfo &app);

private slots:
    void onAppClicked(const AppInfo &app);
    void onScrollAnimationFinished();
    void updateScrollIndicator();

private:
    void setupUI();
    void setupAnimations();
    void applyTheme();
    void smoothScrollTo(int targetPosition);
    void launchApp(const AppInfo &app);
    
    Database *m_db;
    QScrollArea *m_scrollArea;
    QWidget *m_contentWidget;
    QHBoxLayout *m_contentLayout;
    QList<BottomAppBarItem*> m_appItems;
    int m_iconSize;
    int m_height;
    bool m_isLoading;
    QTimer *m_loadTimer;
    Theme m_theme;
    
    QLabel *m_scrollIndicator;
    QTimer *m_scrollIndicatorTimer;
    
    QPropertyAnimation *m_scrollAnimation;
    int m_targetScrollPosition;
    
    QString m_bgColor;
    QString m_borderColor;
    QString m_hoverBgColor;
    QString m_iconBaseColor;
};

#endif
