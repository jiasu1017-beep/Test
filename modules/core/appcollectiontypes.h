#ifndef APPCOLLECTIONTYPES_H
#define APPCOLLECTIONTYPES_H

#include <QString>
#include <QVector>

struct RecommendedAppInfo {
    QString name;
    QString url;
    QString description;
    QString iconEmoji;
    QString category;
    bool isFavorite;
};

struct CategoryInfo {
    QString name;
    QString iconEmoji;
    QVector<RecommendedAppInfo> apps;
};

// 相等比较运算符
inline bool operator==(const RecommendedAppInfo &lhs, const RecommendedAppInfo &rhs) {
    return lhs.name == rhs.name &&
           lhs.url == rhs.url &&
           lhs.description == rhs.description &&
           lhs.iconEmoji == rhs.iconEmoji &&
           lhs.category == rhs.category &&
           lhs.isFavorite == rhs.isFavorite;
}

inline bool operator!=(const RecommendedAppInfo &lhs, const RecommendedAppInfo &rhs) {
    return !(lhs == rhs);
}

inline bool operator==(const CategoryInfo &lhs, const CategoryInfo &rhs) {
    return lhs.name == rhs.name &&
           lhs.iconEmoji == rhs.iconEmoji &&
           lhs.apps == rhs.apps;
}

inline bool operator!=(const CategoryInfo &lhs, const CategoryInfo &rhs) {
    return !(lhs == rhs);
}

#endif // APPCOLLECTIONTYPES_H
