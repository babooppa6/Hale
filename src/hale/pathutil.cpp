#include <QDir>

#include "pathutil.h"

int PathUtil::findFirstNonExisting(const QString &path)
{
    QDir dir(path);
    while (!dir.exists()) {
        if (dir.cdUp() == false) {
            return 0; // dir.path().length();
        }
    }
    return dir.path().length();
}

void PathUtil::findSegment(const QString &path, int offset, int *begin, int *end)
{
    Q_ASSERT(begin);
    Q_ASSERT(end);

    *end = path.indexOf('/', offset);
    if (*end == -1) {
        *end = path.length();
    }

    *begin = path.lastIndexOf('/', - (path.length() - offset) - 1);
    if (*begin == -1) {
        *begin = 0;
    } else {
        (*begin)++;
    }
}

bool findRoot(const QString &path, PathUtil::IsRootFunction is_root, PathUtil::RequestInfo *info)
{
    int root_type = 0;
    QString tmp;

    info->root_type = 0;
    info->path.clear();
    info->root.clear();

    QDir dir(path);
    for (;;) {
        if (dir.exists()) {
            tmp = dir.canonicalPath();
            if (!tmp.endsWith('/')) {
                tmp.append('/');
            }
            if (root_type == 0) {
                // Remember the first existing root.
                info->path = tmp;
                // This will probably be also our root.
                info->root = tmp;
                info->root_type = 1;
            }

            // Check what root it is.
            root_type = is_root(tmp);
            if (root_type > 1) {
                // It's a good root.
                info->root_type = root_type;
                info->root = tmp;
                break;
            }
        }
        if (dir.cdUp() == false) {
            break;
        }
    }

    if (info->root_type == 0) {
        return false;
    }
    return true;
}

bool PathUtil::parseRequest(const QString &path, const QString &request, int cursor, IsRootFunction is_root, RequestInfo *info)
{
    if (cursor == -1) {
        cursor = request.length();
    }
    findSegment(request, cursor, &info->begin, &info->end);

    QString request_prefix(request.mid(0, info->begin));
    QString request_segment(request.mid(info->begin, info->end - info->begin));

    QFileInfo f(request_prefix);
    if (f.isRelative()) {
        f.setFile(path + "/" + request_prefix);
    }

    QString base(f.absoluteFilePath());
    if (!findRoot(base, is_root, info)) {
        return false;
    }

    info->request = base.mid(info->path.length()) + request_segment;
    // This is experimental
    info->begin = info->end - info->request.length();

    // Chop the part between root end and path end, so we get a prefix really existing.
    info->prefix = base.mid(info->root.length(), info->path.length() - info->root.length());

    return true;
}
