#Õ®”√makefile
#√¸¡Ó£∫
#±‡“Î◊º±∏: make config
#±‡“Îdebug∞Ê£∫make 
#±‡“Îrelease∞Ê£∫make -e DEBUG=0

linux : MakeLinux

UpdateSubModuleLinux : 
	 git submodule update --init
MakeBasicLib:
	cd lib/linux && $(MAKE)

MakeLinux : UpdateSubModuleLinux MakeBasicLib
	cd basic_test && $(MAKE)


LIBEVENT_PATH := 3rd/libevent

libevent : 
	cd $(LIBEVENT_PATH) && ./autogen.sh && ./configure && make && make install

clean : 
	cd lib/linux && $(MAKE) clean

