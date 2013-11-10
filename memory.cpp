#include "memory.h"
#include "cartridgeInfo.h"
#include "mbc.h"
#include "bootRom.h"
#include "papu.h"
#include <memory>

namespace {
    bool isBetween(
        unsigned short tested,
        unsigned short lower,
        unsigned short upper
    )
    {
        return lower <= tested && tested < upper;
    }
}

namespace gbemu {
    bool Memory::isROMBank0( unsigned short addr )
    {
        return isBetween( addr, 0x0000, 0x4000 );
    }
    bool Memory::isSwitchableROMBank( unsigned short addr )
    {
        return isBetween( addr, getSwitchableROMBankStart(), 0x8000 );
    }
    bool Memory::isVideoRAM( unsigned short addr )
    {
        return isBetween( addr, 0x8000, 0xA000 );
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
    bool Memory::isOAM( unsigned short addr )
    {
        return isBetween( addr, 0xFE00, 0xFEA0 );
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
        PAPU& papu
    ) : _keyState( 0 ),
        _bootRom( bootRom ),
        _isBooting( bootRom.isInitialized() ),
        _papu( papu )
    {
        memset( _bytes, 0, sizeof( _bytes ) );
        _bytes[ kP1 ] = 0xff;
        if (!hasBootRom()) {
            memoryRegister(kTIMA) = 0x0;
            memoryRegister(kTMA) = 0;
            memoryRegister(kTAC) = 0;
            memoryRegister(kNR10) = 0x80;
            memoryRegister(kNR11) = 0xBF;
            memoryRegister(kNR12) = 0xF3;
            memoryRegister(kNR14) = 0xBF;
            memoryRegister(kNR21) = 0x3F;
            memoryRegister(kNR22) = 0x00;
            memoryRegister(kNR24) = 0xBF;
            memoryRegister(kNR30) = 0x7F;
            memoryRegister(kNR31) = 0xFF;
            memoryRegister(kNR32) = 0x9F;
            memoryRegister(kNR33) = 0xBF;
            memoryRegister(kNR41) = 0xFF;
            memoryRegister(kNR42) = 0x00;
            memoryRegister(kNR43) = 0x00;
            memoryRegister(kNR30) = 0xBF;
            memoryRegister(kNR50) = 0x77;
            memoryRegister(kNR51) = 0xF3;
            memoryRegister(kNR52) = 0xF1;
            memoryRegister(kLCDC) = 0x91;
            memoryRegister(kSCY)  = 0x00;
            memoryRegister(kSCX)  = 0x00;
            memoryRegister(kLYC)  = 0x00;
            memoryRegister(kBGP)  = 0xFC;
            memoryRegister(kOBP0) = 0xFF;
            memoryRegister(kOBP1) = 0xFF;
            memoryRegister(kWX)   = 0x00;
            memoryRegister(kWY)   = 0x00;
            memoryRegister(kIE)   = 0x00;
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
        if ( addr == kP1 ) {
            noop();
        }
        else if ( addr == kSB ) {
            noop();
        }
        else if ( addr == kSC ) {
            noop();
        }
        else if ( addr == kDIV ) {
            noop();
        }
        else if ( addr == kTIMA ) {
            noop();
        }
        else if ( addr == kTMA ) {
            noop();
        }
        else if ( addr == kTAC ) {
            noop();
        }
        else if ( addr == kLCDC ) {
            noop();
        }
        else if ( addr == kSTAT ) {
            noop();
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
        else if ( isVideoRAM( addr ) ) {
            // Can't write to this region of memory during mode 3
            if ( getBit( _bytes[ kLCDC ], 7 ) && ( _bytes[kSTAT] & 0x03 ) == 0x03 ) {
                //JFX_MSG_ASSERT( "Trying to write at RAM when not allowed to" );
				_bytes[ addr ] = value;
            }
            else {
                 if ( addr >= 0x8000 && addr < 0x9800 && value != 0 ) {
                    noop();
                 }
                _bytes[ addr ] = value;
            }
        }
        else if ( isInternalRAM( addr ) ) {
            _bytes[addr] = value;
        }
        else if ( isInternalRAMEcho( addr ) ) {
            _bytes[addr - 0x2000] = value;
        }
        else if (addr < 0xfea0) {
            // Can't write to this region of memory during mode 2
            if ( getBit( _bytes[ kLCDC ], 7 ) && ( _bytes[kSTAT] & 0x02 ) == 0x02 ) {
                JFX_MSG_ASSERT( "Illegal write to OAM" );
            } else {
                _bytes[addr] = value;
            }
        } 
        else if (addr < 0xff00) {
            _bytes[addr] = value;
        }
        else if ( addr < 0xff80 ) {
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
            else if ( addr == kDIV ) {
                _bytes[ addr ] = 0;
            }
            else if ( addr == kIF ) {
                _bytes[ addr ] = value;
            }
            else if ( addr == kTIMA ) {
                _bytes[ addr ] = value;
            }
            else if ( addr == kTMA ) {
                _bytes[ addr ] = value;
            }
            else if ( addr == kTAC ) {
                _bytes[ addr ] = value;
            }
            else if ( addr == kSTAT ) {
                static const unsigned char writableBytesMask = GetMask( 0, 1, 1, 1, 1, 1, 0, 0 );
                // writable bytes are 2, 3, 4 and 5 and 7 is always set
                _bytes[ addr ] = ( _bytes[ addr ] & (~writableBytesMask) ) | ( value & writableBytesMask ) | 0x80;
            }
            else if ( addr == kDMA ) {
                for ( unsigned short i = 0; i < ( 0xFEA0 - 0xFE00 ); ++i ) {
                    _bytes[ 0xFE00 + i ] = readByte( ( value * 256 ) + i );
                }
                _bytes[ addr ] = value;
            }
            else if ( addr == kLCDC ) {
                _bytes[ addr ] = value;
            }
            else if ( addr == kLYC ) {
                _bytes[ addr ] = value;
            }
            else if ( addr == kSCX ) {
                _bytes[ addr ] = value;
            }
            else if ( kSoundRegistersStart < addr && addr < kSoundRegistersEnd ) {
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
