#ifndef DOCUMENTTYPES_H
#define DOCUMENTTYPES_H

#include <QDebug>
#include <QString>
#include <QMetaType>
#include <QScopedPointer>

class Document;
class DocumentModel;

struct _DocumentEdit
{
    _DocumentEdit() :
        m_view(NULL),
        m_document(NULL),
        m_undo(false),
        m_internal(false)
    {
    }

    bool isOpen() const
    { return m_document != NULL; }
    bool isUndo() const
    { return m_undo; }
    bool isInternal() const
    { return m_internal; }

    DocumentModel *view() const
    { return m_view; }

    void commit();
    void revert();

private:
    friend class Document;

    bool m_undo;
    DocumentModel *m_view;
    Document *m_document;
    bool m_internal;

    Q_DISABLE_COPY(_DocumentEdit)
};

typedef QSharedPointer<_DocumentEdit> DocumentEdit;

class DocumentPosition
{
public:

    DocumentPosition() : block(0), position(0) {}
    DocumentPosition(int block, int position) : block(block), position(position) {}
    DocumentPosition(const DocumentPosition& other) :
        block(other.block),
        position(other.position)
    {}
    DocumentPosition &operator =(const DocumentPosition &other)
    {
        block = other.block;
        position = other.position;
        return *this;
    }

    bool operator ==(const DocumentPosition &other) const
    { return block == other.block && position == other.position; }
    bool operator !=(const DocumentPosition &other) const
    { return block != other.block || position != other.position; }

    bool operator >(const DocumentPosition &other) const
    {
        if (block == other.block) {
            return position > other.position;
        }
        return block > other.block;
    }

    bool operator <(const DocumentPosition &other) const
    {
        if (block == other.block) {
            return position < other.position;
        }
        return block < other.block;
    }

    bool operator >=(const DocumentPosition &other) const
    {
        if (block == other.block) {
            return position >= other.position;
        }
        return block >= other.block;
    }

    bool operator <=(const DocumentPosition &other) const
    {
        if (block == other.block) {
            return position <= other.position;
        }
        return block <= other.block;
    }

    void swap(DocumentPosition &other)
    {
        DocumentPosition tmp = other;
        other = *this;
        *this = tmp;
    }

    int block;
    int position;
};

Q_DECLARE_METATYPE(DocumentPosition)

QDebug operator<<(QDebug dbg, const DocumentPosition &p);

class DocumentRange
{
public:
    DocumentRange()
    {}

    explicit DocumentRange(const DocumentPosition &position) :
        first(position),
        second(position)
    {}


    DocumentRange(const DocumentPosition &first, const DocumentPosition &second) :
        first(first),
        second(second)
    {}

    DocumentRange(const DocumentRange &other) :
        first(other.first),
        second(other.second)
    {}

    DocumentRange(const DocumentRange &&other) :
        first(other.first),
        second(other.second)
    {}

    DocumentRange &operator =(const DocumentRange &other)
    {
        first = other.first;
        second = other.second;
        return *this;
    }

    DocumentRange &operator =(const DocumentRange &&other)
    {
        if (this != &other) {
            first = other.first;
            second = other.second;
        }
        return *this;
    }

    bool operator ==(const DocumentRange &other) const
    {
        return first == other.first && second == other.second;
    }

    bool operator !=(const DocumentRange &other) const
    {
        return first != other.first || second != other.second;
    }

    void sort() {
        if (first > second) {
            first.swap(second);
        }
    }

    void reverse() {
        first.swap(second);
    }

    DocumentRange sorted() const {
        DocumentRange p(*this);
        p.sort();
        return p;
    }

    bool intersectsSorted(const DocumentRange &other_sorted) const {
        if (other_sorted.second < first || other_sorted.first > second) {
            return false;
        }
        return true;
    }

    DocumentPosition begin() const {
        if (first < second) {
            return first;
        }
        return second;
    }

    DocumentPosition end() const {
        if (first < second) {
            return second;
        }
        return first;
    }

    void sorted(DocumentPosition *o_begin, DocumentPosition *o_end) const
    {
        if (first < second) {
            *o_begin = first;
            *o_end = second;
        } else {
            *o_begin = second;
            *o_end = first;
        }
    }

    void setSorted(DocumentPosition begin, DocumentPosition end) {
        if (first < second) {
            first = begin;
            second = end;
        } else {
            first = end;
            second = begin;
        }
    }

    DocumentPosition first;
    DocumentPosition second;
};

Q_DECLARE_METATYPE(DocumentRange)

QDebug operator<<(QDebug dbg, const DocumentRange &r);

class DocumentEditEvent
{
public:
    enum EditType
    {
        Insert,
        Remove,
    };

    EditType type;
    /// Index of the block that has changed it's text.
    int block_text_change;
    /// Index at which the new blocks were inserted or removed
    int block_list_change_first;
    /// Number of new blocks inserted or removed.
    int block_list_change_count;

    /// Document position where the change began.
    DocumentPosition begin;
    /// Document position where the change end.
    /// NOTE: END position is important when we are removing text.
    DocumentPosition end;

    /// Offset where the change began.
    int offset_begin;
    /// Offset where the change ended.
    int offset_end;

    /// Set to true if this operation is undo.
    bool undo;
    /// Undo head.
    int undo_head;
    /// Set to the DocumentView that did the change.
    DocumentModel *sender;
};

struct LineEndingImpl;

class LineEnding
{
public:
    static LineEnding UNKNOWN;
    static LineEnding CR;
    static LineEnding LF;
    static LineEnding CRLF;

private:
    explicit LineEnding(LineEndingImpl * impl);
    LineEndingImpl * m_impl;

public:
    LineEnding();
    LineEnding(const LineEnding &other);

    /// Returns length of line ending.
    int length();
    /// Returns string representing the line ending.
    const QString &string();

    bool operator ==(const LineEnding &other) const {
        return m_impl == other.m_impl;
    }

    LineEnding &operator =(const LineEnding &other) {
        m_impl = other.m_impl;
        return *this;
    }

public:
    /// Finds next line ending.
    template<class It>
    static LineEnding findNext(It * io_begin, It end)
    {
        for (; (*io_begin) != end; (*io_begin)++)
        {

            switch ((**io_begin).unicode())
            {
            case '\r':
                if (((*io_begin)+1) != end && *((*io_begin)+1) == '\n') {
                    (*io_begin) += 2;
                    return CRLF;
                } else {
                    (*io_begin)++;
                    return CR;
                }
            case '\n':
                (*io_begin)++;
                return LF;
            }
        }

        return UNKNOWN;
    }
};

//
//
//

#endif // DOCUMENTTYPES_H
