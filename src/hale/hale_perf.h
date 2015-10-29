#ifndef HALE_PERF_H
#define HALE_PERF_H

#include "hale.h"

#ifdef HALE_OS_WIN
#include <intrin.h>
#endif

namespace hale {

// TODO: __rdtsc

struct performance_timer
{
    r64 time;
    u64 cycles;
    ch *name;

#ifdef HALE_OS_WIN
    performance_timer(const ch *name0) :
        time(platform.read_time_counter()),
        cycles(__rdtsc())
    {
        memi length = (string_length(name0)+1);
        name = (ch*)malloc(length * sizeof(ch));
        memory_copy(name, (ch*)name0, length);
    }
#else
    performance_timer(const char *name) :
        name(strdup(name)),
        time(platform.read_time_counter()),
        cycles(__rdtsc())
    {}
#endif

    r64 time_delta() {
        return platform.read_time_counter() - time;
    }

    void print()
    {
        u64 ticks_delta = __rdtsc() - cycles;
        r64 time_d  = time_delta();

        Print() << name << hale_ch("\n")
                << hale_ch("    time ") << (time_d * 1e3) << hale_ch("ms") << hale_ch("\n")
                << hale_ch("    ticks") << ((r64)ticks_delta) << hale_ch("cy") << hale_ch("\n")
                << hale_ch("    ratio") << ((r64)ticks_delta / time_d)/(1e6) << hale_ch("cy/ms") << hale_ch("\n");

        cycles = __rdtsc() - ticks_delta;
        time  = platform.read_time_counter() - time_d;
    }

    ~performance_timer()
    {
        print();
        free(name);
    }
};

#define HALE_PERFORMANCE_TIMER_RELEASE(t) performance_timer t(hale_ch(#t))
#define HALE_PERFORMANCE_TIMER(t) performance_timer t(hale_ch(#t))

} // namespace hale

#endif // HALE_PERF_H
