//
//  memoryRegion.h
//  gbemu
//
//  Created by JF Boismenu on 2012-10-23.
//
//

#include <memory/bootRom.h>
#include <common/common.h>

namespace gbemu
{
    BootRom::BootRom(const char* const bootRomPath)
    {
        if (bootRomPath!= 0) {
            _bytes = readFile(bootRomPath);
            JFX_CMP_ASSERT(_bytes.size(), == , 256);
        }

    }

    bool BootRom::isInitialized() const
    {
        return !_bytes.empty();
    }

    unsigned char BootRom::readByte( const unsigned short addr ) const
    {
        return _bytes.at( addr );
    }

    unsigned short BootRom::getLastByteAddr() const
    {
        return (unsigned short)_bytes.size() - 1;
    }
}
