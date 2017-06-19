#include "rsaencode.h"

#include "hex.h"
#include "files.h"

CSCBasicRSA::CSCBasicRSA(const char* pSeed)
{
	m_rng.IncorporateEntropy((byte*)pSeed, strlen(pSeed));
}

CSCBasicRSA::~CSCBasicRSA()
{

}

void CSCBasicRSA::GenerateRSAKey(unsigned int keyLength, const char *privFilename, const char *pubFilename)
{
	CryptoPP::RSAES_OAEP_SHA_Decryptor priv(m_rng, keyLength);
	CryptoPP::HexEncoder privFile(new CryptoPP::FileSink(privFilename));
	priv.DEREncode(privFile);
	privFile.MessageEnd();

	CryptoPP::RSAES_OAEP_SHA_Encryptor pub(priv);
	CryptoPP::HexEncoder pubFile(new CryptoPP::FileSink(pubFilename));
	pub.DEREncode(pubFile);
	pubFile.MessageEnd();
}
void CSCBasicRSA::GenerateRSAKey(unsigned int keyLength, string& strPub, string& strPri)
{
	CryptoPP::RSAES_OAEP_SHA_Decryptor priv(m_rng, keyLength);
	CryptoPP::HexEncoder privFile(new CryptoPP::StringSink(strPri));
	priv.DEREncode(privFile);
	privFile.MessageEnd();

	CryptoPP::RSAES_OAEP_SHA_Encryptor pub(priv);
	CryptoPP::HexEncoder pubFile(new CryptoPP::StringSink(strPub));
	pub.DEREncode(pubFile);
	pubFile.MessageEnd();
}

//设置publicfile
void CSCBasicRSA::SetPublicFileName(const char* pPubFileName)
{
	//android编译出错无法转换FileSource to BufferedTransformation
	{
		CryptoPP::FileSource source(pPubFileName, true, new CryptoPP::HexDecoder);
		CryptoPP::Filter& filter = source;
		m_pubEncode.AccessKey().Load(filter);
	}
	{
		CryptoPP::FileSource source(pPubFileName, true, new CryptoPP::HexDecoder);
		CryptoPP::Filter& filter = source;
		m_pubDecode.AccessKey().Load(filter);
	}
	
}
void CSCBasicRSA::SetPublicKey(const char* pData)
{
	//android编译出错无法转换FileSource to BufferedTransformation
	{
		CryptoPP::StringSource source(pData, true, new CryptoPP::HexDecoder);
		CryptoPP::Filter& filter = source;
		m_pubEncode.AccessKey().Load(filter);
	}
	{
		CryptoPP::StringSource source(pData, true, new CryptoPP::HexDecoder);
		CryptoPP::Filter& filter = source;
		m_pubDecode.AccessKey().Load(filter);
	}
}
void CSCBasicRSA::SetPrivateFileName(const char* pPrivateFileName)
{
	{
		CryptoPP::FileSource source(pPrivateFileName, true, new CryptoPP::HexDecoder);
		CryptoPP::Filter& filter = source;
		m_priDecode.AccessKey().Load(filter);
	}
	{
		CryptoPP::FileSource source(pPrivateFileName, true, new CryptoPP::HexDecoder);
		CryptoPP::Filter& filter = source;
		m_priEncode.AccessKey().Load(filter);
	}
}
void CSCBasicRSA::SetPrivateKey(const char* pData)
{
	{
		CryptoPP::StringSource source(pData, true, new CryptoPP::HexDecoder);
		CryptoPP::Filter& filter = source;
		m_priDecode.AccessKey().Load(filter);
	}
	{
		CryptoPP::StringSource source(pData, true, new CryptoPP::HexDecoder);
		CryptoPP::Filter& filter = source;
		m_priEncode.AccessKey().Load(filter);
	}
}

//! 计算加密所需的数据长度
size_t CSCBasicRSA::CalcEncryptNeedLength(int nLength)
{
	size_t fixedLen = m_pubEncode.FixedMaxPlaintextLength();
	return (nLength / fixedLen + 1) * m_pubEncode.FixedCiphertextLength();
}

size_t CSCBasicRSA::Encrypt(const char* pEncode, int nLength, byte* pOutput, int nOutputLength)
{
	size_t putLen = 0;
	size_t fixedLen = m_pubEncode.FixedMaxPlaintextLength();
	for (int i = 0; i < nLength; i += fixedLen)
	{
		size_t len = fixedLen < (nLength - i) ? fixedLen : (nLength - i);
		CryptoPP::ArraySink *dstArr = new CryptoPP::ArraySink(pOutput + putLen, nOutputLength - putLen);
		CryptoPP::ArraySource source((byte*)(pEncode + i), len, true, new CryptoPP::PK_EncryptorFilter(m_rng, m_pubEncode, dstArr));
		putLen += (size_t)dstArr->TotalPutLength();
	}
	return putLen;
}

size_t CSCBasicRSA::Decrypt(const char* pDecode, int nLength, byte* pOutput, int nOutputLength)
{
	size_t putLen = 0;
	size_t fixedLen = m_priDecode.FixedCiphertextLength();
	for (int i = 0; i < nLength; i += fixedLen)
	{
		size_t len = fixedLen < (nLength - i) ? fixedLen : (nLength - i);
		CryptoPP::ArraySink *dstArr = new CryptoPP::ArraySink(pOutput + putLen, nOutputLength - putLen);
        CryptoPP::ArraySource source((byte*)(pDecode + i), len, true, new CryptoPP::PK_DecryptorFilter(m_rng, m_priDecode, dstArr));
		putLen += (size_t)dstArr->TotalPutLength();
	}
	return putLen;
}
size_t CSCBasicRSA::Sign(const char* pEncode, int nLength, byte* pOutput, int nOutputLength)
{
	size_t putLen = 0;
	int fixedLen = (int)m_priEncode.MaxSignatureLength();
	if (nLength > fixedLen)
		return 0;
	size_t len = nLength;
	CryptoPP::ArraySink *dstArr = new CryptoPP::ArraySink(pOutput + putLen, nOutputLength - putLen);
	CryptoPP::ArraySource arraySource((byte*)pEncode, len, true, new CryptoPP::SignerFilter(m_rng, m_priEncode, dstArr));
	putLen += (size_t)dstArr->TotalPutLength();
	return putLen;
}

bool CSCBasicRSA::Verify(const char* pDecode, int nLength, const char* pVerify, int nVerifyLength)
{
	bool bSuccess = false;
	size_t fixedLen = m_pubDecode.SignatureLength();
	if (nLength != fixedLen)
		return false;
	CryptoPP::SignatureVerificationFilter *pVerifierFilter = new CryptoPP::SignatureVerificationFilter(m_pubDecode);
	pVerifierFilter->Put((byte*)pDecode, nLength);
    CryptoPP::ArraySource source((byte*)pVerify, nVerifyLength, true, pVerifierFilter);
	return pVerifierFilter->GetLastResult();
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CXKRSAManager::CXKRSAManager()
{
	m_pDefaultRSA = nullptr;
	m_usDefaultVersion = 0;
}
CXKRSAManager::~CXKRSAManager()
{
	for (auto& rsa : m_mapRSA) {
		delete rsa.second;
	}
}
bool CXKRSAManager::InitFromPath(const char* pPath)
{
	basiclib::CBasicString strConfig = pPath;
	strConfig += "rsa.ini";
	basiclib::CBasicIniOp iniOp(strConfig.c_str());
	iniOp.GetSection("main", [&](basiclib::CBasicString& strName, basiclib::CBasicString& strValue)->void {
		Net_UShort nKey = (Net_UShort)atol(strName.c_str());
		MapVersionToRSAIterator iter = m_mapRSA.find(nKey);
		CSCBasicRSA* pRSA = nullptr;
		if (iter == m_mapRSA.end()) {
			pRSA = new CSCBasicRSA();
			m_mapRSA[nKey] = pRSA;
		}
		else {
			pRSA = iter->second;
		}
		basiclib::CBasicStringArray ayItem;
		basiclib::BasicSpliteString(strValue.c_str(), ",", basiclib::IntoContainer_s<basiclib::CBasicStringArray>(ayItem));
		if (ayItem.GetSize() >= 1) {
			basiclib::CBasicString strPrivateFile = pPath;
			strPrivateFile += ayItem[0];
			pRSA->SetPrivateFileName(strPrivateFile.c_str());
		}
		if (ayItem.GetSize() >= 2) {
			basiclib::CBasicString strPublicFile = pPath;
			strPublicFile += ayItem[1];
			pRSA->SetPublicFileName(strPublicFile.c_str());
		}
	});
	m_usDefaultVersion = (Net_UShort)iniOp.GetLong("default", "version", "1");
	MapVersionToRSAIterator iter = m_mapRSA.find(m_usDefaultVersion);
	if (iter != m_mapRSA.end()) {
		m_pDefaultRSA = iter->second;
		return true;
	}
	return false;
}
bool CXKRSAManager::InitFromPath(const char* pPath, const std::function<void(const char* pFileName, basiclib::CBasicSmartBuffer& smBuf)>& func)
{
	basiclib::CBasicSmartBuffer smBuf;
	basiclib::CBasicString strConfig = pPath;
	strConfig += "rsa.ini";
	smBuf.SetDataLength(0);
	func(strConfig.c_str(), smBuf);
	basiclib::CBasicIniOp iniOp;
	iniOp.InitFromMem(smBuf.GetDataBuffer(), smBuf.GetDataLength());
	iniOp.GetSection("main", [&](basiclib::CBasicString& strName, basiclib::CBasicString& strValue)->void {
		Net_UShort nKey = (Net_UShort)atol(strName.c_str());
		MapVersionToRSAIterator iter = m_mapRSA.find(nKey);
		CSCBasicRSA* pRSA = nullptr;
		if (iter == m_mapRSA.end()) {
			pRSA = new CSCBasicRSA();
			m_mapRSA[nKey] = pRSA;
		}
		else {
			pRSA = iter->second;
		}
		basiclib::CBasicStringArray ayItem;
		basiclib::BasicSpliteString(strValue.c_str(), ",", basiclib::IntoContainer_s<basiclib::CBasicStringArray>(ayItem));
		if (ayItem.GetSize() >= 1) {
			basiclib::CBasicString strPrivateFile = pPath;
			strPrivateFile += ayItem[0];
			smBuf.SetDataLength(0);
			func(strPrivateFile.c_str(), smBuf);
			pRSA->SetPrivateKey(smBuf.GetDataBuffer());
		}
		if (ayItem.GetSize() >= 2) {
			basiclib::CBasicString strPublicFile = pPath;
			strPublicFile += ayItem[1];
			smBuf.SetDataLength(0);
			func(strPublicFile.c_str(), smBuf);
			pRSA->SetPublicKey(smBuf.GetDataBuffer());
		}
	});
	m_usDefaultVersion = (Net_UShort)iniOp.GetLong("default", "version", "1");
	MapVersionToRSAIterator iter = m_mapRSA.find(m_usDefaultVersion);
	if (iter != m_mapRSA.end()) {
		m_pDefaultRSA = iter->second;
		return true;
	}
	return false;
}
bool CXKRSAManager::InitFromData(Net_UShort usDefaultVersion, VTRSAInitData& vtData) {
	m_usDefaultVersion = usDefaultVersion;
	for (auto& data : vtData) {
		Net_UShort nKey = data.m_usVersion;
		if (data.m_smBufPri.IsEmpty())
			continue;
		MapVersionToRSAIterator iter = m_mapRSA.find(nKey);
		CSCBasicRSA* pRSA = nullptr;
		if (iter == m_mapRSA.end()) {
			pRSA = new CSCBasicRSA();
			m_mapRSA[nKey] = pRSA;
		}
		else {
			pRSA = iter->second;
		}
		pRSA->SetPrivateKey(data.m_smBufPri.GetDataBuffer());
		if (!data.m_smBufPub.IsEmpty()) {
			pRSA->SetPublicKey(data.m_smBufPub.GetDataBuffer());
		}
	}
	MapVersionToRSAIterator iter = m_mapRSA.find(m_usDefaultVersion);
	if (iter != m_mapRSA.end()) {
		m_pDefaultRSA = iter->second;
		return true;
	}
	return false;
}

CSCBasicRSA* CXKRSAManager::GetRSAByVersion(Net_UShort sVersion)
{
	MapVersionToRSAIterator iter = m_mapRSA.find(sVersion);
	if (iter != m_mapRSA.end()) {
		return iter->second;
	}
	return nullptr;
}

