#include <audio/common.h>

namespace gbemu {

SoundEvent::SoundEvent(
    int64_t t /* time */,
    char s /* sample */
): time(t),
   sample(s)
{}

}
