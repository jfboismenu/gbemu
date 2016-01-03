#include <audio/common.h>


namespace gbemu {

WaveChannelStateBase::WaveChannelStateBase(
    bool il,
    int64_t ws,
    float wsis,
    float wlis,
    float wf,
    float d
) : isLooping(il),
    waveFrequency(wf),
    waveStart(ws),
    waveStartInSeconds(wsis),
    waveLengthInSeconds(wlis),
    delta(d)
{}

float WaveChannelStateBase::waveEndInSeconds() const
{
    return waveStartInSeconds + waveLengthInSeconds;
}

float WaveChannelStateBase::getPositionInsideWaveform(const float frameTimeInSeconds) const
{
    const float timeSinceEventStart = (frameTimeInSeconds - (waveStartInSeconds + delta));

    // Compute how many times the sound has played, including fractions.
    const float howManyTimes = (timeSinceEventStart - int(timeSinceEventStart)) * waveFrequency;
    // Compute how many times the sound has completely played.
    const float howManyTimesCompleted = int(howManyTimes);
    // Compute how far we are in the current cycle.
    const float howManyInCurrent = howManyTimes - howManyTimesCompleted;

    return howManyInCurrent;
}

}
