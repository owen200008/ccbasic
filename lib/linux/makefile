#Õ®”√makefile
#√¸¡Ó£∫
#±‡“Î◊º±∏: make config
#±‡“Îdebug∞Ê£∫make 
#±‡“Îrelease∞Ê£∫make -e DEBUG=0


ROOTDIR := ../..
SRCDIRS :=$(ROOTDIR)/algorithm $(ROOTDIR)/algorithm/rc5 $(ROOTDIR)/algorithm/aes $(ROOTDIR)/algorithm/sha1 $(ROOTDIR)/algorithm/zip $(ROOTDIR)/util 
SRCDIRS :=$(SRCDIRS) $(ROOTDIR)/datastruct $(ROOTDIR)/debug $(ROOTDIR)/exception $(ROOTDIR)/file
SRCDIRS :=$(SRCDIRS) $(ROOTDIR)/graphics $(ROOTDIR)/i18n $(ROOTDIR)/log $(ROOTDIR)/mem
SRCDIRS :=$(SRCDIRS) $(ROOTDIR)/net $(ROOTDIR)/net/socket $(ROOTDIR)/sys $(ROOTDIR)/thread
SRCDIRS :=$(SRCDIRS) $(ROOTDIR)/dll $(ROOTDIR)/file/ini $(ROOTDIR)/util/strutil
SRCDIRS :=$(SRCDIRS) $(ROOTDIR)/misc
SRCDIRS :=$(SRCDIRS) $(ROOTDIR)/time $(ROOTDIR)/time/timelib $(ROOTDIR)/types $(ROOTDIR)/util/regex ${ROOTDIR}/util/strutil/charset
SRCEXTS :=.cpp .c
OUTDIR :=.
DEBUG_DIR :=$(OUTDIR)/debug/
RELEASE_DIR :=$(OUTDIR)/release/

DEBUG=0

ifeq ($(DEBUG),1)
PROGRAMDIR := ../../../def/lib
OBJDIR :=$(DEBUG_DIR)
CPPFLAGS :=-g -D_DEBUG -shared -fPIC
PROGRAM :=$(PROGRAMDIR)/basiclibD.a
else
PROGRAMDIR := ../../../def/lib
OBJDIR :=$(RELEASE_DIR)
CPPFLAGS :=-g -D_USE_SYS_MALLOC -O2 -shared -fPIC
#CPPFLAGS :=-g -DLINUX -D_USE_SYS_MALLOC 
PROGRAM :=$(PROGRAMDIR)/basiclib.a
endif

INLIBS  :=

CPPFLAGS := $(CPPFLAGS) -DLINUX -D__LINUX -I../../../def/inc -fshort-wchar -D_LARGEFILE_SOURCE -D_LARGEFILE_SOURCE64 -D_FILE_OFFSET_BITS=64 -Wno-deprecated
LDFLAGS := -lgcc_s -lpthread -levent -ldl -lexpat  -lbfd -liberty
CFLAGS :=
CXXFLAGS := 

CC  = gcc
CXX = g++
AR=ar

RM = rm -f

SHELL = /bin/sh
FULLSOURCES = $(foreach d,$(SRCDIRS),$(wildcard $(addprefix $(d)/*,$(SRCEXTS))))
SOURCES = $(notdir $(FULLSOURCES))
OBJS = $(foreach x,$(SRCEXTS), $(patsubst %$(x),%.o,$(filter %$(x),$(SOURCES))))
FULLOBJS = $(addprefix $(OBJDIR),$(OBJS))
DEPS    = $(patsubst %.o,%.d,$(OBJS))

vpath %.c $(SRCDIRS)
vpath %.cpp $(SRCDIRS)
vpath %.o $(OBJDIR)

.PHONY : all objs clean cleanall rebuild link cleanout

all : config $(PROGRAM)

# Rules for producing the objects.
#---------------------------------------------------
objs : $(OBJS)

%.o : %.c
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $< -o $(OBJDIR)$(notdir $@)

%.o : %.cpp
	@echo -e "building: $(notdir $@) \t please wait ..."
	$(CXX) -c -std=c++11 $(CPPFLAGS) $(CXXFLAGS) $< -o $(OBJDIR)$(notdir $@)


# Rules for producing the executable.
#----------------------------------------------
$(PROGRAM) : $(OBJS)
ifeq ($(findstring .a, $(PROGRAM)), .a)
	$(AR) -rc $(PROGRAM) $(FULLOBJS)
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

config: dir
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




