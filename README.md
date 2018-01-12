## ccbasic
ccbasic is c++ basic lib. make it support ios, android, linux, windows.

## compile
```
It support cmake compile. 

For windows
mkdir build && cd build
cmake ..
(cryptopp compile error when asm code compile，because vs not support create dir for asm，so you can delete rdrand.asm compile again to create dir first and goto build cmake .. again and compile)

For centos
mkdir build && cd build
cmake ..

For Android
use android studio

For Mac
mkdir build && cd build
cmake -G Xcode ..

For IOS
mkdir build && cd build
cmake -G Xcode ..
change the sdk to ios

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


