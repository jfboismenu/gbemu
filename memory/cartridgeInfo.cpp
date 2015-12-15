#include <memory/cartridgeInfo.h>
#include <memory/memory.h>
#include <memory/mbc.h>

#include <fstream>
#include <cassert>

namespace gbemu {

    Cartridge::Cartridge()
    {}

    Cartridge::~Cartridge()
    {}

    std::vector< unsigned char > allocateRAM(
        unsigned char value,
        Cartridge::Type cartType
    )
    {
        switch( cartType ) {
            // MBC2 carts have their memory byte set to 0 even tough they have
            // 2k of RAM
            case Cartridge::kROM_MBC2:
            case Cartridge::kROM_MBC2_BATTERY: {
                return std::vector< unsigned char >( 2 * 1024, 0 );
            }

            default: {
                switch( value ) {
                    case 0:
                        return std::vector< unsigned char >();
                    case 1:
                        return std::vector< unsigned char >( 2 * 1024, 0 );
                    case 2:
                        return std::vector< unsigned char >( 8 * 1024, 0 );
                    case 3:
                        return std::vector< unsigned char >( 32 * 1024, 0 );
                    case 4:
                        return std::vector< unsigned char >( 128 * 1024, 0 );
                }
            }
        }
        JFX_MSG_ASSERT( "Unknown RAM size." );
        return std::vector< unsigned char >();
    }

    void Cartridge::Load( const std::string& filename )
    {
        _bytes = readFile( filename );
        // Validate it and allocate buffer
        JFX_ASSERT( !_bytes.empty() );
        JFX_CMP_ASSERT( _bytes.size(), <, 2 * 1024 * 1024 );

        std::vector< unsigned char > ram;

        const std::string::size_type pos = filename.rfind( '.' );
        if ( pos == std::string::npos ) {
            _ramPath = filename + ".ram";
        }
        else {
            _ramPath = filename.substr( 0, pos ) + ".ram";
        }

        // Read RAM from file
        _ramBytes = readFile( _ramPath );
        // If nothing was read, allocate space for the RAM based on the cart info
        if ( _ramBytes.empty() ) {
            _ramBytes = allocateRAM( _bytes[ 0x149 ], getType() );
        }


        _mbc = MemoryBlockController::create( getType(), _bytes, _ramBytes );

    }
    unsigned char Cartridge::getByte( const unsigned short pos ) const
    {
        return _bytes[ pos ];
    }
    MemoryBlockController& Cartridge::getMBC()
    {
        return *_mbc;
    }
    Cartridge::Type Cartridge::getType() const
    {
        return static_cast< Cartridge::Type >( getByte( 0x147 ) );
    }
    size_t Cartridge::getROMSize() const
    {
        return _bytes.size();
    }
    size_t Cartridge::getRAMSize() const
    {
        return _ramBytes.size();
    }
    void Cartridge::saveRAM()
    {
        if ( getRAMSize() > 0 ) {
            std::ofstream fileRAM( _ramPath, std::ios::binary );
            fileRAM.write( reinterpret_cast< const char* >( &_ramBytes.front() ),
                           (std::streamsize)_ramBytes.size() );
        }
    }


    namespace cartridgeInfo {
        const char* cartridgeTypeToString( const Cartridge::Type type )
        {
            switch ( type ) {
                case Cartridge::kROM_ONLY:
                    return "ROM Only";
                case Cartridge::kROM_MBC1:
                    return "ROM+MCB1";
                case Cartridge::kROM_MBC1_RAM:
                    return "ROM+MCB1+RAM";
                case Cartridge::kROM_MBC1_RAM_BATT:
                    return "ROM+MCB1+RAM+BATT";
                case Cartridge::kROM_MBC2:
                    return "ROM+MCB2";
                case Cartridge::kROM_MBC2_BATTERY:
                    return "ROM+MCB2+BATTERY";
                case Cartridge::kROM_RAM:
                    return "ROM+RAM";
                case Cartridge::kROM_RAM_BATTERY:
                    return "ROM+RAM+BATTERY";
                case Cartridge::kROM_MMM01:
                    return "ROM+MMM01";
                case Cartridge::kROM_MMM01_SRAM:
                    return "ROM+MMM01+SRAM";
                case Cartridge::kROM_MMM01_SRAM_BATT:
                    return "ROM+MMM01+SRAM+BATT";
                case Cartridge::kROM_MBC3_RAM:
                    return "ROM+MCB3+RAM";
                case Cartridge::kROM_MBC3_RAM_BATT:
                    return "ROM+MCB3+RAM+BATT";
                case Cartridge::kROM_MBC5:
                    return "ROM+MCB5";
                case Cartridge::kROM_MBC5_RAM:
                    return "ROM+MCB5+RAM";
                case Cartridge::kROM_MBC5_RAM_BATT:
                    return "ROM+MCB5+RAM+BATT";
                case Cartridge::kROM_MBC5_RUMBLE:
                    return "ROM+MCB5+RUMBLE";
                case Cartridge::kROM_MBC5_RUMBLE_SRAM:
                    return "ROM+MCB5+RUMBLE+SRAM";
                case Cartridge::kROM_MBC5_RUMBLE_SRAM_BATT:
                    return "ROM+MCB5+RUMBLE+SRAM+BATT";
                case Cartridge::kPocketCamera:
                    return "Pocket Camera";
                case Cartridge::kBandaiTAMA5:
                    return "Bandai TAMA5";
                case Cartridge::kHudsonHuC1:
                    return "Hudson HuC-1";
                case Cartridge::kHudsonHuC3:
                    return "Hudson HuC-3";
                case Cartridge::kROM_MBC3_TIMER_BATT:
                    return "ROM+MBC3+TIMER+BATT";
                case Cartridge::kROM_MBC3_TIMER_RAM_BATT:
                    return "ROM+MBC3+TIMER+RAM+BATT";
                case Cartridge::kROM_MBC3:
                    return "ROM+MBC3";
                default:
                    return "Unknown Cartridge Type";
	        }
        }

        bool isGameBoyColor( const Cartridge& cart )
        {
            return cart.getByte( 0x143 ) == 0x80;
        }

        bool isSuperGameBoy( const Cartridge& cart )
        {
            return cart.getByte( 0x146 ) == 3;
        }
    }
}
