#include "gameboy.h"

namespace gbemu
{   
    Memory& Gameboy::getMemory()
    {
        return _memory;
    }

    CPU& Gameboy::getCPU()
    {
        return _cpu;
    }

    Cartridge& Gameboy::getCartridge()
    {
        return _cartridge;
    }

    VideoDisplay& Gameboy::getVideo()
    {
        return _video;
    }

    DebugStringHandlerRegistry& Gameboy::getDebugRegistry()
    {
        return _debugReg;
    }

    int Gameboy::doCycle()
    {
        const int soundBufferLength = ( 4 * 1024 * 1024 / 32 );
        const int halfSoundBufferLength = soundBufferLength / 2;
        // emulate as many cycles as the cpu will be executing
        _papu.emulate( _cpu.previewInstructionTiming() );
        const int nbCycles = _cpu.emulateCycle() ;
        _clock = ( _clock + nbCycles ) % ( 4 * 1024 * 1024 );
        // If should stop emulating, break the loop
        if ( nbCycles <= 0 ) {
            return -1;
        }
        _video.emulate( nbCycles );
        emulateTimers( nbCycles );
        handleInterrupts();
        return nbCycles;
    }

    Gameboy::Gameboy(const char* const bootRom) :
        _memory( _bootRom, _video, _papu ),
        _cpu( _memory, _cartridge ),
        _video( _memory, !_bootRom.isInitialized() ),
        _cyclesToIncTimerCounter( 0 ),
        _cyclesToIncDivider( 0 ),
        _papu( _clock ),
        _clock( 0 ),
        _bootRom( bootRom )
    {}
    
    Gameboy::~Gameboy()
    {
    }

    void Gameboy::emulateTimers( int nbCycles )
    {
        // timer register is increment 16384 times per second, once every 256 cycles. When doing so, increment div
        _cyclesToIncDivider -= nbCycles;
        if ( _cyclesToIncDivider < 0 ) {
            _cyclesToIncDivider += kClockPerDividerCycle;
            ++_memory.memoryRegister( kDIV );
        }

        // If Timer counter is enabled
        if ( getBit( _memory.memoryRegister( kTAC ), 2 ) ) {
            static int kInputClockSelect[] = { 4096, 262144, 65536, 16384 };
            static int kCyclesPerTimerCounter[] = { 
                kCPUSpeed / kInputClockSelect[ 0 ],
                kCPUSpeed / kInputClockSelect[ 1 ],
                kCPUSpeed / kInputClockSelect[ 2 ],
                kCPUSpeed / kInputClockSelect[ 3 ] };
            
            _cyclesToIncTimerCounter -= nbCycles;
            if ( _cyclesToIncTimerCounter <= 0 ) {
                _cyclesToIncTimerCounter += kCyclesPerTimerCounter[ _memory.memoryRegister( kTAC ) & 0x3 ];
                ++_memory.memoryRegister( kTIMA );
                if ( _memory.memoryRegister( kTIMA ) == 0 ) {
                    _memory.memoryRegister( kIF ) |= Memory::kIFTimerOverflowFlag;
                    _memory.memoryRegister( kTIMA ) = _memory.memoryRegister( kTMA );
                }
            }
        }
    }

    void Gameboy::handleInterrupts()
    {
        enum InterruptIndexes { kVBlankIndex = 0, kLCDCIndex = 1, kTimerOverflowIndex = 2, kSerialIOComplete = 3, kHighLowPin = 4 };

        static const unsigned short interruptStartAddress [] = { 0x40, 0x48, 0x50, 0x58, 0x60 };
      //  static const int kVBlankStartAddress = 0x40;

        if ( _cpu.areInterruptsEnabled() ) {
            for ( unsigned int i = 0; i < 5; ++i ) {
                // if interrupt is enabled and is set
                if ( getBit( _memory.memoryRegister( kIE ), i ) &&
                        getBit( _memory.memoryRegister( kIF ), i ) )
                {
                    _cpu.executeInterrupt( interruptStartAddress[ i ] );
                    // reset the interrupt flag
                    //resetBit( _memory._bytes[ kIF ], i );
                    _memory.memoryRegister( kIF ) = 0;
                    JFX_ASSERT( !_cpu.areInterruptsEnabled() );
                    // we will be servicing an interrupt, we can't handle more for now
                    break;
                }
            }
        }
    }
}
