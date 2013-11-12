#pragma once

#include <array>
#include "common.h"

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

    class VideoDisplay : public WordIOProtocol< VideoDisplay >
    {
    public:
        static bool isVideoRAM(unsigned short addr);
        static bool isOAM(unsigned short addr);
        static bool isVideoMemory(unsigned short addr);

        VideoDisplay( Memory& memory, bool isInitialized );
        void emulate( int nbCycles );
        bool isFrameReady() const;
        const Color* getPixels() const;
        virtual void writeByte(unsigned short addr, unsigned char byte);
        virtual unsigned char readByte(unsigned short addr) const;
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

        unsigned char _lcdc;
        unsigned char _scx;
        unsigned char _scy;
        unsigned char _stat;
        unsigned char _lyc;
        unsigned char _ly;
        unsigned char _oamRegion[ 0XFEA0 - 0xFE00 ];
        unsigned char _dmaRegister;
        unsigned char _bgp;
        unsigned char _obp0;
        unsigned char _obp1;
        unsigned char _wx;
        unsigned char _wy;
        unsigned char _videoRam[ 0xA000 - 0x8000 ];

        Color _pixels[ 144 ][ 160 ];
    };
}
