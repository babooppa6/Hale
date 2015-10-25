#ifndef EDITORLAYOUT_H
#define EDITORLAYOUT_H

#include <QObject>
#include <QStack>
#include <QPainter>
#include <QTextLayout>
#include <QIcon>

#include "documenttypes.h"

class Theme;

class DocumentEditor;
class DocumentEditEvent;
class DocumentRange;
class DocumentModel;
class DocumentCursor;

struct DocumentOptions;

class DocumentEditorLayout : public QObject
{
    Q_OBJECT

public:
    DocumentEditorLayout(DocumentEditor *editor, DocumentModel *dv);
    ~DocumentEditorLayout();

    struct Block
    {
        // Block index
        int i;
        // Block y-position in the viewport
        qreal y;
        // Block height
        qreal h;
    };


    /// Initializes the viewport.
    void init();
    /// Resizes the viewport.
    void resize(qreal w, qreal h);

    /// Scrolls the viewport by given deltas.
    void scrollBy(qreal dx, qreal dy);
    /// Scrolls the viewport so that the given position is on the bottom of the viewport.
    void scrollPositionToBottom(const DocumentPosition &p);
    /// Called to scroll to given range.
    void scrollToRange(const DocumentRange &r);
    /// Scrolls so block's i y is on the top.
    void scrollToBlock(int i, qreal y);
    /// Returns true if last document's block is fully visible
    bool isLastBlockVisible();


    bool isValid();
    DocumentRange viewportRange();

    /// Gutter width that is calculated by the editor.
    qreal normalGutterWidth()
    { return m_gutter_width_normal; }
    /// Gutter width that is set externally to the editor. If none is set, then returns normalGutterWidth()
    qreal gutterWidth()
    { return m_gutter_width < 0 ? m_gutter_width_normal : m_gutter_width; }
    /// Sets gutter width, but only in case the gutter_width is larger than normalGutterWidth()
    void setGutterWidth(qreal gutter_width);

    // void setGutterOptions(const DocumentGutterOptions &options);

    void configured(const DocumentOptions &previous);
    void textChanged(const DocumentEditEvent *event);
    void formatChanged(int block);
    void cursorChanged(const DocumentRange &o, const DocumentRange &n);

    void paint(QPainter *painter);

    QRectF gutterRect();
    QRectF documentRect();

    QRectF documentPositionToRect(const DocumentPosition &position);

signals:
    void normalGutterWidthChanged();

private:
    DocumentModel *m_dv;
    DocumentEditor *m_editor;

    int m_width;
    int m_height;

//    qreal m_scroll_boundary;
//    qreal m_padding_top;

    typedef QStack<Block *> Pool;
    Pool m_pool;

    typedef QVector<Block *> Blocks;
    Blocks m_blocks;

    // QFont m_gutter_font_numbers;
    QFont m_gutter_font_icons;
    qreal m_gutter_width;
    qreal m_gutter_width_normal;

//    DocumentGutterOptions m_gutter_options;
//    QIcon m_gutter_icon_circle;
//    QIcon m_gutter_icon_arrow_right;

    //
    //
    //

    void textInserted(const DocumentEditEvent *event);
    void textRemoved(const DocumentEditEvent *event);

    void scrollUp(qreal dy);
    void scrollDown(qreal dy);

    //
    //
    //

    int visibleIndex(int i);
    Block *blockAt(int i);

    /// Pulls a block from pool or creates a new one if pool is empty.
    Block *createBlock(int i);
    /// Adds block to the pool. Does not remove it from the m_blocks (!) use removeBlock for that.
    void destroyBlock(Block *block);
    /// Sets the block's properties.
    int setupBlock(int i);
    int setupBlock(Block *block);
    /// Gets what's changed on the block and updates it, and all blocks that are affected.
    void updateBlock(int block_index);
    void updateBlock(Block *block);
    /// Walks through the m_blocks and updates block's y.
    void layoutBlocks();
    void layoutBlocksParametric(int visible_index, int y);
    /// Updates indexes on the blocks.
    void reindexBlocksParametric(int vi, int i);

    /// Removes block from the layout and calls destroyBlock()
    void removeBlock(int vi);

    //
    //
    //

    void updateBlocks();

    bool clampBlocksFromBottom();
    bool fillBlocksToBottom();

    void fillBlocksToBottomParametric(int i, int y);

    qreal findDistance(int from_i, qreal from_y, int to_i, qreal to_y);
    int findBlockForY(qreal delta, qreal *o_y);
    int findBlockForY(int from_i, qreal from_y, qreal target_y, qreal *o_y);

    /// Inserts blocks to the m_blocks.
    void insertBlocks(int block_index, int count);
    /// Removes blocks from the m_blocks.
    void removeBlocks(int block_index, int count);

    // void reset();
    void resetBlocks(int i, int y);

    //
    //
    //

    bool layoutBlockBelow(Block *block, int *y);
    bool layoutBlockAbove(Block *block, int *y);
    bool layoutAdvanceBelow(int block_height, int *y);
    bool layoutAdvanceAbove(int block_height, int *y);
    bool layoutEndBottom(int *y);
    bool layoutEndTop(int *y);

    qreal blockTop(Block *block);
    qreal blockBottom(Block *block);
    qreal layoutTop();
    qreal layoutBottom();

    void updateGutterWidth();
    void paintGutter(QPainter *p, qreal y, qreal block_height, qreal line_height, int block, QSharedPointer<Theme> theme);

    bool blockSelections(QList<DocumentCursor *>::iterator &cursor_it,
                         QList<DocumentCursor *>::iterator &cursor_end,
                         int block,
                         QSharedPointer<Theme> theme,
                         QVector<QTextLayout::FormatRange> *ret
                         );

    void paintBlockCursors(QPainter *painter,
                           QList<DocumentCursor *>::iterator &cursor_it,
                           QList<DocumentCursor *>::iterator &cursor_end,
                           int block,
                           qreal block_x,
                           qreal block_y,
                           QSharedPointer<Theme> theme
                           );

    void checkBlocks();
};

#endif // EDITORLAYOUT_H
