#include <audio/common.h>


namespace gbemu {

SoundEventBase::SoundEventBase(
    bool ip,
    bool il,
    int wf
) : isPlaying(ip),
    isLooping(il),
    waveFrequency(wf)
{}

}
