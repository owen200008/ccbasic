LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

$(call import-add-path,$(LOCAL_PATH))

LOCAL_CFLAGS := -D__ANDROID -DANDROID
LOCAL_CFLAGS += -D_USE_SYS_MALLOC
LOCAL_CFLAGS += -DSQLITE_HAS_CODEC=1 -DSQLITE_SECURE_DELETE -DSQLITE_SOUNDEX -DSQLITE_ENABLE_COLUMN_METADATA
LOCAL_CFLAGS += -Wno-deprecated

LOCAL_CPPFLAGS += -frtti
LOCAL_CPPFLAGS += -fexceptions

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../3rd/libevent/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../3rd/libevent/build/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../libs/iconv/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/3rd/libevent/include

LOCAL_MODULE := basic_static
LOCAL_MODULE_FILENAME := basic

LOCAL_SRC_FILES :=	../../algorithm/base64.cpp 					\
					../../algorithm/crc16.cpp 					\
					../../algorithm/algorithm.cpp 				\
					../../algorithm/crc32.cpp 					\
					../../algorithm/md5.cpp 					\
					../../algorithm/tlmath.cpp 					\
					../../algorithm/aes/aes.cpp 				\
					../../algorithm/rc5/rc5.cpp 				\
					../../algorithm/sha1/sha1.cpp 				\
					../../algorithm/zip/tlgzip.cpp 				\
					../../algorithm/zip/tlzipfile.cpp 			\
					../../algorithm/xtra.cpp					\
					../../datastruct/basic_string.cpp  			\
					../../datastruct/key_value.cpp  			\
					../../datastruct/stringex.cpp  				\
					../../datastruct/tlcoll.cpp  				\
					../../datastruct/tllong.cpp  				\
					../../datastruct/tlvalue.cpp  				\
					../../datastruct/pbzk.cpp					\
					../../debug/debug.cpp 						\
					../../debug/debug_android.cpp				\
					../../exception/exception_linux.cpp 		\
					../../exception/exception.cpp 				\
					../../file/ini/inifile.cpp 					\
					../../file/ini/mergeini.cpp 				\
					../../file/diskfile.cpp 					\
					../../file/file_linux.cpp 					\
					../../file/filefind.cpp 					\
					../../file/fileman.cpp 						\
					../../file/filenotify.cpp 					\
					../../file/fileobj.cpp 						\
					../../file/memfile.cpp 						\
					../../graphics/colorutil.cpp 				\
					../../i18n/i18n.cpp 						\
					../../log/log.cpp							\
					../../mem/mem.cpp 							\
					../../mem/singleton.cpp 					\
					../../mem/smallobj.cpp 						\
					../../mem/smartbuffer.cpp 					\
					../../mem/tlstaticbuffer.cpp 				\
					../../mem/bitstream.cpp 					\
					../../net/socket/socket_event.cpp 			\
					../../net/socket/socket_link.cpp 			\
					../../net/socket/socket_net.cpp 			\
					../../net/net.cpp 							\
					../../net/proxy.cpp 						\
					../../net/proxyinfo.cpp 					\
					../../net/sendbuffer.cpp 					\
					../../sys/hardwareinfo_linux.cpp 			\
					../../sys/sysinfo_android.cpp 				\
					../../sys/sysinfo.cpp 						\
					../../thread/mt_linux.cpp 					\
					../../thread/mt.cpp 						\
					../../thread/thread_linux.cpp 				\
					../../time/timelib/astro.c 					\
					../../time/timelib/dow.c 					\
					../../time/timelib/parse_date.c 			\
					../../time/timelib/parse_tz.c 				\
					../../time/timelib/timelib.c 				\
					../../time/timelib/tm2unixtime.c 			\
					../../time/timelib/unixtime2tm.c 			\
					../../time/basicTimeDWord.cpp 				\
					../../time/scheduletime.cpp 				\
					../../time/timeutil.cpp 					\
					../../time/tltime.cpp 						\
					../../types/archive.cpp 					\
					../../types/basicobj.cpp 					\
					../../types/runtime.cpp 					\
					../../util/strutil/charset/charset_iconv.cpp \
					../../util/strutil/charset.cpp 				\
					../../util/strutil/cppstring.cpp 			\
					../../util/strutil/maa.cpp 					\
					../../util/strutil/strmisc.cpp 				\
					../../util/strutil/strutil.cpp				\
					../../sqlite/sqlite3secure.c				\
					../../sqlite/sqlite3db.cpp                  \
					../../sqlite/sqlite3dbquery.cpp             \
					../../sqlite/sqlite3dbtable.cpp             \
					../../net/networkprotocal.cpp

LOCAL_STATIC_LIBRARIES := event_static
LOCAL_STATIC_LIBRARIES += libiconv

include $(BUILD_STATIC_LIBRARY)

$(call import-module,libevent-patches-2.0)
$(call import-module,libiconv_android/jni)

