#include "papu.h"
#include "registers.h"
#include "common.h"
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
{}

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
//        std::cout << "All Sound Flag: " << ( _nr52.bits._allSoundOn ? "on" : "off" ) << std::endl;
    }
    else if ( addr == 0xFF24 ) {
        _nr50.write( value );
//        std::cout << "-----NR50-ff24-----" << std::endl;
//        std::cout << "Output Vin to left               :" << ( _nr50.bits.outputVinToLeftTerminal == 1 ) << std::endl;
//        std::cout << "left Main output level (volume)  :" << (int)_nr50.bits.leftMainOutputLevel << std::endl;
//        std::cout << "Output Vin to left               :" << ( _nr50.bits.outputVinToRightTerminal == 1 ) << std::endl;
//        std::cout << "right Main output level (volume) :" << (int)( _nr50.bits.leftMainOutputLevel ) << std::endl;
    }
    else if ( addr == 0xFF25 ) {
        _nr51.write( value );
//        std::cout << "-----NR51-ff25-----" << std::endl;
//        std::cout << "Channel 1 to right : " << ( _nr51.bits.channel1Right == 1 ) << std::endl;
//        std::cout << "Channel 2 to right : " << ( _nr51.bits.channel2Right == 1 ) << std::endl;
//        std::cout << "Channel 3 to right : " << ( _nr51.bits.channel3Right == 1 ) << std::endl;
//        std::cout << "Channel 4 to right : " << ( _nr51.bits.channel4Right == 1 ) << std::endl;
//        std::cout << "Channel 1 to left  : " << ( _nr51.bits.channel1Left == 1 ) << std::endl;
//        std::cout << "Channel 2 to left  : " << ( _nr51.bits.channel2Left == 1 ) << std::endl;
//        std::cout << "Channel 3 to left  : " << ( _nr51.bits.channel3Left ==  1 ) << std::endl;
//        std::cout << "Channel 4 to left  : " << ( _nr51.bits.channel4Left == 1 ) << std::endl;
    }
    else if ( addr == kNR11 || addr == kNR12 || addr == kNR13 || addr == kNR14 ) {
        _squareWaveChannel.writeByte( addr, value );
    }
    else {
//        std::cout << "Untracked write at " << std::hex << addr << std::endl;
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
//        std::cout << "Untracked read at " << std::hex << addr << std::endl;
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
        else if ( _phase < dutyCycle[ _nr11.bits.wavePatternDuty ] ) {
            _channel.writeSample( _clock + i, -10000 );
        }
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
//        std::cout << "-----NR11-ff11-----" << std::endl;
//        std::cout << "Wave pattern duty            : " << _nr11.bits.getWaveDutyPercentage() << std::endl;
//        std::cout << "Length counter load register : " << (int)_nr11.bits.soundLength << std::endl;
    }
    else if ( addr == kNR12 ) {
        _nr12.write( value );
//        std::cout << "-----NR12-ff12-----" << std::endl;
//        std::cout << "Initial channel volume       : " << (int)_nr12.bits.initialVolume << std::endl;
//        std::cout << "Volume sweep direction       : " << ( _nr12.bits.isAmplify() ? "up" : "down" ) << std::endl;
//        std::cout << "Length of each step          : " << (int)_nr12.bits.sweepLength << std::endl;
    }
    else if ( addr == kNR13 ) {
        _nr13.write( value );
//        std::cout << "-----NR13-ff13-----" << std::endl;
//        std::cout << "Frequency lo: " << (int)_nr13.bits._freqLo << std::endl;
    }
    else if ( addr == kNR14 ) {
        _nr14.write( value );
//        std::cout << "-----NR14-ff14-----" << std::endl;
//        std::cout << "Frequency hi : " << (int)_nr14.bits._freqHi << std::endl;
//        std::cout << "Consecutive  : " << ( _nr14.bits._consecutive == 0 ? "loop" : "play until NR21-length expires" ) << std::endl;
//        std::cout << "Initialize?  : " << ( _nr14.bits._initialize == 1 ) << std::endl;
//        std::cout << "Period       : " << _periodOneEight << std::endl;

        if ( _nr14.bits._initialize ) {
            _timeBeforeNextPhase = _periodOneEight = _nr13.bits._freqLo | ( _nr14.bits._freqHi << 8 );
            _soundLength = ( 64 - _nr11.bits.soundLength ) * 4 * 1024 * 1024 / 256;
            _phase = 0;
        }
    }
}

}
