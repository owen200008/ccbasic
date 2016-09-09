#Õ®”√makefile
#√¸¡Ó£∫
#±‡“Î◊º±∏: make config
#±‡“Îdebug∞Ê£∫make 
#±‡“Îrelease∞Ê£∫make -e DEBUG=0

PLAT ?= none
PLATS = linux

linux : MakeLinux

UpdateSubModuleLinux :
	PLAT = linux
	git submodule update --init
	
MakeLinux : UpdateSubModuleLinux 
	include lib/linux/makefile




