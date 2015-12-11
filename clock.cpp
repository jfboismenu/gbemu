#include "clock.h"

namespace gbemu {


    Clock::Clock(int64_t rate) : _rate(rate), _timeFromStart(0)
    {}

    Clock& Clock::operator+=(int cycles)
    {
        _timeFromStart += cycles;
        return *this;
    }

    int Clock::getTimeFromStart() const
    {
        return _timeFromStart;
    }

    int Clock::getTimeInCycle() const
    {
        return _timeFromStart % _rate;
    }

    int Clock::getRate() const
    {
        return _rate;
    }

}
