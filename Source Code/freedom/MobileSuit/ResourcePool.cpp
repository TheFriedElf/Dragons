#include "StdAfx.h"
#include "ResourcePool.h"

//template<typename T_Key, typename T_Res>
//bool CResourcePool<T_Key, T_Res>::Add(T_Key& rkKey, T_Res& rkRes)
//{
//	std::pair<Container::iterator, bool> ret;
//
//	ret = m_kContainer.insert(std::make_pair(rkKey, rkRes));
//
//	return ret->second;
//}
//
//template<typename T_Key, typename T_Res>
//void CResourcePool<T_Key, T_Res>::Delete(T_Key& rkKey)
//{
//	m_kContainer.erase(rkKey);
//}
//
//template<typename T_Key, typename T_Res>
//T_Res& CResourcePool<T_Key, T_Res>::Get(T_Key& rkKey)
//{
//	Container itr = m_kContainer.find(rkKey);
//
//	if(itr != m_kContainer.end())
//	{
//		return itr->second;
//	}
//
//	assert(!"resouce not found");
//	return T_Res();
//}