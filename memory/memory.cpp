#include <memory/memory.h>
#include <memory/cartridgeInfo.h>
#include <memory/mbc.h>
#include <memory/bootRom.h>
#include <audio/papu.h>
#include <video/videoDisplay.h>
#include <cpu/timers.h>
#include <memory>

namespace gbemu {
    bool Memory::isROMBank0( unsigned short addr )
    {
        return isBetween( addr, 0x0000, 0x4000 );
    }
    bool Memory::isSwitchableROMBank( unsigned short addr )
    {
        return isBetween( addr, getSwitchableROMBankStart(), 0x8000 );
    }

    bool Memory::isSwitchableRAMBank( unsigned short addr )
    {
        return isBetween( addr, 0xA000, 0xC000 );
    }
    bool Memory::isInternalRAM( unsigned short addr )
    {
        return isBetween( addr, 0xC000, 0xE000 );
    }
    bool Memory::isInternalRAMEcho( unsigned short addr )
    {
        return isBetween( addr, 0xE000, 0xFE00 );
    }

    bool Memory::isMemoryMapped( unsigned short addr )
    {
        return isROMBank0( addr ) ||
               isSwitchableROMBank( addr ) ||
               isSwitchableRAMBank( addr );
    }
    unsigned short Memory::getSwitchableROMBankStart()
    {
        return 0x4000;
    }

    Memory::Memory(
        const BootRom& bootRom,
        VideoDisplay&  videoDisplay,
        Timers&        timers,
        PAPU&          papu
    ) : _keyState( 0 ),
        _bootRom( bootRom ),
        _videoDisplay( videoDisplay ),
        _isBooting( bootRom.isInitialized() ),
        _papu( papu ),
        _timers( timers )
    {
        memset( _bytes, 0, sizeof( _bytes ) );
        _bytes[ kP1 ] = 0xff;
        if (!hasBootRom()) {
            memoryRegister(kIE) = 0x00;
        }
    }

    void Memory::loadCartridge( Cartridge& cartridge )
    {
        _cartridge = &cartridge;
    }

    void Memory::setKeyState( unsigned char state )
    {
        _keyState = state;
    }

    unsigned char Memory::readByte( unsigned short addr ) const
    {
        // When we are booting, the first 256 bytes are in the boot rom
        if ( _isBooting && addr <= _bootRom.getLastByteAddr() ) {
            const unsigned char oneByte = _bootRom.readByte( addr );
            if ( _bootRom.getLastByteAddr() == addr ) {
                _isBooting = false;
            }
            return oneByte;
        }
        if ( isMemoryMapped( addr ) ) {
            return _cartridge->getMBC().readByte( addr );
        }
        if ( isInternalRAMEcho( addr ) ) {
            return _bytes[addr - 0x2000];
        }
        if ( VideoDisplay::isVideoMemory( addr ) ) {
            return _videoDisplay.readByte( addr );
        }
        if ( _papu.contains( addr ) ) {
            return _papu.readByte( addr );
        }
        if ( _timers.contains( addr ) ) {
            return _timers.readByte( addr );
        }
        return _bytes[ addr ];
    }

    unsigned char& Memory::memoryRegister( unsigned short addr )
    {
        return _bytes[ addr ];
    }

    bool Memory::hasBootRom() const
    {
        return _bootRom.isInitialized();
    }

    void Memory::writeByte( unsigned short addr, unsigned char value )
    {
        using namespace cartridgeInfo;
        if ( isMemoryMapped( addr ) ) {
            _cartridge->getMBC().writeByte( addr, value );
        }
        else if ( isInternalRAM( addr ) ) {
            _bytes[addr] = value;
        }
        else if ( isInternalRAMEcho( addr ) ) {
            _bytes[addr - 0x2000] = value;
        }
        else if ( isBetween( addr, 0xFEA0, 0xff00 ) ) {
            _bytes[addr] = value;
        }
        else if (VideoDisplay::isVideoMemory(addr)) {
            _videoDisplay.writeByte( addr, value );
        }
        else if ( _timers.contains( addr ) ) {
            _timers.writeByte( addr, value );
        }
        else if ( isBetween( addr, 0xff00, 0xff80 ) ) {
            if ( addr == kP1 ) {
/*
  bit 4 - P14        P15 - bit - 5
          |          |
P10-------O-Right----O-A  ------- bit 0
          |          |
P11-------O-Left-----O-B  ------- bit 1
          |          |
P12-------O-Up-------O-Select --- bit 2
          |          |
P13-------O-Down-----O-Start ---- bit 3
*/
                if ( !getBit( value, 4 ) ) {
                    _bytes[ kP1 ] = 0xc0 | ( value & 0x30 ) | ( (~_keyState) & 0x0F );
                }
                if ( !getBit( value, 5 ) ) {
                    _bytes[ kP1 ] = 0xc0 | ( value & 0x30 ) | ( ( ~_keyState >> 4 ) & 0x0F );
                }

            }
            else if ( addr == kSB ) {
                _bytes[ addr ] = value;
            }
            else if ( addr == kSC ) {
                _bytes[ addr ] = value;
            }
            else if ( addr == kIF ) {
                _bytes[ addr ] = value;
            }
            else if ( _papu.contains( addr ) ) {
                _papu.writeByte( addr, value );
            }
            else {
                _bytes[ addr ] = value;
            }
        }
        else if ( addr < 0xffff ) {
            _bytes[ addr ] = value;
        }
        else {
            _bytes[ addr ] = value | 0xe0; // writing IE register bits
        }
    }
}
