#include <QCoreApplication>
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QDebug>
#include <QTimer>
#include <QThread>

void copyDirectory(const QString &sourcePath, const QString &targetPath)
{
    QDir sourceDir(sourcePath);
    QDir targetDir(targetPath);
    
    if (!sourceDir.exists()) {
        qWarning() << "Source directory does not exist:" << sourcePath;
        return;
    }
    
    if (!targetDir.exists()) {
        targetDir.mkpath(targetPath);
    }
    
    QFileInfoList fileList = sourceDir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QFileInfo &info : fileList) {
        QString srcPath = info.absoluteFilePath();
        QString destPath = targetPath + "/" + info.fileName();
        
        if (info.isDir()) {
            copyDirectory(srcPath, destPath);
        } else {
            QFile destFile(destPath);
            if (destFile.exists()) {
                int retries = 0;
                bool removed = false;
                while (retries < 10 && !removed) {
                    if (destFile.remove()) {
                        removed = true;
                    } else {
                        qWarning() << "Failed to remove file, retrying:" << destPath;
                        QThread::msleep(500);
                        retries++;
                    }
                }
                if (!removed) {
                    qWarning() << "Failed to remove file after retries:" << destPath;
                    continue;
                }
            }
            
            if (QFile::copy(srcPath, destPath)) {
                qDebug() << "Copied:" << info.fileName();
            } else {
                qWarning() << "Failed to copy:" << srcPath << "to" << destPath;
            }
        }
    }
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    
    qDebug() << "Updater started";
    
    if (argc < 4) {
        qWarning() << "Usage: Updater <source_path> <target_path> <app_path>";
        return 1;
    }
    
    QString sourcePath = QString::fromLocal8Bit(argv[1]);
    QString targetPath = QString::fromLocal8Bit(argv[2]);
    QString appPath = QString::fromLocal8Bit(argv[3]);
    
    qDebug() << "Source path:" << sourcePath;
    qDebug() << "Target path:" << targetPath;
    qDebug() << "App path:" << appPath;
    
    qDebug() << "Waiting for main application to exit...";
    QThread::sleep(2);
    
    qDebug() << "Starting file replacement...";
    copyDirectory(sourcePath, targetPath);
    
    qDebug() << "File replacement completed";
    
    qDebug() << "Starting main application...";
    QProcess::startDetached(appPath);
    
    qDebug() << "Updater finished";
    
    QTimer::singleShot(1000, &a, &QCoreApplication::quit);
    
    return a.exec();
}
