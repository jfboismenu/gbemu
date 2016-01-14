#include <base/counter.h>

namespace gbemu {

    Counter::Counter(
        int count,
        int cycleLength
    ) : _count(count), _cycleLength(cycleLength)
    {}

    bool Counter::increment()
    {
        _count = (++_count) % _cycleLength;
        return _count == 0;
    }

    int Counter::getCycleLength() const
    {
        return _cycleLength;
    }

    int Counter::count() const
    {
        return _count;
    }
    void Counter::reset()
    {
        _count = 0;
    }

}
