//
//  memoryRegion.h
//  gbemu
//
//  Created by JF Boismenu on 2012-10-23.
//
//

#ifndef __gbemu__bootRom__
#define __gbemu__bootRom__

#include <vector>

namespace gbemu {

    class BootRom
    {
    public:
        BootRom( const char* const bootRomPath );
        unsigned char readByte( unsigned short addr ) const;
        unsigned short getLastByteAddr() const;
        bool isInitialized() const;
    private:
        std::vector< unsigned char > _bytes;
    };
    
}
#endif // __gbemu__bootRom__
