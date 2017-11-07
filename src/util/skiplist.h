/***********************************************************************************************
// 文件名:     skiplist.h
// 创建者:     蔡振球
// Email:      zqcai@w.cn
// 创建时间:   2017/8/23 
// 内容描述:   实现skiplist
// 版本信息:   1.0V
************************************************************************************************/
#ifndef BASIC_SKIPLIST_H
#define BASIC_SKIPLIST_H

#include "../inc/basic.h"

#define ZSKIPLIST_P 0.25      /* Skiplist P = 1/4 */
template <class T, size_t defaultLevelCount = 32>
class CBasicSkipListTemplate : public basiclib::CBasicObject{
public:
	struct BasicLinkSkipNode{
		BasicLinkSkipNode* m_pPrev;
		BasicLinkSkipNode* m_pNext;
		BasicLinkSkipNode(){
			memset(this, 0, sizeof(this));
		}
		void Reset(){
			memset(this, 0, sizeof(this));
		}
		void Add(BasicLinkSkipNode* pPrev, BasicLinkSkipNode* pNext){
			m_pNext = pNext;
			m_pPrev = pPrev;
			pNext->m_pPrev = this;
			pPrev->m_pNext = this;
		}
		void Add(BasicLinkSkipNode* pPrev){
			Add(pPrev, pPrev->m_pNext);
		}
		static void Del(BasicLinkSkipNode* pPrev, BasicLinkSkipNode* pNext){
			pPrev->m_pNext = pNext;
			pNext->m_pPrev = pPrev;
		}
		void Del(){
			Del(m_pPrev, m_pNext);
			Reset();
		}
	};
	struct BasicSkipListNode{
		T						m_data;
		BasicSkipListNode*		m_pPrev;
		BasicSkipListNode*		m_pLinks[1];
		void ResetData(){
			m_pPrev = nullptr;
			m_pLinks[0] = nullptr;
			m_data.Empty();
		}
		static BasicSkipListNode* CeateLevelNode(int nLevel){
			return (BasicSkipListNode*)basiclib::BasicAllocate(sizeof(BasicSkipListNode) + (nLevel - 1) * sizeof(BasicSkipListNode*));
		}
		static void DestroyLevelNode(BasicSkipListNode* pNode){
			basiclib::BasicDeallocate(pNode);
		}
	};
public:
	CBasicSkipListTemplate(int nDefaultAllocate = 32, int nMaxAllocateOnce = 64){
		m_pTail = nullptr;
		m_pHeader = BasicSkipListNode::CeateLevelNode(defaultLevelCount);
		m_pHeader->m_pPrev = nullptr;
		for(int i = 0; i < defaultLevelCount; i++){
			m_pHeader->m_pLinks[i] = nullptr;
		}
		if(nDefaultAllocate < 0){
			nDefaultAllocate = 32;
		}
		m_nMaxAllocateCountOnce = nMaxAllocateOnce;
		if(m_nMaxAllocateCountOnce < 0)
			m_nMaxAllocateCountOnce = 1;
		for(int i = defaultLevelCount - 1; i >= 0; i--){

		}

	}
	virtual ~CBasicSkipListTemplate(){
		if(m_pHeader){
			BasicSkipListNode::DestroyLevelNode(m_pHeader);
		}
	}
	//! 插入
	BasicSkipListNode* Insert(T& data){

	}

protected:
	int GetRandLevel(){
		int level = 1;
		while((random() & 0xffff) < 0xffff * ZSKIPLIST_P){
			level++;
		}
		return level > defaultLevelCount ? defaultLevelCount : level;
	}
	//! 提供比较函数
	virtual bool CompareTFunction(T& t1, T& t2) = 0;
protected:
	int						m_nMaxAllocateCountOnce;
	BasicSkipListNode*		m_pHeader;
	BasicSkipListNode*		m_pTail;
};

#endif