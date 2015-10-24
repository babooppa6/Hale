#ifndef UNDOSTREAM_H
#define UNDOSTREAM_H

#include <functional>
#include <QBuffer>

#include "documenttypes.h"

class UndoStream : public QObject
{
    Q_OBJECT
public:
    bool debug;

    UndoStream(QObject *parent);

    void copyFrom(const UndoStream *other);

    enum EventType
    {
        EventType_Break = 0,
        EventType_User,
    };

    void writeBreak();

    typedef std::function<void(int event, QDataStream &s)> ReadFunc;
    typedef std::function<void(int event, QDataStream &s, QTextStream &output)> DumpFunc;
    typedef std::function<void(QDataStream &s)> WriteFunc;

    void dump(DumpFunc f);

    void write(int event_type, WriteFunc f);

    void undo(ReadFunc f);
    void redo(ReadFunc f);

    qint64 writingHead() const
    { return m_head_write; }
    qint64 readingHead() const
    { return m_head_read; }

private:
    /// Memory-based buffer.
    QByteArray *m_buffer_memory;
    /// Buffer
    QBuffer *m_buffer;

    /// Where the head is at.
    qint64 m_head_read;
    qint64 m_head_write;

    bool dumpEvent(DumpFunc f, QDataStream &s, QTextStream &output);
    bool readEvent(ReadFunc f, QDataStream &s, qint64 *head);

    void writeHeader(QDataStream &s, int event_type);
};

///*
// * Todo:
// * - Way to remember the selection
// * - Remove Document ptr as member.
// *
// **/

//class Document;
//class DocumentEditEvent;

//class DocumentUndoItem
//{
//public:
//    DocumentUndoItem(DocumentEditEvent *event);

//    Document *document();
//    int offset();
//    const QString &text();

//    virtual void undo(DocumentEdit edit);
//    virtual void redo(DocumentEdit edit);

//protected:
//    DocumentEditEvent *m_event;
//};

//class TextInsertedDocumentUndoItem : public DocumentUndoItem
//{
//public:
//    TextInsertedDocumentUndoItem(QSharedPointer<DocumentTextEvent> event);

//    virtual void undo(DocumentEdit edit);
//    virtual void redo(DocumentEdit edit);
//};

//class TextRemovedDocumentUndoItem : public DocumentUndoItem
//{
//public:
//    TextRemovedDocumentUndoItem(QSharedPointer<DocumentTextEvent> event);

//    virtual void undo(DocumentEdit edit);
//    virtual void redo(DocumentEdit edit);
//};

//class DocumentUndo : public TUndo<DocumentEdit, DocumentUndoItem>
//{
//public:
//    DocumentUndo() {};
//};

#endif // UNDOSTREAM_H
