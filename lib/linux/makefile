#Õ®”√makefile
#√¸¡Ó£∫
#±‡“Î◊º±∏: make config
#±‡“Îdebug∞Ê£∫make 
#±‡“Îrelease∞Ê£∫make -e DEBUG=0


ROOTDIR := ../../src
SRCDIRS :=$(ROOTDIR)/algorithm $(ROOTDIR)/algorithm/rc5 $(ROOTDIR)/algorithm/aes $(ROOTDIR)/algorithm/sha1 $(ROOTDIR)/algorithm/zip $(ROOTDIR)/util 
SRCDIRS :=$(SRCDIRS) $(ROOTDIR)/datastruct $(ROOTDIR)/debug $(ROOTDIR)/file
SRCDIRS :=$(SRCDIRS) $(ROOTDIR)/graphics $(ROOTDIR)/i18n $(ROOTDIR)/log $(ROOTDIR)/mem
SRCDIRS :=$(SRCDIRS) $(ROOTDIR)/net $(ROOTDIR)/net/socket $(ROOTDIR)/sys $(ROOTDIR)/thread
SRCDIRS :=$(SRCDIRS) $(ROOTDIR)/dll $(ROOTDIR)/file/ini $(ROOTDIR)/util/strutil
SRCDIRS :=$(SRCDIRS) $(ROOTDIR)/misc
SRCDIRS :=$(SRCDIRS) $(ROOTDIR)/time $(ROOTDIR)/time/timelib $(ROOTDIR)/types $(ROOTDIR)/util/regex ${ROOTDIR}/util/strutil/charset
SRCEXTS :=.cpp .c .S
OUTDIR :=.
DEBUG_DIR :=$(OUTDIR)/debug/
RELEASE_DIR :=$(OUTDIR)/release/

DEBUG=1

ifeq ($(DEBUG),1)
PROGRAMDIR := ..
OBJDIR :=$(DEBUG_DIR)
CPPFLAGS :=-g -D_DEBUG -shared -fPIC
PROGRAM :=$(PROGRAMDIR)/basiclib.a
else
PROGRAMDIR := ..
OBJDIR :=$(RELEASE_DIR)
CPPFLAGS :=-O2 -shared -fPIC
PROGRAM :=$(PROGRAMDIR)/basiclib.a
endif


CC  = gcc
CXX = g++
AR=ar

RM = rm -f

SHELL = /bin/sh

all : config $(PROGRAM)


JEMALLOC_PATH := ../../3rd/jemalloc
JEMALLOC_STATICLIB := $(JEMALLOC_PATH)/lib/libjemalloc_pic.a
JEMALLOC_INC := $(JEMALLOC_PATH)/include/jemalloc
$(JEMALLOC_STATICLIB) : $(JEMALLOC_PATH)/Makefile
	cd $(JEMALLOC_PATH) && $(MAKE) CC=$(CC)
$(JEMALLOC_PATH)/Makefile :
	cd $(JEMALLOC_PATH) && ./autogen.sh --with-jemalloc-prefix=je_ --disable-valgrind

CPPFLAGS := $(CPPFLAGS) -DLINUX -D__LINUX -I$(JEMALLOC_INC) -fshort-wchar -D_LARGEFILE_SOURCE -D_LARGEFILE_SOURCE64 -D_FILE_OFFSET_BITS=64 -Wno-deprecated
INLIBS := $(JEMALLOC_STATICLIB)
LDFLAGS := -lgcc_s -lpthread -ldl -lexpat  -lbfd -liberty -levent
CFLAGS :=
CXXFLAGS := 

FULLSOURCES = $(foreach d,$(SRCDIRS),$(wildcard $(addprefix $(d)/*,$(SRCEXTS))))
SOURCES = $(notdir $(FULLSOURCES))
SOURCES := $(SOURCES) call_stack_gcc.cpp coroutineplus.cpp libco_coroutine.cpp coctx_swap.S
OBJS = $(foreach x,$(SRCEXTS), $(patsubst %$(x),%.o,$(filter %$(x),$(SOURCES))))
FULLOBJS = $(addprefix $(OBJDIR),$(OBJS))
DEPS    = $(patsubst %.o,%.d,$(OBJS))

SRCDIRS := $(SRCDIRS) $(ROOTDIR)/exception $(ROOTDIR)/coroutine

vpath %.c $(SRCDIRS)
vpath %.cpp $(SRCDIRS)
vpath %.S $(ROOTDIR)/coroutine
vpath %.o $(OBJDIR)

.PHONY : all objs clean cleanall rebuild link cleanout

# Rules for producing the objects.
#---------------------------------------------------
objs : $(OBJS)

%.o : %.c
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $< -o $(OBJDIR)$(notdir $@)

%.o : %.cpp
	@echo -e "building: $(notdir $@) \t please wait ..."
	$(CXX) -c -std=c++11 $(CPPFLAGS) $(CXXFLAGS) $< -o $(OBJDIR)$(notdir $@)

%.o : %.S
	$(CC) -c $< -o $(OBJDIR)$(notdir $@)

# Rules for producing the executable.
#----------------------------------------------
$(PROGRAM) : $(OBJS)
ifeq ($(findstring .a, $(PROGRAM)), .a)
	$(AR) -rc $(PROGRAM) $(FULLOBJS) $(INLIBS)
else
ifeq ($(strip $(SRCEXTS)), .c) # C file
	$(CC) -o $(PROGRAM) $(FULLOBJS) $(INLIBS) $(LDFLAGS)
else # C++ file
	$(CXX) -o $(PROGRAM) $(FULLOBJS) $(INLIBS) $(LDFLAGS)
endif
endif

#-include $(DEPS)

test:
	@echo -e "objs: $(OBJS)"
	@echo -e "all objs: $(FULLOBJS)"

config: dir $(JEMALLOC_STATICLIB)
dir:
	@echo -e "making dir..."
	mkdir -p $(OUTDIR)
	mkdir -p $(DEBUG_DIR)
	mkdir -p $(RELEASE_DIR)

rebuild: clean all

link: cleanout all

cleanout:
	@$(RM) $(PROGRAM)

clean :
	@$(RM) $(FULLOBJS) 
	@$(RM) $(DEPS) 

cleanall: clean cleanout
ifneq (,$(wildcard $(JEMALLOC_PATH)/Makefile))
	cd $(JEMALLOC_PATH) && $(MAKE) clean
endif




