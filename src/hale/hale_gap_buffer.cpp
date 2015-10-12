#include "hale.h"
#include "hale_test.h"
#include "hale_gap_buffer.h"
#include "hale_platform.h"

#define hale_gap_buffer_gap_size(g) (g->gap_end - g->gap_start)

namespace hale {

inline void
buffer_move(GapBuffer *buffer,
            ch * dst, memi dst_length, memi dst_offset,
            ch * src, memi src_length, memi src_offset,
            memi length)
{
    // hale_assert(length != 0);

    // TODO: This should be debug-only, and we should add checks to higher level.
    hale_assert_debug(dst != 0);
    hale_assert_debug(src != 0);
    hale_assert_debug(dst_offset <= dst_length);
    hale_assert_debug(src_offset <= src_length);
    hale_assert_debug((dst_offset + length) <= dst_length);
    hale_assert_debug((src_offset + length) <= src_length);

    // TODO: Test with if-ing the length != 0
    platform.move_memory(dst + dst_offset, src + src_offset, length * sizeof(ch));

#ifdef HALE_TEST
    buffer->statistics.mem_move_size += length * sizeof(ch);
    buffer->statistics.mem_move_count ++;
#endif
}

hale_internal void
ensure_capacity(GapBuffer *buffer, memi capacity)
{
    if (capacity >= buffer->capacity)
    {
        // TODO: Align the capacity to page size.
        memi new_capacity = hale_align_up_to_page_size_t(buffer->capacity * ((capacity / buffer->capacity) + 1), sizeof(ch));
        ch  *new_text = (ch*)platform.allocate_memory(new_capacity * sizeof(ch));
        memi buffer_size = buffer->length + hale_gap_buffer_gap_size(buffer);
        buffer_move(buffer,
                    new_text, new_capacity, 0,
                    buffer->text, buffer_size, 0,
                    buffer_size
                    );

        platform.deallocate_memory(buffer->text);
        buffer->text = new_text;
        buffer->capacity = new_capacity;
    }
}

//
//
//

void
gap_buffer_init(GapBuffer *buffer, memi capacity, memi gap)
{
    hale_assert(capacity > 0);
    hale_assert(gap < capacity);

    buffer->text = (ch*)platform.allocate_memory(capacity * sizeof(ch));
    buffer->gap_start = 0;
    buffer->gap_end = gap;
    buffer->length = 0;
    buffer->capacity = capacity;
    buffer->gap = gap;

#ifdef HALE_TEST
    buffer->statistics = {};
    test_coverage_init(&buffer->coverage);
#endif
}

void
gap_buffer_release(GapBuffer *buffer)
{
    hale_assert(buffer);

    platform.deallocate_memory(buffer->text);
    buffer->text = NULL;
}

//
// Internal
//

hale_internal void
move_gap_start(GapBuffer *buffer, memi new_start)
{
    memi new_end = buffer->gap_end + (new_start - buffer->gap_start);

    if (new_start > buffer->gap_start)
    {
        // .....#####.|....
        //      v----^
        // ......#####|....
        buffer_move(buffer,
                    buffer->text, buffer->capacity, buffer->gap_start,
                    buffer->text, buffer->capacity, buffer->gap_end,
                    new_start - buffer->gap_start
                    );
    }
    else if (new_start < buffer->gap_start)
    {
        // .|....#####  .....
        //   ^^^^-vvvvv
        // .|#####..... .....
        buffer_move(buffer,
                    buffer->text, buffer->capacity, new_end,
                    buffer->text, buffer->capacity, new_start,
                    buffer->gap_start - new_start
                    );
    }

    buffer->gap_start = new_start;
    buffer->gap_end = new_end;
}

hale_internal void
move_gap_end(GapBuffer *buffer, memi new_end)
{
    // .....#####.....
    //        v--^^^^^--v
    //

    buffer_move(buffer,
                buffer->text, buffer->capacity, new_end,
                buffer->text, buffer->capacity, buffer->gap_end,
                buffer->length - buffer->gap_start
                );

    buffer->gap_end = new_end;
}

//
//
//

void
gap_buffer_text(GapBuffer *buffer, memi start, memi length, ch *out)
{
    if (start >= buffer->gap_start) {
        buffer_move(buffer,
                    out, length, 0,
                    buffer->text, buffer->capacity, start + hale_gap_buffer_gap_size(buffer),
                    length
                    );
    }
    else if ((start + length) <= buffer->gap_start)
    {
        buffer_move(buffer,
                    out, length, 0,
                    buffer->text, buffer->capacity, start,
                    length
                    );
    }
    else
    {
        buffer_move(buffer,
                    out, length, 0,
                    buffer->text, buffer->capacity, start,
                    buffer->gap_start - start
                    );

        buffer_move(buffer,
                    out, length, (buffer->gap_start - start),
                    buffer->text, buffer->capacity, buffer->gap_end,
                    start + length - buffer->gap_start
                    );
    }
}

// Gap buffer tries to do as much work as possible up-front. That means that when a reallocations or multiple memory moves are required to edit the text, the buffer will also include creating of a new gap. The resulting behavior is that the gap is never empty.

#ifdef HALE_GAP_BUFFER_SIMPLE

void
gap_buffer_insert(GapBuffer *buffer, memi insert_offset, ch *insert_text, memi insert_length)
{
    move_gap_start(buffer, insert_offset);

    if (hale_gap_buffer_gap_size(buffer) < insert_length)
    {
        ensure_capacity(buffer, buffer->length + insert_length + buffer->gap);
        move_gap_end(buffer, insert_offset + insert_length + buffer->gap);
    }

    buffer_move(buffer,
                buffer->text, buffer->capacity, insert_offset,
                insert_text, insert_length, 0,
                insert_length
                );

    buffer->gap_start += insert_length;
    buffer->length += insert_length;
}

#else // HALE_GAP_BUFFER_SIMPLE

void
gap_buffer_insert(GapBuffer *buffer, memi insert_offset, ch *insert_text, memi insert_length)
{
    // TODO: Try to pull-out the insert_text memmove to the end.
#ifdef HALE_TEST
    buffer->coverage.branches_total = __COUNTER__;
#endif
    memi buffer_gap = hale_gap_buffer_gap_size(buffer);
    memi buffer_size = buffer->length + buffer_gap;

    // Not enough space in the gap for the text.

#ifdef HALE_GAP_BUFFER_NEVER_EMPTY
    memi dst_capacity = buffer->length + buffer->gap + insert_length;
#else
    memi dst_capacity = buffer->length + buffer_gap + insert_length;
#endif

#ifdef HALE_GAP_BUFFER_NEVER_EMPTY
    if (buffer->capacity <= dst_capacity)
#else
    if (buffer->capacity < dst_capacity)
#endif
    {
        // Not enough capacity for the text

        memi new_gap_end = insert_offset + insert_length + buffer->gap;

        dst_capacity = buffer->capacity * ((dst_capacity / buffer->capacity) + 1);
        dst_capacity = hale_align_up_to_page_size_t(dst_capacity, sizeof(ch));
        // dst_capacity = buffer->capacity * ((dst_capacity / buffer->capacity) + 1);
        ch *dst_text = (ch*)platform.allocate_memory(dst_capacity * sizeof(ch));

        // TODO: Special for when start == 0?
        // TODO: Special for when start == buffer_size (end of buffer)?

        memi src_index, dst_index, size;

        if (insert_offset == buffer->gap_start)
        {
            /**/ HALE_TEST_COVERAGE_BRANCH(&buffer->coverage, "realloc ==");
            // test_gap_buffer_t1_ne

            //     .....|###.....
            // (1) vvvvv
            //     .....

            //          |+++
            // (2)       vvv
            //     .....|+++###

            //     .....|###.....
            // (3)          ^^^^^vvvvv
            //     .....|+++###  .....


            // (1)
            buffer_move(buffer,
                        dst_text, dst_capacity, 0,
                        buffer->text, buffer_size, 0,
                        insert_offset);
            // (2)
            buffer_move(buffer,
                        dst_text, dst_capacity, buffer->gap_start,
                        insert_text, insert_length, 0,
                        insert_length
                        );
            // (3)
            buffer_move(buffer,
                        dst_text, dst_capacity, new_gap_end,
                        buffer->text, buffer_size, buffer->gap_end,
                        buffer_size - buffer->gap_end
                        );

            buffer->gap_start += insert_length;
            buffer->gap_end = new_gap_end;
        }
        else if (insert_offset < buffer->gap_start)
        {
            /**/ HALE_TEST_COVERAGE_BRANCH(&buffer->coverage, "realloc <");
            // test_gap_buffer_t3_ne

            // (1) .|....###.....
            //     v
            //     .

            // (2)   +++
            //       vvv
            //     .|+++###

            // (3) .|....###.....
            //       ^^^^--vvvv
            //     .|+++###....

            // (4) .|....###.....
            //              ^^^^^vvvvv
            //     .|+++###....  .....

            // (1)
            dst_index = 0;
            src_index = 0;
            size = insert_offset;
            buffer_move(buffer,
                        dst_text, dst_capacity, dst_index,
                        buffer->text, buffer_size, src_index,
                        size);

            // (2)
            dst_index += size;
            buffer_move(buffer,
                        dst_text, dst_capacity, dst_index,
                        insert_text, insert_length, 0,
                        insert_length);

            // (3)
            src_index += size; // size from (1)
            dst_index += insert_length + buffer->gap; // skip gap
            hale_assert_debug(dst_index == new_gap_end);
            size = buffer->gap_start - insert_offset;
            buffer_move(buffer,
                        dst_text, dst_capacity, dst_index,
                        buffer->text, buffer_size, src_index,
                        size);

            // (4)
            src_index += size + buffer_gap; // skip gap
            hale_assert_debug(src_index == buffer->gap_end);
            dst_index += size;
            size = buffer_size - buffer->gap_end;
            buffer_move(buffer,
                        dst_text, dst_capacity, dst_index,
                        buffer->text, buffer_size, src_index,
                        size);

            buffer->gap_start = insert_offset + insert_length;
            buffer->gap_end = new_gap_end;
        }
        else if (insert_offset > buffer->gap_start)
        {
            /**/ HALE_TEST_COVERAGE_BRANCH(&buffer->coverage, "realloc >");
            // (1) .....###..|...
            //     vvvvv
            //     .....

            // (2) .....###..|...
            //          vv-^^
            //     .......|

            // (3)        |+++
            //             vvv
            //     .......|+++###|

            // (4) .....###..|...
            //                ^^^vvv
            //     .......|+++###...

            // (1)
            src_index = 0;
            dst_index = 0;
            size = buffer->gap_start;
            buffer_move(buffer,
                        dst_text, dst_capacity, dst_index,
                        buffer->text, buffer_size, src_index,
                        size);

            // (2)
            src_index += size + buffer_gap; // skip gap
            hale_assert_debug(src_index == buffer->gap_end);
            dst_index += size;
            size = (insert_offset + buffer_gap) - buffer->gap_end;
            buffer_move(buffer,
                        dst_text, dst_capacity, dst_index,
                        buffer->text, buffer_size, src_index,
                        size);

            // (3)
            dst_index += size;
            buffer_move(buffer,
                        dst_text, dst_capacity, dst_index,
                        insert_text, insert_length, 0,
                        insert_length);

            // (4)
            src_index += size; // size from (2)
            dst_index += insert_length + buffer->gap; // skip gap
            size = buffer_size - src_index;
            buffer_move(buffer,
                        dst_text, dst_capacity, dst_index,
                        buffer->text, buffer_size, src_index,
                        size);

            buffer->gap_start = insert_offset + insert_length;
            buffer->gap_end = dst_index;
        }

        platform.deallocate_memory(buffer->text);
        buffer->text = dst_text;
        buffer->capacity = dst_capacity;

    } // if (buffer->capacity < dst_capacity)
#ifdef HALE_GAP_BUFFER_NEVER_EMPTY
    else if (buffer_gap <= insert_length)
#else
    else if (buffer_gap < insert_length)
#endif
    {
        // Insufficient gap space, but enough capacity.
        // We will insert the new text and create a new gap space.
#ifdef HALE_GAP_BUFFER_NEVER_EMPTY
        memi new_gap_end = insert_offset + insert_length + buffer->gap;
#else
        memi new_gap_end = insert_offset + insert_length + buffer_gap;
#endif

#ifdef HALE_TEST
        buffer->statistics.gap_too_small++;
#endif

        if (insert_offset == buffer->gap_start)
        {
            if (buffer_size != buffer->gap_end)
            {
                /**/ HALE_TEST_COVERAGE_BRANCH(&buffer->coverage, "gap ==");
                //     .....|#.....
                // (1)        ^^^^^vvvvv
                //     .....|      ..... // move gap end to accomodate new text_length (and a new gap?)
                // (2)       +++
                //     .....|+++###..... // insert the text

                buffer_move(buffer,
                            buffer->text, buffer->capacity, new_gap_end,
                            buffer->text, buffer->capacity, buffer->gap_end,
                            buffer_size - buffer->gap_end);
            }
            else
            {
                HALE_TEST_COVERAGE_BRANCH(&buffer->coverage, "ok == (gap)");
            }
        }
        else if (insert_offset < buffer->gap_start)
        {
            /**/ HALE_TEST_COVERAGE_BRANCH(&buffer->coverage, "gap <");
            //     .|....#.....
            // (1)        ^^^^^vvvvv
            //     .|....      .....
            // (2)   ^^^^--vvvv
            //     .|      .........
            // (3)   +++
            //     .|+++###.........

            // (1)
            buffer_move(buffer,
                        buffer->text, buffer->capacity, new_gap_end + (buffer->gap_start - insert_offset),
                        buffer->text, buffer->capacity, buffer->gap_end,
                        buffer_size - buffer->gap_end);

            // (2) c
            buffer_move(buffer,
                        buffer->text, buffer->capacity, new_gap_end,
                        buffer->text, buffer->capacity, insert_offset,
                        buffer->gap_start - insert_offset);
        }
        else if (insert_offset > buffer->gap_start)
        {
            /**/ HALE_TEST_COVERAGE_BRANCH(&buffer->coverage, "gap >");
            //     .....#.|....
            // (1)         ^^^^vvvv
            //     .....#.|    ....
            // (2)      v^
            //     ......|     ....
            // (3)        +++
            //     ......|+++###....

            // (1)
            memi n = insert_offset + buffer_gap;

            buffer_move(buffer,
                        buffer->text, buffer->capacity, new_gap_end,
                        buffer->text, buffer->capacity, insert_offset + buffer_gap,
                        buffer_size - n);

            // (2)
            buffer_move(buffer,
                        buffer->text, buffer->capacity, buffer->gap_start,
                        buffer->text, buffer->capacity, buffer->gap_end,
                        n - buffer->gap_end);

        }

        // (3)
        buffer_move(buffer,
                    buffer->text, buffer->capacity, insert_offset,
                    insert_text, insert_length, 0,
                    insert_length);

        buffer->gap_start = insert_offset + insert_length;
        buffer->gap_end = new_gap_end;

    } // if (buffer_gap < insert_length)
    else
    {
        // Enough gap space (and therefore capacity).

        if (insert_offset == buffer->gap_start)
        {
            /**/ HALE_TEST_COVERAGE_BRANCH(&buffer->coverage, "ok ==");
            // test_gap_buffer_t1

            // ----------------------------------------------------------
            // Most common scenario.
            // ----------------------------------------------------------


            //     .....|#####.....
            // (1)       +++
            //     .....|+++##.....

            // Stays the same.
            buffer->gap_end = buffer->gap_end;
        }
        else if (insert_offset < buffer->gap_start)
        {
            /**/ HALE_TEST_COVERAGE_BRANCH(&buffer->coverage, "ok <");
            //       ____ <- length = n
            //     .|....#####.....
            // (1)   ^^^^-vvvv
            //     .|#####.........
            // (2)   +++
            //     .|+++##.........

            memi n = buffer->gap_start - insert_offset;
            buffer_move(buffer,
                        buffer->text, buffer->capacity, buffer->gap_end - n,
                        buffer->text, buffer->capacity, insert_offset,
                        n);

            buffer->gap_end = buffer->gap_end - n;
        }
        else if (insert_offset > buffer->gap_start)
        {
            /**/ HALE_TEST_COVERAGE_BRANCH(&buffer->coverage, "ok >");

            //               _ <- length = n
            //     .....#####.|....
            // (1)      v----^
            //     ......#####|....
            //
            //
            // (2)        +++
            //     ......|+++##....

            // (1)
            memi n = (insert_offset + buffer_gap) - buffer->gap_end;
            buffer_move(buffer,
                        buffer->text, buffer->capacity, buffer->gap_start,
                        buffer->text, buffer->capacity, buffer->gap_end,
                        n);

            buffer->gap_end += n;
        }

        buffer_move(buffer,
                    buffer->text, buffer->capacity, insert_offset,
                    insert_text, insert_length, 0,
                    insert_length);

        buffer->gap_start = insert_offset + insert_length;
    }

    buffer->length += insert_length;
    hale_assert_debug(buffer->gap_start <= buffer->length);
    hale_assert_debug(buffer->gap_start <= buffer->gap_end);

#ifdef HALE_TEST
    buffer->coverage.branches_total = __COUNTER__ - buffer->coverage.branches_total - 1;
#endif
}
#endif

void
gap_buffer_remove(GapBuffer *buffer, memi start, memi length)
{
    move_gap_start(buffer, start);
    buffer->gap_end += length;
    buffer->length -= length;
}

void
gap_buffer_dump(GapBuffer *buffer, Vector<ch> *string)
{
    vector_clear(string);

    memi i = 0;

    for (; i < buffer->gap_start; i++) {
        vector_push(string, buffer->text[i]);
    }

    if (buffer->gap_start == buffer->gap_end) {
        vector_push(string, L'|');
    } else {
        for (; i < buffer->gap_end; i++) {
            vector_push(string, L'#');
        }
    }


    for (; i < (buffer->length + hale_gap_buffer_gap_size(buffer)); i++) {
        vector_push(string, buffer->text[i]);
    }

    for (; i < buffer->capacity; i++) {
        vector_push(string, L'-');
    }

    // vector_push(string, (ch)0);
}

}
