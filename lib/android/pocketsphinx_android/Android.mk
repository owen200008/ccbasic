####################################  
# Build pocketsphinx as separate library  
  
LOCAL_PATH := $(call my-dir)  
  
include $(CLEAR_VARS)  

$(call import-add-path,$(LOCAL_PATH)/../../../)
  
LOCAL_MODULE := pocketsphinx_dy
  
LOCAL_SRC_FILES := ../../../3rd/pocketsphinx/src/libpocketsphinx/acmod.c\
    ../../../3rd/pocketsphinx/src/libpocketsphinx/allphone_search.c\
    ../../../3rd/pocketsphinx/src/libpocketsphinx/bin_mdef.c\
    ../../../3rd/pocketsphinx/src/libpocketsphinx/blkarray_list.c\
    ../../../3rd/pocketsphinx/src/libpocketsphinx/dict.c\
    ../../../3rd/pocketsphinx/src/libpocketsphinx/dict2pid.c\
    ../../../3rd/pocketsphinx/src/libpocketsphinx/fsg_history.c\
    ../../../3rd/pocketsphinx/src/libpocketsphinx/fsg_lextree.c\
    ../../../3rd/pocketsphinx/src/libpocketsphinx/fsg_search.c\
    ../../../3rd/pocketsphinx/src/libpocketsphinx/hmm.c\
    ../../../3rd/pocketsphinx/src/libpocketsphinx/kws_detections.c\
    ../../../3rd/pocketsphinx/src/libpocketsphinx/kws_search.c\
    ../../../3rd/pocketsphinx/src/libpocketsphinx/mdef.c\
    ../../../3rd/pocketsphinx/src/libpocketsphinx/ms_gauden.c\
    ../../../3rd/pocketsphinx/src/libpocketsphinx/ms_mgau.c\
    ../../../3rd/pocketsphinx/src/libpocketsphinx/ms_senone.c\
    ../../../3rd/pocketsphinx/src/libpocketsphinx/ngram_search.c\
    ../../../3rd/pocketsphinx/src/libpocketsphinx/ngram_search_fwdflat.c\
    ../../../3rd/pocketsphinx/src/libpocketsphinx/ngram_search_fwdtree.c\
    ../../../3rd/pocketsphinx/src/libpocketsphinx/phone_loop_search.c\
    ../../../3rd/pocketsphinx/src/libpocketsphinx/pocketsphinx.c\
	../../../3rd/pocketsphinx/src/libpocketsphinx/ps_alignment.c\
    ../../../3rd/pocketsphinx/src/libpocketsphinx/ps_lattice.c\
    ../../../3rd/pocketsphinx/src/libpocketsphinx/ps_mllr.c\
    ../../../3rd/pocketsphinx/src/libpocketsphinx/ptm_mgau.c\
    ../../../3rd/pocketsphinx/src/libpocketsphinx/s2_semi_mgau.c\
	../../../3rd/pocketsphinx/src/libpocketsphinx/state_align_search.c\
    ../../../3rd/pocketsphinx/src/libpocketsphinx/tmat.c\
    ../../../3rd/pocketsphinx/src/libpocketsphinx/vector.c
   
LOCAL_C_INCLUDES :=	$(LOCAL_PATH)/../../../3rd/pocketsphinx/include  \
	$(LOCAL_PATH)/../../../3rd/sphinxbase/include/android 			\
	$(LOCAL_PATH)/../../../3rd/sphinxbase/include	

LOCAL_STATIC_LIBRARIES := sphinxbase_static

LOCAL_CFLAGS := -fpic -DHAVE_CONFIG_H -DANDROID -fvisibility=hidden
  
include $(BUILD_SHARED_LIBRARY)

$(call import-module,3rd/sphinxbase)