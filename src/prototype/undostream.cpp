#include "document.h"
#include "undostream.h"

UndoStream::UndoStream(QObject *parent) :
    QObject(parent),
    m_head_read(0),
    m_head_write(0),
    debug(false)
{
    m_buffer_memory = new QByteArray();
    m_buffer = new QBuffer(m_buffer_memory, this);
}

//void UndoStream::read(ReadFunc f)
//{
//    m_read = f;
//}

// TODO: Better copying!
void UndoStream::copyFrom(const UndoStream *other)
{
    m_head_read = other->m_head_read;
    m_head_write = other->m_head_write;
    delete m_buffer_memory;
    delete m_buffer;
    m_buffer_memory = new QByteArray(*other->m_buffer_memory);
    m_buffer = new QBuffer(m_buffer_memory, this);
}

void UndoStream::writeBreak()
{
    write(EventType_Break, [&] (QDataStream &) {});
}

void UndoStream::write(int event_type, std::function<void (QDataStream &)> f)
{
    if (debug) {
        qDebug() << __FUNCTION__ << "read" << m_head_read;
    }
    m_buffer->open(QIODevice::WriteOnly);
    m_buffer->seek(m_head_write);


    QDataStream s(m_buffer);
    writeHeader(s, event_type);
    f(s);

    m_head_read = m_head_write;
    m_head_write = m_buffer->pos();
    m_buffer->close();
}

void UndoStream::undo(ReadFunc f)
{
    m_buffer->open(QIODevice::ReadOnly);
    QDataStream s(m_buffer);
    do {
        m_buffer->seek(m_head_read);
        m_head_write = m_head_read;
        if (readEvent(f, s, &m_head_read) == false) {
            break;
        }
    } while (m_head_write > 0);
    m_buffer->close();
}

void UndoStream::redo(ReadFunc f)
{
    // head_write always points to a break here, so we need to skip the first break
    // and read everything after it until another break or end of stream.

    // We will keep the m_head_read and m_head_write at semi-sync through the readEvent() calls.

    qint64 head;
    m_buffer->open(QIODevice::ReadOnly);
    m_buffer->seek(m_head_write);
    QDataStream s(m_buffer);
    if (!s.atEnd())
    {
        m_head_read = m_head_write;
        // This should be the break.
        bool is_break = readEvent(f, s, &head) == false;
        Q_ASSERT(is_break);

        // Now read everything until next break or eos
        do {
            if (readEvent(f, s, &m_head_read) == false) {
                // Break hit.
                break;
            }
            m_head_write = m_buffer->pos();
        } while (!s.atEnd());
    }
    m_buffer->close();
}

void UndoStream::dump(DumpFunc f)
{
    qDebug() << __FUNCTION__ << "read" << m_head_read << "write" << m_head_write;
    m_buffer->open(QIODevice::ReadOnly);
    QDataStream s(m_buffer);
    QString o;
    QTextStream output(&o);
    while (dumpEvent(f, s, output)) {}
    m_buffer->close();
    qDebug().noquote() << o;
}

bool UndoStream::dumpEvent(DumpFunc f, QDataStream &s, QTextStream &output)
{
    qint64 head;
    qint64 pos = m_buffer->pos();
    int event;

    s >> head;
    s >> event;


    if (pos == m_head_read) {
        if (pos == m_head_write) {
            output << "RW> ";
        } else {
            output << "R>  ";
        }
    } else if (pos == m_head_write) {
        output << "W>  ";
    } else {
        output << "    ";
    }
    output << qSetFieldWidth(5);
    output << pos << " H=" << head << " E=" << event << ' ' << qSetFieldWidth(0);
    if (event != EventType_Break) {
        f(event, s, output);
    } else {
        output << "--------------------------";
    }
    output << '\n';

    return !s.atEnd();
}

bool UndoStream::readEvent(ReadFunc f, QDataStream &s, qint64 *head)
{
    int event_type;

    // Read the head of the previous chunk.
    s >> *head;
    // Read the event type.
    s >> event_type;
    // If it is a break point, then we will stop reading.
    if (event_type != EventType_Break) {
        // Read otherwise.
        f(event_type, s);
        return true;
    }
    return false;
}

void UndoStream::writeHeader(QDataStream &s, int event_type)
{
    /// Write head position to the previous chunk.
    s << m_head_read;
    /// Write event type.
    s << event_type;
}
