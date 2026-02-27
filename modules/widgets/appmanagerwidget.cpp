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
#include <QPainter>
#include <QTextStream>
#include <QProcess>
#include <QTcpSocket>
#include <QDesktopServices>
#include <QUrl>
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
    
    ui->initButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogResetButton));
    ui->initButton->setStyleSheet("QPushButton { background-color: #e91e63; color: white; padding: 8px 16px; border-radius: 5px; font-weight: bold; } "
                                   "QPushButton:hover { background-color: #c2185b; }");
    connect(ui->initButton, &QPushButton::clicked, this, &AppManagerWidget::onInitApps);
    
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
    if (app.isRemoteDesktop || app.type == AppType_RemoteDesktop) {
        return QApplication::style()->standardIcon(QStyle::SP_ComputerIcon);
    }

    if (!app.iconPath.isEmpty() && QFile::exists(app.iconPath)) {
        QIcon icon(app.iconPath);
        if (!icon.isNull()) {
            return icon;
        }
    }
    
    if (app.type == AppType_Website) {
        return QApplication::style()->standardIcon(QStyle::SP_FileDialogDetailedView);
    }
    
    if (app.type == AppType_Folder) {
        return QApplication::style()->standardIcon(QStyle::SP_DirIcon);
    }
    
    if (app.type == AppType_Document) {
        if (QFile::exists(app.path)) {
            QFileInfo fileInfo(app.path);
            return iconProvider.icon(fileInfo);
        }
        return QApplication::style()->standardIcon(QStyle::SP_FileIcon);
    }
    
    if (QFile::exists(app.path)) {
        QFileInfo fileInfo(app.path);
        return iconProvider.icon(fileInfo);
    }
    
    return QApplication::style()->standardIcon(QStyle::SP_FileIcon);
}

void AppManagerWidget::onAddApp()
{
    QDialog dialog(this);
    dialog.setWindowTitle("添加应用");
    dialog.setMinimumWidth(400);
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
    app.isRemoteDesktop = false;
    app.remoteDesktopId = -1;
    
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
    app.isRemoteDesktop = false;
    app.remoteDesktopId = -1;
    
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
    app.isRemoteDesktop = false;
    app.remoteDesktopId = -1;
    
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
    app.isRemoteDesktop = false;
    app.remoteDesktopId = -1;
    
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
    if (app.type == AppType_Website) {
        QDesktopServices::openUrl(QUrl(app.path));
        AppInfo updatedApp = app;
        updatedApp.useCount++;
        db->updateApp(updatedApp);
        refreshAppList();
        return;
    }
    
    if (app.type == AppType_Folder) {
        QDesktopServices::openUrl(QUrl::fromLocalFile(app.path));
        AppInfo updatedApp = app;
        updatedApp.useCount++;
        db->updateApp(updatedApp);
        refreshAppList();
        return;
    }
    
    if (app.type == AppType_Document) {
        QDesktopServices::openUrl(QUrl::fromLocalFile(app.path));
        AppInfo updatedApp = app;
        updatedApp.useCount++;
        db->updateApp(updatedApp);
        refreshAppList();
        return;
    }
    
    if (app.isRemoteDesktop && app.remoteDesktopId > 0) {
        RemoteDesktopConnection conn = db->getRemoteDesktopById(app.remoteDesktopId);
        if (conn.id != -1) {
            QString targetName = "TERMSRV/" + conn.hostAddress;

            if (!conn.username.isEmpty() && !conn.password.isEmpty()) {
                QString username = conn.username;
                if (!conn.domain.isEmpty()) {
                    username = conn.domain + "\\" + username;
                }

                QStringList cmdkeyArgs;
                cmdkeyArgs << "/generic:" + targetName << "/user:" + username << "/pass:" + conn.password;

                QProcess::execute("cmdkey.exe", cmdkeyArgs);
            }

            QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
            QString rdpFilePath = tempDir + "/PonyWork_RDP_" + QString::number(conn.id) + ".rdp";

            QFile rdpFile(rdpFilePath);
            if (rdpFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QTextStream out(&rdpFile);
                out.setCodec("UTF-8");

                QString fullAddress = conn.hostAddress;
                if (conn.port != 3389) {
                    fullAddress += ":" + QString::number(conn.port);
                }

                out << "full address:s:" << fullAddress << "\n";
                out << "screen mode id:i:" << (conn.fullScreen ? "2" : "1") << "\n";
                out << "desktopwidth:i:" << conn.screenWidth << "\n";
                out << "desktopheight:i:" << conn.screenHeight << "\n";
                out << "use multimon:i:" << (conn.useAllMonitors ? "1" : "0") << "\n";
                out << "audiomode:i:" << (conn.enableAudio ? "0" : "2") << "\n";
                out << "redirectclipboard:i:" << (conn.enableClipboard ? "1" : "0") << "\n";
                out << "redirectprinters:i:" << (conn.enablePrinter ? "1" : "0") << "\n";
                out << "redirectdrives:i:" << (conn.enableDrive ? "1" : "0") << "\n";
                out << "authentication level:i:2\n";
                out << "prompt for credentials:i:0\n";
                out << "administrative session:i:0\n";

                if (!conn.username.isEmpty()) {
                    QString username = conn.username;
                    if (!conn.domain.isEmpty()) {
                        username = conn.domain + "\\" + username;
                    }
                    out << "username:s:" << username << "\n";
                }

                rdpFile.close();
            }

            conn.lastUsedTime = QDateTime::currentDateTime();
            db->updateRemoteDesktop(conn);

            QStringList args;
            args << rdpFilePath;

            QProcess::startDetached("mstsc.exe", args);

            AppInfo updatedApp = app;
            updatedApp.useCount++;
            db->updateApp(updatedApp);
            refreshAppList();
            return;
        }
    }

    QString fullPath = app.path;
    QString args = app.arguments;
    
    QString fileName = QFileInfo(fullPath).fileName().toLower();
    
    if (fileName == "cmd.exe" || fileName == "cmd") {
        if (args.trimmed().isEmpty()) {
            QProcess::startDetached("cmd /c start cmd.exe");
        } else {
            QProcess::startDetached(QString("cmd /c start \"\" %1 %2").arg(fullPath, args));
        }
    } else if (fileName == "powershell.exe" || fileName == "powershell") {
        if (args.trimmed().isEmpty()) {
            QProcess::startDetached("cmd /c start powershell.exe");
        } else {
            QProcess::startDetached(QString("cmd /c start \"\" %1 %2").arg(fullPath, args));
        }
    } else {
        if (args.trimmed().isEmpty()) {
            QProcess::startDetached(fullPath, QStringList());
        } else {
            QProcess::startDetached(fullPath, QProcess::splitCommand(args));
        }
    }
    
    AppInfo updatedApp = app;
    updatedApp.useCount++;
    db->updateApp(updatedApp);
    
    refreshAppList();
}

void AppManagerWidget::onInitApps()
{
    emit resetAppsRequested();
}
