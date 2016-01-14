#include <gameboy.h>

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

    PAPU& Gameboy::getPAPU()
    {
        return _papu;
    }

    const CPUClock& Gameboy::getClock() const
    {
        return _clock;
    }

    int Gameboy::doCycle()
    {
        // emulate as many cycles as the cpu will be executing
        const int nbCycles = _cpu.emulateCycle();
        _clock +=nbCycles;
        // If should stop emulating, break the loop
        if ( nbCycles <= 0 ) {
            return -1;
        }
        _papu.emulate( nbCycles );
        _video.emulate( nbCycles );
        _timers.emulate( nbCycles );
        handleInterrupts();
        return nbCycles;
    }

    Gameboy::Gameboy(const char* const bootRom) :
        _clock( 4194304 ),
        _memory( _bootRom, _video, _timers, _papu ),
        _cpu( _memory, _cartridge ),
        _video( _memory, !_bootRom.isInitialized() ),
        _papu( _clock ),
        _bootRom( bootRom ),
        _timers( _memory )
    {}

    Gameboy::~Gameboy()
    {
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
