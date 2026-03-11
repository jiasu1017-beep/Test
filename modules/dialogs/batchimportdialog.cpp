#include "batchimportdialog.h"
#include "modules/core/database.h"
#include "modules/core/applicationmanager.h"

BatchImportDialog::BatchImportDialog(const QString &title, const QString &labelText,
                                     const QList<AppInfo> &items, Database *db,
                                     QWidget *parent)
    : QDialog(parent)
    , db(db)
    , m_labelText(labelText)
    , allItems(items)
{
    setWindowTitle(title);
    setMinimumSize(700, 500);
    setStyleSheet("QDialog { background-color: #fafbfc; }");

    setupUI();
    setupConnections();
}

BatchImportDialog::~BatchImportDialog()
{
}

void BatchImportDialog::setupUI()
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    label = new QLabel(QString("%1 共 %2 项：").arg(m_labelText).arg(allItems.size()), this);
    label->setStyleSheet("font-size: 14px; color: #2d3436; font-weight: 500;");
    layout->addWidget(label);

    searchBox = new QLineEdit(this);
    searchBox->setPlaceholderText("输入名称进行搜索...");
    searchBox->setStyleSheet("QLineEdit { padding: 8px; border: 1px solid #dfe6e9; border-radius: 5px; background-color: white; font-size: 13px; } "
                             "QLineEdit:focus { border: 1px solid #74b9ff; }");
    layout->addWidget(searchBox);

    listWidget = new QListWidget(this);
    listWidget->setStyleSheet("QListWidget { border: 1px solid #dfe6e9; border-radius: 5px; background-color: white; } "
                             "QListWidget::item { padding: 5px; border-bottom: 1px solid #f1f2f6; } "
                             "QListWidget::item:selected { background-color: #e3f2fd; }");
    listWidget->setSelectionMode(QAbstractItemView::NoSelection);

    for (int i = 0; i < allItems.size(); ++i) {
        QListWidgetItem *item = new QListWidgetItem(listWidget);
        item->setSizeHint(QSize(0, 42));

        BatchImportListItemWidget *widget = new BatchImportListItemWidget(allItems[i], true, listWidget);
        widget->checkBox->setChecked(false);
        listWidget->setItemWidget(item, widget);
        itemWidgets[i] = widget;
    }

    layout->addWidget(listWidget);

    btnSelectAll = new QPushButton("全选", this);
    btnDeselectAll = new QPushButton("取消全选", this);
    btnInvertSelect = new QPushButton("反选", this);
    btnImport = new QPushButton("导入所选", this);
    btnCancel = new QPushButton("取消", this);

    btnSelectAll->setStyleSheet("QPushButton { background-color: #74b9ff; color: white; padding: 8px 16px; border-radius: 5px; } "
                                "QPushButton:hover { background-color: #0984e3; }");
    btnDeselectAll->setStyleSheet("QPushButton { background-color: #a29bfe; color: white; padding: 8px 16px; border-radius: 5px; } "
                                 "QPushButton:hover { background-color: #6c5ce7; }");
    btnInvertSelect->setStyleSheet("QPushButton { background-color: #fdcb6e; color: white; padding: 8px 16px; border-radius: 5px; } "
                                 "QPushButton:hover { background-color: #f0b960; }");
    btnImport->setStyleSheet("QPushButton { background-color: #00b894; color: white; padding: 8px 16px; border-radius: 5px; } "
                             "QPushButton:hover { background-color: #00a085; }");
    btnCancel->setStyleSheet("QPushButton { background-color: #b2bec3; color: white; padding: 8px 16px; border-radius: 5px; } "
                             "QPushButton:hover { background-color: #a0aab0; }");

    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addWidget(btnSelectAll);
    btnLayout->addWidget(btnDeselectAll);
    btnLayout->addWidget(btnInvertSelect);
    btnLayout->addStretch();
    btnLayout->addWidget(btnImport);
    btnLayout->addWidget(btnCancel);
    layout->addLayout(btnLayout);
}

void BatchImportDialog::setupConnections()
{
    searchTimer = new QTimer(this);
    searchTimer->setSingleShot(true);

    connect(searchTimer, &QTimer::timeout, this, [this]() {
        performSearch();
    });
    connect(searchBox, &QLineEdit::textChanged, this, [this]() {
        searchTimer->start(300);
    });

    connect(btnSelectAll, &QPushButton::clicked, this, &BatchImportDialog::selectAllVisible);
    connect(btnDeselectAll, &QPushButton::clicked, this, &BatchImportDialog::deselectAll);
    connect(btnInvertSelect, &QPushButton::clicked, this, &BatchImportDialog::invertSelections);
    connect(btnImport, &QPushButton::clicked, this, &BatchImportDialog::importSelected);
    connect(btnCancel, &QPushButton::clicked, this, &QDialog::reject);
}

void BatchImportDialog::performSearch()
{
    QString keyword = searchBox->text();
    QString lowerKeyword = keyword.toLower();
    int visibleCount = 0;
    for (int i = 0; i < listWidget->count(); ++i) {
        QListWidgetItem *item = listWidget->item(i);
        BatchImportListItemWidget *widget = itemWidgets[i];

        bool matches = lowerKeyword.isEmpty() ||
                       widget->appInfo.name.toLower().contains(lowerKeyword) ||
                       widget->appInfo.path.toLower().contains(lowerKeyword);

        item->setHidden(!matches);
        if (matches) {
            visibleCount++;
        } else {
            widget->checkBox->setChecked(false);
        }
    }

    label->setText(QString("%1 共 %2 项，已显示 %3 项").arg(m_labelText).arg(allItems.size()).arg(visibleCount));
}

void BatchImportDialog::selectAllVisible()
{
    for (int i = 0; i < listWidget->count(); ++i) {
        QListWidgetItem *item = listWidget->item(i);
        if (!item->isHidden()) {
            itemWidgets[i]->checkBox->setChecked(true);
        }
    }
}

void BatchImportDialog::deselectAll()
{
    for (auto widget : itemWidgets.values()) {
        widget->checkBox->setChecked(false);
    }
}

void BatchImportDialog::invertSelections()
{
    for (int i = 0; i < listWidget->count(); ++i) {
        if (!listWidget->item(i)->isHidden()) {
            BatchImportListItemWidget *widget = itemWidgets[i];
            widget->checkBox->setChecked(!widget->checkBox->isChecked());
        }
    }
}

void BatchImportDialog::importSelected()
{
    QList<AppInfo> existingApps = db->getAllApps();
    int addedCount = 0;
    int maxSortOrder = db->getMaxSortOrder();
    QList<AppInfo> selectedItems;

    for (int i = 0; i < listWidget->count(); ++i) {
        BatchImportListItemWidget *widget = itemWidgets[i];
        if (widget->checkBox->isChecked()) {
            AppInfo app = widget->appInfo;

            bool exists = false;
            for (const AppInfo &existing : existingApps) {
                if (existing.path.compare(app.path, Qt::CaseInsensitive) == 0) {
                    exists = true;
                    break;
                }
            }

            if (!exists) {
                app.sortOrder = ++maxSortOrder;
                selectedItems.append(app);
            }
        }
    }

    for (const AppInfo &app : selectedItems) {
        db->addApp(app);
        addedCount++;
    }

    accept();
    QMessageBox::information(this, "完成", QString("成功导入 %1 个应用").arg(addedCount));
}

BatchImportListItemWidget::BatchImportListItemWidget(const AppInfo &app, bool showIcon, QWidget *parent)
    : QWidget(parent)
    , appInfo(app)
{
    setupUI(app, showIcon);
}

void BatchImportListItemWidget::setupUI(const AppInfo &app, bool showIcon)
{
    QHBoxLayout *hLayout = new QHBoxLayout(this);
    hLayout->setContentsMargins(8, 4, 8, 4);
    hLayout->setSpacing(8);

    checkBox = new QCheckBox(this);
    hLayout->addWidget(checkBox);

    if (showIcon) {
        QIcon icon = ApplicationManager::getFileIcon(app.path);
        if (icon.isNull()) {
            icon = QApplication::style()->standardIcon(QStyle::SP_FileDialogDetailedView);
        }
        QPixmap pixmap = icon.pixmap(QSize(32, 32));
        iconLabel = new QLabel(this);
        iconLabel->setPixmap(pixmap);
        iconLabel->setFixedSize(32, 32);
        iconLabel->setScaledContents(false);
        hLayout->addWidget(iconLabel);
    } else {
        iconLabel = nullptr;
    }

    nameLabel = new QLabel(app.name, this);
    nameLabel->setStyleSheet("font-size: 13px; color: #2d3436;");
    nameLabel->setWordWrap(false);
    nameLabel->setToolTip(app.path);
    nameLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    hLayout->addWidget(nameLabel, 1);

    setLayout(hLayout);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    setFixedHeight(42);
}
