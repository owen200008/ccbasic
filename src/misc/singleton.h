/***********************************************************************************************
// 文件名:     singleton.h
// 创建者:     蔡振球
// Email:      zqcai@w.cn
// 创建时间:   2012/2/17 12:06:43
// 内容描述:   实现设计模式 Singleton ，是一种改进的全局变量。
// 版本信息:   1.0V
************************************************************************************************/
#ifndef BASIC_SINGLETON_H
#define BASIC_SINGLETON_H

#pragma once
__NS_BASIC_START
//////////////////////////////////////////////////////////////////////////////////////////////////
//声明
#define ST_LIFETIME_LIFO		0		//last-in, first-out (LIFO) order
#define ST_LIFETIME_NODESTROY	-1		//不销毁对象

template<class T, int nLifetime = ST_LIFETIME_LIFO>
class CBasicSingleton 
{
public:
	static T& Instance() // Unique point of access
	{ 
		if (0 == _instance) 
		{
			CSingleLock lock(&GetMutex());
			lock.Lock();
			if (0 == _instance) 
			{
				_instance = new T();
				if(nLifetime == ST_LIFETIME_LIFO)
				{
					atexit(Destroy);  //注册销毁函数，实现 LIFO
				}
			}
		}
		return *_instance;
	}
	static T* GetInstance()
	{
		return _instance;
	}
protected:
	CBasicSingleton(){}
	~CBasicSingleton(){}
private:
	CBasicSingleton(const CBasicSingleton&);
	CBasicSingleton& operator=(const CBasicSingleton&);
private:
	static void Destroy() // Destroy the only instance
	{ 
		if ( _instance != 0 ) 
		{
			delete _instance;
			_instance = 0;
		}
	}
	static CCriticalSection& GetMutex()
	{
		static CCriticalSection _mutex;
		return _mutex;
	}
	
	static T * volatile _instance;				// The one and only instance
};

//template<class T, int nLifetime> CCriticalSection CBasicSingleton<T, nLifetime>::_mutex;
template<class T, int nLifetime> T * volatile CBasicSingleton<T, nLifetime>::_instance = 0;

__NS_BASIC_END
//////////////////////////////////////////////////////////////////////////////////////////////////
#endif 
