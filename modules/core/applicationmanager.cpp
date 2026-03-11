#include "applicationmanager.h"
#include <QDesktopServices>
#include <QUrl>
#include <QFileInfo>
#include <QStandardPaths>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QDir>
#include <QStyle>
#include <QApplication>
#include <QFileIconProvider>
#include <QSettings>
#include <QRegExp>
#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <shlobj.h>
#include <processthreadsapi.h>

ApplicationManager::ApplicationManager(Database *db, QObject *parent)
    : QObject(parent), m_db(db)
{
}

ApplicationManager::~ApplicationManager()
{
}

ApplicationManager::LaunchResult ApplicationManager::launchApp(const AppInfo &app)
{
    LaunchOptions options;
    options.updateUseCount = true;
    options.refreshUI = true;
    options.silentMode = false;
    return launchApp(app, options);
}

ApplicationManager::LaunchResult ApplicationManager::launchApp(const AppInfo &app, const LaunchOptions &options)
{
    if (app.id <= 0) {
        emit launchFailed(app, "Invalid application ID");
        return InvalidApp;
    }

    QFileInfo fileInfo(app.path);
    if (!app.path.isEmpty() && app.type != AppType_Website && !fileInfo.exists()) {
        emit launchFailed(app, "Application path not found: " + app.path);
        return PathNotFound;
    }

    bool success = false;

    switch (app.type) {
    case AppType_Website:
        success = launchWebsiteApp(app, options);
        break;
    case AppType_Folder:
        success = launchFolderApp(app, options);
        break;
    case AppType_Document:
        success = launchDocumentApp(app, options);
        break;
    default:
        if (app.type == AppType_RemoteDesktop && app.remoteDesktopId > 0) {
            success = launchRemoteDesktopApp(app, options);
        } else {
            success = launchExecutableApp(app, options);
        }
        break;
    }

    if (success) {
        if (options.updateUseCount) {
            updateAppUsage(app);
        }
        emit appLaunched(app);
        return Success;
    } else {
        emit launchFailed(app, "Failed to launch application");
        return LaunchFailed;
    }
}

ApplicationManager::LaunchResult ApplicationManager::launchAppById(int appId)
{
    LaunchOptions options;
    options.updateUseCount = true;
    options.refreshUI = true;
    options.silentMode = false;
    return launchAppById(appId, options);
}

ApplicationManager::LaunchResult ApplicationManager::launchAppById(int appId, const LaunchOptions &options)
{
    if (!m_db) {
        qWarning() << "Database is null in ApplicationManager";
        return InvalidApp;
    }

    AppInfo app = m_db->getAppById(appId);
    if (app.id <= 0) {
        qWarning() << "Application not found with ID:" << appId;
        return InvalidApp;
    }

    return launchApp(app, options);
}

void ApplicationManager::launchRemoteDesktop(const RemoteDesktopConnection &conn, Database *db)
{
    if (conn.hostAddress.isEmpty()) {
        return;
    }

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
    if (!rdpFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return;
    }

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

    if (db) {
        RemoteDesktopConnection updateConn = conn;
        updateConn.lastUsedTime = QDateTime::currentDateTime();
        db->updateRemoteDesktop(updateConn);
    }

    QStringList args;
    args << rdpFilePath;

    QProcess::startDetached("mstsc.exe", args);
}

bool ApplicationManager::launchWebsite(const QString &url)
{
    return QDesktopServices::openUrl(QUrl(url));
}

bool ApplicationManager::launchFolder(const QString &path)
{
    return QDesktopServices::openUrl(QUrl::fromLocalFile(path));
}

bool ApplicationManager::launchDocument(const QString &path)
{
    return QDesktopServices::openUrl(QUrl::fromLocalFile(path));
}

bool ApplicationManager::launchExecutable(const QString &path, const QString &arguments)
{
    QString fullPath = path;
    QString args = arguments;
    
    QFileInfo fileInfo(fullPath);
    QString fileName = fileInfo.fileName().toLower();
    
    if (fileName == "cmd.exe" || fileName == "cmd") {
        if (args.trimmed().isEmpty()) {
            return QProcess::startDetached("cmd /c start cmd.exe");
        } else {
            return QProcess::startDetached(QString("cmd /c start \"\" %1 %2").arg(fullPath, args));
        }
    } else if (fileName == "powershell.exe" || fileName == "powershell") {
        if (args.trimmed().isEmpty()) {
            return QProcess::startDetached("cmd /c start powershell.exe");
        } else {
            return QProcess::startDetached(QString("cmd /c start \"\" %1 %2").arg(fullPath, args));
        }
    } else {
        if (args.trimmed().isEmpty()) {
            return QProcess::startDetached(fullPath, QStringList());
        } else {
            return QProcess::startDetached(fullPath, QProcess::splitCommand(args));
        }
    }
}

bool ApplicationManager::isExecutable(const QString &path)
{
    QFileInfo fileInfo(path);
    return fileInfo.exists() && fileInfo.isExecutable();
}

bool ApplicationManager::isWebsite(const QString &path)
{
    return path.startsWith("http://", Qt::CaseInsensitive) || 
           path.startsWith("https://", Qt::CaseInsensitive);
}

bool ApplicationManager::isFolder(const QString &path)
{
    QFileInfo fileInfo(path);
    return fileInfo.exists() && fileInfo.isDir();
}

bool ApplicationManager::launchWebsiteApp(const AppInfo &app, const LaunchOptions &options)
{
    Q_UNUSED(options);
    return launchWebsite(app.path);
}

bool ApplicationManager::launchFolderApp(const AppInfo &app, const LaunchOptions &options)
{
    Q_UNUSED(options);
    return launchFolder(app.path);
}

bool ApplicationManager::launchDocumentApp(const AppInfo &app, const LaunchOptions &options)
{
    Q_UNUSED(options);
    return launchDocument(app.path);
}

bool ApplicationManager::launchRemoteDesktopApp(const AppInfo &app, const LaunchOptions &options)
{
    Q_UNUSED(options);
    
    if (app.remoteDesktopId <= 0 || !m_db) {
        return false;
    }
    
    RemoteDesktopConnection conn = m_db->getRemoteDesktopById(app.remoteDesktopId);
    if (conn.id == -1) {
        return false;
    }
    
    launchRemoteDesktop(conn, m_db);
    return true;
}

bool ApplicationManager::launchExecutableApp(const AppInfo &app, const LaunchOptions &options)
{
    Q_UNUSED(options);
    return launchExecutable(app.path, app.arguments);
}

void ApplicationManager::updateAppUsage(const AppInfo &app)
{
    if (!m_db) {
        return;
    }
    
    AppInfo updatedApp = app;
    updatedApp.useCount++;
    
    if (m_db->updateApp(updatedApp)) {
        emit useCountUpdated(updatedApp);
    }
}

QIcon ApplicationManager::getFileIcon(const QString &filePath)
{
    if (filePath.isEmpty() || !QFile::exists(filePath)) {
        return QApplication::style()->standardIcon(QStyle::SP_FileIcon);
    }

    QFileInfo fileInfo(filePath);
    QFileIconProvider provider;
    return provider.icon(fileInfo);
}

QIcon ApplicationManager::getAppIcon(const AppInfo &app)
{
    if (app.type == AppType_RemoteDesktop) {
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
        return getFileIcon(app.path);
    }

    return getFileIcon(app.path);
}

QList<AppInfo> ApplicationManager::getAppsFromRegistry()
{
    QList<AppInfo> apps;

    QStringList registryPaths = {
        R"(HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall)",
        R"(HKEY_LOCAL_MACHINE\SOFTWARE\WOW6432Node\Microsoft\Windows\CurrentVersion\Uninstall)",
        R"(HKEY_CURRENT_USER\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall)"
    };

    for (const QString &regPath : registryPaths) {
        QSettings settings(regPath, QSettings::Registry64Format);
        QStringList subkeys = settings.childGroups();

        for (const QString &subkey : subkeys) {
            settings.beginGroup(subkey);
            QString displayName = settings.value("DisplayName").toString();
            QString installLocation = settings.value("InstallLocation").toString();
            QString displayIcon = settings.value("DisplayIcon").toString();
            settings.endGroup();

            if (displayName.isEmpty()) continue;

            QString exePath;
            if (!displayIcon.isEmpty()) {
                QRegExp rx("\"?([^\"]+\\.exe)\"?.*");
                if (rx.indexIn(displayIcon) != -1) {
                    exePath = rx.cap(1);
                }
                if (exePath.isEmpty() && displayIcon.contains(".exe", Qt::CaseInsensitive)) {
                    QStringList parts = displayIcon.split(",");
                    if (!parts.isEmpty()) {
                        exePath = parts.first().trimmed().remove("\"");
                    }
                }
            }

            if (exePath.isEmpty() && !installLocation.isEmpty()) {
                QDir dir(installLocation);
                QFileInfoList exeFiles = dir.entryInfoList(QStringList() << "*.exe", QDir::Files | QDir::Readable, QDir::Name);
                if (!exeFiles.isEmpty()) {
                    exePath = exeFiles.first().filePath();
                }
            }

            if (exePath.isEmpty()) continue;

            QFileInfo fileInfo(exePath);
            if (!fileInfo.exists()) continue;

            AppInfo app;
            app.name = displayName;
            app.path = exePath;
            app.arguments = "";
            app.iconPath = "";
            app.category = "从注册表导入";
            app.useCount = 0;
            app.isFavorite = false;
            app.type = AppType_Executable;
            app.remoteDesktopId = -1;
            app.sortOrder = 0;

            bool exists = false;
            for (const AppInfo &existing : apps) {
                if (existing.path.compare(app.path, Qt::CaseInsensitive) == 0) {
                    exists = true;
                    break;
                }
            }
            if (!exists) {
                apps.append(app);
            }
        }
    }

    return apps;
}

QList<AppInfo> ApplicationManager::getBookmarksFromBrowsers()
{
    QList<AppInfo> bookmarks;

    QStringList browserPaths = {
        QDir::homePath() + R"(/AppData/Local/Google/Chrome/User Data/Default/Bookmarks)",
        QDir::homePath() + R"(/AppData/Roaming/Mozilla/Firefox/Profiles)",
        QDir::homePath() + R"(/AppData/Local/Microsoft/Edge/User Data/Default/Bookmarks)",
        QDir::homePath() + R"(/AppData/Roaming/Opera Software/Opera Stable/Bookmarks)"
    };

    QString chromePath = browserPaths[0];
    if (QFile::exists(chromePath)) {
        QFile file(chromePath);
        if (file.open(QIODevice::ReadOnly)) {
            QByteArray data = file.readAll();
            file.close();

            QJsonDocument doc = QJsonDocument::fromJson(data);
            if (!doc.isNull() && doc.isObject()) {
                parseChromeBookmarks(doc.object(), bookmarks, "Chrome");
            }
        }
    }

    QString edgePath = browserPaths[2];
    if (QFile::exists(edgePath)) {
        QFile file(edgePath);
        if (file.open(QIODevice::ReadOnly)) {
            QByteArray data = file.readAll();
            file.close();

            QJsonDocument doc = QJsonDocument::fromJson(data);
            if (!doc.isNull() && doc.isObject()) {
                parseChromeBookmarks(doc.object(), bookmarks, "Edge");
            }
        }
    }

    QString operaPath = browserPaths[3];
    if (QFile::exists(operaPath)) {
        QFile file(operaPath);
        if (file.open(QIODevice::ReadOnly)) {
            QByteArray data = file.readAll();
            file.close();

            QJsonDocument doc = QJsonDocument::fromJson(data);
            if (!doc.isNull() && doc.isObject()) {
                parseChromeBookmarks(doc.object(), bookmarks, "Opera");
            }
        }
    }

    QString firefoxBasePath = browserPaths[1];
    QDir firefoxDir(firefoxBasePath);
    if (firefoxDir.exists()) {
        QStringList profiles = firefoxDir.entryList(QDir::Dirs);
        for (const QString &profile : profiles) {
            if (profile == "." || profile == "..") continue;
            QString placesPath = firefoxBasePath + "/" + profile + "/places.sqlite";
            if (QFile::exists(placesPath)) {
                parseFirefoxBookmarks(placesPath, bookmarks);
                break;
            }
        }
    }

    return bookmarks;
}

void ApplicationManager::parseChromeBookmarks(const QJsonObject &root, QList<AppInfo> &bookmarks, const QString &source)
{
    if (root.contains("roots")) {
        QJsonObject roots = root["roots"].toObject();
        parseChromeBookmarkFolder(roots["bookmark_bar"].toObject(), bookmarks, source);
        parseChromeBookmarkFolder(roots["other"].toObject(), bookmarks, source);
        parseChromeBookmarkFolder(roots["synced"].toObject(), bookmarks, source);
    }
}

void ApplicationManager::parseChromeBookmarkFolder(const QJsonObject &folder, QList<AppInfo> &bookmarks, const QString &source)
{
    if (folder.isEmpty()) return;

    if (folder["type"].toString() == "url") {
        QString url = folder["url"].toString();
        QString name = folder["name"].toString();

        if (url.startsWith("http://", Qt::CaseInsensitive) || url.startsWith("https://", Qt::CaseInsensitive)) {
            AppInfo app;
            app.name = name;
            app.path = url;
            app.arguments = "";
            app.iconPath = "";
            app.category = "浏览器收藏夹-" + source;
            app.useCount = 0;
            app.isFavorite = false;
            app.type = AppType_Website;
            app.remoteDesktopId = -1;
            app.sortOrder = 0;

            bool exists = false;
            for (const AppInfo &existing : bookmarks) {
                if (existing.path.compare(app.path, Qt::CaseInsensitive) == 0) {
                    exists = true;
                    break;
                }
            }
            if (!exists) {
                bookmarks.append(app);
            }
        }
        return;
    }

    if (folder.contains("children")) {
        QJsonArray children = folder["children"].toArray();
        for (int i = 0; i < children.size(); ++i) {
            if (children[i].isObject()) {
                parseChromeBookmarkFolder(children[i].toObject(), bookmarks, source);
            }
        }
    }
}

void ApplicationManager::parseFirefoxBookmarks(const QString &dbPath, QList<AppInfo> &bookmarks)
{
    Q_UNUSED(dbPath);
    Q_UNUSED(bookmarks);
}

QList<AppInfo> ApplicationManager::getRunningApps()
{
    QList<AppInfo> runningApps;

    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return runningApps;
    }

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (!Process32First(hSnapshot, &pe32)) {
        CloseHandle(hSnapshot);
        return runningApps;
    }

    QStringList excludedProcesses = {
        "System", "Registry", "smss.exe", "csrss.exe", "wininit.exe", "services.exe",
        "lsass.exe", "svchost.exe", "winlogon.exe", "dwm.exe", "explorer.exe",
        "taskhostw.exe", "RuntimeBroker.exe", "SearchHost.exe", "StartMenuExperienceHost.exe",
        "textinputhost.exe", "ShellExperienceHost.exe", "sihost.exe", "ctfmon.exe",
        "fontdrvhost.exe", "conhost.exe", "dllhost.exe", "WmiPrvSE.exe",
        "audiodg.exe", "SearchIndexer.exe", "SecurityHealthService.exe",
        "MsMpEng.exe", "NisSrv.exe", "spoolsv.exe"
    };

    do {
        QString processName = QString::fromUtf16(reinterpret_cast<const ushort*>(pe32.szExeFile));

        bool shouldExclude = false;
        for (const QString &excluded : excludedProcesses) {
            if (processName.compare(excluded, Qt::CaseInsensitive) == 0) {
                shouldExclude = true;
                break;
            }
        }
        if (shouldExclude) continue;

        if (pe32.th32ProcessID == 0 || pe32.th32ProcessID == 4) continue;

        QString exePath = getProcessPath(pe32.th32ProcessID);
        if (exePath.isEmpty()) continue;

        QFileInfo fileInfo(exePath);
        if (!fileInfo.exists()) continue;

        AppInfo app;
        app.name = fileInfo.baseName();
        app.path = exePath;
        app.arguments = "";
        app.iconPath = "";
        app.category = "正在运行";
        app.useCount = 0;
        app.isFavorite = false;
        app.type = AppType_Executable;
        app.remoteDesktopId = -1;
        app.sortOrder = 0;

        bool exists = false;
        for (const AppInfo &existing : runningApps) {
            if (existing.path.compare(app.path, Qt::CaseInsensitive) == 0) {
                exists = true;
                break;
            }
        }
        if (!exists) {
            runningApps.append(app);
        }

    } while (Process32Next(hSnapshot, &pe32));

    CloseHandle(hSnapshot);
    return runningApps;
}

QString ApplicationManager::getProcessPath(DWORD processID)
{
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, processID);
    if (!hProcess) {
        return QString();
    }

    wchar_t path[MAX_PATH];
    DWORD size = MAX_PATH;

    typedef BOOL (WINAPI * QueryFullProcessImageNameWType)(HANDLE, DWORD, LPWSTR, PDWORD);
    HMODULE kernel32 = GetModuleHandleW(L"kernel32.dll");
    QueryFullProcessImageNameWType QueryFullProcessImageNameWFunc = (QueryFullProcessImageNameWType)GetProcAddress(kernel32, "QueryFullProcessImageNameW");

    if (QueryFullProcessImageNameWFunc) {
        if (QueryFullProcessImageNameWFunc(hProcess, 0, path, &size)) {
            CloseHandle(hProcess);
            return QString::fromUtf16(reinterpret_cast<const ushort*>(path));
        }
    }

    size = MAX_PATH;
    if (GetModuleFileNameExW(hProcess, NULL, path, size) > 0) {
        CloseHandle(hProcess);
        return QString::fromUtf16(reinterpret_cast<const ushort*>(path));
    }

    CloseHandle(hProcess);
    return QString();
}