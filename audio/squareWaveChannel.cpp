#include <audio/squareWaveChannel.h>
#include <base/cyclicCounter.imp.h>
#include <base/clock.h>
#include <base/logger.h>

namespace gbemu {

SquareWaveChannel::SquareWaveChannel(
    const Clock& clock,
    std::mutex& mutex,
    unsigned short frequencyShiftRegisterAddr,
    unsigned short soundLengthRegisterAddr,
    unsigned short evenloppeRegisterAddr,
    unsigned short frequencyLowRegisterAddr,
    unsigned short frequencyHiRegisterAddr
) :
    ChannelBase(clock, mutex),
    _frequency_timer(0, 131000),
    _current_duty_step(0),
    _frequencySweepRegisterAddr(frequencyShiftRegisterAddr),
    _soundLengthRegisterAddr(soundLengthRegisterAddr),
    _evenloppeRegisterAddr(evenloppeRegisterAddr),
    _frequencyLowRegisterAddr(frequencyLowRegisterAddr),
    _frequencyHiRegisterAddr(frequencyHiRegisterAddr)
{}

bool SquareWaveChannel::contains(unsigned short addr) const
{
    return addr == _frequencySweepRegisterAddr ||
        addr == _soundLengthRegisterAddr ||
        addr == _evenloppeRegisterAddr ||
        addr == _frequencyLowRegisterAddr ||
        addr == _frequencyHiRegisterAddr;
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

        _duty = _rLengthDuty.bits.wavePatternDuty();
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

        _frequency_period = (2048 - getGbNote()) * 4;
        // JFX_LOG("-----NR13-ff13-----");
        // JFX_LOG("Frequency lo: " << (int)_rFrequencyLo.bits.freqLo);
    }
    else if ( addr == _frequencyHiRegisterAddr ) {
        _rFrequencyHiPlayback.write(value);
        JFX_LOG("-----NR14-" << std::hex << _frequencyHiRegisterAddr << std::dec << "-----");
        JFX_LOG("Frequency hi : " << (int)_rFrequencyHiPlayback.bits.freqHi);
        JFX_LOG("Consecutive  : " << ( _rFrequencyHiPlayback.bits.isLooping() ? "loop" : "play until NR11-length expires" ));
        JFX_LOG("Initialize?  : " << ( _rFrequencyHiPlayback.bits.initialize == 1 ));

        _frequency_period = (2048 - getGbNote()) * 4;

        if ( _rFrequencyHiPlayback.bits.initialize ) {
            // FIXME: Enable channel bit.
            // FIXME: Set the sound length counter.
            // Reload period counter with frequency period.
            _frequency_timer = CyclicCounter(_frequency_period - 1, _frequency_period);
            // Don't reset the step counter!!
            // FIXME: Volume envelope timer is reloaded with period.
            // Channel volume is reloaded from NR12.
            _volume = _rEnveloppe.bits.initialVolume;
            // FIXME: Noise channel's LFSR bits are all set to 1.
            // FIXME: Wave channel's position is set to 0 but sample buffer is NOT refilled.
            // FIXME: Square 1's sweep does several things (see frequency sweep).
        }
    }
}

void SquareWaveChannel::emulate(int nbCycles)
{
    // Decrement the period counter by the number of cycles that need to be emulated.
    // For each underflow, increment by one the step counter.
    for (int i = 0; i < nbCycles; ++i) {
        if (_frequency_timer.getCycleLength() == 0) {
            break;
        }
        const bool changed{_frequency_timer.decrement() != 0};
        if (!changed) {
            continue;
        }
        _frequency_timer = CyclicCounter(_frequency_period - 1, _frequency_period);
        _current_duty_step.increment();
        //std::cout << _current_duty_step.count() << " " << _duty << std::endl;
        insertEvent(
            _clock.getTimeInCycles() - (nbCycles - i + 1),
            _current_duty_step < _duty ? -_volume : _volume
        );
    }
}

short SquareWaveChannel::getGbNote() const
{
    return _rFrequencyLo.bits.freqLo | ( _rFrequencyHiPlayback.bits.freqHi << 8 );
}

/*
unsigned char SquareWaveChannelState::getVolumeAt(float currentTime) const
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

char SquareWaveChannelState::computeSample(
    float frameTimeInSeconds
) const
{
    const char sample = (getPositionInsideWaveform(frameTimeInSeconds) < waveDuty) ? 1 : -1;
    return sample * getVolumeAt(frameTimeInSeconds);
}
*/

}
