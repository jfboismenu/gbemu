#pragma once

#include <common/common.h>
#include <base/clock.h>

namespace gbemu {

template<int CycleLength, int ClockAt>
JFX_INLINE ClockT<CycleLength, ClockAt>::ClockT(
    int const count
) : _count(count)
{}

template<int CycleLength, int ClockAt>
JFX_INLINE bool ClockT<CycleLength, ClockAt>::increment()
{
    _count = (++_count) % CycleLength;
    return _count == ClockAt;
}

template<int CycleLength, int ClockAt>
int ClockT<CycleLength, ClockAt>::count() const
{
    return _count;
}

template<int CycleLength, int ClockAt>
void ClockT<CycleLength, ClockAt>::reset()
{
    _count = 0;
}

}
