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
#include <QFile>
#include <QTextStream>
#include <QFileDialog>
#include <QGraphicsDropShadowEffect>

void drawProfessionalIconBackground(QPainter *painter, const QRect &rect, const QColor &color, bool isHovered, bool isSelected)
{
    painter->save();
    
    QColor baseColor = color;
    if (isSelected) {
        baseColor = baseColor.lighter(115);
    } else if (isHovered) {
        baseColor = baseColor.lighter(108);
    }
    
    QLinearGradient gradient(rect.topLeft(), rect.bottomRight());
    gradient.setColorAt(0, baseColor.lighter(125));
    gradient.setColorAt(0.5, baseColor);
    gradient.setColorAt(1, baseColor.darker(110));
    
    painter->setPen(Qt::NoPen);
    painter->setBrush(gradient);
    painter->drawRoundedRect(rect, 12, 12);
    
    QPainterPath path;
    path.addRoundedRect(rect.adjusted(1, 1, -1, -1), 11, 11);
    painter->setClipPath(path);
    
    QLinearGradient highlightGrad(rect.topLeft(), rect.bottomLeft());
    highlightGrad.setColorAt(0, QColor(255, 255, 255, 60));
    highlightGrad.setColorAt(0.5, QColor(255, 255, 255, 20));
    highlightGrad.setColorAt(1, QColor(255, 255, 255, 0));
    painter->setBrush(highlightGrad);
    painter->drawRect(rect);
    
    painter->restore();
}

void drawModernWorkIcon(QPainter *painter, const QRect &rect)
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    
    QColor white(255, 255, 255);
    painter->setPen(white);
    painter->setBrush(white);
    
    QRect mainRect = rect.adjusted(10, 12, -10, -8);
    
    painter->drawRoundedRect(mainRect, 4, 4);
    
    QRect handleRect = mainRect.adjusted(mainRect.width() / 3, -5, -mainRect.width() / 3, 0);
    handleRect.setHeight(10);
    handleRect.moveTop(mainRect.top() - 7);
    painter->drawRoundedRect(handleRect, 3, 3);
    
    painter->setPen(QColor(255, 255, 255, 200));
    painter->setBrush(Qt::NoBrush);
    painter->drawLine(mainRect.left() + 6, mainRect.top() + 12, mainRect.right() - 6, mainRect.top() + 12);
    painter->drawLine(mainRect.left() + 6, mainRect.top() + 20, mainRect.right() - 12, mainRect.top() + 20);
    
    painter->restore();
}

void drawModernEntertainmentIcon(QPainter *painter, const QRect &rect)
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    
    QColor white(255, 255, 255);
    painter->setPen(white);
    painter->setBrush(white);
    
    QPoint center = rect.center();
    
    painter->drawEllipse(center.x() - 14, center.y(), 10, 10);
    
    painter->drawEllipse(center.x() + 4, center.y() - 8, 6, 6);
    painter->drawEllipse(center.x() + 10, center.y(), 6, 6);
    
    painter->drawRoundedRect(center.x() - 16, center.y() - 6, 32, 12, 4, 4);
    
    painter->restore();
}

void drawModernThinkingIcon(QPainter *painter, const QRect &rect)
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    
    QColor white(255, 255, 255);
    painter->setPen(white);
    painter->setBrush(white);
    
    QPoint center = rect.center();
    
    painter->drawEllipse(center.x(), center.y() - 2, 14, 16);
    
    QRect baseRect(center.x() - 4, center.y() + 10, 8, 6);
    painter->drawRoundedRect(baseRect, 1, 1);
    
    painter->setPen(QPen(white, 2));
    painter->drawLine(center.x(), center.y() - 20, center.x(), center.y() - 16);
    painter->drawLine(center.x() - 7, center.y() - 17, center.x() - 5, center.y() - 14);
    painter->drawLine(center.x() + 7, center.y() - 17, center.x() + 5, center.y() - 14);
    
    painter->restore();
}

void drawModernLearningIcon(QPainter *painter, const QRect &rect)
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    
    QColor white(255, 255, 255);
    painter->setPen(white);
    painter->setBrush(white);
    
    QRect bookRect = rect.adjusted(8, 10, -8, -10);
    
    painter->drawRoundedRect(bookRect, 3, 3);
    
    painter->setPen(QColor(255, 255, 255, 180));
    painter->drawLine(bookRect.center().x(), bookRect.top(), bookRect.center().x(), bookRect.bottom());
    painter->drawLine(bookRect.left() + 5, bookRect.top() + 6, bookRect.left() + 5, bookRect.bottom() - 6);
    painter->drawLine(bookRect.right() - 5, bookRect.top() + 6, bookRect.right() - 5, bookRect.bottom() - 6);
    
    painter->setPen(white);
    painter->setBrush(Qt::NoBrush);
    for (int i = 0; i < 3; i++) {
        int y = bookRect.top() + 10 + i * 7;
        painter->drawLine(bookRect.left() + 8, y, bookRect.center().x() - 3, y);
        painter->drawLine(bookRect.center().x() + 3, y, bookRect.right() - 8, y);
    }
    
    painter->restore();
}

void drawModernDefaultIcon(QPainter *painter, const QRect &rect)
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    
    QColor white(255, 255, 255);
    painter->setPen(white);
    painter->setBrush(white);
    
    QRect iconRect = rect.adjusted(10, 10, -10, -10);
    int cellSize = (iconRect.width() - 4) / 3;
    
    for (int row = 0; row < 3; row++) {
        for (int col = 0; col < 3; col++) {
            QRect cell(iconRect.left() + col * cellSize + 2, 
                       iconRect.top() + row * cellSize + 2, 
                       cellSize - 4, cellSize - 4);
            if ((row + col) % 2 == 0) {
                painter->drawRoundedRect(cell, 2, 2);
            }
        }
    }
    
    painter->restore();
}

CollectionItemDelegate::CollectionItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
    tagColors["工作"] = {"工作", "#e74c3c", "工作", "briefcase"};
    tagColors["娱乐"] = {"娱乐", "#27ae60", "娱乐", "gamepad"};
    tagColors["思考"] = {"思考", "#f39c12", "思考", "bulb"};
    tagColors["学习"] = {"学习", "#3498db", "学习", "book"};
    tagColors["未分类"] = {"未分类", "#95a5a6", "未分类", "grid"};
}

void CollectionItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setRenderHint(QPainter::TextAntialiasing, true);
    
    bool isSelected = option.state & QStyle::State_Selected;
    bool isHovered = option.state & QStyle::State_MouseOver;
    
    QRect itemRect = option.rect.adjusted(6, 6, -6, -6);
    
    if (isSelected) {
        QLinearGradient bgGrad(itemRect.topLeft(), itemRect.bottomLeft());
        bgGrad.setColorAt(0, QColor(156, 39, 176, 30));
        bgGrad.setColorAt(1, QColor(156, 39, 176, 18));
        painter->fillRect(itemRect, bgGrad);
        
        painter->setPen(QColor(156, 39, 176, 120));
        painter->setBrush(Qt::NoBrush);
        painter->drawRoundedRect(itemRect.adjusted(1, 1, -1, -1), 12, 12);
    } else if (isHovered) {
        QLinearGradient bgGrad(itemRect.topLeft(), itemRect.bottomLeft());
        bgGrad.setColorAt(0, QColor(0, 0, 0, 10));
        bgGrad.setColorAt(1, QColor(0, 0, 0, 4));
        painter->fillRect(itemRect, bgGrad);
    }
    
    int collectionId = index.data(Qt::UserRole).toInt();
    AppCollection col;
    for (const auto &c : collections) {
        if (c.id == collectionId) {
            col = c;
            break;
        }
    }
    
    int iconSize = 52;
    QRect iconRect = QRect(itemRect.left() + 14, itemRect.top() + (itemRect.height() - iconSize) / 2, iconSize, iconSize);
    
    QRect contentRect = itemRect.adjusted(iconRect.right() + 16, 10, -14, -10);
    
    QString effectiveTag = col.tag;
    if (effectiveTag == "上班") {
        effectiveTag = "工作";
    }
    
    TagInfo tagInfo = tagColors.value(effectiveTag, tagColors["未分类"]);
    drawProfessionalIconBackground(painter, iconRect, QColor(tagInfo.color), isHovered, isSelected);
    
    if (tagInfo.iconSymbol == "briefcase") {
        drawModernWorkIcon(painter, iconRect);
    } else if (tagInfo.iconSymbol == "gamepad") {
        drawModernEntertainmentIcon(painter, iconRect);
    } else if (tagInfo.iconSymbol == "bulb") {
        drawModernThinkingIcon(painter, iconRect);
    } else if (tagInfo.iconSymbol == "book") {
        drawModernLearningIcon(painter, iconRect);
    } else {
        drawModernDefaultIcon(painter, iconRect);
    }
    
    painter->setPen(QColor(44, 62, 80));
    QFont titleFont("Microsoft YaHei", 12, QFont::Bold);
    painter->setFont(titleFont);
    QString elidedName = painter->fontMetrics().elidedText(col.name, Qt::ElideRight, contentRect.width() - 80);
    QRect titleRect = QRect(contentRect.left(), contentRect.top(), contentRect.width() - 80, painter->fontMetrics().height());
    painter->drawText(titleRect, Qt::AlignLeft | Qt::AlignVCenter, elidedName);
    
    if (!col.description.isEmpty()) {
        painter->setPen(QColor(149, 165, 166));
        QFont descFont("Microsoft YaHei", 10);
        painter->setFont(descFont);
        QString elidedDesc = painter->fontMetrics().elidedText(col.description, Qt::ElideRight, contentRect.width() - 10);
        QRect descRect = QRect(contentRect.left(), contentRect.top() + painter->fontMetrics().height() + 4, contentRect.width() - 10, painter->fontMetrics().height());
        painter->drawText(descRect, Qt::AlignLeft | Qt::AlignVCenter, elidedDesc);
    }
    
    if (isHovered || isSelected) {
        QRect tagBadgeRect = QRect(itemRect.right() - 74, itemRect.top() + (itemRect.height() - 26) / 2, 64, 26);
        
        painter->setPen(Qt::NoPen);
        QColor badgeColor(tagInfo.color);
        QLinearGradient badgeGrad(tagBadgeRect.topLeft(), tagBadgeRect.bottomRight());
        badgeGrad.setColorAt(0, badgeColor.lighter(118));
        badgeGrad.setColorAt(1, badgeColor);
        painter->setBrush(badgeGrad);
        painter->drawRoundedRect(tagBadgeRect, 13, 13);
        
        painter->setPen(QColor(255, 255, 255));
        QFont badgeFont("Microsoft YaHei", 9, QFont::Bold);
        painter->setFont(badgeFont);
        painter->drawText(tagBadgeRect, Qt::AlignCenter, tagInfo.displayName);
    }
    
    painter->restore();
}

QSize CollectionItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return QSize(290, 90);
}

CollectionManagerWidget::CollectionManagerWidget(Database *db, QWidget *parent)
    : QWidget(parent), db(db), currentCollectionId(-1)
{
    initTagColors();
    iconDelegate = new AppIconDelegate(this);
    collectionDelegate = new CollectionItemDelegate(this);
    appsModel = new QStandardItemModel(this);
    
    setupUI();
    refreshCollectionList();
}

CollectionManagerWidget::~CollectionManagerWidget()
{
}

void CollectionManagerWidget::selectFirstCollection()
{
    if (collectionListWidget->count() > 0 && currentCollectionId <= 0) {
        collectionListWidget->setCurrentRow(0);
        onCollectionSelected(collectionListWidget->item(0));
    }
}

void CollectionManagerWidget::initTagColors()
{
    tagColors["工作"] = {"工作", "#e74c3c", "工作", "briefcase"};
    tagColors["娱乐"] = {"娱乐", "#27ae60", "娱乐", "gamepad"};
    tagColors["思考"] = {"思考", "#f39c12", "思考", "bulb"};
    tagColors["学习"] = {"学习", "#3498db", "学习", "book"};
    tagColors["未分类"] = {"未分类", "#95a5a6", "未分类", "grid"};
}

void CollectionManagerWidget::setupUI()
{
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(16, 16, 16, 16);
    mainLayout->setSpacing(16);
    
    QWidget *leftWidget = new QWidget(this);
    leftWidget->setStyleSheet("background-color: #fafbfc; border-radius: 12px;");
    QVBoxLayout *leftLayout = new QVBoxLayout(leftWidget);
    leftLayout->setContentsMargins(14, 14, 14, 14);
    leftLayout->setSpacing(12);
    
    QLabel *collectionsLabel = new QLabel("应用集合", this);
    collectionsLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #2d3436; padding: 4px 0;");
    leftLayout->addWidget(collectionsLabel);
    
    collectionListWidget = new QListWidget(this);
    collectionListWidget->setStyleSheet(
        "QListWidget { border: none; background-color: transparent; padding: 0; } "
        "QListWidget::item { margin: 0; padding: 0; }"
    );
    collectionListWidget->setItemDelegate(collectionDelegate);
    collectionListWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    collectionListWidget->setSpacing(4);
    connect(collectionListWidget, &QListWidget::itemClicked, this, &CollectionManagerWidget::onCollectionSelected);
    connect(collectionListWidget, &QListWidget::customContextMenuRequested, this, &CollectionManagerWidget::onShowCollectionContextMenu);
    leftLayout->addWidget(collectionListWidget);
    
    QHBoxLayout *leftButtonLayout = new QHBoxLayout();
    leftButtonLayout->setSpacing(8);
    
    createCollectionButton = new QPushButton("新建集合", this);
    createCollectionButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_FileDialogNewFolder));
    createCollectionButton->setStyleSheet(
        "QPushButton { background-color: #00b894; color: white; padding: 10px 18px; border-radius: 8px; font-weight: bold; font-size: 13px; } "
        "QPushButton:hover { background-color: #00a085; } "
        "QPushButton:pressed { background-color: #008970; }"
    );
    connect(createCollectionButton, &QPushButton::clicked, this, &CollectionManagerWidget::onCreateCollection);
    leftButtonLayout->addWidget(createCollectionButton);
    
    editCollectionButton = new QPushButton("属性", this);
    editCollectionButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_FileDialogContentsView));
    editCollectionButton->setStyleSheet(
        "QPushButton { background-color: #0984e3; color: white; padding: 10px 18px; border-radius: 8px; font-weight: bold; font-size: 13px; } "
        "QPushButton:hover { background-color: #0770c4; } "
        "QPushButton:pressed { background-color: #065da6; }"
    );
    connect(editCollectionButton, &QPushButton::clicked, this, &CollectionManagerWidget::onEditCollection);
    leftButtonLayout->addWidget(editCollectionButton);
    
    deleteCollectionButton = new QPushButton("删除", this);
    deleteCollectionButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_TrashIcon));
    deleteCollectionButton->setStyleSheet(
        "QPushButton { background-color: #d63031; color: white; padding: 10px 18px; border-radius: 8px; font-weight: bold; font-size: 13px; } "
        "QPushButton:hover { background-color: #c0292a; } "
        "QPushButton:pressed { background-color: #a92425; }"
    );
    connect(deleteCollectionButton, &QPushButton::clicked, this, &CollectionManagerWidget::onDeleteCollection);
    leftButtonLayout->addWidget(deleteCollectionButton);
    
    leftLayout->addLayout(leftButtonLayout);
    leftWidget->setLayout(leftLayout);
    leftWidget->setMaximumWidth(340);
    
    QWidget *rightWidget = new QWidget(this);
    rightWidget->setStyleSheet("background-color: #fafbfc; border-radius: 12px;");
    QVBoxLayout *rightLayout = new QVBoxLayout(rightWidget);
    rightLayout->setContentsMargins(16, 16, 16, 16);
    rightLayout->setSpacing(14);
    
    QGroupBox *infoGroup = new QGroupBox("集合信息", this);
    infoGroup->setStyleSheet("QGroupBox { font-weight: bold; font-size: 14px; color: #2d3436; border: 2px solid #e8ecf1; border-radius: 10px; margin-top: 10px; padding-top: 12px; } QGroupBox::title { subcontrol-origin: margin; left: 12px; padding: 0 6px; }");
    QVBoxLayout *infoLayout = new QVBoxLayout(infoGroup);
    infoLayout->setSpacing(8);
    infoLayout->setContentsMargins(12, 16, 12, 12);
    
    collectionNameLabel = new QLabel("请选择一个应用集合", this);
    collectionNameLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #2d3436; padding: 2px 0;");
    infoLayout->addWidget(collectionNameLabel);
    
    collectionTagLabel = new QLabel("", this);
    collectionTagLabel->setStyleSheet("font-size: 12px; color: #636e72; padding: 2px 0;");
    infoLayout->addWidget(collectionTagLabel);
    
    collectionDescLabel = new QLabel("", this);
    collectionDescLabel->setStyleSheet("font-size: 13px; color: #636e72; line-height: 1.6;");
    collectionDescLabel->setWordWrap(true);
    infoLayout->addWidget(collectionDescLabel);
    
    rightLayout->addWidget(infoGroup);
    
    QGroupBox *appsGroup = new QGroupBox("包含的应用", this);
    appsGroup->setStyleSheet("QGroupBox { font-weight: bold; font-size: 14px; color: #2d3436; border: 2px solid #e8ecf1; border-radius: 10px; margin-top: 10px; padding-top: 12px; } QGroupBox::title { subcontrol-origin: margin; left: 12px; padding: 0 6px; }");
    QVBoxLayout *appsLayout = new QVBoxLayout(appsGroup);
    appsLayout->setSpacing(12);
    appsLayout->setContentsMargins(12, 16, 12, 12);
    
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
        "QListView { border: 2px solid #e8ecf1; border-radius: 10px; padding: 12px; background-color: white; }"
    );
    connect(appsListView, &QListView::doubleClicked, this, &CollectionManagerWidget::onAppItemDoubleClicked);
    connect(appsListView, &QListView::customContextMenuRequested, this, &CollectionManagerWidget::onShowContextMenu);
    appsLayout->addWidget(appsListView);
    
    QHBoxLayout *appsButtonLayout = new QHBoxLayout();
    appsButtonLayout->setSpacing(10);
    
    addAppButton = new QPushButton("添加应用", this);
    addAppButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_FileDialogDetailedView));
    addAppButton->setStyleSheet(
        "QPushButton { background-color: #6c5ce7; color: white; padding: 10px 18px; border-radius: 8px; font-weight: bold; font-size: 13px; } "
        "QPushButton:hover { background-color: #5f4fd6; } "
        "QPushButton:pressed { background-color: #5243c5; }"
    );
    connect(addAppButton, &QPushButton::clicked, this, &CollectionManagerWidget::onAddAppToCollection);
    appsButtonLayout->addWidget(addAppButton);
    
    removeAppButton = new QPushButton("移除应用", this);
    removeAppButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogCancelButton));
    removeAppButton->setStyleSheet(
        "QPushButton { background-color: #fd79a8; color: white; padding: 10px 18px; border-radius: 8px; font-weight: bold; font-size: 13px; } "
        "QPushButton:hover { background-color: #f06795; } "
        "QPushButton:pressed { background-color: #e35682; }"
    );
    connect(removeAppButton, &QPushButton::clicked, this, &CollectionManagerWidget::onRemoveAppFromCollection);
    appsButtonLayout->addWidget(removeAppButton);
    
    appsButtonLayout->addStretch();
    
    runCollectionButton = new QPushButton("批量运行", this);
    runCollectionButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_MediaPlay));
    runCollectionButton->setStyleSheet(
        "QPushButton { background-color: #00cec9; color: white; padding: 10px 24px; border-radius: 8px; font-weight: bold; font-size: 14px; } "
        "QPushButton:hover { background-color: #00b8b3; } "
        "QPushButton:pressed { background-color: #00a29d; }"
    );
    connect(runCollectionButton, &QPushButton::clicked, this, &CollectionManagerWidget::onRunCollection);
    appsButtonLayout->addWidget(runCollectionButton);
    
    appsLayout->addLayout(appsButtonLayout);
    
    rightLayout->addWidget(appsGroup, 1);
    rightWidget->setLayout(rightLayout);
    
    mainLayout->addWidget(leftWidget);
    mainLayout->addWidget(rightWidget, 1);
    
    this->setStyleSheet("QWidget { background-color: #f1f2f6; }");
}

void CollectionManagerWidget::refreshCollectionList()
{
    collectionListWidget->clear();
    QList<AppCollection> collections = db->getAllCollections();
    
    std::sort(collections.begin(), collections.end(), [](const AppCollection &a, const AppCollection &b) {
        return a.sortPriority < b.sortPriority;
    });
    
    collectionDelegate->setCollections(collections);
    
    for (const AppCollection &col : collections) {
        QListWidgetItem *item = new QListWidgetItem(col.name);
        item->setData(Qt::UserRole, col.id);
        collectionListWidget->addItem(item);
    }
    
    if (!collections.isEmpty()) {
        collectionListWidget->setCurrentRow(0);
        onCollectionSelected(collectionListWidget->item(0));
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

void CollectionManagerWidget::showCollectionPropertiesDialog(AppCollection &collection, bool isNew)
{
    QDialog dialog(this);
    dialog.setWindowTitle(isNew ? "新建应用集合" : "集合属性");
    dialog.setMinimumWidth(480);
    dialog.setStyleSheet("QDialog { background-color: #fafbfc; }");
    
    QFormLayout *formLayout = new QFormLayout(&dialog);
    formLayout->setSpacing(14);
    formLayout->setContentsMargins(24, 24, 24, 24);
    
    QLineEdit *nameEdit = new QLineEdit(collection.name, &dialog);
    nameEdit->setStyleSheet("QLineEdit { padding: 10px 14px; border: 2px solid #e8ecf1; border-radius: 8px; font-size: 14px; } QLineEdit:focus { border-color: #0984e3; }");
    formLayout->addRow("集合名称:", nameEdit);
    
    QTextEdit *descEdit = new QTextEdit(collection.description, &dialog);
    descEdit->setMaximumHeight(100);
    descEdit->setStyleSheet("QTextEdit { padding: 10px 14px; border: 2px solid #e8ecf1; border-radius: 8px; font-size: 14px; } QTextEdit:focus { border-color: #0984e3; }");
    formLayout->addRow("集合描述:", descEdit);
    
    QComboBox *tagCombo = new QComboBox(&dialog);
    tagCombo->setStyleSheet("QComboBox { padding: 10px 14px; border: 2px solid #e8ecf1; border-radius: 8px; font-size: 14px; } QComboBox:focus { border-color: #0984e3; }");
    for (auto it = tagColors.begin(); it != tagColors.end(); ++it) {
        tagCombo->addItem(it.value().displayName, it.key());
    }
    QString effectiveTag = collection.tag;
    if (effectiveTag == "上班") {
        effectiveTag = "工作";
    }
    int currentTagIndex = tagCombo->findData(effectiveTag.isEmpty() ? "未分类" : effectiveTag);
    if (currentTagIndex >= 0) {
        tagCombo->setCurrentIndex(currentTagIndex);
    }
    formLayout->addRow("标签分类:", tagCombo);
    
    QSpinBox *prioritySpin = new QSpinBox(&dialog);
    prioritySpin->setRange(0, 999);
    prioritySpin->setValue(collection.sortPriority);
    prioritySpin->setStyleSheet("QSpinBox { padding: 10px 14px; border: 2px solid #e8ecf1; border-radius: 8px; font-size: 14px; } QSpinBox:focus { border-color: #0984e3; }");
    formLayout->addRow("排序优先级:", prioritySpin);
    
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *okButton = new QPushButton("确定", &dialog);
    okButton->setStyleSheet("QPushButton { background-color: #00b894; color: white; padding: 12px 28px; border-radius: 8px; font-weight: bold; font-size: 14px; } QPushButton:hover { background-color: #00a085; }");
    QPushButton *cancelButton = new QPushButton("取消", &dialog);
    cancelButton->setStyleSheet("QPushButton { background-color: #b2bec3; color: white; padding: 12px 28px; border-radius: 8px; font-weight: bold; font-size: 14px; } QPushButton:hover { background-color: #a0aab0; }");
    
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
        collection.tag = tagCombo->currentData().toString();
        collection.sortPriority = prioritySpin->value();
        
        if (isNew) {
            if (db->addCollection(collection)) {
                QMessageBox::information(this, "成功", "集合创建成功！");
            } else {
                QMessageBox::warning(this, "错误", "集合创建失败！");
            }
        } else {
            if (db->updateCollection(collection)) {
                QMessageBox::information(this, "成功", "集合更新成功！");
            } else {
                QMessageBox::warning(this, "错误", "集合更新失败！");
            }
        }
        
        refreshCollectionList();
        if (!isNew) {
            onCollectionSelected(nullptr);
            for (int i = 0; i < collectionListWidget->count(); ++i) {
                if (collectionListWidget->item(i)->data(Qt::UserRole).toInt() == collection.id) {
                    collectionListWidget->setCurrentRow(i);
                    onCollectionSelected(collectionListWidget->item(i));
                    break;
                }
            }
        }
    }
}

void CollectionManagerWidget::onCreateCollection()
{
    AppCollection newCol;
    newCol.tag = "未分类";
    newCol.sortPriority = 0;
    showCollectionPropertiesDialog(newCol, true);
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
    
    showCollectionPropertiesDialog(collection, false);
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
            collectionTagLabel->setText("");
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
        collectionTagLabel->setText("");
        appsModel->clear();
        return;
    }
    
    currentCollectionId = item->data(Qt::UserRole).toInt();
    AppCollection collection = db->getCollectionById(currentCollectionId);
    
    if (collection.id > 0) {
        collectionNameLabel->setText(collection.name);
        collectionDescLabel->setText(collection.description.isEmpty() ? "暂无描述" : collection.description);
        
        QString effectiveTag = collection.tag;
        if (effectiveTag == "上班") {
            effectiveTag = "工作";
        }
        
        TagInfo tagInfo = tagColors.value(effectiveTag, tagColors["未分类"]);
        collectionTagLabel->setText(QString("<span style='background-color: %1; color: white; padding: 6px 16px; border-radius: 20px; font-weight: bold; font-size: 12px;'>🏷️ %2</span>")
                                    .arg(tagInfo.color, tagInfo.displayName));
        
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
    dialog.setMinimumWidth(520);
    dialog.setMinimumHeight(420);
    dialog.setStyleSheet("QDialog { background-color: #fafbfc; }");
    
    QVBoxLayout *layout = new QVBoxLayout(&dialog);
    layout->setSpacing(14);
    layout->setContentsMargins(20, 20, 20, 20);
    
    QLabel *label = new QLabel("选择要添加的应用（可多选）：", &dialog);
    label->setStyleSheet("font-size: 14px; color: #2d3436; font-weight: 500;");
    layout->addWidget(label);
    
    QListWidget *appListWidget = new QListWidget(&dialog);
    appListWidget->setSelectionMode(QAbstractItemView::MultiSelection);
    appListWidget->setStyleSheet("QListWidget { border: 2px solid #e8ecf1; border-radius: 10px; padding: 10px; background-color: white; } QListWidget::item { padding: 10px; border-radius: 6px; } QListWidget::item:selected { background-color: #0984e3; color: white; } QListWidget::item:hover { background-color: #e8ecf1; }");
    
    QList<AppInfo> allApps = db->getAllApps();
    for (const AppInfo &app : allApps) {
        if (!collection.appIds.contains(app.id)) {
            QListWidgetItem *item = new QListWidgetItem(app.name);
            item->setData(Qt::UserRole, app.id);
            appListWidget->addItem(item);
        }
    }
    
    layout->addWidget(appListWidget, 1);
    
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *okButton = new QPushButton("确定", &dialog);
    okButton->setStyleSheet("QPushButton { background-color: #00b894; color: white; padding: 12px 28px; border-radius: 8px; font-weight: bold; font-size: 14px; } QPushButton:hover { background-color: #00a085; }");
    QPushButton *cancelButton = new QPushButton("取消", &dialog);
    cancelButton->setStyleSheet("QPushButton { background-color: #b2bec3; color: white; padding: 12px 28px; border-radius: 8px; font-weight: bold; font-size: 14px; } QPushButton:hover { background-color: #a0aab0; }");
    
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
    menu.setStyleSheet("QMenu { background-color: white; border: 2px solid #e8ecf1; border-radius: 10px; padding: 8px; } QMenu::item { padding: 10px 20px; border-radius: 6px; } QMenu::item:selected { background-color: #0984e3; color: white; }");
    
    QAction *launchAction = menu.addAction("启动");
    QAction *removeAction = menu.addAction("从集合移除");
    
    QAction *selected = menu.exec(appsListView->mapToGlobal(pos));
    
    if (selected == launchAction) {
        onLaunchAppFromCollection();
    } else if (selected == removeAction) {
        onRemoveAppFromCollection();
    }
}

void CollectionManagerWidget::onShowCollectionContextMenu(const QPoint &pos)
{
    QListWidgetItem *item = collectionListWidget->itemAt(pos);
    if (!item) return;
    
    currentCollectionId = item->data(Qt::UserRole).toInt();
    
    QMenu menu(this);
    menu.setStyleSheet("QMenu { background-color: white; border: 2px solid #e8ecf1; border-radius: 10px; padding: 8px; } QMenu::item { padding: 10px 20px; border-radius: 6px; } QMenu::item:selected { background-color: #0984e3; color: white; }");
    
    QAction *propertiesAction = menu.addAction("修改属性");
    QAction *renameAction = menu.addAction("重命名");
    QAction *addAppAction = menu.addAction("添加应用");
    QAction *exportAction = menu.addAction("导出集合");
    menu.addSeparator();
    QAction *deleteAction = menu.addAction("删除");
    
    QAction *selected = menu.exec(collectionListWidget->mapToGlobal(pos));
    
    if (selected == propertiesAction) {
        onEditCollectionProperties();
    } else if (selected == renameAction) {
        onRenameCollection();
    } else if (selected == addAppAction) {
        onAddAppToCollection();
    } else if (selected == exportAction) {
        onExportCollection();
    } else if (selected == deleteAction) {
        onDeleteCollection();
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

void CollectionManagerWidget::onRenameCollection()
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
    
    bool ok;
    QString newName = QInputDialog::getText(this, "重命名集合", "输入新名称:", QLineEdit::Normal, collection.name, &ok);
    
    if (ok && !newName.trimmed().isEmpty()) {
        collection.name = newName.trimmed();
        if (db->updateCollection(collection)) {
            QMessageBox::information(this, "成功", "集合重命名成功！");
            refreshCollectionList();
            collectionNameLabel->setText(collection.name);
        } else {
            QMessageBox::warning(this, "错误", "集合重命名失败！");
        }
    }
}

void CollectionManagerWidget::onEditCollectionProperties()
{
    onEditCollection();
}

void CollectionManagerWidget::onExportCollection()
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
    
    QString fileName = QFileDialog::getSaveFileName(this, "导出集合", collection.name + ".json", "JSON文件 (*.json)");
    if (fileName.isEmpty()) {
        return;
    }
    
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, "错误", "无法保存文件！");
        return;
    }
    
    QJsonArray appArray;
    for (int appId : collection.appIds) {
        AppInfo app = db->getAppById(appId);
        if (app.id > 0) {
            QJsonObject appObj;
            appObj["name"] = app.name;
            appObj["path"] = app.path;
            appObj["arguments"] = app.arguments;
            appObj["iconPath"] = app.iconPath;
            appArray.append(appObj);
        }
    }
    
    QJsonObject exportObj;
    exportObj["name"] = collection.name;
    exportObj["description"] = collection.description;
    exportObj["tag"] = collection.tag;
    exportObj["apps"] = appArray;
    
    QJsonDocument doc(exportObj);
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    
    QMessageBox::information(this, "成功", "集合导出成功！");
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
