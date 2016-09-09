#Õ®”√makefile
#√¸¡Ó£∫
#±‡“Î◊º±∏: make config
#±‡“Îdebug∞Ê£∫make 
#±‡“Îrelease∞Ê£∫make -e DEBUG=0

linux : MakeLinux

UpdateSubModuleLinux : 
	 git submodule update --init
	
MakeLinux : UpdateSubModuleLinux
	cd lib/linux && $(MAKE)


clean : 
	cd lib/linux && $(MAKE) clean

