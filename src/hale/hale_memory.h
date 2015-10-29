#ifndef HALE_MEMORY_H
#define HALE_MEMORY_H

#if HALE_INCLUDES
#include "hale_macros.h"
#include "hale_types.h"
#include "hale_os.h"
#endif

namespace hale {

template<typename _T>
struct Memory;

template<typename _T, memi Size>
struct StaticMemory;

//
// Helpers.
//

template<typename T>
inline void
memory_copy(T *destination, T *source, memi count)
{
    platform.copy_memory(destination, source, count * sizeof(T));
}

template<typename T>
inline void
memory_move(T *destination, T *source, memi count)
{
    platform.move_memory(destination, source, count * sizeof(T));
}

template<typename T>
inline T*
memory_insert(T *e, memi e_count, T *at, memi count)
{
    // .....|.....~~~~~
    //       ^^^^^vvvvv
    // .....|#####.....
    memory_move(at + count,
                at,
                e_count - (at - e));
    return at;
}

//
//
//

template<typename M>
inline typename M::T *
memory_insert(M *memory, memi offset, memi count, memi capacity = 0)
{
    hale_assert_input(count);
    // TODO: Do custom "insert" adjustment, as we're doing two memory moves here (one for allocation, one for insert)
    memory->reallocate_if_more(memory->count + maximum(count, capacity));
    memory->count += count;
    return memory_insert(&memory->e[0], memory->count, &memory->e[0] + offset, count);
}

template<typename M>
inline typename M::T*
memory_remove_ordered(M *memory, memi offset, memi count)
{
	hale_assert_input(count);
    hale_assert_input((offset + count) <= memory->count);

    // .....xxx...~~
    //      vvv^^^
    // ........~~~~~
    M::T *e = memory->e + offset;
    memory_move(e,
                e + count,
                memory->count - (offset + count));

    memory->count -= count;
    return e;
}

// TODO: This is just stupid, I need memory_push for fucking with count, and other memory_push_capacity for just allocating at the end without fucking with count.
//       For now I'm going with more complex way of doing this, but that actually should do what is expected it to do.

template<typename M>
inline typename M::T *
memory_push(M *memory, memi count, memi capacity)
{
    hale_assert_input(count + capacity);

    M::T *p;
    capacity = maximum(count, capacity);
    if ((memory->count + capacity) <= memory->capacity()) {
        p = memory->e + memory->count;
    } else {
        p = memory->reallocate_any(memory->capacity() + capacity) + memory->count;
    }

    memory->count += count;
    return p;
}

template<typename M>
inline typename M::T *
memory_pop(M *memory)
{
    hale_assert_input(memory->count != 0);
    memory->count--;
    return &memory->e[memory->count];
}

//
//
//

template<typename _T>
struct Memory
{
//#if HALE_DEBUG
//    Memory<_T>() = default : e((_T*)-1), count((memi)-1), _capacity((memi)-1) {}
//    Memory<_T> &operator =(const Memory<_T> &other) {
//        e = other.e;
//        count = other.count;
//        _capacity = other._capacity;
//        return *this;
//    }

//#define hale_memory_check_debug hale_assert_debug(e != (_T*)-1 && count != (memi)-1 && _capacity != (memi)-1)
//#else
//#define hale_memory_check_debug
//#endif

    typedef _T T;

    _T *e;
    memi count;
    memi _capacity;

    inline _T* allocate(memi capacity) {
        return e = (_T*)malloc(capacity * sizeof(_T));
    }

    inline _T* allocate_safe(memi capacity) {
        hale_assert_debug(e == NULL);
        return e = (_T*)malloc(capacity * sizeof(_T));
    }

    inline void deallocate() {
        hale_assert_debug(e);
        free(e);
    }

    inline void deallocate_safe() {
        if (e) {
            deallocate();
            *this = {};
        }
    }

    inline _T* reallocate_any(memi new_capacity) {
        e = (_T*)realloc(e, new_capacity * sizeof(_T));
        hale_assert(e);
        _capacity = new_capacity;
        return e;
    }

    inline _T* reallocate_if_more(memi new_capacity) {
        if (new_capacity > _capacity) {
            return reallocate_any(new_capacity);
        }
        return e;
    }

    inline memi capacity() {
        return _capacity;
    }

    inline T* insert(memi offset, memi count, memi prealloc = 0) {
        return memory_insert(this, offset, count, prealloc);
    }

    inline T* remove_ordered(memi offset, memi count) {
        return memory_remove_ordered(this, offset, count);
    }

    inline T* push(memi count, memi capacity) {
        return memory_push(this, count, capacity);
    }

    inline T* pop() {
        return memory_pop(this);
    }
};

template<typename _T, memi Size>
struct StaticMemory
{
    typedef _T T;

    _T e[Size];
    memi count;

    // TODO: reallocate_any

    inline _T* reallocate_if_more(memi new_capacity) {
        hale_assert(new_capacity <= Size);
        return &e[0];
    }

    inline memi capacity() {
        return Size;
    }

    inline T* insert(memi offset, memi count, memi prealloc = 0) {
        return memory_insert(this, offset, count, prealloc);
    }

    inline T* remove_ordered(memi offset, memi count) {
        return memory_remove_ordered(this, offset, count);
    }

    inline T* push(memi count, memi capacity) {
        return memory_push(this, count, capacity);
    }

    inline T* pop() {
        return memory_pop(this);
    }
};

template<typename T>
inline b32
equal(Memory<T> *a, Memory<T> *b)
{
    return equal<T>(a->e, a->e + a->count,
                    b->e, b->e + b->count);
}

} // namespace hale

#endif // HALE_MEMORY_H

