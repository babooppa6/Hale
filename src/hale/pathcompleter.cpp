#include <QDirIterator>
#include <QCoreApplication>
#include <QThread>
#include <QTimer>

#include "util.h"
#include "pathutil.h"
#include "pathcompleter.h"

/// Maximum batch size for indexing.
static const int MAX_INDEX_BATCH = 50;
/// Maximum batch size for filtering.
static const int MAX_FILTER_BATCH = 500;
/// Maximum results for filtering.
static const int MAX_FILTER_RESULTS = 10;

PathCompleter::PathCompleter(QObject *parent)
    : QObject(parent)
{
    qRegisterMetaType<QSharedPointer<PathIndex> >("QSharedPointer<PathIndex>");
    qRegisterMetaType<QSharedPointer<PathIndexFilter> >("QSharedPointer<PathIndexFilter>");
    qRegisterMetaType<IndexType>("IndexType");
    qRegisterMetaType<FilterType>("FilterType");

    m_index_thread = new QThread(this);
    m_index_thread->setObjectName("IndexThread");

    m_index = new PathCompleterWorker(NULL);
    m_index->moveToThread(m_index_thread);
    connect(m_index, &PathCompleterWorker::filterResults, this, &PathCompleter::filterResults, Qt::QueuedConnection);

    m_index_thread->start();
    // connect(m_index_thread, SIGNAL(started()), m_index, SLOT(work()));
}

PathCompleter::~PathCompleter()
{
    // metaObject()->invokeMethod(m_index, "quit", Qt::QueuedConnection);
    m_index_thread->quit();
    m_index_thread->wait();

    delete m_index;
}

QSharedPointer<PathIndexFilter> PathCompleter::completePath(const QString &request, int cursor, FilterType type)
{
    QString working_dir(QDir::currentPath());
    QSharedPointer<PathIndexFilter> filter;
    // int handle = -1;
    QMetaObject::invokeMethod(m_index, "completePath",
                              Qt::BlockingQueuedConnection,
                              Q_RETURN_ARG(QSharedPointer<PathIndexFilter>, filter),
                              Q_ARG(QString, working_dir),
                              Q_ARG(QString, request),
                              Q_ARG(int, cursor),
                              Q_ARG(FilterType, type)
                              );
    return filter;
}

//
//
//

PathIndex::PathIndex(const QString &p, IndexType type) :
    path(p),
    type(type),
    lock(QReadWriteLock::Recursive)
{
    Q_ASSERT(!path.isEmpty());

    if (!path.endsWith("/")) {
        path.append("/");
    }

    QFileInfo fi(path);
    Q_ASSERT(fi.exists() && fi.isReadable() && fi.isDir());
    path = fi.canonicalFilePath();
    if (!path.endsWith("/")) {
        path.append("/");
    }

    add_queue.append(path);

    batch = 0;
}

bool PathIndex::index(int index_batch_size, int filter_batch_size)
{
    if (isFinished()) {
        // We are done.
        return true;
    }

    QString path(add_queue.front());
    add_queue.pop_front();

    QDir::Filters filters = QDir::Dirs | QDir::Files | QDir::NoDot | QDir::NoDotDot | QDir::Readable;
    QDirIterator::IteratorFlags flags = QDirIterator::FollowSymlinks;
    QDirIterator it(path, filters, flags);

    Entry entry;
    QFileInfo info;
    while (it.hasNext())
    {
        it.next();
        info = it.fileInfo();

        entry.path = it.filePath().remove(0, PathIndex::path.length());
        if (info.isDir()) {
            if (type == IndexType::All) {
                add_queue.append(it.filePath());
            }
            entry.path.append("/");
        }

        // qDebug() << __FUNCTION__ << " - " << entry.path;
        entry.d = StringDistance::NO_MATCH;
        entries.append(entry);

        batch++;
        if (batch == index_batch_size) {
            yield(filter_batch_size);
            batch = 0;
        }
    }

    if (isFinished()) {
        yield(filter_batch_size);
    }

    return isFinished();
}

void PathIndex::yield(int filter_batch_size)
{
    // This does not seem to trigger the timer in the PathCompleterWorker.
    QCoreApplication::processEvents(QEventLoop::AllEvents);

    // If we have a filter, we will advance it.
    if (filter && !filter->isFinished()) {
        filter->filter(filter_batch_size);
    }
}

//
//
//

PathIndexFilter::PathIndexFilter(QSharedPointer<PathIndex> index, const QString &prefix, const QString &request, int begin, int end, FilterType type) :
    root(index),
    type(type),
    prefix(prefix),
    request(request.toLower()),
    replacement_begin(begin),
    replacement_end(end)
{
    index_position = 0;
}

bool PathIndexFilter::isFinished()
{
    return index_position == root->entries.count();
}

bool PathIndexFilter::filter(int batch_size)
{
    if (isFinished()) {
        return true;
    }

    if (root->type == IndexType::Top && request.isEmpty()) {
        // Return all entries in root sorted.
        QStringList r;
        for (auto entry : root->entries) {
            r.append(entry.path);
        }
        // TODO: Add flag whether entry is a directory or not.
        qSort(r.begin(), r.end(), [](const QString &a, const QString &b) {
            int a_dir = a.endsWith("/");
            int b_dir = b.endsWith("/");
            if (a_dir == b_dir) {
                return QString::compare(a, b) < 0;
            }
            return a_dir < b_dir;
        });

        index_position = root->entries.count();

        emit PathIndexFilter::resultsAvailable(r);
    }
    else
    {
        filter(&root->entries, batch_size);
    }

    return isFinished();
}

void PathIndexFilter::filter(PathIndex::Entries *entries, int batch_size)
{
    int matched = 0;
    int tested = 0;
    while (index_position < entries->count() && tested < batch_size)
    {
        PathIndex::Entry &entry = (*entries)[index_position];
        if (entry.path.startsWith(prefix)) {
            QString path(entry.path.mid(prefix.length()).toLower());
            entry.d = StringDistance::pathDistance(request.constData(),
                                                   request.length(),
                                                   path.constData(),
                                                   path.length(),
                                                   "/"
                                                   );
        } else {
            entry.d = -1;
        }

        if (entry.d != -1) {
            results.append(&entry);
            matched++;
        }

        index_position++;
        tested++;
    }

    if (matched > 0) {
        sort();
    }
}

void PathIndexFilter::sort()
{
    // Sort the filtered results.
    qSort(results.begin(), results.end(), [](const PathIndex::Entry *a, const PathIndex::Entry *b) {
        if (a->d == b->d) {
            if (a->path.length() == b->path.length()) {
                return QString::compare(a->path, b->path) < 0;
             }
            return a->path.length() < b->path.length();
        }
        return a->d < b->d;
    });

    int count = 0;
    QStringList r;

    if (type == FilterType::Recursive) {
        for (auto entry : results) {
            r.append(entry->path.mid(prefix.length()));
            count++;
            if (count == MAX_FILTER_RESULTS) {
                break;
            }
        }
        qDebug() << __FUNCTION__ << "recursive results" << r.size();
    } else {
        for (auto entry : results) {
            r.append(entry->path.mid(prefix.length()));
        }
        qDebug() << __FUNCTION__ << "single results" << r.size();
    }

    emit PathIndexFilter::resultsAvailable(r);
}

//
//
//

PathCompleterWorker::PathCompleterWorker(QObject *parent) :
    QObject(parent)
{
    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(work()));

    m_root_files << ".git" << ".svn" << ".hale";
}

PathCompleterWorker::~PathCompleterWorker()
{
    m_roots.clear();
}

int PathCompleterWorker::isRoot(const QString &path)
{
    QDir dir(path);
    for (auto name : m_root_files) {
        if (dir.exists(name)) {
            return (int)IndexType::All;
        }
    }
    return (int)IndexType::Top;
}

QSharedPointer<PathIndexFilter> PathCompleterWorker::completePath(const QString &path, const QString &request, int cursor, FilterType type)
{
    using namespace std::placeholders;
    PathUtil::RequestInfo req;
    if (!PathUtil::parseRequest(path, request, cursor, std::bind(&PathCompleterWorker::isRoot, this, _1), &req)) {
        qDebug() << "Unable to find root.";
        return QSharedPointer<PathIndexFilter>();
    }

    qDebug() << __FUNCTION__
             << "root_type" << req.root_type
             << "root" << req.root
             << "prefix" << req.prefix
             << "request" << req.request
             << "begin" << req.begin
             << "end" << req.end;

    IndexType index_type;
    FilterType filter_type;

    switch (req.root_type) {
    case (int)IndexType::All:
        index_type = IndexType::All;
        filter_type = FilterType::Recursive;
        break;
    case (int)IndexType::Top:
        index_type = IndexType::Top;
        filter_type = FilterType::Single;
        break;
    default:
        qDebug() << __FUNCTION__ << "Invalid index type." << req.root_type;
        return QSharedPointer<PathIndexFilter>();
    }

    if (m_filter) {
        m_filter->root->filter.reset();
        m_filter->root.reset();
    }

    auto root = addRoot(req.root, index_type);

    m_filter.reset(new PathIndexFilter(root, req.prefix, req.request, req.begin, req.end, filter_type));
    m_filter->moveToThread(QThread::currentThread());
    root->filter = m_filter;

    scheduleWork();

    return m_filter;
}

//
//
//

void PathCompleterWorker::work()
{
    static int recursion = 0;
    Q_ASSERT(recursion == 0);

    recursion++;

    int pending = 0;
    int processed = 0;

    QSharedPointer<PathIndex> filter_root;
    if (m_filter)
    {
        filter_root = m_filter->root;
        // If we have a filter, then it's root has highest priority in indexing.
        // So we take a next indexing job from the queue and run it.
        if (!m_filter->root->isFinished()) {
            processed++;
            if (m_filter->root->index(MAX_INDEX_BATCH, MAX_FILTER_BATCH) == false) {
                pending++;
            }
            // We have run the filter internally within the index(),
            // so no need to run the filter again.
        } else if (!m_filter->isFinished()) {
            processed++;
            if (m_filter->filter(MAX_FILTER_BATCH) == false) {
                pending++;
            }
        }
    }

    if (processed == 0)
    {
        // No filter is active, so we will run index any root that is not finished indexing.
        if (!m_index_jobs.empty()) {
            Roots::iterator it = m_roots.end();
            while (it != m_roots.begin()) {
                it--;
                if (!(*it)->isFinished()) {
                    processed++;
                    if ((*it)->index(MAX_INDEX_BATCH, MAX_FILTER_BATCH) == false) {
                        pending++;
                    }
                    break;
                }
            }
        }
    }

    // qDebug() << __FUNCTION__ << "pending" << pending << "processed" << processed;
    if (pending == 0) {
        qDebug() << __FUNCTION__ << "FINISHED";
        m_timer->stop();
    }

    recursion--;
}

void PathCompleterWorker::scheduleWork()
{
    if (!m_timer->isActive()) {
        m_timer->start();
    }
}

//
//
//

QSharedPointer<PathIndex> PathCompleterWorker::addRoot(const QString &path, IndexType type)
{
    Roots::iterator it = m_roots.find(path);
    if (it != m_roots.end()) {
        return it.value();
    }

    // Erase all single roots we already had.
    it = m_roots.begin();
    while (it != m_roots.end()) {
        if (it.value()->type == IndexType::Top) {
            m_roots.erase(it);
            break;
        }
        it++;
    }

    auto root = QSharedPointer<PathIndex>(new PathIndex(path, type));
    m_roots.insert(root->path, root);
    return root;
}
