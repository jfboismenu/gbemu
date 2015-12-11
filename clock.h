#pragma once

#include <cstdint>

namespace gbemu {

    class Clock
    {
    public:
        Clock(int64_t rate);
        Clock& operator+=(int cycles);
        int getTimeFromStart() const;
        int getTimeInCycle() const;
        int getRate() const;

    private:
        int64_t _rate;
        int64_t _timeFromStart;
    };
}
