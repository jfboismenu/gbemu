#include <base/clock.imp.h>

namespace gbemu {


    CPUClock::CPUClock(int64_t rate) : _rate(rate), _time(0)
    {}

    CPUClock& CPUClock::operator+=(int cycles)
    {
        _time += cycles;
        return *this;
    }

    int64_t CPUClock::getTimeInCycles() const
    {
        return _time;
    }

    float CPUClock::getTimeInSeconds() const
    {
        return float(_time) / getRate();
    }

    int CPUClock::getRate() const
    {
        return _rate;
    }
}
