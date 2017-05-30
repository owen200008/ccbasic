if (WIN32 OR APPLE)
    cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
else()
    cmake_minimum_required(VERSION 2.8.5 FATAL_ERROR)
endif()

project(basiclibTest)

option(BASICLIBTEST_BUILD_TEST "Build BasicLibTest Exe" ON)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/libs)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/libs)

#build basiclib
add_subdirectory(${PROJECT_SOURCE_DIR}/src)
#build libevent
set(BUILD_TESTING OFF CACHE BOOL "flags")
set(EVENT__DISABLE_BENCHMARK ON CACHE BOOL "flags")
set(EVENT__DISABLE_OPENSSL ON CACHE BOOL "flags")
set(EVENT__DISABLE_REGRESS ON CACHE BOOL "flags")
set(EVENT__DISABLE_SAMPLES ON CACHE BOOL "flags")
set(EVENT__DISABLE_TESTS ON CACHE BOOL "flags")
set(EVENT__DISABLE_THREAD_SUPPORT ON CACHE BOOL "flags")
#add_subdirectory(${PROJECT_SOURCE_DIR}/3rd/libevent)
#build cryptopp
set(BUILD_TESTING OFF CACHE BOOL "flags")
set(BUILD_SHARED OFF CACHE BOOL "flags")
add_subdirectory(${PROJECT_SOURCE_DIR}/3rd/cryptopp)
#build sproto
add_subdirectory(${PROJECT_SOURCE_DIR}/src/scbasic/sproto)

#include
include_directories(${PROJECT_SOURCE_DIR}/src/inc)
include_directories(${PROJECT_SOURCE_DIR}/3rd/libevent/include)
include_directories(${PROJECT_SOURCE_DIR}/3rd/cryptopp)
#define
set(BasicLibTestDefinitions)
set(RunProjectCXX_FLAGS ${CMAKE_CXX_FLAGS})
set(RunProjectC_FLAGS ${CMAKE_C_FLAGS})
#link
set(RunProjectLinks)

if(MSVC)
	include_directories(${PROJECT_SOURCE_DIR}/3rd/libevent/build/include)
	set(RunProjectCXX_FLAGS "${RunProjectCXX_FLAGS} /MP")
	set(RunProjectC_FLAGS "${RunProjectC_FLAGS} /MP")
	set(BasicLibTestDefinitions ${BasicLibTestDefinitions} -DUNICODE -D_UNICODE -D__BASICWINDOWS)
	set(RunProjectLinks ${RunProjectLinks} ${PROJECT_SOURCE_DIR}/libs/FLzlib13.lib 
		${PROJECT_SOURCE_DIR}/3rd/libevent/build/lib/Debug/event.lib)
else()
	set(RunProjectCXX_FLAGS "${RunProjectCXX_FLAGS} --std=c++11")
	set(BasicLibTestDefinitions ${BasicLibTestDefinitions} -DLINUX -D__LINUX)
	set(RunProjectLinks ${RunProjectLinks} ${PROJECT_SOURCE_DIR}/3rd/libevent/.libs/libevent.a)
endif()

#define
add_definitions(${BasicLibTestDefinitions})
set(RunProjectLinks ${RunProjectLinks} basiclib cryptopp-static)
set(CMAKE_CXX_FLAGS ${RunProjectCXX_FLAGS} CACHE  string "..." FORCE)
set(CMAKE_C_FLAGS ${RunProjectC_FLAGS} CACHE string "..." FORCE)
#build test
if(BASICLIBTEST_BUILD_TEST)
	if (BASICLIB_BUILD_STATIC)
		set(RunProject_SOURCES
		${PROJECT_SOURCE_DIR}/src/scbasic/commu/basicclient.h
		${PROJECT_SOURCE_DIR}/src/scbasic/commu/ipverify.h
		${PROJECT_SOURCE_DIR}/src/scbasic/commu/servertemplate.h
		${PROJECT_SOURCE_DIR}/src/scbasic/encode/rsaencode.h
		${PROJECT_SOURCE_DIR}/basic_test/comm/functionxiaolvtest.h
		${PROJECT_SOURCE_DIR}/basic_test/coroutine/coroutinetest.h
		${PROJECT_SOURCE_DIR}/basic_test/dll/basictesthead.h
		${PROJECT_SOURCE_DIR}/basic_test/dll/dlltest.h
		${PROJECT_SOURCE_DIR}/basic_test/exception/stackwalkcheck.h
		${PROJECT_SOURCE_DIR}/basic_test/headdefine.h
		${PROJECT_SOURCE_DIR}/basic_test/misc/fastdelegatetest.h
		${PROJECT_SOURCE_DIR}/basic_test/net/nettest.h
		${PROJECT_SOURCE_DIR}/basic_test/thread/threadtest.h
		${PROJECT_SOURCE_DIR}/basic_test/time/ontimertest.h
		${PROJECT_SOURCE_DIR}/basic_test/util/containerexttest.h
		${PROJECT_SOURCE_DIR}/src/scbasic/net/net.h
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
		set(TotalSourceData ${RunProject_SOURCES})
		source_group_by_dir(TotalSourceData)
		add_executable(basicTest ${RunProject_SOURCES}) 
		target_link_libraries(basicTest ${RunProjectLinks})
		if(MSVC)
			set_target_properties(basicTest PROPERTIES LINK_FLAGS "/SAFESEH:NO")
			add_custom_command(TARGET basicTest POST_BUILD COMMAND xcopy /Y /Q "\"${PROJECT_SOURCE_DIR}\\libs\\FLzlib13.dll\"" "\"$(OutDir)\"")
			add_custom_command(TARGET basicTest POST_BUILD COMMAND xcopy /Y /Q "\"${PROJECT_SOURCE_DIR}\\3rd\\libevent\\build\\bin\\Debug\\event.dll\"" "\"$(OutDir)\"" )
		endif()
	endif()
endif()


