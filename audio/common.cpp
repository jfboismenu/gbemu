#include <audio/common.h>

namespace {
    using namespace gbemu;
    short sampleToShort(SoundMix mix, char sample)
    {
        if (mix == SoundMix::silent) {
            return 0;
        } else if (mix == SoundMix::left) {
            return sample;
        } else if (mix == SoundMix::right) {
            return sample;
        } else {
            return sample;
        }
    }

}

namespace gbemu {

SoundEvent::SoundEvent(
    int64_t t /* time */,
    char s /* sample */,
    SoundMix m
): time(t),
   endTime(t + 1),
   sample(sampleToShort(m, s))
{}

}
