#include <base/clock.h>

namespace gbemu {


    Clock::Clock(int64_t rate) : _rate(rate), _time(0)
    {}

    Clock& Clock::operator+=(int cycles)
    {
        _time += cycles;
        return *this;
    }

    int64_t Clock::getTimeInCycles() const
    {
        return _time;
    }

    float Clock::getTimeInSeconds() const
    {
        return float(_time) / getRate();
    }

    int Clock::getRate() const
    {
        return _rate;
    }

}
