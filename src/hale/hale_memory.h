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
memory_grow(M *memory, memi count, memi capacity)
{
    if ((memory->count + count) > memory->capacity()) {
        memory->reallocate(memory->count + count + capacity);
    }
    return memory->ptr;
}

// TODO: Merge with memory_grow?
template<typename M>
inline typename M::T *
memory_push_capacity(M *memory, memi capacity)
{
    if ((memory->count + capacity) > memory->capacity()) {
        memory->reallocate(memory->count + capacity);
    }
    return memory->ptr + memory->count;
}

template<typename M>
inline typename M::T *
memory_insert(M *memory, memi offset, memi count, memi capacity = 0)
{
    hale_assert_input(count);
    // TODO: Do custom "insert" adjustment, as we're doing two memory moves here (one for allocation, one for insert)
    memory_grow(memory, memory->count, capacity);
    memory->count += count;
    return memory_insert(memory->ptr, memory->count, memory->ptr + offset, count);
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
    M::T *ptr = memory->ptr + offset;
    memory_move(ptr,
                ptr + count,
                memory->count - (offset + count));

    memory->count -= count;
    return ptr;
}

template<typename M>
inline typename M::T *
memory_push(M *memory, memi count, memi capacity)
{
    hale_assert_input(count);

    M::T *p = memory_grow(memory, count, capacity);

    p += memory->count;
    memory->count += count;
    return p;
}

template<typename M>
inline typename M::T *
memory_pop(M *memory)
{
    hale_assert_input(memory->count != 0);
    memory->count--;
    return memory->ptr + memory->count;
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

    _T *ptr;
    memi count;
    memi _capacity;

    inline _T* allocate(memi capacity) {
        _capacity += capacity;
        return ptr = (_T*)malloc(capacity * sizeof(_T));
    }

    inline _T* allocate_safe(memi capacity) {
        _capacity += capacity;
        hale_assert_debug(ptr == NULL);
        return ptr = (_T*)malloc(capacity * sizeof(_T));
    }

    inline void deallocate() {
        hale_assert_debug(ptr);
        free(ptr);
    }

    inline void deallocate_safe() {
        if (ptr) {
            deallocate();
            *this = {};
        }
    }

    inline void reallocate(memi new_capacity) {
        ptr = (_T*)realloc(ptr, new_capacity * sizeof(_T));
        hale_assert(ptr);
        _capacity = new_capacity;
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

    inline T* push_capacity(memi capacity) {
        return memory_push_capacity(this, capacity);
    }

    inline T* push(memi count, memi capacity) {
        return memory_push(this, count, capacity);
    }

    inline T* pop() {
        return memory_pop(this);
    }

    T &operator [](size_t index) {
        hale_assert(index < count);
        return ptr[index];
    }
};

template<typename _T, memi Size>
struct StaticMemory
{
    typedef _T T;

    _T ptr[Size];
    memi count;

    // TODO: reallocate_any

    inline void reallocate(memi new_capacity) {
        hale_assert(new_capacity <= Size);
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
    return equal<T>(a->ptr, a->ptr + a->count,
                    b->ptr, b->ptr + b->count);
}

} // namespace hale

#endif // HALE_MEMORY_H

