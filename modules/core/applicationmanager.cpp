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