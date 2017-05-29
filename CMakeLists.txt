if (WIN32 OR APPLE)
    cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
else()
    cmake_minimum_required(VERSION 2.8.5 FATAL_ERROR)
endif()

project(basiclibTest)

option(BASICLIBTEST_BUILD_TEST "Build BasicLibTest Exe" ON)

#build basiclib
add_subdirectory(${PROJECT_SOURCE_DIR}/src ${PROJECT_SOURCE_DIR}/src/build)
#build cryptopp
set(BUILD_TESTING OFF CACHE BOOL "flags")
add_subdirectory(${PROJECT_SOURCE_DIR}/3rd/cryptopp ${PROJECT_SOURCE_DIR}/3rd/cryptopp/build)
#build sproto
add_subdirectory(${PROJECT_SOURCE_DIR}/src/scbasic/sproto ${PROJECT_SOURCE_DIR}/3rd/src/scbasic/sproto/build)

#include
include_directories(${PROJECT_SOURCE_DIR}/src/inc)
include_directories(${PROJECT_SOURCE_DIR}/3rd/libevent/include)
include_directories(${PROJECT_SOURCE_DIR}/3rd/cryptopp)
#define
set(BasicLibTestDefinitions)
set(BasicLibTestCMakeCXX_FLAGS ${CMAKE_CXX_FLAGS})
#link
set(BasicLibTestLinks)
#command
set(BasicLibTestCommand)

if(MSVC)
	include_directories(${PROJECT_SOURCE_DIR}/3rd/libevent/WIN32-Code/nmake)
	set(BasicLibTestCMakeCXX_FLAGS "${BasicLibTestCMakeCXX_FLAGS} /MP")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP")
	set(BasicLibTestDefinitions ${BasicLibTestDefinitions} -DUNICODE -D_UNICODE -D__BASICWINDOWS)
	set(BasicLibTestLinks ${BasicLibTestLinks} ${PROJECT_SOURCE_DIR}/libs/FLzlib13.lib 
		${PROJECT_SOURCE_DIR}/3rd/libevent/libevent.lib)
	set(BasicLibTestCommand ${BasicLibTestCommand} "${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/libs/FLzlib13.dll ${CMAKE_BINARY_DIR}")
else()
	set(BasicLibTestCMakeCXX_FLAGS "${BasicLibTestCMakeCXX_FLAGS} --std=c++11")
	set(BasicLibTestDefinitions ${BasicLibTestDefinitions} -DLINUX -D__LINUX)
	set(BasicLibTestLinks ${BasicLibTestLinks} ${PROJECT_SOURCE_DIR}/3rd/libevent/.libs/libevent.a)
endif()

#define
add_definitions(${BasicLibTestDefinitions})
set(BasicLibTestLinks ${BasicLibTestLinks} basiclib cryptopp-static)
set(CMAKE_CXX_FLAGS ${BasicLibTestCMakeCXX_FLAGS})
#build test
if(BASICLIBTEST_BUILD_TEST)
	if (BASICLIB_BUILD_STATIC)
		set(basicTest_SOURCES
		${PROJECT_SOURCE_DIR}/src/scbasic/net/net.cpp
		${PROJECT_SOURCE_DIR}/src/scbasic/net/sendbuffer.cpp
		${PROJECT_SOURCE_DIR}/src/scbasic/commu/basicclient.cpp
		${PROJECT_SOURCE_DIR}/src/scbasic/commu/ipverify.cpp
		${PROJECT_SOURCE_DIR}/src/scbasic/commu/servertemplate.cpp
		${PROJECT_SOURCE_DIR}/src/scbasic/encode/rsaencode.cpp
		${PROJECT_SOURCE_DIR}/basic_test/basic_Test.cpp
		${PROJECT_SOURCE_DIR}/basic_test/coroutine/coroutinetest.cpp
		${PROJECT_SOURCE_DIR}/basic_test/dll/basictesthead.cpp
		${PROJECT_SOURCE_DIR}/basic_test/net/nettest.cpp
			)

		add_executable(basicTest ${basicTest_SOURCES}) 
		target_link_libraries(basicTest ${BasicLibTestLinks})
		if(MSVC)
			set_target_properties(basicTest PROPERTIES LINK_FLAGS "/SAFESEH:NO")
		endif()
		foreach(f ${BasicLibTestCommand})
			add_custom_command(TARGET basicTest POST_BUILD COMMAND ${f})
		endforeach()
	endif()
endif()


