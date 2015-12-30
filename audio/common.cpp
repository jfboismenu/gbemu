#include <audio/common.h>


namespace gbemu {

SoundEventBase::SoundEventBase(
    bool il,
    int wf,
    int64_t ws,
    float wsis,
    float wlis
) : isLooping(il),
    waveFrequency(wf),
    waveStart(ws),
    waveStartInSeconds(wsis),
    waveLengthInSeconds(wlis)
{}

float SoundEventBase::waveEndInSeconds() const
{
    return waveStartInSeconds + waveLengthInSeconds;
}


}
