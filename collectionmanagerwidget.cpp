#include "collectionmanagerwidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QInputDialog>
#include <QDialog>
#include <QFormLayout>
#include <QCheckBox>
#include <QApplication>
#include <QStyle>
#include <QProcess>
#include <QFileInfo>
#include <QColor>
#include <QLinearGradient>
#include <QPainterPath>
#include <QStandardItem>

CollectionManagerWidget::CollectionManagerWidget(Database *db, QWidget *parent)
    : QWidget(parent), db(db), currentCollectionId(-1)
{
    iconDelegate = new AppIconDelegate(this);
    appsModel = new QStandardItemModel(this);
    
    setupUI();
    refreshCollectionList();
}

CollectionManagerWidget::~CollectionManagerWidget()
{
}

void CollectionManagerWidget::setupUI()
{
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);
    
    QWidget *leftWidget = new QWidget(this);
    QVBoxLayout *leftLayout = new QVBoxLayout(leftWidget);
    
    QLabel *collectionsLabel = new QLabel("应用集合", this);
    collectionsLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: #333;");
    leftLayout->addWidget(collectionsLabel);
    
    collectionListWidget = new QListWidget(this);
    collectionListWidget->setStyleSheet(
        "QListWidget { border: 2px solid #e0e0e0; border-radius: 8px; padding: 5px; background-color: #fafafa; } "
        "QListWidget::item { padding: 10px; border-radius: 5px; margin: 2px; } "
        "QListWidget::item:selected { background-color: #6200ea; color: white; } "
        "QListWidget::item:hover { background-color: #e0e0e0; }"
    );
    connect(collectionListWidget, &QListWidget::itemClicked, this, &CollectionManagerWidget::onCollectionSelected);
    leftLayout->addWidget(collectionListWidget);
    
    QHBoxLayout *leftButtonLayout = new QHBoxLayout();
    
    createCollectionButton = new QPushButton("新建集合", this);
    createCollectionButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_FileDialogNewFolder));
    createCollectionButton->setStyleSheet(
        "QPushButton { background-color: #4caf50; color: white; padding: 8px 16px; border-radius: 5px; font-weight: bold; } "
        "QPushButton:hover { background-color: #45a049; }"
    );
    connect(createCollectionButton, &QPushButton::clicked, this, &CollectionManagerWidget::onCreateCollection);
    leftButtonLayout->addWidget(createCollectionButton);
    
    editCollectionButton = new QPushButton("编辑", this);
    editCollectionButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_FileDialogContentsView));
    editCollectionButton->setStyleSheet(
        "QPushButton { background-color: #2196f3; color: white; padding: 8px 16px; border-radius: 5px; font-weight: bold; } "
        "QPushButton:hover { background-color: #1976d2; }"
    );
    connect(editCollectionButton, &QPushButton::clicked, this, &CollectionManagerWidget::onEditCollection);
    leftButtonLayout->addWidget(editCollectionButton);
    
    deleteCollectionButton = new QPushButton("删除", this);
    deleteCollectionButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_TrashIcon));
    deleteCollectionButton->setStyleSheet(
        "QPushButton { background-color: #f44336; color: white; padding: 8px 16px; border-radius: 5px; font-weight: bold; } "
        "QPushButton:hover { background-color: #d32f2f; }"
    );
    connect(deleteCollectionButton, &QPushButton::clicked, this, &CollectionManagerWidget::onDeleteCollection);
    leftButtonLayout->addWidget(deleteCollectionButton);
    
    leftLayout->addLayout(leftButtonLayout);
    leftWidget->setLayout(leftLayout);
    leftWidget->setMaximumWidth(300);
    
    QWidget *rightWidget = new QWidget(this);
    QVBoxLayout *rightLayout = new QVBoxLayout(rightWidget);
    
    QGroupBox *infoGroup = new QGroupBox("集合信息", this);
    QVBoxLayout *infoLayout = new QVBoxLayout(infoGroup);
    
    collectionNameLabel = new QLabel("请选择一个应用集合", this);
    collectionNameLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #333;");
    infoLayout->addWidget(collectionNameLabel);
    
    collectionDescLabel = new QLabel("", this);
    collectionDescLabel->setStyleSheet("font-size: 12px; color: #666;");
    collectionDescLabel->setWordWrap(true);
    infoLayout->addWidget(collectionDescLabel);
    
    rightLayout->addWidget(infoGroup);
    
    QGroupBox *appsGroup = new QGroupBox("包含的应用", this);
    QVBoxLayout *appsLayout = new QVBoxLayout(appsGroup);
    
    appsListView = new QListView(this);
    appsListView->setModel(appsModel);
    appsListView->setItemDelegate(iconDelegate);
    appsListView->setViewMode(QListView::IconMode);
    appsListView->setIconSize(QSize(72, 72));
    appsListView->setGridSize(QSize(110, 130));
    appsListView->setResizeMode(QListView::Adjust);
    appsListView->setSpacing(15);
    appsListView->setContextMenuPolicy(Qt::CustomContextMenu);
    appsListView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    appsListView->setStyleSheet(
        "QListView { border: 2px solid #e0e0e0; border-radius: 8px; padding: 10px; background-color: #fafafa; }"
    );
    connect(appsListView, &QListView::doubleClicked, this, &CollectionManagerWidget::onAppItemDoubleClicked);
    connect(appsListView, &QListView::customContextMenuRequested, this, &CollectionManagerWidget::onShowContextMenu);
    appsLayout->addWidget(appsListView);
    
    QHBoxLayout *appsButtonLayout = new QHBoxLayout();
    
    addAppButton = new QPushButton("添加应用", this);
    addAppButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_FileDialogDetailedView));
    addAppButton->setStyleSheet(
        "QPushButton { background-color: #673ab7; color: white; padding: 8px 16px; border-radius: 5px; font-weight: bold; } "
        "QPushButton:hover { background-color: #5e35b1; }"
    );
    connect(addAppButton, &QPushButton::clicked, this, &CollectionManagerWidget::onAddAppToCollection);
    appsButtonLayout->addWidget(addAppButton);
    
    removeAppButton = new QPushButton("移除应用", this);
    removeAppButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogCancelButton));
    removeAppButton->setStyleSheet(
        "QPushButton { background-color: #ff9800; color: white; padding: 8px 16px; border-radius: 5px; font-weight: bold; } "
        "QPushButton:hover { background-color: #f57c00; }"
    );
    connect(removeAppButton, &QPushButton::clicked, this, &CollectionManagerWidget::onRemoveAppFromCollection);
    appsButtonLayout->addWidget(removeAppButton);
    
    appsButtonLayout->addStretch();
    
    runCollectionButton = new QPushButton("批量运行", this);
    runCollectionButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_MediaPlay));
    runCollectionButton->setStyleSheet(
        "QPushButton { background-color: #00bcd4; color: white; padding: 8px 20px; border-radius: 5px; font-weight: bold; font-size: 14px; } "
        "QPushButton:hover { background-color: #0097a7; }"
    );
    connect(runCollectionButton, &QPushButton::clicked, this, &CollectionManagerWidget::onRunCollection);
    appsButtonLayout->addWidget(runCollectionButton);
    
    appsLayout->addLayout(appsButtonLayout);
    
    rightLayout->addWidget(appsGroup);
    rightWidget->setLayout(rightLayout);
    
    mainLayout->addWidget(leftWidget);
    mainLayout->addWidget(rightWidget, 1);
}

void CollectionManagerWidget::refreshCollectionList()
{
    collectionListWidget->clear();
    QList<AppCollection> collections = db->getAllCollections();
    
    for (const AppCollection &col : collections) {
        QListWidgetItem *item = new QListWidgetItem(col.name);
        item->setData(Qt::UserRole, col.id);
        collectionListWidget->addItem(item);
    }
}

void CollectionManagerWidget::refreshCollectionApps()
{
    appsModel->clear();
    
    if (currentCollectionId <= 0) {
        return;
    }
    
    AppCollection collection = db->getCollectionById(currentCollectionId);
    if (collection.id <= 0) {
        return;
    }
    
    for (int appId : collection.appIds) {
        AppInfo app = db->getAppById(appId);
        if (app.id > 0) {
            QStandardItem *item = new QStandardItem();
            item->setText(app.name);
            item->setData(app.id, Qt::UserRole);
            item->setIcon(getAppIcon(app));
            
            if (app.isFavorite) {
                item->setBackground(QColor(255, 249, 196));
            }
            
            item->setEditable(false);
            appsModel->appendRow(item);
        }
    }
}

QIcon CollectionManagerWidget::getAppIcon(const AppInfo &app)
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

void CollectionManagerWidget::onCreateCollection()
{
    QDialog dialog(this);
    dialog.setWindowTitle("新建应用集合");
    dialog.setMinimumWidth(400);
    
    QFormLayout *formLayout = new QFormLayout(&dialog);
    
    QLineEdit *nameEdit = new QLineEdit(&dialog);
    formLayout->addRow("集合名称:", nameEdit);
    
    QTextEdit *descEdit = new QTextEdit(&dialog);
    descEdit->setMaximumHeight(100);
    formLayout->addRow("集合描述:", descEdit);
    
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *okButton = new QPushButton("确定", &dialog);
    QPushButton *cancelButton = new QPushButton("取消", &dialog);
    
    buttonLayout->addStretch();
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);
    
    formLayout->addRow(buttonLayout);
    
    connect(okButton, &QPushButton::clicked, &dialog, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);
    
    if (dialog.exec() == QDialog::Accepted) {
        QString name = nameEdit->text().trimmed();
        if (name.isEmpty()) {
            QMessageBox::warning(this, "提示", "集合名称不能为空！");
            return;
        }
        
        AppCollection newCol;
        newCol.name = name;
        newCol.description = descEdit->toPlainText().trimmed();
        
        if (db->addCollection(newCol)) {
            QMessageBox::information(this, "成功", "集合创建成功！");
            refreshCollectionList();
        } else {
            QMessageBox::warning(this, "错误", "集合创建失败！");
        }
    }
}

void CollectionManagerWidget::onEditCollection()
{
    if (currentCollectionId <= 0) {
        QMessageBox::warning(this, "提示", "请先选择一个集合！");
        return;
    }
    
    AppCollection collection = db->getCollectionById(currentCollectionId);
    if (collection.id <= 0) {
        QMessageBox::warning(this, "错误", "无法找到该集合！");
        return;
    }
    
    QDialog dialog(this);
    dialog.setWindowTitle("编辑应用集合");
    dialog.setMinimumWidth(400);
    
    QFormLayout *formLayout = new QFormLayout(&dialog);
    
    QLineEdit *nameEdit = new QLineEdit(collection.name, &dialog);
    formLayout->addRow("集合名称:", nameEdit);
    
    QTextEdit *descEdit = new QTextEdit(collection.description, &dialog);
    descEdit->setMaximumHeight(100);
    formLayout->addRow("集合描述:", descEdit);
    
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *okButton = new QPushButton("确定", &dialog);
    QPushButton *cancelButton = new QPushButton("取消", &dialog);
    
    buttonLayout->addStretch();
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);
    
    formLayout->addRow(buttonLayout);
    
    connect(okButton, &QPushButton::clicked, &dialog, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);
    
    if (dialog.exec() == QDialog::Accepted) {
        QString name = nameEdit->text().trimmed();
        if (name.isEmpty()) {
            QMessageBox::warning(this, "提示", "集合名称不能为空！");
            return;
        }
        
        collection.name = name;
        collection.description = descEdit->toPlainText().trimmed();
        
        if (db->updateCollection(collection)) {
            QMessageBox::information(this, "成功", "集合更新成功！");
            refreshCollectionList();
            onCollectionSelected(nullptr);
            for (int i = 0; i < collectionListWidget->count(); ++i) {
                if (collectionListWidget->item(i)->data(Qt::UserRole).toInt() == currentCollectionId) {
                    collectionListWidget->setCurrentRow(i);
                    onCollectionSelected(collectionListWidget->item(i));
                    break;
                }
            }
        } else {
            QMessageBox::warning(this, "错误", "集合更新失败！");
        }
    }
}

void CollectionManagerWidget::onDeleteCollection()
{
    if (currentCollectionId <= 0) {
        QMessageBox::warning(this, "提示", "请先选择一个集合！");
        return;
    }
    
    auto reply = QMessageBox::question(this, "确认删除", "确定要删除这个集合吗？此操作不可恢复！",
                                       QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        if (db->deleteCollection(currentCollectionId)) {
            QMessageBox::information(this, "成功", "集合删除成功！");
            currentCollectionId = -1;
            refreshCollectionList();
            collectionNameLabel->setText("请选择一个应用集合");
            collectionDescLabel->setText("");
            appsModel->clear();
        } else {
            QMessageBox::warning(this, "错误", "集合删除失败！");
        }
    }
}

void CollectionManagerWidget::onCollectionSelected(QListWidgetItem *item)
{
    if (!item) {
        currentCollectionId = -1;
        collectionNameLabel->setText("请选择一个应用集合");
        collectionDescLabel->setText("");
        appsModel->clear();
        return;
    }
    
    currentCollectionId = item->data(Qt::UserRole).toInt();
    AppCollection collection = db->getCollectionById(currentCollectionId);
    
    if (collection.id > 0) {
        collectionNameLabel->setText(collection.name);
        collectionDescLabel->setText(collection.description.isEmpty() ? "无描述" : collection.description);
        refreshCollectionApps();
    }
}

void CollectionManagerWidget::onAddAppToCollection()
{
    if (currentCollectionId <= 0) {
        QMessageBox::warning(this, "提示", "请先选择一个集合！");
        return;
    }
    
    AppCollection collection = db->getCollectionById(currentCollectionId);
    if (collection.id <= 0) {
        QMessageBox::warning(this, "错误", "无法找到该集合！");
        return;
    }
    
    QDialog dialog(this);
    dialog.setWindowTitle("添加应用到集合");
    dialog.setMinimumWidth(500);
    dialog.setMinimumHeight(400);
    
    QVBoxLayout *layout = new QVBoxLayout(&dialog);
    
    QLabel *label = new QLabel("选择要添加的应用（可多选）：", &dialog);
    layout->addWidget(label);
    
    QListWidget *appListWidget = new QListWidget(&dialog);
    appListWidget->setSelectionMode(QAbstractItemView::MultiSelection);
    
    QList<AppInfo> allApps = db->getAllApps();
    for (const AppInfo &app : allApps) {
        if (!collection.appIds.contains(app.id)) {
            QListWidgetItem *item = new QListWidgetItem(app.name);
            item->setData(Qt::UserRole, app.id);
            appListWidget->addItem(item);
        }
    }
    
    layout->addWidget(appListWidget);
    
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *okButton = new QPushButton("确定", &dialog);
    QPushButton *cancelButton = new QPushButton("取消", &dialog);
    
    buttonLayout->addStretch();
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);
    
    layout->addLayout(buttonLayout);
    
    connect(okButton, &QPushButton::clicked, &dialog, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);
    
    if (dialog.exec() == QDialog::Accepted) {
        QList<QListWidgetItem *> selectedItems = appListWidget->selectedItems();
        if (selectedItems.isEmpty()) {
            return;
        }
        
        for (QListWidgetItem *item : selectedItems) {
            int appId = item->data(Qt::UserRole).toInt();
            if (!collection.appIds.contains(appId)) {
                collection.appIds.append(appId);
            }
        }
        
        if (db->updateCollection(collection)) {
            QMessageBox::information(this, "成功", "应用添加成功！");
            refreshCollectionApps();
        } else {
            QMessageBox::warning(this, "错误", "应用添加失败！");
        }
    }
}

void CollectionManagerWidget::onRemoveAppFromCollection()
{
    if (currentCollectionId <= 0) {
        QMessageBox::warning(this, "提示", "请先选择一个集合！");
        return;
    }
    
    QModelIndexList selectedIndexes = appsListView->selectionModel()->selectedIndexes();
    if (selectedIndexes.isEmpty()) {
        QMessageBox::warning(this, "提示", "请先选择要移除的应用！");
        return;
    }
    
    AppCollection collection = db->getCollectionById(currentCollectionId);
    if (collection.id <= 0) {
        QMessageBox::warning(this, "错误", "无法找到该集合！");
        return;
    }
    
    QList<int> appIdsToRemove;
    for (const QModelIndex &index : selectedIndexes) {
        QStandardItem *item = appsModel->itemFromIndex(index);
        if (item) {
            int appId = item->data(Qt::UserRole).toInt();
            appIdsToRemove.append(appId);
        }
    }
    
    for (int appId : appIdsToRemove) {
        collection.appIds.removeAll(appId);
    }
    
    if (db->updateCollection(collection)) {
        QMessageBox::information(this, "成功", "应用移除成功！");
        refreshCollectionApps();
    } else {
        QMessageBox::warning(this, "错误", "应用移除失败！");
    }
}

void CollectionManagerWidget::onRunCollection()
{
    if (currentCollectionId <= 0) {
        QMessageBox::warning(this, "提示", "请先选择一个集合！");
        return;
    }
    
    AppCollection collection = db->getCollectionById(currentCollectionId);
    if (collection.id <= 0) {
        QMessageBox::warning(this, "错误", "无法找到该集合！");
        return;
    }
    
    if (collection.appIds.isEmpty()) {
        QMessageBox::warning(this, "提示", "该集合中没有应用！");
        return;
    }
    
    int successCount = 0;
    int failCount = 0;
    
    for (int appId : collection.appIds) {
        AppInfo app = db->getAppById(appId);
        if (app.id > 0) {
            launchApp(app);
            successCount++;
        } else {
            failCount++;
        }
    }
    
    QString message = QString("批量运行完成！\n成功: %1 个\n失败: %2 个").arg(successCount).arg(failCount);
    QMessageBox::information(this, "完成", message);
}

void CollectionManagerWidget::onAppItemDoubleClicked(const QModelIndex &index)
{
    if (!index.isValid()) return;
    
    QStandardItem *item = appsModel->itemFromIndex(index);
    if (!item) return;
    
    int appId = item->data(Qt::UserRole).toInt();
    AppInfo app = db->getAppById(appId);
    if (app.id > 0) {
        launchApp(app);
    }
}

void CollectionManagerWidget::onShowContextMenu(const QPoint &pos)
{
    QModelIndex index = appsListView->indexAt(pos);
    if (!index.isValid()) return;
    
    QMenu menu(this);
    
    QAction *launchAction = menu.addAction("启动");
    QAction *removeAction = menu.addAction("从集合移除");
    
    QAction *selected = menu.exec(appsListView->mapToGlobal(pos));
    
    if (selected == launchAction) {
        onLaunchAppFromCollection();
    } else if (selected == removeAction) {
        onRemoveAppFromCollection();
    }
}

void CollectionManagerWidget::onLaunchAppFromCollection()
{
    QModelIndexList selected = appsListView->selectionModel()->selectedIndexes();
    if (selected.isEmpty()) {
        QMessageBox::warning(this, "提示", "请先选择要启动的应用");
        return;
    }
    
    for (const QModelIndex &index : selected) {
        QStandardItem *item = appsModel->itemFromIndex(index);
        if (item) {
            int appId = item->data(Qt::UserRole).toInt();
            AppInfo app = db->getAppById(appId);
            if (app.id > 0) {
                launchApp(app);
            }
        }
    }
}

void CollectionManagerWidget::launchApp(const AppInfo &app)
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
    
    refreshCollectionApps();
}

void CollectionManagerWidget::runApp(const AppInfo &app)
{
    launchApp(app);
}
