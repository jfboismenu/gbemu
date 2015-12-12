#include "papu.h"
#include "registers.h"
#include "common.h"
#include "logger.h"
#include "clock.h"
#include "mutex.h"
#include <iostream>

namespace {
    float gbNoteToFrequency(const int gbNote)
    {
        return 131072.f / (2048 - gbNote);
    }

    const int PLAY_FOREVER = 0x7fffffff;

    gbemu::Mutex mutex;
}

namespace gbemu {

CyclicCounter::CyclicCounter(const int cycleLength) :
    _cycleLength(cycleLength),
    _count(0)
{}

CyclicCounter& CyclicCounter::operator++()
{
    _count = (_count + 1) % _cycleLength;
    return *this;
}

CyclicCounter CyclicCounter::operator+(int i) const
{
    CyclicCounter counter(*this);
    counter._count = (counter._count + i) % _cycleLength;
    return counter;
}

CyclicCounter CyclicCounter::operator-(int i) const
{
    CyclicCounter counter(*this);
    counter._count = (counter._count - i) % _cycleLength;
    return counter;
}

bool CyclicCounter::operator!=(const CyclicCounter& that) const
{
    return _count != that._count;
}

bool CyclicCounter::operator==(const CyclicCounter& that) const
{
    return _count == that._count;
}

CyclicCounter::operator int() const
{
    return _count;
}

void PAPU::renderAudio(void* output, const unsigned long frameCount, const int rate, void* userData)
{
    memset(output, 0, frameCount);
    reinterpret_cast<PAPU*>(userData)->renderAudioInternal(output, frameCount, rate);
}

PAPU::PAPU( const Clock& clock ) : 
    _clock( clock ),
    _squareWaveChannel1( clock, kNR11, kNR12, kNR13, kNR14 ),
    _squareWaveChannel2( clock, kNR21, kNR22, kNR23, kNR24 )
{
/*
    mr(kNR10) = 0x80;
    mr(kNR11) = 0xBF;
    mr(kNR12) = 0xF3;
    mr(kNR14) = 0xBF;
    mr(kNR21) = 0x3F;
    mr(kNR22) = 0x00;
    mr(kNR24) = 0xBF;
    mr(kNR30) = 0x7F;
    mr(kNR31) = 0xFF;
    mr(kNR32) = 0x9F;
    mr(kNR33) = 0xBF;
    mr(kNR41) = 0xFF;
    mr(kNR42) = 0x00;
    mr(kNR43) = 0x00;
    mr(kNR30) = 0xBF;
    mr(kNR50) = 0x77;
    mr(kNR51) = 0xF3;
    mr(kNR52) = 0xF1;
*/
}

void PAPU::writeByte(
    const unsigned short addr,
    const unsigned char value
)
{
    // When _nr52 all sound on bit is set to 0, we can't write to most registers
    if ( !isRegisterAvailable( addr ) ) {
        return;
    }
    if ( addr == kNR52 ) {
        _nr52.write( value );
        // JFX_LOG("All Sound Flag: " << ( _nr52.bits._allSoundOn ? "on" : "off" ));
    }
    else if ( addr == 0xFF24 ) {
        _nr50.write( value );
        // JFX_LOG("-----NR50-ff24-----");
        // JFX_LOG("Output Vin to left               :" << ( _nr50.bits.outputVinToLeftTerminal == 1 ));
        // JFX_LOG("left Main output level (volume)  :" << (int)_nr50.bits.leftMainOutputLevel);
        // JFX_LOG("Output Vin to left               :" << ( _nr50.bits.outputVinToRightTerminal == 1 ));
        // JFX_LOG("right Main output level (volume) :" << (int)( _nr50.bits.leftMainOutputLevel ));
    }
    else if ( addr == 0xFF25 ) {
        _nr51.write( value );
        // JFX_LOG("-----NR51-ff25-----");
        // JFX_LOG("Channel 1 to right : " << ( _nr51.bits.channel1Right == 1 ));
        // JFX_LOG("Channel 2 to right : " << ( _nr51.bits.channel2Right == 1 ));
        // JFX_LOG("Channel 3 to right : " << ( _nr51.bits.channel3Right == 1 ));
        // JFX_LOG("Channel 4 to right : " << ( _nr51.bits.channel4Right == 1 ));
        // JFX_LOG("Channel 1 to left  : " << ( _nr51.bits.channel1Left == 1 ));
        // JFX_LOG("Channel 2 to left  : " << ( _nr51.bits.channel2Left == 1 ));
        // JFX_LOG("Channel 3 to left  : " << ( _nr51.bits.channel3Left ==  1 ));
        // JFX_LOG("Channel 4 to left  : " << ( _nr51.bits.channel4Left == 1 ));
    }
    else if ( addr == kNR11 || addr == kNR12 || addr == kNR13 || addr == kNR14 ) {
        _squareWaveChannel1.writeByte( addr, value );
    }
    else if ( addr == kNR21 || addr == kNR22 || addr == kNR23 || addr == kNR24 ) {
        _squareWaveChannel2.writeByte( addr, value );
    }
    else {
//        JFX_LOG("Untracked write at " << std::hex << addr);
    }
}

unsigned char PAPU::readByte( unsigned short addr ) const
{
    if ( !isRegisterAvailable( addr ) ) {
        return 0;
    }
    if ( addr == kNR52 )
    {
        // When the sound if off, we can only see the state of the unused bits
        if ( _nr52.bits._allSoundOn == 0 ) {
            return _nr52.bits.readUnused();
        }
        else {
            return _nr52.read();
        }
    }
    else {
//        JFX_LOG("Untracked read at " << std::hex << addr);
    }
    return 0;
}

bool PAPU::isRegisterAvailable( const unsigned short addr ) const
{
    // NR52 is always available. However, if it's off and we are accessing a non wave-pattern address, we can't access them.
    return addr == kNR52 || !( ( _nr52.bits._allSoundOn == 0 ) && ( 0xFF10 <= addr ) && ( addr <= 0xFF2F ) );
}

void PAPU::renderAudioInternal(void* output, const unsigned long frameCount, const int rate)
{
    static unsigned long currentTime(0);
    float realTime(float(currentTime)/rate);
    currentTime += frameCount;

    _squareWaveChannel1.renderAudio(output, frameCount, rate, realTime);
    _squareWaveChannel2.renderAudio(output, frameCount, rate, realTime);
}

PAPU::SquareWaveChannel::SquareWaveChannel(
    const Clock& clock,
    unsigned short soundLengthRegisterAddr,
    unsigned short evenloppeRegisterAddr,
    unsigned short frequencyLowRegisterAddr,
    unsigned short frequencyHiRegisterAddr
) :
    _clock( clock ),
    _playbackIntervalStart(0),
    _playbackIntervalEnd(0),
    _firstEvent(_soundEvents.size()),
    _lastEvent(_soundEvents.size()),
    _soundLengthRegisterAddr(soundLengthRegisterAddr),
    _evenloppeRegisterAddr(evenloppeRegisterAddr),
    _frequencyLowRegisterAddr(frequencyLowRegisterAddr),
    _frequencyHiRegisterAddr(frequencyHiRegisterAddr)
{}

char PAPU::SquareWaveChannel::computeSample(
    float frequency,
    float timeSinceNoteStart,
    float duty
) const
{
    const float cycleLength = 1 / frequency;
    // Compute how many times the sound has played, including fractions.
    const float howManyTimes = timeSinceNoteStart / cycleLength;
    // Compute how many times the sound has completely played.
    const float howManyTimesCompleted = int(howManyTimes);
    // Compute how far we are in the current cycle.
    const float howManyInCurrent = howManyTimes - howManyTimesCompleted;
    // SINE
    // return sin(pos_in_cycle * 2 * M_PI) * 2;
    // SQUARE
    return howManyInCurrent < duty ? 1 : -1;
}

void PAPU::SquareWaveChannel::renderAudio(void* raw_output, const unsigned long frameCount, const int rate, const float realTime)
{
    char* output = reinterpret_cast<char*>(raw_output);
    // Update the interval of sound we're about to produce
    updatePlaybackInterval();

    // Convert the start and end to seconds.
    const float startInSeconds = float(_playbackIntervalStart) / _clock.getRate();
    const float endInSeconds = float(_playbackIntervalEnd) / _clock.getRate();
    const float lengthInSeconds = endInSeconds - startInSeconds;
    updateEventsQueue(startInSeconds);

    // Queue is empty, do not play anything.
    if (_firstEvent == _lastEvent) {
        return;
    }

    CyclicCounter currentEvent = _firstEvent;

    for (unsigned long i = 0 ; i < frameCount; ++i) {
        // Peneration of the loop.
        const float depth = float(i) / frameCount;
        // Compute the current cpu cycle.
        const int currentCpuCycle = depth * (_playbackIntervalEnd - _playbackIntervalStart) + _playbackIntervalStart;
        // Compute the real time in seconds this sample will represent.
        const float frameTimeInSeconds = realTime + (float(i) / rate);

        // If there are no more events to process, play nothing.
        if (currentEvent == _lastEvent) {
            continue;
        } else if (currentCpuCycle < _soundEvents[currentEvent].waveStart) {
            // If we still haven't reached the first note, play nothing.
            continue;
        }
        if (_soundEvents[currentEvent].isPlaying) {
            const float timeSinceEventStart = (frameTimeInSeconds - _soundEvents[currentEvent].waveStartInSeconds);
            output[i] += computeSample(_soundEvents[currentEvent].waveFrequency, timeSinceEventStart, _soundEvents[currentEvent].waveDuty) * 4;
        }
    }
}

void PAPU::SquareWaveChannel::updatePlaybackInterval()
{
    // Take a note of the current emulator time.
    const int64_t currentTime = _clock.getTimeInCycles();

    // Compare that time with the end of the last render window.
    // If the end time has changed, emulation has move forward, so we need to update
    // the audio queue.
    if (_playbackIntervalEnd != currentTime) {
        // take the end time and make that the new start time
        // update the end time with the new time we've found.
        _playbackIntervalStart = _playbackIntervalEnd;
        _playbackIntervalEnd = currentTime;
    }
}

void PAPU::SquareWaveChannel::updateEventsQueue(const float audioFrameStartInSeconds)
{
    MutexGuard g(mutex);
    for (CyclicCounter i = _firstEvent; i != _lastEvent ; ++i) {

        // If sound is looping and the end of that audio event is before this audio frame.
        if (_soundEvents[i].isLooping) {
             // If this is not the last event, the next event might silence this one?
            if (i + 1 != _lastEvent) {
                // if that next event starts before the current audio
                if (_soundEvents[i + 1].waveStart < _playbackIntervalStart) {
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
}

void PAPU::SquareWaveChannel::writeByte(
    const unsigned short addr,
    const unsigned char value
)
{
    if ( addr == _soundLengthRegisterAddr ) {
        _nr11.write( value );
        JFX_LOG("-----NR11-ff11-----");
        JFX_LOG("Wave pattern duty            : " << _nr11.bits.getWaveDutyPercentage());
        JFX_LOG("Length counter load register : " << (int)_nr11.bits.getSoundLength());
    }
    else if ( addr == _evenloppeRegisterAddr ) {
        _nr12.write( value );
        JFX_LOG("-----NR12-ff12-----");
        JFX_LOG("Initial channel volume       : " << (int)_nr12.bits.initialVolume);
        JFX_LOG("Volume sweep direction       : " << ( _nr12.bits.isAmplifying() ? "up" : "down" ));
        JFX_LOG("Length of each step          : " << (int)_nr12.bits.sweepLength);
    }
    else if ( addr == _frequencyLowRegisterAddr ) {
        _nr13.write( value );
        JFX_LOG("-----NR13-ff13-----");
        JFX_LOG("Frequency lo: " << (int)_nr13.bits._freqLo);
    }
    else if ( addr == _frequencyHiRegisterAddr ) {
        _nr14.write( value );
        JFX_LOG("-----NR14-ff14-----");
        JFX_LOG("Frequency hi : " << (int)_nr14.bits._freqHi);
        JFX_LOG("Consecutive  : " << ( _nr14.bits.isLooping() ? "loop" : "play until NR11-length expires" ));
        JFX_LOG("Initialize?  : " << ( _nr14.bits._initialize == 1 ));

        // Push a new sound event in a thread-safe manner.
        MutexGuard g(mutex);
        const int gbNote = getGbNote();
        JFX_CMP_ASSERT(2048 - gbNote, >, 0);
        _soundEvents[_lastEvent] = SoundEvent(
            _nr14.bits._initialize == 1,
            _nr14.bits.isLooping(),
            _clock.getTimeInCycles(),
            _clock.getTimeInSeconds(),
            _nr11.bits.getSoundLength(),
            gbNoteToFrequency(gbNote),
            _nr11.bits.getWaveDutyPercentage()
        );
        ++_lastEvent;
    }
}

short PAPU::SquareWaveChannel::getGbNote() const
{
    return _nr13.bits._freqLo | ( _nr14.bits._freqHi << 8 );
}

PAPU::SquareWaveChannel::SoundEvent::SoundEvent(
    bool ip,
    bool il,
    int64_t ws,
    float wsis,
    float wlis,
    int wf,
    float d
) : isPlaying(ip),
    isLooping(il),
    waveStart(ws),
    waveStartInSeconds(wsis),
    waveLengthInSeconds(wlis),
    waveFrequency(wf),
    waveDuty(d)
{}

float PAPU::SquareWaveChannel::SoundEvent::waveEndInSeconds() const
{
    return waveStartInSeconds + waveLengthInSeconds;
}

}
