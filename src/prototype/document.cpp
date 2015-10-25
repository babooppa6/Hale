#include "precompiled.h"

#include "configuration.h"

#include "parser.h"
#include "theme.h"
// #include "grammar.h"

#include "util.h"

#include "undostream.h"

#include "documentmodel.h"
#include "document.h"

//
//
//

Document::Document(QObject *parent) :
    QObject(parent),
    m_id(QUuid::createUuid()),
    m_parser(this),
    m_buffer(1024),
    m_active(false)
    // m_edit(NULL)
{
    indentation_size = 4;
    indentation_mode = IndentationMode::Spaces;

    m_blocks_storage.append(NULL);
    m_undo = new UndoStream(this);
    // m_undo->debug = true;

    // using std::placeholders::_1;
    // m_undo->read(std::bind(&Document::readUndo, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));
}

void Document::addModel(DocumentModel *model)
{
    Q_ASSERT(!models.contains(model));
    models.append(model);
    emit modelAdded(model);
}

void Document::removeModel(DocumentModel *model)
{
    Q_ASSERT(models.contains(model));
    models.removeOne(model);
    emit modelRemoved(model);
}

void Document::setID(QUuid id)
{
    m_id = id;
}

void Document::setPath(const QString &path)
{\
    if (m_file_path != path) {
        m_file_path = path;

        emit pathChanged(m_file_path);
    }
}

void Document::setIndentationMode(IndentationMode indentation_mode)
{
    if (indentation_mode != Document::indentation_mode) {
        Document::indentation_mode = indentation_mode;
        emit indentationModeChanged();
    }
}

void Document::setIndentationSize(int indentation_size)
{
    if (indentation_size < 1 || indentation_size > MAX_INDENTATION_SIZE) {
        qWarning() << __FUNCTION__ << "Indentation size should be in range 1 .." << MAX_INDENTATION_SIZE;
        return;
    }

    if (indentation_size != Document::indentation_size) {
        Document::indentation_size = indentation_size;
        emit indentationSizeChanged();
    }
}

void Document::setActiveHint(bool active)
{
    if (active != m_active)
    {
        m_active = active;
        if (m_active) {
            m_parser.resume();
        } else {
            m_parser.suspend();
        }
    }
}

QString Document::grammarName()
{
    return m_grammar_name;
}

void Document::setGrammarName(const QString &grammar)
{
    if (grammar != m_grammar_name) {
        m_grammar_name = grammar;
        emit grammarNameChanged(grammar);
    }
}

void Document::setGrammar(QSharedPointer<Grammar> grammar)
{
    m_parser.setGrammar(grammar);
    emit grammarChanged(grammar);
}

void Document::setText(DocumentEdit edit, const QString &text)
{
    DocumentPosition pos;
    removeText(edit, pos, length());
    insertText(edit, pos, text);

    m_parser.invalidate();
}

DocumentPosition Document::offsetToBlockPosition(int offset)
{
    Q_ASSERT(checkOffset(offset));
    DocumentPosition position;
    position.block = blockAt(offset);
    position.position = offset - blockBeginOffset(position.block);
    return position;
}

//DocumentPosition Document::offsetToBlockPositionVirtual(int offset)
//{
//    DocumentPosition position;
//    if (offset >= blockCount()) {
//        // When offset is beyond the document, then treat it as if the last block.
//        position.block = blockCount() - 1;
//    } else {
//        position.block = blockAt(offset);
//    }
//    position.position = offset - blockBeginOffset(position.block);
//    return position;
//}

int Document::blockPositionToOffset(const DocumentPosition &position)
{
    Q_ASSERT(checkBlockPosition(&position));
    int offset = blockBeginOffset(position.block) + position.position;
    return offset;
}

//int Document::blockPositionToOffsetVirtual(const DocumentPosition &position)
//{

//}

int Document::minus(const DocumentPosition &a, const DocumentPosition &b)
{
    if (a.block == b.block) {
        return a.position - b.position;
    }
    return blockPositionToOffset(a) - blockPositionToOffset(b);
}

DocumentPosition Document::plus(const DocumentPosition &a, int n)
{
    // I'm doing this instead of calling blockPositionToOffset, as the a is allowed to be virtual at this point.
    // Calling offsetToBlockPosition will do the validation afterwards.
    int o = blockBeginOffset(a.block) + a.position + n;
    // int o = blockPositionToOffset(a) + n;
    return offsetToBlockPosition(o);
}

DocumentRange Document::range(int begin, int end)
{
    return DocumentRange(offsetToBlockPosition(begin), offsetToBlockPosition(end));
}

bool Document::checkOffset(int offset)
{
    if (offset < 0) {
        qWarning() << __FUNCTION__ << "Offset" << offset << "< 0";
        return false;
    } else if (offset > length()) {
        qWarning() << __FUNCTION__ << "Offset" << offset << ">" << length();
        return false;
    }
    return true; // offset >= 0 && offset <= length();
}

bool Document::checkBlockPosition(const DocumentPosition *position)
{
    if (position == NULL) {
        return false;
    }
    if (position->block >= 0 && position->block < blockCount()) {
        int block_length = blockLengthWithoutLE(position->block);
        return position->position >= 0 && position->position <= block_length;
    }
    return false;
}

DocumentPosition Document::insertText(DocumentEdit edit, const DocumentPosition &position, const QString &text)
{
    checkEdit(edit);

    // TODO(cohen) Protect insertText in release.

    // TODO(cohen) If a new line is inserted at the beginning of a line,
    // that line will remain untouched (by changed or added callbacks).
    // We need to take care to not let line changes affect breakpoints, line data, etc.
    // That will also make it easier to reuse data in editor.

    Q_ASSERT(checkBlockPosition(&position));

    emit beforeTextChanged();

    DocumentEditEvent event;
    event.type = DocumentEditEvent::Insert;
    event.undo = edit->isUndo();
    event.undo_head = m_undo->writingHead();
    event.sender = edit->m_view;
    event.offset_begin = blockPositionToOffset(position);

    int old_document_end = length();

    std::vector<int> offsets;
    int text_length = insertAndConvertText(event.offset_begin, text.constData(), (int)text.length(), &offsets);

    event.offset_end = event.offset_begin + text_length;

    if (offsets.size() == 0) {
        event.block_text_change = position.block;
        event.block_list_change_first = position.block;
        event.block_list_change_count = 0;
    } else if (position.position == 0) {
        // INSERT BEFORE

        // - Inserting at == 0, will result in:
        //   - lines added 0-1 (1 line inserted before 0)
        //   - line 1 updated (see TODO below)

        // TODO(cohen): Check if the text actually changed the text of the line.
        // - Inserting a text ending with LE won't cause the change.
        // - Can be checked by whether there has been any text flowing to this line
        //   checking the offset_end being inside the line.
        // - Set block_text_change to -1 if no change has been made to the line.

        if (event.offset_begin == old_document_end) {
            // If we're inserting at the end of the document, it's always the first line that is changed.
            event.block_text_change = position.block;
            event.block_list_change_first = position.block + 1;
            event.block_list_change_count = (int)offsets.size();
        } else {
            // Last line is changed.
            event.block_text_change = position.block + (int)offsets.size();
            event.block_list_change_first = position.block;
            event.block_list_change_count = (int)offsets.size();
        }

    // TODO(cohen): } else if (position.position == blockLengthWithoutLE(position.block)) {
        // New text has been inserted after the line.
    } else {
        // - Inserting at  > 0, will result in:
        //   - lines added 1-1 (1 line inserted before 1 / 1 line inserted after 0)
        //   - line 0 updated

        // First line is changed.
        event.block_text_change = position.block;
        event.block_list_change_first = position.block + 1;
        event.block_list_change_count = (int)offsets.size();

    }

    int block_begin, block_end;
    m_blocks.insert(event.offset_begin, text_length, offsets, &block_begin, &block_end);

    if (event.block_list_change_count > 0) {
        m_blocks_storage.insert(event.block_list_change_first, event.block_list_change_count, NULL);
    }

    writeUndo(edit, UndoEvent_Insert, event.offset_begin, event.offset_end - event.offset_begin);

    event.begin = position;
    // This has to be done after we update the blocks, otherwise it'll return wrong position, when a new block is inserted.
    event.end = plus(event.begin, text_length);

    // TODO(cohen): If the time distance between caret types is small, do not parse immediately.
    m_parser.parseImmediate(block_begin, block_end);

    emit textChanged(&event);

    return offsetToBlockPosition(event.offset_end);
}

DocumentPosition Document::appendText(DocumentEdit edit, const QString &text)
{
    DocumentPosition p;
    p.block = blockCount() - 1;
    p.position = blockLengthWithoutLE(p.block);
    return insertText(edit, p, text);
}

void Document::removeText(DocumentEdit edit, const DocumentPosition &position, int length)
{
    checkEdit(edit);

    emit beforeTextChanged();

    // TODO(cohen) Protect removeText in release.
    Q_ASSERT(length >= 0 && length <= Document::length());
    Q_ASSERT(checkBlockPosition(&position));

    DocumentEditEvent event;
    event.undo = edit->isUndo();
    event.undo_head = m_undo->writingHead();
    event.sender = edit->m_view;
    event.type = DocumentEditEvent::Remove;
    event.begin = position;
    event.end = plus(event.begin, length);
    event.offset_begin = blockPositionToOffset(position);
    event.offset_end = event.offset_begin + length;
    event.block_text_change = position.block;

    Q_ASSERT((event.offset_end) <= Document::length());

    writeUndo(edit, UndoEvent_Remove, event.offset_begin, event.offset_end - event.offset_begin);

    m_buffer.remove(event.offset_begin, length);
    int block_begin, block_end;
    m_blocks.remove(event.offset_begin, length, &block_begin, &block_end);

    if (block_begin == block_end) {
        event.block_list_change_first = position.block;
        event.block_list_change_count = 0;
    } else {
        event.block_list_change_first = position.block + 1;
        event.block_list_change_count = block_end - block_begin;
    }

    if (event.block_list_change_count > 0) {
        // TODO(cohen): Cleanup
        m_blocks_storage.remove(event.block_list_change_first, event.block_list_change_count);
    }

    m_parser.parseImmediate(event.block_text_change, event.block_text_change);

    emit textChanged(&event);
}

QString Document::text(const DocumentPosition &position, int length)
{
    return text(blockPositionToOffset(position), length);
}

QString Document::text(const DocumentPosition &begin, const DocumentPosition &end)
{
    if (begin < end) {
        int length = minus(end, begin);
        return text(begin, length);
    } else if (begin > end) {
        int length = minus(begin, end);
        return text(end, length);
    }
    return QString();
}

QString Document::text(int position, int length)
{
    QString ret;
    if (length == 0) {
        return ret;
    }
    Q_ASSERT(position + length <= Document::length());
    ret.resize(length);
    m_buffer.getText(position, length, ret.data(), false);
    return ret;
}

QString Document::text()
{
    return text(0, length());
}

QChar Document::charAt(int position)
{
    return m_buffer.charAt(position);
}

//
//
//

int Document::length() const
{
    return m_buffer.length();
}

int Document::blockCount()
{
    return m_blocks.count();
}

int Document::blockAt(int position)
{
    return m_blocks.blockAt(position);
}

int Document::blockBeginOffset(int block)
{
    return m_blocks.blockBegin(block);
}

int Document::blockEndOffset(int block)
{
    return m_blocks.blockEnd(block);
}

int Document::blockLength(int block)
{
    return m_blocks.blockLength(block);
}

int Document::blockLengthWithoutLE(int block)
{
    int length = blockLength(block);
    if (block < blockCount() - 1) {
        return length -1;
    }
    return length;
}

namespace {
struct calculate_column_in_chunk_p {
    int indentation_size;
    int result;
};

static bool calculate_column(QChar* it, QChar *it_end, calculate_column_in_chunk_p *p)
{
    while (it != it_end) {
        if (*it == '\t') {
            p->result += p->indentation_size - (p->result % p->indentation_size);
        } else {
            p->result++;
        }
        it++;
    }
    return false;
}

static bool calculate_indentation(QChar *it, QChar *it_end, calculate_column_in_chunk_p *p)
{
    while (it != it_end) {
        switch (it->unicode()) {
        case L'\t':
            p->result += p->indentation_size - (p->result % p->indentation_size);
            break;
        case L' ':
            p->result++;
            break;
        default:
            return true;
        }
        it++;
    }
    return false;
}

static bool has_non_whitespace_characters(QChar *it, QChar *it_end, int *has)
{
    while (it != it_end) {
        if (it->unicode() != L' ' && it->unicode() != L'\t') {
            *has = 1;
            return true;
        }
        it++;
    }
    return false;
}
}

int Document::blockColumn(int block, int position)
{
    int offset = blockBeginOffset(block);

    calculate_column_in_chunk_p p;
    p.indentation_size = indentation_size;
    p.result = 0;
    m_buffer.chunk<calculate_column_in_chunk_p*>(offset, offset + position, &calculate_column, &p);

    return p.result;
}

int Document::blockIndentation(int block)
{
    int offset = blockBeginOffset(block);
    int length = blockLengthWithoutLE(block);

    calculate_column_in_chunk_p p;
    p.indentation_size = indentation_size;
    p.result = 0;
    m_buffer.chunk<calculate_column_in_chunk_p*>(offset, offset + length, &calculate_indentation, &p);

    return p.result;
}

bool Document::blockEmpty(int block)
{
    int offset = blockBeginOffset(block);
    int length = blockLengthWithoutLE(block);

    int has = 0;
    m_buffer.chunk<int*>(offset, offset + length, &has_non_whitespace_characters, &has);

    return has == 0;
}

QString Document::blockText(int block, bool include_line_ending)
{
    return text(m_blocks.blockBegin(block),
                include_line_ending ? blockLength(block) : blockLengthWithoutLE(block));
}

int Document::indentationForBlock(int block)
{
    Q_ASSERT(block >= 0);
    // Find first non-empty block above.
    for (;;) {
        block--;
        if (!blockEmpty(block)) {
            break;
        }
        if (block == -1) {
            break;
        }
    }

    if (block == -1) {
        // No block found.
        return 0;
    }

    return blockIndentation(block);
}

QList<QTextLayout::FormatRange> *Document::blockFormats(int block_index)
{
    return m_parser.blockFormats(block_index);
}

//
//
//

int Document::insertAndConvertText(int offset, const QChar *text, int text_length, std::vector<int> *o_line_offsets)
{
    // Walk through the string and convert the line endings to internal format (LF).
    // We are directly inserting the text to the m_content so we don't have to do more memory allocations here.

    const QChar *it = text;
    const QChar *it_text = it;
    const QChar *end = text + text_length;

    int insert_length = 0;
    int insert_offset = offset;
    LineEnding le;
    for (;;)
    {
        le = LineEnding::findNext<const QChar*>(&it, end);
        insert_offset = m_buffer.insert(insert_offset, it-it_text - le.length(), it_text);
        insert_length += it-it_text - le.length();
        if (le == LineEnding::UNKNOWN) {
            break;
        } else {
            // Insert LF new line.
            insert_offset = m_buffer.insert(insert_offset, 1, reinterpret_cast<const QChar *>(L"\n"));
            o_line_offsets->push_back(insert_offset);
            insert_length += 1;
        }
        it_text = it;
    }
    return insert_length;
}

//
//
//

Document::Block *Document::blockData(int block_index)
{
    Block *block = m_blocks_storage[block_index];
    if (block == NULL)
    {
        block = new Block;
        block->flags = 0;
        m_blocks_storage[block_index] = block;
    }
    return block;
}

//
//
//

namespace {
    static void commit_edit(_DocumentEdit *edit) {
        edit->commit();
    }
}

bool Document::isEditing()
{
    return m_edit.isOpen() || m_edit.m_undo;
}

DocumentModel *Document::editingView()
{
    if (!m_edit.isOpen()) {
        return NULL;
    }
    return m_edit.m_view;
}

DocumentEdit Document::edit(DocumentModel *view)
{
    if (m_edit.isOpen()) {
        // TODO: At this point, the other DocumentEdit might be still available, so
        // we need to invalidate it. Otherwise it'll gain the new changes.
        qFatal("Double edit.");
    }

    if (view) {
        Q_ASSERT(models.contains(view));
    }

    m_edit.m_document = this;
    m_edit.m_undo = false;
    m_edit.m_view = view;
    m_edit.m_internal = view == NULL;
    m_undo->writeBreak();
    emit editBegin(m_edit.m_view, m_edit.m_undo);
    return DocumentEdit(&m_edit, &commit_edit);
}

DocumentEdit Document::editUndo()
{
    if (m_edit.isOpen()) {
        qFatal("Double edit.");
    }

    m_edit.m_document = NULL;
    m_edit.m_view = NULL;
    m_edit.m_internal = false;
    m_edit.m_undo = true;
    return DocumentEdit(&m_edit, &commit_edit);
}

void Document::commit(_DocumentEdit *edit)
{
    Q_ASSERT(edit == &m_edit);
    if (edit == &m_edit && edit->isOpen()) {
        // TODO: Commit undo.
        emit editEnd(m_edit.m_view, m_edit.m_undo);
    }
}

void Document::revert(_DocumentEdit *edit)
{
    Q_ASSERT(edit == &m_edit);
    if (edit == &m_edit && edit->isOpen()) {
        // TODO: Revert edit's undo entries.
        emit editEnd(m_edit.m_view, m_edit.m_undo);
    }
}

void Document::checkEdit(DocumentEdit edit)
{
    if (edit == NULL) {
        qFatal("Edit handle was NULL.");
    } else if (!edit->isOpen() && !edit->m_undo) {
        qFatal("Edit handle is not open.");
    }
}

//
// Undo
//

void Document::undo()
{
    auto e(editUndo());
    m_undo->undo([&] (int event, QDataStream &s) {
        readUndo(e, s, event, false);
    });
    dumpUndo();
    emit undoTriggered();
}

void Document::redo()
{
    auto e(editUndo());
    m_undo->redo([&] (int event, QDataStream &s) {
        readUndo(e, s, event, true);
    });
    dumpUndo();
    emit redoTriggered();
}

void Document::writeUndo(DocumentEdit edit, int type, int offset, int length)
{
    if (edit->isUndo()) {
        return;
    }

    m_undo->write(type, [&] (QDataStream &s) {
        s << offset;
        s << length;
        s << text(offset, length);
    });

    dumpUndo();
}

void Document::readUndo(DocumentEdit edit, QDataStream &s, int event, bool redo)
{
    int offset;
    int length;
    QString text;

    s >> offset;
    s >> length;

    auto position(offsetToBlockPosition(offset));

    if (redo) {
        switch (event) {
        case UndoEvent_Insert:
            event = UndoEvent_Remove;
            break;
        case UndoEvent_Remove:
            event = UndoEvent_Insert;
            break;
        }
    }

    switch (event)
    {
    case UndoEvent_Insert:
        removeText(edit, position, length);
        break;
    case UndoEvent_Remove:
        s >> text;
        insertText(edit, position, text);
        break;
    }
}

void Document::dumpUndo()
{
    return;

    m_undo->dump([&] (int event, QDataStream &s, QTextStream &output) {
       switch (event)
       {
       case UndoEvent_Insert:
           output << "Insert ";
           break;
       case UndoEvent_Remove:
           output << "Remove ";
           break;
       }

       int offset;
       int length;
       QString text;

       s >> offset;
       s >> length;
       s >> text;
       output << offset << ' ' << length << " \"" << text << "\" ";
    });
}

//
//
//

DocumentBlocks::DocumentBlocks()
{
    m_offsets.push_back(0);
}

void DocumentBlocks::insert(int offset, int text_length, const std::vector<int> & offsets, int *o_line_begin, int *o_line_end)
{
    Q_ASSERT(o_line_begin != NULL);
    Q_ASSERT(o_line_end != NULL);

    *o_line_begin = blockAt(offset);
    *o_line_end = (*o_line_begin) + offsets.size();

    // Insert line ends.
    m_offsets.insert(m_offsets.begin() + (*o_line_begin), offsets.begin(), offsets.end());

    // Shift lines.
    Offsets::iterator it = m_offsets.begin() + (*o_line_end);
    Offsets::iterator it_end = m_offsets.end();
    for (; it != it_end; it++) {
        *it += text_length;
    }
}

void DocumentBlocks::remove(int offset, int text_length, int *o_line_begin, int *o_line_end)
{
    Q_ASSERT(o_line_begin != NULL);
    Q_ASSERT(o_line_end != NULL);

    blockRangeAt(offset, offset+text_length, o_line_begin, o_line_end);
    // Erase lines.
    Offsets::iterator it = m_offsets.erase(m_offsets.begin() + *o_line_begin, m_offsets.begin() + *o_line_end);
    // Shift the offsets.
    for (; it != m_offsets.end(); it++) {
        *it -= text_length;
    }
}

int DocumentBlocks::blockBegin(int index) const
{
    if (index < 0) {
        return 0;
    } if (index > count()) {
        return m_offsets.back();
    }
    return !index ? 0 : m_offsets[index - 1];
}

int DocumentBlocks::blockEnd(int index) const
{
    if (index < 0) {
        return 0;
    } else if (index >= count()) {
        return m_offsets.back();
    }
    return m_offsets[index];
}

int DocumentBlocks::blockLength(int index) const
{
    return blockEnd(index) - blockBegin(index);
}

int DocumentBlocks::blockAt(int offset, int search_begin, int search_end) const
{
    if (offset < 0) {
        return 0;
    } else if (offset >= m_offsets[count()-1]) {
        return count() - 1;
    }

    int begin = search_begin;
    int end = search_end < 0 ? count() - 1 : search_end;

    Q_ASSERT(begin <= end);

    for (;;)
    {
        switch (end - begin)
        {
        case 0:
            if (m_offsets[begin] <= offset)
                return begin + 1;
            else
                return begin;
        case 1:
            if (m_offsets[begin] <= offset)
            {
                if(m_offsets[end] <= offset)
                    return end + 1;
                else
                    return end;
            }
            else
                return begin;
        default:
            int pivot = (end + begin) / 2;
            int value = m_offsets[pivot];
            if (value == offset) {
                return pivot + 1;
            } else if (value < offset) {
                begin = pivot + 1;
            } else {
                end = pivot - 1;
            }

            break;
        }
    }
}

void DocumentBlocks::blockRangeAt(int begin, int end, int *o_line_begin, int *o_line_end)
{
    Q_ASSERT(o_line_begin != NULL);
    Q_ASSERT(o_line_end != NULL);

    *o_line_begin = blockAt(begin);
    *o_line_end = blockAt(end, *o_line_begin);
}

//
//
//


DocumentManager::DocumentManager(QObject *parent) :
    QObject(parent)
{
    connect(Configuration::instance, &Configuration::themeChanged, this, &DocumentManager::themeChanged);
}

void DocumentManager::themeChanged(QSharedPointer<Theme> theme)
{
    for (auto descriptor : descriptors) {
        if (descriptor.document) {
            descriptor.document->setTheme(theme);
        }
    }
}

void DocumentManager::attachDocument(Descriptor *descriptor)
{
    Q_ASSERT(descriptor);
    Q_ASSERT(descriptor->document);

    if (descriptor->grammar.isEmpty()) {
        // auto grammar = GrammarManager::instance->findGrammarForDocument(descriptor->document);
//        if (grammar) {
//            descriptor->document->setGrammar(grammar);
//            qDebug() << __FUNCTION__ << "Applying" << grammar->rule->name << "to" << descriptor->document->path();
//        }
    } else {
        descriptor->document->setGrammarName(descriptor->grammar);
    }

    descriptor->document->setTheme(Configuration::theme());

    connect(descriptor->document, &Document::modelRemoved, this, &DocumentManager::documentModelRemoved);
}

void DocumentManager::detachDocument(Descriptor *descriptor)
{
    if (descriptor->document) {
        Q_ASSERT(descriptor->document->parent() == this);
        descriptor->document->deleteLater();
        descriptor->document = NULL;
    }
}

void DocumentManager::documentModelRemoved(DocumentModel *)
{
    auto document = qobject_cast<Document*>(sender());
    if (document->models.empty()) {
        closeDocument(document);
    }
}

DocumentManager::Descriptor *DocumentManager::attachDescriptor(Descriptor *descriptor)
{
    descriptors.append(*descriptor);
    if (descriptor->document) {
        attachDocument(descriptor);
    }
    return &descriptors.back();
}

void DocumentManager::detachDescriptor(Descriptor *descriptor)
{
    detachDocument(descriptor);

    Descriptors::iterator it = descriptors.begin();
    while (it != descriptors.end()) {
        if (&(*it) == descriptor) {
            descriptors.erase(it);
            break;
        }
        it++;
    }
}

void DocumentManager::loadState()
{
    descriptors.clear();
    QDir dir(storageDir());
    QDir::Filters filters = QDir::Files;
    QFileInfoList files = dir.entryInfoList(filters);
    for (auto file : files) {
        if (file.completeSuffix() != "json") {
            continue;
        }
        QUuid id(file.baseName());
        if (id.isNull()) {
            continue;
        }
        loadDescriptor(id);
    }
}

void DocumentManager::saveState()
{
    for (auto &descriptor : descriptors) {
        if (descriptor.document)  {
            saveDescriptor(&descriptor);
        }
    }
}

DocumentManager::Descriptor *DocumentManager::getDescriptor(const QString &path)
{
    QString absolute_path(QFileInfo(path).absoluteFilePath());
    for (auto &descriptor : descriptors) {
        if (descriptor.source_path == absolute_path) {
            return &descriptor;
        }
    }
    return NULL;
}

DocumentManager::Descriptor *DocumentManager::getDescriptor(const QUuid &id)
{
    for (auto &descriptor : descriptors) {
        if (descriptor.id == id) {
            return &descriptor;
        }
    }
    return NULL;
}

DocumentManager::Descriptor *DocumentManager::getDescriptor(const Document *document)
{
    for (auto &descriptor : descriptors) {
        if (descriptor.document == document) {
            return &descriptor;
        }
    }
    return NULL;
}

Document *DocumentManager::document(const QUuid &id)
{
    auto descriptor = getDescriptor(id);
    if (descriptor) {
        return descriptor->document;
    }
    return NULL;
}

Document *DocumentManager::document(const QString &path)
{
    auto descriptor = getDescriptor(path);
    if (descriptor) {
        return descriptor->document;
    }
    return NULL;
}


Document *DocumentManager::newDocument()
{
    auto descriptor = newDescriptor(new Document(this));
    emit documentCreated(descriptor->document);
    return descriptor->document;
}

Document *DocumentManager::loadDocument(const QString &path)
{
    auto descriptor = getDescriptor(path);
    if (descriptor != NULL) {
        if (descriptor->document) {
            return descriptor->document;
        }
        return loadDocument(descriptor);
    }

    auto document = loadDocumentInternal(path, NULL);
    if (document) {
        descriptor = newDescriptor(document);
        emit documentLoaded(descriptor->document);
        return descriptor->document;
    }

    return NULL;
}

Document *DocumentManager::loadDocument(const QUuid &id)
{
    auto descriptor = getDescriptor(id);
    if (descriptor != NULL) {
        if (descriptor->document) {
            return descriptor->document;
        }
        return loadDocument(descriptor);
    }
    return NULL;
}

Document *DocumentManager::loadDocument(Descriptor *descriptor)
{
    if (descriptor == NULL) {
        return NULL;
    }

    if (descriptor->source_path.isEmpty()) {
        return NULL;
    }

    QFileInfo original_file(descriptor->source_path);

    qDebug() << __FUNCTION__
             << "Original" << original_file.lastModified().toUTC()
             << "Storage" << descriptor->source_modified;

    bool storage_is_older = !descriptor->source_modified.isValid() || original_file.lastModified() > descriptor->source_modified;
    if (original_file.exists() && storage_is_older) {
        // Load document from original file.
        descriptor->document = loadDocumentInternal(descriptor->source_path, descriptor);
        descriptor->source_modified = original_file.lastModified();
    } else {
        // Load document from storage.
        descriptor->document = loadDocumentInternal(descriptorStoragePath(descriptor, ".document"), descriptor);
    }

    if (descriptor->document) {
        if (descriptor->document) {
            descriptor->document->setPath(original_file.absoluteFilePath());
        }
        attachDocument(descriptor);

        emit documentLoaded(descriptor->document);
    }
    return descriptor->document;
}

Document *DocumentManager::loadDocumentInternal(const QString &path, Descriptor *descriptor)
{
    QFileInfo file_info(path);
    QString string;
    if (Util::loadStringFromFile(&string, file_info.absoluteFilePath())) {
        Document *document = new Document(this);
        if (descriptor) {
            Q_ASSERT(!descriptor->id.isNull());
            document->setPath(descriptor->source_path);
            document->setID(descriptor->id);
        } else {
            document->setPath(file_info.absoluteFilePath());
            document->setID(QUuid::createUuid());
        }

        {   DocumentEdit edit = document->edit(NULL);
            document->setText(edit, string);
        }

        return document;
    }
    return NULL;
}

bool DocumentManager::saveDocument(Document *document)
{
    if (!document->path().isEmpty()) {
        return saveDocument(document, document->path());
    }
    return false;
}

bool DocumentManager::saveDocument(Document *document, const QString &path)
{
    auto descriptor = getDescriptor(document);
    Q_ASSERT(descriptor);

    QFileInfo info(path);
    QString absolute_path = info.absoluteFilePath();
    if (saveDocumentInternal(document, absolute_path)) {
        QFileInfo storage_info(descriptorStoragePath(descriptor, ".document"));
        if (storage_info.isFile() && storage_info.absoluteFilePath() != absolute_path) {
            // Remove the storage as it's now invalid.
            QFile::remove(storage_info.absoluteFilePath());
        }
        descriptor->source_path = absolute_path;
        descriptor->source_modified = info.lastModified();
        document->setPath(absolute_path);
        emit documentSaved(document);
        return true;
    }
    return false;
}

void DocumentManager::closeDocument(Document *document)
{
    auto descriptor = getDescriptor(document);
    Q_ASSERT(descriptor);

    // Force save the descriptor.
    saveDescriptor(descriptor);

    // Detach the document.
    detachDocument(descriptor);
}

bool DocumentManager::saveDocumentInternal(Document *document, const QString &path)
{
    if (Util::saveStringToFile(document->text(), path)) {
        return true;
    }
    return false;
}

DocumentManager::Descriptor *DocumentManager::newDescriptor(Document *document)
{
    Descriptor descriptor;
    descriptor.id = document->id();
    descriptor.document = document;
    descriptor.source_path = document->path();
    if (!document->path().isEmpty()) {
        descriptor.source_modified = QFileInfo(document->path()).lastModified();
    }

    return attachDescriptor(&descriptor);
}

bool DocumentManager::saveDescriptor(const Descriptor *descriptor)
{
    QString path(descriptorStoragePath(descriptor, ""));
    if (!saveDocumentInternal(descriptor->document, path + ".document")) {
        qWarning() << __FUNCTION__ << "Unable to save backup to" << path + ".document";
    }

    QJsonObject j;
    j["id"] = descriptor->id.toString();
    j["path"] = descriptor->source_path;
    if (descriptor->document) {
        j["grammar"] = descriptor->document->grammarName();
    }
    j["modified"] = Util::fromDateTime(descriptor->source_modified);

    if (!Util::saveStringToFile(QJsonDocument(j).toJson(), path + ".json")) {
        qWarning() << __FUNCTION__ << "Unable to save state to" << path + ".json";
        return false;
    }
    return true;
}

DocumentManager::Descriptor *DocumentManager::loadDescriptor(const QUuid &id)
{
    QString string;
    if (!Util::loadStringFromFile(&string, descriptorStoragePath(id, ".json"))) {
        return NULL;
    }
    QJsonDocument json(QJsonDocument::fromJson(string.toUtf8()));
    QJsonObject object(json.object());
    if (object.empty()) {
        return NULL;
    }

    Descriptor descriptor;
    descriptor.id = id;
    descriptor.grammar = object["grammar"].toString();
    descriptor.source_path = object["path"].toString();
    descriptor.source_modified = Util::toDateTime(object["modified"]);
    descriptor.document = NULL;

    return attachDescriptor(&descriptor);
}

QString DocumentManager::descriptorStoragePath(const QUuid &id, const char *ext)
{
    return storageDir().filePath(id.toString() + ext);
}

QString DocumentManager::descriptorStoragePath(const Descriptor *descriptor, const char *ext)
{
    return descriptorStoragePath(descriptor->id, ext);
}

QDir DocumentManager::storageDir()
{
    return QDir(storage_path);
}
