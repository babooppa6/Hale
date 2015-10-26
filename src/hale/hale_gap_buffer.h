#ifndef HALE_GAP_BUFFER_H
#define HALE_GAP_BUFFER_H

#if HALE_INCLUDES
#include "hale_types.h"
#include "hale_test.h"
#endif

#define HALE_GAP_BUFFER_NEVER_EMPTY

// TODO: Make gap buffer work with bytes. Type safety has to be done in the caller.

// HALE_GAP_BUFFER_SIMPLE
// mem move size  909 473 966
// mem move count 176 291
// mem move       5158.94 bytes per move

// no HALE_GAP_BUFFER_SIMPLE
// mem move size  908 721 442
// mem move count 176 293
// mem move       5154.61 bytes per move

// size diff = 752 524

// #define HALE_GAP_BUFFER_SIMPLE

namespace hale {

struct GapBufferStatistics
{
    memi gap_too_small;
    memi mem_move_size;
    memi mem_move_count;
};

struct GapBuffer
{
    ch *text;
    memi gap_start;
    memi gap_end;
    memi length;
    memi capacity;
    memi gap;

#ifdef HALE_TEST
    GapBufferStatistics statistics;
    TestCoverage coverage;
#endif
};

void gap_buffer_init(GapBuffer *buffer, memi capacity, memi gap);
void gap_buffer_release(GapBuffer *buffer);
void gap_buffer_text(GapBuffer *buffer, memi start, memi length, ch *out);
void gap_buffer_insert(GapBuffer *buffer, memi start, ch *text, memi text_length);
void gap_buffer_remove(GapBuffer *buffer, memi start, memi length);

void gap_buffer_dump(GapBuffer *buffer, Vector<ch> *string);

}

#endif // HALE_GAP_BUFFER_H

