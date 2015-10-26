#if HALE_INCLUDES
#include "hale_fixed_gap_buffer.h"
#include "hale_print.h"
#endif

namespace hale {

#define BUF_DUMP(buffer)\
{   Vector<ch8> dump;\
    vector_init(&dump);\
    buf_dump(buffer, &dump);\
    vector_push(&dump, '\n');\
    print(&dump);\
    vector_release(&dump);\
    }

#if HALE_REQUIREMENT
hale_internal void
_requirement_check_buf(Buf *buffer)
{
    hale_assert_debug(buffer->gap_end >= buffer->gap_start);
    memi buf_length_value = buf_length(buffer);
    hale_assert_debug(buffer->debug_length == buf_length_value);
    hale_assert_debug(buffer->debug_page == buf_page_begin(buffer));
    // End is allowed to be either at end (on the next page) or within the debug_page.
    hale_assert_debug(((memi)buffer->gap_end & ~buf_align_mask) == (memi)buffer->debug_page ||
                      buffer->gap_end == (buffer->debug_page + buf_capacity));
}
#define requirement_check_buf _requirement_check_buf
#else
#define requirement_check_buf
#endif

inline memi
buf_move(Buf *buffer,
         ch8 *src_begin, ch8 *src_end,
         ch8 *dst_begin)
{
#ifdef HALE_DEBUG
    hale_assert_debug(buffer);
    hale_assert_debug(src_begin);
    hale_assert_debug(src_end);
    hale_assert_debug(dst_begin);

    hale_assert_debug(src_begin <= src_end);

    ch8 *dst_end = dst_begin + (src_end - src_begin);

    hale_assert_debug(dst_begin <= dst_end);

    ch8 *page_begin = buf_page_begin(buffer);
    ch8 *page_end = buf_page_end(buffer);
    hale_assert_debug(dst_begin >= page_begin);
    hale_assert_debug(dst_begin <  page_end);
    hale_assert_debug(dst_end   >= page_begin);
    hale_assert_debug(dst_end   <= page_end);

    hale_assert_debug(src_begin >= page_begin);
    hale_assert_debug(src_begin <  page_end);
    hale_assert_debug(src_end   >= page_begin);
    hale_assert_debug(src_end   <= page_end);

    hale_assert_debug((dst_end - dst_begin) == (src_end - src_begin));
#endif

    platform.move_memory(dst_begin, src_begin, src_end - src_begin);

#ifdef HALE_TEST
//    buffer->statistics.mem_move_size += length * sizeof(ch);
//    buffer->statistics.mem_move_count ++;
#endif

    return src_end - src_begin;
}

void buf_allocate(Buf *buffer)
{
    ch8 *memory = (ch8*)platform.allocate_paged_memory(buf_capacity);
    buffer->gap_start = memory;
    buffer->gap_end = memory + buf_capacity;
#ifdef HALE_DEBUG
    buffer->debug_page = memory;
    buffer->debug_length = 0;
#endif
}

void buf_deallocate(Buf *buffer)
{
    platform.deallocate_paged_memory(buf_page_begin(buffer));
}

void buf_clear(Buf *buffer)
{
    requirement_check_buf(buffer);
    buffer->gap_start = buf_page_begin(buffer);
    buffer->gap_end   = buffer->gap_start + buf_capacity;
    hale_debug(buffer->debug_length = 0);
    hale_debug(buffer->debug_length = 0);
    requirement_check_buf(buffer);
}

//void
//buf_set(Buf *buffer, memi begin, memi size)
//{
//    platform.copy_memory(buf_page_begin(buffer), begin, size);
//    if (size == buf_capacity) {
//        buffer->gap_start = buffer->gap_end = buf_page_begin(buffer);
//    } else {
//        buffer->gap_end = buf_page_end(buffer);
//        buffer->gap_start = buf_page_begin(buffer) + size;
//    }
//}

void
buf_insert(Buf *buffer, memi offset, const ch8 *data, memi size)
{
    requirement_check_buf(buffer);
    // Are we making the gap larger? Do we need to?
    hale_assert_requirement(buffer->gap_end != buffer->gap_start);
    hale_assert_requirement(offset < buf_capacity);
    ch8 *at = buf_page_begin(buffer) + offset;

    if (at < buffer->gap_start)
    {

        // .|....#####.....
        //   ^^^^-vvvv
        // .|#####.........
        // forward  [at..gap_start] -> [(gap_end-(gap_start-at)) .. gap_end]
        //                          => [(at + gap) .. gap_end]

        buffer->gap_end -= buf_move(buffer,
                                    at, buffer->gap_start,
                                    at + buf_gap(buffer));

    }
    else if (at > buffer->gap_start)
    {

        // .....#####....|.
        //      vvvv-^^^^
        // .........|#####.
        // backward [gap_end..at]   -> [gap_start .. gap_start+(at-gap_end)]
        //                             [gap_start .. gap_start+at-gap_end]

        buffer->gap_end += buf_move(buffer,
                                    buffer->gap_end, at + buf_gap(buffer),
                                    buffer->gap_start);

    }

    // .|....#####.....
    //   ++++
    // .|++++#.........

    platform.copy_memory((void*)at, (void*)data, size);

    buffer->gap_start = at + size;

    if (((memi)at & ~buf_align_mask) != ((memi)buffer->gap_start & ~buf_align_mask))
    {
        // We're using buffer->gap_start to tell where the buffer's page is at.
        // However when the buffer is filled, the buffer->gap_start might end
        // up at the end of the page, which is actually a pointer to the next
        // page (when masked out).

        // That only happens when the buffer is fully filled. Asserted below.
        hale_assert_requirement(buffer->gap_start == buffer->gap_end);

        // Therefore we're detecting this case and we move the gap to the
        // beginning of the buffer.

        buffer->gap_start = buffer->gap_end = (ch8*)((memi)at & ~buf_align_mask);
    }

#if HALE_DEBUG
    buffer->debug_length += size;
#endif
    requirement_check_buf(buffer);

//    BUF_DUMP(buffer);
}

memi
buf_remove(Buf *buffer, memi offset, memi size)
{
    hale_assert(size <= buf_length(buffer));
    hale_assert(offset <= buf_length(buffer));
    hale_assert((offset + size) <= buf_length(buffer));
    requirement_check_buf(buffer);

    ch8 *begin = buf_page_begin(buffer) + offset;
    ch8 *end   = buf_page_begin(buffer) + offset + size;

    memi s;
    // No gap
    if (buffer->gap_end == buffer->gap_start)
    {
        buffer->gap_start = begin;
        buffer->gap_end = end;
    }
    // Before gap
    else if (begin <= buffer->gap_start && end <= buffer->gap_start)
    {
        // .xx..###########
        //  vv^^
        // ...#############

        // .xxx.#####.....
        //  v--^
        // ..#########.....

        s = buf_move(buffer,
                     end, buffer->gap_start,
                     begin);

        buffer->gap_start = begin + s;
    }
    // Around the gap
    else if (begin <= buffer->gap_start)
    {
        // ..[..]#####.....
        // ..#########.....

        // ..[...#####...]..
        // ..#############..

        // ......[#####...]..
        // ......##########..

        // ......#####[...]..
        // ......##########..

        buffer->gap_end = maximum(end + buf_gap(buffer), buffer->gap_end);
        // Must be done after calling `buf_gap`. Idiot.
        buffer->gap_start = begin;
    }
    // After the gap
    else if(begin > buffer->gap_start && end > buffer->gap_start)
    {
        // ......#####.[..]..
        //       v----^
        // .......#########..
        begin += buf_gap(buffer);
        end   += buf_gap(buffer);

        s = buf_move(buffer,
                     buffer->gap_end, begin,
                     buffer->gap_start);

        buffer->gap_start += s;
        buffer->gap_end = end;
    }

    hale_debug(buffer->debug_length -= size);
    requirement_check_buf(buffer);

    return size;
}

ch8 *
buf_text(Buf *buffer, memi offset, memi size, ch8 *out)
{
    requirement_check_buf(buffer);

    hale_assert_requirement(size <= buf_capacity);
    hale_assert_requirement(size <= buf_length(buffer));
    hale_assert_requirement(offset <= buf_capacity);
    hale_assert_requirement(offset <= buf_length(buffer));
    hale_assert_requirement((offset + size) <= buf_length(buffer));

    ch8 *begin = buf_page_begin(buffer);
    ch8 *ptr = begin + offset;

    if (ptr >= buffer->gap_start)
    {
        //     .....#####..|xxx
        //     vvv----------^^^
        // out xxx

        platform.copy_memory(out, ptr + buf_gap(buffer), size);
        out += size;
    }
    else if ((ptr + size) <= buffer->gap_start)
    {
        //     .|xxx.#####.....
        //     vvI^^
        // out xxx

        platform.copy_memory(out, ptr, size);
        out += size;
    }
    else
    {
        //     ...|xx#####x....
        //     vv--^^
        // out xx

#if HALE_DEBUG
        memi p = size;
#endif
        memi s = buffer->gap_start - ptr;
        platform.copy_memory(out, ptr, s);
        hale_debug(p -= s);

        //     ...|xx#####o....
        //       v--------^
        // out xxo

        out += s;
        platform.copy_memory(out,
                             buffer->gap_end,
                             size - s);
        out += (size-s);
        hale_debug(p -= (size-s));
        hale_assert_debug(p == 0);
    }

    return out;
}

void
buf_move_suffix(Buf *A, memi offset, Buf *B)
{
    requirement_check_buf(A);
    requirement_check_buf(B);

    hale_assert_requirement(buf_length(A) != 0);
    hale_assert_requirement(offset < buf_length(A));

#if HALE_REQUIREMENT
    ch8 *B_page = buf_page_begin(B);
    ch8 *A_page = buf_page_begin(A);
#endif

    ch8 *A_ptr = buf_page_begin(A) + offset;
    // ch8 *B_ptr = buf_page_begin(B);
    B->gap_start = buf_page_begin(B);
    B->gap_end = buf_page_end(B);
    hale_debug(B->debug_length = 0);

    memi s;

    if (A_ptr >= A->gap_start)
    {
        // A .....#####..|xxx
        //        vv---^^
        // A .......#####|xxx

        A_ptr += buf_gap(A);

        s = A_ptr - A->gap_end;

        if (s) {
            platform.move_memory(A->gap_start, A->gap_end, s);
//            A->gap_start += s;
//#if HALE_REQUIREMENT
//        A->gap_end += s;
//#endif
        }


        // A .......#####|xxx
        //   vvv----------^^^
        // B xxx#############

        s = buf_length(A) - offset;
        platform.copy_memory(B->gap_start, A_ptr, s);
        B->gap_start += s;
        hale_debug(B->debug_length += s);
        hale_debug(A->debug_length -= s);
        A->gap_start = A_ptr - buf_gap(A);
        A->gap_end = buf_page_end(A);

//#if HALE_REQUIREMENT
//        A->gap_end += s;
//#else
//        A->gap_end = buf_page_end(A);
//#endif
    }
    else // if(A_ptr < A->gap_start)
    {
        hale_assert_requirement(A_ptr < A->gap_start);

        // A ..ooo#####xxxxx
        //   v-^^^
        // B ooo############

        s = A->gap_start - A_ptr;
        platform.copy_memory(B->gap_start, A_ptr, s);
        B->gap_start += s;
        hale_debug(B->debug_length += s);
        hale_debug(A->debug_length -= s);

        // A ..ooo#####xxxxx
        //      vvvvv--^^^^^
        // B oooxxxxx#######

        s = buf_page_end(A) - A->gap_end;
        if (s) {
            platform.copy_memory(B->gap_start, A->gap_end, s);
            B->gap_start += s;
            hale_debug(B->debug_length += s);
            hale_debug(A->debug_length -= s);
        }

        A->gap_start = A_ptr;
        A->gap_end += s;
    }

    hale_assert_requirement(A->gap_start == buf_page_begin(A) + offset);
    hale_assert_requirement(A->gap_end == buf_page_end(A));
    hale_assert_requirement(((memi)A->gap_start &~ buf_align_mask)
                         == ((memi)A_ptr &~ buf_align_mask));

    // B->gap_end is always at the end.
    if (B->gap_start == B->gap_end) {
        // Special case, when we always reset the buffer to be at the beginning once it's full.
        // In this case we're sure that we will never move
        B->gap_start = B->gap_end = (ch8*)((memi)(B->gap_end - 1) & ~buf_align_mask);
    }

    hale_assert_requirement(buf_page_begin(A) == A_page);
    hale_assert_requirement(buf_page_begin(B) == B_page);

    requirement_check_buf(A);
    requirement_check_buf(B);
}


//
// FixedGapArena
//

hale_internal inline Buf *
allocate_buffers(FixedGapArena *arena, memi at, memi count)
{
    vector_insert(&arena->buffers, at, count);
    Buf *buffer = &arena->buffers[at];
    Buf *buffer_end = buffer + count;
    ch8 *memory = (ch8*)platform.allocate_paged_memory(buf_capacity * count);
    while (buffer != buffer_end)
    {
        buffer->gap_start = memory;
        buffer->gap_end = memory + buf_capacity;
        hale_debug(buffer->debug_page = memory);
        hale_debug(buffer->debug_length = 0);
        requirement_check_buf(buffer);
        ++buffer;
        memory += buf_capacity;
    }
    return buffer - count;
}

void
fixed_gap_arena_init(FixedGapArena *arena, memi count)
{
    // TODO: Buffer pool.
    vector_init(&arena->buffers);
    allocate_buffers(arena, 0, 1);

    arena->size = 0;
}

void fixed_gap_arena_release(FixedGapArena *arena)
{
    for (memi i = 0; i != vector_count(arena->buffers); ++i)
    vector_release(&arena->buffers);
}

#if 0
hale_internal
Buf *
find_buf(memi offset, Buf *begin, Buf *end)
{
    Buf *pivot;

    for (;;)
    {
        switch (end - begin)
        {
        case 0:
            if (begin->offset <= offset)
                return begin + 1;
            else
                return begin;
        case 1:
            if (begin->offset <= offset)
            {
                if(end->offset <= offset)
                    return end + 1;
                else
                    return end;
            }
            else
                return begin;
        default:
            pivot = (end + begin) / 2;
            if (pivot->offset == offset) {
                return pivot + 1;
            } else if (pivot->offset < offset) {
                begin = pivot + 1;
            } else {
                end = pivot - 1;
            }

            break;
        }
    }

    hale_not_reached;
}
#else
#endif

hale_internal
Buf *
find_buf_GTE(FixedGapArena *arena, memi *offset, Buf *begin, Buf *end)
{
    Buf *it = begin;
    memi o = 0;
    while (it != end)
    {
        o += buf_length(it);
        if (o >= *offset) {
            *offset -= (o - buf_length(it));
            return it;
        }
        ++it;
    }
    return NULL;
}

hale_internal
Buf *
find_buf_GT(FixedGapArena *arena, memi *offset, Buf *begin, Buf *end)
{
    Buf *it = begin;
    memi o = 0;
    while (it != end)
    {
        o += buf_length(it);
        if (o > *offset) {
            *offset -= (o - buf_length(it));
            return it;
        }
        ++it;
    }
    return NULL;
}

// TODO: Do we really need to fill the previous buffer?
// TODO: Do we really need to move remainder to the next buffer?
// Perhaps it'll be better if we will make an incremental defragmentation.
// - That way code here will get easier and faster.
// - Also can swap to disk and update the pools.

hale_internal
void
insert_non_crit_branch(FixedGapArena *arena,
                       memi offset,
                       const ch8 *data,
                       memi size,
                       Buf *it0)
{
    requirement_check_buf(it0);

    Buf *it1 = NULL;

    // TODO: Merge the `it` with next and previous buffers.

    // [...**]
    //     ^^----suffix----vv
    // [..+++] [+++++] [+++**]
    //    ^^^   ^^^^^   ^^^
    //    +++   +++++   +++**
    //    p0      p1     p2 ^-sx

    // http://cpp.sh/9yp22

    // Copy to first (possible split)
    memi p0 = minimum(buf_capacity - offset, size);
    // Copy to new buffers (full)
    memi p1 = (size-p0) & ~buf_align_mask;
    // Copy to last (partial)
    memi p2 = (size-p0) &  buf_align_mask; // same as (data_size - r.p1 - r.p0);
    // Split size (won't underflow, as offset must be within the block (or == buf_length))
    memi sx = buf_length(it0) - offset;

    hale_assert(p0 + p1 + p2 == size);

    // TODO: Check if we can put part of `p1`, `p2` into sx.
    // - `p1` probably makes no sense to be merged with sx, as it's already calculated to be full.

    memi n = 0;
    // if (p0 && sx) { n += 1; }
    n += p0 && sx;
    // if (p1)       { n += p1 >> buf_capacity_shift; }
    n += p1 >> buf_align_shift;
    // if (p2)       { n += 1; }
    n += !!p2;

    if (n) {
        it1 = allocate_buffers(arena, buf_index(arena, it0) + 1, n);
        // TODO: This wouldn't be needed if allocate_buffers wouldn't
        // invalidate the pointers. (deque?)
        it0 = it1 - 1;
    }

    if (p0) {
        if (sx) {
            hale_assert_requirement(n != 0)
            buf_move_suffix(it0, offset, it0 + n); // same as `it1 + n - 1`
        }
        buf_insert(it0, offset, data, p0);

        // `data` and `size` is used in `p1` and `p2`,
        // so we update he right away.
        data += p0;
        size -= p0;
    }

    if (p1)
    {
        hale_assert_debug(it1);

        while (size != p2)
        {
            // TODO: buf_set
            buf_insert(it1, 0, data, buf_capacity);
            data += buf_capacity;
            size -= buf_capacity;
            ++it1;
        }
    }

    if (p2) {
        hale_assert_requirement(it1);
        hale_assert_requirement(p2 == size);
        // TODO: buf_set
        buf_insert(it1, 0, data, p2);
    }
}

void
fixed_gap_arena_insert(FixedGapArena *arena,
                       memi offset,
                       const ch8 *data,
                       memi size)
{
    // TODO: In case we can take ownership of data, we can just wrap it with gap buffers and be done.
    // TODO: In case we cannot take ownership of data, but the data is persistent, we can wrap it with copy-on-write gap buffers.

    // TODO: Remove the checks.
    if (size == 0) {
        return;
    }
    hale_assert(offset <= arena->size);
    hale_assert_debug(vector_count(arena->buffers));

    Buf *it0 = find_buf_GTE(arena,
                       &offset,
                       vector_begin(&arena->buffers),
                       vector_end(&arena->buffers));

    // Move these to tests.
    hale_assert_requirement(it0);
    // hale_assert(buf_length(it) != 0);
    hale_assert(buf_length(it0) >= offset);

    if (buf_available(it0) >= size) {
        buf_insert(it0, offset, data, size);
    } else {
        insert_non_crit_branch(arena, offset, data, size, it0);
    }

    arena->size += size;
}

hale_internal
void
remove_non_crit_branch(FixedGapArena *arena,
                       memi offset,
                       memi size,
                       Buf *it)
{

}

void
fixed_gap_arena_remove(FixedGapArena *arena, memi offset, memi size)
{
    // TODO: Decide how much we want to protect the API.
    //       This one is public, so maybe allow more protection?
    if (size == 0) {
        return;
    }

    hale_assert_input((offset + size) <= arena->size);
    hale_assert_input(offset <= arena->size);

    Buf *it0 = vector_begin(&arena->buffers);
    Buf *end = vector_end(&arena->buffers);
    it0 = find_buf_GT(arena, &offset, it0, end);
    hale_assert_requirement(it0 != end);

    memi length = buf_length(it0);
    if ((offset + size) < length)
    {
        buf_remove(it0, offset, size);
    }
    else
    {
        memi p2 = size;

        if (offset != 0)
        {
            p2 -= buf_remove(it0, offset, length - offset);
            ++it0;
        }

        if (p2)
        {
            Buf *itE = it0;
            length = buf_length(it0);

            if (p2 == length)
            {
                p2 = 0;
                ++itE;
            }
            else // if (p2)
            {
                while (length < p2)
                {
                    p2 -= length;
                    ++itE;
                    if (itE == end) {
                        break;
                    }
                    length = buf_length(itE);
                }

                if (p2) {
                    buf_remove(itE, 0, p2);
                }
            }


            if (it0 != itE) {
                if (vector_count(&arena->buffers) == 1) {
                    // Do not remove the first buffer.
                    // Requirement for `insert`.
                    buf_clear(it0);
                } else {
                    vector_remove(&arena->buffers, it0, itE);
                }
            }
        }
    }

    arena->size -= size;
}

ch8 *
fixed_gap_arena_text(FixedGapArena *arena, memi offset, memi size, ch8 *out)
{
    hale_assert(offset <= arena->size);
    hale_assert(size <= arena->size);
    hale_assert(offset + size <= arena->size);
    if (size == 0) {
        return out;
    }

    Buf *it = &vector_first(&arena->buffers);
    Buf *end = it + vector_count(arena->buffers);
    it = find_buf_GT(arena, &offset, it, end);

#if HALE_DEBUG
    ch8 *out_end = out + size;
#endif

    // Protected by asserts above.
    hale_assert_requirement(it != end);

    memi s = buf_length(it);
    s = minimum(size, s - offset);
    out = buf_text(it, offset, s, out);
    hale_assert_debug(out <= out_end);
    size -= s;

    while (size)
    {
        ++it;
        s = minimum(size, buf_length(it));
        out = buf_text(it, 0, s, out);
        hale_assert_debug(out <= out_end);
        size -= s;
    }

    return out;
}

void
buf_dump(Buf *buffer, Vector<ch8> *string)
{
    vector_clear(string);

    ch8 *i = buf_page_begin(buffer);

    for (; i < buffer->gap_start; ++i) {
        vector_push(string, *i);
    }

    if (buffer->gap_start == buffer->gap_end) {
        vector_push(string, (ch8)'|');
    } else {
        for (; i < buffer->gap_end; ++i) {
            vector_push(string, (ch8)'#');
        }
    }


    for (; i < buf_data_end(buffer); ++i) {
        vector_push(string, *i);
    }

    for (; i < buf_page_end(buffer); ++i) {
        vector_push(string, (ch8)'-');
    }

#ifdef HALE_DEBUG
    hale_assert(vector_count(string) == buf_capacity);
#endif

    vector_push(string, (ch8)'\n');
}

} // namespace hale
