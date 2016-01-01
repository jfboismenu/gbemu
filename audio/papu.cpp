#include <audio/papu.h>
#include <cpu/registers.h>
#include <common/common.h>
#include <base/logger.h>
#include <base/clock.h>
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

PAPU::PAPU( const Clock& clock ) : 
    _clock( clock ),
    _squareWaveChannel1( clock, _mutex, kNR10, kNR11, kNR12, kNR13, kNR14 ),
    _squareWaveChannel2( clock, _mutex, 0, kNR21, kNR22, kNR23, kNR24 ),
    _waveChannel( clock, _mutex ),
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

void PAPU::renderAudioInternal(void* output, const unsigned long frameCount, const int rate)
{
    _rate = rate;
    float realTime(float(_currentPlaybackTime)/_rate);

    // Update sound event queue.
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _squareWaveChannel1.updateEventsQueue(realTime);
        _squareWaveChannel2.updateEventsQueue(realTime);
        _waveChannel.updateEventsQueue(realTime);
    }

    // Render audio to the output buffer.
    _squareWaveChannel1.renderAudio(output, frameCount, rate, realTime);
    _squareWaveChannel2.renderAudio(output, frameCount, rate, realTime);
    _waveChannel.renderAudio(output, frameCount, rate, realTime);

    _currentPlaybackTime += frameCount;
}

}
