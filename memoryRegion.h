//
//  memoryRegion.h
//  gbemu
//
//  Created by JF Boismenu on 2012-10-23.
//
//

#ifndef __gbemu__memoryRegion__
#define __gbemu__memoryRegion__

template< int StartAddr, int EndAddr >
class MemoryRegion
{
public:
    MemoryRegion();
    ~MemoryRegion();
    bool isInside( int addr ) const;
    const unsigned char& byte( int addr ) const;
    const unsigned short& word( int addr ) const;
    unsigned char& byte( int addr );
    unsigned short& word( int addr );
private:
    unsigned char* _bytes;
    unsigned char* _startAddr;
};

template< int StartAddr, int EndAddr >
inline MemoryRegion< StartAddr, EndAddr >::MemoryRegion() :
    _bytes( new unsigned char[ EndAddr - StartAddr ] ),
    _startAddr( _bytes - StartAddr )
{}

template< int StartAddr, int EndAddr >
inline MemoryRegion< StartAddr, EndAddr >::~MemoryRegion()
{
    delete [] _bytes;
}

template< int StartAddr, int EndAddr >
inline bool MemoryRegion< StartAddr, EndAddr >::isInside( int addr ) const
{
    return StartAddr <= addr && addr < EndAddr;
}

template< int StartAddr, int EndAddr >
inline const unsigned char& MemoryRegion< StartAddr, EndAddr >::byte( int addr ) const
{
    return _startAddr[ addr ];
}

template< int StartAddr, int EndAddr >
inline unsigned char& MemoryRegion< StartAddr, EndAddr >::byte( int addr )
{
    return _startAddr[ addr ];
}

template< int StartAddr, int EndAddr >
inline const unsigned short& MemoryRegion< StartAddr, EndAddr >::word( int addr ) const
{
    return *static_cast< const unsigned short* >( _startAddr + addr );
}

template< int StartAddr, int EndAddr >
inline unsigned short& MemoryRegion< StartAddr, EndAddr >::word( int addr )
{
    return *static_cast< unsigned short* >( _startAddr + addr );
}

#endif /* defined(__gbemu__memoryRegion__) */
