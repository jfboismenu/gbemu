#pragma once

#include <cpu/registers.h>
#include <memory/memoryRegion.h>

namespace gbemu {

    class Cartridge;
    class BootRom;
    class PAPU;
    class VideoDisplay;
    class Timers;

    class Memory : public WordIOProtocol< Memory >
    {
    public:

        static const unsigned char kIFVBlankFlag = 0x1;
        static const unsigned char kIFHVBlankFlag = 0x2;
        static const unsigned char kIFTimerOverflowFlag = 0x4;
        static const unsigned char kIFHighToLowFlag = 0x10;

        static bool isROMBank0( unsigned short addr );
        static bool isSwitchableROMBank( unsigned short addr );
        static bool isVideoRAM( unsigned short addr );
        static bool isSwitchableRAMBank( unsigned short addr );
        static bool isInternalRAM( unsigned short addr );
        static bool isInternalRAMEcho( unsigned short addr );
        static bool isOAM( unsigned short addr );
        static bool isMemoryMapped( unsigned short addr );
        static unsigned short getSwitchableROMBankStart();

        Memory(
            const BootRom& rom,
            VideoDisplay&  videoDisplay,
            Timers&        timers,
            PAPU&          papu
        );

        void setKeyState( unsigned char state );
        unsigned char readByte( unsigned short addr ) const;
        void writeByte( unsigned short addr, unsigned char value );
        void loadCartridge( Cartridge& cartridge );
        unsigned char& memoryRegister( unsigned short addr );
        bool hasBootRom() const;


    private:
        MemoryRegion< 0xC000, 0xE000 > _internalRAM;
        // FIXME: This big array should be deprecated
        // in favor of a smaller byte array that covers
        // only the memory registers
        unsigned char                  _bytes[64*1024];
        Cartridge*                     _cartridge;
        unsigned char                  _keyState;
        const BootRom&                 _bootRom;
        VideoDisplay&                  _videoDisplay;
        mutable bool                   _isBooting;
        PAPU&                          _papu;
        Timers&                        _timers;
    };
}
