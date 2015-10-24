#ifndef PATHUTIL_H
#define PATHUTIL_H

#include <functional>

#include <QString>
#include <QList>

class PathUtil
{
public:
    /// Finds path segment containing the offset.
    static void findSegment(const QString &path, int offset, int *begin, int *end);

    /// Finds a valid root.
    struct RequestInfo
    {
        int begin;
        int end;

        /// Path to a root directory.
        QString root;
        /// Path to a first existing path.
        QString path;
        /// Type of the root.
        int root_type;
        /// Request to filter the root directory.
        QString request;
        /// Request's prefix that all the entries in the root must match.
        QString prefix;
    };

    typedef std::function<int (const QString &path)> IsRootFunction;
    static bool parseRequest(const QString &path, const QString &request, int cursor, IsRootFunction is_root, RequestInfo *info);

    /// Returns index to a first non-existing path segment.
    static int findFirstNonExisting(const QString &path);
    /// Returns index to a segment that contains one of files given.
    static int findLastContaining(const QString &path, const QStringList &files, int *file_index);
};

#endif // PATHUTIL_H
