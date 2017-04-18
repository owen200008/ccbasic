ROOTDIR := ./
SRCDIRS :=$(ROOTDIR) $(ROOTDIR)/comm $(ROOTDIR)/coroutine $(ROOTDIR)/dll $(ROOTDIR)/exception $(ROOTDIR)/misc $(ROOTDIR)/net $(ROOTDIR)/thread $(ROOTDIR)/time $(ROOTDIR)/util
SRCEXTS :=.cpp .c .S

CC := gcc
CXX := g++
PLAT := linux

PROJECTNAME := basic_test
release := 0
ifeq ($(release),1)
RELEASEFLAG := -O2
else
RELEASEFLAG := -g
endif

all : compileproject

total : compileall

CCBASIC_PATH := ../../ccbasic
CCBASIC_STATICLIB := $(CCBASIC_PATH)/lib/basiclib.a
CCBASIC_INC := $(CCBASIC_PATH)/src/inc
$(CCBASIC_STATICLIB) : 
	cd $(CCBASIC_PATH)/lib/linux && $(MAKE)

CFLAGS := $(RELEASEFLAG) -Wno-deprecated -D__LINUX -I$(CCBASIC_INC)
CPPFLAGS := $(RELEASEFLAG) -Wno-deprecated -std=c++11 -D__LINUX -I$(CCBASIC_INC) -I$(CCBASIC_INC)/../scbasic -I$(CCBASIC_PATH)/3rd/cryptopp
INLIBS := $(CCBASIC_STATICLIB) -pthread -lrt -ldl $(CCBASIC_PATH)/3rd/jemalloc/lib/libjemalloc_pic.a
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

compileall : UpdateSubModule $(CCBASIC_STATICLIB) $(MAKEFILEEIM)
compileproject : $(OUTPUT_PATH)/$(PROJECTNAME)

clean :
	rm -rf obj
	rm $(MAKEFILEEIM)

cleanall:
	cd $(CCBASIC_PATH)/lib/linux && $(MAKE) cleanall

	

