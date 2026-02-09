#include "recommendedappswidget.h"
#include <QApplication>
#include <QStyle>

RecommendedAppsWidget::RecommendedAppsWidget(QWidget *parent)
    : QWidget(parent)
{
    loadAppData();
    setupUI();
}

void RecommendedAppsWidget::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    QLabel *titleLabel = new QLabel("推荐应用", this);
    titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #1976d2; padding: 15px;");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);
    
    QLabel *subtitleLabel = new QLabel("精选实用工具和资源，提升您的工作效率", this);
    subtitleLabel->setStyleSheet("font-size: 14px; color: #666; padding: 5px;");
    subtitleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(subtitleLabel);
    
    tabWidget = new QTabWidget(this);
    tabWidget->setStyleSheet(
        "QTabWidget::pane { border: 1px solid #e0e0e0; border-radius: 5px; } "
        "QTabBar::tab { background: #f5f5f5; padding: 10px 20px; border-top-left-radius: 5px; border-top-right-radius: 5px; } "
        "QTabBar::tab:selected { background: #1976d2; color: white; } "
        "QTabBar::tab:hover { background: #42a5f5; color: white; }"
    );
    
    for (const auto &category : categories) {
        QWidget *categoryWidget = createCategoryWidget(category);
        tabWidget->addTab(categoryWidget, category.name);
    }
    
    mainLayout->addWidget(tabWidget);
}

QWidget* RecommendedAppsWidget::createCategoryWidget(const CategoryInfo &category)
{
    QWidget *widget = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(widget);
    
    QScrollArea *scrollArea = new QScrollArea(widget);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    
    QWidget *contentWidget = new QWidget();
    QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
    
    for (const auto &app : category.apps) {
        QGroupBox *appGroup = new QGroupBox(app.name, contentWidget);
        appGroup->setStyleSheet(
            "QGroupBox { font-weight: bold; font-size: 14px; border: 2px solid #e3f2fd; border-radius: 8px; margin-top: 10px; padding-top: 10px; } "
            "QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 5px; }"
        );
        
        QVBoxLayout *appLayout = new QVBoxLayout(appGroup);
        
        if (!app.description.isEmpty()) {
            QLabel *descLabel = new QLabel(app.description, appGroup);
            descLabel->setStyleSheet("color: #666; padding: 5px; font-size: 12px;");
            descLabel->setWordWrap(true);
            appLayout->addWidget(descLabel);
        }
        
        QPushButton *openButton = new QPushButton("打开链接", appGroup);
        openButton->setStyleSheet(
            "QPushButton { background-color: #1976d2; color: white; padding: 8px 20px; border-radius: 5px; font-weight: bold; } "
            "QPushButton:hover { background-color: #1565c0; }"
        );
        openButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_ComputerIcon));
        
        connect(openButton, &QPushButton::clicked, [this, app]() {
            openAppUrl(app.url);
        });
        
        QHBoxLayout *buttonLayout = new QHBoxLayout();
        buttonLayout->addStretch();
        buttonLayout->addWidget(openButton);
        appLayout->addLayout(buttonLayout);
        
        contentLayout->addWidget(appGroup);
    }
    
    contentLayout->addStretch();
    scrollArea->setWidget(contentWidget);
    layout->addWidget(scrollArea);
    
    return widget;
}

void RecommendedAppsWidget::loadAppData()
{
    CategoryInfo cat1;
    cat1.name = "📝 编程开发";
    {
        RecommendedAppInfo app;
        app.name = "Visual Studio Code";
        app.url = "https://code.visualstudio.com/";
        app.description = "强大的代码编辑器";
        cat1.apps.append(app);
    }
    {
        RecommendedAppInfo app;
        app.name = "小龙Dev-C++";
        app.url = "https://github.com/royqh1979/Dev-Cpp";
        app.description = "C/C++ 集成开发环境";
        cat1.apps.append(app);
    }
    {
        RecommendedAppInfo app;
        app.name = "HxD";
        app.url = "https://mh-nexus.de/en/hxd/";
        app.description = "专业的二进制文件编辑器";
        cat1.apps.append(app);
    }
    categories.append(cat1);
    
    CategoryInfo cat2;
    cat2.name = "🎨 设计与绘图";
    {
        RecommendedAppInfo app;
        app.name = "High-speed Charting Control";
        app.url = "https://github.com/iwancofossati/HightChart";
        app.description = "MFC 图形控件库";
        cat2.apps.append(app);
    }
    {
        RecommendedAppInfo app;
        app.name = "创客贴设计神器";
        app.url = "https://www.chuangkit.com/";
        app.description = "在线平面设计工具";
        cat2.apps.append(app);
    }
    {
        RecommendedAppInfo app;
        app.name = "Draw.io";
        app.url = "https://app.diagrams.net/";
        app.description = "免费流程图绘制工具";
        cat2.apps.append(app);
    }
    categories.append(cat2);
    
    CategoryInfo cat3;
    cat3.name = "🔧 实用工具";
    {
        RecommendedAppInfo app;
        app.name = "Everything";
        app.url = "https://www.voidtools.com/";
        app.description = "极速文件搜索工具";
        cat3.apps.append(app);
    }
    {
        RecommendedAppInfo app;
        app.name = "ShareX";
        app.url = "https://getsharex.com/";
        app.description = "强大的截图和录屏工具";
        cat3.apps.append(app);
    }
    {
        RecommendedAppInfo app;
        app.name = "Snipaste";
        app.url = "https://www.snipaste.com/";
        app.description = "快捷截图工具";
        cat3.apps.append(app);
    }
    {
        RecommendedAppInfo app;
        app.name = "SpaceSniffer";
        app.url = "http://www.uderzo.it/main_products/space_sniffer.html";
        app.description = "磁盘空间分析工具";
        cat3.apps.append(app);
    }
    {
        RecommendedAppInfo app;
        app.name = "Tablacus Explorer";
        app.url = "https://tablacus.github.io/explorer.html";
        app.description = "标签页多窗口文件管理器";
        cat3.apps.append(app);
    }
    {
        RecommendedAppInfo app;
        app.name = "PDFgear";
        app.url = "https://www.pdfgear.com/";
        app.description = "免费的 PDF 工具";
        cat3.apps.append(app);
    }
    categories.append(cat3);
    
    CategoryInfo cat4;
    cat4.name = "📄 办公与文档";
    {
        RecommendedAppInfo app;
        app.name = "Typora";
        app.url = "https://typora.io/";
        app.description = "Markdown 编辑器";
        cat4.apps.append(app);
    }
    {
        RecommendedAppInfo app;
        app.name = "Notepad++";
        app.url = "https://notepad-plus-plus.org/";
        app.description = "文本编辑器";
        cat4.apps.append(app);
    }
    {
        RecommendedAppInfo app;
        app.name = "Pandoc";
        app.url = "https://pandoc.org/";
        app.description = "文档格式转换工具";
        cat4.apps.append(app);
    }
    {
        RecommendedAppInfo app;
        app.name = "方方格子Excel工具箱";
        app.url = "https://www.ffcell.com/";
        app.description = "Excel 增强工具";
        cat4.apps.append(app);
    }
    categories.append(cat4);
    
    CategoryInfo cat5;
    cat5.name = "🌐 网站与资源";
    {
        RecommendedAppInfo app;
        app.name = "实用网站汇总";
        app.url = "https://haiezan.github.io/page/collections/";
        app.description = "包含国家标准、图标下载、配色方案等实用网站";
        cat5.apps.append(app);
    }
    {
        RecommendedAppInfo app;
        app.name = "百度云盘资源";
        app.url = "https://pan.baidu.com/s/1YukU_ZY3LpNztvpANTd-9w";
        app.description = "提取码：fvrs - 软件资源合集";
        cat5.apps.append(app);
    }
    {
        RecommendedAppInfo app;
        app.name = "GitBook";
        app.url = "https://www.gitbook.com/";
        app.description = "电子书制作平台";
        cat5.apps.append(app);
    }
    {
        RecommendedAppInfo app;
        app.name = "sm.ms 图床";
        app.url = "https://sm.ms/";
        app.description = "免费图床服务";
        cat5.apps.append(app);
    }
    categories.append(cat5);
    
    CategoryInfo cat6;
    cat6.name = "⚙️ Git与版本控制";
    {
        RecommendedAppInfo app;
        app.name = "GitHub Desktop";
        app.url = "https://desktop.github.com/";
        app.description = "GitHub 桌面客户端";
        cat6.apps.append(app);
    }
    {
        RecommendedAppInfo app;
        app.name = "Git";
        app.url = "https://git-scm.com/";
        app.description = "版本控制系统";
        cat6.apps.append(app);
    }
    categories.append(cat6);
    
    CategoryInfo cat7;
    cat7.name = "🎵 生活与娱乐";
    {
        RecommendedAppInfo app;
        app.name = "Listen1";
        app.url = "https://listen1.github.io/listen1/";
        app.description = "音乐播放工具，聚合多平台音乐";
        cat7.apps.append(app);
    }
    {
        RecommendedAppInfo app;
        app.name = "GitHub Games";
        app.url = "https://haiezan.github.io/page/collections/";
        app.description = "在线小游戏";
        cat7.apps.append(app);
    }
    {
        RecommendedAppInfo app;
        app.name = "Agoda";
        app.url = "https://www.agoda.com/";
        app.description = "酒店预订平台";
        cat7.apps.append(app);
    }
    categories.append(cat7);
    
    CategoryInfo cat8;
    cat8.name = "🔌 Chrome 插件";
    {
        RecommendedAppInfo app;
        app.name = "Octotree";
        app.url = "https://www.octotree.io/";
        app.description = "GitHub 代码树插件";
        cat8.apps.append(app);
    }
    {
        RecommendedAppInfo app;
        app.name = "Gestures for Chrome";
        app.url = "https://chrome.google.com/webstore";
        app.description = "Chrome 浏览器手势插件";
        cat8.apps.append(app);
    }
    {
        RecommendedAppInfo app;
        app.name = "Dribbble New Tab";
        app.url = "https://chrome.google.com/webstore";
        app.description = "新建标签页显示 Dribbble 作品";
        cat8.apps.append(app);
    }
    categories.append(cat8);
    
    CategoryInfo cat9;
    cat9.name = "🖥️ 远程协助";
    {
        RecommendedAppInfo app;
        app.name = "ToDesk";
        app.url = "https://www.todesk.com/";
        app.description = "免费远程协助软件";
        cat9.apps.append(app);
    }
    categories.append(cat9);
    
    CategoryInfo cat10;
    cat10.name = "📊 数据与图表";
    {
        RecommendedAppInfo app;
        app.name = "Gnuplot";
        app.url = "http://www.gnuplot.info/";
        app.description = "动态曲线绘制工具";
        cat10.apps.append(app);
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
