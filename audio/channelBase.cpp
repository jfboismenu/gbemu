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
    const float realTime
)
{}

}
