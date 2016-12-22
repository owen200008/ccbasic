cd ../../../3rd/lua/src
cl /O2 /W3 /c /DLUA_BUILD_AS_DLL l*.c
del lua.obj luac.obj
link /DLL /out:../../../lib/lua53.dll l*.obj
cl /O2 /W3 /c /DLUA_BUILD_AS_DLL lua.c luac.c
link /out:../../../lib/lua.exe lua.obj lua53.lib 
del lua.obj
link /out:../../../lib/luac.exe l*.obj 
del *.obj
cd ..
