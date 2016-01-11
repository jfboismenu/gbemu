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

    Clock::Clock(
        int64_t count,
        int64_t cycleLength
    ) : _count(count), _cycleLength(cycleLength)
    {}

    bool Clock::increment()
    {
        _count = (++_count) % _cycleLength;
        return _count == 0;
    }

    int64_t Clock::getCycleLength() const
    {
        return _cycleLength;
    }

}
