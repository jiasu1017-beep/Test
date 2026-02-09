#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>

struct AppInfo {
    int id;
    QString name;
    QString path;
    QString iconPath;
    QString category;
    int useCount;
    bool isFavorite;
};

struct AppCollection {
    int id;
    QString name;
    QList<int> appIds;
};

class Database : public QObject
{
    Q_OBJECT
public:
    explicit Database(QObject *parent = nullptr);
    ~Database();

    bool init();
    
    bool addApp(const AppInfo &app);
    bool updateApp(const AppInfo &app);
    bool deleteApp(int id);
    QList<AppInfo> getAllApps();
    QList<AppInfo> getFavoriteApps();
    AppInfo getAppById(int id);
    
    bool addCollection(const AppCollection &collection);
    bool updateCollection(const AppCollection &collection);
    bool deleteCollection(int id);
    QList<AppCollection> getAllCollections();
    
    bool setAutoStart(bool enabled);
    bool getAutoStart();

private:
    QSqlDatabase db;
    bool createTables();
};

#endif
