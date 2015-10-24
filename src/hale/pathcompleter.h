#ifndef PATHINDEXER_H
#define PATHINDEXER_H

#include <QObject>
#include <QStringList>
#include <QReadWriteLock>
#include <QList>
#include <QStack>
#include <QMap>
#include <QSharedPointer>
#include <QTimer>
#include <QElapsedTimer>

class PathCompleterWorker;
class PathIndex;
class PathIndexFilter;

enum struct FilterType
{
    Single,
    Recursive
};

enum struct IndexType
{
    Top = 1,
    All
};

class PathCompleter : public QObject
{
    Q_OBJECT
public:
    explicit PathCompleter(QObject *parent = 0);
    ~PathCompleter();

    QSharedPointer<PathIndexFilter> completePath(const QString &path, int cursor, FilterType type);

//    QSharedPointer<PathIndexRoot> addRoot(const QString &root);
//    QSharedPointer<PathIndexRoot> root(int index);

//    void setFilter(QSharedPointer<PathIndexRoot> root, const QString &needle);
//    void clearFilter();

signals:
    void filterResults(int handle, int position, const QStringList &results);

private:
    QThread *m_index_thread;
    PathCompleterWorker *m_index;
};

//
//
//

// TODO: Do we need QObject here?
class PathIndex : public QObject
{
    Q_OBJECT
public:
    PathIndex(const QString &path, IndexType type);

    IndexType type;

    // Root path.
    QString path;
    // Lock.
    QReadWriteLock lock;

    struct Entry {
        QString path;
        // Distance of the path against the current filter.
        int d;
    };

    // Index.
    typedef QList<Entry> Entries;
    Entries entries;

    // Current filter.
    QSharedPointer<PathIndexFilter> filter;
    int batch;

    // Queue
    QStringList add_queue;

    bool isFinished() {
        return add_queue.isEmpty();
    }

    bool index(int index_batch_size, int filter_batch_size);
    void yield(int filter_batch_size);
};

//
//
//

class PathIndexFilter : public QObject
{
    Q_OBJECT
public:
    PathIndexFilter(QSharedPointer<PathIndex> root, const QString &prefix, const QString &request, int begin, int end, FilterType type);

    bool isFinished();
    bool filter(int batch_size);

    FilterType type;
    QSharedPointer<PathIndex> root;
    QString request;
    QString prefix;
    int replacement_begin;
    int replacement_end;
    int index_position;

    typedef QList<PathIndex::Entry *> Results;
    Results results;

    void filter(PathIndex::Entries *entries, int batch_size);
    void sort();

signals:
    void resultsAvailable(const QStringList &results);
};

//
//
//

class PathCompleterWorker : public QObject
{
    Q_OBJECT
public:
    PathCompleterWorker(QObject *parent);
    ~PathCompleterWorker();

public slots:
    // Completes given path
    QSharedPointer<PathIndexFilter> completePath(const QString &path, const QString &request, int cursor, FilterType type);

    // Sets the filter.
    // void setFilter(QSharedPointer<PathIndexRoot> root, const QString &needle);

signals:
    void filterResults(int handle, int position, const QStringList &results);

private slots:
    // void removePath(QSharedPointer<PathIndexRoot> root, const QString &path);
    void work();

private:
    struct IndexJob
    {
        enum Type {
            AddPath,
            RemovePath
        };

        Type type;
        QSharedPointer<PathIndex> root;
        QString path;
    };

    QTimer *m_timer;

    typedef QList<QString> RootFiles;
    RootFiles m_root_files;

    int isRoot(const QString &path);

    typedef QMap<QString, QSharedPointer<PathIndex>> Roots;
    Roots m_roots;

    /// Indexing job queue. (Filter job is kept in m_filter_job)
    typedef QList<IndexJob> IndexJobs;
    IndexJobs m_index_jobs;

    /// Runs current indexing job.
    int index();

    bool index(IndexJob &job);

    /// Adds a new root to the index.
    QSharedPointer<PathIndex> addRoot(const QString &path, IndexType type);
    /// Indexes given path and adds more indexing jobs.
    int addPath(QSharedPointer<PathIndex> root, const QString &path);
    /// Adds indexing job to queue.
    void addIndexJob(IndexJob::Type type, QSharedPointer<PathIndex> root, const QString &path);

    // There is only one filter active at a time.
    QSharedPointer<PathIndexFilter> m_filter;

    /// Runs current filtering job.
    bool filter();

    /// Schedules queued call to work().
    void scheduleWork();

    /// Finds root.
    void findRoot(const QString &path, const QString &request, int begin, int end, FilterType type);
};

#endif // PATHINDEXER_H
