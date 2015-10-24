#ifndef HALE_FIXED_GAP_BUFFER_H
#define HALE_FIXED_GAP_BUFFER_H

#include "hale.h"
#include "hale_test.h"

namespace hale {

// TODO: Allow changing buffers to arbitrary values (power of 2).

#define buf_capacity          (platform.page_size)
#define buf_align_mask        (platform.page_mask)
#define buf_align_shift       (platform.page_shift)

// Returns size of the gap.
#define buf_gap(buffer)\
        (memi)(((buffer)->gap_end - (buffer)->gap_start))

// Amount of available space in the buffer
#define buf_available(buffer)\
        buf_gap(buffer)

// TODO: Define this without bounds to capacity.
// Returns size of data in the buffer.
#define buf_length(buffer)\
        (buf_capacity - buf_gap(buffer))

// Returns size of data before the gap.
#define buf_left_length(buffer)\
        ((buffer)->gap_start >> buf_align_shift)

// Returns size of data after the gap.
#define buf_right_length(buffer)\
        (buf_length(buffer) - ((buffer)->gap_end >> buf_align_shift))

// Returns pointer to the beginning of the buffer page.
#define buf_page_begin(buffer)\
        (ch8*)((memi)(buffer)->gap_start & ~buf_align_mask)

// Returns pointer to the end of the buffer page.
#define buf_page_end(buffer)\
        (ch8*)(buf_page_begin(buffer) + buf_capacity)

// Returns pointer to the end of the buffer data (including gap).
#define buf_data_end(buffer)\
        (ch8*)(buf_page_begin(buffer) + buf_length(buffer) + buf_gap(buffer))

// Returns index of the buffer in the arena
#define buf_index(arena, buffer)\
        ((buffer) - &(arena)->buffers[0])

struct Buf
{
    ch8 *gap_start;
    ch8 *gap_end;
#if HALE_DEBUG
    memi debug_length;
    ch8 *debug_page;
#endif
};

void buf_allocate(Buf *buffer);
void buf_deallocate(Buf *buffer);

void buf_clear(Buf *buffer);
void buf_set(Buf *buffer, ch8 *begin, ch8 *end);
void buf_insert(Buf *buffer, memi offset, const ch8 *data, memi size);
memi buf_remove(Buf *buffer, memi offset, memi size);
ch8 *buf_text(Buf *buffer, memi offset, memi size, ch8 *out);
void buf_dump(Buf *buffer, Vector<ch8> *string);

// Moves all data after offset from buffer to start of the other buffer.
void buf_move_suffix(Buf *buffer, memi offset, Buf *other);

struct FixedGapArena
{
    Vector<Buf> buffers;
    memi size;

#ifdef HALE_TEST
    TestCoverage coverage;
    memi stat_allocations_count;
    memi stat_allocations_bytes;
#endif
};

void fixed_gap_arena_init(FixedGapArena *arena, memi count);
void fixed_gap_arena_release(FixedGapArena *arena);

void fixed_gap_arena_insert(FixedGapArena *arena,
                            memi insert_offset,
                            const ch8 *insert_data,
                            memi insert_size);

void fixed_gap_arena_remove(FixedGapArena *arena,
                            memi remove_offset,
                            memi remove_size);

ch8 *fixed_gap_arena_text(FixedGapArena *arena,
                          memi offset,
                          memi size,
                          ch8 *buffer);

void buf_dump(Buf *buffer,
                              Vector<ch8> *string);

} // namespace hale

#endif // HALE_FIXED_GAP_BUFFER_H
