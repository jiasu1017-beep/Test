#include "desktopsnapshotdialog.h"
#include <QApplication>
#include <QStyle>
#include <QHeaderView>
#include <QMessageBox>
#include <QInputDialog>
#include <QLineEdit>
#include <QFileInfo>
#include <windows.h>
#include <psapi.h>

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
    QList<SnapshotWindowInfo> *windows = reinterpret_cast<QList<SnapshotWindowInfo>*>(lParam);
    
    if (!IsWindowVisible(hwnd)) {
        return TRUE;
    }
    
    char title[256];
    GetWindowTextA(hwnd, title, 256);
    QString windowTitle = QString::fromLocal8Bit(title);
    
    if (windowTitle.isEmpty()) {
        return TRUE;
    }
    
    if (windowTitle == "Program Manager" || windowTitle == "Start") {
        return TRUE;
    }
    
    SnapshotWindowInfo info;
    info.hwnd = hwnd;
    info.title = windowTitle;
    info.isSelected = true;
    windows->append(info);
    
    return TRUE;
}

DesktopSnapshotDialog::DesktopSnapshotDialog(Database *db, QWidget *parent)
    : QDialog(parent), db(db)
{
    setWindowTitle("桌面快照");
    setMinimumSize(800, 600);
    setupUI();
    onCaptureSnapshot();
}

void DesktopSnapshotDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(12);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    
    QLabel *titleLabel = new QLabel("桌面快照", this);
    titleLabel->setStyleSheet("font-size: 22px; font-weight: bold; color: #2d3436;");
    mainLayout->addWidget(titleLabel);
    
    statusLabel = new QLabel("准备捕获桌面快照...", this);
    statusLabel->setStyleSheet("font-size: 13px; color: #636e72;");
    mainLayout->addWidget(statusLabel);
    
    windowTable = new QTableWidget(this);
    windowTable->setColumnCount(4);
    windowTable->setHorizontalHeaderLabels(QStringList() << "选择" << "窗口标题" << "进程名" << "进程路径");
    windowTable->horizontalHeader()->setStretchLastSection(true);
    windowTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    windowTable->setAlternatingRowColors(true);
    windowTable->setStyleSheet(
        "QTableWidget { border: 2px solid #e8ecf1; border-radius: 10px; background-color: white; }"
        "QTableWidget::item { padding: 8px; }"
        "QHeaderView::section { background-color: #f8f9fa; padding: 10px; border: none; border-bottom: 2px solid #e8ecf1; font-weight: bold; }"
    );
    connect(windowTable, &QTableWidget::itemChanged, this, &DesktopSnapshotDialog::onTableItemChanged);
    mainLayout->addWidget(windowTable, 1);
    
    QHBoxLayout *topButtonLayout = new QHBoxLayout();
    topButtonLayout->setSpacing(10);
    
    captureButton = new QPushButton("重新捕获", this);
    captureButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_BrowserReload));
    captureButton->setStyleSheet(
        "QPushButton { background-color: #0984e3; color: white; padding: 10px 18px; border-radius: 8px; font-weight: bold; font-size: 13px; } "
        "QPushButton:hover { background-color: #0770c4; } "
        "QPushButton:pressed { background-color: #065da6; }"
    );
    connect(captureButton, &QPushButton::clicked, this, &DesktopSnapshotDialog::onCaptureSnapshot);
    topButtonLayout->addWidget(captureButton);
    
    selectAllButton = new QPushButton("全选", this);
    selectAllButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogApplyButton));
    selectAllButton->setStyleSheet(
        "QPushButton { background-color: #00b894; color: white; padding: 10px 18px; border-radius: 8px; font-weight: bold; font-size: 13px; } "
        "QPushButton:hover { background-color: #00a085; } "
        "QPushButton:pressed { background-color: #008970; }"
    );
    connect(selectAllButton, &QPushButton::clicked, this, &DesktopSnapshotDialog::onSelectAll);
    topButtonLayout->addWidget(selectAllButton);
    
    deselectAllButton = new QPushButton("取消全选", this);
    deselectAllButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogCancelButton));
    deselectAllButton->setStyleSheet(
        "QPushButton { background-color: #d63031; color: white; padding: 10px 18px; border-radius: 8px; font-weight: bold; font-size: 13px; } "
        "QPushButton:hover { background-color: #c0292a; } "
        "QPushButton:pressed { background-color: #a92425; }"
    );
    connect(deselectAllButton, &QPushButton::clicked, this, &DesktopSnapshotDialog::onDeselectAll);
    topButtonLayout->addWidget(deselectAllButton);
    
    topButtonLayout->addStretch();
    mainLayout->addLayout(topButtonLayout);
    
    QHBoxLayout *bottomButtonLayout = new QHBoxLayout();
    bottomButtonLayout->setSpacing(10);
    
    QLabel *collectionLabel = new QLabel("目标集合:", this);
    collectionLabel->setStyleSheet("font-size: 14px; color: #2d3436;");
    bottomButtonLayout->addWidget(collectionLabel);
    
    collectionComboBox = new QComboBox(this);
    collectionComboBox->setStyleSheet(
        "QComboBox { padding: 10px 14px; border: 2px solid #e8ecf1; border-radius: 8px; font-size: 14px; min-width: 200px; } "
        "QComboBox:focus { border-color: #0984e3; }"
    );
    bottomButtonLayout->addWidget(collectionComboBox);
    
    QList<AppCollection> collections = db->getAllCollections();
    collectionComboBox->addItem("新建集合...", -1);
    for (const AppCollection &col : collections) {
        collectionComboBox->addItem(col.name, col.id);
    }
    
    bottomButtonLayout->addStretch();
    
    addToCollectionButton = new QPushButton("添加到集合", this);
    addToCollectionButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogYesButton));
    addToCollectionButton->setStyleSheet(
        "QPushButton { background-color: #6c5ce7; color: white; padding: 12px 28px; border-radius: 8px; font-weight: bold; font-size: 14px; } "
        "QPushButton:hover { background-color: #5f4fd6; } "
        "QPushButton:pressed { background-color: #5243c5; }"
    );
    connect(addToCollectionButton, &QPushButton::clicked, this, &DesktopSnapshotDialog::onAddToCollection);
    bottomButtonLayout->addWidget(addToCollectionButton);
    
    QPushButton *cancelButton = new QPushButton("取消", this);
    cancelButton->setStyleSheet(
        "QPushButton { background-color: #b2bec3; color: white; padding: 12px 28px; border-radius: 8px; font-weight: bold; font-size: 14px; } "
        "QPushButton:hover { background-color: #a0a9ad; } "
        "QPushButton:pressed { background-color: #8d969b; }"
    );
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    bottomButtonLayout->addWidget(cancelButton);
    
    mainLayout->addLayout(bottomButtonLayout);
}

QList<SnapshotWindowInfo> DesktopSnapshotDialog::captureOpenWindows()
{
    QList<SnapshotWindowInfo> windows;
    EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&windows));
    
    for (int i = 0; i < windows.size(); ++i) {
        windows[i].processPath = getProcessPathFromWindow(windows[i].hwnd);
        windows[i].processName = getProcessNameFromPath(windows[i].processPath);
    }
    
    return windows;
}

QString DesktopSnapshotDialog::getProcessPathFromWindow(HWND hwnd)
{
    DWORD processId;
    GetWindowThreadProcessId(hwnd, &processId);
    
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);
    if (!hProcess) {
        return "";
    }
    
    char path[MAX_PATH];
    if (GetModuleFileNameExA(hProcess, NULL, path, MAX_PATH)) {
        CloseHandle(hProcess);
        return QString::fromLocal8Bit(path);
    }
    
    CloseHandle(hProcess);
    return "";
}

QString DesktopSnapshotDialog::getProcessNameFromPath(const QString &path)
{
    if (path.isEmpty()) {
        return "未知";
    }
    QFileInfo fileInfo(path);
    return fileInfo.fileName();
}

QString DesktopSnapshotDialog::extractWindowTitle(const QString &title)
{
    QString cleanTitle = title;
    int dashIndex = cleanTitle.lastIndexOf(" - ");
    if (dashIndex > 0) {
        cleanTitle = cleanTitle.left(dashIndex).trimmed();
    }
    return cleanTitle;
}

void DesktopSnapshotDialog::populateTable(const QList<SnapshotWindowInfo> &windows)
{
    windowTable->setRowCount(0);
    
    for (int i = 0; i < windows.size(); ++i) {
        const SnapshotWindowInfo &info = windows[i];
        windowTable->insertRow(i);
        
        QCheckBox *checkBox = new QCheckBox(this);
        checkBox->setChecked(info.isSelected);
        windowTable->setCellWidget(i, 0, checkBox);
        
        QTableWidgetItem *titleItem = new QTableWidgetItem(info.title);
        titleItem->setFlags(titleItem->flags() & ~Qt::ItemIsEditable);
        windowTable->setItem(i, 1, titleItem);
        
        QTableWidgetItem *processItem = new QTableWidgetItem(info.processName);
        processItem->setFlags(processItem->flags() & ~Qt::ItemIsEditable);
        windowTable->setItem(i, 2, processItem);
        
        QTableWidgetItem *pathItem = new QTableWidgetItem(info.processPath);
        pathItem->setFlags(pathItem->flags() & ~Qt::ItemIsEditable);
        windowTable->setItem(i, 3, pathItem);
    }
}

void DesktopSnapshotDialog::onCaptureSnapshot()
{
    statusLabel->setText("正在捕获桌面快照...");
    QApplication::processEvents();
    
    QList<SnapshotWindowInfo> windows = captureOpenWindows();
    populateTable(windows);
    
    statusLabel->setText(QString("捕获完成！共发现 %1 个窗口").arg(windows.size()));
}

void DesktopSnapshotDialog::onSelectAll()
{
    for (int i = 0; i < windowTable->rowCount(); ++i) {
        QCheckBox *checkBox = qobject_cast<QCheckBox*>(windowTable->cellWidget(i, 0));
        if (checkBox) {
            checkBox->setChecked(true);
        }
    }
}

void DesktopSnapshotDialog::onDeselectAll()
{
    for (int i = 0; i < windowTable->rowCount(); ++i) {
        QCheckBox *checkBox = qobject_cast<QCheckBox*>(windowTable->cellWidget(i, 0));
        if (checkBox) {
            checkBox->setChecked(false);
        }
    }
}

void DesktopSnapshotDialog::onTableItemChanged(QTableWidgetItem *item)
{
}

void DesktopSnapshotDialog::onAddToCollection()
{
    QList<SnapshotWindowInfo> selectedWindows;
    for (int i = 0; i < windowTable->rowCount(); ++i) {
        QCheckBox *checkBox = qobject_cast<QCheckBox*>(windowTable->cellWidget(i, 0));
        if (checkBox && checkBox->isChecked()) {
            SnapshotWindowInfo info;
            info.title = windowTable->item(i, 1)->text();
            info.processName = windowTable->item(i, 2)->text();
            info.processPath = windowTable->item(i, 3)->text();
            selectedWindows.append(info);
        }
    }
    
    if (selectedWindows.isEmpty()) {
        QMessageBox::warning(this, "提示", "请至少选择一个窗口！");
        return;
    }
    
    int collectionId = collectionComboBox->currentData().toInt();
    AppCollection collection;
    
    if (collectionId == -1) {
        bool ok;
        QString collectionName = QInputDialog::getText(this, "新建集合", "请输入集合名称:", QLineEdit::Normal, "桌面快照", &ok);
        if (!ok || collectionName.trimmed().isEmpty()) {
            return;
        }
        
        collection.name = collectionName.trimmed();
        collection.description = "通过桌面快照创建的集合";
        collection.tag = "未分类";
        
        if (db->addCollection(collection)) {
            collection = db->getAllCollections().last();
        } else {
            QMessageBox::warning(this, "错误", "创建集合失败！");
            return;
        }
    } else {
        collection = db->getCollectionById(collectionId);
        if (collection.id <= 0) {
            QMessageBox::warning(this, "错误", "无法找到该集合！");
            return;
        }
    }
    
    int addedCount = 0;
    QList<AppInfo> allApps = db->getAllApps();
    
    for (const SnapshotWindowInfo &window : selectedWindows) {
        if (window.processPath.isEmpty()) {
            continue;
        }
        
        bool appExists = false;
        AppInfo existingApp;
        for (const AppInfo &app : allApps) {
            if (app.path == window.processPath) {
                appExists = true;
                existingApp = app;
                break;
            }
        }
        
        if (appExists) {
            if (!collection.appIds.contains(existingApp.id)) {
                collection.appIds.append(existingApp.id);
                addedCount++;
            }
        } else {
            int maxSortOrder = 0;
            for (const AppInfo &a : allApps) {
                if (a.sortOrder > maxSortOrder) {
                    maxSortOrder = a.sortOrder;
                }
            }
            
            AppInfo newApp;
            newApp.name = extractWindowTitle(window.title);
            newApp.path = window.processPath;
            newApp.arguments = "";
            newApp.iconPath = "";
            newApp.category = "桌面快照";
            newApp.useCount = 0;
            newApp.isFavorite = false;
            newApp.sortOrder = maxSortOrder + 1;
            newApp.type = AppType_Executable;
            newApp.isRemoteDesktop = false;
            newApp.remoteDesktopId = -1;
            
            if (db->addApp(newApp)) {
                AppInfo addedApp = db->getAllApps().last();
                allApps.append(addedApp);
                collection.appIds.append(addedApp.id);
                addedCount++;
            }
        }
    }
    
    if (db->updateCollection(collection)) {
        QMessageBox::information(this, "成功", QString("已成功将 %1 个应用添加到集合 \"%2\"！").arg(addedCount).arg(collection.name));
        accept();
    } else {
        QMessageBox::warning(this, "错误", "更新集合失败！");
    }
}
