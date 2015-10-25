#include "precompiled.h"

#include <QPainter>

#include "util.h"
#include "configuration.h"

#include "documentmodel.h"
#include "theme.h"
#include "documenteditor.h"

#include "documenteditorlayout.h"

#ifdef QT_DEBUG
#define CHECK_BLOCKS checkBlocks()
#else
#define CHECK_BLOCKS
#endif

DocumentEditorLayout::DocumentEditorLayout(DocumentEditor *editor, DocumentModel *dv) :
    m_dv(dv),
    m_editor(editor),
    m_width(0),
    m_height(0),
    m_gutter_width(-1),
    m_gutter_width_normal(0)
{
//    m_gutter_icon_arrow_right = QIcon(":/images/IconArrowRight.svg");
//    m_gutter_icon_circle = QIcon(":/images/IconCircle.svg");

    m_width = editor->width();
    m_height = editor->height();

    configured(m_dv->options);

    resetBlocks(0, 0);
}

DocumentEditorLayout::~DocumentEditorLayout()
{
    for (Block * block : m_blocks) {
        delete block;
    }
    for (Block * block : m_pool) {
        delete block;
    }
}

bool DocumentEditorLayout::isValid()
{
    return !m_blocks.isEmpty();
}

void DocumentEditorLayout::configured(const DocumentOptions &previous)
{
    const DocumentOptions &current = m_dv->options;
    bool was_on_top = false;
    if (!m_blocks.empty()) {
        auto block = m_blocks.first();
        was_on_top = block->i == 0 && block->y == previous.padding.top();
    }

    m_gutter_font_icons = QFont("inuaicons");
    m_gutter_font_icons.setPointSize(8);

    updateGutterWidth();

    m_dv->setLayoutWidth(documentRect().width());

    if (was_on_top) {
        auto block = m_blocks.first();
        resetBlocks(block->i, current.padding.top());
    }
}

DocumentRange DocumentEditorLayout::viewportRange()
{
    Block *first = m_blocks.first();
    Block *last = m_blocks.last();
    // Lazy bastard.
    return DocumentRange(DocumentPosition(first->i, 0), DocumentPosition(last->i + 1, 0));
}

QRectF DocumentEditorLayout::gutterRect()
{
    return QRectF(0, 0, gutterWidth(), m_height);
}

QRectF DocumentEditorLayout::documentRect()
{
    QRect r;
    if (m_dv->options.gutter.visible) {
        r.setRect(gutterWidth(), 0, m_width - gutterWidth(), m_height);
    } else {
        r.setRect(0, 0, m_width, m_height);
    }

    return r.adjusted(m_dv->options.padding.left(), 0, -m_dv->options.padding.right(), 0);
}

QRectF DocumentEditorLayout::documentPositionToRect(const DocumentPosition &position)
{
    QRectF rect;
    Block *block = blockAt(position.block);
    if (block == NULL) {
        return rect;
    }

    rect = m_dv->blockCursorRectAtPosition(position);
    rect.moveTopLeft(rect.topLeft() + documentRect().topLeft() + QPointF(0, block->y));
    return rect;
}

void DocumentEditorLayout::resize(qreal w, qreal h)
{
    // NOTE: This is being called with the same w and h in case the gutter size changes.

    QRectF old_document_rect(documentRect());

    // TODO: Limit.
    m_width = w;
    m_height = h;

    QRectF new_document_rect(documentRect());

    if (old_document_rect.width() != new_document_rect.width()) {
        m_dv->setLayoutWidth(new_document_rect.width());
        // Update existing blocks.
        updateBlocks();
        // Clamp or fill as needed.
        if (clampBlocksFromBottom() == false) {
            fillBlocksToBottom();
        }
    } else if (new_document_rect.height() > old_document_rect.height()) {
        // Height is greater, so fill blocks to bottom.
        fillBlocksToBottom();
    } else if (new_document_rect.height() < old_document_rect.height()) {
        // Height is smaller, so remove blocks from bottom.
        clampBlocksFromBottom();
    }
    m_editor->update();
}

void DocumentEditorLayout::scrollBy(qreal dx, qreal dy)
{
    Q_UNUSED(dx);

    // PERFORMANCE_TIMER(t, __FUNCTION__);

    if (dy > 0) {
        scrollDown(dy);
    } else if (dy < 0) {
        scrollUp(dy);
    }
}

void DocumentEditorLayout::scrollPositionToBottom(const DocumentPosition &p)
{
    if (m_blocks.isEmpty()) {
        return;
    }

    qreal bottom = m_height;

    QRectF rect = m_dv->blockCursorRectAtPosition(p);
    qreal block_y = -rect.bottom();
    qreal delta = -bottom;

    qreal first_y;
    int first_i = findBlockForY(p.block, block_y, delta, &first_y);
    // resetBlocks(first_i, first_y);
    scrollToBlock(first_i, first_y);
}

void DocumentEditorLayout::scrollToRange(const DocumentRange &r)
{
    if (m_blocks.isEmpty()) {
        return;
    }

    qreal boundary = m_dv->options.scroll_threshold + m_dv->options.padding.top(); // m_dv->lineHeight() * 3;
    qreal top = boundary;
    qreal bottom = m_height - boundary;

    bool snap_bottom = false;
    bool snap_top = false;

    if (r.first.block < m_blocks.first()->i) {
        // Snap to top boundary.
        snap_top = true;
    } else if (r.first.block > m_blocks.last()->i) {
        // Snap to bottom boundary.
        snap_bottom = true;
    } else {
        // The position is within the visible blocks.
        Block *block = blockAt(r.first.block);
        Q_ASSERT(block != NULL);
        // Get the rectangle of the position.
        QRectF rect = m_dv->blockCursorRectAtPosition(r.first);
        // Move the rectangle by block's y (as it was in block-coordinates)
        rect.moveTop(block->y);

        // Determine which boundary we have crossed with the position rectangle.
        if (rect.bottom() > bottom) {
            // Snap to bottom boundary.
            snap_bottom = true;
        } else if (rect.top() < top) {
            // Snap to top boundary.
            snap_top = true;
        }
    }

    if (snap_bottom) {
        QRectF rect = m_dv->blockCursorRectAtPosition(r.first);
        qreal block_y = -rect.bottom();
        qreal delta = -bottom;

        qreal first_y;
        int first_i = findBlockForY(r.first.block, block_y, delta, &first_y);
        // resetBlocks(first_i, first_y);
        scrollToBlock(first_i, first_y);
    } else if (snap_top) {
        QRectF rect = m_dv->blockCursorRectAtPosition(r.first);
        qreal block_y = -rect.top();
        qreal delta = -top;

        qreal first_y;
        int first_i = findBlockForY(r.first.block, block_y, delta, &first_y);
        // resetBlocks(first_i, first_y);
        scrollToBlock(first_i, first_y);
    }
}

void DocumentEditorLayout::scrollToBlock(int i, qreal y)
{
    // OPTIMIZE: This is a good opportunity to estimate the distance (maybe just by the difference of i's).
    // And do the jump to a page before the target and scroll down to the target smoothly.

    // Find distance between current first block and i, y.
    Block *first = m_blocks.first();
    // qDebug() << __FUNCTION__ << "Scrolling from" << first->i << -first->y << "to" << i << -y;
    qreal distance = findDistance(first->i, -first->y, i, -y);
    // qDebug() << __FUNCTION__ << "distance" << distance;
    scrollBy(0, distance);
}

bool DocumentEditorLayout::isLastBlockVisible()
{
    if (m_blocks.isEmpty()) {
        return false;
    }

    int last_block = m_dv->blockCount() - 1;
    if (m_blocks.last()->i != last_block) {
        return false;
    }
    if (layoutBottom() > blockBottom(m_blocks.last())) {
        return false;
    }
    return true;
}

void DocumentEditorLayout::textChanged(const DocumentEditEvent *event)
{
    if (event->type == DocumentEditEvent::Insert)
    {
        textInserted(event);
    }
    else // if (event->type == DocumentEditEvent::Remove)
    {
        textRemoved(event);
    }
}

void DocumentEditorLayout::formatChanged(int)
{
    updateGutterWidth();
    m_editor->update();
}

void DocumentEditorLayout::cursorChanged(const DocumentRange &, const DocumentRange &)
{
    m_editor->update();
}


//
//
//

void DocumentEditorLayout::textInserted(const DocumentEditEvent *event)
{
    CHECK_BLOCKS;

    // qDebug() << "onDocumentBlocksAdded" << event->block_list_change_first << event->block_list_change_count;
    // TODO(cohen) Animate the insert.
    // - Simply insert a new line, animate it's height from zero to height it actually needs.

    if (event->block_list_change_count > 0)
    {
        // NOTE(cohen): Insert has two possible modes:
        // - Lines were inserted immediately above the changed line.
        // - Lines were inserted immediately below the changed line.

        // TODO(cohen): If we're changing anything above the current viewport, we only need to reindex.
        // TODO(cohen): If we're changing anything below the current viewport, we will just ignore that.

        if (event->block_text_change > event->block_list_change_first) {
            // First insert, then update the changed block.
            insertBlocks(event->block_list_change_first, event->block_list_change_count);
            // Is this really necessary? Insert is layouting text itself.
            // updateBlock(event->block_text_change);
            CHECK_BLOCKS;
        } else if (event->block_text_change < event->block_list_change_first) {
            // First update, then insert.
            updateBlock(event->block_text_change);
            CHECK_BLOCKS;
            insertBlocks(event->block_list_change_first, event->block_list_change_count);
            CHECK_BLOCKS;
        }
        updateGutterWidth();
    }
    else if (event->block_text_change >= 0)
    {
        // Single line has been changed.
        updateBlock(event->block_text_change);
        CHECK_BLOCKS;
    }

//    CHECK_BLOCKS();
//    updateScrollbarPosition();

    m_editor->update();
}

void DocumentEditorLayout::textRemoved(const DocumentEditEvent *event)
{
    // qDebug() << "onDocumentBlocksRemoved" << event->block_list_change_first << event->block_list_change_count;

    if (event->block_list_change_count > 0)
    {
        Q_ASSERT(event->block_list_change_first == (event->block_text_change+1));

        int i = visibleIndex(event->block_text_change);
        if (i != -1) {
            Block *block = m_blocks[i];
            setupBlock(block);
        }

        removeBlocks(event->block_list_change_first, event->block_list_change_count);
    }
    else if (event->block_text_change >= 0)
    {
        updateBlock(event->block_text_change);
    }

    CHECK_BLOCKS;

//    CHECK_BLOCKS();
//    updateScrollbarPosition();
    updateGutterWidth();
    m_editor->update();
}

void DocumentEditorLayout::scrollUp(qreal dy)
{
    qreal y = 0.0;
    int i = findBlockForY(dy, &y);
    qDebug() << __FUNCTION__ << i << y;
    resetBlocks(i, y);
//    if (i < m_blocks.first()->i) {
//        // The
//    }
    CHECK_BLOCKS;
}

void DocumentEditorLayout::scrollDown(qreal dy)
{
    // qDebug() << "onScrollDown" << y;

    qreal y = 0.0;
    int i = findBlockForY(dy, &y);
    qDebug() << __FUNCTION__ << dy << i << y;

    if (i > m_blocks.last()->i) {
        // We have to completely rebuild our view as we scrolled past the last block.
        resetBlocks(i, y);
    } else {
        // We have to remove everything above the i.
        for (;;) {
            Block *block = m_blocks[0];
            if (block->i < i) {
                removeBlock(0);
            } else {
                break;
            }
        }

        // Move the block at i to the top (block_y), including the remaining ones.
        Block *block = m_blocks.first();
        qreal y_delta = y - block->y;
        for (Block *block : m_blocks) {
            block->y += y_delta;
        }
        // Fill the rest.
        fillBlocksToBottom();
    }

    CHECK_BLOCKS;
}

//
//
//

int DocumentEditorLayout::visibleIndex(int i)
{
    if (m_blocks.empty()) {
        return -1;
    }

    if (i < m_blocks.first()->i || i > m_blocks.last()->i) {
        return -1;
    }

    return i - m_blocks.first()->i;
}

DocumentEditorLayout::Block *DocumentEditorLayout::blockAt(int i)
{
    int vi = visibleIndex(i);
    if (vi == -1) {
        return NULL;
    }
    return m_blocks[vi];
}

DocumentEditorLayout::Block *DocumentEditorLayout::createBlock(int i)
{
    Q_ASSERT(i < m_dv->blockCount());

    Block *block;
    if (m_pool.empty()) {
        block = new Block();
    } else {
        block = m_pool.pop();
    }
    block->i = i;
    setupBlock(block);

    return block;
}

void DocumentEditorLayout::removeBlock(int vi)
{
    Block *block = m_blocks[vi];
    m_blocks.remove(vi);
    destroyBlock(block);
}

void DocumentEditorLayout::destroyBlock(Block *block)
{
    block->i = -1;
    m_pool.push(block);
}

int DocumentEditorLayout::setupBlock(int i)
{
    Block *block = blockAt(i);
    if (block != NULL) {
        return setupBlock(block);
    }
    return 0;
}

int DocumentEditorLayout::setupBlock(Block *block)
{
    int height = m_dv->blockHeight(block->i);
    int height_o = block->h;
    block->h = height;
    return height_o - height;
}

void DocumentEditorLayout::updateBlock(int block_index)
{
    int visible_index = visibleIndex(block_index);
    if (visible_index != -1) {
        Block *block = m_blocks[visible_index];
        updateBlock(block);
    }
}

void DocumentEditorLayout::updateBlock(Block *block)
{
    int height_delta = setupBlock(block);
    if (height_delta != 0) {
        layoutBlocks();
        if (height_delta < 0) {
            fillBlocksToBottom();
        } else {
            clampBlocksFromBottom();
        }
    }
}

void DocumentEditorLayout::layoutBlocks()
{
    if (m_blocks.empty()) {
        return;
    }

    int y = blockBottom(m_blocks[0]);
    if (m_blocks.size() > 1) {
        layoutBlocksParametric(1, y);
    }
}

void DocumentEditorLayout::layoutBlocksParametric(int visible_index, int y)
{
    Q_ASSERT(visible_index >= 0 && visible_index < m_blocks.size());

    for (int i = visible_index; i < m_blocks.size(); i++) {
        layoutBlockBelow(m_blocks[i], &y);
    }

    CHECK_BLOCKS;
}

void DocumentEditorLayout::reindexBlocksParametric(int visible_index, int block_index)
{
    for (int i = visible_index; i < m_blocks.size(); i++) {
        m_blocks[i]->i = block_index;
        block_index++;
    }
}


//
// Block fills and clamps.
//

void DocumentEditorLayout::updateBlocks()
{
    if (m_blocks.empty()) {
        return;
    }

    int y = blockTop(m_blocks.first());
    for (int i = 0; i < m_blocks.size(); i++) {
        setupBlock(m_blocks[i]);
        layoutBlockBelow(m_blocks[i], &y);
    }

    CHECK_BLOCKS;
}

bool DocumentEditorLayout::clampBlocksFromBottom()
{
    if (m_blocks.empty()) {
        // There is nothing to clamp.
        return false;
    }

    // If the last block is above the viewport's bottom, then we have nothing to do.
    Block *block = m_blocks.last();
    if (blockTop(block) < layoutBottom()) {
        return false;
    }

    // Remove blocks that are below the viewport.
    // We always have to keep at least the top one block.
    while ((m_blocks.size() > 1) && blockTop(m_blocks.last()) >= layoutBottom()) {
        removeBlock(m_blocks.size() - 1);
    }

    CHECK_BLOCKS;

    return true;
}

bool DocumentEditorLayout::fillBlocksToBottom()
{
    Block *block;
    int y;
    int i;
    if (!m_blocks.empty())
    {
        block = m_blocks.last();
        if (block->i == m_dv->blockCount() - 1) {
            // The last block is actually last block of the view. So we have nothing to fill.
            return true;
        }

        if (blockTop(block) >= layoutBottom()) {
            // Last block is below the viewport, so we have nothing to fill.
            return true;
        } else if (blockBottom(block) >= layoutBottom()) {
            // Last block's bottom is below the viewport, so we have nothing to fill.
            return true;
        }

        // At what y to start the fill.
        y = blockBottom(block);
        // At what i to start the fill.
        i = block->i + 1;
    }
    else
    {
        Q_ASSERT(false && "Unable to fill to bottom.");
        return false;
    }

    fillBlocksToBottomParametric(i, y);

    return true;
}

void DocumentEditorLayout::fillBlocksToBottomParametric(int i, int y)
{
    // Parameter check.
    Q_ASSERT(m_blocks.empty() || i == (m_blocks.last()->i+1));
    Q_ASSERT(m_blocks.empty() || y == (blockBottom(m_blocks.last())));

    Block *block;
    while (i < m_dv->blockCount())
    {
        block = createBlock(i);
        m_blocks.append(block);
        layoutBlockBelow(block, &y);
        if (blockBottom(block) >= layoutBottom()) {
            break;
        }
        i++;
    }

    CHECK_BLOCKS;
}

//void DocumentEditorLayout::reset()
//{
//}

void DocumentEditorLayout::resetBlocks(int block_i, int block_y)
{
    if (!m_blocks.empty())
    {
        Block *block = m_blocks.first();
        if (block->i != block_i || blockTop(block) != block_y)
        {
            while (!m_blocks.empty()) {
                removeBlock(0);
            }
            fillBlocksToBottomParametric(block_i, block_y);
        }
    }
    else
    {
        fillBlocksToBottomParametric(block_i, block_y);
    }

    CHECK_BLOCKS;
}

qreal DocumentEditorLayout::findDistance(int from_i, qreal from_y, int to_i, qreal to_y)
{
    // COMMENTED: Temporarily
//    Q_ASSERT(from_y >= 0);
//    Q_ASSERT(to_y >= 0);

    qreal delta = 0;
    if (to_i > from_i) {
        delta -= from_y;
        for (int j = from_i; j < to_i; j++) {
            delta += m_dv->blockHeight(j);
        }
        delta += to_y;
        Q_ASSERT(delta >= 0);
    } else if (to_i < from_i) {
        delta -= from_y;
        for (int j = from_i - 1; j != to_i; j--) {
            delta -= m_dv->blockHeight(j);
        }
        delta -= m_dv->blockHeight(to_i)-to_y;
        Q_ASSERT(delta <= 0);
    } else if (to_i == from_i) {
        if (to_y > from_y) {
            delta = to_y - from_y;
            Q_ASSERT(delta >= 0);
        } else if (to_y < from_y) {
            delta = to_y - from_y;
            Q_ASSERT(delta <= 0);
        }
    }
    return delta;
}

int DocumentEditorLayout::findBlockForY(qreal delta, qreal *o_y)
{
    Block *first = m_blocks.first();
    return findBlockForY(first->i, first->y, delta, o_y);
}

int DocumentEditorLayout::findBlockForY(int from_i, qreal from_y, qreal target_y, qreal *o_y)
{
    int i = from_i;
    qreal block_y = from_y;
    qreal block_h;

    if (target_y > 0) {
        for (;;) {
            block_h = m_dv->blockHeight(i);
            if (block_y + block_h >= target_y) {
                *o_y = - (target_y - block_y);
                return i;
            }
            block_y += block_h;
            i++;
            if (i >= m_dv->blockCount()) {
                // TODO: Here we snap beyond the document!
                *o_y = block_y + block_h;
                return m_dv->blockCount() - 1;
            }
        }
    } else {
        for (;;) {
            if (block_y <= target_y) {
                *o_y = - (target_y - block_y);
                return i;
            }
            i--;
            if (i < 0) {
                *o_y = qMin(m_dv->options.padding.top(), - (target_y - block_y));
                return 0;
            }
            block_h = m_dv->blockHeight(i);
            block_y -= block_h;
        }
    }
    *o_y = from_y; // first->y;
    return from_i; // first->i;
}

void DocumentEditorLayout::insertBlocks(int block_index, int count)
{
    Q_ASSERT(!m_blocks.empty());

    if (block_index < m_blocks.first()->i) {
        // Blocks were inserted above the visible blocks, so only do the reindex.
        reindexBlocksParametric(0, m_blocks.first()->i + count);
        CHECK_BLOCKS;
        return;
    }

    int i, insert_index;
    int y, insert_y;
    Block *block;

    if (block_index > m_blocks.last()->i) {
        if (blockBottom(m_blocks.last()) > m_height) {
            // Blocks were inserted below the visible blocks.
            // Nothing to do.
            return;
        } else if (block_index == (m_blocks.last()->i + 1)){
            // Blocks were appended.
            insert_index = m_blocks.size();
            insert_y = blockBottom(m_blocks.last());
            // qDebug() << __FUNCTION__ << "Appended" << insert_index << insert_y;
        } else {
            // Blocks were inserted on an index that is not consecutive to
            // our last block.
            // Nothing to do.
            return;
        }
    } else {
        // Blocks were inserted.
        insert_index = visibleIndex(block_index);
        Q_ASSERT(insert_index  != -1);

        // Remember where we are inserting the lines.
        if (insert_index  == 0) {
            insert_y = blockTop(m_blocks.first());
        } else {
            insert_y = blockBottom(m_blocks[insert_index -1]);
        }

        // Advance y by heights of all new blocks that fit to viewport.
        y = insert_y;
        for (i = 0; i < count; i++) {
            // We must use m_dv->blockHeight() as the lines are new.
            if (!layoutAdvanceBelow(m_dv->blockHeight(block_index + i), &y)) {
                break;
            }
        }

        // Layout current blocks and remove those who would not fit.
        for (i = insert_index; i < m_blocks.size(); ) {
            block = m_blocks[i];
            if (layoutEndBottom(&y)) {
                removeBlock(i);
            } else {
                layoutBlockBelow(block, &y);
                block->i = block->i + count;
                i++;
            }
        }
    }

    // Reset layouter and insert the new blocks.
    y = insert_y;
    for (i = 0; i < count; i++) {

        block = createBlock(block_index + i);
        m_blocks.insert(insert_index + i, block);
        if (!layoutBlockBelow(block, &y)) {
            break;
        }
    }
}

void DocumentEditorLayout::removeBlocks(int block_index, int count)
{
    Q_ASSERT(count > 0);
    if (m_blocks.empty()) {
        return;
    }

    int block_first = block_index;
    int block_last = block_index + count - 1;

    if (block_last < m_blocks.first()->i) {
        reindexBlocksParametric(0, m_blocks.first()->i + count);
        CHECK_BLOCKS;
        return;
    }

    if (block_first > m_blocks.last()->i) {
        return;
    }

    if (block_first < m_blocks.first()->i) {
        block_first = m_blocks.first()->i;
    }

    if (block_last > m_blocks.last()->i) {
        block_last = m_blocks.last()->i;
    }

    int visible_index = visibleIndex(block_first);
    Q_ASSERT(visible_index != -1);
    if (visible_index == 0) {
        // Removing lines from the top.

        // Remember the y position of the first block.
        int y = blockTop(m_blocks.first());

        // Remove the blocks.
        int count = block_last - block_first + 1;
        for ( ; count > 0; count--) {
            removeBlock(visible_index);
        }

        // Insert a block on top.
        Block *block = createBlock(block_first - 1);
        m_blocks.prepend(block);
        layoutBlockBelow(block, &y);

        reindexBlocksParametric(visible_index+1, block_first);
        layoutBlocksParametric(visible_index+1, y);

        if (clampBlocksFromBottom() == false) {
            fillBlocksToBottom();
        }
    } else {
        int count = block_last - block_first + 1;
        for ( ; count > 0; count--) {
            removeBlock(visible_index);
        }
        if (visible_index < m_blocks.size()) {
            // Deal with the following blocks.
            reindexBlocksParametric(visible_index, block_first);
            layoutBlocksParametric(visible_index, blockBottom(m_blocks[visible_index-1]));
        }

        fillBlocksToBottom();
    }
}

//
//
//

bool DocumentEditorLayout::layoutBlockBelow(Block *block, int *y)
{
    block->y = *y;
    return layoutAdvanceBelow(block->h, y);
}

bool DocumentEditorLayout::layoutBlockAbove(Block *block, int *y)
{
    block->y = *y - block->h;
    return layoutAdvanceAbove(block->h, y);
}

bool DocumentEditorLayout::layoutAdvanceBelow(int block_height, int *y)
{
    *y += block_height;
    return !layoutEndBottom(y);
}

bool DocumentEditorLayout::layoutAdvanceAbove(int block_height, int *y)
{
    *y -= block_height;
    return !layoutEndTop(y);
}

bool DocumentEditorLayout::layoutEndBottom(int *y)
{
    return ((*y) > layoutBottom());
}

bool DocumentEditorLayout::layoutEndTop(int *y)
{
    return ((*y) < layoutTop());
}

qreal DocumentEditorLayout::blockTop(Block *block)
{
    return block->y;
}

qreal DocumentEditorLayout::blockBottom(Block *block)
{
    return block->y + block->h;
}

qreal DocumentEditorLayout::layoutTop()
{
    return 0; //-m_shift_y;
}

qreal DocumentEditorLayout::layoutBottom()
{
    return m_height;
}

//
// Painting
//

void DocumentEditorLayout::paint(QPainter *painter)
{
    if (m_blocks.empty()) {
        return;
    }

    // PERFORMANCE_TIMER(t, __FUNCTION__);

    QRectF block_rect(documentRect());
    QVector<QTextLayout::FormatRange> selections;
    QSharedPointer<Theme> theme(Configuration::theme());

    QList<DocumentCursor *> cursors;
    // qDebug() << __FUNCTION__ << "Viewport" << viewportRange();
    m_dv->findCursors(viewportRange(), &cursors);
    QList<DocumentCursor *>::iterator cursor_it = cursors.begin();
    QList<DocumentCursor *>::iterator selection_it = cursors.begin();
    // int cursor_block = m_dv->cursor()->position().block;

    Block *block = m_blocks.first();
    if (block->y > 0) {
        painter->fillRect(QRectF(0, block_rect.top(), m_width, block->y), theme->backgroundColor());
    }

    for (Block *block : m_blocks)
    {
//        qDebug() << __FUNCTION__ << block->i << "Painting";
        block_rect.moveTop(block->y);
        block_rect.setHeight(block->h);

        // Block background
        painter->fillRect(block_rect.adjusted(-m_dv->options.padding.left(), 0, m_dv->options.padding.right(), 0), theme->backgroundColor());

        // Block text
        QTextLayout *block_layout = m_dv->blockLayout(block->i);

        selections.clear();
        blockSelections(selection_it, cursors.end(), block->i, theme, &selections);
        // TODO: clip
        painter->setPen(theme->foregroundColor());
        block_layout->draw(painter, QPoint(block_rect.x(), block_rect.y()), selections);


        paintBlockCursors(painter, cursor_it, cursors.end(), block->i, block_rect.x(), block_rect.y(), theme);

//        // Block cursor
//        if (block->i == cursor_block) {
//            // Draw cursor
//            QRectF rect = m_dv->blockCursorRectAtPosition(m_dv->cursor()->position());
//            rect.moveLeft(block_rect.x() + rect.x());
//            rect.moveTop(block_rect.y() + rect.y());
//            painter->fillRect(rect, theme->caretColor());
//        }

        // Block gutter
        if (m_dv->options.gutter.visible) {
            paintGutter(painter, block_rect.y(), block_rect.height(), block_layout->lineAt(0).height(), block->i, theme);
        }
    }

    // Fill the rest of the editor.
    if (block_rect.bottom() < m_height) {
        painter->fillRect(QRectF(0, block_rect.bottom(), m_width, m_height - block_rect.bottom()), theme->backgroundColor());
    }

    // Boundary.
//    painter->fillRect(QRectF(0, m_height - 80, m_width, 80), QColor(255, 0, 0, 128));
//    painter->fillRect(QRectF(0, 0, m_width, 80), QColor(255, 0, 0, 128));
}

bool DocumentEditorLayout::blockSelections(QList<DocumentCursor *>::iterator &cursor_it, QList<DocumentCursor *>::iterator &cursor_end, int i, QSharedPointer<Theme> theme, QVector<QTextLayout::FormatRange> *ret)
{
    if (cursor_it == cursor_end) {
        return false;
    }

    // Find first cursor that starts on this block. (Skip all cursors that were before)

    while (cursor_it != cursor_end && (*cursor_it)->end().block < i) {
        cursor_it++;
    }

    if (cursor_it == cursor_end || !(*cursor_it)->containsBlock(i)) {
        return false;
    }

    bool has_more = true;
    while (cursor_it != cursor_end && has_more)
    {
        DocumentCursor *cursor = *cursor_it;
        if (!cursor->hasSelection()) {
            return false;
        }

        int begin = -1;
        int end = -1;

        DocumentRange r = cursor->range().sorted();

        qDebug() << __FUNCTION__ << i << "Selection" << r;

        if (i == r.first.block && i == r.second.block) {
            // One line selection.
            begin = r.first.position;
            end = r.second.position;
            has_more = true;
        } else if (i == r.first.block) {
            // Set partial beginning selection.
            begin = r.first.position;
            end = INT_MAX;
            has_more = false;
        } else if (i == r.second.block) {
            // Set partial ending selection.
            begin = 0;
            end = r.second.position;
            has_more = true;
        } else if (i >= r.first.block && i <= r.second.block) {
            // Set full selection.
            begin = 0;
            end = INT_MAX;
            has_more = false;
        } else {
            qDebug() << __FUNCTION__ << i << r;
            Q_ASSERT(0 && "Not reached");
        }

        if (begin != -1) {
            // qDebug() << __FUNCTION__ << "Selection on line" << i << begin << end;
            ret->append(QTextLayout::FormatRange());
            QTextLayout::FormatRange *f = &ret->back();
            f->start = begin;
            f->length = end - begin;
            f->format.setBackground(theme->selectionColor());
        }

        if (!has_more) {
            break;
        }
        cursor_it++;
    }

    return ret->isEmpty();
}

void DocumentEditorLayout::paintBlockCursors(QPainter *painter, QList<DocumentCursor *>::iterator &cursor_it, QList<DocumentCursor *>::iterator &cursor_end, int i, qreal block_x, qreal block_y, QSharedPointer<Theme> theme)
{
    while (cursor_it != cursor_end && (*cursor_it)->position().block < i) {
        cursor_it++;
    }

    while (cursor_it != cursor_end && (*cursor_it)->position().block == i)
    {
        DocumentCursor *cursor = *cursor_it;
        // qDebug() << __FUNCTION__ << "Cursor on line" << i << cursor->position();
        // Draw cursor
        if (m_editor->hasFocus()) {
            QRectF rect = m_dv->blockCursorRectAtPosition(cursor->position());
            rect.moveLeft(block_x + rect.x());
            rect.moveTop(block_y + rect.y());
            painter->fillRect(rect, theme->caretColor());
        }
        cursor_it++;
    }
}


//void DocumentEditorLayout::setGutterType(GutterType type)
//{
//    if (type != m_gutter_type) {
//        m_gutter_type = type;
//        updateGutterWidth();
//        m_editor->update();
//    }
//}

void DocumentEditorLayout::updateGutterWidth()
{
    qreal width = 0;

    GutterOptions &options = m_dv->options.gutter;

    width = options.padding.left() + options.padding.right();

    if (options.format == "%")
    {
        int i = QString::number(m_dv->blockCount()).length();
        QFontMetrics metrics(options.font);
        width += metrics.width(" ") * i;
    }
    else // Circle, ArrowRight,...
    {
        int i = QString::number(m_dv->blockCount()).length();
        QFontMetrics metrics(options.font);
        width += metrics.width(" ") * i;
    }

    if (width != m_gutter_width_normal) {
        m_gutter_width_normal = width;
        setGutterWidth(m_gutter_width_normal);
        emit normalGutterWidthChanged();
    }
}

void DocumentEditorLayout::setGutterWidth(qreal gutter_width)
{
    if (gutter_width > m_gutter_width) {
        m_gutter_width = gutter_width;
        resize(m_width, m_height);
    }
}


void DocumentEditorLayout::paintGutter(QPainter *p, qreal y, qreal h, qreal line_height, int block, QSharedPointer<Theme> theme)
{
    GutterOptions &options = m_dv->options.gutter;

    QRectF gutter_rect(gutterRect());
    gutter_rect.setTop(y);
    gutter_rect.setHeight(h);
    p->fillRect(gutter_rect, options.background);

    gutter_rect.setHeight(line_height);
    gutter_rect.adjust(options.padding.left(),
                       options.padding.top(),
                       -options.padding.right(),
                       -options.padding.bottom());

    p->setPen(options.foreground);

    if (m_dv->options.gutter.format == "%")
    {
        // gutter_rect.setWidth();
        p->setFont(options.font);
        p->drawText(gutter_rect, Qt::AlignRight | Qt::AlignVCenter, QString::number(block + 1));
    }
    else
    {
        p->setFont(m_gutter_font_icons);
        p->drawText(gutter_rect, Qt::AlignRight | Qt::AlignVCenter, options.format);
    }
}

void DocumentEditorLayout::checkBlocks()
{
    if (m_blocks.isEmpty()) {
        return;
    }

    const char *reason = NULL;
    Block *previous_block = m_blocks.first();
    Block *block;
    int i = 1;
    for (; i < m_blocks.size(); i++)
    {
        block = m_blocks[i];
        if(block->i != (previous_block->i + 1)) {
            reason = "line index";
            break;
        }
        if(block->y != (previous_block->y + previous_block->h)) {
            reason = "line y-offset";
            break;
        }

        previous_block = block;
    }

    if (reason != NULL) {
        int j = 0;
        Block *b;
        for (; j < m_blocks.size(); j++) {
            b = m_blocks[j];
            qDebug() << qSetPadChar(' ')
                     << qSetFieldWidth(0) << "vi = " << qSetFieldWidth(4) << j
                     << qSetFieldWidth(0) << " i = " << qSetFieldWidth(4) << b->i
                     << qSetFieldWidth(0) << " y = " << qSetFieldWidth(4) << b->y
                     << qSetFieldWidth(0) << "     " << qSetFieldWidth(4) << (b->y + b->h)
                     << qSetFieldWidth(0) << (b == block ? reason : "");
        }
        Q_ASSERT(0 && "Check blocks failed.");
    }
}
