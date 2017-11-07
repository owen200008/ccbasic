#include "basic.h"
#include "basicversion.h"

static basiclib::CBasicString* g_strBasiclibVersion = nullptr;
//获取版本号
basiclib::CBasicString& GetBasicLibVersion() {
	if (g_strBasiclibVersion)
		return *g_strBasiclibVersion;
	g_strBasiclibVersion = new basiclib::CBasicString();
	g_strBasiclibVersion->Format("%d.%d.%d", VERSION_MAJOR, VERSION_MINOR, VERSION_BUGOR);
	return *g_strBasiclibVersion;
}

#define CHECKSUPPORTATOMIC(a) \
	{\
		std::atomic<a> tmp;\
		if (!tmp.is_lock_free())\
			return false;\
	}
//判断basic库是否可用
bool IsSupportBasiclib()
{
	CHECKSUPPORTATOMIC(bool);
	CHECKSUPPORTATOMIC(char);
	CHECKSUPPORTATOMIC(signed char);
	CHECKSUPPORTATOMIC(unsigned char);
	CHECKSUPPORTATOMIC(short);
	CHECKSUPPORTATOMIC(unsigned short);
	CHECKSUPPORTATOMIC(int);
	CHECKSUPPORTATOMIC(unsigned int);
	CHECKSUPPORTATOMIC(unsigned int);
	CHECKSUPPORTATOMIC(long long);
	CHECKSUPPORTATOMIC(unsigned long long);
	CHECKSUPPORTATOMIC(void*);
	return true;
}
