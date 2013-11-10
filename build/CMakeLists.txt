cmake_minimum_required(VERSION 2.6)
project(gbemu)
find_package(OpenGL REQUIRED)

# Glut is not standard on Windows, so use the copy provided in the repo.
if (WIN32)
	include_directories( "../thirdparties/glut/includes" )
	set(GLUT_LIBRARY "../thirdparties/glut/lib/freeglut_static")
	add_definitions(-DFREEGLUT_STATIC)
else (WIN32)
	find_package(GLUT REQUIRED)
endif (WIN32)

include_directories( ${OPENGL_INCLUDE_DIRS}  ${GLUT_INCLUDE_DIRS} )

add_executable(gbemu ../bootRom.cpp ../cartridgeInfo.cpp ../cpu.cpp ../debugging.cpp ../gameboy.cpp ../gbemu.cpp ../glutapp.cpp ../mbc.cpp ../memory.cpp ../memoryRegion.cpp ../opcode.cpp ../papu.cpp ../register.cpp ../registers.cpp ../videoDisplay.cpp)

target_link_libraries(gbemu ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} )

install_targets (../output gbemu)
