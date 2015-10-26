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
memory_insert(M *memory, memi offset, memi count, memi prealloc = 0)
{
    hale_assert_input(count);
    // TODO: Do custom "insert" adjustment, as we're doing two memory moves here (one for allocation, one for insert)
    memory->grow(memory->count + count + prealloc);
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

template<typename M>
inline typename M::T *
memory_push(M *memory, memi count, memi prealloc = 0)
{
    hale_assert_input(count);
    memory->grow(memory->count + count + prealloc);
    auto ret = &memory->e[memory->count];
    memory->count += count;
    return ret;
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
    typedef _T T;

    _T *e;
    memi count;
    memi _capacity;

    inline _T* allocate(memi capacity) {
        return (_T*)malloc(capacity * sizeof(_T));
    }

    inline void deallocate() {
        hale_assert(e);
        free(e);
    }

    inline void reset() {
        if (e) {
            deallocate();
            *this = {};
        }
    }

    inline _T* reallocate(memi new_capacity) {
        return e = (_T*)realloc(e, new_capacity * sizeof(_T));
    }

    inline _T* grow(memi new_capacity) {
        if (e == 0) {
            e = allocate(new_capacity);
            hale_assert(e);
            _capacity = new_capacity;
        } else if (new_capacity > _capacity) {
            reallocate(new_capacity);
            hale_assert(e);
            _capacity = new_capacity;
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

    inline T* push(memi count, memi prealloc = 0) {
        return memory_push(this, count, prealloc);
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

    inline _T* grow(memi new_capacity) {
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

    inline T* push(memi count, memi prealloc = 0) {
        return memory_push(this, count, prealloc);
    }

    inline T* pop() {
        return memory_pop(this);
    }
};

template<typename T>
inline b32
equal(Memory<T> *a, Memory<T> *b)
{
    return equal<T>(a->e, &a->e[a->count],
                    b->e, &b->e[b->count]);
}

} // namespace hale

#endif // HALE_MEMORY_H

