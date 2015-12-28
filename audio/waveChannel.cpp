#include <audio/waveChannel.h>
#include <audio/channelBase.imp.h>
#include <base/cyclicCounter.imp.h>
#include <base/clock.h>
#include <base/logger.h>

namespace gbemu {

WaveChannel::WaveChannel(
    const Clock& clock
) :
    ChannelBase(clock)
{}

void WaveChannel::renderAudio(
    void* raw_output,
    const unsigned long frameCount,
    const int rate,
    const float realTime
)
{
    char* output = reinterpret_cast<char*>(raw_output);
    // Update the interval of sound we're about to produce
    // Convert the start and end to seconds.
    const float startInSeconds = realTime;
    const float endInSeconds = realTime + (float(frameCount) / rate);
    updateEventsQueue(startInSeconds);


    const int cycleStart = startInSeconds * _clock.getRate();
    const int cycleEnd = endInSeconds * _clock.getRate();

    // Queue is empty, do not play anything.
    if (_firstEvent == _playbackLastEvent) {
        return;
    }

    BufferIndex currentEvent = _firstEvent;

    for (unsigned long i = 0 ; i < frameCount; ++i) {
        // Peneration of the loop.
        const float depth = float(i) / frameCount;
        // Compute the current cpu cycle.
        const int currentCpuCycle = depth * (cycleEnd - cycleStart) + cycleStart;
        // Compute the real time in seconds this sample will represent.
        const float frameTimeInSeconds = realTime + (float(i) / rate);

        // If there are no more events to process, play nothing.
        if (currentEvent == _playbackLastEvent) {
            break;
        } else if (currentCpuCycle < _soundEvents[currentEvent].waveStart) {
            // If we still haven't reached the first note, play nothing.
            continue;
        }
        if (_soundEvents[currentEvent].isPlaying) {
            const float timeSinceEventStart = (frameTimeInSeconds - _soundEvents[currentEvent].waveStartInSeconds);
            output[i] += 0;
            /*computeSample(
                _soundEvents[currentEvent].waveFrequency, 
                timeSinceEventStart,
                _soundEvents[currentEvent].waveDuty
            ) * _soundEvents[currentEvent].getVolumeAt(frameTimeInSeconds);*/
        }
    }
}

void WaveChannel::writeByte(
    const unsigned short addr,
    const unsigned char value
)
{
    // {
    //     _rFrequencyHiPlayback.write( value );
    //     // JFX_LOG("-----NR14-ff14-----");
    //     // JFX_LOG("Frequency hi : " << (int)_rFrequencyHiPlayback.bits.freqHi);
    //     // JFX_LOG("Consecutive  : " << ( _rFrequencyHiPlayback.bits.isLooping() ? "loop" : "play until NR11-length expires" ));
    //     // JFX_LOG("Initialize?  : " << ( _rFrequencyHiPlayback.bits.initialize == 1 ));

    //     // Push a new sound event in a thread-safe manner.
    //     std::lock_guard<std::mutex> lock(_mutex);
    //     const int gbNote = getGbNote();
    //     JFX_CMP_ASSERT(2048 - gbNote, >, 0);
    //     _soundEvents[_lastEvent] = SquareWaveSoundEvent(
    //         _rFrequencyHiPlayback.bits.initialize == 1,
    //         _rFrequencyHiPlayback.bits.isLooping(),
    //         gbNoteToFrequency(gbNote),
    //         _clock.getTimeInCycles(),
    //         _clock.getTimeInSeconds(),
    //         _rLengthDuty.bits.getSoundLength(),
    //         _rLengthDuty.bits.getWaveDutyPercentage(),
    //         _rEnveloppe.bits.initialVolume,
    //         _rEnveloppe.bits.isAmplifying(),
    //         _rEnveloppe.bits.getSweepLength()
    //     );
    //     ++_lastEvent;
    //     JFX_CMP_ASSERT(_firstEvent, !=, _lastEvent);
    // }
}

}
