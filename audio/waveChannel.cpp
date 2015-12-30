#include <audio/waveChannel.h>
#include <cpu/registers.h>
#include <audio/channelBase.imp.h>
#include <base/cyclicCounter.imp.h>
#include <base/clock.h>
#include <base/logger.h>

namespace {
    float gbNoteToFrequency(const int gbNote)
    {
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

char WaveChannel::computeSample(
    float frequency,
    float timeSinceNoteStart,
    const WavePatternSamples& samples,
    const char volumeShift
) const
{
    const float cycleLength = 1 / frequency;
    // Compute how many times the sound has played, including fractions.
    const float howManyTimes = timeSinceNoteStart / cycleLength;
    // Compute how many times the sound has completely played.
    const float howManyTimesCompleted = int(howManyTimes);
    // Compute how far we are in the current cycle.
    const float howManyInCurrent = (howManyTimes - howManyTimesCompleted);

    const int sampleIdx = howManyInCurrent * 32;
    const int samplePos = sampleIdx / 2;
    const int sampleNibble = sampleIdx % 2;

    const unsigned char nibble = (sampleNibble == 0) ? (samples[samplePos] >> 4) : (samples[samplePos] & 0xf);

    return ( static_cast< char >( nibble ) - 8 )>> volumeShift;
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
        const float timeSinceEventStart = (frameTimeInSeconds - _soundEvents[currentEvent].waveStartInSeconds);
        output[ i ] += computeSample(
            _soundEvents[currentEvent].waveFrequency,
            timeSinceEventStart,
            _soundEvents[currentEvent].samples,
            _soundEvents[currentEvent].getVolumeShift()
        );
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
        const int gbNote = getGbNote();
        const float frequency(gbNoteToFrequency(gbNote));
        // JFX_LOG("-----NR34-ff1e-----");
        // JFX_LOG("On/Off : " << (_rOnOff.bits.isOn() ? "On": "Off"));
        // JFX_LOG("Sound length : " << _rSoundLength.bits.getSoundLength() << " seconds");
        // JFX_LOG("Volume shift : " << (int)_rVolume.bits.getVolumeShift());
        // JFX_LOG("Frequency    : " << frequency);
        // JFX_LOG("Consecutive  : " << ( _rFrequencyHiPlayback.bits.isLooping() ? "loop" : "play until NR11-length expires" ));
        // JFX_LOG("Initialize?  : " << ( _rFrequencyHiPlayback.bits.initialize == 1 ));

        // Push a new sound event in a thread-safe manner.
        std::lock_guard<std::mutex> lock(_mutex);

        JFX_CMP_ASSERT(2048 - gbNote, >, 0);
        _soundEvents[_lastEvent] = WaveSoundEvent(
            _rFrequencyHiPlayback.bits.isLooping(),
            frequency,
            _clock.getTimeInCycles(),
            _clock.getTimeInSeconds(),
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
    int wf,
    int64_t ws,
    float wsis,
    float wlis,
    char vs,
    const WavePatternSamples& s
) : SoundEventBase(il, wf, ws, wsis, wlis),
    _volumeShift(vs),
    samples(s)
{}

char WaveSoundEvent::getVolumeShift() const
{
    return _volumeShift;
}

template void ChannelBase<WaveSoundEvent>::updateEventsQueue(
    const float audioFrameStartInSeconds
);

}
