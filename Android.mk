LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

$(call import-add-path,$(LOCAL_PATH))

LOCAL_CFLAGS := -D__ANDROID -DANDROID
LOCAL_CFLAGS += -D_USE_SYS_MALLOC
LOCAL_CFLAGS += -DSQLITE_HAS_CODEC=1 -DSQLITE_SECURE_DELETE -DSQLITE_SOUNDEX -DSQLITE_ENABLE_COLUMN_METADATA -DCODEC_TYPE=CODEC_TYPE_AES256
LOCAL_CFLAGS += -Wno-deprecated

LOCAL_CPPFLAGS += -frtti
LOCAL_CPPFLAGS += -fexceptions

LOCAL_C_INCLUDES := $(LOCAL_PATH)/lib/android/libevent-patches-2.0
LOCAL_C_INCLUDES += $(LOCAL_PATH)/lib/android/libevent-patches-2.0/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/lib/android/libevent-patches-2.0/android
LOCAL_C_INCLUDES += $(LOCAL_PATH)/lib/android/libiconv_android/jni/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/src/inc

LOCAL_MODULE := basic_static
LOCAL_MODULE_FILENAME := basic

LOCAL_SRC_FILES :=	src/datastruct/basic_string.cpp  			\
					src/debug/debug.cpp 						\
					src/debug/debug_android.cpp				\
					src/exception/call_stack_gcc.cpp 			\
					src/file/file_linux.cpp 					\
					src/file/filefind.cpp 					\
					src/file/fileman.cpp 						\
					src/log/log.cpp							\
					src/mem/mem.cpp 							\
					src/mem/smartbuffer.cpp 					\
					src/mem/tlstaticbuffer.cpp 				\
					src/sys/sysinfo_android.cpp 				\
					src/sys/sysinfo.cpp 						\
					src/thread/mt_linux.cpp 					\
					src/thread/mt.cpp 						\
					src/thread/thread_linux.cpp 				\
					src/time/tltime.cpp 						\
					src/types/basicobj.cpp 					\
					src/util/strutil/charset/charset_iconv.cpp \
					src/util/strutil/charset.cpp 				\
					src/util/strutil/cppstring.cpp 			\
					src/util/strutil/maa.cpp 					\
					src/util/strutil/strmisc.cpp 				\
					src/util/strutil/strutil.cpp				
					

LOCAL_STATIC_LIBRARIES := event_static
LOCAL_STATIC_LIBRARIES += libiconv

include $(BUILD_STATIC_LIBRARY)

$(call import-module,lib/android/libevent-patches-2.0)
$(call import-module,lib/android/libiconv_android/jni)

