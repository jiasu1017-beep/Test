#include "appmanagerwidget.h"
#include <QApplication>
#include <QStyle>
#include <QFileInfo>
#include <QListWidgetItem>
#include <QStandardPaths>
#include <QColor>
#include <QLinearGradient>
#include <QPainterPath>
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
        painter->fillRect(rect, QColor(240, 248, 255, 150));
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

AppManagerWidget::AppManagerWidget(Database *db, QWidget *parent)
    : QWidget(parent), db(db)
{
    iconDelegate = new AppIconDelegate(this);
    setupUI();
    refreshAppList();
}

void AppManagerWidget::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    QLabel *titleLabel = new QLabel("应用管理", this);
    titleLabel->setStyleSheet("font-size: 22px; font-weight: bold; padding: 10px; color: #1976d2;");
    mainLayout->addWidget(titleLabel);
    
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    
    addButton = new QPushButton("添加应用", this);
    addButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_FileDialogNewFolder));
    addButton->setStyleSheet("QPushButton { background-color: #4caf50; color: white; padding: 8px 16px; border-radius: 5px; font-weight: bold; } "
                            "QPushButton:hover { background-color: #43a047; }");
    connect(addButton, &QPushButton::clicked, this, &AppManagerWidget::onAddApp);
    
    deleteButton = new QPushButton("删除应用", this);
    deleteButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_TrashIcon));
    deleteButton->setStyleSheet("QPushButton { background-color: #f44336; color: white; padding: 8px 16px; border-radius: 5px; font-weight: bold; } "
                               "QPushButton:hover { background-color: #d32f2f; }");
    connect(deleteButton, &QPushButton::clicked, this, &AppManagerWidget::onDeleteApp);
    
    launchButton = new QPushButton("启动应用", this);
    launchButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_MediaPlay));
    launchButton->setStyleSheet("QPushButton { background-color: #2196f3; color: white; padding: 8px 16px; border-radius: 5px; font-weight: bold; } "
                               "QPushButton:hover { background-color: #1976d2; }");
    connect(launchButton, &QPushButton::clicked, this, &AppManagerWidget::onLaunchApp);
    
    refreshButton = new QPushButton("刷新", this);
    refreshButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_BrowserReload));
    refreshButton->setStyleSheet("QPushButton { background-color: #ff9800; color: white; padding: 8px 16px; border-radius: 5px; font-weight: bold; } "
                                "QPushButton:hover { background-color: #fb8c00; }");
    connect(refreshButton, &QPushButton::clicked, this, &AppManagerWidget::refreshAppList);
    
    buttonLayout->addWidget(addButton);
    buttonLayout->addWidget(deleteButton);
    buttonLayout->addWidget(launchButton);
    buttonLayout->addWidget(refreshButton);
    buttonLayout->addStretch();
    
    iconViewButton = new QPushButton("大图标", this);
    iconViewButton->setCheckable(true);
    iconViewButton->setChecked(true);
    iconViewButton->setStyleSheet("QPushButton:checked { background-color: #1976d2; color: white; } "
                                  "QPushButton { background-color: #e0e0e0; color: #333; }");
    connect(iconViewButton, &QPushButton::clicked, this, &AppManagerWidget::onIconViewMode);
    
    listViewButton = new QPushButton("列表", this);
    listViewButton->setCheckable(true);
    listViewButton->setStyleSheet("QPushButton:checked { background-color: #1976d2; color: white; } "
                                 "QPushButton { background-color: #e0e0e0; color: #333; }");
    connect(listViewButton, &QPushButton::clicked, this, &AppManagerWidget::onListViewMode);
    
    buttonLayout->addWidget(iconViewButton);
    buttonLayout->addWidget(listViewButton);
    
    mainLayout->addLayout(buttonLayout);
    
    appListWidget = new QListWidget(this);
    appListWidget->setViewMode(QListWidget::IconMode);
    appListWidget->setIconSize(QSize(72, 72));
    appListWidget->setResizeMode(QListWidget::Adjust);
    appListWidget->setSpacing(15);
    appListWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    appListWidget->setDragDropMode(QAbstractItemView::InternalMove);
    appListWidget->setDefaultDropAction(Qt::MoveAction);
    appListWidget->setMovement(QListView::Snap);
    appListWidget->setItemDelegate(iconDelegate);
    appListWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    appListWidget->setStyleSheet("QListWidget { background-color: #fafafa; border: none; }");
    
    connect(appListWidget, &QListWidget::customContextMenuRequested, this, &AppManagerWidget::onShowContextMenu);
    connect(appListWidget, &QListWidget::itemDoubleClicked, this, &AppManagerWidget::onAppItemDoubleClicked);
    connect(appListWidget->model(), &QAbstractItemModel::rowsMoved, this, &AppManagerWidget::onRowsMoved);
    
    mainLayout->addWidget(appListWidget);
}

void AppManagerWidget::onIconViewMode()
{
    appListWidget->setViewMode(QListWidget::IconMode);
    appListWidget->setItemDelegate(iconDelegate);
    appListWidget->setIconSize(QSize(72, 72));
    appListWidget->setSpacing(15);
    iconViewButton->setChecked(true);
    listViewButton->setChecked(false);
}

void AppManagerWidget::onListViewMode()
{
    appListWidget->setViewMode(QListWidget::ListMode);
    appListWidget->setItemDelegate(nullptr);
    appListWidget->setIconSize(QSize(32, 32));
    appListWidget->setSpacing(5);
    iconViewButton->setChecked(false);
    listViewButton->setChecked(true);
}

void AppManagerWidget::onRowsMoved(const QModelIndex &parent, int start, int end, const QModelIndex &destination, int row)
{
    Q_UNUSED(parent);
    Q_UNUSED(start);
    Q_UNUSED(end);
    Q_UNUSED(destination);
    Q_UNUSED(row);
    saveAppOrder();
}

void AppManagerWidget::saveAppOrder()
{
    for (int i = 0; i < appListWidget->count(); ++i) {
        QListWidgetItem *item = appListWidget->item(i);
        int appId = item->data(Qt::UserRole).toInt();
        AppInfo app = db->getAppById(appId);
        app.sortOrder = i;
        db->updateApp(app);
    }
}

void AppManagerWidget::refreshAppList()
{
    appListWidget->clear();
    
    QList<AppInfo> apps = db->getAllApps();
    
    for (const AppInfo &app : apps) {
        QListWidgetItem *item = new QListWidgetItem(app.name);
        item->setData(Qt::UserRole, app.id);
        item->setIcon(getAppIcon(app));
        
        if (app.isFavorite) {
            item->setBackground(QColor(255, 249, 196));
        }
        
        appListWidget->addItem(item);
    }
}

QIcon AppManagerWidget::getAppIcon(const AppInfo &app)
{
    if (!app.iconPath.isEmpty() && QFile::exists(app.iconPath)) {
        QIcon icon(app.iconPath);
        if (!icon.isNull()) {
            return icon;
        }
    }
    
    QFileInfo fileInfo(app.path);
    if (fileInfo.exists()) {
        return iconProvider.icon(fileInfo);
    }
    
    return QApplication::style()->standardIcon(QStyle::SP_ComputerIcon);
}

void AppManagerWidget::onAddApp()
{
    QString filePath = QFileDialog::getOpenFileName(this, "选择应用程序", "", "可执行文件 (*.exe);;所有文件 (*.*)");
    
    if (filePath.isEmpty()) {
        return;
    }
    
    QFileInfo fileInfo(filePath);
    QString appName = fileInfo.baseName();
    
    QList<AppInfo> currentApps = db->getAllApps();
    int maxSortOrder = 0;
    for (const AppInfo &app : currentApps) {
        if (app.sortOrder > maxSortOrder) {
            maxSortOrder = app.sortOrder;
        }
    }
    
    AppInfo newApp;
    newApp.name = appName;
    newApp.path = filePath;
    newApp.category = "自定义应用";
    newApp.isFavorite = false;
    newApp.useCount = 0;
    newApp.sortOrder = maxSortOrder + 1;
    
    if (db->addApp(newApp)) {
        QMessageBox::information(this, "成功", "应用添加成功！");
        refreshAppList();
    } else {
        QMessageBox::warning(this, "错误", "添加应用失败！");
    }
}

void AppManagerWidget::onDeleteApp()
{
    QList<QListWidgetItem *> selectedItems = appListWidget->selectedItems();
    if (selectedItems.isEmpty()) {
        QMessageBox::warning(this, "提示", "请先选择要删除的应用！");
        return;
    }
    
    auto reply = QMessageBox::question(this, "确认", 
                                      QString("确定要删除选中的 %1 个应用吗？").arg(selectedItems.size()), 
                                      QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        for (QListWidgetItem *item : selectedItems) {
            int appId = item->data(Qt::UserRole).toInt();
            db->deleteApp(appId);
        }
        QMessageBox::information(this, "成功", "应用已删除！");
        refreshAppList();
    }
}

void AppManagerWidget::onLaunchApp()
{
    QList<QListWidgetItem *> selectedItems = appListWidget->selectedItems();
    if (selectedItems.isEmpty()) {
        QMessageBox::warning(this, "提示", "请先选择要启动的应用！");
        return;
    }
    
    for (QListWidgetItem *item : selectedItems) {
        int appId = item->data(Qt::UserRole).toInt();
        AppInfo app = db->getAppById(appId);
        launchApp(app);
    }
}

void AppManagerWidget::launchApp(const AppInfo &app)
{
    QString fullPath = app.path;
    QString args = app.arguments;
    
    if (args.trimmed().isEmpty()) {
        QProcess::startDetached(fullPath, QStringList());
    } else {
        QProcess::startDetached(fullPath, QProcess::splitCommand(args));
    }
    
    AppInfo updatedApp = app;
    updatedApp.useCount++;
    db->updateApp(updatedApp);
    
    refreshAppList();
}

void AppManagerWidget::onAppItemDoubleClicked(QListWidgetItem *item)
{
    if (!item) return;
    
    int appId = item->data(Qt::UserRole).toInt();
    AppInfo app = db->getAppById(appId);
    launchApp(app);
}

void AppManagerWidget::onShowContextMenu(const QPoint &pos)
{
    QListWidgetItem *item = appListWidget->itemAt(pos);
    if (!item) return;
    
    int appId = item->data(Qt::UserRole).toInt();
    AppInfo app = db->getAppById(appId);
    
    QMenu menu(this);
    menu.setStyleSheet("QMenu { background-color: white; border: 1px solid #ccc; } "
                      "QMenu::item { padding: 8px 20px; } "
                      "QMenu::item:selected { background-color: #1976d2; color: white; }");
    
    QAction *launchAction = menu.addAction("启动");
    connect(launchAction, &QAction::triggered, [this, app]() {
        launchApp(app);
    });
    
    QAction *toggleFavoriteAction = menu.addAction(app.isFavorite ? "取消常用" : "设为常用");
    connect(toggleFavoriteAction, &QAction::triggered, [this, app]() {
        AppInfo updatedApp = app;
        updatedApp.isFavorite = !updatedApp.isFavorite;
        db->updateApp(updatedApp);
        refreshAppList();
    });
    
    menu.addSeparator();
    
    QAction *renameAction = menu.addAction("修改名称");
    connect(renameAction, &QAction::triggered, this, &AppManagerWidget::onRenameApp);
    
    QAction *changeIconAction = menu.addAction("修改图标");
    connect(changeIconAction, &QAction::triggered, this, &AppManagerWidget::onChangeIcon);
    
    QAction *changePathAction = menu.addAction("修改启动路径");
    connect(changePathAction, &QAction::triggered, this, &AppManagerWidget::onChangePath);
    
    QAction *changeArgsAction = menu.addAction("修改启动参数");
    connect(changeArgsAction, &QAction::triggered, this, &AppManagerWidget::onChangeArguments);
    
    menu.addSeparator();
    
    QAction *deleteAction = menu.addAction("删除");
    connect(deleteAction, &QAction::triggered, [this, app]() {
        auto reply = QMessageBox::question(this, "确认", "确定要删除这个应用吗？", 
                                            QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            db->deleteApp(app.id);
            refreshAppList();
        }
    });
    
    menu.exec(appListWidget->mapToGlobal(pos));
}

void AppManagerWidget::onRenameApp()
{
    QListWidgetItem *currentItem = appListWidget->currentItem();
    if (!currentItem) return;
    
    int appId = currentItem->data(Qt::UserRole).toInt();
    AppInfo app = db->getAppById(appId);
    
    bool ok;
    QString newName = QInputDialog::getText(this, "修改名称", "请输入新名称:", QLineEdit::Normal, app.name, &ok);
    
    if (ok && !newName.isEmpty()) {
        AppInfo updatedApp = app;
        updatedApp.name = newName;
        db->updateApp(updatedApp);
        refreshAppList();
        QMessageBox::information(this, "成功", "名称已修改！");
    }
}

void AppManagerWidget::onChangeIcon()
{
    QListWidgetItem *currentItem = appListWidget->currentItem();
    if (!currentItem) return;
    
    int appId = currentItem->data(Qt::UserRole).toInt();
    AppInfo app = db->getAppById(appId);
    
    QString filePath = QFileDialog::getOpenFileName(this, "选择图标文件", "", 
                                                    "图标文件 (*.png *.jpg *.jpeg *.bmp *.ico);;所有文件 (*.*)");
    
    if (!filePath.isEmpty()) {
        AppInfo updatedApp = app;
        updatedApp.iconPath = filePath;
        db->updateApp(updatedApp);
        refreshAppList();
        QMessageBox::information(this, "成功", "图标已更换！");
    }
}

void AppManagerWidget::onChangePath()
{
    QListWidgetItem *currentItem = appListWidget->currentItem();
    if (!currentItem) return;
    
    int appId = currentItem->data(Qt::UserRole).toInt();
    AppInfo app = db->getAppById(appId);
    
    QFileInfo fileInfo(app.path);
    QString defaultDir = fileInfo.exists() ? fileInfo.absolutePath() : "";
    QString defaultFile = fileInfo.exists() ? fileInfo.fileName() : "";
    
    QString filePath = QFileDialog::getOpenFileName(this, "选择应用程序", 
                                                     defaultDir + "/" + defaultFile, 
                                                     "可执行文件 (*.exe);;所有文件 (*.*)");
    
    if (!filePath.isEmpty()) {
        AppInfo updatedApp = app;
        updatedApp.path = filePath;
        db->updateApp(updatedApp);
        refreshAppList();
        QMessageBox::information(this, "成功", "启动路径已修改！");
    }
}

void AppManagerWidget::onChangeArguments()
{
    QListWidgetItem *currentItem = appListWidget->currentItem();
    if (!currentItem) return;
    
    int appId = currentItem->data(Qt::UserRole).toInt();
    AppInfo app = db->getAppById(appId);
    
    bool ok;
    QString newArgs = QInputDialog::getText(this, "修改启动参数", "请输入启动参数:", QLineEdit::Normal, app.arguments, &ok);
    
    if (ok) {
        AppInfo updatedApp = app;
        updatedApp.arguments = newArgs;
        db->updateApp(updatedApp);
        refreshAppList();
        QMessageBox::information(this, "成功", "启动参数已修改！");
    }
}
