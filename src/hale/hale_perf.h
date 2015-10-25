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
    char *name;

#ifdef HALE_OS_WIN
    performance_timer(const char *name) :
        name(_strdup(name)),
        time(platform.read_time_counter()),
        cycles(__rdtsc())
    {}
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

//        qDebug() << name << "\n"
//                 << "    time "  << (time_d * 1e3) << "ms" << "\n"
//                 << "    ticks" << ((r64)ticks_delta) << "cy" << "\n"
//                 << "    ratio" << ((r64)ticks_delta / time_d)/(1e6) << "cy/ms" << "\n";

        cycles = __rdtsc() - ticks_delta;
        time  = platform.read_time_counter() - time_d;
    }

    ~performance_timer()
    {
        print();
        delete [] name;
    }
};

#define HALE_PERFORMANCE_TIMER_RELEASE(t) performance_timer t(#t)
#define HALE_PERFORMANCE_TIMER(t) performance_timer t(#t)

} // namespace hale

#endif // HALE_PERF_H
