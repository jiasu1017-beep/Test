#include "recommendedappswidget.h"
#include <QApplication>
#include <QStyle>
#include <QGridLayout>
#include <QFrame>
#include <QShowEvent>

RecommendedAppsWidget::RecommendedAppsWidget(QWidget *parent)
    : QWidget(parent)
{
    updater = new AppCollectionUpdater(this);
    
    connect(updater, &AppCollectionUpdater::updateCheckStarted, this, &RecommendedAppsWidget::onUpdateCheckStarted);
    connect(updater, &AppCollectionUpdater::updateAvailable, this, &RecommendedAppsWidget::onUpdateAvailable);
    connect(updater, &AppCollectionUpdater::noUpdateAvailable, this, &RecommendedAppsWidget::onNoUpdateAvailable);
    connect(updater, &AppCollectionUpdater::updateCheckFailed, this, &RecommendedAppsWidget::onUpdateCheckFailed);
    connect(updater, &AppCollectionUpdater::updateFinished, this, &RecommendedAppsWidget::onUpdateFinished);
    connect(updater, &AppCollectionUpdater::updateFailed, this, &RecommendedAppsWidget::onUpdateFailed);
    connect(updater, &AppCollectionUpdater::logMessage, this, &RecommendedAppsWidget::onLogMessage);
    
    loadAppData();
    setupUI();
    
    // 标记模块尚未打开过，第一次打开时执行更新检查
    m_hasOpened = false;
}

void RecommendedAppsWidget::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    QLabel *titleLabel = new QLabel("🎉 推荐应用", this);
    titleLabel->setStyleSheet("font-size: 28px; font-weight: bold; color: #1976d2; padding: 15px;");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);
    
    QLabel *subtitleLabel = new QLabel("精选实用工具和资源，提升您的工作效率", this);
    subtitleLabel->setStyleSheet("font-size: 14px; color: #666; padding: 5px;");
    subtitleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(subtitleLabel);
    
    QFrame *updateStatusFrame = new QFrame(this);
    updateStatusFrame->setStyleSheet("background-color: #f0f8ff; border-radius: 10px; padding: 10px;");
    QVBoxLayout *updateStatusLayout = new QVBoxLayout(updateStatusFrame);
    
    statusLabel = new QLabel("🔄 准备就绪", this);
    statusLabel->setStyleSheet("color: #1976d2; padding: 5px; font-size: 12px;");
    statusLabel->setObjectName("statusLabel");
    updateStatusLayout->addWidget(statusLabel);
    
    updateProgressBar = new QProgressBar(this);
    updateProgressBar->setMaximumHeight(8);
    updateProgressBar->setStyleSheet(
        "QProgressBar { border-radius: 4px; background-color: #e3f2fd; } "
        "QProgressBar::chunk { background-color: #1976d2; border-radius: 4px; }"
    );
    updateProgressBar->setValue(0);
    updateProgressBar->setVisible(false);
    updateStatusLayout->addWidget(updateProgressBar);
    
    mainLayout->addWidget(updateStatusFrame);
    
    QFrame *searchFrame = new QFrame(this);
    searchFrame->setStyleSheet("background-color: #f5f5f5; border-radius: 10px; padding: 10px;");
    QHBoxLayout *searchLayout = new QHBoxLayout(searchFrame);
    searchLayout->setContentsMargins(0, 0, 0, 0);
    
    searchEdit = new QLineEdit(this);
    searchEdit->setPlaceholderText("🔍 搜索应用名称或描述...");
    searchEdit->setStyleSheet("QLineEdit { padding: 10px; border: 2px solid #e0e0e0; border-radius: 8px; background-color: white; font-size: 14px; } QLineEdit:focus { border-color: #1976d2; }");
    connect(searchEdit, &QLineEdit::textChanged, this, &RecommendedAppsWidget::onSearchTextChanged);
    searchLayout->addWidget(searchEdit);
    
    searchLayout->addStretch();
    
    showFavoritesCheck = new QCheckBox("⭐ 仅显示收藏", this);
    showFavoritesCheck->setStyleSheet("QCheckBox { padding: 8px; font-size: 14px; color: #555; } QCheckBox::indicator { width: 20px; height: 20px; }");
    connect(showFavoritesCheck, &QCheckBox::stateChanged, this, &RecommendedAppsWidget::onShowFavoritesChanged);
    searchLayout->addWidget(showFavoritesCheck);
    
    searchLayout->addSpacing(10);
    
    updateButton = new QPushButton("🔄 更新", this);
    updateButton->setStyleSheet(
        "QPushButton { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #1976d2, stop:1 #42a5f5); "
        "color: white; padding: 10px 20px; border-radius: 20px; font-weight: bold; font-size: 13px; } "
        "QPushButton:hover { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #1565c0, stop:1 #1976d2); } "
        "QPushButton:disabled { background: #bdbdbd; color: #757575; }"
    );
    updateButton->setCursor(Qt::PointingHandCursor);
    connect(updateButton, &QPushButton::clicked, this, &RecommendedAppsWidget::checkForUpdates);
    searchLayout->addWidget(updateButton);
    
    mainLayout->addWidget(searchFrame);
    
    QLabel *statsLabel = new QLabel(QString("📊 共 %1 个应用，%2 个已收藏").arg(allApps.size()).arg(favoriteApps.size()), this);
    statsLabel->setStyleSheet("color: #666; padding: 10px; font-size: 12px;");
    statsLabel->setObjectName("statsLabel");
    mainLayout->addWidget(statsLabel);
    
    tabWidget = new QTabWidget(this);
    tabWidget->setStyleSheet(
        "QTabWidget::pane { border: 1px solid #e0e0e0; border-radius: 5px; } "
        "QTabBar::tab { background: #f5f5f5; padding: 12px 24px; border-top-left-radius: 8px; border-top-right-radius: 8px; font-size: 13px; font-weight: 500; } "
        "QTabBar::tab:selected { background: #1976d2; color: white; } "
        "QTabBar::tab:hover:!selected { background: #bbdefb; }"
    );
    
    allAppsWidget = new QWidget();
    QVBoxLayout *allAppsLayout = new QVBoxLayout(allAppsWidget);
    allAppsScrollArea = new QScrollArea(this);
    allAppsScrollArea->setWidgetResizable(true);
    allAppsScrollArea->setFrameShape(QFrame::NoFrame);
    allAppsScrollArea->setWidget(allAppsWidget);
    
    tabWidget->addTab(allAppsScrollArea, "🌟 全部应用");
    
    for (const auto &category : categories) {
        QWidget *categoryWidget = createCategoryWidget(category);
        tabWidget->addTab(categoryWidget, category.name);
    }
    
    mainLayout->addWidget(tabWidget);
    
    refreshAllViews();
}

QWidget* RecommendedAppsWidget::createCategoryWidget(const CategoryInfo &category)
{
    QWidget *widget = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(widget);
    
    QScrollArea *scrollArea = new QScrollArea(widget);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    
    QWidget *contentWidget = new QWidget();
    QGridLayout *gridLayout = new QGridLayout(contentWidget);
    gridLayout->setSpacing(15);
    gridLayout->setContentsMargins(10, 10, 10, 10);
    
    int row = 0, col = 0;
    for (const auto &app : category.apps) {
        QWidget *card = createAppCard(app);
        gridLayout->addWidget(card, row, col);
        col++;
        if (col >= 2) {
            col = 0;
            row++;
        }
    }
    
    gridLayout->setRowStretch(row + 1, 1);
    scrollArea->setWidget(contentWidget);
    layout->addWidget(scrollArea);
    
    return widget;
}

QWidget* RecommendedAppsWidget::createAppCard(const RecommendedAppInfo &app)
{
    QWidget *card = new QWidget();
    card->setMinimumHeight(160);
    
    // 使用更高效的样式设置方式
    card->setProperty("class", "appCard");
    
    QVBoxLayout *cardLayout = new QVBoxLayout(card);
    
    QHBoxLayout *headerLayout = new QHBoxLayout();
    
    QLabel *iconLabel = new QLabel(app.iconEmoji.isEmpty() ? "📦" : app.iconEmoji, card);
    iconLabel->setStyleSheet("font-size: 32px;");
    headerLayout->addWidget(iconLabel);
    
    QVBoxLayout *titleLayout = new QVBoxLayout();
    QLabel *nameLabel = new QLabel(app.name, card);
    nameLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #1565c0;");
    nameLabel->setWordWrap(true);
    titleLayout->addWidget(nameLabel);
    
    QLabel *categoryLabel = new QLabel(app.category, card);
    categoryLabel->setStyleSheet("font-size: 11px; color: #78909c; background-color: #e3f2fd; padding: 3px 8px; border-radius: 10px;");
    titleLayout->addWidget(categoryLabel);
    titleLayout->addStretch();
    
    headerLayout->addLayout(titleLayout);
    headerLayout->addStretch();
    
    QToolButton *favoriteBtn = new QToolButton(card);
    bool isFav = favoriteApps.contains(app.name);
    favoriteBtn->setText(isFav ? "⭐" : "☆");
    favoriteBtn->setStyleSheet(QString("QToolButton { font-size: 24px; border: none; background: transparent; } QToolButton:hover { color: %1; }").arg(isFav ? "#ff9800" : "#ffc107"));
    favoriteBtn->setCursor(Qt::PointingHandCursor);
    connect(favoriteBtn, &QToolButton::clicked, [this, app, favoriteBtn]() {
        toggleFavorite(app.name);
    });
    headerLayout->addWidget(favoriteBtn);
    
    cardLayout->addLayout(headerLayout);
    
    QLabel *descLabel = new QLabel(app.description, card);
    descLabel->setStyleSheet("font-size: 12px; color: #666; padding: 5px;");
    descLabel->setWordWrap(true);
    descLabel->setMaximumHeight(60);
    cardLayout->addWidget(descLabel);
    
    cardLayout->addStretch();
    
    QHBoxLayout *footerLayout = new QHBoxLayout();
    footerLayout->addStretch();
    
    QPushButton *openBtn = new QPushButton("🚀 打开", card);
    openBtn->setStyleSheet(
        "QPushButton { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #1976d2, stop:1 #42a5f5); "
        "color: white; padding: 8px 16px; border-radius: 15px; font-weight: bold; font-size: 12px; } "
        "QPushButton:hover { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #1565c0, stop:1 #1976d2); }"
    );
    openBtn->setCursor(Qt::PointingHandCursor);
    connect(openBtn, &QPushButton::clicked, [this, app]() {
        openAppUrl(app.url);
    });
    footerLayout->addWidget(openBtn);
    
    cardLayout->addLayout(footerLayout);
    
    // 直接设置卡片的样式
    card->setStyleSheet(
        "QWidget { background-color: white; border: 2px solid #e3f2fd; border-radius: 12px; } "
        "QWidget:hover { border-color: #1976d2; }"
    );
    
    return card;
}

void RecommendedAppsWidget::loadAppData()
{
    CategoryInfo cat1;
    cat1.name = "📝 编程开发";
    cat1.iconEmoji = "💻";
    {
        RecommendedAppInfo app;
        app.name = "Visual Studio Code";
        app.url = "https://code.visualstudio.com/";
        app.description = "强大的代码编辑器，支持多种编程语言，丰富的插件生态";
        app.iconEmoji = "📝";
        app.category = "编程开发";
        app.isFavorite = false;
        cat1.apps.append(app);
        allApps.append(app);
    }
    {
        RecommendedAppInfo app;
        app.name = "小龙Dev-C++";
        app.url = "https://github.com/royqh1979/Dev-Cpp";
        app.description = "C/C++ 集成开发环境，轻量级，适合初学者";
        app.iconEmoji = "⚙️";
        app.category = "编程开发";
        app.isFavorite = false;
        cat1.apps.append(app);
        allApps.append(app);
    }
    {
        RecommendedAppInfo app;
        app.name = "HxD";
        app.url = "https://mh-nexus.de/en/hxd/";
        app.description = "专业的二进制文件编辑器，支持磁盘编辑和内存编辑";
        app.iconEmoji = "🔧";
        app.category = "编程开发";
        app.isFavorite = false;
        cat1.apps.append(app);
        allApps.append(app);
    }
    categories.append(cat1);
    
    CategoryInfo cat2;
    cat2.name = "🎨 设计与绘图";
    cat2.iconEmoji = "🎨";
    {
        RecommendedAppInfo app;
        app.name = "High-speed Charting Control";
        app.url = "https://github.com/iwancofossati/HightChart";
        app.description = "MFC 图形控件库，高性能图表绘制";
        app.iconEmoji = "📊";
        app.category = "设计与绘图";
        app.isFavorite = false;
        cat2.apps.append(app);
        allApps.append(app);
    }
    {
        RecommendedAppInfo app;
        app.name = "创客贴设计神器";
        app.url = "https://www.chuangkit.com/";
        app.description = "在线平面设计工具，海量模板，轻松作图";
        app.iconEmoji = "🖌️";
        app.category = "设计与绘图";
        app.isFavorite = false;
        cat2.apps.append(app);
        allApps.append(app);
    }
    {
        RecommendedAppInfo app;
        app.name = "Draw.io";
        app.url = "https://app.diagrams.net/";
        app.description = "免费流程图绘制工具，功能强大，完全免费";
        app.iconEmoji = "📐";
        app.category = "设计与绘图";
        app.isFavorite = false;
        cat2.apps.append(app);
        allApps.append(app);
    }
    categories.append(cat2);
    
    CategoryInfo cat3;
    cat3.name = "🔧 实用工具";
    cat3.iconEmoji = "🛠️";
    {
        RecommendedAppInfo app;
        app.name = "Everything";
        app.url = "https://www.voidtools.com/";
        app.description = "极速文件搜索工具，秒搜全硬盘文件";
        app.iconEmoji = "🔍";
        app.category = "实用工具";
        app.isFavorite = true;
        cat3.apps.append(app);
        allApps.append(app);
        favoriteApps.insert(app.name);
    }
    {
        RecommendedAppInfo app;
        app.name = "7-Zip";
        app.url = "https://www.7-zip.org/";
        app.description = "开源压缩软件，支持多种格式，压缩率高";
        app.iconEmoji = "📦";
        app.category = "实用工具";
        app.isFavorite = false;
        cat3.apps.append(app);
        allApps.append(app);
    }
    {
        RecommendedAppInfo app;
        app.name = "Greenshot";
        app.url = "https://getgreenshot.org/";
        app.description = "截图工具，功能强大，支持多种截图方式";
        app.iconEmoji = "📸";
        app.category = "实用工具";
        app.isFavorite = false;
        cat3.apps.append(app);
        allApps.append(app);
    }
    categories.append(cat3);
}

void RecommendedAppsWidget::openAppUrl(const QString &url)
{
    if (!url.isEmpty()) {
        QDesktopServices::openUrl(QUrl(url));
    }
}

void RecommendedAppsWidget::toggleFavorite(const QString &appName)
{
    if (favoriteApps.contains(appName)) {
        favoriteApps.remove(appName);
    } else {
        favoriteApps.insert(appName);
    }
    refreshAllViews();
}

void RecommendedAppsWidget::onSearchTextChanged(const QString &text)
{
    applyFilter();
}

void RecommendedAppsWidget::onShowFavoritesChanged(int state)
{
    applyFilter();
}

void RecommendedAppsWidget::refreshAllViews()
{
    applyFilter();
    
    QLabel *statsLabel = findChild<QLabel*>("statsLabel");
    if (statsLabel) {
        statsLabel->setText(QString("📊 共 %1 个应用，%2 个已收藏").arg(allApps.size()).arg(favoriteApps.size()));
    }
}

void RecommendedAppsWidget::applyFilter()
{
    QString searchText = searchEdit->text().toLower();
    bool showFavoritesOnly = showFavoritesCheck->isChecked();
    
    // 清空现有布局
    QLayout *oldLayout = allAppsWidget->layout();
    if (oldLayout) {
        QLayoutItem *item;
        while ((item = oldLayout->takeAt(0)) != nullptr) {
            if (item->widget()) {
                item->widget()->deleteLater();
            }
            delete item;
        }
        delete oldLayout;
    }
    
    // 创建新的网格布局
    QGridLayout *gridLayout = new QGridLayout(allAppsWidget);
    gridLayout->setSpacing(15);
    gridLayout->setContentsMargins(10, 10, 10, 10);
    
    // 快速创建应用卡片
    int row = 0, col = 0;
    for (const auto &app : allApps) {
        if (showFavoritesOnly && !favoriteApps.contains(app.name)) {
            continue;
        }
        
        if (!searchText.isEmpty()) {
            if (!app.name.toLower().contains(searchText) && 
                !app.description.toLower().contains(searchText)) {
                continue;
            }
        }
        
        QWidget *card = createAppCard(app);
        gridLayout->addWidget(card, row, col);
        col++;
        if (col >= 2) {
            col = 0;
            row++;
        }
    }
    
    gridLayout->setRowStretch(row + 1, 1);
}

void RecommendedAppsWidget::updateTabs()
{
    int currentTab = tabWidget->currentIndex();
    
    while (tabWidget->count() > 1) {
        tabWidget->removeTab(1);
    }
    
    for (const auto &category : categories) {
        QWidget *categoryWidget = createCategoryWidget(category);
        tabWidget->addTab(categoryWidget, category.name);
    }
    
    if (currentTab < tabWidget->count()) {
        tabWidget->setCurrentIndex(currentTab);
    }
    
    refreshAllViews();
}

void RecommendedAppsWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    
    // 第一次打开模块时执行更新检查
    if (!m_hasOpened) {
        m_hasOpened = true;
        
        // 延迟执行更新检查，确保界面完全显示
        QTimer::singleShot(300, this, [this]() {
            statusLabel->setText("🔄 正在加载推荐应用...");
            updateProgressBar->setVisible(true);
            updateProgressBar->setValue(10);
            checkForUpdates();
        });
    }
}

void RecommendedAppsWidget::checkForUpdates()
{
    updater->checkForUpdates();
}

void RecommendedAppsWidget::onInitialLoad()
{
    statusLabel->setText("🔄 正在加载推荐应用...");
    updateProgressBar->setVisible(true);
    updateProgressBar->setValue(10);
    
    checkForUpdates();
}

void RecommendedAppsWidget::onUpdateCheckStarted()
{
    statusLabel->setText("🔄 正在检查更新...");
    updateProgressBar->setVisible(true);
    updateProgressBar->setValue(30);
    updateButton->setEnabled(false);
    updateButton->setText("⏳ 检查中...");
}

void RecommendedAppsWidget::onUpdateAvailable(int appCount)
{
    statusLabel->setText(QString("✅ 发现更新，共 %1 个应用").arg(appCount));
    updateProgressBar->setValue(70);
    updateButton->setText("📥 下载中...");
    
    categories = updater->categories();
    allApps = updater->allApps();
    
    // 更新分类标签页，updateTabs() 内部会调用 refreshAllViews() 刷新【全部应用】标签页
    updateTabs();
    
    updater->downloadUpdate();
}

void RecommendedAppsWidget::onNoUpdateAvailable()
{
    statusLabel->setText("✅ 当前已是最新版本");
    updateProgressBar->setVisible(false);
    updateProgressBar->setValue(0);
    updateButton->setEnabled(true);
    updateButton->setText("🔄 更新");
    
    // 即使没有更新，也要更新界面数据
    categories = updater->categories();
    allApps = updater->allApps();
    
    // 更新界面
    updateTabs();
}

void RecommendedAppsWidget::onUpdateCheckFailed(const QString &error)
{
    statusLabel->setText(QString("❌ 更新失败: %1").arg(error));
    updateProgressBar->setVisible(false);
    updateProgressBar->setValue(0);
    updateButton->setEnabled(true);
    updateButton->setText("🔄 更新");
}

void RecommendedAppsWidget::onUpdateFinished()
{
    statusLabel->setText("✅ 更新完成");
    updateProgressBar->setVisible(false);
    updateProgressBar->setValue(0);
    updateButton->setEnabled(true);
    updateButton->setText("🔄 更新");
    
    // 更新完成后，确保界面显示最新数据
    refreshAllViews();
}

void RecommendedAppsWidget::onUpdateFailed(const QString &error)
{
    statusLabel->setText(QString("❌ 更新失败: %1").arg(error));
    updateProgressBar->setVisible(false);
    updateProgressBar->setValue(0);
    updateButton->setEnabled(true);
    updateButton->setText("🔄 更新");
}

void RecommendedAppsWidget::onLogMessage(const QString &message)
{
    qDebug() << "[AppCollection]" << message;
}
