#include "papu.h"
#include "registers.h"
#include "common.h"
#include "logger.h"
#include <iostream>

namespace gbemu {

SoundChannel::SoundChannel() : _samples( ( 4 * 1024 * 1024 / 32 ) / 16, 0 ), _accumulator( 0 )
{
}

void SoundChannel::writeSample( int time, short sample )
{
    time %= _samples.size();
    _accumulator += sample;
    // If this is the last sample to accumulate
    if ( ( time & 0xF ) == 0xF ) {
        // Divide by 16
        _accumulator >>= 4;
        // Write the byte in the right slot
        _samples[ static_cast< size_t >( time >> 4 ) ] =
            static_cast< short >( _accumulator );
        // Reset the accumulator
        _accumulator = 0;
    }
}

const short* SoundChannel::getFirstHalf() const
{
    return &_samples[0];
}

const short* SoundChannel::getSecondHalf() const
{
    return &_samples[ getHalfSize() ];
}

size_t SoundChannel::getHalfSize() const
{
    return _samples.size() / 2;
}

size_t SoundChannel::getSize() const
{
    return _samples.size();
}

Counter::Counter( int length ) : _length( length ), _count( 0 )
{

}

Counter& Counter::operator++()
{
    ++_count;
    if ( _count == _length ) {
        _overflowed = true;
        _count = 0;
    }

    return *this;
}

bool Counter::hasOverflowed() const
{
    return _overflowed;
}

PAPU::PAPU( const int& clock ) : _clock( clock ), _squareWaveChannel( clock )
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

void PAPU::registerSoundReadyCb( SoundReadyCb cb, void* clientData )
{
    _soundReadyCb = cb;
    _clientData = clientData;
}

void PAPU::emulate( int nbCycles )
{
    for ( int i = 0; i < nbCycles; ++i ) {
        _squareWaveChannel.emulate( 1 );
    }
}

const SoundChannel& PAPU::getSoundMix() const
{
    return _squareWaveChannel.getSoundChannel();
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
        _squareWaveChannel.writeByte( addr, value );
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

PAPU::SquareWaveChannel::SquareWaveChannel( const int& clock ) :
    _clock( clock ),
    _soundLength( 0 )
{}

const SoundChannel& PAPU::SquareWaveChannel::getSoundChannel() const
{
    return _channel;
}

void PAPU::SquareWaveChannel::emulate( const int nbCycles )
{
    static const int dutyCycle[] = { 1, 2, 4, 6 };
    for ( int i = 0; i < nbCycles; ++i ) {
        if ( isMuted() ) {
            _channel.writeSample( _clock + i, 0 );
            continue;
        }
        if ( _timeBeforeNextPhase == 0 ) {
            _phase = ( _phase + 1 ) * 8;
        }
        // else if ( _phase < dutyCycle[ _nr11.bits.wavePatternDuty ] ) {
        //     _channel.writeSample( _clock + i, -10000 );
        // }
        else {
            _channel.writeSample( _clock + i, 10000 );
        }
        --_timeBeforeNextPhase;
        --_soundLength;
    }
}

bool PAPU::SquareWaveChannel::isMuted() const
{
    return _soundLength == 0;
}


void PAPU::SquareWaveChannel::writeByte(
    const unsigned short addr,
    const unsigned char value
)
{
    if ( addr == kNR11 ) {
        _nr11.write( value );
        JFX_LOG("-----NR11-ff11-----");
        JFX_LOG("Wave pattern duty            : " << _nr11.bits.getWaveDutyPercentage());
        JFX_LOG("Length counter load register : " << (int)_nr11.bits.getSoundLength());
    }
    else if ( addr == kNR12 ) {
        _nr12.write( value );
        JFX_LOG("-----NR12-ff12-----");
        JFX_LOG("Initial channel volume       : " << (int)_nr12.bits.initialVolume);
        JFX_LOG("Volume sweep direction       : " << ( _nr12.bits.isAmplifying() ? "up" : "down" ));
        JFX_LOG("Length of each step          : " << (int)_nr12.bits.sweepLength);
    }
    else if ( addr == kNR13 ) {
        _nr13.write( value );
        JFX_LOG("-----NR13-ff13-----");
        JFX_LOG("Frequency lo: " << (int)_nr13.bits._freqLo);
    }
    else if ( addr == kNR14 ) {
        _nr14.write( value );
        JFX_LOG("-----NR14-ff14-----");
        JFX_LOG("Frequency hi : " << (int)_nr14.bits._freqHi);
        JFX_LOG("Consecutive  : " << ( _nr14.bits._consecutive == 0 ? "loop" : "play until NR21-length expires" ));
        JFX_LOG("Initialize?  : " << ( _nr14.bits._initialize == 1 ));
        JFX_LOG("Period       : " << _periodOneEight);

        if ( _nr14.bits._initialize ) {
            _timeBeforeNextPhase = _periodOneEight = _nr13.bits._freqLo | ( _nr14.bits._freqHi << 8 );
            _soundLength = ( 64 - _nr11.bits.getSoundLength() ) * 4 * 1024 * 1024 / 256;
            _phase = 0;
        }
    }
}

}
