#include <audio/common.h>


namespace gbemu {

WaveChannelStateBase::WaveChannelStateBase(
    bool il,
    float wf,
    int64_t ws,
    float wsis,
    float wlis
) : isLooping(il),
    waveFrequency(wf),
    waveStart(ws),
    waveStartInSeconds(wsis),
    waveLengthInSeconds(wlis)
{}

float WaveChannelStateBase::waveEndInSeconds() const
{
    return waveStartInSeconds + waveLengthInSeconds;
}


}
