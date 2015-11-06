#ifdef HALE_INCLUDES
#include "hale_memory.h"
#include "hale_undo.h"
#endif

namespace hale {

void undo_write_begin(Undo *undo, s32 event_type)
{
    undo->head_read = undo->head_write;

    // We keep the writing position in _write for `undo_write()`.
    undo->_write = undo->head_write;
    // Write head position to the previous chunk.
    undo_write(undo, &undo->head_read, sizeof(memi));
    // Write event type.
    undo_write(undo, &event_type, sizeof(s32));
}

void
undo_write(Undo *undo, void *data, memi size)
{
    u8 *ptr = undo->data.push(size, 4096);
    memory_copy(ptr, (u8*)data, size);
    undo->_write += size;
}

void *
undo_write(Undo *undo, memi size)
{
    void *ret = undo->data.insert(undo->_write, size, 4096);
    undo->_write += size;
    return ret;
}

void undo_write_end(Undo *undo)
{
    // Update the writing head.
    undo->head_write = undo->_write;
}

//
//
//

b32
undo_next_event(Undo *undo, s32 *type)
{
    if (undo->head_write != 0)
    {
        undo->_read = undo->head_read;
        undo->head_write = undo->head_read;
        undo_read(undo, &undo->head_read, sizeof(memi));
        undo_read(undo, type, sizeof(s32));
        return *type != Undo::EventType_Break;
    }
    return 0;
}

void *
undo_read(Undo *undo, memi size)
{
    void *ret = undo->data.ptr + undo->_read;
    undo->_read += size;
    return ret;
}

void
undo_read(Undo *undo, void *ptr, memi size)
{
    // TODO: Assert we're not at the end of current event.
    void *ret = undo->data.ptr + undo->_read;
    memory_copy((u8*)ptr, (u8*)ret, size);
    undo->_read += size;
}

} // namespace hale
