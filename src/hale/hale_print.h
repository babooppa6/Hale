#ifndef HALE_PRINT
#define HALE_PRINT

#if HALE_INCLUDES
#include "hale_types.h"
#include "hale_macros.h"
#include "hale_os.h"
#endif

namespace hale {

inline void
print(const ch8 *string) {
    platform.debug_print((ch8*)string);
}

inline void
print(const char *string) {
    platform.debug_print((ch8*)string);
}

inline void
print(const ch16 *string) {
    platform.debug_print((ch16*)string);
}

//
// TODO: Remove {
//

inline void
print(Vector<ch8> *string) {
    platform.debug_print(&vector_first(string), vector_count(string));
}

inline void
print(Vector<ch8> &string) {
    platform.debug_print(&vector_first(&string), vector_count(&string));
}

inline void
print(Vector<ch16> *string) {
    platform.debug_print(&vector_first(string), vector_count(string));
}

inline void
print(Vector<ch16> &string) {
    platform.debug_print(&vector_first(&string), vector_count(&string));
}

//
// }
//

inline void
print(const ch *b, const ch *e)
{
    platform.debug_print((ch*)b, e-b);
}

inline void
print(const ch *b, memi count)
{
    platform.debug_print((ch*)b, count);
}

inline void
print(const Memory<ch> &memory)
{
    hale_assert_input(memory.ptr);
    platform.debug_print(memory.ptr, memory.count);
}

//
//
//

// TODO: Support for stack memory.

template<typename M>
struct StringSink
{
    M *memory;

    enum {
        Flag_AddSpaces  = 0x01,
        Flag_AddNewLine = 0x02,
        Flag_AddZero    = 0x04,

        Flags_All = Flag_AddSpaces | Flag_AddNewLine | Flag_AddZero,
    };
    u32 flags;

    StringSink(M *memory, u32 flags = Flags_All) :
        memory(memory),
        flags(flags)
    {}

    ~StringSink()
    {
        if (flags & Flag_AddNewLine) {
            ch *nl = memory->push(1, 0);
            *(nl) = '\n';
        }

        if (flags & Flag_AddZero) {
            ch *nl = memory->push(1, 0);
            *(nl) = 0;
        }
    }

    inline ch *
    push(memi count) {
        ch *sp;
        if (count) {
            if (flags & Flag_AddSpaces) {
                sp = memory->push(count + 1, 0);
                *(sp + count) = ' ';
            } else {
                sp = memory->push(count, 0);
            }
        } else {
            sp = memory->push(0, 0);
        }
        return sp;
    }

    inline ch *
    push_capacity(memi capacity) {
        return memory->push_capacity(capacity);
    }

    template<typename T>
    inline StringSink &
    operator <<(T value) {
        return hale::sink<StringSink>(*this, value);
    }
};


template<typename M>
inline void
print(M *memory)
{
    platform.debug_print(memory->ptr, memory->count);
}

template<typename M>
inline void
print(const StringSink<M> &sink)
{
    print(sink.memory);
}

struct PrintSink
{
    Memory<ch> memory;
    StringSink<Memory<ch>> sink;
    PrintSink() : memory {}, sink(&memory) {}
    ~PrintSink() {
        print(sink);
        memory.deallocate_safe();
    }

    template<typename T>
    PrintSink &operator <<(T value) {
        sink << value;
        return *this;
    }
};


#if 0
struct Print
{
    Memory<ch> *memory;
    Memory<ch> _internal;
    b32 own;

    Print() { _internal = {}; memory = &_internal; own = true; }
    Print(Memory<ch> *memory) : memory(memory),  own(false) {}
    Print(Memory<ch> &memory) : memory(&memory), own(false) {}
    ~Print() {
        ch *nl = memory->push(2, 0);
        *(nl-1) = '\n';
        *(nl-0) = 0;
        // TODO: Whether we will use zero-terminated or length-string depends
        //       on the platform preference, so maybe it'll be better to either
        //       #if it or implement in the platform.
        print(memory->e);
        if (own) {
            hale_assert_input(memory);
            memory->deallocate_safe();
        }
    }

    inline ch *
    push(memi count, memi capacity) {
		ch *sp;
		if (count) {
			sp = memory->push(count + 1, capacity);
			*(sp + count) = ' ';
		} else {
			sp = memory->push(0, capacity);
		}
        return sp;
    }

    template<typename T>
    inline Print &
    sink(T value) {
        return hale::sink<Print>(*this, value);
    }

    template<typename T>
    inline Print &
    operator <<(T value) {
        return hale::sink<Print>(*this, value);
    }
};
#endif

} // namespace hale

#endif // HALE_PRINT

