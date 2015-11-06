#if HALE_INCLUDES
#include "hale_macros.h"
#include "hale_types.h"
#include "hale_stack_memory.h"
#include "hale_set.h"
#endif

namespace hale {

void
set_init(Set *S, memi reserve)
{
    *S = {};
    memory_init(&S->memory, reserve, 0);
}

// TODO: This is just a memcmp.
hale_internal b32
_set_equal(ch8 *haystack, ch8 *needle, memi count)
{
    hale_assert_requirement(count != 0);

    ch8 *haystack_it = haystack + count - 1;
    ch8 *needle_it = needle + count - 1;
    for (;;)
    {
        if (*haystack_it != *needle_it) {
            return 0;
        }
        if (haystack_it == haystack) {
            // hale_assert_requirement(needle_it == needle);
            return 1;
        }
        haystack_it--;
        needle_it--;
    }
    hale_not_reached;
}

u8 *
set_find(Set *S, u8 tag, u8 *needle_begin, u8 *needle_end)
{
    hale_assert_input(needle_begin < needle_end);
    memi length = needle_end - needle_begin;
    hale_assert_input(length < 0xFF);

    u8 *search_it = hale_memory_begin(u8, &S->memory);
    u8 *search_end = hale_memory_end(u8, &S->memory);
    memi key_len;
    u8 key_tag;

    while (search_it != search_end)
    {
        key_len = *search_it;
        key_tag = *search_it++;
        search_it++;
        if (key_len == length &&
            _set_equal(search_it, needle_begin, length))
        {
            return search_it;
        }
        search_it += key_len + 1 + sizeof(void*);
    }

    return 0;
}

u8 *
set_find(Set *S,
         u8 tag,
         u8 *needle_begin, u8 *needle_end,
         memi *out_index,
         void **out_data)
{
    hale_assert_input(needle_begin < needle_end);

    hale_assert_input(out_index);
    hale_assert_input(out_data);

    memi needle_length = needle_end - needle_begin;
    hale_assert_input(needle_length < 0xFF);

    *out_index = 0;

    u8 *search_it = hale_memory_begin(u8, &S->memory);
    u8 *search_end = hale_memory_end(u8, &S->memory);
    memi key_len;
    u8 key_tag;

    while (search_it != search_end)
    {
        key_len = *search_it++;
        key_tag = *search_it++;
        if (key_tag == tag &&
            key_len == needle_length &&
            _set_equal(search_it, needle_begin, needle_length))
        {
            *out_data = *((void**)(search_it + key_len));
            return search_it;
        }
        // Skip key, zero and data.
        search_it += key_len + 1 + sizeof(void*);
        *out_index++;
    }

    return 0;
}

u8 *
set_add(Set *S, u8 tag, u8 *begin, u8 *end, void *data)
{
    hale_assert_input((end - begin) < 0xFF);

    memi key_count = end - begin;

    u8 *ptr = hale_memory_push(u8,
                               &S->memory,
                               // count + tag + length + 0
                               (1 + 1 + key_count + 1) + sizeof(void*));

    // Write key length.
    *ptr++ = (u8)(key_count);
    *ptr++ = (u8)(tag);
    // Write key.
    memory_copy(ptr, begin, key_count); ptr += key_count;
    // Write zero terminator.
    *ptr++ = 0;

    *(void**)ptr++ = data;

    S->count++;

    return ptr;
}


}
