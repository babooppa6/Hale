#ifndef HALE_UNDO_H
#define HALE_UNDO_H

#if HALE_INCLUDES
#include "hale_types.h"
#endif

namespace hale {

struct Undo
{
    enum EventType {
        EventType_Break = -1,
    };

    Memory<u8> data;
    memi head_read;
    memi head_write;
    memi _read;
    memi _write;
};

void  undo_write_begin(Undo *undo, s32 type);
void  undo_write(Undo *undo, void *data, memi size);
void *undo_write(Undo *undo, memi size);
void  undo_write_end(Undo *undo);

struct UndoWriter
{
    Undo *undo;

    UndoWriter(Undo *undo, s32 type) : undo(undo)
    { undo_write_begin(undo, type); }

    ~UndoWriter()
    { undo_write_end(undo); }
};

b32   undo_next_event(Undo *undo, s32 *type);
void *undo_read(Undo *undo, memi size);
void  undo_read(Undo *undo, void *ptr, memi size);


} // namespace hale

#endif // HALE_UNDO_H
