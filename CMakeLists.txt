if (WIN32 OR APPLE)
    cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
else()
    cmake_minimum_required(VERSION 2.8.5 FATAL_ERROR)
endif()

project(basiclibTest)
#build basiclib
if (BASICLIB_BUILD_SHARED)
    SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR})
	set(BASICLIB_BUILD_LUA ON CACHE BOOL "flags")
else()
	set(BASICLIB_BUILD_LUA OFF CACHE BOOL "flags")
endif()
add_subdirectory(${PROJECT_SOURCE_DIR}/src src)

#define
set(BasicLibTestDefinitions)
set(RunProjectCXX_FLAGS ${CMAKE_CXX_FLAGS})
set(RunProjectC_FLAGS ${CMAKE_C_FLAGS})
#link
set(RunProjectLinks)

if(MSVC)
	set(RunProjectCXX_FLAGS "${RunProjectCXX_FLAGS} /MP")
	set(RunProjectC_FLAGS "${RunProjectC_FLAGS} /MP")
	set(BasicLibTestDefinitions ${BasicLibTestDefinitions} -DUNICODE -D_UNICODE)
	set(RunProjectLinks ${RunProjectLinks})
else()
	set(RunProjectCXX_FLAGS "${RunProjectCXX_FLAGS} --std=c++11 -frtti -fexceptions")
	set(BasicLibTestDefinitions ${BasicLibTestDefinitions})
	set(RunProjectLinks ${RunProjectLinks})
endif()

#define
add_definitions(${BasicLibTestDefinitions})
set(RunProjectLinks ${RunProjectLinks} basiclib cryptopp-static)
if (BASICLIB_BUILD_SHARED)
    set(RunProjectLinks ${RunProjectLinks} event_shared)
else()
    set(RunProjectLinks ${RunProjectLinks} event_static)
endif()
set(CMAKE_CXX_FLAGS ${RunProjectCXX_FLAGS} CACHE  string "..." FORCE)
set(CMAKE_C_FLAGS ${RunProjectC_FLAGS} CACHE string "..." FORCE)
#build test

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
        ${PROJECT_SOURCE_DIR}/basic_test/util/cclockfreequeue.h
        ${PROJECT_SOURCE_DIR}/basic_test/util/cbasicqueuearray.h
		${PROJECT_SOURCE_DIR}/src/scbasic/commu/basicclient.cpp
		${PROJECT_SOURCE_DIR}/src/scbasic/commu/ipverify.cpp
		${PROJECT_SOURCE_DIR}/src/scbasic/commu/servertemplate.cpp
		${PROJECT_SOURCE_DIR}/src/scbasic/encode/rsaencode.cpp
		${PROJECT_SOURCE_DIR}/basic_test/basic_Test.cpp
		${PROJECT_SOURCE_DIR}/basic_test/coroutine/coroutinetest.cpp
		${PROJECT_SOURCE_DIR}/basic_test/dll/basictesthead.cpp
		${PROJECT_SOURCE_DIR}/basic_test/net/nettest.cpp
        ${PROJECT_SOURCE_DIR}/basic_test/scbasic/encode/rsatest.cpp
        ${PROJECT_SOURCE_DIR}/basic_test/scbasic/encode/rsatest.h
        ${PROJECT_SOURCE_DIR}/basic_test/sys/systemperformance.cpp
        ${PROJECT_SOURCE_DIR}/basic_test/sys/systemperformance.h
			)
set(TotalSourceData ${RunProject_SOURCES})
source_group_by_dir(TotalSourceData)
add_executable(basicTest ${RunProject_SOURCES}) 
target_link_libraries(basicTest ${RunProjectLinks})
if (BASICLIB_BUILD_SHARED)
    if(MSVC)
        add_custom_command(TARGET basicTest POST_BUILD COMMAND xcopy /Y /Q "\"${PROJECT_BINARY_DIR}\\src\\libevent\\bin\\${CMAKE_BUILD_TYPE}\\event.dll\"" "\"$(OutDir)\"")
    endif()
endif()

if(MSVC)
    set_target_properties(basicTest PROPERTIES LINK_FLAGS "/SAFESEH:NO")
endif()




