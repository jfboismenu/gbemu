#pragma once

#include <cstdint>

namespace gbemu {

    class Clock
    {
    public:
        Clock(int64_t rate);
        Clock& operator+=(int cycles);
        int64_t getTimeInCycles() const;
        float getTimeInSeconds() const;
        int getRate() const;

    private:
        int64_t _rate;
        int64_t _time;
    };
}
