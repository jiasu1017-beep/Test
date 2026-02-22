#include "desktopsnapshotdialog.h"
#include <QApplication>
#include <QStyle>
#include <QHeaderView>
#include <QMessageBox>
#include <QInputDialog>
#include <QLineEdit>
#include <QFileInfo>
#include <QDir>
#include <QFileDialog>
#include <QStandardPaths>
#include <windows.h>
#include <psapi.h>
#include <shlwapi.h>
#include <objbase.h>
#include <exdisp.h>

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

QString DesktopSnapshotDialog::getExplorerFolderPath(HWND hwnd)
{
    QString folderPath;
    
    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr)) {
        return folderPath;
    }
    
    IShellWindows* pShellWindows = NULL;
    hr = CoCreateInstance(CLSID_ShellWindows, NULL, CLSCTX_LOCAL_SERVER, IID_PPV_ARGS(&pShellWindows));
    if (FAILED(hr) || !pShellWindows) {
        CoUninitialize();
        return folderPath;
    }
    
    long lCount = 0;
    hr = pShellWindows->get_Count(&lCount);
    if (SUCCEEDED(hr)) {
        for (long i = 0; i < lCount; i++) {
            VARIANT varIndex;
            varIndex.vt = VT_I4;
            varIndex.lVal = i;
            
            IDispatch* pDispatch = NULL;
            hr = pShellWindows->Item(varIndex, &pDispatch);
            
            if (SUCCEEDED(hr) && pDispatch) {
                IWebBrowser2* pWebBrowser = NULL;
                hr = pDispatch->QueryInterface(IID_PPV_ARGS(&pWebBrowser));
                pDispatch->Release();
                
                if (SUCCEEDED(hr) && pWebBrowser) {
                    HWND hwndBrowser = NULL;
                    hr = pWebBrowser->get_HWND((SHANDLE_PTR*)&hwndBrowser);
                    
                    if (SUCCEEDED(hr) && hwndBrowser == hwnd) {
                        BSTR bstrURL = NULL;
                        hr = pWebBrowser->get_LocationURL(&bstrURL);
                        
                        if (SUCCEEDED(hr) && bstrURL) {
                            QString url = QString::fromWCharArray(bstrURL);
                            if (url.startsWith("file:///")) {
                                folderPath = url.mid(8);
                                folderPath.replace("/", "\\");
                            }
                            SysFreeString(bstrURL);
                        }
                    }
                    
                    pWebBrowser->Release();
                }
            }
        }
    }
    
    pShellWindows->Release();
    CoUninitialize();
    
    return folderPath;
}

BOOL IsDescendantWindow(HWND hwndParent, HWND hwndChild)
{
    if (hwndParent == hwndChild) {
        return TRUE;
    }
    
    HWND hwnd = GetWindow(hwndChild, GW_OWNER);
    while (hwnd) {
        if (hwnd == hwndParent) {
            return TRUE;
        }
        hwnd = GetWindow(hwnd, GW_OWNER);
    }
    
    hwnd = GetParent(hwndChild);
    while (hwnd) {
        if (hwnd == hwndParent) {
            return TRUE;
        }
        hwnd = GetParent(hwnd);
    }
    
    return FALSE;
}

QString DesktopSnapshotDialog::getBrowserURL(HWND hwnd)
{
    QString url;
    
    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr)) {
        return url;
    }
    
    IShellWindows* pShellWindows = NULL;
    hr = CoCreateInstance(CLSID_ShellWindows, NULL, CLSCTX_LOCAL_SERVER, IID_PPV_ARGS(&pShellWindows));
    if (FAILED(hr) || !pShellWindows) {
        CoUninitialize();
        return url;
    }
    
    long lCount = 0;
    hr = pShellWindows->get_Count(&lCount);
    if (SUCCEEDED(hr)) {
        for (long i = 0; i < lCount; i++) {
            VARIANT varIndex;
            varIndex.vt = VT_I4;
            varIndex.lVal = i;
            
            IDispatch* pDispatch = NULL;
            hr = pShellWindows->Item(varIndex, &pDispatch);
            
            if (SUCCEEDED(hr) && pDispatch) {
                IWebBrowser2* pWebBrowser = NULL;
                hr = pDispatch->QueryInterface(IID_PPV_ARGS(&pWebBrowser));
                pDispatch->Release();
                
                if (SUCCEEDED(hr) && pWebBrowser) {
                    HWND hwndBrowser = NULL;
                    hr = pWebBrowser->get_HWND((SHANDLE_PTR*)&hwndBrowser);
                    
                    if (SUCCEEDED(hr)) {
                        if (hwndBrowser == hwnd || IsDescendantWindow(hwnd, hwndBrowser)) {
                            BSTR bstrURL = NULL;
                            hr = pWebBrowser->get_LocationURL(&bstrURL);
                            
                            if (SUCCEEDED(hr) && bstrURL) {
                                url = QString::fromWCharArray(bstrURL);
                                SysFreeString(bstrURL);
                            }
                        }
                    }
                    
                    pWebBrowser->Release();
                }
            }
        }
    }
    
    pShellWindows->Release();
    CoUninitialize();
    
    return url;
}

QString DesktopSnapshotDialog::getDocumentPath(HWND hwnd)
{
    QString filePath;
    
    wchar_t windowTitle[256];
    GetWindowTextW(hwnd, windowTitle, 256);
    QString title = QString::fromWCharArray(windowTitle);
    
    QString docName;
    int dashIndex = title.lastIndexOf(" - ");
    if (dashIndex > 0) {
        docName = title.left(dashIndex).trimmed();
    } else {
        docName = title.trimmed();
    }
    
    if (docName.isEmpty()) {
        return filePath;
    }
    
    QStringList commonExtensions;
    commonExtensions << ".docx" << ".doc" << ".pdf" << ".txt" 
                     << ".xlsx" << ".xls" << ".pptx" << ".ppt"
                     << ".wps" << ".rtf" << ".odt";
    
    QStringList searchPaths;
    searchPaths << QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    searchPaths << QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    searchPaths << QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    searchPaths << QDir::homePath();
    
    QFileInfo docInfo(docName);
    QString baseName = docInfo.completeBaseName();
    QString existingExt = docInfo.suffix().toLower();
    
    for (const QString &searchPath : searchPaths) {
        if (searchPath.isEmpty()) continue;
        
        QDir dir(searchPath);
        if (!dir.exists()) continue;
        
        if (!existingExt.isEmpty()) {
            QString fullPath = searchPath + "/" + docName;
            if (QFile::exists(fullPath)) {
                filePath = QDir::toNativeSeparators(fullPath);
                return filePath;
            }
        }
        
        for (const QString &ext : commonExtensions) {
            QString testPath = searchPath + "/" + baseName + ext;
            if (QFile::exists(testPath)) {
                filePath = QDir::toNativeSeparators(testPath);
                return filePath;
            }
        }
    }
    
    return filePath;
}

QList<SnapshotWindowInfo> DesktopSnapshotDialog::captureOpenWindows()
{
    QList<SnapshotWindowInfo> windows;
    EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&windows));
    
    for (int i = 0; i < windows.size(); ++i) {
        windows[i].processPath = getProcessPathFromWindow(windows[i].hwnd);
        windows[i].processName = getProcessNameFromPath(windows[i].processPath);
        
        if (windows[i].processName.toLower() == "explorer.exe") {
            QString folderPath = getExplorerFolderPath(windows[i].hwnd);
            if (!folderPath.isEmpty()) {
                windows[i].processPath = folderPath;
            }
        }
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
    currentWindows = windows;
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

AppTypeDetection DesktopSnapshotDialog::detectAppType(const SnapshotWindowInfo &window)
{
    AppTypeDetection result;
    result.type = AppType_Executable;
    result.path = window.processPath;
    result.name = window.title;
    
    QString lowerProcessName = window.processName.toLower();
    QString lowerTitle = window.title.toLower();
    
    if (lowerProcessName == "explorer.exe") {
        result.type = AppType_Folder;
        QString cleanTitle = window.title;
        int dashIndex = cleanTitle.lastIndexOf(" - ");
        if (dashIndex > 0) {
            cleanTitle = cleanTitle.left(dashIndex).trimmed();
        }
        result.name = cleanTitle;
    } else if (lowerProcessName.contains("chrome") || lowerProcessName.contains("firefox") || 
               lowerProcessName.contains("msedge") || lowerProcessName.contains("opera") ||
               lowerProcessName.contains("brave") || lowerProcessName.contains("safari")) {
        result.type = AppType_Website;
        QString cleanTitle = window.title;
        int dashIndex = cleanTitle.lastIndexOf(" - ");
        if (dashIndex > 0) {
            cleanTitle = cleanTitle.left(dashIndex).trimmed();
        }
        result.name = cleanTitle;
    } else if (lowerProcessName.contains("notepad") || lowerProcessName.contains("word") || 
               lowerProcessName.contains("excel") || lowerProcessName.contains("powerpoint") ||
               lowerProcessName.contains("pdf") || lowerProcessName.contains("acrobat") ||
               lowerProcessName.contains("wps") || lowerProcessName.contains("office") ||
               lowerTitle.endsWith(".docx") || lowerTitle.endsWith(".pdf") ||
               lowerTitle.endsWith(".txt") || lowerTitle.endsWith(".xlsx") ||
               lowerTitle.endsWith(".pptx")) {
        result.type = AppType_Document;
        QString cleanTitle = window.title;
        int dashIndex = cleanTitle.lastIndexOf(" - ");
        if (dashIndex > 0) {
            cleanTitle = cleanTitle.left(dashIndex).trimmed();
        }
        result.name = cleanTitle;
    } else {
        result.name = extractWindowTitle(window.title);
    }
    
    return result;
}

void DesktopSnapshotDialog::onTableItemChanged(QTableWidgetItem *item)
{
}

void DesktopSnapshotDialog::onAddToCollection()
{
    QList<SnapshotWindowInfo> selectedWindows;
    
    for (int i = 0; i < windowTable->rowCount(); ++i) {
        QCheckBox *checkBox = qobject_cast<QCheckBox*>(windowTable->cellWidget(i, 0));
        if (checkBox && checkBox->isChecked() && i < currentWindows.size()) {
            selectedWindows.append(currentWindows[i]);
        }
    }
    
    if (selectedWindows.isEmpty()) {
        QMessageBox::warning(this, "提示", "请至少选择一个窗口！");
        return;
    }
    
    QList<AppInfo> allApps = db->getAllApps();
    QList<AppTypeDetection> validApps;
    
    for (const SnapshotWindowInfo &window : selectedWindows) {
        AppTypeDetection detection = this->detectAppType(window);
        
        if (detection.type == AppType_Folder) {
            QString folderPath = getExplorerFolderPath(window.hwnd);
            if (folderPath.isEmpty()) {
                folderPath = QFileDialog::getExistingDirectory(this, "选择文件夹路径", QDir::homePath());
                if (folderPath.isEmpty()) {
                    continue;
                }
            }
            detection.path = folderPath;
            validApps.append(detection);
        } else if (detection.type == AppType_Website) {
            QString url = getBrowserURL(window.hwnd);
            if (url.isEmpty() || (!url.startsWith("http://") && !url.startsWith("https://"))) {
                bool ok;
                QString defaultUrl = "https://";
                QString lowerTitle = window.title.toLower();
                
                if (lowerTitle.contains("youtube")) {
                    defaultUrl = "https://www.youtube.com";
                } else if (lowerTitle.contains("github")) {
                    defaultUrl = "https://github.com";
                } else if (lowerTitle.contains("baidu")) {
                    defaultUrl = "https://www.baidu.com";
                } else if (lowerTitle.contains("bing")) {
                    defaultUrl = "https://www.bing.com";
                } else if (lowerTitle.contains("google")) {
                    defaultUrl = "https://www.google.com";
                }
                
                QString inputUrl = QInputDialog::getText(this, "输入网站地址", 
                    QString("自动获取失败，请手动输入网站URL:\n\n"
                           "📋 窗口标题: %1\n"
                           "💡 提示: 您可以从浏览器地址栏复制并粘贴URL").arg(detection.name), 
                    QLineEdit::Normal, defaultUrl, &ok);
                if (ok && !inputUrl.trimmed().isEmpty()) {
                    QString trimmedUrl = inputUrl.trimmed();
                    if (!trimmedUrl.startsWith("http://") && !trimmedUrl.startsWith("https://")) {
                        trimmedUrl = "https://" + trimmedUrl;
                    }
                    url = trimmedUrl;
                    detection.path = url;
                    validApps.append(detection);
                }
            } else {
                detection.path = url;
                validApps.append(detection);
            }
        } else if (detection.type == AppType_Document) {
            QString foundPath = getDocumentPath(window.hwnd);
            QString filePath;
            
            if (!foundPath.isEmpty() && QFile::exists(foundPath)) {
                filePath = foundPath;
            } else {
                QString initialPath = QDir::homePath();
                QString fileNameFilter = "所有文件 (*.*);;Word文档 (*.docx *.doc);;PDF文档 (*.pdf);;文本文档 (*.txt);;Excel文档 (*.xlsx *.xls);;PowerPoint文档 (*.pptx *.ppt)";
                filePath = QFileDialog::getOpenFileName(this, "选择文档文件", initialPath, fileNameFilter);
                if (filePath.isEmpty()) {
                    continue;
                }
            }
            
            QFileInfo fileInfo(filePath);
            detection.name = fileInfo.completeBaseName();
            detection.path = filePath;
            validApps.append(detection);
        }
    }
    
    if (validApps.isEmpty()) {
        QMessageBox::warning(this, "添加失败", "未识别到有效应用，无法创建集合！\n请确保选择的窗口包含有效的文件夹、网站或文档。");
        return;
    }
    
    int collectionId = collectionComboBox->currentData().toInt();
    AppCollection collection;
    QList<AppCollection> allCollections = db->getAllCollections();
    
    if (collectionId == -1) {
        bool ok;
        QString collectionName = QInputDialog::getText(this, "新建集合", "请输入集合名称:", QLineEdit::Normal, "桌面快照", &ok);
        if (!ok || collectionName.trimmed().isEmpty()) {
            return;
        }
        
        QString trimmedCollectionName = collectionName.trimmed();
        
        for (const AppCollection &existingCol : allCollections) {
            if (existingCol.name == trimmedCollectionName) {
                QMessageBox::information(this, "提示", QString("集合 \"%1\" 已存在，将把应用添加到该集合中。").arg(trimmedCollectionName));
                collection = existingCol;
                collectionId = existingCol.id;
                break;
            }
        }
        
        if (collectionId == -1) {
            collection.name = trimmedCollectionName;
            collection.description = "通过桌面快照创建的集合";
            collection.tag = "未分类";
            
            if (db->addCollection(collection)) {
                collection = db->getAllCollections().last();
            } else {
                QMessageBox::warning(this, "错误", "创建集合失败！");
                return;
            }
        }
    } else {
        collection = db->getCollectionById(collectionId);
        if (collection.id <= 0) {
            QMessageBox::warning(this, "错误", "无法找到该集合！");
            return;
        }
    }
    
    int addedCount = 0;
    int skippedCount = 0;
    QStringList skippedAppNames;
    
    for (const AppTypeDetection &detection : validApps) {
        bool appExists = false;
        AppInfo existingApp;
        
        for (const AppInfo &app : allApps) {
            if (app.type == detection.type && app.path == detection.path && app.name == detection.name) {
                appExists = true;
                existingApp = app;
                break;
            }
        }
        
        if (appExists) {
            if (!collection.appIds.contains(existingApp.id)) {
                collection.appIds.append(existingApp.id);
                addedCount++;
            } else {
                skippedCount++;
                skippedAppNames.append(detection.name);
            }
        } else {
            int maxSortOrder = 0;
            for (const AppInfo &a : allApps) {
                if (a.sortOrder > maxSortOrder) {
                    maxSortOrder = a.sortOrder;
                }
            }
            
            AppInfo newApp;
            newApp.name = detection.name;
            newApp.path = detection.path;
            newApp.arguments = "";
            newApp.iconPath = "";
            newApp.category = "桌面快照";
            newApp.useCount = 0;
            newApp.isFavorite = false;
            newApp.sortOrder = maxSortOrder + 1;
            newApp.type = detection.type;
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
        QString message = QString("已成功将 %1 个应用添加到集合 \"%2\"！").arg(addedCount).arg(collection.name);
        if (skippedCount > 0) {
            message += QString("\n\n跳过了 %1 个已存在的应用：\n").arg(skippedCount);
            for (int i = 0; i < skippedAppNames.size() && i < 5; ++i) {
                message += QString("  • %1\n").arg(skippedAppNames[i]);
            }
            if (skippedAppNames.size() > 5) {
                message += QString("  ... 还有 %1 个应用").arg(skippedAppNames.size() - 5);
            }
        }
        QMessageBox::information(this, "成功", message);
        accept();
    } else {
        QMessageBox::warning(this, "错误", "更新集合失败！");
    }
}
