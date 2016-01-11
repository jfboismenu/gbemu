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

    template<int CycleLength, int ClockAt>
    class ClockT
    {
    public:
        enum {kLength = CycleLength, kClockAt = ClockAt};
        ClockT(int count = 0);
        int count() const;
        void reset();
        bool increment();
    private:
        int _count;
    };
}
