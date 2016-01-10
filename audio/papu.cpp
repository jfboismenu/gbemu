#include <audio/papu.h>
#include <cpu/registers.h>
#include <common/common.h>
#include <base/logger.h>
#include <base/clock.imp.h>
#include <iostream>

namespace gbemu {

void PAPU::renderAudio(void* output, const unsigned long frameCount, const int rate, void* userData)
{
    memset(output, 0, frameCount);
    reinterpret_cast<PAPU*>(userData)->renderAudioInternal(output, frameCount, rate);
}

bool PAPU::contains( unsigned short addr ) const
{
    return kSoundRegistersStart <= addr && addr <= kSoundRegistersEnd;
}

PAPU::PAPU( const CPUClock& clock ) :
    _clocks( clock ),
    _squareWaveChannel1( _clocks, _mutex, kNR10, kNR11, kNR12, kNR13, kNR14 ),
    _squareWaveChannel2( _clocks, _mutex, 0, kNR21, kNR22, kNR23, kNR24 ),
    _waveChannel( _clocks, _mutex ),
    _initializing( true )
{

    // writeByte(kNR10, 0x80);
    // writeByte(kNR11, 0xBF);
    // writeByte(kNR12, 0xF3);
    // writeByte(kNR14, 0xBF);
    // writeByte(kNR21, 0x3F);
    // writeByte(kNR22, 0x00);
    // writeByte(kNR24, 0xBF);
    // writeByte(kNR30, 0x7F);
    // writeByte(kNR31, 0xFF);
    // writeByte(kNR32, 0x9F);
    // writeByte(kNR33, 0xBF);
    // writeByte(kNR41, 0xFF);
    // writeByte(kNR42, 0x00);
    // writeByte(kNR43, 0x00);
    // writeByte(kNR30, 0xBF);
    // writeByte(kNR50, 0x77);
    // writeByte(kNR51, 0xF3);
    // writeByte(kNR52, 0xF1);
    _initializing = false;
}

void PAPU::emulate(int nbCycles)
{
    const int64_t endTick = _clocks.cpu.getTimeInCycles();
    for (int64_t i = endTick - nbCycles; i < endTick; ++i) {
        if (_clocks.hz512Clock.increment()) {
            if (_clocks.lengthClock.increment()) {
                // FIXME: Implement.
            }
            if (_clocks.volumeEnvelopeClock.increment()) {
                _squareWaveChannel1.clockEnvelope();
                _squareWaveChannel2.clockEnvelope();
            }
            if (_clocks.sweepClock.increment()) {
                // FIXME: Implement.
            }
        }
        _squareWaveChannel1.emulate(i);
        _squareWaveChannel2.emulate(i);
        _waveChannel.emulate(i);
    }
}

void PAPU::writeByte(
    const unsigned short addr,
    const unsigned char value
)
{
    // When _nr52 all sound on bit is set to 0, we can't write to most registers
    if ( !isRegisterAvailable( addr ) ) {
        // std::cout << "not available " << std::hex << addr << std::dec << std::endl;
        return;
    }
    if ( addr == kNR52 ) {
        _nr52.write( value );
        // JFX_LOG("All Sound Flag: " << ( _nr52.bits._allSoundOn ? "on" : "off" ));
    }
    else if ( addr == kNR50 ) {
        _nr50.write( value );
        // JFX_LOG("-----NR50-ff24-----");
        // JFX_LOG("Output Vin to left               :" << ( _nr50.bits.outputVinToLeftTerminal == 1 ));
        // JFX_LOG("left Main output level (volume)  :" << (int)_nr50.bits.leftMainOutputLevel);
        // JFX_LOG("Output Vin to left               :" << ( _nr50.bits.outputVinToRightTerminal == 1 ));
        // JFX_LOG("right Main output level (volume) :" << (int)( _nr50.bits.leftMainOutputLevel ));
    }
    else if ( addr == kNR51 ) {
        _nr51.write( value );
        _squareWaveChannel1.setMix(_nr51.bits.getMix(1));
        _squareWaveChannel2.setMix(_nr51.bits.getMix(2));
        _waveChannel.setMix(_nr51.bits.getMix(3));
        //_noiseChannel.setMix(_nr51.bits.getMix(4));
    }
    else if ( _squareWaveChannel1.contains( addr ) ) {
        _squareWaveChannel1.writeByte( addr, value );
    }
    else if ( _squareWaveChannel2.contains( addr ) ) {
        _squareWaveChannel2.writeByte( addr, value );
    }
    else if ( _waveChannel.contains( addr ) ) {
        _waveChannel.writeByte( addr, value );
    } else {
        std::cout << "Untracked PAPU write at " << std::hex << addr << std::dec << std::endl;
    }
}

unsigned char PAPU::readByte( unsigned short addr ) const
{
    if ( !isRegisterAvailable( addr ) ) {
        // std::cout << "not available " << std::hex << addr << std::dec << std::endl;
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
    } else if (addr == kNR51) {
        return _nr51.read();
    }

    else {
        JFX_LOG("Untracked read at " << std::hex << addr);
    }
    return 0;
}

float PAPU::getCurrentPlaybackTime() const
{
    return float(_currentPlaybackTime) / _rate;
}

bool PAPU::isRegisterAvailable( const unsigned short addr ) const
{
    //  However, if it's off and we are accessing a non wave-pattern address, we can't access them.
    return _initializing ||
        addr == kNR52 || // NR52 is always available.
        _nr52.bits._allSoundOn == 1 || // Register is available if sound chip is on.
        // Only wave pattern is accessible when audio register is on.
        (kWavePatternRAMStart <= addr && addr < kWavePatternRAMEnd);
}

void PAPU::renderAudioInternal(void* output, unsigned long frameCount, const int rate)
{
    _rate = rate;
    const unsigned long sampleCount{frameCount};
    //std::cout << "audio in proc : " << audioTimeInProcTime << std::endl;

    // Update sound event queue.
    _squareWaveChannel1.updateEventsQueue(_currentPlaybackTime);
    _squareWaveChannel2.updateEventsQueue(_currentPlaybackTime);
    _waveChannel.updateEventsQueue(_currentPlaybackTime);

    // Render audio to the output buffer.
    _squareWaveChannel1.renderAudio(output, sampleCount, rate, _currentPlaybackTime);
    _squareWaveChannel2.renderAudio(output, sampleCount, rate, _currentPlaybackTime);
    _waveChannel.renderAudio(output, sampleCount, rate, _currentPlaybackTime);

    // There are two frames per sample, so divide it by two.
    _currentPlaybackTime += sampleCount;
}

}
