#include "stdafx.h"
#include "PgMobileSuit.h"
#include "ServerLib.h"
#include "Lohengrin/LogGroup.h"
#include "Variant/MonsterDefMgr.h"
#include "PgUIScene.h"
#include "Variant/PgClassDefMgr.h"
#include "Variant/ItemDefMgr.h"
#include "Variant/PgItemOptionMgr.h"
#include "Variant/PgMonsterBag.h"
#include "Variant/ItemBagMgr.h"
#include "Variant/ItemMakingDefMgr.h"
#include "Variant/PgDefSpendMoney.h"
#include "PgMissionQuestMan.h"
#include "PgPvPGame.h"
#include "Variant/PgJumpingCharEventMgr.h"

extern int CALLBACK ClientItemResChooser(int const iGenderLimit);
extern int CALLBACK ClientItemNewResChooser(int const iGenderLimit);

bool LoadTBData(std::wstring const &strFolder)
{
	bool const bRet = g_kTblDataMgr.LoadDump(strFolder);
	NILOG(PGLOG_LOG, "LoadTBData - Load Dump End\n");

	if( bRet )
	{
		CItemDefUtil::SetServerMode(false);
		int const iLoadDef = ControlDefUtil::iClientLoadDef;

		SReloadDef kReloadDef;
		g_kTblDataMgr.GetReloadDef(kReloadDef, iLoadDef);
		if (!g_kControlDefMgr.Update(kReloadDef, iLoadDef))
		{
			ASSERT_LOG(false, BM::LOG_LV1, __FL__ << _T("Table Build failed"));
		}
		
		NILOG(PGLOG_LOG, "LoadTBData - Build End\n");
	}

	g_kJumpingCharEventMgr.Build();
	return bRet && AfterLoadTBData();
	//return bRet;
}

bool AfterLoadTBData()
{
	{
		NILOG(PGLOG_LOG, "LoadTBData - Icon Rsc Load Start\n");
		CItemDef::SetResChooserFunc(ClientItemResChooser);
		CItemDef::SetResChooserFuncNew(ClientItemNewResChooser);

		CONT_DEFRES const* pkContDefRes = NULL;
		g_kTblDataMgr.GetContDef(pkContDefRes);

		CONT_DEFRES::const_iterator res_itr = pkContDefRes->begin();
		unsigned int iCount = 0;
		float fIconTime = 0.0f;
		float fXmlTime = 0.0f;
		while(res_itr != pkContDefRes->end())
		{
			//이제 필요 할때만 로딩 하자
			//SUVInfo kUVInfo( (*res_itr).second.U, (*res_itr).second.V, (*res_itr).second.UVIndex);
			float fTime = NiGetCurrentTimeInSec();
			//g_kUIScene.AddIconRsc( (*res_itr).second.ResNo, (*res_itr).second.strIconPath, kUVInfo);			
			float fTime2 = NiGetCurrentTimeInSec();
			fIconTime += (fTime2 - fTime);
			PgXmlLoader::PrepareXmlDocument(MB((*res_itr).second.strXmlPath));
			fXmlTime += (NiGetCurrentTimeInSec() - fTime2);
			++res_itr;
			iCount++;
		}
		NILOG(PGLOG_LOG, "AddIconRsc %f, %f time %d count \n", fIconTime, fXmlTime, iCount);
		NILOG(PGLOG_LOG, "LoadTBData - Icon Rsc Load End\n");

		CONT_DEF_QUEST_REWARD const* pkQuestReward = NULL;
		g_kTblDataMgr.GetContDef(pkQuestReward);
		CONT_DEF_QUEST_REWARD::const_iterator quest_iter = pkQuestReward->begin();
		fXmlTime = 0.f;
		while( pkQuestReward->end() != quest_iter )
		{
			CONT_DEF_QUEST_REWARD::mapped_type const& rkDefQuest = (*quest_iter).second;
			if( PgQuestInfoUtil::IsLoadXmlType(rkDefQuest.iDBQuestType) )
			{
				float fTime = NiGetCurrentTimeInSec();
				PgXmlLoader::PrepareXmlDocument( MB(rkDefQuest.kXmlPath), PXDAW_ParseQuest );
				fXmlTime += (NiGetCurrentTimeInSec() - fTime);
			}
			++quest_iter;
		}
		NILOG(PGLOG_LOG, "QuestXML Load %f time %d count \n", fXmlTime, pkQuestReward->size());
		NILOG(PGLOG_LOG, "LoadTBData - QuestXML Load End\n");
	}

	return true;
}

bool GetAbilName(WORD const AbilType, const wchar_t *&pString)
{
	CONT_DEF_ABIL_TYPE const* pkDefAbilType = NULL;
	g_kTblDataMgr.GetContDef( pkDefAbilType );

	assert(pkDefAbilType);
	if ( pkDefAbilType )
	{
		CONT_DEF_ABIL_TYPE::const_iterator itor = pkDefAbilType->find(AbilType);
		if(itor != pkDefAbilType->end())
		{
			return GetDefString((*itor).second.NameNo, pString);
		}
	}
//	assert(NULL && "Can't Find Abil Type");
	return false;
}

bool GetEffectName(DWORD const dwEffectID, wchar_t const *&pString)
{
	GET_DEF(CEffectDefMgr, kEffectDefMgr);
	const	CEffectDef	*pkEffectDef = kEffectDefMgr.GetDef(dwEffectID);
	if(!pkEffectDef) return false;

	return	GetDefString(pkEffectDef->m_iName, pString);
}

bool GetItemName(DWORD const dwItemNo, wchar_t const *&pString)
{
	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const *pkItemDef = kItemDefMgr.GetDef(dwItemNo);

	if(pkItemDef)
	{
		return GetDefString(pkItemDef->NameNo(), pString);
	}
	return false;
}

int	GetClassUpgradeOrder(int iLastClassID,int *pkout_UpgradeOrderArray)
{
	int	iOrderCount = 0;

	CONT_DEFUPGRADECLASS const* pkDefUpgradeClass = NULL;
	g_kTblDataMgr.GetContDef( pkDefUpgradeClass );

	if ( !pkDefUpgradeClass )
	{
		PG_ASSERT_LOG(false);
		return 0;
	}

	TBL_DEF_EFFGRADECLASS	*pUpgradeClassData = NULL;
	for(int i=0;i<10;i++)
	{
		for(CONT_DEFUPGRADECLASS::const_iterator itor = pkDefUpgradeClass->begin(); itor != pkDefUpgradeClass->end();itor++)
		{
			pUpgradeClassData = (TBL_DEF_EFFGRADECLASS*)&(itor->second);
			if(pUpgradeClassData->iClass == iLastClassID)
			{
				if(pkout_UpgradeOrderArray)
					*(pkout_UpgradeOrderArray+pUpgradeClassData->byGrade) = iLastClassID;
				if(pUpgradeClassData->byGrade == 0) return	iOrderCount;

				iLastClassID = pUpgradeClassData->iParentClass;
				iOrderCount++;
				break;
			}
		}
	}
	return	iOrderCount;	//	전직 횟수
}

#ifndef USE_INB
namespace PgDefStringDebugUtil
{
	typedef std::map< size_t, std::wstring > ContDebugString;
	static ContDebugString m_kDebugDefString;
	static ContDebugString m_kDebugTextString;

	void ClearDebugString()
	{
		m_kDebugDefString.clear();
		m_kDebugTextString.clear();
	}

	std::wstring const* GetDebugString(ContDebugString& rkDebugStr, size_t const iTextNo, std::wstring const& rkOriginalStr, TCHAR const* szHeader)
	{
		ContDebugString::iterator find_iter = rkDebugStr.find(iTextNo);
		if( rkDebugStr.end() == find_iter )
		{
			std::wstring const kText = (BM::vstring(szHeader) + BM::vstring(iTextNo) + _T(".") + rkOriginalStr).operator const std::wstring &();
			std::pair<ContDebugString::iterator, bool> kRet = rkDebugStr.insert( std::make_pair(iTextNo, kText) );
			
			find_iter = kRet.first;
		}

		if( rkDebugStr.end() != find_iter )
		{
			return &((*find_iter).second);
		}
		return NULL;
	}

	std::wstring const* GetDebugDefString(int const iTextNo, std::wstring const& rkOriginalStr)
	{
		if( !g_pkApp->VisibleTextDebug() )
		{
			return &rkOriginalStr;
		}
		return GetDebugString(m_kDebugDefString, iTextNo, rkOriginalStr, _T("D"));
	}

	std::wstring const* GetDebugTextString(unsigned long const iTextNo, std::wstring const& rkOriginalStr)
	{
		bool bVisibleDebug = g_pkApp->VisibleTextDebug();
		if( 500000 <= iTextNo
		&&	599999 >= iTextNo ) // 퀘스트 텍스트 테이블 범위
		{
			bVisibleDebug = g_pkApp->VisibleQTextDebug();
		}

		if( !bVisibleDebug )
		{
			return &rkOriginalStr;
		}
		return GetDebugString(m_kDebugTextString, iTextNo, rkOriginalStr, _T("T"));
	}
}
#endif

bool GetDefString(int const iTextNo, std::wstring const *&pkOut)
{
	CONT_DEFSTRINGS const *pContDefStrings = NULL;
	g_kTblDataMgr.GetContDef(pContDefStrings);

	CONT_DEFSTRINGS::const_iterator str_itor = pContDefStrings->find(iTextNo);
	if( pContDefStrings->end() != str_itor )
	{
#ifndef USE_INB
		pkOut = PgDefStringDebugUtil::GetDebugDefString(iTextNo, (*str_itor).second.strText); // 변환된 문자열로
#else
		pkOut = &(*str_itor).second.strText; // 원본
#endif
		return true;
	}
	return false;
}

bool GetDefString(int const iTextNo, wchar_t const *&pString)
{
	std::wstring const *pkDefStr = NULL;
	if( GetDefString(iTextNo, pkDefStr) )
	{
		pString = pkDefStr->c_str();
		return true;
	}
	return false;
}