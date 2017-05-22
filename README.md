## ccbasic
ccbasic是一个C++的基础库，目的是跨手机（ios，android），pc（linux，windows，mac）的通用基础库

## 编译 
```
For Windows：
mkdir build & cd build
cmake ..

For Linux
mkdir build & cd build
cmake ..

For Mac
目前没有用到

For Android
Android.mk

For IOS
lib/mac
```
## 功能
```
1.内存池的管理
2.基础字符串
3.通用数据结构
4.通用异常保护机制（堆栈回溯）
5.通用系统函数实现
6.动态库支持（动态替换）
7.通用成员函数指针实现
8.基于libevent实现的网络（windows下目前还是select模式）
9.支持sqlite
10.支持线程
11.支持时间处理,定时器模块（时间轮实现）
12.支持正则
13.支持协程
14.通用日志和错误模块
15.文件处理，压缩
```
## 附加功能
```
1.通用网络框架实现（C,S）
2.通用http框架实现
3.支持RSA加解密
4.支持JSON
5.支持lua, lpeg解析, sproto协议（修改过）
```


