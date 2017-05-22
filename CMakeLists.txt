cmake_minimum_required(VERSION 2.8)

project(basiclib)

function(buildcryptopp)
	set(DISABLE_SSSE3 ON CACHE BOOL "flags")
	add_subdirectory(${PROJECT_SOURCE_DIR}/3rd/cryptopp ${PROJECT_SOURCE_DIR}/3rd/cryptopp/build)
endfunction()

function(buildlibevent)
	set(EVENT__DISABLE_OPENSSL ON CACHE BOOL "flags")
	add_subdirectory(${PROJECT_SOURCE_DIR}/3rd/libevent ${PROJECT_SOURCE_DIR}/3rd/libevent/build)
endfunction()

function(buildsproto)
	add_subdirectory(${PROJECT_SOURCE_DIR}/src/scbasic/sproto ${PROJECT_SOURCE_DIR}/3rd/src/scbasic/sproto/build)
endfunction()

function(buildbasiclib)
	#system set
	add_definitions(-DSQLITE_HAS_CODEC=1 -DSQLITE_SECURE_DELETE -DSQLITE_SOUNDEX -DSQLITE_ENABLE_COLUMN_METADATA )
	if(MSVC)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP")
		add_definitions(-D_USE_SYS_MALLOC -DUNICODE -D_UNICODE -D__BASICWINDOWS)
		include_directories(${PROJECT_SOURCE_DIR}/3rd/libevent/include)
		include_directories(${PROJECT_SOURCE_DIR}/3rd/libevent/WIN32-Code/nmake)
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++11")
		if(BUILD_ANDROID)
			add_definitions(-D_USE_SYS_MALLOC -D__ANDROID -DANDROID)
		else()
			if(BUILD_IOS)
				add_definitions(-D_USE_SYS_MALLOC -D__MAC -DMAC)
			else()
				add_definitions(-DLINUX -D__LINUX)
				include_directories(${PROJECT_SOURCE_DIR}/3rd/jemalloc/include)
			endif()
		endif()
	endif()
	set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/libs)

	set(basiclib_SOURCES ${PROJECT_SOURCE_DIR}/src/inc/basic_func.cpp
	${PROJECT_SOURCE_DIR}/src/algorithm/md5.cpp
    ${PROJECT_SOURCE_DIR}/src/algorithm/sha1/sha1.cpp
    ${PROJECT_SOURCE_DIR}/src/algorithm/xtra.cpp
    ${PROJECT_SOURCE_DIR}/src/algorithm/zip/tlgzip.cpp
    ${PROJECT_SOURCE_DIR}/src/algorithm/zip/tlzipfile.cpp
    ${PROJECT_SOURCE_DIR}/src/coroutine/coroutineplus.cpp
    ${PROJECT_SOURCE_DIR}/src/datastruct/basic_string.cpp
    ${PROJECT_SOURCE_DIR}/src/datastruct/key_value.cpp
    ${PROJECT_SOURCE_DIR}/src/datastruct/pbzk.cpp
    ${PROJECT_SOURCE_DIR}/src/datastruct/stringex.cpp
    ${PROJECT_SOURCE_DIR}/src/debug/debug.cpp
    ${PROJECT_SOURCE_DIR}/src/debug/debug_android.cpp
    ${PROJECT_SOURCE_DIR}/src/debug/debug_linux.cpp
    ${PROJECT_SOURCE_DIR}/src/debug/debug_mac.cpp
    ${PROJECT_SOURCE_DIR}/src/debug/debug_win.cpp
    ${PROJECT_SOURCE_DIR}/src/dll/loaddll.cpp
    ${PROJECT_SOURCE_DIR}/src/exception/call_stack_gcc.cpp
    ${PROJECT_SOURCE_DIR}/src/exception/call_stack_msvc.cpp
    ${PROJECT_SOURCE_DIR}/src/exception/exception.cpp
    ${PROJECT_SOURCE_DIR}/src/exception/exception_linux.cpp
    ${PROJECT_SOURCE_DIR}/src/exception/exception_win.cpp
    ${PROJECT_SOURCE_DIR}/src/exception/StackWalker.cpp
    ${PROJECT_SOURCE_DIR}/src/file/diskfile.cpp
    ${PROJECT_SOURCE_DIR}/src/file/filefind.cpp
    ${PROJECT_SOURCE_DIR}/src/file/fileman.cpp
    ${PROJECT_SOURCE_DIR}/src/file/filenotify.cpp
    ${PROJECT_SOURCE_DIR}/src/file/fileobj.cpp
    ${PROJECT_SOURCE_DIR}/src/file/file_linux.cpp
    ${PROJECT_SOURCE_DIR}/src/file/inifile.cpp
    ${PROJECT_SOURCE_DIR}/src/file/memfile.cpp
    ${PROJECT_SOURCE_DIR}/src/log/log.cpp
    ${PROJECT_SOURCE_DIR}/src/mem/bitstream.cpp
    ${PROJECT_SOURCE_DIR}/src/mem/mem.cpp
    ${PROJECT_SOURCE_DIR}/src/mem/smartbuffer.cpp
    ${PROJECT_SOURCE_DIR}/src/mem/tlstaticbuffer.cpp
    ${PROJECT_SOURCE_DIR}/src/net/net.cpp
    ${PROJECT_SOURCE_DIR}/src/net/sendbuffer.cpp
    ${PROJECT_SOURCE_DIR}/src/sqlite/shell.c
    ${PROJECT_SOURCE_DIR}/src/sqlite/sqlite3db.cpp
    ${PROJECT_SOURCE_DIR}/src/sqlite/sqlite3dbquery.cpp
    ${PROJECT_SOURCE_DIR}/src/sqlite/sqlite3dbtable.cpp
    ${PROJECT_SOURCE_DIR}/src/sqlite/sqlite3secure.c
    ${PROJECT_SOURCE_DIR}/src/sys/sysinfo.cpp
    ${PROJECT_SOURCE_DIR}/src/sys/sysinfo_android.cpp
    ${PROJECT_SOURCE_DIR}/src/sys/sysinfo_linux.cpp
    ${PROJECT_SOURCE_DIR}/src/sys/sysinfo_mac.cpp
    ${PROJECT_SOURCE_DIR}/src/sys/sysinfo_win.cpp
    ${PROJECT_SOURCE_DIR}/src/thread/basicpoll.cpp
    ${PROJECT_SOURCE_DIR}/src/thread/mt.cpp
    ${PROJECT_SOURCE_DIR}/src/thread/mt_linux.cpp
    ${PROJECT_SOURCE_DIR}/src/thread/mt_spinlock.cpp
    ${PROJECT_SOURCE_DIR}/src/thread/thread.cpp
    ${PROJECT_SOURCE_DIR}/src/thread/thread_linux.cpp
    ${PROJECT_SOURCE_DIR}/src/thread/thread_win.cpp
    ${PROJECT_SOURCE_DIR}/src/time/ontimer.cpp
    ${PROJECT_SOURCE_DIR}/src/time/timelib/astro.c
    ${PROJECT_SOURCE_DIR}/src/time/timelib/dow.c
    ${PROJECT_SOURCE_DIR}/src/time/timelib/parse_date.c
    ${PROJECT_SOURCE_DIR}/src/time/timelib/parse_tz.c
    ${PROJECT_SOURCE_DIR}/src/time/timelib/timelib.c
    ${PROJECT_SOURCE_DIR}/src/time/timelib/tm2unixtime.c
    ${PROJECT_SOURCE_DIR}/src/time/timelib/unixtime2tm.c
    ${PROJECT_SOURCE_DIR}/src/time/timeutil.cpp
    ${PROJECT_SOURCE_DIR}/src/time/tltime.cpp
    ${PROJECT_SOURCE_DIR}/src/types/basicobj.cpp
    ${PROJECT_SOURCE_DIR}/src/util/containerext.cpp
    ${PROJECT_SOURCE_DIR}/src/util/regex/regcomp.c
    ${PROJECT_SOURCE_DIR}/src/util/regex/regerror.c
    ${PROJECT_SOURCE_DIR}/src/util/regex/regexec.c
    ${PROJECT_SOURCE_DIR}/src/util/regex/regfree.c
    ${PROJECT_SOURCE_DIR}/src/util/strutil/charset.cpp
    ${PROJECT_SOURCE_DIR}/src/util/strutil/charset/charset_iconv.cpp
    ${PROJECT_SOURCE_DIR}/src/util/strutil/charset/charset_win.cpp
    ${PROJECT_SOURCE_DIR}/src/util/strutil/cppstring.cpp
    ${PROJECT_SOURCE_DIR}/src/util/strutil/maa.cpp
    ${PROJECT_SOURCE_DIR}/src/util/strutil/strmisc.cpp
    ${PROJECT_SOURCE_DIR}/src/util/strutil/strutil.cpp
	${PROJECT_SOURCE_DIR}/src/coroutine/coctx_swap_win.asm
		)

	if (BUILD_STATIC)
		add_library(basiclib STATIC ${basiclib_SOURCES})
	endif()
	if (BUILD_SHARED)
		add_library(basicdll SHARED ${basiclib_SOURCES})
		set_target_properties(basicdll PROPERTIES LINK_FLAGS "/SAFESEH:NO")
		target_link_libraries(basicdll ${PROJECT_SOURCE_DIR}/libs/FLzlib13.lib) 
		target_link_libraries(basicdll ${PROJECT_SOURCE_DIR}/3rd/libevent/libevent.lib)
	endif()
endfunction()

function(buildbasictest)

	set(basicTest_SOURCES
	${PROJECT_SOURCE_DIR}/src/scbasic/commu/basicclient.cpp
    ${PROJECT_SOURCE_DIR}/src/scbasic/commu/ipverify.cpp
    ${PROJECT_SOURCE_DIR}/src/scbasic/commu/servertemplate.cpp
    ${PROJECT_SOURCE_DIR}/src/scbasic/encode/rsaencode.cpp
    ${PROJECT_SOURCE_DIR}/basic_test/basic_Test.cpp
    ${PROJECT_SOURCE_DIR}/basic_test/coroutine/coroutinetest.cpp
    ${PROJECT_SOURCE_DIR}/basic_test/dll/basictesthead.cpp
    ${PROJECT_SOURCE_DIR}/basic_test/net/nettest.cpp
		)

	if (BUILD_STATIC)
		add_executable(basicTest ${basicTest_SOURCES}) 
		target_link_libraries(basicTest basiclib)
		target_link_libraries(basicTest ${PROJECT_SOURCE_DIR}/libs/FLzlib13.lib) 
		target_link_libraries(basicTest ${PROJECT_SOURCE_DIR}/3rd/libevent/libevent.lib)
		target_link_libraries(basicTest cryptopp-static)
		set_target_properties(basicTest PROPERTIES LINK_FLAGS "/SAFESEH:NO")
	endif()
endfunction()

include_directories(${PROJECT_SOURCE_DIR}/src/inc)
include_directories(${PROJECT_SOURCE_DIR}/src/scbasic)
include_directories(${PROJECT_SOURCE_DIR}/3rd/cryptopp)
set(VERSION_MAJOR 1)
set(VERSION_MINOR 0)
set(VERSION_BUGOR 0)
configure_file(
    "${PROJECT_SOURCE_DIR}/src/inc/basicversion.h.in"
    "${PROJECT_SOURCE_DIR}/src/inc/basicversion.h"
)
option(BUILD_ANDROID "Build android library" OFF)
option(BUILD_IOS "Build android library" OFF)
option(BUILD_STATIC "Build static library" ON)
option(BUILD_SHARED "Build shared library" ON)
if(MSVC)
	enable_language(ASM_MASM)
else()
	enable_language(ASM)
endif()

buildlibevent()
buildcryptopp()
buildbasiclib()
buildbasictest()
buildsproto()


