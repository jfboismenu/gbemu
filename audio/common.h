#pragma once

#include <common/common.h>

namespace gbemu {
    struct SoundEvent
    {
        SoundEvent(
            int64_t time,
            char sample
        );
        SoundEvent() = default;

        int64_t time;
        int64_t endTime;
        char    sample;
    };
}
