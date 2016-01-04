#include <audio/channelBase.h>
#include <base/cyclicCounter.imp.h>
#include <base/clock.h>
#include <common/common.h>

namespace gbemu {

ChannelBase::ChannelBase(
    const Clock& clock,
    std::mutex& mutex
) :
    _clock( clock ),
    _mutex( mutex ),
    _firstEvent( 0 ),
    _lastEvent( 0 ),
    _playbackLastEvent( 0 )
{}

void ChannelBase::renderAudio(
    void* raw_output,
    const unsigned long frameCount,
    const int rate,
    const int64_t audioStartInFrames
)
{
/*    bool dumped = false;
    if (!dumped) {
        std::cout << "Audio range : [" << audioStartInFrames << ", " << audioStartInFrames + frameCount << "]" << std::endl;
        std::cout << "Index range : [" << _firstEvent << ", " << _lastEvent << "]" << std::endl;
        std::cout << "Sample range : [" << _soundEvents[_firstEvent].time << ", " << _soundEvents[_lastEvent - 1].time << "]" << std::endl;
        dumped = true;
    }*/
    /*for (BufferIndex i = _firstEvent; i != _lastEvent; ++i) {
        std::cout << _soundEvents[i].time << " " << int(_soundEvents[i].sample) << std::endl;
    }*/

    // FIXME: Why do I have to do this???
    char* output = reinterpret_cast<char*>(raw_output);

    if (_firstEvent == _lastEvent) {
        //std::cout << "Nothing to play!!!" << std::endl;
        return;
    }
    else {
        BufferIndex currentEvent = _firstEvent;
        //std::cout << (int)_soundEvents[_firstEvent].sample << std::endl;
        for (int i = 0; i < frameCount; ++i) {
            const int64_t currentFrame = audioStartInFrames + i;
            // Sync up to the next valid sample.
            // While there is another event after this one.
            while (
                (currentEvent + 1) != _lastEvent &&
                _soundEvents[currentEvent + 1].time <= currentFrame
            ) {
                ++currentEvent;
            }

            if (currentFrame < _soundEvents[currentEvent].time) {

                //std::cout << "Missing@" << currentFrame << std::endl;
                continue;
            }
            output[i] += _soundEvents[currentEvent].sample;
            //std::cout << int(output[i]) << std::endl;
        }
    }
    // if (dumped) {
    //     std::cout << "----" << std::endl;
    // }
}

void ChannelBase::updateEventsQueue(int64_t currentTime)
{
    // Empty
    if (_firstEvent == _lastEvent) {
        return;
    }

    // std::cout << "first sample time : " << _soundEvents[_firstEvent].time << std::endl;
    // std::cout << "current : " << currentTime << std::endl;
    // std::cout << "last sample time : " << _soundEvents[_lastEvent - 1].time << std::endl;
    // std::cout << "nb samples : " << _lastEvent - _firstEvent << std::endl;

    // If the event list only contains one event, there's nothing to do.
    // Either it started before this currentTime, and therefore it is still playing.
    // Either if started afrer this current time, and therefore it might start playing
    // in this frame.
    for (BufferIndex i = _firstEvent + 1; i < _lastEvent; ++i, ++_firstEvent) {
        // As soon as we find an event that starts after the currentTime,
        // we know that the previous one will have started at or before it.

        // Consider the following:
        // Current time = 5
        // Sound events at 1, 3, 5, 7.
        // Indexes are     0, 1, 2, 3 (lastEvent is 4)
        // i = 1, firstEvent = 0
        // 3 not greater than 5, so we can move our first event pointer to it. 
        // i = 2, firstEvent = 1
        // 5 is not greater than 5, so we can move out first event point to it.
        // i = 3, firstEvent = 2
        // 7 is greater than 5, so we break.

        // Consider a scenario where current is 5
        // events are  0, 3, 4
        // Indexes are 0, 1, 2 (lastEvent is 3)
        // on the last loop,
        // i = 2, firstEvent = 1
        // 4 is still not lower than 5.
        // i now becomes 3, first event is not 2.
        // loop breaks beause i is not lower than lastEvent
        if (_soundEvents[i].time > currentTime) {
            break;
        }
    }
    // std::cout << "new first sample : " << _soundEvents[_firstEvent].time << std::endl;
    // std::cout << "-----------------------------" << std::endl;
}

void ChannelBase::insertEvent(
    int64_t cpuTime,
    char sample
)
{
    const SoundEvent& previousEvent = _soundEvents[_lastEvent - 1];
    // No need to add an event if the sample hasn't changed.
    if (previousEvent.sample == sample) {
        return;
    }
    _soundEvents[_lastEvent] = SoundEvent(
        cpuTime * 44100 / _clock.getRate(),
        sample
    );
    ++_lastEvent;
}

}
