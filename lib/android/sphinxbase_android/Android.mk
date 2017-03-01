####################################  
# Build sphinxbase as separate library  
  
LOCAL_PATH := $(call my-dir)  
  
include $(CLEAR_VARS)  
  
$(call import-add-path,$(LOCAL_PATH)/../)

LOCAL_MODULE := sphinxbase_static  
  
LOCAL_MODULE_TAGS := optional   
  
LOCAL_SRC_FILES := ../../../3rd/sphinxbase/src/libsphinxad/ad_openal.c\
    ../../../3rd/sphinxbase/src/libsphinxbase/feat/agc.c\
    ../../../3rd/sphinxbase/src/libsphinxbase/feat/cmn.c\
    ../../../3rd/sphinxbase/src/libsphinxbase/feat/cmn_live.c\
    ../../../3rd/sphinxbase/src/libsphinxbase/feat/feat.c\
    ../../../3rd/sphinxbase/src/libsphinxbase/feat/lda.c\
    ../../../3rd/sphinxbase/src/libsphinxbase/fe/fe_interface.c\
    ../../../3rd/sphinxbase/src/libsphinxbase/fe/fe_noise.c\
    ../../../3rd/sphinxbase/src/libsphinxbase/fe/fe_prespch_buf.c\
    ../../../3rd/sphinxbase/src/libsphinxbase/fe/fe_sigproc.c\
    ../../../3rd/sphinxbase/src/libsphinxbase/fe/fe_warp_affine.c\
    ../../../3rd/sphinxbase/src/libsphinxbase/fe/fe_warp.c\
    ../../../3rd/sphinxbase/src/libsphinxbase/fe/fe_warp_inverse_linear.c\
    ../../../3rd/sphinxbase/src/libsphinxbase/fe/fe_warp_piecewise_linear.c\
    ../../../3rd/sphinxbase/src/libsphinxbase/fe/fixlog.c\
    ../../../3rd/sphinxbase/src/libsphinxbase/fe/yin.c\
    ../../../3rd/sphinxbase/src/libsphinxbase/lm/fsg_model.c\
    ../../../3rd/sphinxbase/src/libsphinxbase/lm/jsgf.c\
    ../../../3rd/sphinxbase/src/libsphinxbase/lm/jsgf_parser.c\
    ../../../3rd/sphinxbase/src/libsphinxbase/lm/jsgf_scanner.c\
    ../../../3rd/sphinxbase/src/libsphinxbase/lm/ngrams_raw.c\
    ../../../3rd/sphinxbase/src/libsphinxbase/lm/lm_trie.c\
    ../../../3rd/sphinxbase/src/libsphinxbase/lm/lm_trie_quant.c\
    ../../../3rd/sphinxbase/src/libsphinxbase/lm/ngram_model.c\
    ../../../3rd/sphinxbase/src/libsphinxbase/lm/ngram_model_set.c\
    ../../../3rd/sphinxbase/src/libsphinxbase/lm/ngram_model_trie.c\
    ../../../3rd/sphinxbase/src/libsphinxbase/util/bio.c\
    ../../../3rd/sphinxbase/src/libsphinxbase/util/bitarr.c\
    ../../../3rd/sphinxbase/src/libsphinxbase/util/bitvec.c\
    ../../../3rd/sphinxbase/src/libsphinxbase/util/blas_lite.c\
    ../../../3rd/sphinxbase/src/libsphinxbase/util/case.c\
    ../../../3rd/sphinxbase/src/libsphinxbase/util/ckd_alloc.c\
    ../../../3rd/sphinxbase/src/libsphinxbase/util/cmd_ln.c\
    ../../../3rd/sphinxbase/src/libsphinxbase/util/dtoa.c\
    ../../../3rd/sphinxbase/src/libsphinxbase/util/err.c\
	../../../3rd/sphinxbase/src/libsphinxbase/util/errno.c\
    ../../../3rd/sphinxbase/src/libsphinxbase/util/f2c_lite.c\
    ../../../3rd/sphinxbase/src/libsphinxbase/util/filename.c\
    ../../../3rd/sphinxbase/src/libsphinxbase/util/genrand.c\
    ../../../3rd/sphinxbase/src/libsphinxbase/util/glist.c\
    ../../../3rd/sphinxbase/src/libsphinxbase/util/hash_table.c\
    ../../../3rd/sphinxbase/src/libsphinxbase/util/heap.c\
    ../../../3rd/sphinxbase/src/libsphinxbase/util/listelem_alloc.c\
    ../../../3rd/sphinxbase/src/libsphinxbase/util/logmath.c\
    ../../../3rd/sphinxbase/src/libsphinxbase/util/matrix.c\
    ../../../3rd/sphinxbase/src/libsphinxbase/util/mmio.c\
    ../../../3rd/sphinxbase/src/libsphinxbase/util/pio.c\
    ../../../3rd/sphinxbase/src/libsphinxbase/util/priority_queue.c\
    ../../../3rd/sphinxbase/src/libsphinxbase/util/profile.c\
    ../../../3rd/sphinxbase/src/libsphinxbase/util/sbthread.c\
    ../../../3rd/sphinxbase/src/libsphinxbase/util/slamch.c\
    ../../../3rd/sphinxbase/src/libsphinxbase/util/slapack_lite.c\
    ../../../3rd/sphinxbase/src/libsphinxbase/util/strfuncs.c
   
LOCAL_C_INCLUDES :=	$(LOCAL_PATH)/../../../3rd/sphinxbase/include/android 			\
					$(LOCAL_PATH)/../../../3rd/sphinxbase/include	  \
					$(LOCAL_PATH)/../../../3rd/sphinxbase/include/sphinxbase 			\
					$(LOCAL_PATH)/../../../3rd/openal-soft/include 			\
					$(LOCAL_PATH)/../../../3rd/openal-soft/include/AL 			\

LOCAL_STATIC_LIBRARIES := openal_static
					
LOCAL_CFLAGS := -DHAVE_CONFIG_H -DANDROID -fvisibility=hidden  
  
include $(BUILD_STATIC_LIBRARY) 

$(call import-module,openal_android)