#pragma once

#include <memory>
#include <vector>
#include <string>

namespace gbemu {
    class Memory;
    class MemoryBlockController;

    class Cartridge
    {
    public:
    
        // Cartridge type information accessors and values
        enum Type {
            kROM_ONLY = 0x0,            kROM_MBC3_RAM=0x12,
            kROM_MBC1=0x1,              kROM_MBC3_RAM_BATT=0x13,
            kROM_MBC1_RAM=0x2,          kROM_MBC5=0x19,
            kROM_MBC1_RAM_BATT=0x3,     kROM_MBC5_RAM=0x1A,
            kROM_MBC2=0x5,              kROM_MBC5_RAM_BATT=0x1B,
            kROM_MBC2_BATTERY=0x6,      kROM_MBC5_RUMBLE=0x1C,
            kROM_RAM=0x8,               kROM_MBC5_RUMBLE_SRAM=0x1D,
            kROM_RAM_BATTERY=0x9,       kROM_MBC5_RUMBLE_SRAM_BATT=0x1E,
            kROM_MMM01=0xB,             kPocketCamera=0x1F,
            kROM_MMM01_SRAM=0xC,        kBandaiTAMA5=0xFD,
            kROM_MMM01_SRAM_BATT=0xD,   kHudsonHuC3 = 0xFE,
            kROM_MBC3_TIMER_BATT=0xF,   kHudsonHuC1 = 0xFF,
            kROM_MBC3_TIMER_RAM_BATT=0x10,
            kROM_MBC3=0x11
	    };
    
        Cartridge();
        void Load( const std::string& filename );
        ~Cartridge();
        
        MemoryBlockController& getMBC();
        
        unsigned char getByte( unsigned short pos ) const;
        const unsigned char* getROMBankBytes( unsigned char value ) const;
        Cartridge::Type getType() const;
        size_t getROMSize() const;
        size_t getRAMSize() const;
        void saveRAM();

    private:
        std::vector< unsigned char > _bytes;
        std::vector< unsigned char > _ramBytes;
        std::string _cartPath;
        std::string _ramPath;
        std::unique_ptr< MemoryBlockController > _mbc;
    };

    namespace cartridgeInfo {
        const char* cartridgeTypeToString( Cartridge::Type );

        // GameBoy kind flags
        bool isGameBoyColor( const Cartridge& );
        bool isSuperGameBoy( const Cartridge& );
    };
}
