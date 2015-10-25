#include "precompiled.h"

#include "configuration.h"
#include "util.h"

#include "document.h"
#include "documentmodel.h"
#include "frameless.h"

#include "completion.h"

CompletionController::CompletionController(DocumentModel *view, QObject *parent) :
    QObject(parent),
    m_dv(view)
{
}

Document *CompletionController::document()
{
    return m_dv->document();
}

void CompletionController::begin(const DocumentPosition &begin, const DocumentPosition &end, const DocumentPosition &cursor)
{
    m_begin = begin;
    m_end = end;
    if (m_begin > m_end) {
        std::swap(m_begin, m_end);
    }
    m_cursor = document()->blockPositionToOffset(cursor);

    beginEvent();
    emit begun();

    update();
}

void CompletionController::end()
{
    endEvent();
    emit ended();
}

void CompletionController::next()
{
    int row = 0;
    QModelIndex index = selectionModel()->currentIndex();
    if (index.isValid()) {
        row = index.row();
    }
    QModelIndex next = model()->index(row + 1, 0);
    if (next.isValid()) {
        selectionModel()->setCurrentIndex(next, QItemSelectionModel::Clear | QItemSelectionModel::Select);
    }
}

void CompletionController::previous()
{
    int row = 0;
    QModelIndex index = selectionModel()->currentIndex();
    if (index.isValid()) {
        row = index.row();
    }
    QModelIndex previous = model()->index(row - 1, 0);
    if (previous.isValid()) {
        selectionModel()->setCurrentIndex(previous, QItemSelectionModel::Clear | QItemSelectionModel::Select);
    }
}

void CompletionController::confirm()
{
    QModelIndex index = selectionModel()->currentIndex();
    QString t(model()->data(index, 0).toString());
    {   auto e = m_dv->edit();

        m_dv->moveCursorTo(document()->plus(m_begin, replacementBegin()));
        m_dv->moveCursorTo(m_end, DocumentModel::MoveFlag_Select);
        m_dv->removeText(e, DocumentModel::Remove_Selected);
        m_dv->insertText(e, t);
    }
}

void CompletionController::update(const DocumentPosition &begin, const DocumentPosition &end, int shift)
{
    if (begin > m_end) {
        CompletionController::end();
        return;
    }

    if (end < m_begin) {
        // Happened before
        m_begin = document()->plus(m_begin, shift);
        m_end = document()->plus(m_end, shift);
    } else if (begin >= m_begin) {
        // Happened inside
        m_end = document()->plus(m_end, shift);
    }

    if (shift > 0) {
        m_cursor = document()->blockPositionToOffset(end);
    } else if (shift < 0) {
        m_cursor = document()->blockPositionToOffset(begin);
    }

    update();
}

void CompletionController::update()
{
    if (m_end < m_begin) {
        end();
    } else if (m_begin != m_end){
        m_text = m_dv->text(m_begin, document()->minus(m_end, m_begin));

        completeEvent(m_text, m_begin, m_end, m_cursor - document()->blockPositionToOffset(m_begin));
        emit updated();
    }
}


static const int TEXT_PADDING_LEFT = 16;
static const int TEXT_PADDING_RIGHT = 16;

CompletionWindow::CompletionWindow(QWidget *parent) :
    QWidget(parent),
    m_model(NULL)
{
    setAttribute(Qt::WA_ShowWithoutActivating);
    // setWindowFlags(Qt::Popup | Qt::WindowDoesNotAcceptFocus);
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint/* | Qt::WindowStaysOnTopHint*/);

    setFocusPolicy(Qt::NoFocus);

    m_item_delegate = new CompletionItemDelegate(this);

    m_list = new QListView(this);
    m_list->setFocusPolicy(Qt::NoFocus);
    m_list->setItemDelegate(m_item_delegate);
    // m_list->setFont(Configuration::editorFont());
//    QFont editor_font = Configuration::editorFont();
//    m_list->setStyleSheet(QString("QTableView { font-family: %1; font-size: %2pt; }").arg(editor_font.family()).arg(editor_font.pointSize()));
    // m_table->setFont();
    // m_list->setShowGrid(false);
    m_list->setFrameStyle(QFrame::NoFrame);
    // m_list->horizontalHeader()->hide();
    // m_list->verticalHeader()->hide();
    // m_list->setUniformItemSizes(true);
    m_list->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
//    m_table->setResizeMode(QListView::Adjust);
//    m_table->setFocusPolicy(Qt::NoFocus);
    auto layout = new QVBoxLayout(this);
    setLayout(layout);
    layout->setSpacing(0);
    layout->setMargin(0);
    layout->addWidget(m_list);

    m_list->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_list->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // resize(700, 50);
}

void CompletionWindow::setCompletionRect(const QRect &rect)
{
    m_completion_rect = rect;
    // TODO: Move the completion window
    move(rect.bottomLeft() + QPoint(-TEXT_PADDING_LEFT, 0));
}

bool CompletionWindow::event(QEvent *event)
{
    if (Frameless::event(this, event)) {
        return true;
    }
    return QWidget::event(event);
}

bool CompletionWindow::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
    return Frameless::nativeEvent(this, eventType, message, result);
}

void CompletionWindow::setModel(QAbstractItemModel *model, QItemSelectionModel *selection)
{
    if (m_model) {
        disconnect(m_model, 0, this, 0);
    }
    auto s = m_list->selectionModel();
    m_list->setModel(model);
    CompletionWindow::model = model;
    CompletionWindow::selection = selection;
    if (model) {
        modelReset();
        connect(model, SIGNAL(modelReset()), this, SLOT(modelReset()));
        m_list->setSelectionModel(selection);

        if (s && s->parent() == NULL) {
            delete s;
        }
    }
}


void CompletionWindow::setFilter(const QString &filter)
{
    m_item_delegate->setFilter(filter);
}

void CompletionWindow::modelReset()
{
    // m_list->resizeColumnsToContents();
    // m_list->resizeRowsToContents();
    // ;
    if (!m_list->selectionModel()->currentIndex().isValid()) {
        m_list->selectionModel()->setCurrentIndex(m_list->model()->index(0, 0), QItemSelectionModel::Clear | QItemSelectionModel::Select);
    }

    int row_height = QFontMetrics(m_list->font()).lineSpacing();
    m_list->setMinimumWidth(qMin(600, m_list->sizeHintForColumn(0)));
    m_list->setMaximumHeight(row_height * 5);
    adjustSize();
}

//
//
//

CompletionItemDelegate::CompletionItemDelegate(QObject *parent) :
    QAbstractItemDelegate(parent)
{
}

void CompletionItemDelegate::setFilter(const QString &filter)
{
    m_filter = filter;
}

void CompletionItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (option.state & QStyle::State_Selected) {
        painter->fillRect(option.rect, option.palette.highlight());
    }

    int text_width = option.rect.width() - TEXT_PADDING_LEFT - TEXT_PADDING_RIGHT;
    QTextLayout layout;
    layout.setFont(option.font); // Configuration::editorFont());

    QFontMetrics font_metrics(layout.font());
    QString text(index.data().toString());

    QString filter(m_filter);

//    text = "inua/sandbox/luarocks-install-test/.install/luarocks-2.2.2-win32/src/luarocks/doc.lua";
//    text_width = 568;
//    filter = "doc.lua";

    QString elided(font_metrics.elidedText(text, Qt::ElideMiddle, text_width));


    int elide_begin = -1;
    int elide_end = -1;
    int elide_difference = 0;

    if (text.length() != elided.length()) {
        for (int i = 0; i < text.length(); i++) {
            if (text[i] != elided[i]) {
                Q_ASSERT(elided[i].unicode() == 8230);
                elide_begin = i;
                break;
            }
        }
        // We're counting with elided containing one ellipsis character.
        elide_difference = text.length() - (elided.length() - 1);
        elide_end = elide_begin + elide_difference;
        // Add the elipsis character to the difference.
        elide_difference -= 1;

        // qDebug() << __FUNCTION__ << text.mid(elide_begin - 1, elide_difference + 2);
    }

    layout.setText(elided);

    QColor text_color = Configuration::theme()->foregroundDimColor();
    QColor match_color = Configuration::theme()->foregroundBrightColor();
    QList<QTextLayout::FormatRange> formats;
    int d = StringDistance::pathDistance(filter, text, "/", [&](int offset) {
        if (offset >= elide_begin && offset < elide_end) {
            // Offset is inside of the elided portion.
            return;
        } else if (offset >= elide_end) {
            // Offset is after the elided portion.
            offset -= elide_difference;
        }
        QTextLayout::FormatRange format;
        format.start = offset;
        format.length = 1;
        format.format.setForeground(match_color);
        formats.append(format);
    });
    if (d != StringDistance::NO_MATCH) {
        layout.setAdditionalFormats(formats);
    }

    // qDebug() << __FUNCTION__ << elided << d;

    QTextOption text_option;
    text_option.setWrapMode(QTextOption::NoWrap);
    layout.setTextOption(text_option);
    layout.setCacheEnabled(true);
    layout.beginLayout();
    QTextLine line = layout.createLine();
    line.setLineWidth(text_width);
    layout.endLayout();

    // painter->fillRect(option.rect, option.palette.background());
    painter->setPen(text_color);
    layout.draw(painter, option.rect.topLeft() + QPoint(TEXT_PADDING_LEFT, 0));
}

QSize CompletionItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);
    QFontMetrics metrics(option.font);
    return QSize(metrics.width(index.data().toString()) + TEXT_PADDING_LEFT + TEXT_PADDING_RIGHT, metrics.lineSpacing());
}

//
// CompletionSelection
//

CompletionSelection::CompletionSelection(QAbstractItemModel *model, QObject *parent) :
    QItemSelectionModel(model, parent)
{

}

bool CompletionSelection::selectNextIndex()
{
    int row = 0;
    auto index = currentIndex();
    if (index.isValid()) {
        row = index.row();
    }

    if (row == model()->rowCount() - 1) {
        index = model()->index(0, 0);
    } else {
        index = model()->index(row + 1, 0);
    }
    setCurrentIndex(index, QItemSelectionModel::Clear | QItemSelectionModel::Select);
    return true;
}

bool CompletionSelection::selectPreviousIndex()
{
    int row = 0;
    auto index = currentIndex();
    if (index.isValid()) {
        row = index.row();
    }

    if (row == 0) {
        index = model()->index(model()->rowCount() - 1, 0);
    } else {
        index = model()->index(row - 1, 0);
    }

    setCurrentIndex(index, QItemSelectionModel::Clear | QItemSelectionModel::Select);
    return true;
}

