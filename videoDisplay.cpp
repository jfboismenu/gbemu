#include "videoDisplay.h"
#include "memory.h"
#include "cpu.h"
#include <array>

namespace gbemu {

    VideoDisplay::VideoDisplay( 
        Memory& memory 
    ) : _memory( memory ),
        _lcdCycle( 0 ),
        _isFrameReady( false )
    {
        memset( _pixels, 0, sizeof( _pixels ) );
    }

    Color shades[ 4 ] = { Color( 252,  232,  160 ),
                          Color( 220, 180, 92 ),
                          Color( 152, 124, 60 ),
                          Color( 76,  60,  28 ) };

    void decodePalette(
        std::array< Color, 4 >& colors,
        const unsigned char     encodedPalette
    )
    {
        colors[ 0 ] = shades[ encodedPalette & 0x3 ];
        colors[ 1 ] = shades[ ( encodedPalette >> 2 ) & 0x3 ];
        colors[ 2 ] = shades[ ( encodedPalette >> 4 ) & 0x3 ];
        colors[ 3 ] = shades[ ( encodedPalette >> 6 ) & 0x3 ];
    }

    void VideoDisplay::drawTiles(
      const int                     scx,
      const int                     scy,
      const int                     y,
      const unsigned short          tileMapStart,
      const bool                    dataSelect,
      const unsigned short          tileTableStart,
      const std::array< Color, 4 >& bgPalette,
      const unsigned char           offsetX,
      const unsigned char           offsetY
    )
    {
        const unsigned char backgroundLine = static_cast< unsigned char >( scy + y - offsetY );
        const unsigned char tileLine = backgroundLine % 8;
        // For every pixel on the scanline
        for ( unsigned char x = offsetX; x < 160; ) {
            // Which pixel from the background are we diplaying now?
            unsigned char backgroundPixel = static_cast< unsigned char >( scx + x - offsetX );

            // Find out which tile this pixel falls into
            const unsigned short tileMapSlotAddr = tileMapStart + ( 32 * ( backgroundLine / 8 ) ) + ( backgroundPixel / 8 );
            JFX_CMP_ASSERT( tileMapSlotAddr, <, tileMapStart+ 0x400 );
            JFX_CMP_ASSERT( tileMapSlotAddr, >=, 0 );
            JFX_CMP_ASSERT( tileMapSlotAddr, <, 65536 );

            // Read the tile index from the tile map
            int tileIndex;
            if ( dataSelect ) {
                tileIndex = _memory.readByte( tileMapSlotAddr );
            }
            else {
                tileIndex = static_cast< char >( _memory.readByte( tileMapSlotAddr ) );
            }

            // Compute the location of the tile
            const unsigned short tileAddr = (unsigned short)(tileTableStart + ( tileIndex * 16 ));
            // Get the pointer to the bytes of that tile
            const unsigned short tileLineBytes( _memory.readWord( tileAddr + tileLine * 2 ) );

            // Make sure we are drawing on screen.
            JFX_CMP_ASSERT( y, >=, 0 );
            JFX_CMP_ASSERT( y, <, 144 );
            JFX_CMP_ASSERT( x, >=, 0 );
            JFX_CMP_ASSERT( x, <, 160 );
            
            const int pixelsToDraw = 8 - ( backgroundPixel % 8 );
            for ( int i = 0; i < pixelsToDraw; ++x, ++i, ++backgroundPixel ) {
               unsigned char colorIndex = static_cast< unsigned char >(
                  getBit( ( tileLineBytes & 0xFF ), 7 - ( backgroundPixel % 8 ) ) +
                  ( getBit( ( tileLineBytes & 0xFF00 ) >> 8, 7 - ( backgroundPixel % 8 ) ) << 1 ) );

               _pixels[ y ][ x ] = bgPalette[ colorIndex ];
            }
        }
    }

    void VideoDisplay::computeLine( int y, int scx, int scy, unsigned char wx, unsigned char wy )
    {
        JFX_CMP_ASSERT( y, >=, 0 );
        JFX_CMP_ASSERT( y, <, 144 );
        const unsigned char lcdc = _memory.memoryRegister( kLCDC  );
        bool isTile8x16 = getBit( lcdc, 2 );

        const bool dataSelect = ( lcdc & ( 1 << 4 ) ) != 0;
        const bool bgMapDataSelect = ( lcdc & ( 1 << 3 ) ) != 0;
        const bool windowMapDataSelect = ( lcdc & ( 1 << 6 ) ) != 0;

        // Address of tile index 0.
        const std::array< unsigned short, 2 > kTileTableStart{ { 0x9000, 0x8000 } };
        // 32x32 tile grid start address
        const std::array< unsigned short, 2 > kTileMapStart{ { 0x9800, 0x9C00 } };

        std::array< Color, 4 > bgPalette;
        std::array< Color, 4 > sprite0Palette;
        std::array< Color, 4 > sprite1Palette;
        decodePalette( bgPalette, _memory.memoryRegister( kBGP ) );
        decodePalette( sprite0Palette, _memory.memoryRegister( kOBP0 ) );
        decodePalette( sprite1Palette, _memory.memoryRegister( kOBP1 ) );
        
        drawTiles(
           scx, scy, y, kTileMapStart[ bgMapDataSelect ], dataSelect,
           kTileTableStart[ dataSelect ], bgPalette, 0, 0
        );
        if ( getBit( lcdc, 5 ) ) {
            if ( wy <= 143 && wx <= 166 && wy <= y ) {
                drawTiles(
                    0, 0, y, kTileMapStart[ windowMapDataSelect ], dataSelect,
                    kTileTableStart[ dataSelect ], bgPalette, (unsigned char)std::max( 0, wx - 7 ), wy );
            }
        }
        
        if ( getBit( lcdc, 1 ) ) {
            unsigned short addr = 0xfe00;
            while( addr < 0xfea0 ) {
                int spriteY = _memory.readByte( addr++ );
                int spriteX = _memory.readByte( addr++ );
                const unsigned char spriteIndex = _memory.readByte( addr++ );
                const unsigned char spriteAttr = _memory.readByte( addr++ );
            
                if ( spriteX == 0 || spriteY == 0 ||
                    spriteX >= 168 || spriteY >= 160 ) 
                {
                    // sprite hidden, continue
                    continue;    
                }  

                spriteY -= 16;
                spriteX -= 8;

                const int spriteHeight = isTile8x16 ? 16 : 8;
                // If that's sprite sits on the scanline
                if ( between( spriteY, spriteY + spriteHeight, y ) ) {
                    int spriteLine = y - spriteY;
                    JFX_CMP_ASSERT( spriteLine, >=, 0 );

                    // If the sprite is flipped on the Y axis, flip the spriteLine
                    if ( getBit( spriteAttr, 6 ) ) {
                        spriteLine = (spriteHeight - 1) - spriteLine;
                    }

                    // for each pixel on the y axis
                    for ( unsigned char i = 0; i < 8; ++i ) {
                        // If that pixel is outside the screen, skip it
                        if ( !between( 0, 160, spriteX + i ) ) {
                            continue;
                        }

                        unsigned short spriteLineBytes;
                        if ( isTile8x16 ) {
                            if ( spriteLine < 8 ) {
                                // Select sprite
                                const unsigned short wordAddr = (unsigned short)(0x8000 + ( ( spriteIndex & 0xFE ) * 16 ) + ( spriteLine * 2 ));
                                spriteLineBytes = _memory.readWord( wordAddr );
                            }
                            else {
                                const unsigned short wordAddr = (unsigned short)(0x8000 + ( ( spriteIndex | 0x1 ) * 16 ) + ( ( spriteLine - 8 ) * 2 ));
                                spriteLineBytes = _memory.readWord( wordAddr );
                            }
                        }
                        else {
                            const unsigned short wordAddr = (unsigned short)(0x8000 + spriteIndex * 16 + ( spriteLine * 2 ));
                            spriteLineBytes = _memory.readWord( wordAddr );
                        }

                        // compute the bit we have to read. Bit 0 is the rightmost pixel.
                        // If we are mirroring, we start from bit 0 to display pixels
                        const unsigned int bitPos = getBit( spriteAttr, 5 ) ? i : 7 - i;

                        const size_t colorIndex = (size_t)(getBit( spriteLineBytes & 0xFF, bitPos ) +
                            ( getBit( ( ( spriteLineBytes & 0xFF00 ) >> 8 ), bitPos ) << 1 ));

                        // if object over background
                        if ( !getBit( spriteAttr, 7 ) ) {
                            if ( colorIndex == 0 ) {
                                continue;
                            }
                        }
                        else {
                            if ( _pixels[ y ][ spriteX + i ] != bgPalette[ 0 ] ) {
                                continue;
                            }
                        }
                        
                        _pixels[ y ][ spriteX + i ] = getBit( spriteAttr, 4 ) ? sprite1Palette[ colorIndex ] : sprite0Palette[ colorIndex ];
                    }
                }

            }
            JFX_CMP_ASSERT( addr, ==, 0xfea0 );
        }
    }

    void VideoDisplay::emulate( int nbCycles )
    {
        _lcdCycle += nbCycles;
        unsigned char stat( _memory.memoryRegister( kSTAT ) );
        unsigned char mode( ( stat & 0x03 ) );
        int ly;
        // If display is off, no need to update the lcd values
        if ( ( _memory.memoryRegister( kLCDC ) & kLCDEnabledBit ) == 0 ) {
            _lcdCycle = 0;
            mode = 0;
            ly = 0;
        }
        else {
            if ( _lcdCycle >= kLCDCycleLength ) {
                //_memory._bytes[ kIF ] &= ~kIFVBlankFlag;
                _lcdCycle -= kLCDCycleLength;
            }

            ly = _lcdCycle / k023ModeCycleLength;
            
            // If we are not in VBlank, go through modes one after the otherr
            if ( _lcdCycle < kVBlankStart ) {
                const int modeCycle( _lcdCycle % k023ModeCycleLength );
                // We are in mode 0
                if ( modeCycle < kMode2Start ) {
                    if ( mode != 0 ) {
                        if(  getBit( _memory.memoryRegister( kSTAT ), 3 ) ) {
                            setLCDCInterruptFlag();
                        }
                        if ( _memory.memoryRegister( kLYC ) == ly ) {
                            setBit( stat, 2 );
                            if ( getBit( _memory.memoryRegister( kSTAT ), 6 ) ) {
                                setLCDCInterruptFlag();
                            }
                        }
                        else {
                            resetBit( stat, 2 );
                        }
                    }
                    mode = 0;
                }
                else if ( modeCycle < kMode3Start ) {
                    if ( mode != 2 && getBit( _memory.memoryRegister( kSTAT ), 5 ) ) {
                        setLCDCInterruptFlag();
                    }
                    mode = 2;
                }
                else if ( modeCycle < k023ModeCycleLength ) {
                    // If we are entering mode 3, we have to draw the lcd line ly.
                    if ( mode != 3 ) {
                        mode = 3;
                        computeLine( ly,
                            _memory.memoryRegister( kSCX ),
                            _memory.memoryRegister( kSCY ),
                            _memory.memoryRegister( kWX ),
                            _memory.memoryRegister( kWY ) );
                    }
                }
                else {
                    JFX_MSG_ASSERT( "Corrupted STAT mode." );
                }
            }
            // we are in vblank-mode, so do something about it
            else {
                if ( mode != 1 ) {
                    _isFrameReady = true;
                    // set vblank interrupt flag
                    _memory.memoryRegister( kIF ) |= Memory::kIFVBlankFlag;
                    if ( mode != 2 && getBit( _memory.memoryRegister( kSTAT ), 4 ) ) {
                        setLCDCInterruptFlag();
                    }
                }
                mode = 1;
            }
        }

        stat = ( stat & 0xfc ) | mode | 0x80;
        _memory.memoryRegister( kLY ) = static_cast< unsigned char >( ly );
        JFX_CMP_ASSERT( ly, <, 154 );
        _memory.memoryRegister( kSTAT ) = stat;
    }

    void VideoDisplay::setLCDCInterruptFlag()
    {
        setBit( _memory.memoryRegister( kIF ), 1 );
    }

    bool VideoDisplay::isFrameReady() const
    {
        if ( _isFrameReady ) {
            _isFrameReady = false;
            return true;
        }
        return false;
    }

    const Color* VideoDisplay::getPixels() const
    {
        return &( _pixels[ 0 ][ 0 ] );
    }
}
