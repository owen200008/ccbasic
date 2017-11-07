#ifndef _INC_SCBASICJSON_H
#define _INC_SCBASICJSON_H

/////////////////////////////////////////////////////////////////////////////////////////////////
#include "../../inc/basic.h"
#include "../../../3rd/rapidjson/include/rapidjson/document.h"
class CSCBasicAllocateForJson {
public:
	static const bool kNeedFree = true;
	void* Malloc(size_t size) {
		if (size) //  behavior of malloc(0) is implementation defined.
			return basiclib::BasicAllocate(size);
		else
			return NULL; // standardize to returning NULL.
	}
	void* Realloc(void* originalPtr, size_t originalSize, size_t newSize) {
		(void)originalSize;
		if (newSize == 0) {
			basiclib::BasicDeallocate(originalPtr);
			return NULL;
		}
		return basiclib::BasicReallocate(originalPtr, newSize);
	}
	static void Free(void *ptr) { basiclib::BasicDeallocate(ptr); }
};

typedef rapidjson::GenericDocument<rapidjson::UTF8<>, rapidjson::MemoryPoolAllocator<>, CSCBasicAllocateForJson> SCBasicDocument;

#endif //_INC_HTTPSESSION_H



