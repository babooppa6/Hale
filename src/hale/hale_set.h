#ifndef HALE_SET_H
#define HALE_SET_H

#if HALE_INCLUDES
#include "hale_stack_memory.h"
#endif

namespace hale {

// Keeps data in format:
// (u8 key_length) (u8 key[]) (u8 0) (u8 data_length) (u8 data[])?
// The zero is added to be compatible with c-strings.
// NOTE: Set is only able to keep data of max length 255.
//       (256 minus zero terminator)

struct Set
{
    // TODO: Use a more suitable memory model here.
    //       Deque would be good.

    PagedMemory memory;
    memi count;
};

void set_init(Set *S, memi reserve);

inline b32
set_contains(Set *S, u8 *string) {
    return string >= hale_memory_at(u8, &S->memory, 0) &&
           string < hale_memory_end(u8, &S->memory);
}

// `out_index` will contain the 0-based index of the string.
// `out_data_begin`, `out_data_end` will contain pointers to data
//     associated with the string from the `set_add` call.
u8 *set_find(Set *S, u8 tag,
             u8 *begin, u8 *end,
             memi *out_index,
             void **out_data);

u8 *set_find(Set *S, u8 tag, u8 *begin, u8 *end);

// WARNING: Use this only for initialization when the set it empty
//          or when set_find returns 0.
u8 *set_add(Set *S, u8 tag, u8 *begin, u8 *end, void *data);

} // namespace hale

#endif // HALE_SET_H
