#include <audio/channelBase.h>
#include <audio/papu.h>
#include <base/cyclicCounter.imp.h>
#include <base/clock.h>
#include <common/common.h>

namespace gbemu {

ChannelBase::ChannelBase(
    const PAPUClocks& clocks,
    std::mutex& mutex
) :
    _clocks( clocks ),
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
    // bool dumped = false;
    // if (!dumped) {
    //     std::cout << "Audio range : [" << audioStartInFrames << ", " << audioStartInFrames + frameCount << "]" << std::endl;
    //     std::cout << "Index range : [" << _firstEvent << ", " << _lastEvent << "]" << std::endl;
    //     std::cout << "Sample range : [" << _soundEvents[_firstEvent].time << ", " << _soundEvents[_lastEvent - 1].time << "]" << std::endl;
    //     dumped = true;
    // }
    /*for (BufferIndex i = _firstEvent; i != _lastEvent; ++i) {
        std::cout << _soundEvents[i].time << " " << int(_soundEvents[i].sample) << std::endl;
    }*/

    // FIXME: Why do I have to do this???
    char* output = reinterpret_cast<char*>(raw_output);

    if (_firstEvent == _lastEvent) {
        //std::cout << "Nothing to play!!!" << std::endl;
        // if (dumped) {
        //     std::cout << "----" << std::endl;
        // }
        return;
    }
    else {
        BufferIndex currentEvent = _firstEvent;
        int64_t currentFrame = audioStartInFrames;
        //std::cout << (int)_soundEvents[_firstEvent].sample << std::endl;
        for (int i = 0; i < frameCount; ++i, ++currentFrame) {
            // Sync up to the next valid sample.
            // This the end of this sound is before the current frame.
            while (
                currentEvent != _lastEvent &&
                _soundEvents[currentEvent].endTime <= currentFrame
            ) {
                ++currentEvent;
            }

            if (currentEvent == _lastEvent) {
                break;
            }

            //if (currentFrame < _soundEvents[currentEvent].time) {
                //std::cout << "Missing@" << currentFrame << std::endl;
              //  continue;
            //}
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
    for (BufferIndex i = _firstEvent; i != _lastEvent; ++i) {
        // If this sound ends after or at the current time.
        if (_soundEvents[i].endTime >= currentTime) {
            break;
        }
        JFX_CMP_ASSERT(_firstEvent, !=, _lastEvent);
        ++_firstEvent;
    }
    
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
    const int64_t audioTime = cpuTime * 44100 / _clocks.cpu.getRate();
    _soundEvents[_lastEvent - 1].endTime = audioTime;
    _soundEvents[_lastEvent] = SoundEvent(audioTime, sample);
    ++_lastEvent;
}

}
