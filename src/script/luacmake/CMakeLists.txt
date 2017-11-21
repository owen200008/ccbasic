cmake_minimum_required(VERSION 2.8.5)

project(lualib)

set(BasicLibSrcPath ${PROJECT_SOURCE_DIR}/../../)

#include 
include_directories(${BasicLibSrcPath}/../3rd/lua/src)

#extra src
set(BasicLibExtra_Src)
#flags
set(BasicLibCMakeCXX_FLAGS ${CMAKE_CXX_FLAGS})
set(BasicLibCMakeC_FLAGS ${CMAKE_C_FLAGS})
#define
set(BasicLibDefinitions -DLUA_BUILD_AS_DLL)
#link
set(BasicLibLinks)

if(MSVC)
	set(BasicLibCMakeCXX_FLAGS "${BasicLibCMakeCXX_FLAGS} /MP")
	set(BasicLibCMakeC_FLAGS "${BasicLibCMakeC_FLAGS} /MP")
else()
	set(BasicLibCMakeCXX_FLAGS "${BasicLibCMakeCXX_FLAGS} --std=c++11 -frtti -fexceptions")
endif()

#define
add_definitions(${BasicLibDefinitions})
#flags
set(CMAKE_CXX_FLAGS ${BasicLibCMakeCXX_FLAGS})
set(CMAKE_C_FLAGS ${BasicLibCMakeC_FLAGS})

#src
aux_source_directory(${BasicLibSrcPath}/../3rd/lua/src basiclib_SOURCES)
list(REMOVE_ITEM basiclib_SOURCES ${BasicLibSrcPath}/../3rd/lua/src/lua.c)
list(REMOVE_ITEM basiclib_SOURCES ${BasicLibSrcPath}/../3rd/lua/src/luac.c)

set(TotalSourceData ${basiclib_SOURCES} ${BasicLibExtra_Src})
source_group_by_dir(TotalSourceData)

add_library(lualib SHARED ${basiclib_SOURCES} ${BasicLibExtra_Src})











