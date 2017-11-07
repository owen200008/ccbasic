/***********************************************************************************************
// 文件名:     net.h
// 创建者:     蔡振球
// Email:      zqcai@w.cn
// 创建时间:   2016-9-12 11:50:18
// 内容描述:   定义TCP通信的基本类
// 版本信息:   1.0V
************************************************************************************************/
#ifndef BASIC_NET_H
#define BASIC_NET_H

/////////////////////////////////////////////////////////////////////////////////////////////
#include "sendbuffer.h"

__NS_BASIC_START
/////////////////////////////////////////////////////////////////////////////////////////////
//声明
//class CBasicObject;
class CBasicSessionNet;				//基类
class CBasicSessionNetNotify;		//主动连接

struct BasicNetStat;		//发送接收的统计信息
/////////////////////////////////////////////////////////////////////////////////////////////
//返回错误定义
#define NET_ERROR								(0xE0000000 | _ERROR_NET)	    //网络错误

#define BASIC_NET_OK							0								//成功，没有错误

#define BASIC_NET_GENERIC_ERROR					(NET_ERROR | 0x0001)			//一般性错误，即未知的错误
#define BASIC_NET_OUTOF_MAXPACKET				(NET_ERROR | 0x0002)			//超出最大数据包的限制
#define BASIC_NET_NO_MEMORY						(NET_ERROR | 0x0003)			//内存不足
#define BASIC_NET_ADDRESS_ERROR					(NET_ERROR | 0x0004)			//地址错误
#define BASIC_NET_BIND_ERROR					(NET_ERROR | 0x0005)			//bind 函数调用错误
#define BASIC_NET_LISTEN_ERROR					(NET_ERROR | 0x0006)			//listen 函数调用错误
#define BASIC_NET_TOCLOSE_ERROR					(NET_ERROR | 0x0007)			//正在关闭
#define BASIC_NET_CONNECTING_ERROR				(NET_ERROR | 0x0008)			//正在连接
#define BASIC_NET_ALREADY_LISTEN				(NET_ERROR | 0x0009)			//已经监听
#define BASIC_NET_NO_CONNECT					(NET_ERROR | 0x000a)			//没有连接
#define BASIC_NET_ALREADY_CONNECT				(NET_ERROR | 0x000b)			//已经连接
#define BASIC_NET_INVALID_ADDRESS				(NET_ERROR | 0x000c)			//地址非法
#define BASIC_NET_FILTER_WRITE_FAIL				(NET_ERROR | 0x000d)			//过滤器处理 OnWriteData 失败
#define BASIC_NET_FILTER_READ_FAIL				(NET_ERROR | 0x000e)			//过滤器处理 OnReadData  失败
#define BASIC_NET_SOCKET_ERROR					(NET_ERROR | 0x000f)			//socket 函数调用错误
#define BASIC_NET_RELEASE_ERROR					(NET_ERROR | 0x0010)			//RELEASE错误

//返回的状态代码
#define BASIC_NETCODE_SUCC					0x00000001				//成功
#define BASIC_NETCODE_CLOSE_REMOTE			0x00000002				//服务端主动关闭
#define BASIC_NETCODE_FILTER_HANDLE			0x00000004				//过滤器处理过
#define BASIC_NETCODE_FILTER_ERROR			0x00000008				//过滤器处理错误

#define BASIC_NETCODE_CONNET_FAIL			0x00000040				//连接失败

//绑定函数 HandleConnect 返回值
#define BASIC_NET_HC_RET_HANDSHAKE			0x00000001		//连接需要握手过程

//绑定函数 HandleReceive 返回值
#define BASIC_NET_HR_RET_HANDSHAKE			0x00000001		//握手过程成功


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//! 过滤器返回值
#define PACK_FILTER_FAIL		-1		/*!<失败*/
#define PACK_FILTER_SKIP		0		/*!<成功，数据未改动*/
#define PACK_FILTER_NEXT		1		/*!<成功，还有数据需要继续处理*/
#define PACK_FILTER_HANDLED		2		/*!<成功，保留数据*/
#define PACK_FILTER_SEARCH		3		/*!<成功，传递给下一个*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*!
* 初始化通信所需的参数。
* \return 返回 BASIC_NET_*
* \remarks 在使用通信类以前必须调用该函数
*/
typedef basiclib::CBasicString(*pGetConfFunc)(const char* pParam);
_BASIC_DLL_API void SetNetInitializeGetParamFunc(pGetConfFunc func);
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class _BASIC_DLL_API CBasicPreSend : public CBasicObject
{
public:
	CBasicPreSend() {}

public:
	/*\brief 过滤收到的数据 */
	// 过滤生成的数据放入buf
	virtual int32_t OnPreReceive(const char *pszData, int32_t cbData, CBasicBitstream& buf, CBasicSessionNetNotify* pNetSession) = 0;

	/*\brief 过滤发送的数据 */
	// 过滤生成的数据放入buf
	virtual int32_t OnPreSend(const char *pszData, int32_t cbData, uint32_t dwFlag, SendBufferCacheMgr& sendBuf) = 0;

	/*\brief 构造新的实例 */
	// 用于Accept
	virtual CBasicPreSend* Construct() = 0;

	/*\brief 重置过滤器状态 */
	virtual void ResetPreSend() = 0;
};
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//! 发送接收的统计信息
/*!
*  用在统计发送接收的字节数和次数。溢出后从零开始。
*/
struct _BASIC_DLL_API  BasicNetStat{
	uint32_t	m_dwSendBytes;		/*!< 发送的字节数 */
	uint32_t	m_dwSendTimes;		/*!< 发送的次数 */
	uint32_t	m_dwReceBytes;		/*!< 接收的字节数 */
	uint32_t	m_dwReceTimes;		/*!< 接收的次数 */
	time_t		m_tmLastRecTime;	//记录最后收到数据的时间
	double		m_fLastSendRate;
	double		m_fLastRecvRate;
	time_t		m_tLastStat;
	BasicNetStat();
	void Empty();
	void OnSendData(int nSend);
	void OnReceiveData(int nRece);
	void GetTransRate(BasicNetStat& lastData, double& dSend, double& dRecv);
};
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define TIL_RESET_MASK					0xFF000000		//重置状态

#define TIL_SS_LINK						0x0000000F		//连接的状态
#define TIL_SS_IDLE						0x00000000		//空闲
#define TIL_SS_CONNECTING				0x00000001		//正在连接
#define TIL_SS_CONNECTED				0x00000002		//已连接
#define TIL_SS_LISTENING				0x00000003		//正在监听

#define TIL_SS_CLOSE					0x000000F0		//关闭的状态
#define TIL_SS_NORMAL					0x00000000		//正常状态
#define TIL_SS_TOCLOSE					0x00000010		//正在关闭

#define TIL_SS_SHAKEHANDLE_MASK			0x00000F00		//认证
#define TIL_SS_SHAKEHANDLE_TRANSMIT		0x00000100		//

//不会随着close 重置
#define TIL_SS_RELEASE_MASK				0xF0000000		//删除相关状态标示
#define TIL_SS_TOSAFEDELETE				0x10000000		//安全删除标示

#define ADDRESS_MAX_LENGTH		64
//////////////////////////////////////////////////////////////////////////////
#pragma warning (push)
#pragma warning (disable: 4251)
#pragma warning (disable: 4275)

class CBasicSessionNetNotify;
class CBasicNet_Socket;
class _BASIC_DLL_API CBasicSessionNet : public basiclib::EnableRefPtr<CBasicSessionNet>
{
public:
	typedef basiclib::CBasicRefPtr<CBasicSessionNet>														CRefBasicSessionNet;		//智能指针
	typedef fastdelegate::FastDelegate4<CBasicSessionNetNotify*, uint32_t, int32_t, const char *, int32_t>	HandleReceive;
	typedef fastdelegate::FastDelegate2<CBasicSessionNetNotify*, uint32_t, int32_t>							HandleConnect;				//处理连接
	typedef fastdelegate::FastDelegate2<CBasicSessionNetNotify*, uint32_t, int32_t>							HandleDisConnect;			//断开的消息
	typedef fastdelegate::FastDelegate2<CBasicSessionNetNotify*, uint32_t, int32_t>							HandleIdle;					//空闲消息
	typedef fastdelegate::FastDelegate3<CBasicSessionNetNotify*, uint32_t, int32_t, int32_t>				HandleError;				//错误消息

	void bind_rece(const HandleReceive& func) { m_funcReceive = func; }
	void bind_connect(const HandleConnect& func) { m_funcConnect = func; }
	void bind_disconnect(const HandleDisConnect& func) { m_funcDisconnect = func; }
	void bind_idle(const HandleIdle& func) { m_funcIdle = func; }
	void bind_error(const HandleError& func) { m_funcError = func; }
	const HandleReceive GetBindRece(){ return m_funcReceive; }
	const HandleConnect GetBindConnect(){ return m_funcConnect; }
	const HandleDisConnect GetBindDisconnect(){ return m_funcDisconnect; }
	const HandleIdle GetBindIdle(){ return m_funcIdle; }
	const HandleError GetBindError(){ return m_funcError; }

	//注册过滤器
	int RegistePreSend(CBasicPreSend* pFilter, uint32_t dwRegOptions = 0);

	//主动关闭
	void Close(bool bNoWaitMustClose = false);

	//提供安全删除的回调接口
	virtual void SafeDelete();

	//! 获取注册的过滤器
	CBasicPreSend* GetPreSend();

	//! 绑定socket
	void InitSocket(CBasicNet_Socket* pSocket);
protected:
	CBasicSessionNet();
	virtual ~CBasicSessionNet();

	//! 智能指针删除的方式
	virtual void DeleteRetPtrObject();

	//! 获取socket
	CBasicNet_Socket* GetBasicNet_Socket(){ return m_pSocket; }
protected:
	CRefBasicSessionNet				m_self;
	CBasicNet_Socket*				m_pSocket;
	HandleReceive					m_funcReceive;
	HandleConnect					m_funcConnect;
	HandleDisConnect				m_funcDisconnect;
	HandleIdle						m_funcIdle;
	HandleError						m_funcError;
};
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class _BASIC_DLL_API CBasicSessionNetNotify : public basiclib::CBasicSessionNet
{
public:
	//! 是否连接
	bool IsConnected();

	//! 是否认证成功
	bool IsTransmit();

	//! 发送
	virtual int32_t Send(void *pData, int32_t cbData, uint32_t dwFlag = 0);
	virtual int32_t Send(basiclib::CBasicSmartBuffer& smBuf, uint32_t dwFlag = 0);

	//! 获取网络状态
	void GetNetStatInfo(BasicNetStat& netState);

	//! 获取网络状态
	virtual void GetNetStatus(CBasicString& strStatus);
protected:
	CBasicSessionNetNotify();
	virtual ~CBasicSessionNetNotify();
protected:
	//! 回调connect
	virtual uint32_t OnConnect(uint32_t dwNetCode) {
		if (m_funcConnect)
			return m_funcConnect(this, dwNetCode);
		return BASIC_NET_OK;
	}
	//! 回调receive
	virtual int32_t OnReceive(uint32_t dwNetCode, const char *pszData, int32_t cbData) {
		if (m_funcReceive)
			return m_funcReceive(this, dwNetCode, cbData, pszData);
		return BASIC_NET_OK;
	}
	//! 回调disconnect
	virtual uint32_t OnDisconnect(uint32_t dwNetCode) {
		if (m_funcDisconnect)
			return m_funcDisconnect(this, dwNetCode);
		return BASIC_NET_OK;
	}
	//! 回调错误
	virtual uint32_t OnError(uint32_t dwNetCode, int32_t lRetCode) {
		if (m_funcError)
			return m_funcError(this, dwNetCode, lRetCode);
		return BASIC_NET_OK;
	}
	virtual int32_t OnIdle(uint32_t dwIdleCount) {
		if (m_funcIdle)
			return m_funcIdle(this, dwIdleCount);
		return BASIC_NET_OK;
	}
protected:
	//定时器,ontimer线程
	virtual void OnTimer(unsigned int nTick){}
protected:

	friend class CBasicNet_SocketTransfer;
};

#pragma warning (pop)
__NS_BASIC_END
//////////////////////////////////////////////////////////////////////////////////////////////////

#endif 
