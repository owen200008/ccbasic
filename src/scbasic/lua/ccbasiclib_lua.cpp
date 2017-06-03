#include "ccbasiclib_lua.h"

void ExportBasiclibClassToLua(lua_State* L) {
	sel::State luaState(L);
	int(basiclib::CBasicString::*pReplaceFunc)(const char*, const char*) = &basiclib::CBasicString::Replace;
	BOOL(basiclib::CBasicString::*pLoadStringFunc)(const char*, const char*) = &basiclib::CBasicString::LoadString;
	void(basiclib::CBasicString::*pTrimLeftFunc)(const char*) = &basiclib::CBasicString::TrimLeft;
	void(basiclib::CBasicString::*pTrimRightFunc)(const char*) = &basiclib::CBasicString::TrimRight;
	int(basiclib::CBasicString::*pFindFunc)(const char*, int) const = &basiclib::CBasicString::Find;
	int(basiclib::CBasicString::*pReverseFindFunc)(char) const = &basiclib::CBasicString::ReverseFind;
	luaState["CBasicString"].SetClass<basiclib::CBasicString>(
		"GetLength", &basiclib::CBasicString::GetLength,
		"IsEmpty", &basiclib::CBasicString::IsEmpty,
		"Empty", &basiclib::CBasicString::Empty,
		"GetAt", &basiclib::CBasicString::GetAt,
		"SetAt", &basiclib::CBasicString::SetAt,
		"Compare", &basiclib::CBasicString::Compare,
		"CompareNoCase", &basiclib::CBasicString::CompareNoCase,
		"MakeUpper", &basiclib::CBasicString::MakeUpper,
		"MakeLower", &basiclib::CBasicString::MakeLower,
		"MakeReverse", &basiclib::CBasicString::MakeReverse,
		"FindOneOf", &basiclib::CBasicString::FindOneOf,
		"GetBuffer", &basiclib::CBasicString::GetBuffer,
		"ReleaseBuffer", &basiclib::CBasicString::ReleaseBuffer,
		"LoadString", pLoadStringFunc,
		"Replace", pReplaceFunc,
		"TrimLeft", pTrimLeftFunc,
		"TrimRight", pTrimRightFunc,
		"Find", pFindFunc,
		"ReverseFind", pReverseFindFunc/*,
		"SetString", [](basiclib::CBasicString* pStr, std::string& str) {{
				pStr->assign(str.c_str(), str.length());
			}},
		"GetTotalString", [](basiclib::CBasicString* pStr) { {
				std::string strRet;
				strRet.assign(pStr->c_str(), pStr->length());
				return strRet;
			}},
		"Left", [](basiclib::CBasicString* pStr, int nCount) { {
				basiclib::CBasicString strRet = pStr->Left(nCount);
				return std::string(strRet.c_str(), strRet.length());
			}},
		"Right", [](basiclib::CBasicString* pStr, int nCount) { {
				basiclib::CBasicString strRet = pStr->Right(nCount);
				return std::string(strRet.c_str(), strRet.length());
			}},
		"Mid", [](basiclib::CBasicString* pStr, int nFirst, int nCount) { {
				basiclib::CBasicString strRet = pStr->Mid(nFirst, nCount);
				return std::string(strRet.c_str(), strRet.length());
			}}*/
		);
	luaState["CBasicBitstream"].SetClass<basiclib::CBasicBitstream>(
		"ResetReadError", &basiclib::CBasicBitstream::ResetReadError,
		"IsReadError", &basiclib::CBasicBitstream::IsReadError,
		"IsEmpty", &basiclib::CBasicBitstream::IsEmpty,
		"Free", &basiclib::CBasicBitstream::Free,
		"GetDataLength", &basiclib::CBasicBitstream::GetDataLength,
		"SetDataLength", &basiclib::CBasicBitstream::SetDataLength,
		"AppendString", &basiclib::CBasicBitstream::AppendString,
		"GetAllocBufferLength", &basiclib::CBasicBitstream::GetAllocBufferLength,
		"InitFormFile", &basiclib::CBasicBitstream::InitFormFile/*,
		"GetDataBuffer", [](basiclib::CBasicBitstream* pSM) { {
				std::string strRet;
				if (pSM->GetDataBuffer()) {
					strRet.assign(pSM->GetDataBuffer(), pSM->GetDataLength());
				}
				return strRet;
			}},
		"AppendData", [](basiclib::CBasicBitstream* pSM, std::string& str) { {
				pSM->AppendData(str.c_str(), str.length());
			}},
		"AppendDataEx", [](basiclib::CBasicBitstream* pSM, std::string& str) { {
				pSM->AppendDataEx(str.c_str(), str.length());
			}},
		"AppendSMBuffer", [](basiclib::CBasicBitstream* pSM, basiclib::CBasicBitstream* pSM2)->void { {
				pSM->AppendData(pSM2->GetDataBuffer(), pSM2->GetDataLength());
			}},
		"Compare", [](basiclib::CBasicBitstream* pSM, basiclib::CBasicBitstream* pSM2)->bool { {
				if (pSM->GetDataLength() != pSM2->GetDataLength())
					return false;
				return memcmp(pSM->GetDataBuffer(), pSM2->GetDataBuffer(), pSM->GetDataLength()) == 0;
			}}*/
		);
	/*luaState["CNetBasicValue"].setClass(kaguya::UserdataMetatable<basiclib::CNetBasicValue>()
		.setConstructors<basiclib::CNetBasicValue(), basiclib::CNetBasicValue()>()
		.addFunction("SetLong", &basiclib::CNetBasicValue::SetLong)
		.addFunction("SetDouble", &basiclib::CNetBasicValue::SetDouble)
		.addFunction("SetLongLong", &basiclib::CNetBasicValue::SetLongLong)
		.addFunction("GetLong", &basiclib::CNetBasicValue::GetLong)
		.addFunction("GetDouble", &basiclib::CNetBasicValue::GetDouble)
		.addFunction("GetLongLong", &basiclib::CNetBasicValue::GetLongLong)
		.addFunction("GetStringRef", &basiclib::CNetBasicValue::GetStringRef)
		.addFunction("GetDataType", &basiclib::CNetBasicValue::GetDataType)
		.addFunction("GetDataLength", &basiclib::CNetBasicValue::GetDataLength)
		.addFunction("IsNull", &basiclib::CNetBasicValue::IsNull)
		.addFunction("SetNull", &basiclib::CNetBasicValue::SetNull)
		.addFunction("IsString", &basiclib::CNetBasicValue::IsString)
		.addFunction("CompareBasicValue", &basiclib::CNetBasicValue::CompareBasicValue)
		.addFunction("CompareInt", &basiclib::CNetBasicValue::CompareInt)
		.addFunction("CompareDouble", &basiclib::CNetBasicValue::CompareDouble)
		.addFunction("CompareLongLong", &basiclib::CNetBasicValue::CompareLongLong)
		.addFunction("GetSeriazeLength", &basiclib::CNetBasicValue::GetSeriazeLength)
		.addStaticFunction("ComparePointString", [](basiclib::CNetBasicValue* pData, std::string& str) { {
				return pData->ComparePointString(str.c_str(), str.length());
			}})
		.addStaticFunction("SeriazeSMBuf", [](basiclib::CNetBasicValue* pData, basiclib::CBasicSmartBuffer* pBuf) { {
				pData->SeriazeSMBuf(*pBuf);
			}})
		.addStaticFunction("UnSeriazeSMBuf", [](basiclib::CNetBasicValue* pData, basiclib::CBasicSmartBuffer* pBuf) { {
				pData->UnSeriazeSMBuf(*pBuf);
			}})
		.addStaticFunction("SetString", [](basiclib::CNetBasicValue* pData, std::string& str) { {
				pData->SetString(str.c_str(), str.length());
			}})
	);
	luaState["CCQLite3DB"].setClass(kaguya::UserdataMetatable<CCQLite3DB>()
		.setConstructors<CCQLite3DB(), CCQLite3DB()>()
		.addFunction("Open", &CCQLite3DB::Open)
		.addFunction("Close", &CCQLite3DB::Close)
		.addFunction("SetOpenPWD", &CCQLite3DB::SetOpenPWD)
		.addFunction("SetPWD", &CCQLite3DB::SetPWD)
		.addFunction("GetDataToTable", &CCQLite3DB::GetDataToTable)
		.addFunction("ExecSQL", &CCQLite3DB::ExecSQL)
		.addFunction("GetLastError", &CCQLite3DB::GetLastError)	
	);
	bool(CCQLite3DBTable::*pFieldIsNullFunc)(const char*) = &CCQLite3DBTable::FieldIsNull;
	const char*(CCQLite3DBTable::*pValueOfFieldFunc)(const char*) = &CCQLite3DBTable::ValueOfField;
	luaState["CCQLite3DBTable"].setClass(kaguya::UserdataMetatable<CCQLite3DBTable>()
		.setConstructors<CCQLite3DBTable(), CCQLite3DBTable()>()
		.addFunction("NumOfFields", &CCQLite3DBTable::NumOfFields)
		.addFunction("NumOfRows", &CCQLite3DBTable::NumOfRows)
		.addFunction("NameOfField", &CCQLite3DBTable::NameOfField)
		.addFunction("ValueOfField", pValueOfFieldFunc)
		.addFunction("FieldIsNull", pFieldIsNullFunc)
		.addFunction("GetUIntField", &CCQLite3DBTable::GetUIntField)
		.addFunction("GetIntField", &CCQLite3DBTable::GetIntField)
		.addFunction("GetDoubleField", &CCQLite3DBTable::GetDoubleField)
		.addFunction("GetStringField", &CCQLite3DBTable::GetStringField)
	);*/
}