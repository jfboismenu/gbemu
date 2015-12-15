#pragma once

#include <common/common.h>

namespace gbemu {
    template< typename traits, int ReadMask = 0xFF, int WriteMask = 0xFF >
    union Register
    {
    public:
        JFX_INLINE Register( unsigned char value ) : _byte( value )
        {
        }
        JFX_INLINE Register()
        {}
        JFX_INLINE void write( unsigned char value )
        {
            _byte = ( WriteMask & value ) | ( ~WriteMask & _byte );
        }
        JFX_INLINE unsigned char read() const
        {
            return _byte & ReadMask;
        }
        traits bits;
    private:
        unsigned char _byte;
    };
}
