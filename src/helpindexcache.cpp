#include "helpindexcache.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

namespace HelpIndexCache {

static QString canonicalIndexKey(const QString &keyword)
{
    const int sep = keyword.indexOf(QStringLiteral("::"));
    if (sep >= 0) {
        const QString cls = keyword.left(sep);
        const QString member = keyword.mid(sep + 2);
        if (member == cls || member == QLatin1String("~") + cls)
            return cls.toCaseFolded();
        return keyword.toCaseFolded();
    }
    QString name = keyword;
    const int pipe = name.indexOf(QStringLiteral(" | "));
    if (pipe > 0)
        name = name.left(pipe).trimmed();
    if (name.endsWith(QStringLiteral(" Class"), Qt::CaseInsensitive))
        name.chop(6);
    return name.trimmed().toCaseFolded();
}

static int indexKeywordPreference(const QString &keyword)
{
    const int sep = keyword.indexOf(QStringLiteral("::"));
    if (sep >= 0) {
        const QString cls = keyword.left(sep);
        const QString member = keyword.mid(sep + 2);
        if (member == cls)
            return 0;
        if (member.startsWith(QLatin1Char('~')) && member.mid(1) == cls)
            return 1;
        return 10;
    }
    if (keyword.contains(QStringLiteral(" | ")))
        return 100;
    if (keyword.endsWith(QStringLiteral(" Class"), Qt::CaseInsensitive))
        return 50;
    return 20;
}

QStringList dedupeKeywords(const QStringList &keywords)
{
    QHash<QString, QString> best;
    QHash<QString, int> bestRank;
    best.reserve(keywords.size());
    for (const QString &kw : keywords) {
        if (kw.isEmpty())
            continue;
        const QString key = canonicalIndexKey(kw);
        const int rank = indexKeywordPreference(kw);
        const auto it = bestRank.constFind(key);
        if (it == bestRank.constEnd() || rank < *it) {
            bestRank.insert(key, rank);
            best.insert(key, kw);
        }
    }
    QStringList out = best.values();
    out.sort(Qt::CaseInsensitive);
    return out;
}

QString keywordCachePath(const QString &collectionPath)
{
    if (collectionPath.isEmpty())
        return QString();
    const QFileInfo fi(collectionPath);
    return fi.absoluteDir().filePath(fi.completeBaseName() + QStringLiteral(".index-keywords.json"));
}

QString keywordMetaPath(const QString &collectionPath)
{
    if (collectionPath.isEmpty())
        return QString();
    const QFileInfo fi(collectionPath);
    return fi.absoluteDir().filePath(fi.completeBaseName() + QStringLiteral(".index-meta.json"));
}

QString searchIndexPath(const QString &collectionPath)
{
    if (collectionPath.isEmpty())
        return QString();
    const QFileInfo fi(collectionPath);
    return fi.absoluteDir().absoluteFilePath(QStringLiteral(".")
        + fi.fileName().left(fi.fileName().lastIndexOf(QStringLiteral(".qhc"))));
}

bool searchIndexReady(const QString &collectionPath)
{
    const QString dir = searchIndexPath(collectionPath);
    if (dir.isEmpty())
        return false;
    const QFileInfo fts(dir + QStringLiteral("/fts"));
    return fts.exists() && fts.isFile() && fts.size() > 4096;
}

static bool metaMatchesQch(const QJsonObject &root, const QFileInfo &qchInfo)
{
    if (root.value(QStringLiteral("version")).toInt() != 1)
        return false;
    if (root.value(QStringLiteral("qchSize")).toVariant().toLongLong() != qchInfo.size())
        return false;
    return root.value(QStringLiteral("qchModified")).toString()
        == qchInfo.lastModified().toString(Qt::ISODate);
}

static QJsonObject readJsonFile(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
        return {};
    return QJsonDocument::fromJson(file.readAll()).object();
}

bool writeKeywordMeta(const QString &collectionPath, const QFileInfo &qchInfo, int keywordCount)
{
    if (collectionPath.isEmpty() || keywordCount <= 0)
        return false;
    QJsonObject root;
    root.insert(QStringLiteral("version"), 1);
    root.insert(QStringLiteral("qchSize"), qchInfo.size());
    root.insert(QStringLiteral("qchModified"), qchInfo.lastModified().toString(Qt::ISODate));
    root.insert(QStringLiteral("keywordCount"), keywordCount);
    const QByteArray data = QJsonDocument(root).toJson(QJsonDocument::Compact);
    QFile file(keywordMetaPath(collectionPath));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
        return false;
    return file.write(data) == data.size();
}

bool keywordCacheFileValid(const QString &collectionPath, const QFileInfo &qchInfo)
{
    if (!qchInfo.exists())
        return false;
    const QJsonObject meta = readJsonFile(keywordMetaPath(collectionPath));
    if (metaMatchesQch(meta, qchInfo))
        return meta.value(QStringLiteral("keywordCount")).toInt() > 0;
    const QJsonObject root = readJsonFile(keywordCachePath(collectionPath));
    if (!metaMatchesQch(root, qchInfo))
        return false;
    const int count = root.value(QStringLiteral("keywords")).toArray().size();
    if (count > 0)
        writeKeywordMeta(collectionPath, qchInfo, count);
    return count > 0;
}

bool prebuiltReady(const QString &collectionPath, const QFileInfo &qchInfo)
{
    return searchIndexReady(collectionPath) && keywordCacheFileValid(collectionPath, qchInfo);
}

bool loadKeywords(const QString &collectionPath, const QFileInfo &qchInfo, QStringList *keywords)
{
    if (!keywords || !qchInfo.exists())
        return false;
    const QJsonObject root = readJsonFile(keywordCachePath(collectionPath));
    if (!metaMatchesQch(root, qchInfo))
        return false;
    const QJsonArray arr = root.value(QStringLiteral("keywords")).toArray();
    if (arr.isEmpty())
        return false;
    keywords->clear();
    keywords->reserve(arr.size());
    for (const QJsonValue &v : arr)
        keywords->append(v.toString());
    *keywords = dedupeKeywords(*keywords);
    return true;
}

bool saveKeywords(const QString &collectionPath, const QFileInfo &qchInfo,
                  const QStringList &keywords)
{
    const QStringList deduped = dedupeKeywords(keywords);
    if (collectionPath.isEmpty() || deduped.isEmpty())
        return false;
    QJsonObject root;
    root.insert(QStringLiteral("version"), 1);
    root.insert(QStringLiteral("qchSize"), qchInfo.size());
    root.insert(QStringLiteral("qchModified"), qchInfo.lastModified().toString(Qt::ISODate));
    QJsonArray arr;
    for (const QString &k : deduped)
        arr.append(k);
    root.insert(QStringLiteral("keywords"), arr);
    const QByteArray data = QJsonDocument(root).toJson(QJsonDocument::Compact);
    QFile file(keywordCachePath(collectionPath));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
        return false;
    if (file.write(data) != data.size())
        return false;
    return writeKeywordMeta(collectionPath, qchInfo, deduped.size());
}

} // namespace HelpIndexCache
