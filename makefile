#Í¨ÓÃmakefile
#ÃüÁî£º
#±àÒë×¼±¸: make config
#±àÒëdebug°æ£ºmake 
#±àÒërelease°æ£ºmake -e DEBUG=0

linux : MakeLinux

UpdateSubModuleLinux : 
	 git submodule update --init
	
MakeLinux : UpdateSubModuleLinux
	cd lib/linux && $(MAKE)




