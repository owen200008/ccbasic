cmake_minimum_required(VERSION 2.8.5)

project(sprotolib)

set(basiclibdst ${PROJECT_SOURCE_DIR}/../../../)

include_directories(${basiclibdst}/src/inc)
include_directories(${basiclibdst}/3rd/lua/src)
if(MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP" CACHE STRING "flags" FORCE)
	set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} /MP" CACHE STRING "flags" FORCE)
	add_definitions(-DUNICODE -D_UNICODE -D__BASICWINDOWS -D_XKEYCHECK_H)
else()
	add_definitions(-DLINUX -D__LINUX)
endif()
add_library(sproto SHARED lsproto.cpp sproto.cpp) 
if(MSVC)
	target_link_libraries(sproto basicdll ${basiclibdst}/libs/lua53.lib)
endif()
