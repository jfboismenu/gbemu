#pragma once

#include <cstdint>

namespace gbemu {

    class CPUClock
    {
    public:
        CPUClock(int64_t rate);
        CPUClock& operator+=(int cycles);
        int64_t getTimeInCycles() const;
        float getTimeInSeconds() const;
        int getRate() const;

    private:
        int64_t _rate;
        int64_t _time;
    };

    template<int CycleLength, int64_t ClockAt>
    class ClockT
    {
    public:
        enum {kLength = CycleLength, kClockAt = ClockAt};
        ClockT(int64_t count = 0);
        bool increment();
        // The clock is clocking each time the internal count
        // reaches the ClockAt value.
        bool isClocking() const;
    private:
        int64_t _count;
    };
}
