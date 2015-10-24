#ifndef HALE_MEMORY_H
#define HALE_MEMORY_H

namespace hale {

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
};

template<typename _T, memi Size>
struct StaticMemory
{
    typedef _T T;

    _T e[Size];
    memi count;

    inline _T* adjust(memi new_capacity) {
        hale_assert(new_capacity <= Size);
        return &e[0];
    }

    inline memi capacity() {
        return Size;
    }
};

//
// Helpers.
//

template<typename T>
inline void
memory_copy(T *destination, T *source, memi count)
{
    platform.copy_memory(destination, source, count * sizeof(T));
}

inline void
memory_copy0(ch8 *destination, memi destination_count, ch8 *source)
{
    memi c = hale_minimum(destination_count-1, string_length(source)+1);
    platform.copy_memory(destination, source, c);
    destination[c] = 0;
}

inline void
memory_copy0(ch16 *destination, memi destination_count, ch16 *source)
{
    memi c = hale_minimum(destination_count-1, string_length(source)+1);
    platform.copy_memory(destination, source, c * sizeof(ch16));
    destination[c] = 0;
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
    hale_assert_input(memory->count);
    // TODO: Do custom "insert" adjustment, as we're doing two memory moves here (one for allocation, one for insert)
    memory->adjust(memory->count + count + prealloc);
    memory->count += count;
    return memory_insert(&memory->e[0], memory->count, &memory->e[0] + offset, count);
}

template<typename M>
inline typename M::T*
memory_remove_ordered(M *memory, memi offset, memi count)
{
    hale_assert_input((offset + count) < memory->count);

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

#if 0

//
// Signatures.
//

struct MallocAllocator
{
    template<typename T>
    static inline T*
    memory_allocate(memi count)
    {
        return (T*)malloc(count * sizeof(T));
    }

    template<typename T>
    static inline void
    memory_deallocate(T *memory)
    {
        free(memory);
    }

    template<typename T>
    static inline T*
    memory_reallocate(T *memory, memi size)
    {
        return (T*)realloc(memory, size * sizeof(T));
    }
};

struct PagedAllocator
{
    template<typename T>
    static inline T *
    memory_allocate(memi count)
    {
        return (T*)platform.allocate_memory(count * sizeof(T));
    }

    template<typename T>
    static inline void
    memory_deallocate(T *memory)
    {
        platform.deallocate_memory(memory);
    }

    template<typename T>
    static inline T *
    memory_reallocate(T *memory, memi new_size)
    {
        return platform.reallocate_memory(memory, new_size);
    }
};

struct StaticAllocator
{
    template<typename T>
    static inline T *
    memory_allocate(memi count);

    template<typename T>
    static inline void
    memory_deallocate(T *memory);

    template<typename T>
    static inline T *
    memory_reallocate(T *memory, memi new_size);
};

//
//
//

//
//
//

template<typename T, typename Allocator>
struct Memory
{
    typedef Allocator Allocator;
    typedef T E;

    E *e;
    memi count;
    memi capacity;
};

template<typename T, memi Size>
struct StaticMemory
{
    typedef T T;

    T e[Size];
    memi count;
};

template<typename T, typename Allocator>
inline memi
memory_allocate(Memory<T, Allocator> *memory, memi capacity)
{
    memory->capacity = capacity;
    memory->e = Allocator::memory_allocate<T>(capacity);
    return capacity;
}

template<typename T, typename Allocator>
inline void
memory_deallocate(Memory<T, Allocator> *memory)
{
    if (memory->e) {
        Allocator::memory_deallocate<T>(memory->e);
        memory->e = NULL;
        memory->count = 0;
        memory->capacity = 0;
    }
}

template<typename T, typename Allocator>
inline void
memory_reallocate(Memory<T, Allocator> *memory, memi new_capacity)
{
    if (memory->e == 0) {
        memory->e = Allocator::memory_allocate<T>(new_capacity);
        memory->capacity = new_capacity;
    } else if (new_capacity > memory->capacity) {
        memory->e = Allocator::memory_reallocate<T>(memory->e, new_capacity);
        memory->capacity = new_capacity;
    }
}

template<typename T, typename Allocator>
inline memi
memory_insert(Memory<T, Allocator> *memory, memi offset, memi count)
{
    hale_not_tested;

    Allocator::memory_reallocate(memory, count + memory->capacity);

    // .....|.....~~~~~
    //       ^^^^^vvvvv
    // .....|#####.....
    memory_move(memory->e + offset + count, memory->e + offset, memory->count - offset);
    memory->count += count;

    return memory->count;
}

//template<typename T, typename Allocator>
//T *
//memory_push(Memory<T, Allocator> *memory, memi count, memi prealloc = 1)
//{
//    hale_assert_input(prealloc >= count);
//    Allocator::memory_reallocate(memory, memory->capacity + prealloc);
//    T *ret = &memory->e[memory->count];
//    memory->count += count;
//    return ret;
//}

void *
_memory_push(void *memory_e, memi *memory_count, memi memory_e_size, memi count, memi prealloc)
{
    hale_assert_input(prealloc >= count);
//    Allocator::memory_reallocate(memory, memory->capacity + prealloc);
//    T *ret = &memory->e[memory->count];
//    memory->count += count;
//    return ret;
    return 0;
}

#define memory_push(memory, _count, _prealloc) \
    (memory_reallocate(memory, memory->capacity + (_count) + (_prealloc)), &memory.e[memory.count++])

template<typename T, typename Allocator>
inline T*
memory_pop(Memory<T, Allocator> *memory)
{
    hale_assert_input(memory->count != 0);
    memory->count--;
    return &memory->e[memory->count];
}

//
//
//

template<typename Memory>
inline typename Memory::T*
memory_insert(Memory *memory, memi offset, memi count)
{
    hale_assert((memory->count + count) <= hale_array_count(memory->e));

    Memory::T *e = memory->e + offset;
    if (memory->count != 0)
    {
        // .....|.....~~~~~
        //       ^^^^^vvvvv
        // .....|#####.....
        memory_move(e + count,
                    e,
                    memory->count - offset);
    }
    memory->count += count;
    return e;
}

template<typename Memory>
inline typename Memory::T*
memory_remove_ordered(Memory *memory, memi offset, memi count)
{
    hale_assert((offset+count) <= memory->count);

    // .....xxx...~~
    //      vvv^^^
    // ........~~~~~
    Memory::T *e = memory->e + offset;
    memory_move(e,
                e + count,
                memory->count - (offset + count));

    memory->count -= count;

    return e;
}
#endif

} // namespace hale

#endif // HALE_MEMORY_H

