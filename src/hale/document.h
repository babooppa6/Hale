#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <QObject>
#include <QUuid>
#include <QDir>
#include <QTextLayout>
#include <QDateTime>

#include "luaobject.h"

#include "enums.h"

#include "documenttypes.h"
#include "tdocumentbuffer.h"
#include "documentparser.h"
#include "undostream.h"
#include "grammar.h"
#include "theme.h"
#include "parser.h"

class DocumentModel;
class UndoStream;

struct _DocumentEdit;

class DocumentBlocks
{
public:
    DocumentBlocks();

    /// Inserts lines
    void insert(int offset, int text_length, const std::vector<int> & offsets, int *o_line_begin, int *o_line_end);
    /// Removes lines
    void remove(int offset, int text_length, int *o_line_begin, int *o_line_end);

    /// Returns count of lines.
    int count() const
    { return (int)m_offsets.size(); }

    /// Returns line length
    int blockLength(int index) const;
    /// Returns line begin offset
    int blockBegin(int index) const;
    /// Returns line end offset
    int blockEnd(int index) const;
    /// Returns line from given offset
    int blockAt(int offset, int search_begin = 0, int search_end = -1) const;
    /// Returns line range from begin and end offsets
    void blockRangeAt(int begin, int end, int * o_line_begin, int * o_line_end);

private:
    typedef std::vector<int> Offsets;
    Offsets m_offsets;
};

//
//
//

class Document : public QObject, public LuaObject
{
    Q_OBJECT

public:
    static const int MAX_INDENTATION_SIZE = 16;

    typedef TDocumentBuffer<QChar, int, int> Buffer;

    explicit Document(QObject *parent = 0);

    enum UndoEvent
    {
        UndoEvent_Insert = UndoStream::EventType_User,
        UndoEvent_Remove,
    };

    void setID(QUuid id);
    QUuid id() const {
        return m_id;
    }

    void setPath(const QString &path);
    QString path() const {
        return m_file_path;
    }

    IndentationMode indentation_mode;
    int indentation_size;
    void setIndentationMode(IndentationMode indentation_mode);
    void setIndentationSize(int indentation_size);

    // Sets hint whether this document is visible.
    // This starts or stops the parser.
    void setActiveHint(bool active);


    QString grammarName();
    void setGrammarName(const QString &grammar);

    QSharedPointer<Grammar> grammar() {
        return m_parser.grammar();
    }
    void setGrammar(QSharedPointer<Grammar> grammar);

    QSharedPointer<Theme> theme() {
        return m_parser.theme();
    }

    void setTheme(QSharedPointer<Theme> theme) {
        m_parser.setTheme(theme);
    }

    void addModel(DocumentModel *model);
    void removeModel(DocumentModel *model);

    typedef QList<DocumentModel*> Models;
    Models models;

signals:
    void pathChanged(const QString &path);
    void grammarChanged(const QSharedPointer<Grammar> grammar);
    void grammarNameChanged(const QString &grammar_name);

    void modelAdded(DocumentModel *model);
    void modelRemoved(DocumentModel *model);

    // void grammarNameChanged();

public:
    DocumentPosition offsetToBlockPosition(int offset);
//    DocumentPosition offsetToBlockPositionVirtual(int offset);
    int blockPositionToOffset(const DocumentPosition &position);
//    int blockPositionToOffsetVirtual(const DocumentPosition &position);

    int minus(const DocumentPosition &a, const DocumentPosition &b);
    DocumentPosition plus(const DocumentPosition &a, int n);

    DocumentRange range(int begin, int end);

    bool checkOffset(int offset);
    bool checkBlockPosition(const DocumentPosition *position);

    void setText(DocumentEdit edit, const QString &text);
    DocumentPosition insertText(DocumentEdit edit, const DocumentPosition &position, const QString &text);
    DocumentPosition appendText(DocumentEdit edit, const QString &text);
    void removeText(DocumentEdit edit, const DocumentPosition &position, int length);

    QString text();
    QString text(const DocumentPosition &position, int length);
    QString text(const DocumentPosition &begin, const DocumentPosition &end);
    QString text(int position, int length);

    QChar charAt(int position);

    bool isEmpty() const
    { return length() == 0; }
    int length() const;

    int blockCount();
    QString blockText(int block, bool include_line_ending = true);
    int blockAt(int position);
    int blockBeginOffset(int block);
    int blockEndOffset(int block);
    int blockLength(int block);
    int blockLengthWithoutLE(int block);


    bool blockEmpty(int block);
    inline int blockColumn(const DocumentPosition &position) {
        return blockColumn(position.block, position.position);
    }
    int blockColumn(int block, int position);

    int blockIndentation(int block);

    int indentationForBlock(int block);

    // TODO(cohen): Join this with document blocks.

    struct Block
    {
        enum Flags
        {
            F_FormatsInvalidated = 0x01,
            F_TokensInvalidated = 0x02,
            F_TextChangeNotified = 0x04
        };

        Parser::Tokens tokens;
        Parser::Stack stack;
        // TODO(cohen): Do not cache this.
        // - It is cached at view so we don't need to store it at all.
        QList<QTextLayout::FormatRange> formats;
        quint32 flags;
    };

    Block *blockData(int block);

    QList<QTextLayout::FormatRange> *blockFormats(int block);

signals:
    void beforeTextChanged();
    void textChanged(const DocumentEditEvent *event);
    void beforeFormatChanged();
    void formatChanged(int block);
    void indentationModeChanged();
    void indentationSizeChanged();

protected:
    void onBlockTextChanged(int block);

public slots:

private:
    QUuid m_id;
    QString m_file_path;
    bool m_active;

    Buffer m_buffer;
    QString m_grammar_name;
    // TODO: Merge with document.
    DocumentBlocks m_blocks;
    QVector<Block*> m_blocks_storage;
    DocumentParser m_parser;

    //
    // Undo
    //

public:
    void undo();
    void redo();

signals:
    void undoTriggered();
    void redoTriggered();

private:
    UndoStream *m_undo;

    int insertAndConvertText(int offset, const QChar *text, int text_length, std::vector<int> *o_line_offsets);
    void readUndo(DocumentEdit edit, QDataStream &s, int event, bool redo);
    void writeUndo(DocumentEdit edit, int type, int offset, int length);
    void dumpUndo();

    //
    // Edit
    //

public:
    DocumentEdit editInternal();
    DocumentEdit edit(DocumentModel *view);
    bool isEditing();
    DocumentModel *editingView();

signals:
    void editBegin(DocumentModel *view, bool undo);
    void editEnd(DocumentModel *view, bool undo);

private:
    friend struct _DocumentEdit;
    _DocumentEdit m_edit;
    void commit(_DocumentEdit *edit);
    void revert(_DocumentEdit *edit);
    void checkEdit(DocumentEdit edit);

    DocumentEdit editUndo();
};

LUAOBJECT_DECLARE(Document)

//
//
//

class DocumentManager : public QObject
{
    Q_OBJECT
public:
    explicit DocumentManager(QObject *parent = 0);

    struct Descriptor {
        QUuid id;
        QString source_path;
        QString grammar;
        QDateTime source_modified;
        Document *document;
    };

    Document *newDocument();
    Document *loadDocument(const QString &path);
    Document *loadDocument(const QUuid &id);
    Document *loadDocument(Descriptor *descriptor);
    bool saveDocument(Document *document);
    bool saveDocument(Document *document, const QString &path);
    void closeDocument(Document *document);

    Document *document(const QUuid &id);
    Document *document(const QString &path);

    void loadState();
    void saveState();

    Descriptor *attachDescriptor(Descriptor *descriptor);
    void detachDescriptor(Descriptor *descriptor);
    void attachDocument(Descriptor *descriptor);
    void detachDocument(Descriptor *descriptor);
    void updateDocument(Descriptor *descriptor);

    typedef QList<Document *> Documents;
    Documents documents;

    Descriptor *getDescriptor(const QString &path);
    Descriptor *getDescriptor(const QUuid &id);
    Descriptor *getDescriptor(const Document *document);

    typedef QList<Descriptor> Descriptors;
    Descriptors descriptors;

    QString storage_path;

signals:
    void documentCreated(Document *document);
    void documentLoaded(Document *document);
    void documentSaved(Document *document);
    void documentClosed(Document *document);

private slots:
    void themeChanged(QSharedPointer<Theme> theme);
    void documentModelRemoved(DocumentModel *model);

private:
    Document *loadDocumentInternal(const QString &path, Descriptor *descriptor);
    bool saveDocumentInternal(Document *document, const QString &path);

    Descriptor *newDescriptor(Document *document);
    bool saveDescriptor(const Descriptor *descriptor);
    Descriptor *loadDescriptor(const QUuid &id);

    QString descriptorStoragePath(const QUuid &id, const char *ext);
    QString descriptorStoragePath(const Descriptor *descriptor, const char *ext);
    QDir storageDir();
};

#endif // DOCUMENT_H
