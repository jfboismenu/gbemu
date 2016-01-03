#include <audio/waveChannel.h>
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
    const Clock& clock,
    std::mutex& mutex
) :
    ChannelBase(clock, mutex),
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

short WaveChannel::getGbNote() const
{
    return _rFrequencyLo.bits.freqLo | ( _rFrequencyHiPlayback.bits.freqHi << 8 );
}

void WaveChannel::writeByte(
    const unsigned short addr,
    const unsigned char value
)
{
    if ( addr == kNR30 ) {
        _rOnOff.write(value);
    } else if ( addr == kNR31 ) {
        _rSoundLength.write(value);
    } else if ( addr == kNR32 ) {
        _rVolume.write(value);
    } else if ( addr == kNR33 ) {
        _rFrequencyLo.write(value);
    } else if ( addr >= kWavePatternRAMStart && addr < kWavePatternRAMEnd ) {
        _wavePatternPtr[ addr ] = value;
    } else if ( addr == kNR34 ) {
        _rFrequencyHiPlayback.write( value );
        // JFX_LOG("-----NR34-ff1e-----");
        // JFX_LOG("On/Off : " << (_rOnOff.bits.isOn() ? "On": "Off"));
        // JFX_LOG("Sound length : " << _rSoundLength.bits.getSoundLength() << " seconds");
        // JFX_LOG("Volume shift : " << (int)_rVolume.bits.getVolumeShift());
        // JFX_LOG("Frequency    : " << frequency);
        // JFX_LOG("Consecutive  : " << ( _rFrequencyHiPlayback.bits.isLooping() ? "loop" : "play until NR11-length expires" ));
        // JFX_LOG("Initialize?  : " << ( _rFrequencyHiPlayback.bits.initialize == 1 ));
    }
}

}
