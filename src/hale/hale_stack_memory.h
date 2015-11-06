#ifndef HALE_STACK_MEMORY_H
#define HALE_STACK_MEMORY_H

#if HALE_INCLUDES
#include "hale_memory.h"
#endif

namespace hale {

struct PagedMemory;

void  memory_init(PagedMemory *memory, memi reserve, void *base);
void *memory_push(PagedMemory *memory, memi size);
void *memory_pop(PagedMemory *memory, memi size);

#define hale_memory_push(type, memory, count)\
    (type*)memory_push((memory), (count) * sizeof(type))

#define hale_memory_at(type, memory, at) \
    ( hale_assert((at * sizeof(type)) <= (memory)->count), \
      (type*)((memory)->base) + at \
    )

#define hale_memory_end(type, memory) \
    ( (type*)((u8*)((memory)->base) + (memory)->count) )

#define hale_memory_begin(type, memory) \
    ( (type*)(memory)->base )

#define hale_memory_count(type, memory) \
    ( hale_memory_end(type, memory) - hale_memory_begin(type, memory) )

struct PagedMemory
{
    void *base;
    memi count;
    memi capacity;
    memi _reserved;

    inline void *push(memi size) {
        memory_push(this, size);
    }

    inline void *pop(memi size) {
        memory_pop(this, size);
    }
};

//
//
//

template<typename _T>
struct StackMemory
{
    typedef _T T;
	_T *base;
    memi count;
    memi _capacity;

    PagedMemory *stack;

    StackMemory(PagedMemory *_stack, memi count) :
        stack(_stack),
        base((_T*)(((u8*)_stack->base) + _stack->count)),
        count(0),
        _capacity(0)
    {
        push(count);
    }

    ~StackMemory() {
        if (_capacity) {
            pop_capacity(_capacity);
        }
    }

    inline _T *
    push_capacity(memi capacity) {
        _capacity += capacity;
        return (_T*)memory_push(stack, capacity * sizeof(_T));
    }

    inline _T *
    pop_capacity(memi capacity) {
        hale_assert_input(capacity >= this->_capacity);
        memory_pop(stack, capacity);
        _T *ret = base + this->_capacity;
        this->_capacity -= capacity;
        return ret;
    }

    inline _T *
    push(memi count) {
        if (this->count + count > _capacity) {
            push_capacity(count);
        }
        _T *ret = this->base + this->count;
        this->count += count;
        return ret;
    }

    inline memi
    capacity() {
        return _capacity;
    }

    inline _T *
    ptr() {
        return base;
    }

    inline _T &
    ref() {
        return *ptr();
    }

    inline _T &operator *() {
        return ref();
    }
    inline _T *operator ->() {
        return ptr();
    }
    inline operator _T*() {
        return ptr();
    }
};

} // namespace hale

#endif // HALE_STACK_MEMORY_H

