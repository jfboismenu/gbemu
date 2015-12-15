#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <cassert>

#ifdef WINDOWS
#pragma warning ( disable : /*4482 4480*/ 4201 )
#endif

#define JFX_INLINE inline
#define JFX_CMP_ASSERT( a, cmp, b ) \
{ \
	if ( !( (a) cmp (b) ) ) { \
		std::cout << #a #cmp #b << " failed!" << std::endl; \
		std::cout << #a << " : " << a << std::endl; \
		std::cout << #b << " : " << b << std::endl; \
        abort(); \
	} \
}

#define JFX_MSG_ASSERT( msg ) \
{ \
    std::cout << msg << std::endl; \
    abort(); \
}

#define JFX_COND_ASSERT( cond, msg ) \
if ( !( cond ) ) { \
   JFX_MSG_ASSERT( #cond " failed: " << msg ); \
}

#define JFX_ASSERT( cond )                \
{                                         \
    if (!(cond)) {                        \
        JFX_MSG_ASSERT(#cond " failed."); \
    }                                     \
}

namespace gbemu {
    JFX_INLINE void noop()
    {
    }
    
    JFX_INLINE std::vector< unsigned char > readFile( const std::string& filename )
    {
        std::ifstream cart( filename, std::ios::binary );
        if ( !cart.is_open() ) {
            return std::vector< unsigned char >();
        }
        
        std::vector< unsigned char > bytes;
        
        // Get the cart size
        cart.seekg( 0, std::ios::end );
        const size_t cartSize = static_cast< size_t >( cart.tellg() );
        cart.seekg( 0, std::ios::beg );
        
        bytes.resize( cartSize );
        
        // read the data in.
        cart.read( reinterpret_cast< char* >( &bytes.front() ), (std::streamsize)bytes.size() );
        JFX_CMP_ASSERT( cart.gcount(), ==, (std::streamsize)bytes.size() );
        
        return bytes;
    }

    JFX_INLINE unsigned char GetMask(
        bool b7,
        bool b6,
        bool b5,
        bool b4,
        bool b3,
        bool b2,
        bool b1,
        bool b0 )
     {
        return (unsigned char)(( b7 << 7 ) | ( b6 << 6 ) | ( b5 << 5 ) | ( b4 << 4 ) |
               ( b3 << 3 ) | ( b2 << 2 ) | ( b1 << 1 ) | ( b0 << 0 ));
     }

    template< typename T, typename U >
    struct IsSameType
    {
        enum { Yes = 0, No = 1 };
    };

    template< typename T >
    struct IsSameType< T, T >
    {
        enum { Yes = 1, No = 0 };
    };

    template< typename T >
    JFX_INLINE void AssertIndex( unsigned int index )
    {
        JFX_CMP_ASSERT( index, <, sizeof( T ) * 8 );
    }

    template< typename T >
    JFX_INLINE bool getBit( T data, unsigned int index )
    {
        AssertIndex< T >( index );
        return ( data & ( 1 << index ) ) != 0;
    }

    template< typename T >
    JFX_INLINE bool getMSB( T data )
    {
        return getBit( data, ( sizeof( T ) * 8 ) - 1 );
    }

    template< typename T >
    JFX_INLINE bool getLSB( T data )
    {
        return getBit( data, 0 );
    }

    template< typename T >
    JFX_INLINE void resetBit( T& data, unsigned int index )
    {
        AssertIndex< T >( index );
        // Keep all bits except index.
        data &= ~( 1 << index );
    }

    template< typename T >
    JFX_INLINE void setBit( T& data, unsigned int index )
    {
        AssertIndex< T >( index );
        data |= ( 1 << index );
    }

    template< typename T >
    JFX_INLINE void copyBit( T& data, unsigned int index, bool value )
    {
        if ( value ) {
            setBit( data, index );
        }
        else {
            resetBit( data, index );
        }
    }
    
    template< typename T >
    JFX_INLINE unsigned char lowNibble( T b )
    {
        return b & 0x0F;
    }

    template< typename T >
    JFX_INLINE unsigned char highNibble( T b )
    {
        return lowNibble( b >> 4 );
    }

    template< typename T >
    JFX_INLINE bool between( const T lower, const T higher, const T value )
    {
        JFX_CMP_ASSERT( lower, <=, higher );
        return lower <= value && value < higher;
    }
    
    template< typename T >
    class WordIOProtocol
    {
    public:
        inline unsigned short readWord( unsigned short addr) const
        {
            const T* t = static_cast< const T* >( this );
            return t->readByte( addr ) + (unsigned short)( t->readByte( addr + 1 ) << 8 );
        }
        inline void writeWord( unsigned short addr, unsigned short value )
        {
            T* t = static_cast< T* >( this );
            t->writeByte( addr, value & 0x00FF );
            t->writeByte( addr + 1, ( value & 0xFF00 ) >> 8 );
        }
    };

    JFX_INLINE bool isBetween(
        unsigned short tested,
        unsigned short lower,
        unsigned short upper
        )
    {
        return lower <= tested && tested < upper;
    }

}
