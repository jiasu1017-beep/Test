#include "appmanagerwidget.h"
#include "ui_appmanagerwidget.h"
#include "modules/core/database.h"
#include <QApplication>
#include <QStyle>
#include <QFileInfo>
#include <QStandardItem>
#include <QStandardPaths>
#include <QColor>
#include <QLinearGradient>
#include <QPainterPath>
#include <QPainter>
#include <QDesktopServices>
#include <QUrl>
#include <QListWidget>
#include <QListWidgetItem>
#include <QLineEdit>
#include <QCheckBox>
#include <QTimer>
#include <QMetaType>
#include <algorithm>
#include <windows.h>
#include <shellapi.h>

void AppIconDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    
    QRect rect = option.rect;
    
    if (option.state & QStyle::State_Selected) {
        QLinearGradient gradient(rect.topLeft(), rect.bottomLeft());
        gradient.setColorAt(0, QColor(66, 165, 245, 100));
        gradient.setColorAt(1, QColor(30, 136, 229, 100));
        painter->fillRect(rect, gradient);
        
        painter->setPen(QColor(33, 150, 243));
        painter->setBrush(Qt::NoBrush);
        painter->drawRoundedRect(rect.adjusted(2, 2, -2, -2), 8, 8);
    } else if (option.state & QStyle::State_MouseOver) {
        QLinearGradient hoverGradient(rect.topLeft(), rect.bottomLeft());
        hoverGradient.setColorAt(0, QColor(66, 165, 245, 40));
        hoverGradient.setColorAt(1, QColor(30, 136, 229, 25));
        painter->fillRect(rect, hoverGradient);
        
        painter->setPen(QColor(66, 165, 245, 80));
        painter->setBrush(Qt::NoBrush);
        painter->drawRoundedRect(rect.adjusted(2, 2, -2, -2), 8, 8);
    }
    
    int iconSize = 72;
    int textHeight = 24;
    int padding = 8;
    
    int totalHeight = iconSize + textHeight + padding * 3;
    int startY = rect.top() + (rect.height() - totalHeight) / 2;
    
    QRect iconRect(rect.left() + (rect.width() - iconSize) / 2, 
                   startY + padding, 
                   iconSize, iconSize);
    
    QPainterPath path;
    path.addRoundedRect(iconRect, 16, 16);
    
    QLinearGradient iconGradient(iconRect.topLeft(), iconRect.bottomRight());
    iconGradient.setColorAt(0, QColor(255, 255, 255));
    iconGradient.setColorAt(1, QColor(245, 245, 245));
    painter->fillPath(path, iconGradient);
    
    painter->setPen(QColor(220, 220, 220));
    painter->drawPath(path);
    
    QIcon icon = index.data(Qt::DecorationRole).value<QIcon>();
    QPixmap pixmap = icon.pixmap(QSize(56, 56));
    QRect pixmapRect(iconRect.left() + (iconSize - 56) / 2,
                    iconRect.top() + (iconSize - 56) / 2,
                    56, 56);
    painter->drawPixmap(pixmapRect, pixmap);
    
    QRect textRect(rect.left() + padding,
                   iconRect.bottom() + padding,
                   rect.width() - padding * 2,
                   textHeight);
    
    QString text = index.data(Qt::DisplayRole).toString();
    QFont font = painter->font();
    font.setPixelSize(12);
    font.setBold(false);
    painter->setFont(font);
    painter->setPen(QColor(51, 51, 51));
    painter->drawText(textRect, Qt::AlignCenter | Qt::TextWordWrap, text);
    
    painter->restore();
}

QSize AppIconDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);
    return QSize(110, 130);
}

void AppListDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    
    QRect rect = option.rect;
    
    if (option.state & QStyle::State_Selected) {
        QLinearGradient gradient(rect.topLeft(), rect.bottomRight());
        gradient.setColorAt(0, QColor(66, 165, 245, 60));
        gradient.setColorAt(1, QColor(30, 136, 229, 40));
        painter->fillRect(rect, gradient);
    } else if (option.state & QStyle::State_MouseOver) {
        QLinearGradient hoverGradient(rect.topLeft(), rect.bottomRight());
        hoverGradient.setColorAt(0, QColor(66, 165, 245, 25));
        hoverGradient.setColorAt(1, QColor(30, 136, 229, 15));
        painter->fillRect(rect, hoverGradient);
    }
    
    AppInfo app = index.data(Qt::UserRole + 1).value<AppInfo>();
    
    QIcon icon = index.data(Qt::DecorationRole).value<QIcon>();
    if (!icon.isNull()) {
        QPixmap pixmap = icon.pixmap(QSize(32, 32));
        int iconY = rect.top() + (rect.height() - 32) / 2;
        painter->drawPixmap(QRect(rect.left() + 10, iconY, 32, 32), pixmap);
    }
    
    QString typeText;
    switch (app.type) {
        case AppType_Executable: typeText = "应用程序"; break;
        case AppType_Website: typeText = "网站"; break;
        case AppType_Folder: typeText = "文件夹"; break;
        case AppType_Document: typeText = "文档"; break;
        case AppType_RemoteDesktop: typeText = "远程桌面"; break;
        default: typeText = "未知"; break;
    }
    
    QString infoText = QString("%1 | %2").arg(typeText, app.path);
    if (app.useCount > 0) {
        infoText += QString(" | 使用次数: %1").arg(app.useCount);
    }
    
    QString nameText = index.data(Qt::DisplayRole).toString();
    
    QFont nameFont = painter->font();
    nameFont.setPixelSize(13);
    nameFont.setBold(true);
    painter->setFont(nameFont);
    painter->setPen(QColor(51, 51, 51));
    
    QRect nameRect(rect.left() + 52, rect.top() + 8, rect.width() - 140, 20);
    painter->drawText(nameRect, Qt::AlignLeft | Qt::AlignVCenter, nameText);
    
    QFont infoFont = painter->font();
    infoFont.setPixelSize(10);
    infoFont.setBold(false);
    painter->setFont(infoFont);
    painter->setPen(QColor(149, 165, 166));
    
    QRect infoRect(rect.left() + 52, rect.top() + 28, rect.width() - 140, 18);
    QString elidedInfo = painter->fontMetrics().elidedText(infoText, Qt::ElideMiddle, infoRect.width());
    painter->drawText(infoRect, Qt::AlignLeft | Qt::AlignVCenter, elidedInfo);
    
    if (app.isFavorite) {
        QRect favRect(rect.right() - 28, rect.top() + (rect.height() - 14) / 2, 14, 14);
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor(241, 196, 15));
        
        QPainterPath starPath;
        QPointF center = favRect.center();
        qreal outerRadius = 7;
        qreal innerRadius = 3;
        const int points = 5;
        
        for (int i = 0; i < points * 2; ++i) {
            qreal radius = (i % 2 == 0) ? outerRadius : innerRadius;
            qreal angle = i * M_PI / points - M_PI / 2;
            qreal x = center.x() + radius * qCos(angle);
            qreal y = center.y() + radius * qSin(angle);
            if (i == 0) {
                starPath.moveTo(x, y);
            } else {
                starPath.lineTo(x, y);
            }
        }
        starPath.closeSubpath();
        painter->drawPath(starPath);
    }
    
    painter->restore();
}

QSize AppListDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);
    return QSize(300, 56);
}

AppManagerWidget::AppManagerWidget(Database *db, QWidget *parent)
    : QWidget(parent), db(db), ui(new Ui::AppManagerWidget), currentSortMode(AppSortMode_Recent)
{
    ui->setupUi(this);

    qRegisterMetaType<AppInfo>("AppInfo");

    appManager = new ApplicationManager(db, this);
    connect(appManager, &ApplicationManager::appLaunched, this, [this](const AppInfo &app) {
        refreshAppList();
    });

    iconDelegate = new AppIconDelegate(this);
    listDelegate = new AppListDelegate(this);
    appModel = new QStandardItemModel(this);
    ui->appListView->setModel(appModel);
    ui->appListView->setItemDelegate(iconDelegate);

    sortModeMenu = new QMenu(this);
    QAction *actionRecent = new QAction("最近使用", this);
    actionRecent->setData(AppSortMode_Recent);
    QAction *actionUsageCount = new QAction("使用次数", this);
    actionUsageCount->setData(AppSortMode_UsageCount);
    QAction *actionManual = new QAction("默认排序", this);
    actionManual->setData(AppSortMode_Manual);
    sortModeMenu->addAction(actionRecent);
    sortModeMenu->addAction(actionUsageCount);
    sortModeMenu->addAction(actionManual);
    connect(sortModeMenu, &QMenu::triggered, this, &AppManagerWidget::onSortModeChanged);

    ui->sortModeButton->setMenu(sortModeMenu);
    ui->sortModeButton->setText("最近使用");

    setupUI();
    refreshAppList();
}

AppManagerWidget::~AppManagerWidget()
{
    delete ui;
}

void AppManagerWidget::setupUI()
{
    ui->addButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_FileDialogNewFolder));
    ui->addButton->setStyleSheet("QPushButton { background-color: #4caf50; color: white; padding: 8px 16px; border-radius: 5px; font-weight: bold; } "
                                "QPushButton:hover { background-color: #43a047; }");
    connect(ui->addButton, &QPushButton::clicked, this, &AppManagerWidget::onAddApp);
    
    ui->deleteButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_TrashIcon));
    ui->deleteButton->setStyleSheet("QPushButton { background-color: #f44336; color: white; padding: 8px 16px; border-radius: 5px; font-weight: bold; } "
                                  "QPushButton:hover { background-color: #d32f2f; }");
    connect(ui->deleteButton, &QPushButton::clicked, this, &AppManagerWidget::onDeleteApp);
    
    ui->launchButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_MediaPlay));
    ui->launchButton->setStyleSheet("QPushButton { background-color: #2196f3; color: white; padding: 8px 16px; border-radius: 5px; font-weight: bold; } "
                                  "QPushButton:hover { background-color: #1976d2; }");
    connect(ui->launchButton, &QPushButton::clicked, this, &AppManagerWidget::onLaunchApp);
    
    ui->refreshButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_BrowserReload));
    ui->refreshButton->setStyleSheet("QPushButton { background-color: #ff9800; color: white; padding: 8px 16px; border-radius: 5px; font-weight: bold; } "
                                   "QPushButton:hover { background-color: #fb8c00; }");
    connect(ui->refreshButton, &QPushButton::clicked, this, &AppManagerWidget::refreshAppList);
    
    ui->moveUpButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_ArrowUp));
    ui->moveUpButton->setStyleSheet("QPushButton { background-color: #9c27b0; color: white; padding: 8px 16px; border-radius: 5px; font-weight: bold; } "
                                  "QPushButton:hover { background-color: #7b1fa2; }");
    connect(ui->moveUpButton, &QPushButton::clicked, this, &AppManagerWidget::onMoveUp);
    
    ui->moveDownButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_ArrowDown));
    ui->moveDownButton->setStyleSheet("QPushButton { background-color: #9c27b0; color: white; padding: 8px 16px; border-radius: 5px; font-weight: bold; } "
                                   "QPushButton:hover { background-color: #7b1fa2; }");
    connect(ui->moveDownButton, &QPushButton::clicked, this, &AppManagerWidget::onMoveDown);
    
    ui->initButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogResetButton));
    ui->initButton->setStyleSheet("QPushButton { background-color: #e91e63; color: white; padding: 8px 16px; border-radius: 5px; font-weight: bold; } "
                                   "QPushButton:hover { background-color: #c2185b; }");
    connect(ui->initButton, &QPushButton::clicked, this, &AppManagerWidget::onInitApps);

    // 视图模式按钮
    QMenu *viewModeMenu = new QMenu(this);
    QAction *actionIconView = new QAction("图标视图", this);
    actionIconView->setData("icon");
    QAction *actionListView = new QAction("列表视图", this);
    actionListView->setData("list");
    viewModeMenu->addAction(actionIconView);
    viewModeMenu->addAction(actionListView);
    connect(viewModeMenu, &QMenu::triggered, this, &AppManagerWidget::onViewModeChanged);

    ui->viewModeButton->setMenu(viewModeMenu);
    ui->viewModeButton->setText("图标视图");

    // 设置视图模式和排序模式按钮的统一样式（参考添加应用按钮风格）
    QString toolButtonStyle = R"(
        QToolButton {
            background-color: #9c27b0;
            color: white;
            padding: 8px 16px;
            border-radius: 5px;
            font-weight: bold;
        }
        QToolButton:hover {
            background-color: #7b1fa2;
        }
        QToolButton:pressed {
            background-color: #6a1b9a;
        }
    )";
    QString menuStyle = R"(
        QMenu {
            background-color: #ffffff;
            border: 1px solid #e0e0e0;
            border-radius: 6px;
            padding: 4px;
        }
        QMenu::item {
            padding: 8px 24px;
            border-radius: 4px;
            color: #333;
        }
        QMenu::item:selected {
            background-color: #e3f2fd;
            color: #1976d2;
        }
    )";

    ui->viewModeButton->setStyleSheet(toolButtonStyle);
    ui->sortModeButton->setStyleSheet(toolButtonStyle);
    viewModeMenu->setStyleSheet(menuStyle);
    sortModeMenu->setStyleSheet(menuStyle);

    ui->categoryComboBox->setStyleSheet(
        "QComboBox {"
        "   padding: 6px 12px;"
        "   border: 1px solid #dcdcdc;"
        "   border-radius: 6px;"
        "   background-color: white;"
        "   color: #333;"
        "   font-size: 13px;"
        "}"
        "QComboBox:hover {"
        "   border-color: #1976d2;"
        "}"
        "QComboBox:focus {"
        "   border-color: #1976d2;"
        "}"
        "QComboBox::drop-down {"
        "   border: none;"
        "   width: 24px;"
        "}"
        "QComboBox::down-arrow {"
        "   image: none;"
        "   border-left: 4px solid transparent;"
        "   border-right: 4px solid transparent;"
        "   border-top: 6px solid #666;"
        "   margin-right: 8px;"
        "}"
        "QComboBox QAbstractItemView {"
        "   border: 1px solid #e0e0e0;"
        "   border-radius: 6px;"
        "   background-color: white;"
        "   selection-background-color: #e3f2fd;"
        "   selection-color: #1976d2;"
        "   padding: 4px;"
        "}"
        "QComboBox QAbstractItemView::item {"
        "   padding: 8px 12px;"
        "   border-radius: 4px;"
        "}"
        "QComboBox QAbstractItemView::item:selected {"
        "   background-color: #e3f2fd;"
        "   color: #1976d2;"
        "}"
        "QComboBox QAbstractItemView::item:hover {"
        "   background-color: #f5f5f5;"
        "}"
    );
    
    QListView *listView = new QListView();
    listView->setSpacing(2);
    ui->categoryComboBox->setView(listView);
    
    connect(ui->categoryComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
        QStandardItemModel *model = qobject_cast<QStandardItemModel*>(ui->categoryComboBox->model());
        if (!model) return;
        
        QStandardItem *item = model->item(index);
        if (!item) return;
        
        QString data = item->data(Qt::UserRole).toString();
        
        if (data == "all") {
            currentCategory = "";
            currentType = -1;
        } else if (data == "type_Executable") {
            currentCategory = "";
            currentType = AppType_Executable;
        } else if (data == "type_Website") {
            currentCategory = "";
            currentType = AppType_Website;
        } else if (data == "type_Folder") {
            currentCategory = "";
            currentType = AppType_Folder;
        } else if (data == "type_Document") {
            currentCategory = "";
            currentType = AppType_Document;
        } else if (data == "type_RemoteDesktop") {
            currentCategory = "";
            currentType = AppType_RemoteDesktop;
        } else if (data.startsWith("category_")) {
            currentCategory = data.mid(9);
            currentType = -1;
        }
        
        filterAppsByCategory(currentCategory);
    });
    
    loadCategories();
    
    connect(ui->appListView, &QListView::customContextMenuRequested, this, &AppManagerWidget::onShowContextMenu);
    connect(ui->appListView, &QListView::doubleClicked, this, &AppManagerWidget::onAppItemDoubleClicked);
}

void AppManagerWidget::onIconViewMode()
{
    ui->appListView->setViewMode(QListView::IconMode);
    ui->appListView->setItemDelegate(iconDelegate);
    ui->appListView->setIconSize(QSize(72, 72));
    ui->appListView->setSpacing(15);
}

void AppManagerWidget::onListViewMode()
{
    ui->appListView->setViewMode(QListView::ListMode);
    ui->appListView->setItemDelegate(listDelegate);
    ui->appListView->setIconSize(QSize(32, 32));
    ui->appListView->setSpacing(5);
}

void AppManagerWidget::onViewModeChanged(QAction *action)
{
    if (!action) return;

    QString mode = action->data().toString();

    if (mode == "icon") {
        ui->appListView->setViewMode(QListView::IconMode);
        ui->appListView->setItemDelegate(iconDelegate);
        ui->appListView->setIconSize(QSize(72, 72));
        ui->appListView->setSpacing(15);
        ui->viewModeButton->setText("图标视图");
    } else if (mode == "list") {
        ui->appListView->setViewMode(QListView::ListMode);
        ui->appListView->setItemDelegate(listDelegate);
        ui->appListView->setIconSize(QSize(32, 32));
        ui->appListView->setSpacing(5);
        ui->viewModeButton->setText("列表视图");
    }
}

void AppManagerWidget::onMoveUp()
{
    QModelIndex current = ui->appListView->currentIndex();
    if (!current.isValid() || current.row() <= 0) return;

    int row = current.row();

    // 切换到手动排序模式
    if (currentSortMode != AppSortMode_Manual) {
        currentSortMode = AppSortMode_Manual;
        ui->sortModeButton->setText("默认排序");
    }

    // 获取按 sortOrder 排序的应用列表
    QList<AppInfo> apps = db->getAllApps();
    std::sort(apps.begin(), apps.end(), [](const AppInfo &a, const AppInfo &b) {
        return a.sortOrder < b.sortOrder;
    });

    if (row > 0 && row < apps.size()) {
        AppInfo currentApp = apps[row];
        AppInfo prevApp = apps[row - 1];

        int tempOrder = currentApp.sortOrder;
        currentApp.sortOrder = prevApp.sortOrder;
        prevApp.sortOrder = tempOrder;

        // 标记为固定
        currentApp.isPinned = true;
        prevApp.isPinned = true;

        db->updateApp(currentApp);
        db->updateApp(prevApp);

        refreshAppList();

        QModelIndex newIndex = appModel->index(row - 1, 0);
        ui->appListView->setCurrentIndex(newIndex);
    }
}

void AppManagerWidget::onMoveDown()
{
    QModelIndex current = ui->appListView->currentIndex();
    if (!current.isValid()) return;

    int row = current.row();

    // 切换到手动排序模式
    if (currentSortMode != AppSortMode_Manual) {
        currentSortMode = AppSortMode_Manual;
        ui->sortModeButton->setText("默认排序");
    }

    // 获取按 sortOrder 排序的应用列表
    QList<AppInfo> apps = db->getAllApps();
    std::sort(apps.begin(), apps.end(), [](const AppInfo &a, const AppInfo &b) {
        return a.sortOrder < b.sortOrder;
    });

    if (row >= 0 && row < apps.size() - 1) {
        AppInfo currentApp = apps[row];
        AppInfo nextApp = apps[row + 1];

        int tempOrder = currentApp.sortOrder;
        currentApp.sortOrder = nextApp.sortOrder;
        nextApp.sortOrder = tempOrder;

        // 标记为固定
        currentApp.isPinned = true;
        nextApp.isPinned = true;

        db->updateApp(currentApp);
        db->updateApp(nextApp);

        refreshAppList();

        QModelIndex newIndex = appModel->index(row + 1, 0);
        ui->appListView->setCurrentIndex(newIndex);
    }
}

void AppManagerWidget::saveAppOrder()
{
    for (int i = 0; i < appModel->rowCount(); ++i) {
        QStandardItem *item = appModel->item(i);
        int appId = item->data(Qt::UserRole).toInt();
        AppInfo app = db->getAppById(appId);
        app.sortOrder = i;
        db->updateApp(app);
    }
}

void AppManagerWidget::refreshAppList()
{
    appModel->clear();

    allApps = db->getAllApps();

    // 根据当前排序模式排序
    switch (currentSortMode) {
    case AppSortMode_Recent:
        std::sort(allApps.begin(), allApps.end(), [](const AppInfo &a, const AppInfo &b) {
            // isPinned 的应用排在前面
            if (a.isPinned != b.isPinned) {
                return a.isPinned > b.isPinned;
            }
            // 都是 pinned 或都不是 pinned 时，按 lastUsedTime 排序
            if (a.isPinned) {
                return a.sortOrder < b.sortOrder;
            }
            // 未使用的排在后面
            if (!a.lastUsedTime.isValid() && !b.lastUsedTime.isValid()) {
                return false;
            }
            if (!a.lastUsedTime.isValid()) return false;
            if (!b.lastUsedTime.isValid()) return true;
            return a.lastUsedTime > b.lastUsedTime;
        });
        break;
    case AppSortMode_UsageCount:
        std::sort(allApps.begin(), allApps.end(), [](const AppInfo &a, const AppInfo &b) {
            // isPinned 的应用排在前面
            if (a.isPinned != b.isPinned) {
                return a.isPinned > b.isPinned;
            }
            // 都是 pinned 或都不是 pinned 时，按 useCount 排序
            if (a.isPinned) {
                return a.sortOrder < b.sortOrder;
            }
            // 未使用过的排在后面
            if (a.useCount == 0 && b.useCount == 0) {
                return false;
            }
            if (a.useCount == 0) return false;
            if (b.useCount == 0) return true;
            return a.useCount > b.useCount;
        });
        break;
    case AppSortMode_Manual:
        std::sort(allApps.begin(), allApps.end(), [](const AppInfo &a, const AppInfo &b) {
            return a.sortOrder < b.sortOrder;
        });
        break;
    }

    for (const AppInfo &app : allApps) {
        QStandardItem *item = new QStandardItem(app.name);
        item->setData(app.id, Qt::UserRole);
        item->setData(QVariant::fromValue(app), Qt::UserRole + 1);
        item->setIcon(getAppIcon(app));

        if (app.isFavorite) {
            item->setBackground(QColor(255, 249, 196));
        }

        appModel->appendRow(item);
    }

    filterAppsByCategory(currentCategory);
}

void AppManagerWidget::loadCategories()
{
    QStandardItemModel *model = new QStandardItemModel(this);
    
    QStandardItem *allItem = new QStandardItem("全部");
    allItem->setData("all", Qt::UserRole);
    allItem->setForeground(QColor("#333333"));
    model->appendRow(allItem);
    
    QStandardItem *typeHeader = new QStandardItem("按类型筛选");
    typeHeader->setData("header", Qt::UserRole);
    typeHeader->setForeground(QColor("#1976d2"));
    typeHeader->setFont(QFont("Microsoft YaHei", 10, QFont::Bold));
    typeHeader->setEnabled(false);
    model->appendRow(typeHeader);
    
    QStringList typeItems = {"应用程序", "网站", "文件夹", "文档", "远程桌面"};
    QStringList typeKeys = {"Executable", "Website", "Folder", "Document", "RemoteDesktop"};
    for (int i = 0; i < typeItems.size(); ++i) {
        QStandardItem *item = new QStandardItem("  " + typeItems[i]);
        item->setData("type_" + typeKeys[i], Qt::UserRole);
        item->setForeground(QColor("#555555"));
        model->appendRow(item);
    }
    
    QList<Category> categories = Database::getBuiltinCategories();
    if (!categories.isEmpty()) {
        QStandardItem *catHeader = new QStandardItem("按分类筛选");
        catHeader->setData("header", Qt::UserRole);
        catHeader->setForeground(QColor("#1976d2"));
        catHeader->setFont(QFont("Microsoft YaHei", 10, QFont::Bold));
        catHeader->setEnabled(false);
        model->appendRow(catHeader);
        
        for (const Category &cat : categories) {
            QStandardItem *item = new QStandardItem("  " + cat.name);
            item->setData("category_" + cat.name, Qt::UserRole);
            item->setForeground(QColor("#555555"));
            model->appendRow(item);
        }
    }
    
    ui->categoryComboBox->setModel(model);
    
    currentCategory = "";
    currentType = -1;
}

void AppManagerWidget::filterAppsByCategory(const QString &category)
{
    appModel->clear();
    
    for (const AppInfo &app : allApps) {
        bool matchCategory = category.isEmpty() || category == "全部" || 
                           app.category == category.trimmed();
        
        bool matchType = true;
        if (currentType >= 0) {
            matchType = (app.type == currentType);
        }
        
        if (matchCategory && matchType) {
            QStandardItem *item = new QStandardItem(app.name);
            item->setData(app.id, Qt::UserRole);
            item->setData(QVariant::fromValue(app), Qt::UserRole + 1);
            item->setIcon(getAppIcon(app));
            
            if (app.isFavorite) {
                item->setBackground(QColor(255, 249, 196));
            }
            
            appModel->appendRow(item);
        }
    }
}

QIcon AppManagerWidget::getAppIcon(const AppInfo &app)
{
    return ApplicationManager::getAppIcon(app);
}

void AppManagerWidget::onAddApp()
{
    QDialog dialog(this);
    dialog.setWindowTitle("添加应用");
    dialog.setMinimumWidth(450);
    dialog.setStyleSheet("QDialog { background-color: #fafbfc; }");

    QVBoxLayout *layout = new QVBoxLayout(&dialog);
    layout->setSpacing(16);
    layout->setContentsMargins(24, 24, 24, 24);

    QLabel *label = new QLabel("请选择要添加的内容类型：", &dialog);
    label->setStyleSheet("font-size: 14px; color: #2d3436; font-weight: 500;");
    layout->addWidget(label);

    QPushButton *btnExecutable = new QPushButton("应用程序 (.exe/.bat/.cmd)", &dialog);
    btnExecutable->setStyleSheet(
        "QPushButton { background-color: #00b894; color: white; padding: 14px; border-radius: 8px; font-weight: bold; font-size: 13px; } "
        "QPushButton:hover { background-color: #00a085; }"
    );
    layout->addWidget(btnExecutable);

    QPushButton *btnWebsite = new QPushButton("网站链接", &dialog);
    btnWebsite->setStyleSheet(
        "QPushButton { background-color: #0984e3; color: white; padding: 14px; border-radius: 8px; font-weight: bold; font-size: 13px; } "
        "QPushButton:hover { background-color: #0770c4; }"
    );
    layout->addWidget(btnWebsite);

    QPushButton *btnFolder = new QPushButton("文件夹", &dialog);
    btnFolder->setStyleSheet(
        "QPushButton { background-color: #fd79a8; color: white; padding: 14px; border-radius: 8px; font-weight: bold; font-size: 13px; } "
        "QPushButton:hover { background-color: #f06795; }"
    );
    layout->addWidget(btnFolder);

    QPushButton *btnDocument = new QPushButton("文档文件 (.docx/.pdf/.txt等)", &dialog);
    btnDocument->setStyleSheet(
        "QPushButton { background-color: #6c5ce7; color: white; padding: 14px; border-radius: 8px; font-weight: bold; font-size: 13px; } "
        "QPushButton:hover { background-color: #5f4fd6; }"
    );
    layout->addWidget(btnDocument);

    QLabel *label2 = new QLabel("批量导入：", &dialog);
    label2->setStyleSheet("font-size: 13px; color: #636e72; font-weight: 500; margin-top: 8px;");
    layout->addWidget(label2);

    QPushButton *btnRegistry = new QPushButton("从注册表导入应用", &dialog);
    btnRegistry->setStyleSheet(
        "QPushButton { background-color: #e17055; color: white; padding: 14px; border-radius: 8px; font-weight: bold; font-size: 13px; } "
        "QPushButton:hover { background-color: #d35d4c; }"
    );
    layout->addWidget(btnRegistry);

    QPushButton *btnBookmarks = new QPushButton("从浏览器收藏夹导入", &dialog);
    btnBookmarks->setStyleSheet(
        "QPushButton { background-color: #00cec9; color: white; padding: 14px; border-radius: 8px; font-weight: bold; font-size: 13px; } "
        "QPushButton:hover { background-color: #00b5b0; }"
    );
    layout->addWidget(btnBookmarks);

    QPushButton *btnRunning = new QPushButton("导入当前运行的应用", &dialog);
    btnRunning->setStyleSheet(
        "QPushButton { background-color: #fdcb6e; color: white; padding: 14px; border-radius: 8px; font-weight: bold; font-size: 13px; } "
        "QPushButton:hover { background-color: #f0b960; }"
    );
    layout->addWidget(btnRunning);

    QPushButton *btnCancel = new QPushButton("取消", &dialog);
    btnCancel->setStyleSheet(
        "QPushButton { background-color: #b2bec3; color: white; padding: 12px 28px; border-radius: 8px; font-weight: bold; font-size: 13px; } "
        "QPushButton:hover { background-color: #a0aab0; }"
    );
    layout->addWidget(btnCancel, 0, Qt::AlignCenter);

    bool *accepted = new bool(false);

    connect(btnExecutable, &QPushButton::clicked, [&]() {
        *accepted = true;
        addExecutableApp();
        dialog.accept();
    });

    connect(btnWebsite, &QPushButton::clicked, [&]() {
        *accepted = true;
        addWebsiteApp();
        dialog.accept();
    });

    connect(btnFolder, &QPushButton::clicked, [&]() {
        *accepted = true;
        addFolderApp();
        dialog.accept();
    });

    connect(btnDocument, &QPushButton::clicked, [&]() {
        *accepted = true;
        addDocumentApp();
        dialog.accept();
    });

    connect(btnRegistry, &QPushButton::clicked, [&]() {
        *accepted = true;
        addAppsFromRegistry();
        dialog.accept();
    });

    connect(btnBookmarks, &QPushButton::clicked, [&]() {
        *accepted = true;
        addBookmarksFromBrowsers();
        dialog.accept();
    });

    connect(btnRunning, &QPushButton::clicked, [&]() {
        *accepted = true;
        addRunningApps();
        dialog.accept();
    });

    connect(btnCancel, &QPushButton::clicked, &dialog, &QDialog::reject);

    dialog.exec();
    delete accepted;
}

void AppManagerWidget::addExecutableApp()
{
    QString filePath = QFileDialog::getOpenFileName(this, "选择应用程序", "", 
                                                    "可执行文件 (*.exe *.bat *.cmd);;所有文件 (*.*)");
    if (filePath.isEmpty()) return;
    
    QFileInfo fileInfo(filePath);
    QString appName = fileInfo.completeBaseName();
    
    AppInfo app;
    app.name = appName;
    app.path = filePath;
    app.arguments = "";
    app.iconPath = "";
    app.category = "应用程序";
    app.useCount = 0;
    app.isFavorite = false;
    app.type = AppType_Executable;
    app.remoteDesktopId = -1;
    
    app.sortOrder = db->getMaxSortOrder() + 1;
    
    db->addApp(app);
    refreshAppList();
}

void AppManagerWidget::addWebsiteApp()
{
    bool ok;
    QString url = QInputDialog::getText(this, "添加网站链接", "请输入网站URL（例如：https://www.baidu.com）：", 
                                         QLineEdit::Normal, "https://", &ok);
    if (!ok || url.trimmed().isEmpty()) return;
    
    QString trimmedUrl = url.trimmed();
    if (!trimmedUrl.startsWith("http://") && !trimmedUrl.startsWith("https://")) {
        trimmedUrl = "https://" + trimmedUrl;
    }
    
    bool ok2;
    QString name = QInputDialog::getText(this, "网站名称", "请输入网站显示名称：", 
                                          QLineEdit::Normal, trimmedUrl, &ok2);
    if (!ok2 || name.trimmed().isEmpty()) {
        name = trimmedUrl;
    }
    
    AppInfo app;
    app.name = name.trimmed();
    app.path = trimmedUrl;
    app.arguments = "";
    app.iconPath = "";
    app.category = "网站";
    app.useCount = 0;
    app.isFavorite = false;
    app.type = AppType_Website;
    app.remoteDesktopId = -1;
    
    app.sortOrder = db->getMaxSortOrder() + 1;
    
    db->addApp(app);
    refreshAppList();
}

void AppManagerWidget::addFolderApp()
{
    QString folderPath = QFileDialog::getExistingDirectory(this, "选择文件夹", "", 
                                                          QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (folderPath.isEmpty()) return;
    
    QFileInfo fileInfo(folderPath);
    QString appName = fileInfo.fileName();
    if (appName.isEmpty()) appName = folderPath;
    
    AppInfo app;
    app.name = appName;
    app.path = folderPath;
    app.arguments = "";
    app.iconPath = "";
    app.category = "文件夹";
    app.useCount = 0;
    app.isFavorite = false;
    app.type = AppType_Folder;
    app.remoteDesktopId = -1;
    
    app.sortOrder = db->getMaxSortOrder() + 1;
    
    db->addApp(app);
    refreshAppList();
}

void AppManagerWidget::addDocumentApp()
{
    QString filePath = QFileDialog::getOpenFileName(this, "选择文档文件", "", 
                                                    "文档文件 (*.docx *.doc *.pdf *.txt *.xlsx *.xls *.pptx *.ppt);;所有文件 (*.*)");
    if (filePath.isEmpty()) return;
    
    QFileInfo fileInfo(filePath);
    QString appName = fileInfo.completeBaseName();
    
    AppInfo app;
    app.name = appName;
    app.path = filePath;
    app.arguments = "";
    app.iconPath = "";
    app.category = "文档";
    app.useCount = 0;
    app.isFavorite = false;
    app.type = AppType_Document;
    app.remoteDesktopId = -1;
    
    app.sortOrder = db->getMaxSortOrder() + 1;

    db->addApp(app);
    refreshAppList();
}

void AppManagerWidget::addAppsFromRegistry()
{
    QList<AppInfo> apps = ApplicationManager::getAppsFromRegistry();

    if (apps.isEmpty()) {
        QMessageBox::information(this, "提示", "未从注册表中找到任何应用程序");
        return;
    }

    BatchImportDialog dialog("从注册表导入应用", "找到", apps, db, this);
    if (dialog.exec() == QDialog::Accepted) {
        refreshAppList();
    }
}

void AppManagerWidget::addBookmarksFromBrowsers()
{
    QList<AppInfo> bookmarks = ApplicationManager::getBookmarksFromBrowsers();

    if (bookmarks.isEmpty()) {
        QMessageBox::information(this, "提示", "未从浏览器收藏夹中找到任何网址\n支持的浏览器：Chrome、Edge、Opera");
        return;
    }

    BatchImportDialog dialog("从浏览器收藏夹导入", "找到", bookmarks, db, this);
    if (dialog.exec() == QDialog::Accepted) {
        refreshAppList();
    }
}

void AppManagerWidget::addRunningApps()
{
    QList<AppInfo> runningApps = ApplicationManager::getRunningApps();

    if (runningApps.isEmpty()) {
        QMessageBox::information(this, "提示", "未找到任何正在运行的用户应用程序");
        return;
    }

    BatchImportDialog dialog("导入当前运行的应用", "找到", runningApps, db, this);
    if (dialog.exec() == QDialog::Accepted) {
        refreshAppList();
    }
}

void AppManagerWidget::onDeleteApp()
{
    QModelIndexList selected = ui->appListView->selectionModel()->selectedIndexes();
    if (selected.isEmpty()) {
        QMessageBox::warning(this, "提示", "请先选择要删除的应用");
        return;
    }
    
    QList<int> idsToDelete;
    for (const QModelIndex &index : selected) {
        QStandardItem *item = appModel->itemFromIndex(index);
        if (item) {
            idsToDelete.append(item->data(Qt::UserRole).toInt());
        }
    }
    
    for (int id : idsToDelete) {
        db->deleteApp(id);
    }
    
    refreshAppList();
}

void AppManagerWidget::onLaunchApp()
{
    QModelIndexList selected = ui->appListView->selectionModel()->selectedIndexes();
    if (selected.isEmpty()) {
        QMessageBox::warning(this, "提示", "请先选择要启动的应用");
        return;
    }
    
    for (const QModelIndex &index : selected) {
        QStandardItem *item = appModel->itemFromIndex(index);
        if (item) {
            int appId = item->data(Qt::UserRole).toInt();
            AppInfo app = db->getAppById(appId);
            if (app.id > 0) {
                launchApp(app);
            }
        }
    }
}

void AppManagerWidget::onAppItemDoubleClicked(const QModelIndex &index)
{
    if (!index.isValid()) return;
    
    QStandardItem *item = appModel->itemFromIndex(index);
    if (!item) return;
    
    int appId = item->data(Qt::UserRole).toInt();
    AppInfo app = db->getAppById(appId);
    if (app.id > 0) {
        launchApp(app);
    }
}

void AppManagerWidget::onShowContextMenu(const QPoint &pos)
{
    QModelIndex index = ui->appListView->indexAt(pos);
    if (!index.isValid()) return;
    
    QMenu menu(this);
    
    QAction *renameAction = menu.addAction("重命名");
    QAction *changeIconAction = menu.addAction("修改图标");
    QAction *changePathAction = menu.addAction("修改路径");
    QAction *changeArgsAction = menu.addAction("修改参数");
    menu.addSeparator();
    QAction *launchAction = menu.addAction("启动");
    
    QAction *selected = menu.exec(ui->appListView->mapToGlobal(pos));
    
    if (selected == renameAction) {
        onRenameApp();
    } else if (selected == changeIconAction) {
        onChangeIcon();
    } else if (selected == changePathAction) {
        onChangePath();
    } else if (selected == changeArgsAction) {
        onChangeArguments();
    } else if (selected == launchAction) {
        onLaunchApp();
    }
}

void AppManagerWidget::onRenameApp()
{
    QModelIndex index = ui->appListView->currentIndex();
    if (!index.isValid()) return;
    
    QStandardItem *item = appModel->itemFromIndex(index);
    if (!item) return;
    
    int appId = item->data(Qt::UserRole).toInt();
    AppInfo app = db->getAppById(appId);
    if (app.id <= 0) return;
    
    bool ok;
    QString newName = QInputDialog::getText(this, "重命名", "新名称:", QLineEdit::Normal, app.name, &ok);
    if (ok && !newName.isEmpty()) {
        app.name = newName;
        db->updateApp(app);
        refreshAppList();
    }
}

void AppManagerWidget::onChangeIcon()
{
    QModelIndex index = ui->appListView->currentIndex();
    if (!index.isValid()) return;

    QStandardItem *item = appModel->itemFromIndex(index);
    if (!item) return;

    int appId = item->data(Qt::UserRole).toInt();
    AppInfo app = db->getAppById(appId);
    if (app.id <= 0) return;

    IconSelectorDialog iconDialog(this);
    if (iconDialog.exec() == QDialog::Accepted) {
        QString iconPath = iconDialog.getSelectedIconPath();
        if (!iconPath.isEmpty()) {
            app.iconPath = iconPath;
            db->updateApp(app);
            refreshAppList();
        }
    }
}

void AppManagerWidget::onChangePath()
{
    QModelIndex index = ui->appListView->currentIndex();
    if (!index.isValid()) return;
    
    QStandardItem *item = appModel->itemFromIndex(index);
    if (!item) return;
    
    int appId = item->data(Qt::UserRole).toInt();
    AppInfo app = db->getAppById(appId);
    if (app.id <= 0) return;
    
    QString defaultPath = app.path;
    QString newPath;
    
    if (app.type == AppType_Website) {
        bool ok;
        newPath = QInputDialog::getText(this, "修改网站链接", "请输入网站URL：", QLineEdit::Normal, defaultPath, &ok);
        if (ok && !newPath.trimmed().isEmpty()) {
            QString trimmedUrl = newPath.trimmed();
            if (!trimmedUrl.startsWith("http://") && !trimmedUrl.startsWith("https://")) {
                trimmedUrl = "https://" + trimmedUrl;
            }
            app.path = trimmedUrl;
            db->updateApp(app);
            refreshAppList();
        }
    } else if (app.type == AppType_Folder) {
        newPath = QFileDialog::getExistingDirectory(this, "选择文件夹", defaultPath, 
                                                     QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
        if (!newPath.isEmpty()) {
            app.path = newPath;
            db->updateApp(app);
            refreshAppList();
        }
    } else if (app.type == AppType_Document) {
        newPath = QFileDialog::getOpenFileName(this, "选择文档文件", defaultPath, 
                                              "文档文件 (*.docx *.doc *.pdf *.txt *.xlsx *.xls *.pptx *.ppt);;所有文件 (*.*)");
        if (!newPath.isEmpty()) {
            app.path = newPath;
            db->updateApp(app);
            refreshAppList();
        }
    } else {
        newPath = QFileDialog::getOpenFileName(this, "选择应用程序", defaultPath, 
                                              "可执行文件 (*.exe *.bat *.cmd);;所有文件 (*.*)");
        if (!newPath.isEmpty()) {
            app.path = newPath;
            db->updateApp(app);
            refreshAppList();
        }
    }
}

void AppManagerWidget::onChangeArguments()
{
    QModelIndex index = ui->appListView->currentIndex();
    if (!index.isValid()) return;
    
    QStandardItem *item = appModel->itemFromIndex(index);
    if (!item) return;
    
    int appId = item->data(Qt::UserRole).toInt();
    AppInfo app = db->getAppById(appId);
    if (app.id <= 0) return;
    
    bool ok;
    QString newArgs = QInputDialog::getText(this, "修改启动参数", "启动参数:", QLineEdit::Normal, app.arguments, &ok);
    if (ok) {
        app.arguments = newArgs;
        db->updateApp(app);
        refreshAppList();
    }
}

void AppManagerWidget::launchApp(const AppInfo &app)
{
    ApplicationManager::LaunchOptions options;
    options.updateUseCount = true;
    options.refreshUI = false;
    
    appManager->launchApp(app, options);
}

void AppManagerWidget::onInitApps()
{
    emit resetAppsRequested();
}

void AppManagerWidget::onSortModeChanged(QAction *action)
{
    if (!action) return;

    bool ok;
    int mode = action->data().toInt(&ok);
    if (!ok) return;

    currentSortMode = static_cast<AppSortMode>(mode);

    // 更新按钮文本
    switch (currentSortMode) {
    case AppSortMode_Recent:
        ui->sortModeButton->setText("最近使用");
        break;
    case AppSortMode_UsageCount:
        ui->sortModeButton->setText("使用次数");
        break;
    case AppSortMode_Manual:
        ui->sortModeButton->setText("手动排序");
        break;
    }

    refreshAppList();
}
