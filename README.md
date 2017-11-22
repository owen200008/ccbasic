## ccbasic
ccbasic is c++ basic lib. make it support ios, android, linux, windows.

## compile
```
It support cmake compile. 
It contain cryptopp so please make sure your cpu support SSSE3 or AES-NI and so on. 
If not support please add param to disable. 

For windows, please open the Native Tools Command and run
src/script/winmakefile.bat   (create the lua lib)
mkdir build && cd build
cmake ..

For others
(first to make the lua lib)
mkdir build && cd build
cmake ..

```
## modules
```
1.memory pool mgr
2.basic string
3.normal datastruct
4.stacktrace
5.sysinfo function
6.dll support
7.function point(like std::function but useful)
8.net frame
9.support sqlite
10.supoort thread
11.support time，time sprit，ontimer module（time wheel）
12.support regular
13.support corutine（linux，windows use）
14.support log， error module
15.support file，ini，zip
```
## extra modules
```
1.http frame
2.support RSA
3.support JSON
4.support lua, lpeg, basicsproto 
```


