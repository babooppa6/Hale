#include <QDebug>

#include <QFont>
#include <QFontMetrics>
#include <QTextLayout>
#include <QItemSelectionModel>
#include <QJsonObject>

#include <qmath.h>

#include "configuration.h"

#include "clipboard.h"

#include "undostream.h"
#include "scopepath.h"
#include "pathutil.h"
#include "statusline.h"

#include "document.h"
#include "documenteditor.h"
#include "documentmodel.h"

DocumentModel::DocumentModel(Document *document, QObject *parent) :
    Model(parent),
    m_status_line_controller(NULL),
    m_completion(NULL),
    m_d(NULL)
{
    state_default = "normal";
    enterMode(state_default);

    setObjectName("document");
    m_layout_width = 600;

    options.font = QFont("Consolas", 11);

    m_undo = new UndoStream(this);
    m_cursor = NULL;
    m_editor = NULL;
    m_block_lines_ratio_total = 0;
    m_block_lines_ratio_samples = 0;
    m_blocks.append(NULL);

    m_cursor_changed_schedule.scheduled = false;

//    gutter_options.padding.left = 16;
//    gutter_options.padding.right = 16;
//    gutter_options.type = DocumentGutterOptions::GutterType::Numbers;

    setDocument(document);

//    {
//        auto e = edit();
//        addCursor(DocumentPosition(0, 0));
//        insertText(e, "hello world");
//        // moveCursor(Move_LineBegin);
//    }

    addCursor(DocumentPosition(0, 0));

//    m_completion.active = false;
//    m_completion.model = NULL;
//    m_completion.selection = NULL;
}

DocumentModel::~DocumentModel()
{
    close();
}

void DocumentModel::clear(DocumentEdit edit)
{
//    moveCursor(DocumentView::Move_DocumentBegin);
//    moveCursor(DocumentView::Move_DocumentEnd, DocumentView::MoveFlag_Select);
    removeText(edit, Remove_All);
}

void DocumentModel::configure(QJsonObject &o)
{
    qDebug() << o;
    DocumentOptions n = options;

    set(&n, o);

    inherit(&n.gutter, &n);
    set(&n.gutter, o["gutter"]);

    inherit(&n.block, &n);
    set(&n.block, o["block"]);

    inherit(&n.status, &n);
    set(&n.status, o["status"]);

    configure(n);
}

void DocumentModel::configure(const DocumentOptions &new_options)
{
    DocumentOptions old_options = options;
    options = new_options;
    invalidateFormat();
    emit configured(old_options);
}

bool DocumentModel::close()
{
    if (m_d) {
        m_d->removeModel(this);
        m_d = NULL;
    }
    return true;
}

void DocumentModel::scope(ScopePath *o_path)
{
    // TODO: Add project?
    // TODO: Add grammar type?

    if (m_completion) {
        o_path->push("completion", NULL);
    }
    o_path->push(objectName(), this);
}

Model *DocumentModel::split(QObject *parent)
{
    auto new_view = new DocumentModel(m_d, parent);
    new_view->m_undo->copyFrom(m_undo);

    qDeleteAll(new_view->m_cursors);
    new_view->m_cursors.clear();

    for (DocumentCursor *cursor : m_cursors)
    {
        new_view->m_cursors.append(new DocumentCursor(*cursor));
        if (cursor == m_cursor) {
            new_view->m_cursor = new_view->m_cursors.last();
        }
    }

    return new_view;
}

void DocumentModel::fromJson(const QJsonObject &j)
{
    Q_UNUSED(j);
}

void DocumentModel::toJson(QJsonObject &j)
{
    Q_UNUSED(j);
    // TODO: Save caret / carets.
    // TODO: Save scrolling position.
    // TODO: Save undo?
}

StatusLineController *DocumentModel::statusLineController()
{
    if (m_status_line_controller == NULL) {
        m_status_line_controller = new DocumentModelStatusLineController(this);
    }
    return m_status_line_controller;
}

void DocumentModel::setDocument(Document *document)
{
    Q_ASSERT(m_d == NULL);

    m_d = document;

    m_d->addModel(this);

    m_blocks.clear();
    m_blocks.resize(m_d->blockCount());

    connect(m_d, &Document::editBegin, this, &DocumentModel::documentEditBegin);
    connect(m_d, &Document::editEnd, this, &DocumentModel::documentEditEnd);
    connect(m_d, SIGNAL(textChanged(const DocumentEditEvent*)), this, SLOT(documentTextChanged(const DocumentEditEvent*)));
    connect(m_d, SIGNAL(formatChanged(int)), this, SLOT(documentFormatChanged(int)));
    connect(m_d, &Document::undoTriggered, this, &DocumentModel::documentUndo);
    connect(m_d, &Document::redoTriggered, this, &DocumentModel::documentRedo);
    connect(m_d, &Document::pathChanged, this, &DocumentModel::pathChanged);
}

void DocumentModel::addCursor(const DocumentPosition &p)
{
    addCursor(DocumentRange(p, p));
}

namespace {
    bool cursor_less_than(const DocumentCursor *a, const DocumentCursor *b)
    {
        return a->position() < b->position();
    }
}

void DocumentModel::addCursor(const DocumentRange &r)
{
    DocumentCursor *cursor = new DocumentCursor(r);
    m_cursor = cursor;
    m_cursors.append(cursor);
    qSort(m_cursors.begin(), m_cursors.end(), &cursor_less_than);
    scheduleCursorChangedCall(cursor);
}

void DocumentModel::resetCursors()
{
    for (auto cursor : m_cursors) {
        if (cursor != m_cursor) {
            delete cursor;
        }
    }
    setCursorRange(m_cursor, DocumentRange(m_cursor->position(), m_cursor->position()));
    m_cursors.clear();
    m_cursors.append(m_cursor);
    scheduleCursorChangedCall(m_cursor);
}

void DocumentModel::setSelection(int begin, int end)
{
    resetCursors();
    DocumentRange r(m_d->range(begin, end));
    setCursorRange(m_cursor, r);
}

void DocumentModel::findCursors(const DocumentRange &r, QList<DocumentCursor *> *cursors)
{
    DocumentRange rs(r.sorted());
    cursors->clear();
    for (DocumentCursor *cursor : m_cursors) {
        if (rs.intersectsSorted(cursor->range().sorted())) {
            cursors->append(cursor);
        }
    }
}

void DocumentModel::scheduleCursorChangedCall(DocumentCursor *)
{
    if (!m_cursor_changed_schedule.scheduled) {
        // Only remember the first time, so we have the
//        if (cursor == m_cursor) {
//            m_cursor_changed_schedule.o = m_cursor->range();
//        }
        m_cursor_changed_schedule.scheduled = true;
        QMetaObject::invokeMethod(this, "cursorChangedCall", Qt::QueuedConnection);
    }
}

void DocumentModel::cursorChangedCall()
{
    m_cursor_changed_schedule.scheduled = false;
    emit cursorChanged(DocumentRange(), m_cursor->range());
}

Document *DocumentModel::document()
{
    return m_d;
}

QString DocumentModel::description()
{
    QString d(m_d->path());
    if (d.isEmpty()) {
        d = "<Unsaved>";
    }
    return d;
}

qreal DocumentModel::lineHeight()
{
    // QFontMetrics &f = Configuration::editorFontMetrics();
    return QFontMetrics(options.font).lineSpacing();
}

void DocumentModel::setEditor(DocumentEditor *editor)
{
    if (editor != NULL && m_editor != NULL) {
        qWarning("View is already assigned to an editor.");
        return;
    }
    m_editor = editor;
}

//
//
//

static int STATE_BEFORE = UndoStream::EventType_User;
static int STATE_AFTER = UndoStream::EventType_User + 1;

void DocumentModel::undo()
{
    m_d->undo();
}

void DocumentModel::redo()
{
    m_d->redo();
}

void DocumentModel::documentEditBegin(DocumentModel *view, bool undo)
{
    // qDebug() << __FUNCTION__;
    if (!undo) {
        m_undo->writeBreak();
        // Saves the state before the edit.
        writeUndo(STATE_BEFORE);
    }

    emit editBegin(view, undo);
}

void DocumentModel::documentEditEnd(DocumentModel *view, bool undo)
{
    // qDebug() << __FUNCTION__;
    if (!undo) {
        // Saves the state after the edit.
        writeUndo(STATE_AFTER);
    }
    emit editEnd(view, undo);
}

void DocumentModel::documentUndo()
{
    m_undo->undo([&] (int event, QDataStream &s) {
        readUndo(event, s, false);
    });
    scheduleCursorChangedCall(m_cursor);
}

void DocumentModel::documentRedo()
{
    m_undo->redo([&] (int event, QDataStream &s) {
        readUndo(event, s, true);
    });
    scheduleCursorChangedCall(m_cursor);
}

void DocumentModel::readUndo(int event, QDataStream &s, bool redo)
{
    qDeleteAll(m_cursors);
    m_cursors.clear();
    m_cursor = NULL;

    DocumentRange r;
    qreal vertical_anchor;
    int count;

    s >> count;
    for (int i = 0; i < count; i++)
    {
        s >> r.first.block;
        s >> r.first.position;
        s >> r.second.block;
        s >> r.second.position;
        s >> vertical_anchor;

        if ((event == STATE_BEFORE && !redo) || (event == STATE_AFTER && redo)) {
            m_cursors.append(new DocumentCursor(r, vertical_anchor));
        }
    }

    if (!m_cursors.isEmpty()) {
        int primary;
        s >> primary;
        m_cursor = m_cursors[primary];
    }
}

void DocumentModel::writeUndo(int event)
{
    m_undo->write(event, [&] (QDataStream &s) {
        s << m_cursors.count();
        int primary = 0;
        DocumentCursor *cursor;
        DocumentRange r;
        for (int i = 0; i < m_cursors.length(); i++)
        {
            cursor = m_cursors[i];
            r = cursor->range();
            s << r.first.block;
            s << r.first.position;
            s << r.second.block;
            s << r.second.position;
            s << cursor->verticalAnchor();
            if (cursor == m_cursor) {
                primary = i;
            }
        }
        s << primary;
    });
}

//
//
//

QRectF DocumentModel::viewport()
{
    QRectF ret;
    if (m_editor) {
        ret = m_editor->viewport();
    }
    return ret;
}

void DocumentModel::scrollTo(qreal x, qreal y)
{
    if (m_editor) {
        m_editor->scrollTo(x, y);
    }
}

void DocumentModel::scrollBy(qreal dx, qreal dy)
{
    if (m_editor) {
        m_editor->scrollBy(dx, dy);
    }
}

qreal DocumentModel::blockLinesRatioDefault()
{
    QFontMetrics font_metrics(options.font);
    qreal average_word_length = 5;
    qreal average_word_width = font_metrics.averageCharWidth() * average_word_length;
    qreal words_per_line = qFloor(layoutWidth() / average_word_width);
    qreal line_natural_width = words_per_line / font_metrics.averageCharWidth();
    return layoutWidth() / line_natural_width;
}

qreal DocumentModel::blockLinesRatio()
{
    if (m_block_lines_ratio_samples == 0) {
        return blockLinesRatioDefault();
    }
    return m_block_lines_ratio_total / (qreal)m_block_lines_ratio_samples;
}

qreal DocumentModel::estimateLinesInBlock(int length)
{
    return blockLinesRatio() * (qreal)length;
}

int DocumentModel::estimateDistanceInLines(DocumentPosition a, DocumentPosition b)
{
    if (!m_editor || layoutWidth() < 0.1) {
        // TODO(cohen): This is wordwrap off calculation.
        qWarning() << "Unable to estimate Y distance, because editor is not available.";
        return qAbs(a.block - b.block);
    } else {
        QFontMetrics font_metrics(options.font);
        // Number of characters between the positions.
        qreal length = qAbs(m_d->blockPositionToOffset(a) - m_d->blockPositionToOffset(b));
        // Average word length (eyeballed).
        // http://www.wolframalpha.com/input/?i=average+english+word+length ==> 5.1
        qreal average_word_characters = 7.0; //5.1;
        // Number of words for given length.
        qreal words_for_length = qCeil(length / average_word_characters);
        // Number of words that would fit to layout width.
        qreal words_per_line = qFloor(layoutWidth() / (average_word_characters*font_metrics.averageCharWidth()));

        int estimated_lines = qCeil(words_for_length / words_per_line) + 1;

        int minimum_lines = qAbs(a.block - b.block);

        return qMax(minimum_lines, estimated_lines);
    }
}

qreal DocumentModel::estimateDistanceInYPixels(DocumentPosition a, DocumentPosition b)
{
    QFontMetrics font_metrics(options.font);
    return estimateDistanceInLines(a, b) * font_metrics.height();
}

int DocumentModel::positionAtVerticalAnchor(const QTextLine &line, qreal *vertical_anchor)
{
    Q_ASSERT(vertical_anchor != NULL);
    // TODO(cohen): Subtract line.x() from the anchor?
    // NOTE(cohen): WTF?: "Note that result cursor position includes possible preedit area text."
    return line.xToCursor(*vertical_anchor, QTextLine::CursorBetweenCharacters);
}

qreal DocumentModel::blockYAtPosition(const DocumentPosition &p)
{
    QTextLayout *layout = blockLayout(p.block);
    return layout->lineForTextPosition(p.position).y();
}

QPointF DocumentModel::blockPointAtPosition(const DocumentPosition &p)
{
    QTextLayout *layout = blockLayout(p.block);
    QTextLine line = layout->lineForTextPosition(p.position);
    QPointF ret(line.cursorToX(p.position) + line.position().x(), line.position().y());
    return ret;
}

QRectF DocumentModel::blockCursorRectAtPosition(const DocumentPosition &p)
{
    QTextLayout *layout = blockLayout(p.block);
    QTextLine line = layout->lineForTextPosition(p.position);
    if (!line.isValid()) {
        return QRectF();
    }
    // Q_ASSERT(line.isValid());
    QRectF ret(
        line.cursorToX(p.position) + line.position().x(),
        line.position().y(),
        2.0f,
        line.height()
        );
    return ret;
}

//
//
//

DocumentEdit DocumentModel::edit()
{
    return m_d->edit(this);
}

bool DocumentModel::isEditing()
{
    return m_d->isEditing();
}


void DocumentModel::insertText(DocumentEdit edit, const QString &text)
{
    int shift = 0;
    for (DocumentCursor *cursor : m_cursors) {
        shiftCursor(cursor, shift);
        insertText(edit, cursor, text, &shift);
    }
}

void DocumentModel::insertIndentation(DocumentEdit edit)
{
    if (m_d->indentation_mode == IndentationMode::Spaces) {
        // If we're in "Spaces" mode, we have to walk each cursor to insert proper amount of spaces.
        int size = m_d->indentation_size;
        int column;
        int shift = 0;
        for (auto cursor : m_cursors) {
            // TODO: We need to first remove the cursor's text.
            column = m_d->blockColumn(cursor->position());
            insertText(edit, cursor, QString(' ').repeated(size - (column % size)), &shift);
        }
    } else if (m_d->indentation_mode == IndentationMode::Tabs) {
        // If we're in "Tabs" mode, we just push a tab and that's about it.
        insertText(edit, QStringLiteral("\t"));
    }
}

void DocumentModel::insertNewLine(DocumentEdit edit)
{
    QString tmp;
    int indentation;
    int shift = 0;
    for (auto cursor : m_cursors) {
        // TODO: We need to first remove the cursor's text.
        indentation = m_d->indentationForBlock(cursor->position().block + 1);
        tmp = "\n";
        tmp.append(QString(' ').repeated(indentation));
        insertText(edit, cursor, tmp, &shift);
    }
}

void DocumentModel::removeText(DocumentEdit edit, RemoveCommand command)
{
    int shift = 0;
    if (command == Remove_All) {
        resetCursors();
        moveCursor(Move_DocumentBegin);
        removeText(edit, m_cursor, documentEnd(), &shift);
        return;
    }

    for (DocumentCursor *cursor : m_cursors) {
        shiftCursor(cursor, shift);
        removeText(edit, cursor, command, &shift);
    }
}

void DocumentModel::moveCursor(MoveCommand move, int flags)
{
    for (DocumentCursor *cursor : m_cursors) {
        moveCursor(cursor, move, flags);
    }
}

void DocumentModel::moveCursorTo(int offset, int flags)
{
    auto p = m_d->offsetToBlockPosition(offset);
    moveCursorTo(p, flags);
}

void DocumentModel::moveCursorTo(const DocumentPosition &position, int flags)
{
    int delta = m_d->minus(position, m_cursor->position());
    for (DocumentCursor *cursor : m_cursors) {
        moveCursorBy(cursor, delta, flags);
    }
}

void DocumentModel::insertText(DocumentEdit edit, DocumentCursor *cursor, const QString &text, int *shift)
{
    if (cursor->hasSelection()) {
        removeText(edit, cursor, cursor->anchor(), shift);
    }
    DocumentPosition i(m_d->insertText(edit, cursor->position(), text));
    *shift += m_d->minus(i, cursor->position());
    setCursorPosition(cursor, i);
}

void DocumentModel::removeText(DocumentEdit edit, DocumentCursor *cursor, const DocumentPosition &anchor, int *shift)
{
    if (cursor->position() == anchor) {
        return;
    }

    DocumentRange o(m_cursor->range());

    int p = m_d->blockPositionToOffset(cursor->position());
    int a = m_d->blockPositionToOffset(anchor);

    if (p < a) {
        setCursorPosition(cursor, cursor->position());
        m_d->removeText(edit, cursor->position(), a-p);
        *shift -= a-p;
    } else {
        setCursorPosition(cursor, anchor);
        m_d->removeText(edit, anchor, p-a);
        *shift -= p-a;
    }

    emit cursorChanged(o, m_cursor->range());
}

void DocumentModel::removeText(DocumentEdit edit, DocumentCursor *cursor, RemoveCommand command, int *shift)
{
    if (cursor->hasSelection()) {
        command = Remove_Selected;
    }

    switch (command)
    {
        case Remove_Selected: {
            removeText(edit, cursor, cursor->anchor(), shift);
        } break;
        case Remove_CharacterBackward: {
            auto pos(previousPosition(QTextLayout::SkipCharacters, cursor->position()));
            removeText(edit, cursor, pos, shift);
        } break;
        case Remove_CharacterForward: {
            auto pos(nextPosition(QTextLayout::SkipCharacters, cursor->position()));
            removeText(edit, cursor, pos, shift);
        } break;
    }
}

// Used when edits come from this view.
void DocumentModel::shiftCursor(DocumentCursor *cursor, int shift)
{
    if (shift != 0) {
        DocumentRange r(cursor->range());
        r.first  = m_d->plus(r.first,  shift);
        r.second = m_d->plus(r.second, shift);
        setCursorRange(cursor, r);
    }
}

QString DocumentModel::text(int offset, int length)
{
    return m_d->text(offset, length);
}

QString DocumentModel::text(const DocumentPosition &position, int length)
{
    return m_d->text(position, length);
}

QString DocumentModel::text()
{
    return m_d->text();
}

QString DocumentModel::text(const DocumentPosition &begin, const DocumentPosition &end)
{
    return m_d->text(begin, end);
}

// Inserting at caret location means "insert before"
// Inserting at begin or before selection means "insert before"
// Inserting at end or after selection means "insert after"

// Used when edits come from other view.
void DocumentModel::shiftCursors(const DocumentPosition &begin, const DocumentPosition &end, int shift)
{
    if (shift == 0) {
        return;
    }

    int block_shift = end.block - begin.block;
    DocumentRange r;
    if (shift > 0) {
        //
        // Insert
        //
        for (DocumentCursor *cursor : m_cursors) {
            r = cursor->range();
            if (begin.block < r.begin().block) {
                // The insert happened before the cursor's block, so only move the block.
                r.first.block += block_shift;
                r.second.block += block_shift;
            } else if (begin.block == r.begin().block) {
                // The insert started at the block.
                if (begin.position <= r.begin().position) {
                    // The insert happened before our position, so we shift block and position.
                    r.first = m_d->plus(r.first, shift);
                    r.second = m_d->plus(r.second, shift);
                } else if (begin.position < r.end().position) {
                    // The insert happened inside the caret's selection, so we only shift the end.
                    r.setSorted(r.begin(), m_d->plus(r.end(), shift));
                }
            } else {
                // The insert happened after the cursor.
                // Nothing to do.
            }
            setCursorRange(cursor, r);
        }
    } else {
        //
        // Remove
        //
        DocumentPosition cursor_begin;
        DocumentPosition cursor_end;
        Cursors::iterator it = m_cursors.begin();
        while (it != m_cursors.end()) {
            (*it)->range().sorted(&cursor_begin, &cursor_end);
            if (begin <= cursor_begin && end >= cursor_end) {
                if (*it == m_cursor) {
                    setCursorRange(m_cursor, DocumentRange(begin, begin));
                    it++;
                } else {
                    it = m_cursors.erase(it);
                }
            } else {
                if (end.block < cursor_begin.block) {
                    // Remove happened before the caret's block, so we only shift the blocks.
                    cursor_begin.block -= block_shift;
                    cursor_end.block -= block_shift;
                } else if (end.block == cursor_begin.block) {
                    // Remove ended at the cursor's block.
                    if (end.position <= cursor_begin.position) {
                        // Remove happened before the cursor.
                        // We have to shift both the block and the position.
                        cursor_begin = m_d->plus(cursor_begin, shift);
                        cursor_end = m_d->plus(cursor_end, shift);
                    } else if (begin.position <= cursor_begin.position && (end > cursor_begin && end <= cursor_end)) {
                        // Remove started before the cursor, but ended inside.
                        // Set the cursor's begin to the begin of the change.
                        cursor_begin = begin;
                        // Shift the cursor end.
                        cursor_end = m_d->plus(cursor_end, shift);
                    } else if ((begin >= cursor_begin && begin <= cursor_end) && end >= cursor_end) {
                        // Remove started inside, but ended after.
                        // We will cut the end.
                        cursor_end = begin;
                    } else {
                        // Remove happened after the cursor.
                        // Nothing to do.
                        Q_ASSERT(begin >= cursor_end);
                    }
                }

                setCursorRangeSorted(*it, cursor_begin, cursor_end);
                it++;
            }
        }
    }
}

DocumentBlock DocumentModel::block(int index)
{
    DocumentBlock block(this, index);
    return block;
}

//
// Layout
//

void DocumentModel::setLayoutWidth(int width)
{
    if (m_layout_width != width) {
        m_layout_width = width;
        // Reset ratios
        m_block_lines_ratio_samples = 0;
        m_block_lines_ratio_total = 0;
    }
}

DocumentModel::Block *DocumentModel::blockData(int i)
{
    Block *block_data = m_blocks[i];
    if (block_data == NULL) {
        block_data = new Block();
        // block_data->index = i;
        block_data->layout_height = -1;
        block_data->layout_width = -1;
        block_data->layout = NULL;
        block_data->flags = Block::F_TextInvalidated | Block::F_FormatInvalidated;
        m_blocks[i] = block_data;
    }

    return block_data;
}

QString DocumentModel::blockText(int block)
{
    return m_d->blockText(block, false);
}

QTextLayout *DocumentModel::blockLayout(int block)
{
    Block *block_data = blockData(block);

    if (
        block_data->layout_width == m_layout_width &&
        !(block_data->flags & Block::F_FormatInvalidated) &&
        !(block_data->flags & Block::F_TextInvalidated) &&
        block_data->layout != NULL
    ) {
        return block_data->layout;
    }

    QTextLayout *layout = block_data->layout;

    // TODO(cohen) Compare the actual width of the layout with the requested layout width.
    // If the layout is a single line and the width is smaller than the layout width, there's nothing to do.
    // qDebug() << "Layouting Block" << block;

    if (layout == NULL) {
        layout = new QTextLayout(m_d->blockText(block, false));
    } else {
        if (block_data->flags & Block::F_TextInvalidated) {
            layout->setText(m_d->blockText(block, false));
        }
        layout->clearAdditionalFormats();
    }
    QTextOption option;
    // option.setFlags(QTextOption::ShowTabsAndSpaces);
    QFontMetrics font_metrics(options.font);
    // TODO: Use document->indentationSize()
    option.setTabStop(font_metrics.averageCharWidth() * 4);
    layout->setTextOption(option);

    layout->setFont(options.font);
    layout->setAdditionalFormats(*m_d->blockFormats(block));

    int leading = font_metrics.leading();
    qreal height = 0;
    layout->setCacheEnabled(true);
    layout->beginLayout();
    QTextLine line;
    while (1) {
        line = layout->createLine();
        if (!line.isValid())
            break;

        line.setLineWidth(m_layout_width);

        height += leading;
        line.setPosition(QPointF(0, height));
        height += line.height();
    }
    layout->endLayout();

    if (layout->lineCount() > 0)
    {
        for (int i = 0; i < layout->lineCount()-1; i++)
        {
            m_block_lines_ratio_total += (qreal)layout->lineAt(i).naturalTextWidth() / (qreal)m_layout_width;
            m_block_lines_ratio_samples++;
        }
//        if (m_block_lines_ratio_samples > 0) {
//            qDebug() << "Block lines ratio" << blockLinesRatio() << "samples" << m_block_lines_ratio_samples << "default" << blockLinesRatioDefault();
//        }
    }

    block_data->layout_width = m_layout_width;
    block_data->layout_height = height;
    block_data->layout = layout;

    block_data->flags &= ~Block::F_FormatInvalidated;
    block_data->flags &= ~Block::F_TextInvalidated;

    // TODO: Invalidate cursor's vertical anchors if they are on this block.
//    if (m_cursor->position().block == block) {
//        m_cursor->invalidateVerticalAnchor();
//    }

/*
    if (m_layout_height != height) {
        m_layout_height = height;
        emit layoutHeightChanged();
    }
*/

    return block_data->layout;
}

QTextLine DocumentModel::blockLineAtY(int block, int y)
{
    QTextLayout *layout = blockLayout(block);
    int line_i;
    if (y <= 0) {
        line_i = 0;
    } else {
        line_i = y / lineHeight();
        if (line_i >= layout->lineCount()) {
            line_i = layout->lineCount() - 1;
        }
    }

    return layout->lineAt(line_i);
}

void DocumentModel::blockTextInvalidate(int block)
{
    Block *block_data = blockData(block);
    block_data->flags |= Block::F_TextInvalidated | Block::F_FormatInvalidated;
    // Notify nothing.
}

void DocumentModel::blockFormatInvalidate(int block)
{
    Block *block_data = blockData(block);
    if (!(block_data->flags & Block::F_FormatInvalidated)) {
        block_data->flags |= Block::F_FormatInvalidated;
        emit formatChanged(block);
    }
}

void DocumentModel::invalidateFormat()
{
    for (int i = 0; i < m_blocks.count(); i++) {
        blockFormatInvalidate(i);
    }
}

int DocumentModel::blockHeight(int block)
{
    blockLayout(block);
    return m_blocks[block]->layout_height;
}

int DocumentModel::blockCount() const
{
    return m_d->blockCount();
}

int DocumentModel::blockLength(int block)
{
    return m_d->blockLength(block);
}

int DocumentModel::blockLengthWithoutLE(int block)
{
    return m_d->blockLengthWithoutLE(block);
}

//
//
//

void DocumentModel::copy()
{
    QString buffer;
    QString one;
    for (auto cursor : m_cursors) {
        one = text(cursor->begin(), cursor->end());
        if (!one.isEmpty()) {
            if (!buffer.isEmpty()) {
                buffer.append('\n');
            }
            buffer.append(one);
        }
    }

    Clipboard::instance->copy(buffer);
}

void DocumentModel::cut(DocumentEdit edit)
{
    copy();
    removeText(edit, Remove_Selected);
}

void DocumentModel::paste(DocumentEdit edit)
{
    QString text(Clipboard::instance->paste());
    insertText(edit, text);
}

//
//
//

void DocumentModel::documentBeforeTextChanged()
{
    emit beforeTextChanged();
}

void DocumentModel::documentTextChanged(const DocumentEditEvent *event)
{
    if (event->type == DocumentEditEvent::Insert) {
        m_blocks.insert(event->block_list_change_first, event->block_list_change_count, NULL);
        if (event->block_text_change != -1) {
            blockTextInvalidate(event->block_text_change);
        }
        if (event->sender != this) {
            // Do this after insert, so we won't be forced to enter the virtual positions with carets.
            shiftCursors(event->begin, event->end, event->offset_end - event->offset_begin);
        }
        updateCompletion(event->begin, event->end, event->offset_end - event->offset_begin);
    } else { // Remove
        if (event->block_text_change != -1) {
            blockTextInvalidate(event->block_text_change);
        }
        if (event->sender != this) {
            shiftCursors(event->begin, event->end, -(event->offset_end - event->offset_begin));
        }
        updateCompletion(event->begin, event->end, -(event->offset_end - event->offset_begin));
        for (int i = 0; i < event->block_list_change_count; i++) {
            delete m_blocks[i + event->block_list_change_first];
        }
        m_blocks.remove(event->block_list_change_first, event->block_list_change_count);
    }

    emit textChanged(event);
}

void DocumentModel::documentBeforeFormatChanged()
{
    emit beforeFormatChanged();
}

void DocumentModel::documentFormatChanged(int block)
{
    blockFormatInvalidate(block);
}

//
// Positions
//

void DocumentModel::saveVerticalAnchor(DocumentCursor *cursor)
{
    cursor->setVerticalAnchor(calculateVerticalAnchor(cursor->position()));
}

qreal DocumentModel::calculateVerticalAnchor(const DocumentPosition &p)
{
    QTextLayout *layout = blockLayout(p.block);
    QTextLine line = layout->lineForTextPosition(p.position);
    return calculateVerticalAnchor(p, line);
}

qreal DocumentModel::calculateVerticalAnchor(const DocumentPosition &p, const QTextLine &line)
{
    return line.cursorToX(p.position);
}


void DocumentModel::setCursorPosition(DocumentCursor *cursor, const DocumentPosition &block_position, int flags)
{
    DocumentRange n(cursor->range());
    n.first = block_position;
    if (!(flags & MoveFlag_Select)) {
        if (n.second != block_position) {
            n.second = block_position;
        }
    }

    if (flags & MoveFlag_VerticalAnchor) {
        saveVerticalAnchor(cursor);
    }

    setCursorRange(cursor, n);
}

void DocumentModel::setCursorRangeSorted(DocumentCursor *cursor, const DocumentPosition &begin, const DocumentPosition &end)
{
    scheduleCursorChangedCall(cursor);
    cursor->setRangeSorted(begin, end);
}

void DocumentModel::setCursorRange(DocumentCursor *cursor, const DocumentRange &range)
{
    scheduleCursorChangedCall(cursor);
    cursor->setRange(range);
}

void DocumentModel::moveCursor(DocumentCursor *cursor, MoveCommand move_command, int flags)
{
    qreal vertical_anchor;
    switch (move_command)
    {
    case Move_CharacterNext:
        setCursorPosition(cursor, nextPosition(QTextLayout::SkipCharacters, cursor->position()), flags | MoveFlag_VerticalAnchor);
        break;
    case Move_CharacterPrevious:
        setCursorPosition(cursor, previousPosition(QTextLayout::SkipCharacters, cursor->position()), flags | MoveFlag_VerticalAnchor);
        break;
    case Move_WordNext:
        setCursorPosition(cursor, nextPosition(QTextLayout::SkipWords, cursor->position()), flags | MoveFlag_VerticalAnchor);
        break;
    case Move_WordPrevious:
        setCursorPosition(cursor, previousPosition(QTextLayout::SkipWords, cursor->position()), flags | MoveFlag_VerticalAnchor);
        break;
    case Move_LineNext:
        Q_ASSERT(!(flags & MoveFlag_VerticalAnchor));
        setCursorPosition(cursor, lineNext(cursor->position(), &vertical_anchor), flags & ~MoveFlag_VerticalAnchor);
        cursor->setVerticalAnchor(vertical_anchor);
        break;
    case Move_LinePrevious: {
        Q_ASSERT(!(flags & MoveFlag_VerticalAnchor));
        setCursorPosition(cursor, linePrevious(cursor->position(), &vertical_anchor), flags & ~MoveFlag_VerticalAnchor);
        cursor->setVerticalAnchor(vertical_anchor);
        } break;
    case Move_LineEnd:
        setCursorPosition(cursor, lineEnd(cursor->position()), flags | MoveFlag_VerticalAnchor);
        break;
    case Move_LineBegin:
        setCursorPosition(cursor, lineBegin(cursor->position()), flags | MoveFlag_VerticalAnchor);
        break;
    case Move_BlockEnd:
        setCursorPosition(cursor, blockEnd(cursor->position()), flags | MoveFlag_VerticalAnchor);
        break;
    case Move_BlockBegin:
        setCursorPosition(cursor, blockBegin(cursor->position()), flags | MoveFlag_VerticalAnchor);
        break;
    case Move_DocumentEnd:
        setCursorPosition(cursor, documentEnd(), flags | MoveFlag_VerticalAnchor);
        break;
    case Move_DocumentBegin:
        setCursorPosition(cursor, documentBegin(), flags | MoveFlag_VerticalAnchor);
        break;
    case Move_PageNext: {
        Q_ASSERT(!(flags & MoveFlag_VerticalAnchor));
        setCursorPosition(cursor, pageNext(cursor->position(), &vertical_anchor), flags & ~MoveFlag_VerticalAnchor);
        cursor->setVerticalAnchor(vertical_anchor);
        // m_dv->scrollBy(0, m_dv->viewport().height());
        } break;
    default:
        qWarning("Unknown cursor move command.");
        break;
    }
}

void DocumentModel::moveCursorBy(DocumentCursor *cursor, int delta, int flags)
{
    DocumentPosition position = m_d->plus(cursor->position(), delta);
    setCursorPosition(cursor, position, flags);
}

DocumentPosition DocumentModel::nextPosition(QTextLayout::CursorMode mode, DocumentPosition p)
{
    DocumentBlock b = block(p.block);
    if (p.position == b.end()) {
        if (!b.isLast()) {
            p.block++;
            p.position = 0;
        }
    } else {
        QTextLayout *layout = b.layout();
        p.position = layout->nextCursorPosition(p.position, mode);
    }
    return p;
}

DocumentPosition DocumentModel::previousPosition(QTextLayout::CursorMode mode, DocumentPosition p)
{
    DocumentBlock b = block(p.block);
    if (p.position == 0) {
        if (!b.isFirst()) {
            p.block--;
            p.position = blockLengthWithoutLE(p.block);
        }
    } else {
        QTextLayout *layout = b.layout();
        p.position = layout->previousCursorPosition(p.position, mode);
    }
    return p;
}

DocumentPosition DocumentModel::linePrevious(DocumentPosition p, qreal *vertical_anchor)
{
    QTextLayout *layout = blockLayout(p.block);
    QTextLine line = layout->lineForTextPosition(p.position);

    qreal vertical_anchor_tmp = -1.0f;
    // NOTE(cohen): If vertical anchor is not set, calculate it in place.
    // - This is not too good for UX, vertical_anchor should be stored.
    if (vertical_anchor == NULL) {
        vertical_anchor = &vertical_anchor_tmp;
    }
    if (*vertical_anchor < 0.0f) {
        *vertical_anchor = calculateVerticalAnchor(p, line);
    }

    int line_index = line.lineNumber();
    if (line.lineNumber() == 0) {
        if (p.block == 0) {
            return p;
        }
        p.block--;
        layout = blockLayout(p.block);
        line = layout->lineAt(layout->lineCount() - 1);
    }
    else
    {
        line = layout->lineAt(line_index - 1);
    }

    p.position = positionAtVerticalAnchor(line, vertical_anchor);

    return p;
}

DocumentPosition DocumentModel::lineNext(DocumentPosition p, qreal *vertical_anchor)
{
    QTextLayout *layout = blockLayout(p.block);
    QTextLine line = layout->lineForTextPosition(p.position);
    Q_ASSERT(line.isValid());

    qreal vertical_anchor_tmp = -1.0f;
    // NOTE(cohen): If vertical anchor is not set, calculate it in place.
    // - This is not too good for UX, vertical_anchor should be stored.
    if (vertical_anchor == NULL) {
        vertical_anchor = &vertical_anchor_tmp;
    }
    if (*vertical_anchor < 0.0f) {
        *vertical_anchor = calculateVerticalAnchor(p, line);
    }

    int line_index = line.lineNumber();
    if (line_index == layout->lineCount() - 1) {
        if (p.block == blockCount() - 1) {
            return p;
        }
        p.block++;
        layout = blockLayout(p.block);
        line = layout->lineAt(0);
    }
    else
    {
        line = layout->lineAt(line_index + 1);
    }

    p.position = positionAtVerticalAnchor(line, vertical_anchor);

    return p;
}

DocumentPosition DocumentModel::lineBegin(DocumentPosition p)
{
    QTextLayout *layout = blockLayout(p.block);
    QTextLine line = layout->lineForTextPosition(p.position);
    if (p.position == line.textStart()) {
        // TODO: Go to indentation start.
        p.position = 0;
    } else {
        p.position = line.textStart();
    }
    return p;
}

DocumentPosition DocumentModel::lineEnd(DocumentPosition p)
{
    QTextLayout *layout = blockLayout(p.block);
    QTextLine line = layout->lineForTextPosition(p.position);
    p.position = line.textStart() + line.textLength();
    return p;
}

DocumentPosition DocumentModel::blockBegin(DocumentPosition p)
{
    p.position = 0;
    return p;
}

DocumentPosition DocumentModel::blockEnd(DocumentPosition p)
{
    p.position = blockLengthWithoutLE(p.block);
    return p;
}

DocumentPosition DocumentModel::documentBegin()
{
    DocumentPosition p;
    p.position = 0;
    p.block = 0;
    return p;
}

DocumentPosition DocumentModel::documentEnd()
{
    DocumentPosition p;
    p.block = blockCount() - 1;
    p.position = blockLength(p.block);
    return p;
}

DocumentPosition DocumentModel::pageNext(DocumentPosition p, qreal *vertical_anchor)
{
    if (!m_editor) {
        return p;
    }

    bool found = true;
    qreal page_height = viewport().height();
    QRectF position_rect = blockCursorRectAtPosition(p);
    qreal h = blockHeight(p.block) - position_rect.top();
    qreal y = h;
    if (y < page_height)
    {
        found = false;
        // If the current block does not contain the page,
        // then find the block that does.

        for (;;) {
            p.block++;
            if (p.block == blockCount()) {
                break;
            }
            h = blockHeight(p.block);
            if ((y + h) > page_height) {
                // We have found our block!
                found = true;
                break;
            }
            y += h;
        }

        if (!found) {
            p.block = blockCount()-1;
            p.position = blockLength(p.block);
            return p;
        }
    }

    qreal y_b = page_height - y;
    // p.block now contains the block that matches the location
    // y_b now contains the y inside the block that matches the location.

    QTextLayout *layout = blockLayout(p.block);
    for (int i = 0; i < layout->lineCount(); i++) {
        QTextLine line = layout->lineAt(i);
        if (line.y() + line.height() > y_b) {
            // Line has been found.
            p.position = positionAtVerticalAnchor(line, vertical_anchor);
            return p;
        }
    }

    Q_ASSERT(false && "Line not found.");
    return p;
}

//void DocumentModel::setFocused(bool focused)
//{
//    if (focused != m_focused) {
//        m_focused = focused;
//        emit focusedChanged(focused);
//    }
//}

//
// Visual properties.
//

//void DocumentModel::setGutterOptions(const DocumentGutterOptions &gutter_options)
//{
//    if (gutter_options != options.gutter) {
//        options.gutter
//        gutter_options = options;
//        emit gutterOptionsChanged();
//    }
//}

//
// Finder
//

//DocumentView::PlainFinder *DocumentView::createFinder(const QString &pattern, bool ignore_case, bool whole_words)
//{
//    auto finder = new PlainFinder(pattern, ignore_case, whole_words);
//    return finder;
//}

//
//
//

#include "pathcompletioncontroller.h"

void DocumentModel::beginCompletionTest(int begin, int end)
{
    // auto model = new PathCompletionModel(this);
    auto controller = new PathCompletionController(this, this);
    // filter->setSourceModel(model);

    beginCompletion(begin, end, controller);
}

void DocumentModel::beginCompletion(int begin, int end, CompletionController *completion_controller)
{
    beginCompletion(m_d->offsetToBlockPosition(begin),
                    m_d->offsetToBlockPosition(end),
                    completion_controller);
}

void DocumentModel::beginCompletion(const DocumentPosition &begin, const DocumentPosition &end, CompletionController *completion_controller)
{
    endCompletion();

    m_completion = completion_controller;
    connect(m_completion, &CompletionController::ended, this, &DocumentModel::completerEnded);
    connect(m_completion, &CompletionController::begun, this, &DocumentModel::completerBegun);
    connect(m_completion, &CompletionController::updated, this, &DocumentModel::completerUpdated);
    m_completion->begin(begin, end, cursor()->position());
}

//
// Completer slots.
//

void DocumentModel::completerBegun()
{
    emit completionBegin();
}

void DocumentModel::completerEnded()
{
    emit completionEnd();

    disconnect(m_completion, 0, this, 0);
    if (m_completion->parent() == this) {
        m_completion->deleteLater();
    }
    m_completion = NULL;
}

void DocumentModel::completerUpdated()
{
    emit completionUpdated();
}

//
//
//

void DocumentModel::endCompletion()
{
    if (m_completion) {
        m_completion->end();
    }
}

void DocumentModel::nextCompletion()
{
    if (m_completion) {
        m_completion->next();
    }
}

void DocumentModel::previousCompletion()
{
    if (m_completion) {
        m_completion->previous();
    }
}

void DocumentModel::confirmCompletion()
{
    if (m_completion) {
        m_completion->confirm();
    }
}

void DocumentModel::updateCompletion(const DocumentPosition &begin, const DocumentPosition &end, int shift)
{
    if (m_completion) {
        m_completion->update(begin, end, shift);
    }
}

//
//
//

DocumentModelStatusLineController::DocumentModelStatusLineController(DocumentModel *view) :
    StatusLineController(view),
    m_view(view),
    segment_grammar(NULL)
{

}

QString DocumentModelStatusLineController::filePath()
{
    QString ret(m_view->document()->path());
    if (ret.isEmpty()) {
        ret = "<Unsaved>";
    }
    return ret;
}

QString DocumentModelStatusLineController::cursorPosition()
{
    auto p = m_view->cursor()->position();
    QString ret;
    ret.append(QString::number(p.block + 1).rightJustified(3, ' '));
    ret.append(" : ");
    ret.append(QString::number(p.position + 1).leftJustified(3, ' '));
    return ret; // QString("%1 : %2").arg(p.block + 1, 3).arg(p.position + 1, 3);
}

QString DocumentModelStatusLineController::grammarTitle(QSharedPointer<Grammar> grammar)
{
    if (grammar) {
        return grammar->description();
    }
    return QStringLiteral("None");
}

void DocumentModelStatusLineController::scope(ScopePath *scope, StatusLine *line)
{
    scope->push("status", line);
    m_view->scope(scope);
}

void DocumentModelStatusLineController::install(StatusLine *line)
{
    StatusLineSegment *segment;

    segment = new StatusLineSegment(line, "cursor");
    segment->addText(cursorPosition());
    line->addSegment(segment, 0);
    connect(m_view, &DocumentModel::cursorChanged, this, [=](const DocumentRange &, const DocumentRange &) {
        segment->setText(0, cursorPosition());
    });

    segment = new StatusLineSegment(line, "path");
    segment->addText(filePath());
    line->addSegment(segment, 1);
    connect(m_view, &DocumentModel::pathChanged, this, [=](const QString &) {
        segment->setText(0, filePath());
    });

    segment_grammar = new StatusLineSegment(line, "grammar");
    segment_grammar->addText(grammarTitle(m_view->document()->grammar()));
    line->addSegment(segment_grammar, 0);
    connect(m_view->document(), &Document::grammarChanged, this, &DocumentModelStatusLineController::grammarChanged);

    segment = new StatusLineSegment(line, "encoding");
    segment->addText("UTF-8");
    line->addSegment(segment, 0);

    segment = new StatusLineSegment(line, "line_ending");
    segment->addText("LF");
    line->addSegment(segment, 0);
}

void DocumentModelStatusLineController::uninstall()
{
    disconnect(m_view, 0, this, 0);
    segment_grammar = NULL;
}

void DocumentModelStatusLineController::grammarChanged(QSharedPointer<Grammar> grammar)
{
    if (segment_grammar) {
        segment_grammar->setText(0, grammarTitle(grammar));
    }
}

//
//
//

DocumentBlock::DocumentBlock(DocumentModel *view, int index) :
    m_index(index),
    m_view(view)
{
}

void DocumentBlock::setIndex(int index)
{
    m_index = index;
}

bool DocumentBlock::isValid() const
{
    return m_index >= 0 && m_index < m_view->blockCount();
}

bool DocumentBlock::isFirst() const
{
    return m_index == 0;
}

bool DocumentBlock::isLast() const
{
    return m_index == m_view->blockCount() - 1;
}

DocumentBlock DocumentBlock::next() const
{
    if (m_index == m_view->blockCount()) {
        return *this;
    }
    // It's ok to return an invalid block.
    return m_view->block(m_index + 1);
}

DocumentBlock DocumentBlock::previous() const
{
    if (m_index == -1) {
        return *this;
    }
    // It's ok to return an invalid block.
    return m_view->block(m_index - 1);
}

int DocumentBlock::beginOffset() const
{
    Q_ASSERT(isValid());
    return m_view->document()->blockBeginOffset(m_index);
}

int DocumentBlock::endOffset() const
{
    Q_ASSERT(isValid());
    return m_view->document()->blockEndOffset(m_index);
}

int DocumentBlock::length() const
{
    Q_ASSERT(isValid());
    return m_view->document()->blockLength(m_index);
}

int DocumentBlock::lengthWithoutLE() const
{
    Q_ASSERT(isValid());
    return m_view->document()->blockLengthWithoutLE(m_index);
}

QTextLayout *DocumentBlock::layout() const
{
    Q_ASSERT(isValid());
    return m_view->blockLayout(m_index);
}

//
//
//

DocumentCursor::DocumentCursor(const DocumentCursor &other) :
    m_r(other.m_r),
    // TODO: This is not correct!
    m_vertical_anchor(-1.0f)
{

}

DocumentCursor::DocumentCursor(const DocumentRange &range, qreal vertical_anchor) :
    m_r(range),
    m_vertical_anchor(vertical_anchor)
{
}

//void DocumentCursor::reverse()
//{
//    if (position() == anchor()) {
//        return;
//    }

//    DocumentRange n(m_r);
//    n.reverse();
//    setRange(n);
//}

bool DocumentCursor::containsBlock(int block) const
{
    DocumentRange r(range().sorted());
    if (block >= r.first.block && block <= r.second.block) {
        return true;
    }
    return false;
}

bool DocumentCursor::setRange(const DocumentRange &r)
{
    if (r != m_r) {
        DocumentRange o(m_r);
        m_r = r;
        return true;
        // emit selectionChanged(o, m_r);
    }
    return false;
}

bool DocumentCursor::setRangeSorted(const DocumentPosition &begin, const DocumentPosition &end)
{
    DocumentRange r(m_r);
    r.setSorted(begin, end);
    return setRange(r);
}

bool DocumentCursor::hasSelection()
{
    return position() != anchor();
}

//void DocumentCursor::insertText(DocumentEdit edit, const QString &string)
//{
//    removeText(edit);
//    setPosition(edit->view()->insertText(edit, this, string));
//}

//void DocumentCursor::removeText(DocumentEdit edit)
//{
//    if (position() != anchor()) {
//        setPosition(edit->view()->removeText(edit, this));
//    }
//}

//void DocumentCursor::removeText(DocumentEdit edit, const DocumentPosition &a)
//{
//    if (position() != anchor()) {
//        // Always remove selection first.
//        removeText(edit);
//    } else if (position() != a) {
//        setPosition(edit->view()->removeText(edit, this, a));
//    }
//}


//
// Remove
//

//void DocumentCursor::remove(DocumentEdit edit, RemoveCommand remove_command)
//{
//    if (position() != anchor()) {
//        removeText(edit);
//        return;
//    }

//    switch (remove_command)
//    {
//    case Remove_CharacterBackward: {
//        auto pos(previousPosition(QTextLayout::SkipCharacters, position()));
//        removeText(edit, pos);
//    } break;
//    case Remove_CharacterForward: {
//        auto pos(nextPosition(QTextLayout::SkipCharacters, position()));
//        removeText(edit, pos);
//    } break;
//    }
//}

//
//
//


void DocumentCursor::setVerticalAnchor(qreal anchor)
{
    m_vertical_anchor = anchor;
}

void DocumentCursor::invalidateVerticalAnchor()
{
    m_vertical_anchor = -1.0f;
}
