ROOTDIR := ./
SRCDIRS :=$(ROOTDIR) $(ROOTDIR)/comm $(ROOTDIR)/coroutine $(ROOTDIR)/dll $(ROOTDIR)/exception $(ROOTDIR)/misc $(ROOTDIR)/net $(ROOTDIR)/thread $(ROOTDIR)/time $(ROOTDIR)/util
SRCDIRS := $(SRCDIRS) $(ROOTDIR)/../src/scbasic/commu $(ROOTDIR)/../src/scbasic/encode 
SRCEXTS :=.cpp .c .S

CC := gcc
CXX := g++
PLAT := linux

PROJECTNAME := basic_test
release := 0
ifeq ($(release),1)
RELEASEFLAG := -O2
else
RELEASEFLAG := -g -D_DEBUG
endif

all : compileproject

total : compileall

CCBASIC_PATH := ../../ccbasic
CCBASIC_STATICLIB := $(CCBASIC_PATH)/lib/basiclib.a
CCBASIC_INC := $(CCBASIC_PATH)/src/inc
$(CCBASIC_STATICLIB) : 
	cd $(CCBASIC_PATH)/lib/linux && $(MAKE)

CCBASIC_LIBEVENTLIB := $(CCBASIC_PATH)/3rd/libevent/.libs/libevent.a
$(CCBASIC_LIBEVENTLIB):
	cd $(CCBASIC_PATH)/3rd/libevent && ./autogen.sh && sh configure && $(MAKE)

CCBASIC_LIBCRYPTOPPLIB :=  $(CCBASIC_PATH)/3rd/cryptopp/libcryptopp.a
$(CCBASIC_LIBCRYPTOPPLIB):
	cd  $(CCBASIC_PATH)/3rd/cryptopp && cmake -DDISABLE_NATIVE_ARCH=TRUE -DBUILD_TESTING=OFF && make -f Makefile


CFLAGS := $(RELEASEFLAG) -Wno-deprecated -D__LINUX -I$(CCBASIC_INC) -I$(CCBASIC_INC) -I$(CCBASIC_INC)/../scbasic -I$(CCBASIC_PATH)/3rd/cryptopp
CPPFLAGS := $(RELEASEFLAG) -Wno-deprecated -std=c++11 -D__LINUX -I$(CCBASIC_INC) -I$(CCBASIC_INC)/../scbasic -I$(CCBASIC_PATH)/3rd/cryptopp
INLIBS := $(CCBASIC_STATICLIB) $(CCBASIC_LIBEVENTLIB) $(CCBASIC_LIBCRYPTOPPLIB) -pthread -lrt -ldl $(CCBASIC_PATH)/3rd/jemalloc/lib/libjemalloc_pic.a
OUTPUT_PATH := ./
COMPILEOBJDIR := ./obj/
FULLSOURCES = $(foreach d,$(SRCDIRS),$(wildcard $(addprefix $(d)/*,$(SRCEXTS))))
SOURCES = $(notdir $(FULLSOURCES))
#add file source
COMPILESOURCE := $(SOURCES)
COMPILEOBJS := $(foreach x,$(SRCEXTS), $(patsubst %$(x),%.o,$(filter %$(x),$(COMPILESOURCE))))
COMPILEFULLOBJS := $(addprefix $(COMPILEOBJDIR),$(COMPILEOBJS))
#add dir source
SRCDIRS := $(SRCDIRS)
vpath %.c $(SRCDIRS)
vpath %.cpp $(SRCDIRS)
vpath %.S $(SRCDIRS)
vpath %.o ${COMPILEOBJDIR}
config : 
	mkdir -p $(COMPILEOBJDIR)
%.o : %.c
	$(CC) -c $(CFLAGS) $< -o $(COMPILEOBJDIR)$(notdir $@)
%.o : %.cpp
	$(CXX) -c $(CPPFLAGS) $< -o $(COMPILEOBJDIR)$(notdir $@)
%.o : %.S
	$(CC) -c $< -o $(COMPILEOBJDIR)$(notdir $@)

MAKEFILEEIM := $(OUTPUT_PATH)/$(PROJECTNAME)
$(MAKEFILEEIM) : config $(COMPILEOBJS)
	$(CXX) -o $@ $(COMPILEFULLOBJS) $(INLIBS)

compileall : $(CCBASIC_STATICLIB) $(CCBASIC_LIBEVENTLIB) $(CCBASIC_LIBCRYPTOPPLIB) $(MAKEFILEEIM)
compileproject : $(OUTPUT_PATH)/$(PROJECTNAME)

clean :
	rm -rf obj
	rm $(MAKEFILEEIM)

cleanall:
	cd $(CCBASIC_PATH)/lib/linux && $(MAKE) cleanall

	

