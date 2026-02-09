#include "appmanagerwidget.h"
#include "ui_appmanagerwidget.h"
#include <QApplication>
#include <QStyle>
#include <QFileInfo>
#include <QStandardItem>
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
    : QWidget(parent), db(db), ui(new Ui::AppManagerWidget)
{
    ui->setupUi(this);
    
    iconDelegate = new AppIconDelegate(this);
    appModel = new QStandardItemModel(this);
    ui->appListView->setModel(appModel);
    ui->appListView->setItemDelegate(iconDelegate);
    
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
    
    ui->iconViewButton->setStyleSheet("QPushButton:checked { background-color: #1976d2; color: white; } "
                                     "QPushButton { background-color: #e0e0e0; color: #333; }");
    connect(ui->iconViewButton, &QPushButton::clicked, this, &AppManagerWidget::onIconViewMode);
    
    ui->listViewButton->setStyleSheet("QPushButton:checked { background-color: #1976d2; color: white; } "
                                    "QPushButton { background-color: #e0e0e0; color: #333; }");
    connect(ui->listViewButton, &QPushButton::clicked, this, &AppManagerWidget::onListViewMode);
    
    connect(ui->appListView, &QListView::customContextMenuRequested, this, &AppManagerWidget::onShowContextMenu);
    connect(ui->appListView, &QListView::doubleClicked, this, &AppManagerWidget::onAppItemDoubleClicked);
}

void AppManagerWidget::onIconViewMode()
{
    ui->appListView->setViewMode(QListView::IconMode);
    ui->appListView->setItemDelegate(iconDelegate);
    ui->appListView->setIconSize(QSize(72, 72));
    ui->appListView->setSpacing(15);
    ui->iconViewButton->setChecked(true);
    ui->listViewButton->setChecked(false);
}

void AppManagerWidget::onListViewMode()
{
    ui->appListView->setViewMode(QListView::ListMode);
    ui->appListView->setItemDelegate(nullptr);
    ui->appListView->setIconSize(QSize(32, 32));
    ui->appListView->setSpacing(5);
    ui->iconViewButton->setChecked(false);
    ui->listViewButton->setChecked(true);
}

void AppManagerWidget::onMoveUp()
{
    QModelIndex current = ui->appListView->currentIndex();
    if (!current.isValid() || current.row() <= 0) return;
    
    int row = current.row();
    QList<AppInfo> apps = db->getAllApps();
    
    if (row > 0 && row < apps.size()) {
        AppInfo currentApp = apps[row];
        AppInfo prevApp = apps[row - 1];
        
        int tempOrder = currentApp.sortOrder;
        currentApp.sortOrder = prevApp.sortOrder;
        prevApp.sortOrder = tempOrder;
        
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
    QList<AppInfo> apps = db->getAllApps();
    
    if (row >= 0 && row < apps.size() - 1) {
        AppInfo currentApp = apps[row];
        AppInfo nextApp = apps[row + 1];
        
        int tempOrder = currentApp.sortOrder;
        currentApp.sortOrder = nextApp.sortOrder;
        nextApp.sortOrder = tempOrder;
        
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
    
    QList<AppInfo> apps = db->getAllApps();
    
    for (const AppInfo &app : apps) {
        QStandardItem *item = new QStandardItem(app.name);
        item->setData(app.id, Qt::UserRole);
        item->setIcon(getAppIcon(app));
        
        if (app.isFavorite) {
            item->setBackground(QColor(255, 249, 196));
        }
        
        appModel->appendRow(item);
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
    
    if (QFile::exists(app.path)) {
        QFileInfo fileInfo(app.path);
        return iconProvider.icon(fileInfo);
    }
    
    return QApplication::style()->standardIcon(QStyle::SP_FileIcon);
}

void AppManagerWidget::onAddApp()
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
    app.category = "自定义";
    app.useCount = 0;
    app.isFavorite = false;
    
    int maxOrder = 0;
    QList<AppInfo> existingApps = db->getAllApps();
    for (const AppInfo &existing : existingApps) {
        if (existing.sortOrder > maxOrder) {
            maxOrder = existing.sortOrder;
        }
    }
    app.sortOrder = maxOrder + 1;
    
    db->addApp(app);
    refreshAppList();
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
    
    QString defaultPath = app.iconPath;
    QString iconPath = QFileDialog::getOpenFileName(this, "选择图标", defaultPath, 
                                                   "图标文件 (*.ico *.png *.jpg *.bmp);;所有文件 (*.*)");
    if (!iconPath.isEmpty()) {
        app.iconPath = iconPath;
        db->updateApp(app);
        refreshAppList();
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
    QString newPath = QFileDialog::getOpenFileName(this, "选择应用程序", defaultPath, 
                                                    "可执行文件 (*.exe *.bat *.cmd);;所有文件 (*.*)");
    if (!newPath.isEmpty()) {
        app.path = newPath;
        db->updateApp(app);
        refreshAppList();
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
