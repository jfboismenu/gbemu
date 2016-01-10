#pragma once

#include <common/common.h>

namespace gbemu {

    enum class SoundMix {silent, left, right, both};
    struct SoundEvent
    {
        SoundEvent(
            int64_t time,
            char sample,
            SoundMix mix
        );
        SoundEvent() = default;

        int64_t   time;
        int64_t   endTime;
        short     sample;
        SoundMix  mix;
    };
}
