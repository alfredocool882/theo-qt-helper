#pragma once

#include <QString>
#include <QStringList>

class QFileInfo;

namespace HelpIndexCache {

QString keywordCachePath(const QString &collectionPath);
QString keywordMetaPath(const QString &collectionPath);
QString searchIndexPath(const QString &collectionPath);
bool searchIndexReady(const QString &collectionPath);
bool keywordCacheFileValid(const QString &collectionPath, const QFileInfo &qchInfo);
bool prebuiltReady(const QString &collectionPath, const QFileInfo &qchInfo);
bool loadKeywords(const QString &collectionPath, const QFileInfo &qchInfo, QStringList *keywords);
bool saveKeywords(const QString &collectionPath, const QFileInfo &qchInfo, const QStringList &keywords);
bool writeKeywordMeta(const QString &collectionPath, const QFileInfo &qchInfo, int keywordCount);

} // namespace HelpIndexCache
