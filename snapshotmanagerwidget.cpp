#include "snapshotmanagerwidget.h"
#include <QApplication>
#include <QStyle>
#include <QFileInfo>
#include <QFileIconProvider>
#include <QStandardItem>
#include <QStandardPaths>
#include <QColor>
#include <QLinearGradient>
#include <QPainterPath>
#include <QPainter>
#include <QTextStream>
#include <QProcess>
#include <QTcpSocket>
#include <QDirIterator>
#include <QJsonDocument>
#include <QJsonObject>

SnapshotIconDelegate::SnapshotIconDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

void SnapshotIconDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
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
    
    int iconSize = 64;
    int textHeight = 48;
    int padding = 8;
    
    int totalHeight = iconSize + textHeight + padding * 3;
    int startY = rect.top() + (rect.height() - totalHeight) / 2;
    
    QRect iconRect(rect.left() + (rect.width() - iconSize) / 2, 
                   startY + padding, 
                   iconSize, iconSize);
    
    QPainterPath path;
    path.addRoundedRect(iconRect, 12, 12);
    
    QLinearGradient iconGradient(iconRect.topLeft(), iconRect.bottomRight());
    iconGradient.setColorAt(0, QColor(255, 255, 255));
    iconGradient.setColorAt(1, QColor(245, 245, 245));
    painter->fillPath(path, iconGradient);
    
    painter->setPen(QColor(220, 220, 220));
    painter->drawPath(path);
    
    QIcon icon = index.data(Qt::DecorationRole).value<QIcon>();
    QPixmap pixmap = icon.pixmap(QSize(48, 48));
    QRect pixmapRect(iconRect.left() + (iconSize - 48) / 2,
                    iconRect.top() + (iconSize - 48) / 2,
                    48, 48);
    painter->drawPixmap(pixmapRect, pixmap);
    
    QRect textRect(rect.left() + padding,
                   iconRect.bottom() + padding,
                   rect.width() - padding * 2,
                   textHeight);
    
    QString text = index.data(Qt::DisplayRole).toString();
    QFont font = painter->font();
    font.setPixelSize(11);
    font.setBold(false);
    painter->setFont(font);
    painter->setPen(QColor(51, 51, 51));
    painter->drawText(textRect, Qt::AlignCenter | Qt::TextWordWrap, text);
    
    painter->restore();
}

QSize SnapshotIconDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);
    return QSize(140, 160);
}

SnapshotManagerWidget::SnapshotManagerWidget(Database *db, QWidget *parent)
    : QWidget(parent), db(db), showingFavorites(false)
{
    iconDelegate = new SnapshotIconDelegate(this);
    snapshotModel = new QStandardItemModel(this);
    setupUI();
    refreshSnapshotList();
}

SnapshotManagerWidget::~SnapshotManagerWidget()
{
}

void SnapshotManagerWidget::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(12);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    
    QLabel *titleLabel = new QLabel("快照管理", this);
    titleLabel->setStyleSheet("font-size: 22px; font-weight: bold; color: #2d3436;");
    mainLayout->addWidget(titleLabel);
    
    QHBoxLayout *searchFilterLayout = new QHBoxLayout();
    searchFilterLayout->setSpacing(10);
    
    searchEdit = new QLineEdit(this);
    searchEdit->setPlaceholderText("搜索快照...");
    searchEdit->setStyleSheet("QLineEdit { padding: 10px 14px; border: 2px solid #e8ecf1; border-radius: 8px; font-size: 14px; } QLineEdit:focus { border-color: #0984e3; }");
    connect(searchEdit, &QLineEdit::textChanged, this, &SnapshotManagerWidget::onSearchSnapshot);
    searchFilterLayout->addWidget(searchEdit, 1);
    
    typeFilterCombo = new QComboBox(this);
    typeFilterCombo->setStyleSheet("QComboBox { padding: 10px 14px; border: 2px solid #e8ecf1; border-radius: 8px; font-size: 14px; } QComboBox:focus { border-color: #0984e3; }");
    typeFilterCombo->addItem("全部类型", -1);
    typeFilterCombo->addItem("文件夹", SnapshotType_Folder);
    typeFilterCombo->addItem("网站", SnapshotType_Website);
    typeFilterCombo->addItem("文档", SnapshotType_Document);
    connect(typeFilterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SnapshotManagerWidget::onFilterByType);
    searchFilterLayout->addWidget(typeFilterCombo);
    
    mainLayout->addLayout(searchFilterLayout);
    
    QHBoxLayout *viewButtonsLayout = new QHBoxLayout();
    viewButtonsLayout->setSpacing(10);
    
    showAllButton = new QPushButton("全部", this);
    showAllButton->setStyleSheet("QPushButton:checked { background-color: #2196f3; color: white; } QPushButton { background-color: #e0e0e0; color: #333; padding: 8px 16px; border-radius: 8px; font-weight: bold; }");
    showAllButton->setCheckable(true);
    showAllButton->setChecked(true);
    connect(showAllButton, &QPushButton::clicked, this, &SnapshotManagerWidget::onShowAll);
    viewButtonsLayout->addWidget(showAllButton);
    
    showFavoritesButton = new QPushButton("收藏", this);
    showFavoritesButton->setStyleSheet("QPushButton:checked { background-color: #2196f3; color: white; } QPushButton { background-color: #e0e0e0; color: #333; padding: 8px 16px; border-radius: 8px; font-weight: bold; }");
    showFavoritesButton->setCheckable(true);
    connect(showFavoritesButton, &QPushButton::clicked, this, &SnapshotManagerWidget::onShowFavorites);
    viewButtonsLayout->addWidget(showFavoritesButton);
    
    viewButtonsLayout->addStretch();
    mainLayout->addLayout(viewButtonsLayout);
    
    snapshotListView = new QListView(this);
    snapshotListView->setModel(snapshotModel);
    snapshotListView->setItemDelegate(iconDelegate);
    snapshotListView->setViewMode(QListView::IconMode);
    snapshotListView->setIconSize(QSize(64, 64));
    snapshotListView->setGridSize(QSize(140, 160));
    snapshotListView->setResizeMode(QListView::Adjust);
    snapshotListView->setSpacing(15);
    snapshotListView->setContextMenuPolicy(Qt::CustomContextMenu);
    snapshotListView->setStyleSheet("QListView { border: 2px solid #e8ecf1; border-radius: 10px; padding: 12px; background-color: white; }");
    connect(snapshotListView, &QListView::doubleClicked, this, &SnapshotManagerWidget::onSnapshotItemDoubleClicked);
    connect(snapshotListView, &QListView::customContextMenuRequested, this, &SnapshotManagerWidget::onShowContextMenu);
    mainLayout->addWidget(snapshotListView, 1);
    
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(10);
    
    QPushButton *addFolderBtn = new QPushButton("添加文件夹", this);
    addFolderBtn->setIcon(QApplication::style()->standardIcon(QStyle::SP_DirIcon));
    addFolderBtn->setStyleSheet("QPushButton { background-color: #fd79a8; color: white; padding: 10px 18px; border-radius: 8px; font-weight: bold; font-size: 13px; } QPushButton:hover { background-color: #f06795; }");
    connect(addFolderBtn, &QPushButton::clicked, this, &SnapshotManagerWidget::onAddFolderSnapshot);
    buttonLayout->addWidget(addFolderBtn);
    
    QPushButton *addWebsiteBtn = new QPushButton("添加网站", this);
    addWebsiteBtn->setIcon(QApplication::style()->standardIcon(QStyle::SP_FileDialogDetailedView));
    addWebsiteBtn->setStyleSheet("QPushButton { background-color: #0984e3; color: white; padding: 10px 18px; border-radius: 8px; font-weight: bold; font-size: 13px; } QPushButton:hover { background-color: #0770c4; }");
    connect(addWebsiteBtn, &QPushButton::clicked, this, &SnapshotManagerWidget::onAddWebsiteSnapshot);
    buttonLayout->addWidget(addWebsiteBtn);
    
    QPushButton *addDocumentBtn = new QPushButton("添加文档", this);
    addDocumentBtn->setIcon(QApplication::style()->standardIcon(QStyle::SP_FileIcon));
    addDocumentBtn->setStyleSheet("QPushButton { background-color: #6c5ce7; color: white; padding: 10px 18px; border-radius: 8px; font-weight: bold; font-size: 13px; } QPushButton:hover { background-color: #5f4fd6; }");
    connect(addDocumentBtn, &QPushButton::clicked, this, &SnapshotManagerWidget::onAddDocumentSnapshot);
    buttonLayout->addWidget(addDocumentBtn);
    
    buttonLayout->addStretch();
    
    QPushButton *openBtn = new QPushButton("打开", this);
    openBtn->setIcon(QApplication::style()->standardIcon(QStyle::SP_MediaPlay));
    openBtn->setStyleSheet("QPushButton { background-color: #00b894; color: white; padding: 10px 18px; border-radius: 8px; font-weight: bold; font-size: 13px; } QPushButton:hover { background-color: #00a085; }");
    connect(openBtn, &QPushButton::clicked, this, &SnapshotManagerWidget::onOpenSnapshot);
    buttonLayout->addWidget(openBtn);
    
    QPushButton *deleteBtn = new QPushButton("删除", this);
    deleteBtn->setIcon(QApplication::style()->standardIcon(QStyle::SP_TrashIcon));
    deleteBtn->setStyleSheet("QPushButton { background-color: #d63031; color: white; padding: 10px 18px; border-radius: 8px; font-weight: bold; font-size: 13px; } QPushButton:hover { background-color: #c0292a; }");
    connect(deleteBtn, &QPushButton::clicked, this, &SnapshotManagerWidget::onDeleteSnapshot);
    buttonLayout->addWidget(deleteBtn);
    
    mainLayout->addLayout(buttonLayout);
    
    statusLabel = new QLabel("准备就绪", this);
    statusLabel->setStyleSheet("font-size: 13px; color: #636e72;");
    mainLayout->addWidget(statusLabel);
    
    this->setStyleSheet("QWidget { background-color: #f1f2f6; }");
}

void SnapshotManagerWidget::refreshSnapshotList()
{
    snapshotModel->clear();
    
    QList<SnapshotInfo> snapshots;
    
    if (showingFavorites) {
        snapshots = db->getFavoriteSnapshots();
    } else {
        int typeFilter = typeFilterCombo->currentData().toInt();
        if (typeFilter == -1) {
            snapshots = db->getAllSnapshots();
        } else {
            snapshots = db->getSnapshotsByType(static_cast<SnapshotType>(typeFilter));
        }
    }
    
    QString searchText = searchEdit->text().trimmed();
    QList<SnapshotInfo> filteredSnapshots;
    
    if (searchText.isEmpty()) {
        filteredSnapshots = snapshots;
    } else {
        for (const SnapshotInfo &snapshot : snapshots) {
            if (snapshot.name.toLower().contains(searchText.toLower()) ||
                snapshot.description.toLower().contains(searchText.toLower()) ||
                snapshot.tags.toLower().contains(searchText.toLower()) ||
                snapshot.path.toLower().contains(searchText.toLower())) {
                filteredSnapshots.append(snapshot);
            }
        }
    }
    
    for (const SnapshotInfo &snapshot : filteredSnapshots) {
        QStandardItem *item = new QStandardItem(snapshot.name);
        item->setData(snapshot.id, Qt::UserRole);
        item->setIcon(getSnapshotIcon(snapshot));
        
        if (snapshot.isFavorite) {
            item->setBackground(QColor(255, 249, 196));
        }
        
        snapshotModel->appendRow(item);
    }
    
    statusLabel->setText(QString("共 %1 个快照").arg(snapshotModel->rowCount()));
}

QIcon SnapshotManagerWidget::getSnapshotIcon(const SnapshotInfo &snapshot)
{
    if (snapshot.type == SnapshotType_Folder) {
        return QApplication::style()->standardIcon(QStyle::SP_DirIcon);
    } else if (snapshot.type == SnapshotType_Website) {
        return QApplication::style()->standardIcon(QStyle::SP_FileDialogDetailedView);
    } else if (snapshot.type == SnapshotType_Document) {
        if (QFile::exists(snapshot.path)) {
            QFileInfo fileInfo(snapshot.path);
            QFileIconProvider iconProvider;
            return iconProvider.icon(fileInfo);
        }
        return QApplication::style()->standardIcon(QStyle::SP_FileIcon);
    }
    return QApplication::style()->standardIcon(QStyle::SP_FileIcon);
}

QString SnapshotManagerWidget::formatSize(qint64 size)
{
    const qint64 KB = 1024;
    const qint64 MB = KB * 1024;
    const qint64 GB = MB * 1024;
    
    if (size >= GB) {
        return QString::number(size / (double)GB, 'f', 2) + " GB";
    } else if (size >= MB) {
        return QString::number(size / (double)MB, 'f', 2) + " MB";
    } else if (size >= KB) {
        return QString::number(size / (double)KB, 'f', 2) + " KB";
    } else {
        return QString::number(size) + " B";
    }
}

void SnapshotManagerWidget::onAddFolderSnapshot()
{
    QString folderPath = QFileDialog::getExistingDirectory(this, "选择文件夹", "", 
                                                          QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (folderPath.isEmpty()) return;
    
    QFileInfo fileInfo(folderPath);
    QString folderName = fileInfo.fileName();
    if (folderName.isEmpty()) folderName = folderPath;
    
    bool ok;
    QString name = QInputDialog::getText(this, "文件夹快照", "请输入快照名称:", QLineEdit::Normal, folderName, &ok);
    if (!ok || name.trimmed().isEmpty()) return;
    
    QDir dir(folderPath);
    QStringList structure;
    QDirIterator it(folderPath, QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    
    int fileCount = 0;
    qint64 totalSize = 0;
    QMap<QString, int> typeCount;
    
    while (it.hasNext()) {
        it.next();
        QFileInfo info = it.fileInfo();
        if (info.isFile()) {
            fileCount++;
            totalSize += info.size();
            QString ext = info.suffix().toLower();
            typeCount[ext]++;
        }
    }
    
    QString typeDist;
    QTextStream ts(&typeDist);
    for (auto it = typeCount.begin(); it != typeCount.end(); ++it) {
        ts << it.key() << ": " << it.value() << ", ";
    }
    typeDist.chop(2);
    
    SnapshotInfo snapshot;
    snapshot.name = name.trimmed();
    snapshot.path = folderPath;
    snapshot.description = QString("文件夹: %1\n文件数: %2\n总大小: %3").arg(folderPath).arg(fileCount).arg(formatSize(totalSize));
    snapshot.type = SnapshotType_Folder;
    snapshot.createdTime = QDateTime::currentDateTime();
    snapshot.lastAccessedTime = QDateTime::currentDateTime();
    snapshot.fileCount = fileCount;
    snapshot.totalSize = totalSize;
    snapshot.fileTypeDistribution = typeDist;
    snapshot.isFavorite = false;
    
    int maxOrder = 0;
    QList<SnapshotInfo> existing = db->getAllSnapshots();
    for (const SnapshotInfo &s : existing) {
        if (s.sortOrder > maxOrder) maxOrder = s.sortOrder;
    }
    snapshot.sortOrder = maxOrder + 1;
    
    if (db->addSnapshot(snapshot)) {
        QMessageBox::information(this, "成功", "文件夹快照创建成功！");
        refreshSnapshotList();
    } else {
        QMessageBox::warning(this, "错误", "创建快照失败！");
    }
}

void SnapshotManagerWidget::onAddWebsiteSnapshot()
{
    bool ok;
    QString url = QInputDialog::getText(this, "网站快照", "请输入网站URL（例如：https://www.baidu.com）：", 
                                         QLineEdit::Normal, "https://", &ok);
    if (!ok || url.trimmed().isEmpty()) return;
    
    QString trimmedUrl = url.trimmed();
    if (!trimmedUrl.startsWith("http://") && !trimmedUrl.startsWith("https://")) {
        trimmedUrl = "https://" + trimmedUrl;
    }
    
    bool ok2;
    QString name = QInputDialog::getText(this, "网站快照", "请输入快照名称:", QLineEdit::Normal, trimmedUrl, &ok2);
    if (!ok2 || name.trimmed().isEmpty()) {
        name = trimmedUrl;
    }
    
    SnapshotInfo snapshot;
    snapshot.name = name.trimmed();
    snapshot.path = trimmedUrl;
    snapshot.description = QString("网站: %1").arg(trimmedUrl);
    snapshot.type = SnapshotType_Website;
    snapshot.websiteTitle = snapshot.name;
    snapshot.websiteUrl = trimmedUrl;
    snapshot.createdTime = QDateTime::currentDateTime();
    snapshot.lastAccessedTime = QDateTime::currentDateTime();
    snapshot.isFavorite = false;
    
    int maxOrder = 0;
    QList<SnapshotInfo> existing = db->getAllSnapshots();
    for (const SnapshotInfo &s : existing) {
        if (s.sortOrder > maxOrder) maxOrder = s.sortOrder;
    }
    snapshot.sortOrder = maxOrder + 1;
    
    if (db->addSnapshot(snapshot)) {
        QMessageBox::information(this, "成功", "网站快照创建成功！");
        refreshSnapshotList();
    } else {
        QMessageBox::warning(this, "错误", "创建快照失败！");
    }
}

void SnapshotManagerWidget::onAddDocumentSnapshot()
{
    QString filePath = QFileDialog::getOpenFileName(this, "选择文档文件", "", 
                                                    "文档文件 (*.docx *.doc *.pdf *.txt *.xlsx *.xls *.pptx *.ppt);;所有文件 (*.*)");
    if (filePath.isEmpty()) return;
    
    QFileInfo fileInfo(filePath);
    QString docName = fileInfo.completeBaseName();
    
    bool ok;
    QString name = QInputDialog::getText(this, "文档快照", "请输入快照名称:", QLineEdit::Normal, docName, &ok);
    if (!ok || name.trimmed().isEmpty()) return;
    
    SnapshotInfo snapshot;
    snapshot.name = name.trimmed();
    snapshot.path = filePath;
    snapshot.description = QString("文档: %1\n大小: %2").arg(filePath).arg(formatSize(fileInfo.size()));
    snapshot.type = SnapshotType_Document;
    snapshot.documentTitle = docName;
    snapshot.documentModifiedDate = fileInfo.lastModified().toString("yyyy-MM-dd hh:mm:ss");
    snapshot.createdTime = QDateTime::currentDateTime();
    snapshot.lastAccessedTime = QDateTime::currentDateTime();
    snapshot.fileCount = 1;
    snapshot.totalSize = fileInfo.size();
    snapshot.isFavorite = false;
    
    int maxOrder = 0;
    QList<SnapshotInfo> existing = db->getAllSnapshots();
    for (const SnapshotInfo &s : existing) {
        if (s.sortOrder > maxOrder) maxOrder = s.sortOrder;
    }
    snapshot.sortOrder = maxOrder + 1;
    
    if (db->addSnapshot(snapshot)) {
        QMessageBox::information(this, "成功", "文档快照创建成功！");
        refreshSnapshotList();
    } else {
        QMessageBox::warning(this, "错误", "创建快照失败！");
    }
}

void SnapshotManagerWidget::onDeleteSnapshot()
{
    QModelIndexList selected = snapshotListView->selectionModel()->selectedIndexes();
    if (selected.isEmpty()) {
        QMessageBox::warning(this, "提示", "请先选择要删除的快照");
        return;
    }
    
    auto reply = QMessageBox::question(this, "确认删除", "确定要删除选中的快照吗？",
                                       QMessageBox::Yes | QMessageBox::No);
    if (reply != QMessageBox::Yes) return;
    
    QList<int> idsToDelete;
    for (const QModelIndex &index : selected) {
        QStandardItem *item = snapshotModel->itemFromIndex(index);
        if (item) {
            idsToDelete.append(item->data(Qt::UserRole).toInt());
        }
    }
    
    for (int id : idsToDelete) {
        db->deleteSnapshot(id);
    }
    
    refreshSnapshotList();
}

void SnapshotManagerWidget::onOpenSnapshot()
{
    QModelIndexList selected = snapshotListView->selectionModel()->selectedIndexes();
    if (selected.isEmpty()) {
        QMessageBox::warning(this, "提示", "请先选择要打开的快照");
        return;
    }
    
    for (const QModelIndex &index : selected) {
        QStandardItem *item = snapshotModel->itemFromIndex(index);
        if (item) {
            int snapshotId = item->data(Qt::UserRole).toInt();
            SnapshotInfo snapshot = db->getSnapshotById(snapshotId);
            if (snapshot.id > 0) {
                openSnapshot(snapshot);
            }
        }
    }
}

void SnapshotManagerWidget::openSnapshot(const SnapshotInfo &snapshot)
{
    if (snapshot.type == SnapshotType_Folder) {
        QDesktopServices::openUrl(QUrl::fromLocalFile(snapshot.path));
    } else if (snapshot.type == SnapshotType_Website) {
        QDesktopServices::openUrl(QUrl(snapshot.path));
    } else if (snapshot.type == SnapshotType_Document) {
        QDesktopServices::openUrl(QUrl::fromLocalFile(snapshot.path));
    }
    
    SnapshotInfo updated = snapshot;
    updated.lastAccessedTime = QDateTime::currentDateTime();
    db->updateSnapshot(updated);
}

void SnapshotManagerWidget::onSnapshotItemDoubleClicked(const QModelIndex &index)
{
    if (!index.isValid()) return;
    
    QStandardItem *item = snapshotModel->itemFromIndex(index);
    if (!item) return;
    
    int snapshotId = item->data(Qt::UserRole).toInt();
    SnapshotInfo snapshot = db->getSnapshotById(snapshotId);
    if (snapshot.id > 0) {
        openSnapshot(snapshot);
    }
}

void SnapshotManagerWidget::onShowContextMenu(const QPoint &pos)
{
    QModelIndex index = snapshotListView->indexAt(pos);
    if (!index.isValid()) return;
    
    QMenu menu(this);
    menu.setStyleSheet("QMenu { background-color: white; border: 2px solid #e8ecf1; border-radius: 10px; padding: 8px; } QMenu::item { padding: 10px 20px; border-radius: 6px; } QMenu::item:selected { background-color: #0984e3; color: white; }");
    
    QAction *openAction = menu.addAction("打开");
    QAction *renameAction = menu.addAction("重命名");
    QAction *detailsAction = menu.addAction("详细信息");
    QAction *favoriteAction = menu.addAction("收藏/取消收藏");
    menu.addSeparator();
    QAction *deleteAction = menu.addAction("删除");
    
    QAction *selected = menu.exec(snapshotListView->mapToGlobal(pos));
    
    if (selected == openAction) {
        onOpenSnapshot();
    } else if (selected == renameAction) {
        onRenameSnapshot();
    } else if (selected == detailsAction) {
        onSnapshotDetails();
    } else if (selected == favoriteAction) {
        onToggleFavorite();
    } else if (selected == deleteAction) {
        onDeleteSnapshot();
    }
}

void SnapshotManagerWidget::onRenameSnapshot()
{
    QModelIndex index = snapshotListView->currentIndex();
    if (!index.isValid()) return;
    
    QStandardItem *item = snapshotModel->itemFromIndex(index);
    if (!item) return;
    
    int snapshotId = item->data(Qt::UserRole).toInt();
    SnapshotInfo snapshot = db->getSnapshotById(snapshotId);
    if (snapshot.id <= 0) return;
    
    bool ok;
    QString newName = QInputDialog::getText(this, "重命名快照", "新名称:", QLineEdit::Normal, snapshot.name, &ok);
    if (ok && !newName.trimmed().isEmpty()) {
        snapshot.name = newName.trimmed();
        db->updateSnapshot(snapshot);
        refreshSnapshotList();
    }
}

void SnapshotManagerWidget::onToggleFavorite()
{
    QModelIndex index = snapshotListView->currentIndex();
    if (!index.isValid()) return;
    
    QStandardItem *item = snapshotModel->itemFromIndex(index);
    if (!item) return;
    
    int snapshotId = item->data(Qt::UserRole).toInt();
    SnapshotInfo snapshot = db->getSnapshotById(snapshotId);
    if (snapshot.id <= 0) return;
    
    snapshot.isFavorite = !snapshot.isFavorite;
    db->updateSnapshot(snapshot);
    refreshSnapshotList();
}

void SnapshotManagerWidget::onSearchSnapshot(const QString &text)
{
    refreshSnapshotList();
}

void SnapshotManagerWidget::onFilterByType(int index)
{
    Q_UNUSED(index);
    refreshSnapshotList();
}

void SnapshotManagerWidget::onShowAll()
{
    showingFavorites = false;
    showAllButton->setChecked(true);
    showFavoritesButton->setChecked(false);
    refreshSnapshotList();
}

void SnapshotManagerWidget::onShowFavorites()
{
    showingFavorites = true;
    showAllButton->setChecked(false);
    showFavoritesButton->setChecked(true);
    refreshSnapshotList();
}

void SnapshotManagerWidget::onSnapshotDetails()
{
    QModelIndex index = snapshotListView->currentIndex();
    if (!index.isValid()) return;
    
    QStandardItem *item = snapshotModel->itemFromIndex(index);
    if (!item) return;
    
    int snapshotId = item->data(Qt::UserRole).toInt();
    SnapshotInfo snapshot = db->getSnapshotById(snapshotId);
    if (snapshot.id <= 0) return;
    
    QString details;
    QTextStream ts(&details);
    ts << "名称: " << snapshot.name << "\n";
    ts << "路径: " << snapshot.path << "\n";
    ts << "描述: " << snapshot.description << "\n";
    ts << "类型: ";
    if (snapshot.type == SnapshotType_Folder) {
        ts << "文件夹\n";
        ts << "文件数: " << snapshot.fileCount << "\n";
        ts << "总大小: " << formatSize(snapshot.totalSize) << "\n";
        ts << "文件类型分布: " << snapshot.fileTypeDistribution << "\n";
    } else if (snapshot.type == SnapshotType_Website) {
        ts << "网站\n";
        ts << "网站标题: " << snapshot.websiteTitle << "\n";
        ts << "网站URL: " << snapshot.websiteUrl << "\n";
    } else if (snapshot.type == SnapshotType_Document) {
        ts << "文档\n";
        ts << "文档标题: " << snapshot.documentTitle << "\n";
        ts << "作者: " << snapshot.documentAuthor << "\n";
        ts << "修改日期: " << snapshot.documentModifiedDate << "\n";
        ts << "文件大小: " << formatSize(snapshot.totalSize) << "\n";
    }
    ts << "创建时间: " << snapshot.createdTime.toString("yyyy-MM-dd hh:mm:ss") << "\n";
    ts << "最后访问: " << snapshot.lastAccessedTime.toString("yyyy-MM-dd hh:mm:ss") << "\n";
    ts << "标签: " << snapshot.tags << "\n";
    ts << "收藏: " << (snapshot.isFavorite ? "是" : "否") << "\n";
    
    QMessageBox::information(this, "快照详细信息", details);
}

void SnapshotManagerWidget::saveSnapshotOrder()
{
    for (int i = 0; i < snapshotModel->rowCount(); ++i) {
        QStandardItem *item = snapshotModel->item(i);
        int snapshotId = item->data(Qt::UserRole).toInt();
        SnapshotInfo snapshot = db->getSnapshotById(snapshotId);
        snapshot.sortOrder = i;
        db->updateSnapshot(snapshot);
    }
}
