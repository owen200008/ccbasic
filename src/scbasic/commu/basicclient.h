/***********************************************************************************************
// 文件名:     basicclient.h
// 创建者:     owen
// Email:      zqcai@w.cn
// 创建时间:   2012-3-10 18:35:56
// 内容描述:   定义客户端通讯类
// 版本信息:   1.0V
// 使用说明:   直接使用CBasicClientSession类只要绑定三个函数处理:
				OnConnect响应
				OnReceive响应
				OnIdle响应,空闲时间的处理
************************************************************************************************/
#ifndef BASIC_BASICCLIENT_H
#define BASIC_BASICCLIENT_H

#include "../../inc/basic.h"

////////////////////////////////////////////////////////////////////////////////////////////////
//连接管理
class CCommonClientSession : public basiclib::CBasicSessionNetClient
{
public:
	static CCommonClientSession* CreateCCommonClientSession(Net_UInt nSessionID){ return new CCommonClientSession(nSessionID); }

protected:
	CCommonClientSession(Net_UInt nSessionID);
	virtual ~CCommonClientSession();
public:
	virtual Net_Int Connect(const char* lpszAddress);
	virtual Net_Int OnDisconnect(Net_UInt dwNetCode);

	//！ 自身不存在定时器,由外部调用
	virtual void OnTimer(Net_UInt nTickTime);
};

#endif 
