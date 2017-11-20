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
11.支持时间处理,定时器模块（时间轮实现）
12.支持正则
13.支持协程（linux，windows正常，其他还没测试）
14.通用日志和错误模块
15.文件处理，压缩
```
## extra modules
```
1.net frame
2.http frame
3.support RSA
4.support JSON
5.support lua, lpeg, basicsproto 
```


