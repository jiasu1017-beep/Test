#include "bottomappbar.h"
#include "modules/widgets/remotedesktopwidget.h"
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QScrollArea>
#include <QScrollBar>
#include <QHBoxLayout>
#include <QLinearGradient>
#include <QRadialGradient>
#include <QFileInfo>
#include <QFile>
#include <QProcess>
#include <QDesktopServices>
#include <QUrl>
#include <QApplication>
#include <QStyle>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QFileIconProvider>
#include <QEasingCurve>
#include <QStandardPaths>
#include <QTextStream>

BottomAppBarItem::BottomAppBarItem(const AppInfo &app, QWidget *parent)
    : QWidget(parent), m_app(app), m_iconSize(DEFAULT_ICON_SIZE), m_scale(1.0), m_opacity(1.0), m_isHovered(false), m_iconLoaded(false)
{
    setFixedSize(m_iconSize, m_iconSize + ICON_PADDING);
    setAttribute(Qt::WA_TranslucentBackground);
    setCursor(Qt::PointingHandCursor);
    
    m_shadowEffect = new QGraphicsDropShadowEffect(this);
    m_shadowEffect->setBlurRadius(0);
    m_shadowEffect->setColor(QColor(0, 0, 0, 0));
    m_shadowEffect->setOffset(0, 0);
    setGraphicsEffect(m_shadowEffect);
    
    m_scaleAnimation = new QPropertyAnimation(this, "scale", this);
    m_scaleAnimation->setDuration(200);
    m_scaleAnimation->setEasingCurve(QEasingCurve::OutCubic);
    
    setToolTip(app.name);
    
    // 预加载图标
    loadIcon();
}

void BottomAppBarItem::setIconSize(int size)
{
    m_iconSize = size;
    setFixedSize(m_iconSize, m_iconSize + 20);
}

qreal BottomAppBarItem::scale() const
{
    return m_scale;
}

void BottomAppBarItem::setScale(qreal scale)
{
    m_scale = scale;
    update();
}

void BottomAppBarItem::enterEvent(QEvent *event)
{
    m_isHovered = true;
    
    // 低配电脑优化：简化阴影效果
    m_shadowEffect->setBlurRadius(8);  // 减少阴影模糊半径
    m_shadowEffect->setColor(QColor(0, 0, 0, 25));  // 降低阴影透明度
    m_shadowEffect->setOffset(0, 2);  // 减少阴影偏移
    
    m_scaleAnimation->stop();
    m_scaleAnimation->setStartValue(m_scale);
    m_scaleAnimation->setEndValue(1.08);  // 减少缩放幅度
    m_scaleAnimation->setDuration(150);  // 缩短动画时间
    m_scaleAnimation->start();
    
    update();
    QWidget::enterEvent(event);
}

void BottomAppBarItem::leaveEvent(QEvent *event)
{
    m_isHovered = false;
    
    // 低配电脑优化：快速恢复
    m_shadowEffect->setBlurRadius(0);
    m_shadowEffect->setColor(QColor(0, 0, 0, 0));
    m_shadowEffect->setOffset(0, 0);
    
    m_scaleAnimation->stop();
    m_scaleAnimation->setStartValue(m_scale);
    m_scaleAnimation->setEndValue(1.0);
    m_scaleAnimation->setDuration(120);  // 缩短动画时间
    m_scaleAnimation->start();
    
    update();
    QWidget::leaveEvent(event);
}

void BottomAppBarItem::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_scaleAnimation->stop();
        m_scaleAnimation->setStartValue(m_scale);
        m_scaleAnimation->setEndValue(0.92);
        m_scaleAnimation->setDuration(80);
        m_scaleAnimation->start();
    }
    QWidget::mousePressEvent(event);
}

void BottomAppBarItem::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_scaleAnimation->stop();
        m_scaleAnimation->setStartValue(m_scale);
        m_scaleAnimation->setEndValue(1.0);
        m_scaleAnimation->setDuration(150);
        m_scaleAnimation->start();
        
        emit clicked(m_app);
    }
    QWidget::mouseReleaseEvent(event);
}

void BottomAppBarItem::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    
    int iconAreaSize = static_cast<int>(m_iconSize * m_scale);
    int iconOffset = (m_iconSize - iconAreaSize) / 2;
    
    QRectF iconRect(iconOffset, iconOffset, iconAreaSize, iconAreaSize);
    qreal radius = iconAreaSize / 2.0 - 1;
    
    painter.setOpacity(m_opacity);
    
    QPainterPath clipPath;
    clipPath.addEllipse(QPointF(iconRect.center().x(), iconRect.center().y()), radius, radius);
    painter.setClipPath(clipPath);
    
    QColor bgColor(BottomAppBar::Colors::BG_NORMAL, BottomAppBar::Colors::BG_NORMAL, BottomAppBar::Colors::BG_NORMAL);
    if (m_isHovered) {
        bgColor = QColor(BottomAppBar::Colors::BG_HOVER, BottomAppBar::Colors::BG_HOVER, BottomAppBar::Colors::BG_HOVER);
    }
    
    QRadialGradient radialGrad(iconRect.center().x(), iconRect.center().y() - radius * 0.3, radius * 1.5);
    radialGrad.setColorAt(0, bgColor.lighter(102));
    radialGrad.setColorAt(0.7, bgColor);
    radialGrad.setColorAt(1, bgColor.darker(98));
    painter.fillPath(clipPath, radialGrad);
    
    QPen borderPen(QColor(BottomAppBar::Colors::BORDER_COLOR, BottomAppBar::Colors::BORDER_COLOR, BottomAppBar::Colors::BORDER_COLOR_ALT));
    borderPen.setWidthF(1.0);
    painter.setPen(borderPen);
    painter.setBrush(Qt::NoBrush);
    painter.drawPath(clipPath);
    
    painter.setClipping(false);
    
    // 使用缓存的图标
    int pixmapSize = static_cast<int>(iconAreaSize - 14);
    QRectF pixmapRect(iconRect.center().x() - pixmapSize / 2.0,
                      iconRect.center().y() - pixmapSize / 2.0,
                      pixmapSize, pixmapSize);
    painter.drawPixmap(pixmapRect.toRect(), m_cachedIcon);
}

void BottomAppBarItem::loadIcon()
{
    if (m_iconLoaded) return;
    
    QIcon icon;
    if (!m_app.iconPath.isEmpty() && QFile::exists(m_app.iconPath)) {
        icon = QIcon(m_app.iconPath);
    }
    
    if (icon.isNull()) {
        icon = getDefaultIcon();
    }
    
    int pixmapSize = m_iconSize - 14;
    m_cachedIcon = icon.pixmap(QSize(pixmapSize, pixmapSize));
    m_iconLoaded = true;
}

QIcon BottomAppBarItem::getDefaultIcon()
{
    return ApplicationManager::getAppIcon(m_app);
}

BottomAppBar::BottomAppBar(Database *db, QWidget *parent)
    : QWidget(parent), m_db(db), m_iconSize(DEFAULT_ICON_SIZE), m_height(DEFAULT_HEIGHT), m_isLoading(true), m_theme(Light)
{
    appManager = new ApplicationManager(m_db, this);
    
    m_bgColor = Colors::LIGHT_BG;
    m_borderColor = Colors::LIGHT_BORDER;
    m_hoverBgColor = Colors::LIGHT_HOVER;
    m_iconBaseColor = Colors::LIGHT_ICON_BASE;
    
    setupUI();
    setupAnimations();
    refreshApps();
}

void BottomAppBar::setupUI()
{
    setFixedHeight(m_height);
    setStyleSheet(QString("QWidget { background-color: %1; }").arg(m_bgColor));
    
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(16, 0, 16, 0);
    mainLayout->setSpacing(0);
    
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setStyleSheet("QScrollArea { border: none; background-color: transparent; }");
    m_scrollArea->installEventFilter(this);
    
    m_contentWidget = new QWidget();
    m_contentWidget->setFixedHeight(m_height);
    m_contentWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    m_contentLayout = new QHBoxLayout(m_contentWidget);
    m_contentLayout->setContentsMargins(12, 6, 12, 6);
    m_contentLayout->setSpacing(12);
    m_contentLayout->setAlignment(Qt::AlignCenter);
    
    m_scrollArea->setWidget(m_contentWidget);
    mainLayout->addWidget(m_scrollArea, 1);
    
    m_scrollIndicator = new QLabel(this);
    m_scrollIndicator->setFixedSize(48, 3);
    m_scrollIndicator->setStyleSheet(
        "QLabel { background-color: #bdbdbd; border-radius: 1.5px; }"
    );
    m_scrollIndicator->hide();
    mainLayout->addWidget(m_scrollIndicator);
    
    QGraphicsOpacityEffect *opacityEffect = new QGraphicsOpacityEffect(m_scrollIndicator);
    m_scrollIndicator->setGraphicsEffect(opacityEffect);
    
    m_loadTimer = new QTimer(this);
    m_loadTimer->setSingleShot(true);
    m_loadTimer->setInterval(LOAD_DELAY_MS);
    connect(m_loadTimer, &QTimer::timeout, this, [this]() {
        m_isLoading = false;
        update();
    });
    
    m_scrollIndicatorTimer = new QTimer(this);
    m_scrollIndicatorTimer->setSingleShot(true);
    m_scrollIndicatorTimer->setInterval(1500);
    connect(m_scrollIndicatorTimer, &QTimer::timeout, this, [this]() {
        QGraphicsOpacityEffect *effect = qobject_cast<QGraphicsOpacityEffect*>(m_scrollIndicator->graphicsEffect());
        if (effect) {
            QPropertyAnimation *anim = new QPropertyAnimation(effect, "opacity", this);
            anim->setDuration(300);
            anim->setStartValue(1.0);
            anim->setEndValue(0.0);
            anim->start();
            connect(anim, &QPropertyAnimation::finished, this, [this, anim]() {
                m_scrollIndicator->hide();
                anim->deleteLater();
            });
        }
    });
    
    setAttribute(Qt::WA_StyledBackground);
    QPalette pal = palette();
    pal.setColor(QPalette::Window, QColor(m_bgColor));
    setPalette(pal);
}

void BottomAppBar::setupAnimations()
{
    m_scrollAnimation = new QPropertyAnimation(this);
    m_scrollAnimation->setDuration(250);
    m_scrollAnimation->setEasingCurve(QEasingCurve::OutCubic);
    connect(m_scrollAnimation, &QPropertyAnimation::finished, this, &BottomAppBar::onScrollAnimationFinished);
}

void BottomAppBar::applyTheme()
{
    if (m_theme == Dark) {
        m_bgColor = Colors::DARK_BG;
        m_borderColor = Colors::DARK_BORDER;
        m_hoverBgColor = Colors::DARK_HOVER;
        m_iconBaseColor = Colors::DARK_ICON_BASE;
    } else {
        m_bgColor = Colors::LIGHT_BG;
        m_borderColor = Colors::LIGHT_BORDER;
        m_hoverBgColor = Colors::LIGHT_HOVER;
        m_iconBaseColor = Colors::LIGHT_ICON_BASE;
    }
    
    setStyleSheet(QString("QWidget { background-color: %1; border-top: 1px solid %2; }").arg(m_bgColor).arg(m_borderColor));
    
    QPalette pal = palette();
    pal.setColor(QPalette::Window, QColor(m_bgColor));
    setPalette(pal);
}

void BottomAppBar::setTheme(Theme theme)
{
    m_theme = theme;
    applyTheme();
}

void BottomAppBar::setApps(const QList<AppInfo> &apps)
{
    qDeleteAll(m_appItems);
    m_appItems.clear();
    
    for (const AppInfo &app : apps) {
        BottomAppBarItem *item = new BottomAppBarItem(app, this);
        item->setIconSize(m_iconSize);
        connect(item, &BottomAppBarItem::clicked, this, &BottomAppBar::onAppClicked);
        m_appItems.append(item);
        m_contentLayout->addWidget(item);
    }
    
    m_loadTimer->start();
    updateScrollIndicator();
}

void BottomAppBar::refreshApps()
{
    if (m_db) {
        QList<AppInfo> apps = m_db->getAllApps();
        setApps(apps);
    }
}

void BottomAppBar::setIconSize(int size)
{
    m_iconSize = size;
    for (BottomAppBarItem *item : m_appItems) {
        item->setIconSize(size);
    }
}

void BottomAppBar::setHeight(int height)
{
    m_height = height;
    setFixedHeight(height);
}

void BottomAppBar::wheelEvent(QWheelEvent *event)
{
    if (event->angleDelta().y() != 0) {
        int delta = event->angleDelta().y();
        QScrollBar *scrollBar = m_scrollArea->horizontalScrollBar();
        int currentValue = scrollBar->value();
        int newValue = currentValue - delta;
        
        // 低配电脑优化：直接滚动，不使用动画
        scrollBar->setValue(qBound(0, newValue, scrollBar->maximum()));
        
        // 延迟更新滚动指示器，减少重绘频率
        QTimer::singleShot(50, this, &BottomAppBar::updateScrollIndicator);
        
        event->accept();
    }
}

void BottomAppBar::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updateScrollIndicator();
}

void BottomAppBar::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    
    if (m_isLoading) {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        
        QLinearGradient gradient(0, 0, width(), 0);
        gradient.setColorAt(0, QColor(m_bgColor));
        gradient.setColorAt(0.5, QColor(m_hoverBgColor));
        gradient.setColorAt(1, QColor(m_bgColor));
        painter.fillRect(rect(), gradient);
        
        painter.setPen(QColor(176, 176, 176));
        QFont font("Microsoft YaHei", 9);
        painter.setFont(font);
        painter.drawText(rect(), Qt::AlignCenter, "加载应用...");
    }
}

bool BottomAppBar::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_scrollArea) {
        if (event->type() == QEvent::Wheel) {
            wheelEvent(static_cast<QWheelEvent*>(event));
            return true;
        }
    }
    return QWidget::eventFilter(watched, event);
}

void BottomAppBar::onAppClicked(const AppInfo &app)
{
    launchApp(app);
    emit appLaunched(app);
}

void BottomAppBar::smoothScrollTo(int targetPosition)
{
    m_targetScrollPosition = targetPosition;
    
    QScrollBar *scrollBar = m_scrollArea->horizontalScrollBar();
    m_scrollAnimation->stop();
    
    m_scrollAnimation->setTargetObject(scrollBar);
    m_scrollAnimation->setPropertyName("value");
    m_scrollAnimation->setStartValue(scrollBar->value());
    m_scrollAnimation->setEndValue(targetPosition);
    
    m_scrollAnimation->start();
    
    updateScrollIndicator();
}

void BottomAppBar::onScrollAnimationFinished()
{
}

void BottomAppBar::updateScrollIndicator()
{
    QScrollBar *scrollBar = m_scrollArea->horizontalScrollBar();
    int maxScroll = scrollBar->maximum();
    
    if (maxScroll > 0 && m_scrollArea->width() > 0) {
        m_scrollIndicator->show();
        
        QGraphicsOpacityEffect *effect = qobject_cast<QGraphicsOpacityEffect*>(m_scrollIndicator->graphicsEffect());
        if (effect) {
            effect->setOpacity(1.0);
        }
        
        int divisor = maxScroll + m_scrollArea->width();
        int indicatorWidth = (divisor > 0) ? qMin(48, m_scrollArea->width() * m_scrollArea->width() / divisor) : 48;
        int indicatorPosition = (maxScroll > 0) ? (scrollBar->value() * (m_scrollArea->width() - indicatorWidth)) / maxScroll : 0;
        
        m_scrollIndicator->setFixedWidth(indicatorWidth);
        m_scrollIndicator->move(m_scrollArea->x() + indicatorPosition, height() - 6);
        
        m_scrollIndicatorTimer->stop();
        m_scrollIndicatorTimer->start();
    } else {
        m_scrollIndicator->hide();
    }
}

void BottomAppBar::launchApp(const AppInfo &app)
{
    ApplicationManager::LaunchOptions options;
    options.updateUseCount = false;
    options.refreshUI = false;
    options.silentMode = true;
    
    appManager->launchApp(app, options);
    emit appLaunched(app);
}
