#pragma once

#include <audio/channelBase.h>
#include <base/cyclicCounter.imp.h>
#include <base/clock.h>
#include <common/common.h>

namespace gbemu {

template< typename Derived, typename SoundEventType >
ChannelBase<Derived, SoundEventType>::ChannelBase(
    const Clock& clock,
    std::mutex& mutex
) :
    _clock( clock ),
    _mutex( mutex ),
    _firstEvent(0),
    _lastEvent(0),
    _playbackLastEvent(0)
{}

template< typename Derived, typename SoundEventType >
void ChannelBase<Derived, SoundEventType>::insertEvent(const SoundEventType& event)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _soundEvents[_lastEvent] = event;
    _soundEvents[_lastEvent].timeStamp = _clock.getTimeInSeconds();
    ++_lastEvent;
    JFX_CMP_ASSERT(_firstEvent, !=, _lastEvent);
}


template< typename Derived, typename SoundEventType >
void ChannelBase<Derived, SoundEventType>::updateEventsQueue(
    const float audioFrameStartInSeconds
)
{
    for (BufferIndex i = _firstEvent; i != _playbackLastEvent ; ++i) {

        // If sound is looping and the end of that audio event is before this audio frame.
        if (_soundEvents[i].isLooping) {
             // If this is not the last event, the next event might silence this one?
            if (i + 1 != _playbackLastEvent) {
                // if that next event starts before the current audio
                if (_soundEvents[i + 1].timeStamp < audioFrameStartInSeconds) {
                    ++_firstEvent;
                } else {
                    // The next event starts after the current playback interval,
                    // so we can assume that this sound event will play
                    // in the current playback interval.
                    break;
                }
            } else {
                // This sound event is looping and is the last in the queue,
                // so it is still playing.
                break;
            }
        }
        // Audio is not looping, so if the end of this event is before the
        // section we want to render, skip it.
        else if (_soundEvents[i].waveEndInSeconds() < audioFrameStartInSeconds) {
            ++_firstEvent;
        } else {
            // The _firstEvent is still playing or hasn't started yet, so it
            // follows logic that
            // the ones after will also be playing, so we can break.
            break;
        }
    }
    // We need to capture the state of the lastEvent member because the main thread may try
    // to udpate that index while we are rendering audio.
    _playbackLastEvent = _lastEvent;
}

template< typename Derived, typename SoundEventType >
void ChannelBase<Derived, SoundEventType>::renderAudio(
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
        } else if (currentCpuCycle < _soundEvents[currentEvent].timeStamp) {
            // If we still haven't reached the first note, play nothing.
            continue;
        }

        output[i] += _soundEvents[currentEvent].computeSample(frameTimeInSeconds);
    }
}

}
