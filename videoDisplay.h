#pragma once

#include <array>

namespace gbemu {

    class Memory;

    struct Color
    {
        Color()
        {}

        Color( unsigned char r, unsigned char g, unsigned char b )
        {
            components[ 0 ] = r;
            components[ 1 ] = g;
            components[ 2 ] = b;
        }
        inline bool operator==( const Color& color ) const
        {
            for( int i = 0; i < 3; ++i ) {
                if ( components[ i ] != color.components[ i ] ) {
                    return false;
                }
            }
            return true;
        }
        inline bool operator!=( const Color& color )
        {
            return !operator==( color );
        }
    private:
        unsigned char components[3];
    };

    class VideoDisplay
    {
    public:
        VideoDisplay( Memory& memory );
        void emulate( int nbCycles );
        bool isFrameReady() const;
        const Color* getPixels() const;
    private:
        void drawTiles(
            const int                     scx,
            const int                     scy,
            const int                     y,
            const unsigned short          bgTileMapStart,
            const bool                    dataSelect,
            const unsigned short          tileTableStart,
            const std::array< Color, 4 >& bgPalette,
            const unsigned char           offsetX,
            const unsigned char           offsetY
        );

        VideoDisplay& operator=( const VideoDisplay& );

        void computeLine( int y, int scx, int scy, unsigned char wx, unsigned char wy );
        void setLCDCInterruptFlag();

        const static int kMode0Start = 0;
        const static int kMode2Start = 204;
        const static int kMode3Start = kMode2Start + 80;
        const static int k023ModeCycleLength = kMode3Start + 172;

        const static int kLCDCycleLength = 70224;
        const static int kVBlankLength = 4560;
        const static int kVBlankStart = kLCDCycleLength - kVBlankLength;

        const static int kLCDEnabledBit = 1 << 7;

        Memory& _memory;
        int _lcdCycle;
        mutable bool _isFrameReady;

        Color _pixels[ 144 ][ 160 ];
    };
}
