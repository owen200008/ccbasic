####################################  
# Build openal as separate library
LOCAL_PATH := $(call my-dir)  
  
include $(CLEAR_VARS)  
  
LOCAL_MODULE := openal_static
LOCAL_ARM_MODE   := arm
  
LOCAL_SRC_FILES := ./ALc.c\
              ../../../3rd/openal-soft/Alc/ALu.c\
              ../../../3rd/openal-soft/Alc/alcConfig.c\
              ../../../3rd/openal-soft/Alc/alcRing.c\
              ../../../3rd/openal-soft/Alc/bs2b.c\
              ../../../3rd/openal-soft/Alc/effects/chorus.c\
              ../../../3rd/openal-soft/Alc/effects/compressor.c\
              ../../../3rd/openal-soft/Alc/effects/dedicated.c\
              ../../../3rd/openal-soft/Alc/effects/distortion.c\
              ../../../3rd/openal-soft/Alc/effects/echo.c\
              ../../../3rd/openal-soft/Alc/effects/equalizer.c\
              ../../../3rd/openal-soft/Alc/effects/flanger.c\
              ../../../3rd/openal-soft/Alc/effects/modulator.c\
              ../../../3rd/openal-soft/Alc/effects/null.c\
              ../../../3rd/openal-soft/Alc/effects/reverb.c\
              ../../../3rd/openal-soft/Alc/helpers.c\
              ../../../3rd/openal-soft/Alc/bsinc.c\
              ../../../3rd/openal-soft/Alc/hrtf.c\
              ../../../3rd/openal-soft/Alc/uhjfilter.c\
              ../../../3rd/openal-soft/Alc/ambdec.c\
              ../../../3rd/openal-soft/Alc/bformatdec.c\
              ../../../3rd/openal-soft/Alc/panning.c\
              ../../../3rd/openal-soft/Alc/mixer.c\
              ../../../3rd/openal-soft/Alc/mixer_c.c\
			  ../../../3rd/openal-soft/Alc/backends/base.c\
              ../../../3rd/openal-soft/Alc/backends/loopback.c\
              ../../../3rd/openal-soft/Alc/backends/null.c\
			  ../../../3rd/openal-soft/Alc/backends/opensl.c\
			  ../../../3rd/openal-soft/Alc/backends/wave.c\
			  ../../../3rd/openal-soft/OpenAL32/alAuxEffectSlot.c\
              ../../../3rd/openal-soft/OpenAL32/alBuffer.c\
              ../../../3rd/openal-soft/OpenAL32/alEffect.c\
              ../../../3rd/openal-soft/OpenAL32/alError.c\
              ../../../3rd/openal-soft/OpenAL32/alExtension.c\
              ../../../3rd/openal-soft/OpenAL32/alFilter.c\
              ../../../3rd/openal-soft/OpenAL32/alListener.c\
              ../../../3rd/openal-soft/OpenAL32/alSource.c\
              ../../../3rd/openal-soft/OpenAL32/alState.c\
              ../../../3rd/openal-soft/OpenAL32/alThunk.c\
              ../../../3rd/openal-soft/OpenAL32/sample_cvt.c\
			  ../../../3rd/openal-soft/common/almalloc.c\
              ../../../3rd/openal-soft/common/atomic.c\
              ../../../3rd/openal-soft/common/rwlock.c\
              ../../../3rd/openal-soft/common/threads.c\
              ../../../3rd/openal-soft/common/uintmap.c
    
   
LOCAL_C_INCLUDES :=	$(LOCAL_PATH)/../../../3rd/openal-soft/include
LOCAL_C_INCLUDES +=	$(LOCAL_PATH)/../../../3rd/openal-soft/Alc
LOCAL_C_INCLUDES += $(LOCAL_PATH)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../3rd/openal-soft/OpenAL32
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../3rd/openal-soft/OpenAL32/include
					 
ifeq ($(APP_ABI),armeabi) 
LOCAL_CFLAGS     := -DAL_BUILD_LIBRARY \
                    -DAL_ALEXT_PROTOTYPES \
                    -DANDROID \
                    -ffunction-sections \
                    -funwind-tables \
                    -fstack-protector \
                    -fno-short-enums \
                    -DHAVE_GCC_VISIBILITY \
                    -O3 \
                    -ffast-math \
                    -std=c99
else
LOCAL_CFLAGS     := -DAL_BUILD_LIBRARY \
                    -DAL_ALEXT_PROTOTYPES \
                    -DANDROID \
                    -ffunction-sections \
                    -funwind-tables \
                    -fstack-protector \
                    -fno-short-enums \
                    -DHAVE_GCC_VISIBILITY \
                    -O3 \
                    -mfpu=neon \
                    -mfloat-abi=softfp \
                    -march=armv7-a \
                    -ffast-math \
                    -std=c99

LOCAL_ARM_NEON := true
endif

include $(BUILD_STATIC_LIBRARY) 