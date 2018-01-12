## ccbasic
ccbasic is c++ basic lib. make it support ios, android, linux, windows.

## compile
```
It support cmake compile. default use release
1.make static
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug

2. make shared
mkdir builddll &&  cd builddll
cmake .. -DCMAKE_BUILD_TYPE=Debug -DBASICLIB_BUILD_SHARED=ON

Note:
the best way to use mingw

=>For windows
libevent always create static and shared, so if you use static unload the event_shared, otherwise the static lib will overwrite by the dll lib。if you use shared same way to unload event_static

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


