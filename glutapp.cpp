//
//  glutapp.cpp
//  gbemu
//
//  Created by JF Boismenu on 2012-10-09.
//
//

// gbemu.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include <iostream>
#include <cassert>
#include <iomanip>
#include <sstream>
#include <memory>
#include <fstream>

#ifdef _WINDOWS
#include <Windows.h>
#endif
#ifdef _WINDOWS
#include <gl/glu.h>
#include <gl/glut.h>
#else
#include <GLUT/GLUT.h>
#endif
#include <set>

#include "common.h"
#include "cartridgeInfo.h"
#include "debugging.h"
#include "memory.h"
#include "cpu.h"
#include "videoDisplay.h"
#include "gameboy.h"
#include "gbemu.h"

namespace {

    using namespace gbemu;

    Gameboy* gbInstance;

    std::string getGameTitle( const Cartridge& cart )
    {
        static const unsigned short kMaxNameLength = 16;
        static const unsigned short kNameAddrStart = 0x134;
        static const unsigned short kNameAddrEnd = kMaxNameLength + kNameAddrStart;
        std::string result;
        result.reserve( kMaxNameLength );

        for ( unsigned short i = kNameAddrStart; cart.getByte( i ) != '\0' && i < kNameAddrEnd; ++i ) {
            result.push_back( (char)cart.getByte( i ) );
        }
        return result;
    }

    void printCartridgeInfo(
        const Cartridge& cart
    )
    {
        static const int kColumnWidth = 25;
        std::cout << std::setw( kColumnWidth ) << "Game Title : " << getGameTitle( cart ) << std::endl;
        std::cout << std::setw( kColumnWidth ) << "Cartridge Type : " << cartridgeInfo::cartridgeTypeToString( cart.getType() ) << std::endl;
        std::cout << std::setw( kColumnWidth ) << "ROM Size : " << cart.getROMSize() << " bytes" << std::endl;
        std::cout << std::setw( kColumnWidth ) << "RAM Size : " << cart.getRAMSize() << " bytes" << std::endl;
        std::cout << std::setw( kColumnWidth ) << "GameBoy Color Flag : " << ( cartridgeInfo::isGameBoyColor( cart ) ? "Yes" : "No" ) << std::endl;
        std::cout << std::setw( kColumnWidth ) << "Super GameBoy Flag : " << ( cartridgeInfo::isSuperGameBoy( cart ) ? "Yes" : "No" ) << std::endl;
    }

    void emulator()
    {
        if ( emulateSomeCycles( *gbInstance, 70224 ) ) {
            glutPostRedisplay();
        }
    }
    
    bool downPressed = false;
    bool upPressed = false;
    bool leftPressed = false;
    bool rightPressed = false;
    bool selectPressed = false;
    bool startPressed = false;
    bool aPressed = false;
    bool bPressed = false;
    
    void doButtons(
        const unsigned char key,
        const bool          setTo )
    {
        bool allButtonsPressed = toupper( key ) == 'R';
        if ( toupper( key ) == 'Z' || allButtonsPressed ) {
            bPressed = setTo;
        }
        if ( toupper( key ) == 'X' || allButtonsPressed ) {
            aPressed = setTo;
        }
        if ( toupper( key ) == 'A' || allButtonsPressed ) {
            selectPressed = setTo;
        }
        if ( toupper( key ) == 'S' || allButtonsPressed ) {
            startPressed = setTo;
        }
    }
    
    void doCross(
        const int  key,
        const bool setTo
    )
    {
        if ( key == GLUT_KEY_LEFT ) {
            leftPressed = setTo;
        }
        if ( key == GLUT_KEY_RIGHT ) {
            rightPressed = setTo;
        }
        if ( key == GLUT_KEY_UP ) {
            upPressed = setTo;
        }
        if ( key == GLUT_KEY_DOWN ) {
            downPressed = setTo;
        }
    }
    
    void writeStateToMemory()
    {
        const unsigned char keyState = GetMask(
            startPressed, // START
            selectPressed, // SELECT
            bPressed, // B
            aPressed, // A
            downPressed, // DOWN
            upPressed, // UP
            leftPressed, // LEFT
            rightPressed ); // RIGHT
        gbInstance->getMemory().setKeyState( keyState );
    }
    
    void keyboardDown(
        const unsigned char key,
        const int,
        const int
    )
    {
        doButtons( key, true );
        writeStateToMemory();
    }
    
    void keyboardUp(
        const unsigned char key,
        const int,
        const int
    )
    {
        doButtons( key, false );
        writeStateToMemory();
    }
    
    void specialDown(
        const int key,
        const int,
        const int
    )
    {
        doCross( key, true );
        writeStateToMemory();
    }
    
    void specialUp(
        const int key,
        const int,
        const int
    )
    {
        doCross( key, false );
        writeStateToMemory();
    }

    GLuint displayTexture;

    void render(void) 
    {
        // emulation might be a faster than it should be
        // in this case, complete the draw-emu cycle by sleeping a bit.
//        static timeval t1;
//        static bool firstFrame = true;
//        double elapsedTime;
//        
//        const double fps = 60.;
//
//        if ( !firstFrame ) {
//            timeval t2;
//            gettimeofday(&t2, NULL);
//            // compute and print the elapsed time in millisec
//            elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;      // sec to ms
//            elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;   // us to ms
//            
//            const double timeToSleep = ( 1000.0 / fps ) - elapsedTime;
//            if ( timeToSleep > 0 ) {
//                usleep( static_cast< unsigned int >( timeToSleep * 1000 ) );
//            }
//        }
//        // start timer
//        gettimeofday(&t1, NULL);
//        
//        firstFrame = false;
//        glClear( GL_COLOR_BUFFER_BIT );
        const Color* pixels = gbInstance->getVideo().getPixels();

        glBindTexture(GL_TEXTURE_2D, displayTexture);
        JFX_CMP_ASSERT( glGetError(), ==, GL_NO_ERROR );
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB,
            160, 144, 0,
            GL_RGB,
            GL_UNSIGNED_BYTE, pixels );
        JFX_CMP_ASSERT( glGetError(), ==, GL_NO_ERROR );
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST); // Linear Filtering
        JFX_CMP_ASSERT( glGetError(), ==, GL_NO_ERROR );
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST); // Linear Filtering
        JFX_CMP_ASSERT( glGetError(), ==, GL_NO_ERROR );
        glColor3f( 1, 1, 1 );
        glBegin( GL_QUADS );
                
            glTexCoord2f( 0.f, 1.f );
            glVertex3f( -1.f, -1.f, 0.f );

            glTexCoord2f( 1.f, 1.f );
            glVertex3f( 1.f, -1.f, 0.f );

            glTexCoord2f( 1.f, 0.f );
            glVertex3f( 1.f, 1.f, 0.f );

            glTexCoord2f( 0.f, 0.f );
            glVertex3f( -1.f, 1.f, 0.f );

        glEnd();

        glFlush();
    }
    

    
//    void staticUnitTest()
//    {
//        struct SweepTraits
//        {
//        public:
//            bool isSweepingEnabled() const
//            {
//                return _time != 0 || _shift == 0;
//            }
//            unsigned char getTime() const
//            {
//                return _time;
//            }
//            bool isDecreasing() const
//            {
//                return _direction == 1;
//            }
//            unsigned char getShift() const
//            {
//                return _shift;
//            }
//        private:
//            unsigned char _shift : 3;
//            unsigned char _direction : 1;
//            unsigned char _time : 3;
//            unsigned char _unused : 1;
//        };
//        
//        Register< SweepTraits > sweepRegister( GetMask( 1, 0, 1, 0, 1, 1, 1, 0 ) );
//        assert( sweepRegister.bits.getTime() == 2 );
//        assert( sweepRegister.bits.isDecreasing() );
//        assert( sweepRegister.bits.getShift() == 6 );
//        
//        static_assert( sizeof( sweepRegister ) == 1, "Wrong size!" );
//    }
}

/*#ifdef _WINDOWS
int _tmain(int argc, char* argv[])
#else*/
int main(int argc, char* argv[])
//#endif
{
    if ( argc == 1 ) {
        std::cout << "Missing cartridge name" << std::endl;
        return 0;
    }
    std::unique_ptr< Gameboy > gbInstanceGuard(
        gbemu::initGlobalEmulatorParams( argv[ 1 ], argc == 3 ? argv[ 2 ] : 0 ) );
    gbInstance = gbInstanceGuard.get();
    
    printCartridgeInfo( gbInstance->getCartridge() );

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA);
    glutInitWindowPosition(1920/2,1200/2);
    glutInitWindowSize(160*3,144*3 );
    glutCreateWindow( "GBEmu" );
    glEnable( GL_TEXTURE_2D );
    glGenTextures( 1, &displayTexture );
    JFX_CMP_ASSERT( glGetError(), ==, GL_NO_ERROR );
    glutDisplayFunc( render );
    glutIdleFunc( emulator );
    glutKeyboardFunc( keyboardDown );
    glutKeyboardUpFunc( keyboardUp );
    glutSpecialFunc( specialDown );
    glutSpecialUpFunc( specialUp );
    glutMainLoop();
	return 0;
}
