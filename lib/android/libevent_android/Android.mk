####################################  
# Build libevent as separate library  
  
LOCAL_PATH := $(call my-dir)  
  
include $(CLEAR_VARS)  
  
LOCAL_MODULE := event_static  
  
LOCAL_MODULE_TAGS := optional   
  
LOCAL_SRC_FILES :=	../../../3rd/libevent/buffer.c				\
					../../../3rd/libevent/bufferevent.c			\
					../../../3rd/libevent/bufferevent_filter.c	\
					../../../3rd/libevent/bufferevent_pair.c		\
					../../../3rd/libevent/bufferevent_ratelim.c	\
					../../../3rd/libevent/bufferevent_sock.c		\
					../../../3rd/libevent/epoll.c					\
					../../../3rd/libevent/epoll_sub.c 			\
					../../../3rd/libevent/evdns.c					\
					../../../3rd/libevent/event.c 				\
					../../../3rd/libevent/event_tagging.c 		\
					../../../3rd/libevent/evmap.c 				\
					../../../3rd/libevent/evrpc.c 				\
					../../../3rd/libevent/evthread.c 				\
					../../../3rd/libevent/evthread_pthread.c 		\
					../../../3rd/libevent/evutil.c 				\
					../../../3rd/libevent/evutil_rand.c 			\
					../../../3rd/libevent/http.c 					\
					../../../3rd/libevent/listener.c 				\
					../../../3rd/libevent/log.c 					\
					../../../3rd/libevent/poll.c 					\
					../../../3rd/libevent/select.c 				\
					../../../3rd/libevent/signal.c 				\
					../../../3rd/libevent/strlcpy.c				\
					../../../3rd/libevent/evutil_time.c				
   
LOCAL_C_INCLUDES :=	$(LOCAL_PATH)/../../../3rd/libevent 			\
					$(LOCAL_PATH)/android 	\
					$(LOCAL_PATH)/../../../3rd/libevent/include  
  
LOCAL_CFLAGS := -DHAVE_CONFIG_H -DANDROID -fvisibility=hidden  
  
include $(BUILD_STATIC_LIBRARY) 