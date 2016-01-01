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

float WaveChannelStateBase::getPositionInsideWaveform(const float frameTimeInSeconds) const
{
    const float timeSinceEventStart = (frameTimeInSeconds - waveStartInSeconds);

    const float cycleLength = 1 / waveFrequency;
    // Compute how many times the sound has played, including fractions.
    const float howManyTimes = timeSinceEventStart / cycleLength;
    // Compute how many times the sound has completely played.
    const float howManyTimesCompleted = int(howManyTimes);
    // Compute how far we are in the current cycle.
    const float howManyInCurrent = howManyTimes - howManyTimesCompleted;

    return howManyInCurrent;
}

}
