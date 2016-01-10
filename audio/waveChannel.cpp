#include <audio/waveChannel.h>
#include <audio/papu.h>
#include <cpu/registers.h>
#include <base/cyclicCounter.imp.h>
#include <base/clock.h>
#include <base/logger.h>

namespace {
    float gbNoteToFrequency(const int gbNote)
    {
        JFX_CMP_ASSERT(2048 - gbNote, >, 0);
        return 65536.f / (2048 - gbNote);
    }
}

namespace gbemu {

WaveChannel::WaveChannel(
    const PAPUClocks& clocks,
    std::mutex& mutex
) :
    ChannelBase(clocks, mutex),
    Frequency(kNR33, kNR34, 2),
    _currentSample(0),
    _wavePatternPtr(&_wavePattern[ 0 ] - kWavePatternRAMStart)
{}

bool WaveChannel::contains(unsigned short addr) const
{
    switch(addr) {
        case kNR30:
        case kNR31:
        case kNR32:
        case kNR33:
        case kNR34:
            // All the registers for this channel.
            return true;
        default:
            // Wave pattern memory location.
            return addr >= kWavePatternRAMStart && addr < kWavePatternRAMEnd;
    }
}

void WaveChannel::writeByte(
    const unsigned short addr,
    const unsigned char value
)
{
    if ( addr == kNR30 ) {
        _rOnOff.write(value);
    } else if ( addr == kNR32 ) {
        _rVolume.write(value);
    } else if ( addr >= kWavePatternRAMStart && addr < kWavePatternRAMEnd ) {
        _wavePatternPtr[ addr ] = value;
    } else if ( Frequency::writeByte( addr, value ) ) {
        if ( addr == kNR34 && _rFrequencyHiPlayback.bits.initialize ) {
            _currentSample.reset();
        }
    }
}

void WaveChannel::emulate(int64_t currentCycle)
{
    // If the frequency clock hasn't overflowed
    if ( !Frequency::emulate() ) {
        return;
    }


    _currentSample.increment();

    const int position = _currentSample.count() / 2;
    const int nibble = _currentSample.count() % 2;

    char sample;
    if (_rOnOff.bits.isOn) {
        if (nibble == 0) {
            sample = (_wavePattern[position] >> 4) - 8;
        } else {
            sample = (_wavePattern[position] & 0xF) - 8;
        }
        sample >>= _rVolume.bits.volumeShift();
    } else {
        sample = 0;
    }

    insertEvent(
        currentCycle,
        sample
    );
}

}
