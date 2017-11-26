cmake_minimum_required(VERSION 2.8.12)

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

macro(source_group_by_dir source_files)
    if(MSVC)
        set(sgbd_cur_dir ${PROJECT_SOURCE_DIR})
        foreach(sgbd_file ${${source_files}})
			set(samefilename FALSE)
			while (NOT samefilename)
				string(REGEX REPLACE "/[^/\\.]*/\\.\\./" "/" sgbd_fpath ${sgbd_file})
				string(COMPARE EQUAL ${sgbd_fpath} ${sgbd_file} samefilename)
				set(sgbd_file ${sgbd_fpath})
			endwhile()
            string(REGEX REPLACE "${sgbd_cur_dir}/" "" sgbd_fpath ${sgbd_file})
			string(REGEX REPLACE "(.*)/" "" sgbd_group_name ${sgbd_fpath})
			string(COMPARE EQUAL ${sgbd_fpath} ${sgbd_group_name} sgbd_nogroup)
			if(sgbd_nogroup)
				set(sgbd_real_group_name "\\")
			else()
				string(REGEX REPLACE "/${sgbd_group_name}" "" sgbd_real_group_name ${sgbd_fpath})
				string(REPLACE "/" "\\" sgbd_real_group_name ${sgbd_real_group_name})
			endif()
			source_group(${sgbd_real_group_name} FILES ${sgbd_file})
        endforeach(sgbd_file)
    endif(MSVC)
endmacro(source_group_by_dir)

#first option
option(BASICLIB_BUILD_SHARED "Build shared library" OFF)
option(BASICLIB_BUILD_LUA "Build lua library" ON)
option(BASICLIB_BUILD_JEMALLOC "Build Use JEMALLOC" OFF)

if(BASICLIB_BUILD_SHARED)
	set(BASICLIB_BUILD_TYPE SHARED)
else()
	set(BASICLIB_BUILD_TYPE STATIC)
endif()

#second include
include_directories(${BasicLibJemallocSrcPath}/include/jemalloc)
include_directories(${BasicLibSrcPath}/../3rd/libevent/include)

#third subdirectory
#build event
set(EVENT__DISABLE_SAMPLES ON CACHE BOOL "flags")
set(EVENT__DISABLE_REGRESS ON CACHE BOOL "flags")
set(EVENT__DISABLE_TESTS ON CACHE BOOL "flags")
set(EVENT__DISABLE_THREAD_SUPPORT ON CACHE BOOL "flags")
set(EVENT__DISABLE_OPENSSL ON CACHE BOOL "flags")
set(EVENT__DISABLE_BENCHMARK ON CACHE BOOL "flags")
set(EVENT__DISABLE_CLOCK_GETTIME ON CACHE BOOL "flags")
set(EVENT__DISABLE_DEBUG_MODE ON CACHE BOOL "flags")
set(EVENT__BUILD_SHARED_LIBRARIES ${BASICLIB_BUILD_SHARED})
add_subdirectory(${BasicLibSrcPath}/../3rd/libevent libevent)
include_directories(${PROJECT_BINARY_DIR}/libevent/include)

#build cryptopp
set(BUILD_TESTING OFF CACHE BOOL "flags")
set(BUILD_SHARED OFF CACHE BOOL "flags")
add_subdirectory(${PROJECT_SOURCE_DIR}/script/cryptoppcmake cryptopp)

if(MSVC)
    #build zlib
    add_subdirectory(${PROJECT_SOURCE_DIR}/script/zlibcmake zlib)
    include_directories(${PROJECT_BINARY_DIR}/zlib)
endif()

#build lua
if(BASICLIB_BUILD_LUA)
	#build lua
	add_subdirectory(${PROJECT_SOURCE_DIR}/script/luacmake lualib)
	if(BASICLIB_BUILD_SHARED)
		#build sproto
		add_subdirectory(${PROJECT_SOURCE_DIR}/scbasic/sproto sproto)
	endif()
endif()

#fourth self build
#extra src
set(BasicLibExtra_Src)
#flags
set(BasicLibCMakeCXX_FLAGS ${CMAKE_CXX_FLAGS})
set(BasicLibCMakeC_FLAGS ${CMAKE_C_FLAGS})
#define
set(BasicLibDefinitions -DSQLITE_HAS_CODEC=1 -DSQLITE_SECURE_DELETE -DSQLITE_SOUNDEX -DSQLITE_ENABLE_COLUMN_METADATA)
#link add event
set(BasicLibLinks event_static)

if(MSVC)
	enable_language(ASM_MASM)
	set(BasicLibExtra_Src ${BasicLibExtra_Src} 
		${BasicLibSrcPath}/coroutine/coctx_swap_win.asm
   		${BasicLibSrcPath}/exception/call_stack_msvc.cpp
   		${BasicLibSrcPath}/exception/StackWalker.cpp)
	set(BasicLibCMakeCXX_FLAGS "${BasicLibCMakeCXX_FLAGS} /MP")
	set(BasicLibCMakeC_FLAGS "${BasicLibCMakeC_FLAGS} /MP")
	set(BasicLibDefinitions ${BasicLibDefinitions} -D_USE_SYS_MALLOC -DUNICODE -D_UNICODE)
	set(BasicLibLinks ${BasicLibLinks} zlib ws2_32.lib)
else()
	set(BasicLibCMakeCXX_FLAGS "${BasicLibCMakeCXX_FLAGS} --std=c++11 -frtti -fexceptions")
    enable_language(ASM)
    set(BasicLibExtra_Src ${BasicLibExtra_Src} ${BasicLibSrcPath}/coroutine/coctx_swap.S)
	if(ANDROID)
		include_directories(${BasicLibSrcPath}/../libs/iconv/include)
		set(BasicLibDefinitions ${BasicLibDefinitions} -D_USE_SYS_MALLOC -D_FILE_OFFSET_BITS=32)
		set(BasicLibLinks ${BasicLibLinks} dl ${BasicLibSrcPath}/../libs/iconv/obj/local/${ANDROID_ABI}/libiconv.a)
	elseif(APPLE)
		set(BasicLibDefinitions ${BasicLibDefinitions} -D_USE_SYS_MALLOC)
		set(BasicLibLinks ${BasicLibLinks} dl iconv)
	else()
		execute_process(COMMAND make -f makefile_jemalloc -j4
			WORKING_DIRECTORY ${BasicLibSrcPath}/script/)
		set(BasicLibDefinitions ${BasicLibDefinitions})
		set(BasicLibLinks ${BasicLibLinks} ${BasicLibSrcPath}/../3rd/jemalloc/lib/libjemalloc_pic.a dl)
	endif()
endif()

#define
add_definitions(${BasicLibDefinitions})
#flags
set(CMAKE_CXX_FLAGS ${BasicLibCMakeCXX_FLAGS})
set(CMAKE_C_FLAGS ${BasicLibCMakeC_FLAGS})

#src
set(basiclib_SOURCES 
	${BasicLibSrcPath}/algorithm/rc5/rc5.h
	${BasicLibSrcPath}/algorithm/rc5/rc5_locl.h
	${BasicLibSrcPath}/algorithm/rc5/rc5.cpp
	${BasicLibSrcPath}/algorithm/md5.h
    ${BasicLibSrcPath}/algorithm/sha1/sha1.h
    ${BasicLibSrcPath}/algorithm/xtra.h
    ${BasicLibSrcPath}/algorithm/zip/tlgzip.h
    ${BasicLibSrcPath}/algorithm/zip/tlzipfile.h
	${BasicLibSrcPath}/algorithm/base64.h
    ${BasicLibSrcPath}/algorithm/base64.cpp
	${BasicLibSrcPath}/algorithm/crc32.h
    ${BasicLibSrcPath}/algorithm/crc32.cpp
    ${BasicLibSrcPath}/coroutine/coroutineplus.h
    ${BasicLibSrcPath}/datastruct/array.h
    ${BasicLibSrcPath}/datastruct/basic_string.h
    ${BasicLibSrcPath}/datastruct/extern.h
    ${BasicLibSrcPath}/datastruct/key_value.h
    ${BasicLibSrcPath}/datastruct/pbzk.h
    ${BasicLibSrcPath}/datastruct/stringex.h
    ${BasicLibSrcPath}/debug/debug.h
    ${BasicLibSrcPath}/dll/loaddll.h
    ${BasicLibSrcPath}/exception/call_stack.hpp
    ${BasicLibSrcPath}/exception/exception.h
    ${BasicLibSrcPath}/exception/exception_linux.h
    ${BasicLibSrcPath}/exception/exception_win.h
    ${BasicLibSrcPath}/exception/StackWalker.h
    ${BasicLibSrcPath}/exception/stack_exception.hpp
    ${BasicLibSrcPath}/file/filebase.h
    ${BasicLibSrcPath}/file/filedefine.h
    ${BasicLibSrcPath}/file/fileman.h
    ${BasicLibSrcPath}/file/filenotify.h
    ${BasicLibSrcPath}/file/fileobj.h
    ${BasicLibSrcPath}/file/file_linux.h
    ${BasicLibSrcPath}/file/inifile.h
    ${BasicLibSrcPath}/inc/basic.h
    ${BasicLibSrcPath}/inc/basic_def.h
    ${BasicLibSrcPath}/inc/error.h
    ${BasicLibSrcPath}/log/log.h
    ${BasicLibSrcPath}/mem/basicallocator.h
    ${BasicLibSrcPath}/mem/bitstream.h
    ${BasicLibSrcPath}/mem/mem.h
    ${BasicLibSrcPath}/mem/smartbuffer.h
    ${BasicLibSrcPath}/mem/tlstaticbuffer.h
    ${BasicLibSrcPath}/misc/fastdelegate.h
    ${BasicLibSrcPath}/misc/fastdelegatebind.h
    ${BasicLibSrcPath}/misc/singleton.h
    ${BasicLibSrcPath}/misc/tlrefptr.h
    ${BasicLibSrcPath}/sqlite/sqlite3.h
    ${BasicLibSrcPath}/sqlite/sqlite3db.h
    ${BasicLibSrcPath}/sqlite/sqlite3dbquery.h
    ${BasicLibSrcPath}/sqlite/sqlite3dbtable.h
    ${BasicLibSrcPath}/sqlite/sqlite3ext.h
    ${BasicLibSrcPath}/sys/syserror.h
    ${BasicLibSrcPath}/sys/sysinfo.h
    ${BasicLibSrcPath}/thread/basicpoll.h
    ${BasicLibSrcPath}/thread/mt.h
    ${BasicLibSrcPath}/thread/mt_linux.h
    ${BasicLibSrcPath}/thread/mt_spinlock.h
    ${BasicLibSrcPath}/thread/thread.h
    ${BasicLibSrcPath}/time/ontimer.h
    ${BasicLibSrcPath}/time/timelib/astro.h
    ${BasicLibSrcPath}/time/timelib/config.w32.h
    ${BasicLibSrcPath}/time/timelib/fallbackmap.h
    ${BasicLibSrcPath}/time/timelib/timelib.h
    ${BasicLibSrcPath}/time/timelib/timelib_config.h
    ${BasicLibSrcPath}/time/timelib/timelib_structs.h
    ${BasicLibSrcPath}/time/timelib/timezonedb.h
    ${BasicLibSrcPath}/time/timelib/timezonemap.h
    ${BasicLibSrcPath}/time/timeutil.h
    ${BasicLibSrcPath}/time/tltime.h
    ${BasicLibSrcPath}/types/basicobj.h
    ${BasicLibSrcPath}/types/linux.h
    ${BasicLibSrcPath}/types/types.h
    ${BasicLibSrcPath}/util/auto_cast.h
	${BasicLibSrcPath}/util/skiplist.h
    ${BasicLibSrcPath}/util/concurrentqueue/blockingconcurrentqueue.h
    ${BasicLibSrcPath}/util/concurrentqueue/concurrentqueue.h
    ${BasicLibSrcPath}/util/container.h
    ${BasicLibSrcPath}/util/containerext.h
    ${BasicLibSrcPath}/util/functor.h
    ${BasicLibSrcPath}/util/private.h
    ${BasicLibSrcPath}/util/regex/cclass.h
    ${BasicLibSrcPath}/util/regex/cgnuregexp.h
    ${BasicLibSrcPath}/util/regex/cname.h
    ${BasicLibSrcPath}/util/regex/regex.h
    ${BasicLibSrcPath}/util/regex/regex2.h
    ${BasicLibSrcPath}/util/regex/regex_extra.h
    ${BasicLibSrcPath}/util/regex/utils.h
    ${BasicLibSrcPath}/util/strutil/charset.h
    ${BasicLibSrcPath}/util/strutil/charset/charset_def.h
    ${BasicLibSrcPath}/util/strutil/charset/charset_win.h
    ${BasicLibSrcPath}/util/strutil/cppstring.h
    ${BasicLibSrcPath}/util/strutil/maa.h
    ${BasicLibSrcPath}/util/strutil/strutil.h
   ${BasicLibSrcPath}/inc/basic_func.cpp
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
   ${BasicLibSrcPath}/exception/exception.cpp
   ${BasicLibSrcPath}/exception/exception_linux.cpp
   ${BasicLibSrcPath}/exception/exception_win.cpp
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
   ${BasicLibSrcPath}/net/net.cpp
   ${BasicLibSrcPath}/net/net.h
   ${BasicLibSrcPath}/net/net_client.cpp
   ${BasicLibSrcPath}/net/net_client.h
   ${BasicLibSrcPath}/net/net_server.cpp
   ${BasicLibSrcPath}/net/net_server.h
   ${BasicLibSrcPath}/net/sendbuffer.cpp
   ${BasicLibSrcPath}/net/sendbuffer.h
   ${BasicLibSrcPath}/net/net_socket.cpp
   ${BasicLibSrcPath}/net/net_socket.h
   ${BasicLibSrcPath}/net/net_mgr.cpp
   ${BasicLibSrcPath}/net/net_mgr.h
	)
set(TotalSourceData ${basiclib_SOURCES} ${BasicLibExtra_Src})
source_group_by_dir(TotalSourceData)

add_library(basiclib ${BASICLIB_BUILD_TYPE} ${basiclib_SOURCES} ${BasicLibExtra_Src})
target_link_libraries(basiclib ${BasicLibLinks})
if(MSVC)
    target_include_directories(basiclib PUBLIC
        $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/zlib>
        $<BUILD_INTERFACE:${BasicLibSrcPath}/inc>
        $<BUILD_INTERFACE:${BasicLibSrcPath}/../3rd/lua/src>
        $<BUILD_INTERFACE:${BasicLibSrcPath}/../3rd/kaguya/include>)
	set_target_properties(basiclib PROPERTIES LINK_FLAGS "/SAFESEH:NO")
else()
    target_include_directories(basiclib PUBLIC
        $<BUILD_INTERFACE:${BasicLibSrcPath}/inc>
        $<BUILD_INTERFACE:${BasicLibSrcPath}/../3rd/lua/src>
        $<BUILD_INTERFACE:${BasicLibSrcPath}/../3rd/kaguya/include>)
endif()










