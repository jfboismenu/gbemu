//
//  mbc.h
//  gbemu
//
//  Created by JF Boismenu on 2012-10-08.
//
//

#ifndef gbemu_mbc_h
#define gbemu_mbc_h

#include <memory/cartridgeInfo.h>
#include <memory>
#include <vector>

namespace gbemu {

    class Memory;

    class MemoryBlockController
    {
    public:
        enum class Type { None, MBC1, MBC2, MBC3, MBC5, MMM01 };

        static std::unique_ptr< MemoryBlockController > create(
            Cartridge::Type type,
            std::vector< unsigned char >& rom,
            std::vector< unsigned char >& ram
        );

        virtual const char* getName() const = 0;
        virtual Type getType() const = 0;

        virtual void writeByte(
            unsigned short addr,
            unsigned char value
        ) = 0;
        virtual unsigned char readByte(
            unsigned short addr
        ) const = 0;
    };

}

#endif
