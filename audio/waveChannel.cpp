#include <audio/waveChannel.h>
#include <cpu/registers.h>
#include <audio/channelBase.imp.h>
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
    const Clock& clock
) :
    ChannelBase(clock),
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

        int64_t waveStart;
        float waveStartInSeconds;
        if (_rFrequencyHiPlayback.bits.initialize) {
            waveStart = _clock.getTimeInCycles();
            waveStartInSeconds = _clock.getTimeInSeconds();
        } else {
            waveStart = _soundEvents[_lastEvent - 1].waveStart;
            waveStartInSeconds = _soundEvents[_lastEvent - 1].waveStartInSeconds;
        }

        // Push a new sound event in a thread-safe manner.
        std::lock_guard<std::mutex> lock(_mutex);

        _soundEvents[_lastEvent] = WaveSoundEvent(
            _rFrequencyHiPlayback.bits.isLooping(),
            gbNoteToFrequency(getGbNote()),
            waveStart,
            waveStartInSeconds,
            _rSoundLength.bits.getSoundLength(),
            _rVolume.bits.getVolumeShift(),
            _wavePattern
        );
        _soundEvents[_lastEvent].timeStamp = _clock.getTimeInSeconds();
        ++_lastEvent;
        JFX_CMP_ASSERT(_firstEvent, !=, _lastEvent);
    }
}

WaveSoundEvent::WaveSoundEvent(
    bool il,
    float wf,
    int64_t ws,
    float wsis,
    float wlis,
    char vs,
    const WavePatternSamples& s
) : SoundEventBase(il, wf, ws, wsis, wlis),
    _volumeShift(vs),
    samples(s)
{}

char WaveSoundEvent::computeSample(
    float frameTimeInSeconds
) const
{
    const float timeSinceEventStart = (frameTimeInSeconds - waveStartInSeconds);

    const float cycleLength = 1 / waveFrequency;
    // Compute how many times the sound has played, including fractions.
    const float howManyTimes = timeSinceEventStart / cycleLength;
    // Compute how many times the sound has completely played.
    const float howManyTimesCompleted = int(howManyTimes);
    // Compute how far we are in the current cycle.
    const float howManyInCurrent = (howManyTimes - howManyTimesCompleted);

    const int sampleIdx = howManyInCurrent * 32;
    const int samplePos = sampleIdx / 2;
    const int sampleNibble = sampleIdx % 2;

    const unsigned char nibble = (sampleNibble == 0) ? (samples[samplePos] >> 4) : (samples[samplePos] & 0xf);

    return ( static_cast< char >( nibble ) - 8 ) >> _volumeShift;
}

template class ChannelBase<WaveChannel, WaveSoundEvent>;

}
