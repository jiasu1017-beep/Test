#include "recommendedappswidget.h"
#include <QApplication>
#include <QStyle>
#include <QGridLayout>
#include <QFrame>

RecommendedAppsWidget::RecommendedAppsWidget(QWidget *parent)
    : QWidget(parent)
{
    loadAppData();
    setupUI();
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
    
    QFrame *searchFrame = new QFrame(this);
    searchFrame->setStyleSheet("background-color: #f5f5f5; border-radius: 10px; padding: 10px;");
    QHBoxLayout *searchLayout = new QHBoxLayout(searchFrame);
    
    searchEdit = new QLineEdit(this);
    searchEdit->setPlaceholderText("🔍 搜索应用名称或描述...");
    searchEdit->setStyleSheet("QLineEdit { padding: 10px; border: 2px solid #e0e0e0; border-radius: 8px; background-color: white; font-size: 14px; } QLineEdit:focus { border-color: #1976d2; }");
    connect(searchEdit, &QLineEdit::textChanged, this, &RecommendedAppsWidget::onSearchTextChanged);
    searchLayout->addWidget(searchEdit);
    
    showFavoritesCheck = new QCheckBox("⭐ 仅显示收藏", this);
    showFavoritesCheck->setStyleSheet("QCheckBox { padding: 8px; font-size: 14px; color: #555; } QCheckBox::indicator { width: 20px; height: 20px; }");
    connect(showFavoritesCheck, &QCheckBox::stateChanged, this, &RecommendedAppsWidget::onShowFavoritesChanged);
    searchLayout->addWidget(showFavoritesCheck);
    
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
    card->setStyleSheet("QWidget { background-color: white; border: 2px solid #e3f2fd; border-radius: 12px; } QWidget:hover { border-color: #1976d2; }");
    card->setMinimumHeight(160);
    
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
    descLabel->setStyleSheet("color: #546e7a; padding: 8px 0; font-size: 12px; line-height: 1.5;");
    descLabel->setWordWrap(true);
    descLabel->setMinimumHeight(40);
    cardLayout->addWidget(descLabel);
    
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    QPushButton *openButton = new QPushButton("🔗 打开链接", card);
    openButton->setStyleSheet(
        "QPushButton { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #1976d2, stop:1 #42a5f5); "
        "color: white; padding: 10px 24px; border-radius: 25px; font-weight: bold; font-size: 13px; } "
        "QPushButton:hover { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #1565c0, stop:1 #1976d2); }"
    );
    openButton->setCursor(Qt::PointingHandCursor);
    connect(openButton, &QPushButton::clicked, [this, app]() {
        openAppUrl(app.url);
    });
    
    buttonLayout->addWidget(openButton);
    cardLayout->addLayout(buttonLayout);
    
    cardLayout->addStretch();
    
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
        app.name = "ShareX";
        app.url = "https://getsharex.com/";
        app.description = "强大的截图和录屏工具，功能极其丰富";
        app.iconEmoji = "📸";
        app.category = "实用工具";
        app.isFavorite = false;
        cat3.apps.append(app);
        allApps.append(app);
    }
    {
        RecommendedAppInfo app;
        app.name = "Snipaste";
        app.url = "https://www.snipaste.com/";
        app.description = "快捷截图工具，支持贴图和标注";
        app.iconEmoji = "✂️";
        app.category = "实用工具";
        app.isFavorite = false;
        cat3.apps.append(app);
        allApps.append(app);
    }
    {
        RecommendedAppInfo app;
        app.name = "SpaceSniffer";
        app.url = "http://www.uderzo.it/main_products/space_sniffer.html";
        app.description = "磁盘空间分析工具，可视化展示文件大小";
        app.iconEmoji = "💾";
        app.category = "实用工具";
        app.isFavorite = false;
        cat3.apps.append(app);
        allApps.append(app);
    }
    {
        RecommendedAppInfo app;
        app.name = "Tablacus Explorer";
        app.url = "https://tablacus.github.io/explorer.html";
        app.description = "标签页多窗口文件管理器，高效办公";
        app.iconEmoji = "📁";
        app.category = "实用工具";
        app.isFavorite = false;
        cat3.apps.append(app);
        allApps.append(app);
    }
    {
        RecommendedAppInfo app;
        app.name = "PDFgear";
        app.url = "https://www.pdfgear.com/";
        app.description = "免费的 PDF 工具，阅读、编辑、转换一站式";
        app.iconEmoji = "📄";
        app.category = "实用工具";
        app.isFavorite = false;
        cat3.apps.append(app);
        allApps.append(app);
    }
    categories.append(cat3);
    
    CategoryInfo cat4;
    cat4.name = "📄 办公与文档";
    cat4.iconEmoji = "📑";
    {
        RecommendedAppInfo app;
        app.name = "Typora";
        app.url = "https://typora.io/";
        app.description = "Markdown 编辑器，所见即所得，优雅简洁";
        app.iconEmoji = "✏️";
        app.category = "办公与文档";
        app.isFavorite = true;
        cat4.apps.append(app);
        allApps.append(app);
        favoriteApps.insert(app.name);
    }
    {
        RecommendedAppInfo app;
        app.name = "Notepad++";
        app.url = "https://notepad-plus-plus.org/";
        app.description = "文本编辑器，轻量高效，插件丰富";
        app.iconEmoji = "📝";
        app.category = "办公与文档";
        app.isFavorite = false;
        cat4.apps.append(app);
        allApps.append(app);
    }
    {
        RecommendedAppInfo app;
        app.name = "Pandoc";
        app.url = "https://pandoc.org/";
        app.description = "文档格式转换工具，支持 Markdown、Word、PDF 等";
        app.iconEmoji = "🔄";
        app.category = "办公与文档";
        app.isFavorite = false;
        cat4.apps.append(app);
        allApps.append(app);
    }
    {
        RecommendedAppInfo app;
        app.name = "方方格子Excel工具箱";
        app.url = "https://www.ffcell.com/";
        app.description = "Excel 增强工具，大幅提升表格处理效率";
        app.iconEmoji = "📊";
        app.category = "办公与文档";
        app.isFavorite = false;
        cat4.apps.append(app);
        allApps.append(app);
    }
    categories.append(cat4);
    
    CategoryInfo cat5;
    cat5.name = "🌐 网站与资源";
    cat5.iconEmoji = "🌍";
    {
        RecommendedAppInfo app;
        app.name = "实用网站汇总";
        app.url = "https://haiezan.github.io/page/collections/";
        app.description = "包含国家标准、图标下载、配色方案等实用网站";
        app.iconEmoji = "🔗";
        app.category = "网站与资源";
        app.isFavorite = false;
        cat5.apps.append(app);
        allApps.append(app);
    }
    {
        RecommendedAppInfo app;
        app.name = "百度云盘资源";
        app.url = "https://pan.baidu.com/s/1YukU_ZY3LpNztvpANTd-9w";
        app.description = "提取码：fvrs - 软件资源合集";
        app.iconEmoji = "☁️";
        app.category = "网站与资源";
        app.isFavorite = false;
        cat5.apps.append(app);
        allApps.append(app);
    }
    {
        RecommendedAppInfo app;
        app.name = "GitBook";
        app.url = "https://www.gitbook.com/";
        app.description = "电子书制作平台，技术文档首选";
        app.iconEmoji = "📚";
        app.category = "网站与资源";
        app.isFavorite = false;
        cat5.apps.append(app);
        allApps.append(app);
    }
    {
        RecommendedAppInfo app;
        app.name = "sm.ms 图床";
        app.url = "https://sm.ms/";
        app.description = "免费图床服务，稳定快速";
        app.iconEmoji = "🖼️";
        app.category = "网站与资源";
        app.isFavorite = false;
        cat5.apps.append(app);
        allApps.append(app);
    }
    categories.append(cat5);
    
    CategoryInfo cat6;
    cat6.name = "⚙️ Git与版本控制";
    cat6.iconEmoji = "🔀";
    {
        RecommendedAppInfo app;
        app.name = "GitHub Desktop";
        app.url = "https://desktop.github.com/";
        app.description = "GitHub 桌面客户端，图形化 Git 操作";
        app.iconEmoji = "🐙";
        app.category = "Git与版本控制";
        app.isFavorite = false;
        cat6.apps.append(app);
        allApps.append(app);
    }
    {
        RecommendedAppInfo app;
        app.name = "Git";
        app.url = "https://git-scm.com/";
        app.description = "版本控制系统，开发者必备";
        app.iconEmoji = "📦";
        app.category = "Git与版本控制";
        app.isFavorite = false;
        cat6.apps.append(app);
        allApps.append(app);
    }
    categories.append(cat6);
    
    CategoryInfo cat7;
    cat7.name = "🎵 生活与娱乐";
    cat7.iconEmoji = "🎮";
    {
        RecommendedAppInfo app;
        app.name = "Listen1";
        app.url = "https://listen1.github.io/listen1/";
        app.description = "音乐播放工具，聚合多平台音乐";
        app.iconEmoji = "🎵";
        app.category = "生活与娱乐";
        app.isFavorite = false;
        cat7.apps.append(app);
        allApps.append(app);
    }
    {
        RecommendedAppInfo app;
        app.name = "GitHub Games";
        app.url = "https://haiezan.github.io/page/collections/";
        app.description = "在线小游戏，休闲放松";
        app.iconEmoji = "🎮";
        app.category = "生活与娱乐";
        app.isFavorite = false;
        cat7.apps.append(app);
        allApps.append(app);
    }
    {
        RecommendedAppInfo app;
        app.name = "Agoda";
        app.url = "https://www.agoda.com/";
        app.description = "酒店预订平台，出行必备";
        app.iconEmoji = "🏨";
        app.category = "生活与娱乐";
        app.isFavorite = false;
        cat7.apps.append(app);
        allApps.append(app);
    }
    categories.append(cat7);
    
    CategoryInfo cat8;
    cat8.name = "🔌 Chrome 插件";
    cat8.iconEmoji = "🧩";
    {
        RecommendedAppInfo app;
        app.name = "Octotree";
        app.url = "https://www.octotree.io/";
        app.description = "GitHub 代码树插件，浏览代码更高效";
        app.iconEmoji = "🌳";
        app.category = "Chrome 插件";
        app.isFavorite = false;
        cat8.apps.append(app);
        allApps.append(app);
    }
    {
        RecommendedAppInfo app;
        app.name = "Gestures for Chrome";
        app.url = "https://chrome.google.com/webstore";
        app.description = "Chrome 浏览器手势插件，鼠标手势操作";
        app.iconEmoji = "✋";
        app.category = "Chrome 插件";
        app.isFavorite = false;
        cat8.apps.append(app);
        allApps.append(app);
    }
    {
        RecommendedAppInfo app;
        app.name = "Dribbble New Tab";
        app.url = "https://chrome.google.com/webstore";
        app.description = "新建标签页显示 Dribbble 作品，美化新标签页";
        app.iconEmoji = "🎨";
        app.category = "Chrome 插件";
        app.isFavorite = false;
        cat8.apps.append(app);
        allApps.append(app);
    }
    categories.append(cat8);
    
    CategoryInfo cat9;
    cat9.name = "🖥️ 远程协助";
    cat9.iconEmoji = "💻";
    {
        RecommendedAppInfo app;
        app.name = "ToDesk";
        app.url = "https://www.todesk.com/";
        app.description = "免费远程协助软件，流畅稳定";
        app.iconEmoji = "🔗";
        app.category = "远程协助";
        app.isFavorite = false;
        cat9.apps.append(app);
        allApps.append(app);
    }
    categories.append(cat9);
    
    CategoryInfo cat10;
    cat10.name = "📊 数据与图表";
    cat10.iconEmoji = "📈";
    {
        RecommendedAppInfo app;
        app.name = "Gnuplot";
        app.url = "http://www.gnuplot.info/";
        app.description = "动态曲线绘制工具，科学绘图首选";
        app.iconEmoji = "📉";
        app.category = "数据与图表";
        app.isFavorite = false;
        cat10.apps.append(app);
        allApps.append(app);
    }
    categories.append(cat10);
}

void RecommendedAppsWidget::openAppUrl(const QString &url)
{
    if (url.isEmpty()) {
        return;
    }
    QDesktopServices::openUrl(QUrl(url));
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
    QLabel *statsLabel = findChild<QLabel*>("statsLabel");
    if (statsLabel) {
        statsLabel->setText(QString("📊 共 %1 个应用，%2 个已收藏").arg(allApps.size()).arg(favoriteApps.size()));
    }
    
    applyFilter();
}

void RecommendedAppsWidget::applyFilter()
{
    QString searchText = searchEdit->text().toLower();
    bool showFavorites = showFavoritesCheck->isChecked();
    
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
    
    QGridLayout *gridLayout = new QGridLayout(allAppsWidget);
    gridLayout->setSpacing(15);
    gridLayout->setContentsMargins(10, 10, 10, 10);
    
    int row = 0, col = 0;
    for (const auto &app : allApps) {
        bool matchesSearch = searchText.isEmpty() || 
                            app.name.toLower().contains(searchText) || 
                            app.description.toLower().contains(searchText);
        bool matchesFavorite = !showFavorites || favoriteApps.contains(app.name);
        
        if (matchesSearch && matchesFavorite) {
            QWidget *card = createAppCard(app);
            gridLayout->addWidget(card, row, col);
            col++;
            if (col >= 2) {
                col = 0;
                row++;
            }
        }
    }
    
    gridLayout->setRowStretch(row + 1, 1);
}
