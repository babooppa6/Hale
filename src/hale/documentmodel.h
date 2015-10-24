#ifndef DOCUMENTVIEW_H
#define DOCUMENTVIEW_H

#include <QObject>
#include <QVector>
#include <QTextLayout>
#include <QJsonObject>

#include "enums.h"
#include "model.h"
#include "documenttypes.h"
#include "option.h"

class QTextLayout;
class QFont;
class QFontMetrics;
class QItemSelectionModel;
class QAbstractItemModel;

class UndoStream;

class Document;
class DocumentCursor;
class DocumentEditor;
class StatusLineController;

class Grammar;

class Theme;

class CompletionController;

struct lua_State;



struct GutterOptions : public CommonOption
{
//    enum struct GutterType
//    {
//        Numbers,
//        Circle,
//        ArrowRight
//    };

    GutterOptions()
    {
        // type = GutterType::Numbers;
        visible = true;
        padding = QMarginsF(16, 0, 16, 0);
        format = "%";
    }

    QString format;
    // GutterType type;

    bool operator ==(const GutterOptions &other) const {
        return memcmp(this, &other, sizeof(GutterOptions)) == 0;
    }

    bool operator !=(const GutterOptions &other) const {
        return memcmp(this, &other, sizeof(GutterOptions)) != 0;
    }
};

static void set(GutterOptions *option, QJsonValue value)
{
    set((CommonOption*)option, value);
    if (value.type() != QJsonValue::Object) {
        return;
    }
    QJsonObject o(value.toObject());
    set(&option->format, o["format"]);
}

template<typename T>
struct StateOptions {
    typedef QMap<QString, T> States;
    States states;
};

struct DocumentOptions : public CommonOption
{
    DocumentOptions() : scroll_threshold(0) {}
    GutterOptions gutter;
    CommonOption block;
    CommonOption status;

    qreal scroll_threshold;
};


class DocumentBlock
{
public:
    DocumentBlock(DocumentModel *view, int index);
    DocumentBlock(const DocumentBlock &other) :
        m_view(other.m_view),
        m_index(other.m_index)
    {}
    DocumentBlock &operator =(const DocumentBlock &other) {
        m_index = other.m_index;
        m_view = other.m_view;
        return *this;
    }

    int index() const
    { return m_index; }
    void setIndex(int index);

    bool isValid() const;
    bool isFirst() const;
    bool isLast() const;

    DocumentBlock next() const;
    DocumentBlock previous() const;

    int beginOffset() const;
    int endOffset() const;

    int length() const;
    int lengthWithoutLE() const;

    /// Same as lengthWithoutLE.
    int end() const {
        return lengthWithoutLE();
    }

    QTextLayout *layout() const;

private:
    DocumentModel *m_view;
    int m_index;
};

class DocumentModel : public Model
{
    // NOTE(cohen) To access insertText/removeText.
    friend class DocumentCursor;

    Q_OBJECT

public:
    explicit DocumentModel(Document *document, QObject *parent = NULL);
    ~DocumentModel();

    bool close();

    void scope(ScopePath *o_path);
    Model *split(QObject *parent);

    void fromJson(const QJsonObject &j);
    void toJson(QJsonObject &j);

    void configure(QJsonObject &options);
    void configure(const DocumentOptions &options);
    // void copyFrom(const DocumentView *other);

    Document *document();
    QString description();

    StatusLineController *statusLineController();

    DocumentCursor *cursor()
    { return m_cursor; }
    void addCursor(const DocumentRange &r);
    void addCursor(const DocumentPosition &p);
    void resetCursors();
    void setSelection(int begin, int end);

    void findCursors(const DocumentRange &r, QList<DocumentCursor*> *cursors);

    void setEditor(DocumentEditor *editor);
    DocumentEditor *editor()
    { return m_editor; }

    void clear(DocumentEdit edit);

    void copy();
    void cut(DocumentEdit edit);
    void paste(DocumentEdit edit);

//    void setFocused(bool focused);

    qreal lineHeight();

    enum MoveFlags
    {
        MoveFlag_Select = 0x01,
        MoveFlag_VerticalAnchor = 0x02
    };

    enum RemoveCommand
    {
        Remove_All,
        Remove_Selected,
        Remove_CharacterBackward,
        Remove_CharacterForward
    };

    enum MoveCommand
    {
        Move_CharacterNext,
        Move_CharacterPrevious,
        Move_WordNext,
        Move_WordPrevious,
        Move_LineNext,
        Move_LinePrevious,
        Move_LineBegin,
        Move_LineEnd,
        Move_BlockBegin,
        Move_BlockEnd,
        Move_PageNext,
        Move_PagePrevious,
        Move_DocumentBegin,
        Move_DocumentEnd
    };

    void insertText(DocumentEdit edit, const QString &text);
    void insertIndentation(DocumentEdit edit);
    void insertNewLine(DocumentEdit edit);

    void removeText(DocumentEdit edit, RemoveCommand command);

    void moveCursor(MoveCommand command, int flags = 0);
    void moveCursorTo(const DocumentPosition &position, int flags = 0);
    void moveCursorTo(int offset, int flags = 0);


    void undo();
    void redo();

    QString text();
    QString text(int offset, int length);
    QString text(const DocumentPosition &position, int length);
    QString text(const DocumentPosition &begin, const DocumentPosition &end);

    /// Returns API convenience class for managing blocks.
    DocumentBlock block(int index);

    QString blockText(int block);
    QTextLayout *blockLayout(int block);
    QTextLine blockLineAtY(int block, int y);

    int blockHeight(int block);
    int blockLength(int block);
    int blockLengthWithoutLE(int block);
    int blockColumn(int block, int position);

    /// Returns current editor's viewport.
    /// If editor is not set, returns invalid rectangle.
    QRectF viewport();

    /// Scrolls the editor's view.
    /// If editor is not set, it does nothing.
    void scrollTo(qreal x, qreal y);

    /// Scrolls the editor's view.
    /// If editor is not set, it does nothing.
    void scrollBy(qreal dx, qreal dy);

    /// Estimates y distance between a and b.
    int estimateDistanceInLines(DocumentPosition a, DocumentPosition b);
    qreal estimateDistanceInYPixels(DocumentPosition a, DocumentPosition b);
    /// Estimates number of lines within a block by it's length.
    qreal estimateLinesInBlock(int length);

    int positionAtVerticalAnchor(const QTextLine &line, qreal *vertical_anchor);
    qreal blockYAtPosition(const DocumentPosition &p);
    QPointF blockPointAtPosition(const DocumentPosition &p);
    QRectF blockCursorRectAtPosition(const DocumentPosition &p);

    //
    //
    //

    DocumentPosition nextPosition(QTextLayout::CursorMode mode, DocumentPosition p);
    DocumentPosition previousPosition(QTextLayout::CursorMode mode, DocumentPosition p);
    DocumentPosition linePrevious(DocumentPosition p, qreal *vertical_anchor = NULL);
    DocumentPosition lineNext(DocumentPosition p, qreal *vertical_anchor = NULL);
    DocumentPosition lineBegin(DocumentPosition p);
    DocumentPosition lineEnd(DocumentPosition p);
    DocumentPosition blockBegin(DocumentPosition p);
    DocumentPosition blockEnd(DocumentPosition p);
    DocumentPosition documentBegin();
    DocumentPosition documentEnd();
    /// Scrolls the editor's view by a page and returns the correct position for page down operation.
    /// If editor is not set, it returns the p.
    DocumentPosition pageNext(DocumentPosition p, qreal *vertical_anchor);

    //
    // Finder
    //

    // typedef FinderBoyerMoore<TDocumentIterator> PlainFinder;
    // PlainFinder *createFinder(const QString &pattern, bool ignore_case, bool whole_words);

    //
    // Layout
    //

    int layoutWidth() {
        return m_layout_width;
    }
    void setLayoutWidth(int width);

    int blockCount() const;

    // TODO: This is a temporary hack.
    DocumentEdit lua_edit;
    DocumentEdit edit();
    bool isEditing();

    //
    // Visual properties for editor.
    //
    DocumentOptions options;

    // void setGutterOptions(const DocumentGutterOptions &options);

signals:
    void configured(const DocumentOptions &previous);

    void pathChanged(const QString &path);
    void focusedChanged(bool focused);

    void beforeFormatChanged();
    void formatChanged(int block);
    void beforeTextChanged();
    void textChanged(const DocumentEditEvent *event);

    /// This is called anytime carets move. Even if the main caret didn't change it's range.
    void cursorChanged(const DocumentRange &o, const DocumentRange &n);

    void editBegin(DocumentModel *view, bool undo);
    void editEnd(DocumentModel *view, bool undo);

    /// Called when gutter type changes.
    // void gutterOptionsChanged();

public slots:

private slots:
    void documentEditBegin(DocumentModel *view, bool undo);
    void documentEditEnd(DocumentModel *view, bool undo);

    void documentBeforeTextChanged();
    void documentTextChanged(const DocumentEditEvent *event);
    void documentBeforeFormatChanged();
    void documentFormatChanged(int block);
    void documentUndo();
    void documentRedo();

    void cursorChangedCall();

private:
    Document *m_d;
    DocumentEditor *m_editor;
    UndoStream *m_undo;
    StatusLineController *m_status_line_controller;
    bool m_focused;

    qreal m_block_lines_ratio_total;
    int m_block_lines_ratio_samples;
    int m_layout_width;

    void init(Document *document);

    void setDocument(Document *document);

    qreal blockLinesRatio();
    qreal blockLinesRatioDefault();

    void blockTextInvalidate(int block);
    void blockFormatInvalidate(int block);
    void invalidateFormat();

    struct Block
    {
        // int index;
        int layout_width;
        int layout_height;

        QTextLayout *layout;

        enum Flags
        {
            /// Text and Format is invalid when set.
            F_TextInvalidated   = 0x01,
            /// Only the Format is invalid.
            F_FormatInvalidated = 0x02
        };

        quint32 flags;
//        typedef QList<DocumentCursor *> Cursors;
//        Cursors cursors;
    };

    // TODO(cohen) Optimize this.
    typedef QVector<Block*> Blocks;
    Blocks m_blocks;
    Block *blockData(int block);

    typedef QList<DocumentCursor*> Cursors;
    Cursors m_cursors;
    DocumentCursor *m_cursor;

    void saveVerticalAnchor(DocumentCursor *cursor);
    void setCursorPosition(DocumentCursor *cursor, const DocumentPosition &position, int flags = 0);
    void setCursorRange(DocumentCursor *cursor, const DocumentRange &range);
    void setCursorRangeSorted(DocumentCursor *cursor, const DocumentPosition &begin, const DocumentPosition &end);
    void moveCursor(DocumentCursor *cursor, MoveCommand move_command, int flags = 0);
    void moveCursorBy(DocumentCursor *cursor, int delta, int flags = 0);

    struct
    {
        // DocumentRange o;
        bool scheduled;
    } m_cursor_changed_schedule;

    void scheduleCursorChangedCall(DocumentCursor *cursor);

    // NOTE(cohen) Accessed by DocumentCursor
    void insertText(DocumentEdit edit, DocumentCursor *cursor, const QString &text, int *shift);
    void removeText(DocumentEdit edit, DocumentCursor *cursor, RemoveCommand command, int *shift);
    void removeText(DocumentEdit edit, DocumentCursor *cursor, const DocumentPosition &to, int *shift);

    void readUndo(int event, QDataStream &s, bool redo);
    void writeUndo(int event);

    qreal calculateVerticalAnchor(const DocumentPosition &p);
    qreal calculateVerticalAnchor(const DocumentPosition &p, const QTextLine &line);

    void shiftCursor(DocumentCursor *cursor, int shift);
    void shiftCursors(const DocumentPosition &begin, const DocumentPosition &end, int shift);

    // Split constructor
    DocumentModel(DocumentModel *other);

    // Common init
    void init();


    //
    // Completion
    //
public:
//    struct Completion
//    {
//        bool active;
//        DocumentPosition begin;
//        DocumentPosition end;
//        int segment_begin;
//        int segment_end;
//        int edit_offset;
//        CompletionFilter *model;
//        QString text;
//        QItemSelectionModel *selection;
//    };

//    Completion *completion()
//    { return &m_completion; }

    void beginCompletionTest(int begin, int end);
    void beginCompletion(int begin, int end, CompletionController *completion_controller);
    void beginCompletion(const DocumentPosition &begin, const DocumentPosition &end, CompletionController *completion_controller);
    void endCompletion();

    void nextCompletion();
    void previousCompletion();
    void confirmCompletion();

    CompletionController *completionController()
    { return m_completion; }

private slots:
    void completerBegun();
    void completerEnded();
    void completerUpdated();

signals:
    void completionBegin();
    void completionUpdated();
    void completionEnd();

private:
    CompletionController *m_completion;

    void updateCompletion(const DocumentPosition &begin, const DocumentPosition &end, int shift);
    // void updateCompletion(int begin, int shift);
    void updateCompletion();
};

LUAOBJECT_DECLARE(DocumentModel)

#include "statusline.h"

class DocumentModelStatusLineController : public StatusLineController
{
    Q_OBJECT
public:
    DocumentModelStatusLineController(DocumentModel *view);

    void scope(ScopePath *scope, StatusLine *line);
    void install(StatusLine *line);
    void uninstall();

    StatusLineSegment *segment_grammar;

private slots:
    void grammarChanged(QSharedPointer<Grammar> grammar);

private:
    DocumentModel *m_view;

    QString filePath();
    QString cursorPosition();
    QString grammarTitle(QSharedPointer<Grammar> grammar);
};

template<typename T>
struct Serializer
{
    static_assert(
        sizeof(T) == -1,
        "Parameter to Value is not supported"
    );
};

class DocumentCursor
{
public:
    DocumentCursor(const DocumentCursor &other);
    explicit DocumentCursor(const DocumentRange &range, qreal vertical_anchor = -1.0f);

    bool setRange(const DocumentRange &r);
    bool setRangeSorted(const DocumentPosition &begin, const DocumentPosition &end);

    bool hasSelection();

    void setVerticalAnchor(qreal anchor);
    void invalidateVerticalAnchor();
    qreal verticalAnchor()
    { return m_vertical_anchor; }

    bool containsBlock(int block) const;

    const DocumentRange &range() const
    { return m_r; }

    const DocumentPosition &position() const
    { return m_r.first; }

    const DocumentPosition &anchor() const
    { return m_r.second; }

    DocumentPosition begin() const
    {
        if (m_r.first < m_r.second) {
            return m_r.first;
        }
        return m_r.second;
    }

    DocumentPosition end() const
    {
        if (m_r.first >= m_r.second) {
            return m_r.first;
        }
        return m_r.second;
    }

public slots:

private:
    DocumentRange m_r;
    qreal m_vertical_anchor;
};

#endif // DOCUMENTVIEW_H
