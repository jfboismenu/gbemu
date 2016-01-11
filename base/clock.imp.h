#pragma once

#include <common/common.h>
#include <base/clock.h>

namespace gbemu {

template<int CycleLength, int64_t ClockAt>
JFX_INLINE ClockT<CycleLength, ClockAt>::ClockT(
    int64_t const count
) : _count(count)
{}

template<int CycleLength, int64_t ClockAt>
JFX_INLINE bool ClockT<CycleLength, ClockAt>::increment()
{
    _count = (++_count) % CycleLength;
    return _count == ClockAt;
}

}
