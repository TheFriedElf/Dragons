#include "StdAfx.h"
#include "PgItemEx.h"
#include "PgItemMan.h"
#include "PgMobileSuit.h"

PgItemMan::PgItemMan()
{
}

PgItemMan::~PgItemMan()
{
}

void PgItemMan::ClearAllCache()
{
	BM::CAutoMutex kLock(m_kItemCacheLock);

	for(ItemCacheContainer::iterator itr = m_kItemCacheContainer.begin();
		itr != m_kItemCacheContainer.end();
		++itr)
	{
		SAFE_DELETE_NI(itr->pItem);
	}

	// 컨테이너를 깡통으로 만든다.
	m_kItemCacheContainer.clear();
}

PgItemEx *PgItemMan::GetItem(int const iItemDefNo, int const iUserGender, int const iClassNo)
{

	PgItemEx* pItem = NULL;
	
	if (iItemDefNo == 0)
	{
		return NULL;
	}

	pItem = FindItemInCache(iItemDefNo, iUserGender, iClassNo);
	if (pItem != NULL)
	{
		NILOG(PGLOG_LOG, "[PgItemMan] GetItem no(%d), gend(%d), class(%d), path(%s) - inside Cache 0x%0X\n", pItem->GetItemDef()->No(), pItem->GetUserGender(), pItem->GetUserClass(), pItem->GetXMLPath().c_str(), pItem);
		return pItem;
	}

	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const *pkItemDef = kItemDefMgr.GetDef(iItemDefNo);
	if (!pkItemDef)
	{
		PG_ASSERT_LOG(!"Can't Find Item from ItemDefMgr");
		return NULL;
	}

	const CONT_DEFRES* pContDefRes = NULL;
	g_kTblDataMgr.GetContDef(pContDefRes);

	CONT_DEFRES::const_iterator itrDest = pContDefRes->find(pkItemDef->ResNo());
	if(itrDest == pContDefRes->end())
	{
		// 아이템 테이블에서 못 찾으면 패스!
		PG_ASSERT_LOG(!"Can't Find ContDefRes");
		return NULL;
	}

	// 아이템을 생성한다.
	std::string xmlPath = MB(itrDest->second.strXmlPath);

	PG_WARNING_LOG(xmlPath.length() == 0);

	if(xmlPath.length() <= 1)	//	경로 길이가 너무 작으면 비정상적인 경로로 가정하고 리턴한다.
	{
		return	NULL;
	}

	pItem = NiNew PgItemEx();
	pItem->SetUserGender(iUserGender);
	pItem->SetUserClass(iClassNo);
	pItem->SetXMLPath(xmlPath.c_str());

	pItem = static_cast<PgItemEx *>(PgXmlLoader::CreateObjectFromFile(xmlPath.c_str(), (void *)pkItemDef, pItem));
	PG_ASSERT_LOG(pItem != NULL);
	if (pItem == NULL)
	{
		_PgMessageBox("","[PgItemEx] GetItemFromDef, CreateObjectFromFile Failed(%d,%d,%s)\n", iItemDefNo, iUserGender, itrDest->second.strXmlPath.c_str());
		NILOG(PGLOG_ERROR, "[PgItemEx] GetItemFromDef, CreateObjectFromFile Failed(%d,%d,%s)\n", iItemDefNo, iUserGender, itrDest->second.strXmlPath.c_str());
	}
	else
	{
		NILOG(PGLOG_LOG, "[PgItemMan] GetItem no(%d), gend(%d), class(%d), path(%s) - create new 0x%0X\n", pItem->GetItemDef()->No(), pItem->GetUserGender(), pItem->GetUserClass(), pItem->GetXMLPath().c_str(), pItem);
	}
	return pItem;
}

PgItemEx *PgItemMan::GetItem(CItemDef const *pkItemDef, int const iUserGender, int const iClassNo)
{
	if (pkItemDef == NULL)
		return NULL;

	return GetItem(pkItemDef->No(), iUserGender, iClassNo);
}

void PgItemMan::PrepareItem(const int iItemDefNo)
{
	
}

PgItemEx *PgItemMan::FindItemInCache(int const iItemDefNo, int const iUserGender, int const iClassNo)
{
	BM::CAutoMutex kLock(m_kItemCacheLock);

	ItemCacheContainer::iterator itr = m_kItemCacheContainer.begin();

	while (itr != m_kItemCacheContainer.end())
	{
		if ((*itr).iItemDefNo == iItemDefNo && (*itr).iUserGender == iUserGender && (*itr).iClassNo == iClassNo)
		{
			PgItemEx* pItem = (*itr).pItem;
			m_kItemCacheContainer.erase(itr);
			return pItem;
		}
		++itr;
	}

	return NULL;
}

void PgItemMan::CacheItem(PgItemEx *pkItem)
{
	if (pkItem == NULL)
		return;

	g_bUseItemCache = (::GetPrivateProfileInt(_T("Debug"), _T("UseItemCache"), 1, g_pkApp->GetConfigFileName()) == 1);
	if (g_bUseItemCache == false)
	{
		THREAD_DELETE_OBJECT(pkItem);
		return;
	}

	BM::CAutoMutex kLock(m_kItemCacheLock);

	bool bFound = false;
	ItemCacheContainer::iterator itr = m_kItemCacheContainer.begin();

	while (itr != m_kItemCacheContainer.end())
	{
		if ((*itr).pItem == pkItem)
		{
			bFound = true;
			break;
		}
		++itr;
	}

	if (bFound)
	{
		NILOG(PGLOG_WARNING, "[PgItemMan] CacheItem 0x%0X already in Cache\n", pkItem);
		return;
	}

	pkItem->Reset();
	pkItem->Update(0);

	NILOG(PGLOG_LOG, "[PgItemMan] CacheItem 0x%0X, no(%d), gend(%d), class(%d), path(%s)\n", pkItem, pkItem->GetItemDef()->No(), pkItem->GetUserGender(), pkItem->GetUserClass(), pkItem->GetXMLPath().c_str());
	m_kItemCacheContainer.push_back(stItemCacheInfo(pkItem->GetItemDef()->No(), pkItem->GetUserGender(), pkItem->GetUserClass(), pkItem));
}

void PgItemMan::ParseXml_SetItemEffectTable(std::wstring const &strTable1, std::wstring const &strTable2)
{
	m_kSetEffectInfo.clear();
	m_kSetItemEffectInfo.clear();

	//세트 아이템 노드 / 이펙트 ID 정보 테이블
	{
		TiXmlDocument kXmlDoc(MB(strTable1.c_str()));

		if(!PgXmlLoader::LoadFile(kXmlDoc, strTable1))
		{
			INFO_LOG(BM::LOG_LV5, __FL__<<L"Failed parse xml ["<<strTable1<<L"]");
			return ;
		}

		TiXmlElement *pkNode = kXmlDoc.FirstChildElement("SET_EFFECT_TABLE");

		pkNode = pkNode->FirstChildElement();
		while(pkNode)
		{
			char const *szName = pkNode->Value();
			if(0 == strcmp(szName, "SET"))
			{
				int iNo = 0;
				SItemEffectInfo kSetInfo1;
				SItemEffectInfo kSetInfo2;
				SItemEffectInfo kSetInfo3;
				SItemEffectInfo kSetInfo4;
				SItemEffectInfo kSetInfo5;
				SItemEffectInfo kSetInfo6;

				TiXmlAttribute const *pkAttr = pkNode->FirstAttribute();
				while(pkAttr)
				{				
					char const *pcAttrName = pkAttr->Name();
					char const *pcAttrValue = pkAttr->Value();

					if(0 == strcmp(pcAttrName, "NO"))
					{
						iNo = atoi(pcAttrValue);
					}
					else if(0 == strcmp(pcAttrName, "NODE_ID1"))
					{
						kSetInfo1.m_strAttachNode = UNI(pcAttrValue);
					}
					else if(0 == strcmp(pcAttrName, "EFFECT_ID1"))
					{
						kSetInfo1.m_strEffectID = UNI(pcAttrValue);
					}
					else if(0 == strcmp(pcAttrName, "NODE_ID2"))
					{
						kSetInfo2.m_strAttachNode = UNI(pcAttrValue);
					}
					else if(0 == strcmp(pcAttrName, "EFFECT_ID2"))
					{
						kSetInfo2.m_strEffectID = UNI(pcAttrValue);
					}
					else if(0 == strcmp(pcAttrName, "NODE_ID3"))
					{
						kSetInfo3.m_strAttachNode = UNI(pcAttrValue);
					}
					else if(0 == strcmp(pcAttrName, "EFFECT_ID3"))
					{
						kSetInfo3.m_strEffectID = UNI(pcAttrValue);
					}
					else if(0 == strcmp(pcAttrName, "NODE_ID4"))
					{
						kSetInfo4.m_strAttachNode = UNI(pcAttrValue);
					}
					else if(0 == strcmp(pcAttrName, "EFFECT_ID4"))
					{
						kSetInfo4.m_strEffectID = UNI(pcAttrValue);
					}
					else if(0 == strcmp(pcAttrName, "NODE_ID5"))
					{
						kSetInfo5.m_strAttachNode = UNI(pcAttrValue);
					}
					else if(0 == strcmp(pcAttrName, "EFFECT_ID5"))
					{
						kSetInfo5.m_strEffectID = UNI(pcAttrValue);
					}
					else if(0 == strcmp(pcAttrName, "NODE_ID6"))
					{
						kSetInfo6.m_strAttachNode = UNI(pcAttrValue);
					}
					else if(0 == strcmp(pcAttrName, "EFFECT_ID6"))
					{
						kSetInfo6.m_strEffectID = UNI(pcAttrValue);
					}

					pkAttr = pkAttr->Next();
				}

				SSetItemEffectIndexInfo kTable;
				if(!kSetInfo1.m_strAttachNode.empty() && !kSetInfo1.m_strEffectID.empty()) kTable.m_kEffect.push_back(kSetInfo1);
				if(!kSetInfo2.m_strAttachNode.empty() && !kSetInfo2.m_strEffectID.empty()) kTable.m_kEffect.push_back(kSetInfo2);
				if(!kSetInfo3.m_strAttachNode.empty() && !kSetInfo3.m_strEffectID.empty()) kTable.m_kEffect.push_back(kSetInfo3);
				if(!kSetInfo4.m_strAttachNode.empty() && !kSetInfo4.m_strEffectID.empty()) kTable.m_kEffect.push_back(kSetInfo4);
				if(!kSetInfo5.m_strAttachNode.empty() && !kSetInfo5.m_strEffectID.empty()) kTable.m_kEffect.push_back(kSetInfo5);
				if(!kSetInfo6.m_strAttachNode.empty() && !kSetInfo6.m_strEffectID.empty()) kTable.m_kEffect.push_back(kSetInfo6);

				m_kSetEffectInfo.insert(std::make_pair(iNo, kTable));
			}

			pkNode = pkNode->NextSiblingElement();
		}
	}

	// 세트 아이템에 연결된 / Set Effect ID 정보 테이블
	{
		TiXmlDocument kXmlDoc(MB(strTable2.c_str()));

		if(!PgXmlLoader::LoadFile(kXmlDoc, strTable2))
		{
			INFO_LOG(BM::LOG_LV5, __FL__<<L"Failed parse xml ["<<strTable2<<L"]");
			return ;
		}

		TiXmlElement *pkNode = kXmlDoc.FirstChildElement("SET_ITEM_EFFECT_TABLE");

		pkNode = pkNode->FirstChildElement();
		while(pkNode)
		{
			char const *szName = pkNode->Value();
			if(0 == strcmp(szName, "SET"))
			{
				int iNo = 0;
				int iSetNo = 0;
				
				SPOTParticleInfo kSetItemParticleInfo;

				TiXmlAttribute const *pkAttr = pkNode->FirstAttribute();
				while(pkAttr)
				{				
					char const *pcAttrName = pkAttr->Name();
					char const *pcAttrValue = pkAttr->Value();

					if(0 == strcmp(pcAttrName, "NO"))
					{
						iNo = atoi(pcAttrValue);
					}
					else if(0 == strcmp(pcAttrName, "SET_EFFECT_ID"))
					{
						iSetNo = atoi(pcAttrValue);
					}

					
					else if(0 == strcmp(pcAttrName, "SETITEM_PARTICLE_ID1"))
					{
						kSetItemParticleInfo.kEffectID = pcAttrValue;
					}
					else if(0 == strcmp(pcAttrName, "SETITEM_PARTICLE_PROJECTILE_ID1"))
					{
						kSetItemParticleInfo.kProjectileEffectID = pcAttrValue;
					}
					else if(0 == strcmp(pcAttrName, "SETITEM_PARTICLE_NODE_ID1"))
					{
						kSetItemParticleInfo.kAttachNode = pcAttrValue;
					}
					else if(0 == strcmp(pcAttrName, "SETITEM_PARTICLE_POS_NODE_ID1"))
					{
						kSetItemParticleInfo.kAttachNodePos = pcAttrValue;
					}
					else if(0 == strcmp(pcAttrName, "SETITEM_PARTICLE_POT1"))
					{
						kSetItemParticleInfo.iAttachPointOfTime = ::atoi(pcAttrValue);
					}
					else if(0 == strcmp(pcAttrName, "SETITEM_PARTICLE_SCALE1"))
					{
						kSetItemParticleInfo.fScale = static_cast<float>(::atof(pcAttrValue));
					}
					else if(0 == strcmp(pcAttrName, "SETITEM_PARTICLE_EVENT_KEY1"))
					{
						kSetItemParticleInfo.kEventKey = pcAttrValue;
					}
					else if(0 == strcmp(pcAttrName, "SETITEM_PARTICLE_DETACH_WHEN_ACTION_END1"))
					{
						kSetItemParticleInfo.bDetachWhenActionEnd = (0 == strcmp("TRUE", pcAttrValue));
					}
			
					pkAttr = pkAttr->Next();
				}
				m_kSetItemEffectInfo.insert(std::make_pair(iNo, iSetNo));
				if(!kSetItemParticleInfo.kEffectID.empty())
				{// 세트아이템으로 인한 특정 시점 파티클 붙이기
					m_kContSetParticleInfo.insert(std::make_pair(iNo, kSetItemParticleInfo));
				}
				
				CONT_ITEM_CHANGE_INFO kContItemChangeInfoSet;

				TiXmlElement const* pkChildElement = pkNode->FirstChildElement();
				while(pkChildElement)
				{
					char const* pcChildTagName = pkChildElement->Value();

					if(0 == strcmp(pcChildTagName, "CHANGE_ITEM"))
					{
						TiXmlAttribute const* pkAttr = pkChildElement->FirstAttribute();
						int iEquipPos = 0;
						int iEquipItem = 0;
						int iInvType = 0;
						while(pkAttr)
						{
							char const* pcName = pkAttr->Name();
							char const* pcValue = pkAttr->Value();
							if(0 == strcmp(pcName, "EQUIP_POS"))
							{
								iEquipPos = ::atoi(pcValue);
							}
							else if(0 == strcmp(pcName, "EQUIP_ITEM"))
							{
								iEquipItem = ::atoi(pcValue);
							}
							else if(0 == strcmp(pcName, "INV_TYPE"))
							{
								if( 0 == strcmp(pcValue, "CASH") )
								{// CASH
									iInvType = static_cast<int>(IT_FIT_CASH);
								}
								else
								{// NORMAL
									iInvType = static_cast<int>(IT_FIT);
								}
							}
							pkAttr = pkAttr->Next();
						}
						if(0 < iEquipPos
							&& 0 < iEquipItem
							)
						{// 중복 체크를 넣을것
							SItemChangeInfo kInfo(static_cast<EEquipPos>(iEquipPos), iEquipItem, static_cast<EInvType>(iInvType));
							kContItemChangeInfoSet.push_back( kInfo );
						}
					}
					if( !kContItemChangeInfoSet.empty() )
					{
						m_kContChangeItem.insert( std::make_pair(iNo, kContItemChangeInfoSet) );
					}
					
					pkChildElement = pkChildElement->NextSiblingElement();
				}
			}
			pkNode = pkNode->NextSiblingElement();
		}
	}
}

SSetItemEffectIndexInfo const *PgItemMan::GetSetItemEffectInfo(int const iSetItemNo) const
{
	CONST_SET_ITEM_EFFECT_INDEX::const_iterator set_index_itor = m_kSetItemEffectInfo.find(iSetItemNo);
	if(m_kSetItemEffectInfo.end() != set_index_itor)
	{		
		CONT_SET_EFFECT_INDEX::const_iterator set_effect_itor = m_kSetEffectInfo.find((*set_index_itor).second);
		if(m_kSetEffectInfo.end() != set_effect_itor)
		{
			return &(*set_effect_itor).second;
		}
	}

	return NULL;
}

bool PgItemMan::GetSetItemParticleInfo(int const iSetItemNo, SPOTParticleInfo &kResult) const
{
	CONT_SETITEM_POT_PARTICLE_INFO::const_iterator kItor = m_kContSetParticleInfo.find(iSetItemNo);
	if(m_kContSetParticleInfo.end() != kItor)
	{
		kResult = kItor->second;
		return true;
	}
	return false;
}

void PgItemMan::ParseXML_ItemPOTParticleInfo(std::wstring const &kXmlFileName)
{
	m_kContItemParticleInfo.clear();

	TiXmlDocument kXmlDoc( MB(kXmlFileName.c_str()) );

	if(!PgXmlLoader::LoadFile(kXmlDoc, kXmlFileName))
	{
		INFO_LOG(BM::LOG_LV5, __FL__<<L"Failed parse xml ["<<kXmlFileName<<L"]");
		return;
	}

	TiXmlElement *pkNode = kXmlDoc.FirstChildElement("ITEM_POT_PARTICLE");

	pkNode = pkNode->FirstChildElement();
	while(pkNode)
	{
		char const *szName = pkNode->Value();
		if(0 == strcmp(szName, "POT_PARTICLE_INFO"))
		{
			int iItemNo = 0;
			SPOTParticleInfo kItemPOTParticleInfo;

			TiXmlAttribute const *pkAttr = pkNode->FirstAttribute();
			while(pkAttr)
			{
				char const *pcAttrName = pkAttr->Name();
				char const *pcAttrValue = pkAttr->Value();

				if(0 == strcmp(pcAttrName, "ITEM_NO"))
				{
					iItemNo = ::atoi(pcAttrValue);
				}
				else if(0 == strcmp(pcAttrName, "ITEM_PARTICLE_ID1"))
				{
					kItemPOTParticleInfo.kEffectID = pcAttrValue;
				}
				else if(0 == strcmp(pcAttrName, "ITEM_PARTICLE_NODE_ID1"))
				{
					kItemPOTParticleInfo.kAttachNode = pcAttrValue;
				}
				else if(0 == strcmp(pcAttrName, "ITEM_PARTICLE_POT1"))
				{
					kItemPOTParticleInfo.iAttachPointOfTime = ::atoi(pcAttrValue);
				}
				else if(0 == strcmp(pcAttrName, "ITEM_PARTICLE_SCALE1"))
				{
					kItemPOTParticleInfo.fScale = static_cast<float>(::atof(pcAttrValue));
				}
				else if(0 == strcmp(pcAttrName, "ITEM_PARTICLE_EVENT_KEY1"))
				{
					kItemPOTParticleInfo.kEventKey = pcAttrValue;
				}
				else if(0 == strcmp(pcAttrName, "ITEM_PARTICLE_DETACH_WHEN_ACTION_END1"))
				{
					kItemPOTParticleInfo.bDetachWhenActionEnd = (0 == ::strcmp("TRUE", pcAttrValue));
				}
				pkAttr = pkAttr->Next();
			}
			
			if(!kItemPOTParticleInfo.kEffectID.empty())
			{// 단일 아이템으로 인한 특정 시점 파티클 붙이기
				m_kContItemParticleInfo.insert(std::make_pair(iItemNo, kItemPOTParticleInfo));
			}
		}
		pkNode = pkNode->NextSiblingElement();
	}
}

bool PgItemMan::GetItemPOTParticleInfo(int const iItemNo, SPOTParticleInfo &rkResult) const
{
	CONT_POT_PARTICLE_INFO::const_iterator kItor = m_kContItemParticleInfo.find(iItemNo);
	if(m_kContItemParticleInfo.end() != kItor)
	{
		rkResult = kItor->second;
		return true;
	}
	return false;
}

bool PgItemMan::GetChangeItemInfo(int const iSetNo, CONT_ITEM_CHANGE_INFO& rkResult) const
{
	CONT_ITEMSET_CHANGE_EQUIP_INFO::const_iterator kItor = m_kContChangeItem.find(iSetNo);
	if( m_kContChangeItem.end() != kItor)
	{
		rkResult = kItor->second;
		return true;
	}
	return false;
}