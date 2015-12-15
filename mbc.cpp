//
//  mbc.cpp
//  gbemu
//
//  Created by JF Boismenu on 2012-10-08.
//
//

#include "mbc.h"
#include "memory.h"
#include "cartridgeInfo.h"

namespace {
    using namespace gbemu;

    class MBCBase : public MemoryBlockController
    {
    public:
        MBCBase(
            std::vector< unsigned char >& rom,
            std::vector< unsigned char >& ram
        ) : _rom( rom ),
            _ram( ram )
        {}

        void writeRAMByte( int index, unsigned char value )
        {
            if ( getRAMSize() == 0 ) {
                return;
            }
            _ram.at( (size_t)(index % getRAMSize()) ) = value;
        }
        unsigned char readRAMByte( const int index ) const
        {
           if ( getRAMSize() == 0 ) {
                return 0;
            }
            return _ram.at( (size_t)(index % getRAMSize()) );
        }
        void writeROMByte( int index, unsigned char value )
        {
            _rom.at( (size_t)(index % getROMSize()) ) = value;
        }
        unsigned char readROMByte( const int index ) const
        {
            return _rom.at( (size_t)(index % getROMSize()) );
        }
        int getRAMSize() const
        {
            return (int)_ram.size();
        }
        int getROMSize() const
        {
            return (int)_rom.size();
        }

    private:
        std::vector< unsigned char >& _rom;
        std::vector< unsigned char >& _ram;
    };

    class MBC1 : public MBCBase
    {
    public:

        enum class Mode { _16_8, _4_32 };

        MBC1(
            std::vector< unsigned char >& rom,
            std::vector< unsigned char >& ram
        ) : MBCBase( rom, ram ),
            _mode( Mode::_16_8 ),
            _lowerRomBankBits( 0 ),
            _romRamBits( 0 ),
            _externalRAMEnabled( false )
        {}

        Type getType() const
        {
            return Type::MBC1;
        }

        const char* getName() const
        {
            return "MBC1";
        }

        virtual void writeByte(
            unsigned short addr,
            unsigned char value
        )
        {
            // rom bank 0, lower 16k
            if ( addr < 0x2000 ) {
                // Do we really care?
                _externalRAMEnabled = ( ( value & 0x0A ) == 0x0A );
            }
            // rom bank 0, upper 16k
            else if ( addr >= 0x2000 && addr < 0x4000 ) {
                _lowerRomBankBits = value & GetMask( 0, 0, 0, 1, 1, 1, 1, 1 );
            }
            // switchable rom bank writing, lower 16k
            else if ( addr >= 0x4000 && addr < 0x6000 ) {
                _romRamBits = value & 0x3;
            }
            // switchable rom bank writing, upper 16k
            else if ( addr >= 0x6000 && addr < 0x8000 ) {
                _mode = getBit( value, 0 ) ? Mode::_4_32 : Mode::_16_8;
            }
            else if ( Memory::isSwitchableRAMBank( addr ) ) {
                if ( !_externalRAMEnabled ) {
                    return;
                }
                if ( _mode == Mode::_16_8 ) {
                    writeRAMByte( addr - 0xa000, value );
                }
                else {
                    writeRAMByte( ( addr - 0xa000 ) + _romRamBits * 8 * 1024, value );
                }
            }
            else {
                JFX_MSG_ASSERT( "Write at " << addr << " not supported for MBC1" );
            }
        }

        virtual unsigned char readByte(
            unsigned short addr
        ) const
        {
            if ( Memory::isROMBank0( addr ) ) {
                return readROMByte( addr );
            }
            else if ( Memory::isSwitchableROMBank( addr ) ) {
                if ( _mode == Mode::_16_8 ) {
                    return readROMByte(
                        ( ( ( _romRamBits << 5 ) | _lowerRomBankBits ) * 16 * 1024 ) |
                        ( addr - Memory::getSwitchableROMBankStart() )
                    );
                }
                else {
                    return readROMByte(
                        ( _lowerRomBankBits * 16 * 1024 ) |
                        ( addr - Memory::getSwitchableROMBankStart() )
                    );
                }
            }
            else if ( Memory::isSwitchableRAMBank( addr ) ) {
                if ( !_externalRAMEnabled ) {
                    return 0;
                }
                if ( _mode == Mode::_16_8 ) {
                    return readRAMByte( addr - 0xa000 );
                }
                else {
                    return readRAMByte( ( addr - 0xa000 ) + _romRamBits * 8 * 1024 );
                }
            }
            else {
                JFX_MSG_ASSERT( "Reading at " << addr << " for MBC1 not supported." );
            }
        }

    private:
        Mode _mode;
        int  _romRamBits;
        int  _lowerRomBankBits;
        bool _externalRAMEnabled;
    };

    class MBC2 : public MBCBase
    {
    public:

        MBC2(
            std::vector< unsigned char >& rom,
            std::vector< unsigned char >& ram
        ) : MBCBase( rom, ram ),
            _romBankIndex( 0 ),
            _externalRAMEnabled( false )
        {}

        Type getType() const
        {
            return Type::MBC2;
        }

        const char* getName() const
        {
            return "MBC2";
        }

        virtual void writeByte(
            unsigned short addr,
            unsigned char value
        )
        {
            // rom bank 0, lower 16k
            if ( addr < 0x2000 ) {
                // Do we really care?
                _externalRAMEnabled = ( ( value & 0x0A ) == 0x0A );
            }
            else if ( addr >= 0x2000 && addr < 0x4000 ) {
                _romBankIndex = value & GetMask( 0, 0, 0, 0, 1, 1, 1, 1 );
            }
            else if ( Memory::isSwitchableRAMBank( addr ) ) {
                if ( !_externalRAMEnabled ) {
                    return;
                }
                writeRAMByte( addr - 0xa000, value );
            }
            else {
                JFX_MSG_ASSERT( "Write at " << addr << " not supported for MBC2" );
            }
        }

        virtual unsigned char readByte(
            unsigned short addr
        ) const
        {
            if ( Memory::isROMBank0( addr ) ) {
                return readROMByte( addr );
            }
            else if ( Memory::isSwitchableROMBank( addr ) ) {
                return readROMByte(
                    ( ( _romBankIndex ) * 16 * 1024 ) |
                    ( addr - Memory::getSwitchableROMBankStart() )
                );
            }
            else if ( Memory::isSwitchableRAMBank( addr ) ) {
                if ( !_externalRAMEnabled ) {
                    return 0;
                }
                return readRAMByte( addr - 0xa000 );
            }
            else {
                JFX_MSG_ASSERT( "Reading at " << addr << " for MBC2 not supported." );
            }
        }

    private:
        int  _romBankIndex;
        bool _externalRAMEnabled;
    };

    class NoMBC : public MBCBase
    {
    public:
        NoMBC(
            std::vector< unsigned char >& rom,
            std::vector< unsigned char >& ram
        ) : MBCBase( rom, ram )
        {}

        Type getType() const
        {
            return Type::None;
        }
        const char* getName() const
        {
            return "None";
        }
        void writeByte(
            unsigned short,
            unsigned char
        )
        {
            // FIXME: Had to deactivate this. Maybe it should only
            // be a warning?
            //JFX_MSG_ASSERT( "Not supposed to write to None MBC" );
        }
        virtual unsigned char readByte(
            unsigned short addr
        ) const
        {
            return readROMByte( addr );
        }
    };
}

namespace gbemu {

    template< typename T >
    std::unique_ptr< T > make_unique( T* const t )
    {
        return std::unique_ptr< T >( t );
    }

    std::unique_ptr< MemoryBlockController > MemoryBlockController::create(
        Cartridge::Type type,
        std::vector< unsigned char >& rom,
        std::vector< unsigned char >& ram
    )
    {
        switch( type ) {
            case Cartridge::kROM_ONLY:
            case Cartridge::kROM_RAM:
            case Cartridge::kROM_RAM_BATTERY:
                return make_unique( new NoMBC( rom, ram ) );
            case Cartridge::kROM_MBC1:
            case Cartridge::kROM_MBC1_RAM:
            case Cartridge::kROM_MBC1_RAM_BATT:
                return make_unique( new MBC1( rom, ram ) );

            case Cartridge::kROM_MBC2:
            case Cartridge::kROM_MBC2_BATTERY:
                return make_unique( new MBC2( rom, ram ) );

            case Cartridge::kROM_MMM01:
            case Cartridge::kROM_MMM01_SRAM:
            case Cartridge::kROM_MMM01_SRAM_BATT:

            case Cartridge::kROM_MBC3:
            case Cartridge::kROM_MBC3_RAM:
            case Cartridge::kROM_MBC3_RAM_BATT:
            case Cartridge::kROM_MBC3_TIMER_BATT:
            case Cartridge::kROM_MBC3_TIMER_RAM_BATT:

            case Cartridge::kROM_MBC5:
            case Cartridge::kROM_MBC5_RAM:
            case Cartridge::kROM_MBC5_RAM_BATT:
            case Cartridge::kROM_MBC5_RUMBLE:
            case Cartridge::kROM_MBC5_RUMBLE_SRAM:
            case Cartridge::kROM_MBC5_RUMBLE_SRAM_BATT:

            case Cartridge::kPocketCamera:
            case Cartridge::kBandaiTAMA5:
            case Cartridge::kHudsonHuC1:
            case Cartridge::kHudsonHuC3:
                JFX_MSG_ASSERT( "Other memory block controllers not implemented." )
                return nullptr;
            default:
                JFX_MSG_ASSERT("Unknown memory block controller" << std::hex << std::endl)
        }
    }
}
