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
    private:
        int64_t _count;
    };

    class Clock
    {
    public:
        Clock(int64_t count = 0, int64_t cycleLength = 1);
        bool increment();
        int64_t getCycleLength() const;
    private:
        int64_t _count;
        int64_t _cycleLength;
    };
}
