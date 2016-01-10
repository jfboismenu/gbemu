#include <cpu/timers.h>
#include <memory/memory.h>


namespace gbemu {

    Timers::Timers( Memory& memory ) :
        _cyclesToIncTimerCounter( 0 ),
        _cyclesToIncDivider( 0 ),
        _memory( memory ),
        _tima(0),
        _tma(0),
        _tac(0),
        _div(0)
    {}

    bool Timers::contains( unsigned short addr ) const
    {
        return addr == kTIMA || addr == kTMA || addr == kTAC || addr == kDIV;
    }

    unsigned char Timers::readByte( unsigned short addr ) const
    {
        if  ( addr == kTIMA ) {
            return _tima;
        }
        else if ( addr == kTAC ) {
            return _tac;
        }
        else if ( addr == kDIV ) {
            return _div;
        }
        else if ( addr == kTMA ) {
            return _tma;
        }
        else {
            JFX_MSG_ABORT( "Unknown memory address: " << addr );
        }
    }

    void Timers::writeByte( unsigned short addr, unsigned char value )
    {
        if ( addr == kTIMA ) {
            _tima = value;
        }
        else if ( addr == kTAC ) {
            _tac = value;
        }
        else if ( addr == kDIV ) {
            _div = 0;
        }
        else if ( addr == kTMA ) {
            _tma = value;
        }
        else {
            JFX_MSG_ABORT( "Unknown memory address: " << addr );
        }
    }

    void Timers::emulate(int nbCycles)
    {
        // timer register is increment 16384 times per second, once every 256 cycles. When doing so, increment div
        _cyclesToIncDivider -= nbCycles;
        if (_cyclesToIncDivider < 0) {
            _cyclesToIncDivider += kClockPerDividerCycle;
            ++_div;
        }

        // If Timer counter is enabled
        if (getBit(_tac, 2)) {
            static int kInputClockSelect[] = { 4096, 262144, 65536, 16384 };
            static int kCyclesPerTimerCounter[] = {
                kCPUSpeed / kInputClockSelect[0],
                kCPUSpeed / kInputClockSelect[1],
                kCPUSpeed / kInputClockSelect[2],
                kCPUSpeed / kInputClockSelect[3] };

            _cyclesToIncTimerCounter -= nbCycles;
            if (_cyclesToIncTimerCounter <= 0) {
                _cyclesToIncTimerCounter += kCyclesPerTimerCounter[_tac & 0x3];
                ++_tima;
                if (_tima == 0) {
                    _memory.memoryRegister(kIF) |= Memory::kIFTimerOverflowFlag;
                    _tima = _tma;
                }
            }
        }
    }
}
