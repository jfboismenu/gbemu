#pragma once

#include <audio/channelBase.h>
#include <base/cyclicCounter.imp.h>
#include <base/clock.h>

namespace gbemu {

template< typename SoundEventType >
ChannelBase<SoundEventType>::ChannelBase(
    const Clock& clock
) :
    _clock( clock ),
    _firstEvent(0),
    _lastEvent(0),
    _playbackLastEvent(0)
{}

template< typename SoundEventType >
void ChannelBase<SoundEventType>::updateEventsQueue(
    const float audioFrameStartInSeconds
)
{
    std::lock_guard<std::mutex> lock(_mutex);
    for (BufferIndex i = _firstEvent; i != _playbackLastEvent ; ++i) {

        // If sound is looping and the end of that audio event is before this audio frame.
        if (_soundEvents[i].isLooping) {
             // If this is not the last event, the next event might silence this one?
            if (i + 1 != _playbackLastEvent) {
                // if that next event starts before the current audio
                if (_soundEvents[i + 1].waveStartInSeconds < audioFrameStartInSeconds) {
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

}
