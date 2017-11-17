cd ../../3rd/lua/src
cl /O2 /W3 /c /DLUA_BUILD_AS_DLL l*.c
del lua.obj luac.obj
link /DLL /out:../../../libs/lua53.dll l*.obj
cl /O2 /W3 /c /DLUA_BUILD_AS_DLL lua.c luac.c
link /out:../../../libs/lua.exe lua.obj ../../../libs/lua53.lib 
del lua.obj
link /out:../../../libs/luac.exe l*.obj 
del *.obj
cd ..
