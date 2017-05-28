cmake_minimum_required(VERSION 2.8.5)

project(basiclib)

set(VERSION_MAJOR 1)
set(VERSION_MINOR 0)
set(VERSION_BUGOR 0)

set(BasicLibSrcPath ${PROJECT_SOURCE_DIR})
set(BasicLibJemallocSrcPath ${BasicLibSrcPath}/../3rd/jemalloc)
configure_file(
    "${BasicLibSrcPath}/inc/basicversion.h.in"
    "${BasicLibSrcPath}/inc/basicversion.h"
)

option(BASICLIB_BUILD_ANDROID "Build android library" OFF)
option(BASICLIB_BUILD_IOS "Build android library" OFF)
option(BASICLIB_BUILD_STATIC "Build static library" ON)
option(BASICLIB_BUILD_SHARED "Build shared library" ON)
option(BASICLIB_BUILD_JEMALLOC "Build Use JEMALLOC" OFF)

if(BASICLIB_BUILD_JEMALLOC)
    if(MSVC)
	message(WARNING, "windows not support jemalloc")
    else()
	execute_process(COMMAND make -f makefile_jemalloc -j4
			WORKING_DIRECTORY ${BasicLibSrcPath}/script/)
    endif()
endif()

#extra src
set(BasicLibExtra_Src "")
#flags
set(BasicLibCMakeCXX_FLAGS ${CMAKE_CXX_FLAGS})
set(BasicLibCMakeC_FLAGS ${CMAKE_C_FLAGS})
#define
set(BasicLibDefinitions "-DSQLITE_HAS_CODEC=1 -DSQLITE_SECURE_DELETE -DSQLITE_SOUNDEX -DSQLITE_ENABLE_COLUMN_METADATA")
#link
set(BasicLibLinks "")
if(MSVC)
	enable_language(ASM_MASM)
	set(BasicLibExtra_Src, "${BasicLibExtra_Src} ${BasicLibSrcPath}/coroutine/coctx_swap_win.asm")
	set(BasicLibCMakeCXX_FLAGS "${BasicLibCMakeCXX_FLAGS} /MP")
	set(BasicLibCMakeC_FLAGS "${BasicLibCMakeC_FLAGS} /MP")
	set(BasicLibDefinitions "${BasicLibDefinitions} -D_USE_SYS_MALLOC -DUNICODE -D_UNICODE -D__BASICWINDOWS")
	set(BasicLibLinks "${BasicLibLinks} ${BasicLibSrcPath}/../libs/FLzlib13.lib")
else()
	enable_language(ASM)
	set(BasicLibExtra_Src, "${BasicLibExtra_Src} ${BasicLibSrcPath}/coroutine/coctx_swap.S")
	set(BasicLibCMakeCXX_FLAGS "${BasicLibCMakeCXX_FLAGS} --std=c++11")
	if(BASICLIB_BUILD_ANDROID)
		set(BasicLibDefinitions "${BasicLibDefinitions} -D_USE_SYS_MALLOC -D__ANDROID -DANDROID")
	else()
		if(BASICLIB_BUILD_IOS)
			set(BasicLibDefinitions "${BasicLibDefinitions} -D_USE_SYS_MALLOC -D__MAC -DMAC")
		else()
			set(BasicLibDefinitions "${BasicLibDefinitions} -DLINUX -D__LINUX")
		endif()
	endif()
endif()

#define
add_definitions(${BasicLibDefinitions})
#flags
set(CMAKE_CXX_FLAGS ${BasicLibCMakeCXX_FLAGS})
set(CMAKE_C_FLAGS ${BasicLibCMakeC_FLAGS})
#output
set(LIBRARY_OUTPUT_PATH ${BasicLibSrcPath}/../libs)

#src
set(basiclib_SOURCES ${BasicLibSrcPath}/inc/basic_func.cpp
   ${BasicLibSrcPath}/algorithm/md5.cpp
   ${BasicLibSrcPath}/algorithm/sha1/sha1.cpp
   ${BasicLibSrcPath}/algorithm/xtra.cpp
   ${BasicLibSrcPath}/algorithm/zip/tlgzip.cpp
   ${BasicLibSrcPath}/algorithm/zip/tlzipfile.cpp
   ${BasicLibSrcPath}/coroutine/coroutineplus.cpp
   ${BasicLibSrcPath}/datastruct/basic_string.cpp
   ${BasicLibSrcPath}/datastruct/key_value.cpp
   ${BasicLibSrcPath}/datastruct/pbzk.cpp
   ${BasicLibSrcPath}/datastruct/stringex.cpp
   ${BasicLibSrcPath}/debug/debug.cpp
   ${BasicLibSrcPath}/debug/debug_android.cpp
   ${BasicLibSrcPath}/debug/debug_linux.cpp
   ${BasicLibSrcPath}/debug/debug_mac.cpp
   ${BasicLibSrcPath}/debug/debug_win.cpp
   ${BasicLibSrcPath}/dll/loaddll.cpp
   ${BasicLibSrcPath}/exception/call_stack_gcc.cpp
   ${BasicLibSrcPath}/exception/call_stack_msvc.cpp
   ${BasicLibSrcPath}/exception/exception.cpp
   ${BasicLibSrcPath}/exception/exception_linux.cpp
   ${BasicLibSrcPath}/exception/exception_win.cpp
   ${BasicLibSrcPath}/exception/StackWalker.cpp
   ${BasicLibSrcPath}/file/diskfile.cpp
   ${BasicLibSrcPath}/file/filefind.cpp
   ${BasicLibSrcPath}/file/fileman.cpp
   ${BasicLibSrcPath}/file/filenotify.cpp
   ${BasicLibSrcPath}/file/fileobj.cpp
   ${BasicLibSrcPath}/file/file_linux.cpp
   ${BasicLibSrcPath}/file/inifile.cpp
   ${BasicLibSrcPath}/file/memfile.cpp
   ${BasicLibSrcPath}/log/log.cpp
   ${BasicLibSrcPath}/mem/bitstream.cpp
   ${BasicLibSrcPath}/mem/mem.cpp
   ${BasicLibSrcPath}/mem/smartbuffer.cpp
   ${BasicLibSrcPath}/mem/tlstaticbuffer.cpp
   ${BasicLibSrcPath}/net/net.cpp
   ${BasicLibSrcPath}/net/sendbuffer.cpp
   ${BasicLibSrcPath}/sqlite/shell.c
   ${BasicLibSrcPath}/sqlite/sqlite3db.cpp
   ${BasicLibSrcPath}/sqlite/sqlite3dbquery.cpp
   ${BasicLibSrcPath}/sqlite/sqlite3dbtable.cpp
   ${BasicLibSrcPath}/sqlite/sqlite3secure.c
   ${BasicLibSrcPath}/sys/sysinfo.cpp
   ${BasicLibSrcPath}/sys/sysinfo_android.cpp
   ${BasicLibSrcPath}/sys/sysinfo_linux.cpp
   ${BasicLibSrcPath}/sys/sysinfo_mac.cpp
   ${BasicLibSrcPath}/sys/sysinfo_win.cpp
   ${BasicLibSrcPath}/thread/basicpoll.cpp
   ${BasicLibSrcPath}/thread/mt.cpp
   ${BasicLibSrcPath}/thread/mt_linux.cpp
   ${BasicLibSrcPath}/thread/mt_spinlock.cpp
   ${BasicLibSrcPath}/thread/thread.cpp
   ${BasicLibSrcPath}/thread/thread_linux.cpp
   ${BasicLibSrcPath}/thread/thread_win.cpp
   ${BasicLibSrcPath}/time/ontimer.cpp
   ${BasicLibSrcPath}/time/timelib/astro.c
   ${BasicLibSrcPath}/time/timelib/dow.c
   ${BasicLibSrcPath}/time/timelib/parse_date.c
   ${BasicLibSrcPath}/time/timelib/parse_tz.c
   ${BasicLibSrcPath}/time/timelib/timelib.c
   ${BasicLibSrcPath}/time/timelib/tm2unixtime.c
   ${BasicLibSrcPath}/time/timelib/unixtime2tm.c
   ${BasicLibSrcPath}/time/timeutil.cpp
   ${BasicLibSrcPath}/time/tltime.cpp
   ${BasicLibSrcPath}/types/basicobj.cpp
   ${BasicLibSrcPath}/util/containerext.cpp
   ${BasicLibSrcPath}/util/regex/regcomp.c
   ${BasicLibSrcPath}/util/regex/regerror.c
   ${BasicLibSrcPath}/util/regex/regexec.c
   ${BasicLibSrcPath}/util/regex/regfree.c
   ${BasicLibSrcPath}/util/strutil/charset.cpp
   ${BasicLibSrcPath}/util/strutil/charset/charset_iconv.cpp
   ${BasicLibSrcPath}/util/strutil/charset/charset_win.cpp
   ${BasicLibSrcPath}/util/strutil/cppstring.cpp
   ${BasicLibSrcPath}/util/strutil/maa.cpp
   ${BasicLibSrcPath}/util/strutil/strmisc.cpp
   ${BasicLibSrcPath}/util/strutil/strutil.cpp
	)

if (BASICLIB_BUILD_STATIC)
	add_library(basiclib STATIC ${basiclib_SOURCES} ${BasicLibExtra_Src})
endif()

if(BASICLIB_BUILD_SHARED)
	add_library(basicdll SHARED ${basiclib_SOURCES} ${BasicLibExtra_Src})
	set_target_properties(basicdll PROPERTIES LINK_FLAGS "/SAFESEH:NO")
	target_link_libraries(basicdll ${BasicLibLinks}) 
endif()










