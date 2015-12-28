#include <audio/squareWaveChannel.h>
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

SquareWaveChannel::SquareWaveChannel(
    const Clock& clock,
    unsigned short frequencyShiftRegisterAddr,
    unsigned short soundLengthRegisterAddr,
    unsigned short evenloppeRegisterAddr,
    unsigned short frequencyLowRegisterAddr,
    unsigned short frequencyHiRegisterAddr
) :
    ChannelBase(clock),
    _frequencySweepRegisterAddr(frequencyShiftRegisterAddr),
    _soundLengthRegisterAddr(soundLengthRegisterAddr),
    _evenloppeRegisterAddr(evenloppeRegisterAddr),
    _frequencyLowRegisterAddr(frequencyLowRegisterAddr),
    _frequencyHiRegisterAddr(frequencyHiRegisterAddr)
{}

char SquareWaveChannel::computeSample(
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

void SquareWaveChannel::renderAudio(void* raw_output, const unsigned long frameCount, const int rate, const float realTime)
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
            output[i] += computeSample(
                _soundEvents[currentEvent].waveFrequency, 
                timeSinceEventStart,
                _soundEvents[currentEvent].waveDuty
            ) * _soundEvents[currentEvent].getVolumeAt(frameTimeInSeconds);
        }
    }
}

void SquareWaveChannel::writeByte(
    const unsigned short addr,
    const unsigned char value
)
{
    if ( addr == _frequencySweepRegisterAddr) {
        _rFrequencySweep.write(value);
        // JFX_LOG("-----NR10-ff10-----")
        // JFX_LOG("Increasing: " << _rFrequencySweep.bits.isIncreasing());
        // JFX_LOG("Number of shifts: " << _rFrequencySweep.bits.getNbSweepShifts());
        // JFX_LOG("Shift length: " << _rFrequencySweep.bits.getSweepLength() << " seconds");
    }
    else if ( addr == _soundLengthRegisterAddr ) {
        _rLengthDuty.write( value );
        // JFX_LOG("-----NR11-ff11-----");
        // JFX_LOG("Wave pattern duty            : " << _rLengthDuty.bits.getWaveDutyPercentage());
        // JFX_LOG("Length counter load register : " << (int)_rLengthDuty.bits.getSoundLength());
    }
    else if ( addr == _evenloppeRegisterAddr ) {
        _rEnveloppe.write( value );
        // JFX_LOG("-----NR12-ff12-----");
        // JFX_LOG("Initial channel volume       : " << (int)_rEnveloppe.bits.initialVolume);
        // JFX_LOG("Volume sweep direction       : " << ( _rEnveloppe.bits.isAmplifying() ? "up" : "down" ));
        // JFX_LOG("Length of each step          : " << _rEnveloppe.bits.getSweepLength() << " seconds");
    }
    else if ( addr == _frequencyLowRegisterAddr ) {
        _rFrequencyLo.write( value );
        // JFX_LOG("-----NR13-ff13-----");
        // JFX_LOG("Frequency lo: " << (int)_rFrequencyLo.bits.freqLo);
    }
    else if ( addr == _frequencyHiRegisterAddr ) {
        _rFrequencyHiPlayback.write( value );
        // JFX_LOG("-----NR14-ff14-----");
        // JFX_LOG("Frequency hi : " << (int)_rFrequencyHiPlayback.bits.freqHi);
        // JFX_LOG("Consecutive  : " << ( _rFrequencyHiPlayback.bits.isLooping() ? "loop" : "play until NR11-length expires" ));
        // JFX_LOG("Initialize?  : " << ( _rFrequencyHiPlayback.bits.initialize == 1 ));

        // Push a new sound event in a thread-safe manner.
        std::lock_guard<std::mutex> lock(_mutex);
        const int gbNote = getGbNote();
        JFX_CMP_ASSERT(2048 - gbNote, >, 0);
        _soundEvents[_lastEvent] = SquareWaveSoundEvent(
            _rFrequencyHiPlayback.bits.initialize == 1,
            _rFrequencyHiPlayback.bits.isLooping(),
            gbNoteToFrequency(gbNote),
            _clock.getTimeInCycles(),
            _clock.getTimeInSeconds(),
            _rLengthDuty.bits.getSoundLength(),
            _rLengthDuty.bits.getWaveDutyPercentage(),
            _rEnveloppe.bits.initialVolume,
            _rEnveloppe.bits.isAmplifying(),
            _rEnveloppe.bits.getSweepLength()
        );
        ++_lastEvent;
        JFX_CMP_ASSERT(_firstEvent, !=, _lastEvent);
    }
}

short SquareWaveChannel::getGbNote() const
{
    return _rFrequencyLo.bits.freqLo | ( _rFrequencyHiPlayback.bits.freqHi << 8 );
}

SquareWaveSoundEvent::SquareWaveSoundEvent(
    bool ip,
    bool il,
    int wf,
    int64_t ws,
    float wsis,
    float wlis,
    float d,
    char v,
    bool va,
    float sl
) : SoundEventBase(ip, il, wf, ws, wsis, wlis),
    waveDuty(d),
    waveVolume(v),
    isVolumeAmplifying(va),
    sweepLength(sl)
{}

unsigned char SquareWaveSoundEvent::getVolumeAt(float currentTime) const
{
    // Sweep length is zero, so don't amplify or reduce volume.
    if (sweepLength == 0.f) {
        return waveVolume;
    }

    // Compute how many sweeps happened since the beginning of this wave.
    const int nbSweepsCompleted = (currentTime - waveStartInSeconds) / sweepLength;

    return static_cast<unsigned char>(
        std::min(
            // Update the volume in the right direction, but clamp it between 0 and 15 inclusively.
            std::max(0, isVolumeAmplifying ? waveVolume + nbSweepsCompleted: waveVolume - nbSweepsCompleted),
            15
        )
    );
}

}
