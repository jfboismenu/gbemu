cmake_minimum_required(VERSION 2.6)
project(gbemu)
find_package(OpenGL REQUIRED)

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
        message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

# Glut is not standard on Windows, so use the copy provided in the repo.
if (WIN32)
	include_directories( "thirdparties/glut/includes" )
	set(GLUT_LIBRARY "thirdparties/glut/lib/freeglut_static")
	add_definitions(-DFREEGLUT_STATIC)
else (WIN32)
	find_package(GLUT REQUIRED)
endif (WIN32)

include_directories( ${OPENGL_INCLUDE_DIRS}  ${GLUT_INCLUDE_DIRS} )

add_executable(gbemu bootRom.cpp cartridgeInfo.cpp cpu.cpp debugging.cpp gameboy.cpp gbemu.cpp glutapp.cpp mbc.cpp memory.cpp memoryRegion.cpp opcode.cpp papu.cpp register.cpp registers.cpp videoDisplay.cpp timers.cpp)

target_link_libraries(gbemu ${OPENGL_LIBRARIES} ${GLUT_LIBRARY})
