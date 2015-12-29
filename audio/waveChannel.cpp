#include <audio/waveChannel.h>
#include <cpu/registers.h>
#include <audio/channelBase.imp.h>
#include <base/cyclicCounter.imp.h>
#include <base/clock.h>
#include <base/logger.h>

namespace {
    float gbNoteToFrequency(const int gbNote)
    {
        return 131072.f / (2048 - gbNote);
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
        JFX_LOG("-----NR30-ff1a-----");
        JFX_LOG("On/Off : " << (_rOnOff.bits.isOn() ? "On": "Off"));
    } else if ( addr == kNR31 ) {
        _rSoundLength.write(value);
        JFX_LOG("-----NR31-ff1b-----");
        JFX_LOG("Sound length : " << _rSoundLength.bits.getSoundLength() << " seconds");
    } else if ( addr == kNR32 ) {
        _rVolume.write(value);
        JFX_LOG("-----NR32-ff1c-----");
        JFX_LOG("Volume shift : " << (int)_rVolume.bits.getVolumeShift());
    } else if ( addr == kNR33 ) {
        JFX_LOG("-----NR33-ff1d-----");
        JFX_LOG("Frequency lo : " << (int)_rFrequencyLo.bits.freqLo);
        _rFrequencyLo.write(value);
    } else if ( addr >= kWavePatternRAMStart && addr < kWavePatternRAMEnd ) {
        JFX_LOG("-----Wave-pattern-ram----");
        _wavePatternPtr[ addr ] = static_cast< char >( value );
        JFX_LOG((int)_wavePatternPtr[ addr ] << " -> " << std::hex << addr);
    } else if ( addr == kNR34 ) {
        _rFrequencyHiPlayback.write( value );
        JFX_LOG("-----NR34-ff1e-----");
        JFX_LOG("Frequency hi : " << (int)_rFrequencyHiPlayback.bits.freqHi);
        JFX_LOG("Consecutive  : " << ( _rFrequencyHiPlayback.bits.isLooping() ? "loop" : "play until NR11-length expires" ));
        JFX_LOG("Initialize?  : " << ( _rFrequencyHiPlayback.bits.initialize == 1 ));

        // Push a new sound event in a thread-safe manner.
        std::lock_guard<std::mutex> lock(_mutex);
        const int gbNote = getGbNote();
        JFX_CMP_ASSERT(2048 - gbNote, >, 0);
        _soundEvents[_lastEvent] = WaveSoundEvent(
            _rFrequencyHiPlayback.bits.initialize == 1,
            _rFrequencyHiPlayback.bits.isLooping(),
            gbNoteToFrequency(gbNote),
            _clock.getTimeInCycles(),
            _clock.getTimeInSeconds(),
            _rSoundLength.bits.getSoundLength(),
            _wavePattern
        );
        ++_lastEvent;
        JFX_CMP_ASSERT(_firstEvent, !=, _lastEvent);
    }
}

WaveSoundEvent::WaveSoundEvent(
    bool ip,
    bool il,
    int wf,
    int64_t ws,
    float wsis,
    float wlis,
    const WavePatternSamples& samples
) : SoundEventBase(ip, il, wf, ws, wsis, wlis),
    _samples(samples)
{}

}
