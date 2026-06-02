#include "stdafx.h"
#include "lwUI.h"
#include "PgNetwork.h"
#include "PgUIScene.h"
#include "PgPilotMan.h"
#include "PgActor.h"
#include "PgPilot.h"
#include "PgMobileSuit.h"
#include "Variant/PgPlayer.h"
#include "Variant/constant.h"
#include "Variant/PgClassDefMgr.h"
#include "Variant/Inventory.H"
#include "Variant/effect.H"
#include "Loki/Threads.h"

wchar_t const BUFFICON_NAME[]=L"BuffIcon";
wchar_t const BUFFCOOL_NAME[]=L"FRM_BUFF_TEXT";
int const NONE_EFFECT_VALUE = -1;

DWORD GetRemainTime(int const iDurationTime, DWORD const dwStartTime, DWORD const dwEndTime)
{
	if(0 < iDurationTime && 0 < dwStartTime && 0 < dwEndTime)
	{
		return std::max<DWORD>(0, dwEndTime - dwStartTime);
	}
	return 0;
}

typedef struct tagBuffInfo
{
	DWORD	m_dwStartTime;	//처음 생성된 시간
	DWORD	m_dwMainTime;	//남은 지속시간
	DWORD	m_dwRestTime;
	int		m_iKey;			//버프 Key 값(BuffNo or ItemNo)
	int		m_iID;			//버프 넘버
	int		m_iValue;
	int		m_iIconNum;
	BM::vstring	m_szID;
	bool	m_bToggle;
	tagBuffInfo() { Clear(); }
	void Clear()
	{
		m_dwStartTime = 0;
		m_dwMainTime = m_dwRestTime = 0;
		m_iKey = NONE_EFFECT_VALUE; m_iID = NONE_EFFECT_VALUE; m_iValue = NONE_EFFECT_VALUE; m_iIconNum = NONE_EFFECT_VALUE;
		m_bToggle = false;
	}
	bool Update(DWORD dwAccumTime);
}SBuffInfo;

bool tagBuffInfo::Update(DWORD dwAccumTime)
{
	if(m_dwStartTime == 0) { return false; }
	if (m_bToggle) { return true; }

	m_dwRestTime = (m_dwMainTime - (dwAccumTime - m_dwStartTime))/100;
	if(m_dwRestTime<0) {	m_dwRestTime = 0; }

	if(m_dwMainTime>0 && m_dwRestTime <= 0) { return false; }

	return true;
}

typedef std::map<int, SBuffInfo> BuffInfoMap;
typedef BuffInfoMap::iterator BuffInfoMapItor;

typedef std::vector<SBuffInfo> BuffInfoVec;
Loki::Mutex g_kBuffMutex;

BuffInfoMap g_BuffInfoMap;
BuffInfoVec g_BuffInfoVec;

extern void ClearBuffInfo()
{
	BM::CAutoMutex kLock(g_kBuffMutex);
	g_BuffInfoMap.clear();
	g_BuffInfoVec.clear();
	BuffInfoVec kSwap;
	kSwap.swap(g_BuffInfoVec);
}

bool RemoveFromBuffVec(int const iEffectID)
{
	if (0>=iEffectID){return false;}

	BM::CAutoMutex kLock(g_kBuffMutex);
	BuffInfoVec::iterator it = g_BuffInfoVec.begin();
	while (g_BuffInfoVec.end() != it)
	{
		if (iEffectID == (*it).m_iID)
		{
			g_BuffInfoVec.erase(it);
			return true;
		}
		++it;
	}

	return false;
}

void ReOrderBuffWnd(XUI::CXUI_Wnd* pWnd)
{
	if (!pWnd) { return; }
	XUI::CXUI_Builder *pBuild = dynamic_cast<XUI::CXUI_Builder *>(pWnd->GetControl(_T("BLD_BUF")));
	if( pBuild == NULL )
	{
		return;
	}
	BM::CAutoMutex kLock(g_kBuffMutex);
	//빈칸 앞으로 땡기기
	BuffInfoMap tempMap;
	std::wstring wsValue;
	int iAdjust = 0;
	XUI::CXUI_Icon *pIcon = NULL;
	int iTotal = pBuild->CountX()*pBuild->CountY();
	BM::vstring kStr;
	BM::vstring kStrCool;
	BuffInfoMapItor itor;
	int iEffectKey = NONE_EFFECT_VALUE;
	int iEffectID = NONE_EFFECT_VALUE;

	BuffInfoVec::const_iterator it = g_BuffInfoVec.begin();
	int iCount = 0;
	while (g_BuffInfoVec.end() != it)
	{
		kStr = BUFFICON_NAME;
		kStr += iCount;
		pIcon = dynamic_cast<XUI::CXUI_Icon *>(pWnd->GetControl((std::wstring const&)kStr));
		if (!pIcon) { continue;	}

		const SBuffInfo& kType = (*it);
		iEffectKey = kType.m_iKey;
		iEffectID = kType.m_iID;

		{
			pIcon->SetCustomData(&iEffectID, sizeof(iEffectID));
		}

		SIconInfo kIconInfo = pIcon->IconInfo();
		kIconInfo.iIconKey = iEffectID;
		pIcon->SetIconInfo(kIconInfo);

		kStrCool = BUFFCOOL_NAME;
		kStrCool += iCount;
		XUI::CXUI_Wnd* pkCool = dynamic_cast<XUI::CXUI_Wnd *>(pWnd->GetControl((std::wstring const&)kStrCool));
		if (pkCool)
		{
			pkCool->Text(_T(""));
		}
		++iCount;
		++it;
	}

	iEffectKey = NONE_EFFECT_VALUE;
	iEffectID = NONE_EFFECT_VALUE;
	for (int i = iCount; i < iTotal; ++i)
	{
		kStr = BUFFICON_NAME;
		kStr += i;
		pIcon = dynamic_cast<XUI::CXUI_Icon *>(pWnd->GetControl((std::wstring const&)kStr));
		if (!pIcon)	{ continue;	}

		{
			pIcon->SetCustomData(&iEffectID, sizeof(iEffectID));
		}
		SIconInfo kIconInfo(KUIG_EFFECT, 0);
		pIcon->SetIconInfo(kIconInfo);

		kStrCool = BUFFCOOL_NAME;
		kStrCool += iCount;
		XUI::CXUI_Wnd* pkCool = dynamic_cast<XUI::CXUI_Wnd *>(pWnd->GetControl((std::wstring const&)kStrCool));
		if (pkCool)
		{
			pkCool->Text(_T(""));
		}
	}

	/*	for(int i = 0; i < iTotal; ++i)
	{		
	kStr = BUFFICON_NAME;
	kStr += i;
	pIcon = dynamic_cast<XUI::CXUI_Icon *>(pWnd->GetControl((std::wstring const&)kStr));
	if (!pIcon)
	{
	continue;
	}
	iEffectID = 0;
	pIcon->GetCustomData(&iEffectID, sizeof(iEffectID));
	SIconInfo kIconInfo = pIcon->IconInfo();
	//if(iEffectID != NONE_EFFECT_VALUE)	//비지 않았으면
	if(kIconInfo.iIconKey>0)
	{
	if(i != iAdjust)
	{
	//데이터를 옮기고
	kStr = BUFFICON_NAME;
	kStr += iAdjust;
	XUI::CXUI_Icon *pNewIcon = dynamic_cast<XUI::CXUI_Icon *>(pWnd->GetControl((std::wstring const&)kStr));
	if (pNewIcon)
	{
	pNewIcon->SetCustomData(&iEffectID, sizeof(iEffectID));

	SIconInfo kNewIconInfo = pNewIcon->IconInfo();
	kNewIconInfo.iIconKey = __max(kIconInfo.iIconKey, iEffectID);

	pNewIcon->SetIconInfo(kNewIconInfo);

	//원래 자리를 비우자.
	kIconInfo.iIconKey = 0;
	pIcon->SetIconInfo(kIconInfo);

	int iTemp = -1;
	pIcon->SetCustomData(&iTemp, sizeof(iTemp));
	pIcon->Text(_T(""));
	}
	}
	++iAdjust;
	}
	}*/
}

bool CheckTimeLimitEffect(SBuffInfo& rkInfo)
{
	if(g_kPilotMan.GetPlayerUnit())
	{
		CUnit* pkUnit = g_kPilotMan.GetPlayerUnit();
		if(pkUnit)
		{
			CEffect* pkEffect = pkUnit->GetEffect(rkInfo.m_iID);
			if(pkEffect)
			{
				BM::DBTIMESTAMP_EX const& rkExpireTime = pkEffect->ExpireTime();
				if(!rkExpireTime.IsNull())	//시간대 제한이 있는 아이템
				{
					BM::DBTIMESTAMP_EX kLocalTime;
					g_kEventView.GetLocalTime(kLocalTime);
					BM::PgPackedTime kPackedTime(kLocalTime);
					
					__int64 const iDiffSec = CGameTime::GetElapsedTime(rkExpireTime, kLocalTime, CGameTime::SECOND);
					rkInfo.m_dwRestTime = static_cast<DWORD>(iDiffSec);
					rkInfo.m_dwMainTime = rkInfo.m_dwRestTime*100;
					return true;
				}
			}
		}
	}

	return false;
}

void lwSetBuffWnd(lwUIWnd UIWnd)	//처음
{
	XUI::CXUI_Wnd* pWnd = NULL;
	if(UIWnd.IsNil())
	{
		pWnd = XUIMgr.Get(_T("FRM_BUFF"));
		if(pWnd == NULL)
		{
			return;
		}
	}
	else
	{
		pWnd = UIWnd.GetSelf();
	}
	
	XUI::CXUI_Builder *pBuild = dynamic_cast<XUI::CXUI_Builder *>(pWnd->GetControl(_T("BLD_BUF")));
	if( pBuild == NULL )
	{
		return;
	}

	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if (pkPlayer == NULL)
	{
		return;
	}

	BM::CAutoMutex kLock(g_kBuffMutex);
	g_BuffInfoMap.clear();
	g_BuffInfoVec.clear();

	PgUnitEffectMgr& rkEffectMgr = pkPlayer->GetEffectMgr();
	int const iMaxCount = pBuild->CountX()*pBuild->CountY();
	BM::vstring kStr;
	BM::vstring kKey;
	ContEffectItor kItor;
	rkEffectMgr.GetFirstEffect(kItor);
	CEffect* pkEffect = NULL;
	int iCount = 0;

	XUI::CXUI_Icon *pIcon = NULL;
	//while ((pkEffect = rkEffectMgr.GetNextEffect(kItor)) != NULL)
	while(iCount < iMaxCount)
	{
		kStr = BUFFICON_NAME;
		kStr += iCount;
		pIcon = dynamic_cast<XUI::CXUI_Icon *>(pWnd->GetControl((std::wstring const&)kStr));
		if( pIcon != NULL )
		{
			pkEffect = rkEffectMgr.GetNextEffect(kItor);
			SBuffInfo tempInfo;
			int iEffectKey = NONE_EFFECT_VALUE;
			int iEffectID = NONE_EFFECT_VALUE;
			if ( !pkEffect )
			{
				tempInfo.m_iID = NONE_EFFECT_VALUE;
				tempInfo.m_iKey = NONE_EFFECT_VALUE;

				BM::vstring kStrCool(BUFFCOOL_NAME);
				kStrCool += iCount;
				XUI::CXUI_Wnd* pkCool = dynamic_cast<XUI::CXUI_Wnd *>(pWnd->GetControl((std::wstring const&)kStrCool));
				if (pkCool)
				{
					pkCool->Text(L"");
				}
			}
			else
			{
				if ( pkEffect->GetAbil(AT_DEF_RES_NO) )
				{// 아이콘이 없다면 그리지 않는다.
					EEffectType const eType = static_cast<EEffectType>(pkEffect->GetAbil(AT_TYPE));
					if( EFFECT_TYPE_BLESSED == eType 
						|| EFFECT_TYPE_CURSED == eType 
						|| EFFECT_TYPE_PENALTY == eType
						|| EFFECT_TYPE_ITEM == eType
						|| EFFECT_TYPE_ABIL == eType
						|| EFFECT_TYPE_SYSTEM == eType
						)
					{
						iEffectKey = pkEffect->GetKey();
						iEffectID = pkEffect->GetEffectNo();

						int const iDurTime = pkEffect->GetDurationTime();
						tempInfo.m_dwStartTime = pkEffect->GetStartTime();
						tempInfo.m_dwMainTime = GetRemainTime(iDurTime, pkEffect->GetStartTime(), pkEffect->GetEndTime());
						tempInfo.m_dwRestTime = tempInfo.m_dwMainTime / 100;
						tempInfo.m_bToggle = iDurTime <= 0;
						tempInfo.m_iKey = iEffectKey;
						tempInfo.m_iID  = iEffectID;
						tempInfo.m_szID = kStr;
						tempInfo.m_iIconNum = iCount;

						CheckTimeLimitEffect(tempInfo);
						
						std::pair<BuffInfoMap::iterator, bool> kRet = g_BuffInfoMap.insert(std::make_pair(iEffectID, tempInfo));
						if(kRet.second)
						{
							g_BuffInfoVec.push_back(tempInfo);
						}
					}
				}
				else
				{
					continue;
				}
			}

			SIconInfo kIconInfo(KUIG_EFFECT, iEffectID);
			pIcon->SetIconInfo(kIconInfo);			
			{
				pIcon->SetCustomData(&iEffectID, sizeof(tempInfo.m_iID));
			}
		}
		++iCount;
	}
}

#include "ServerLib.h"
void lwAddBuff(int const iEffectKey, int const iEffectID,int const iValue, bool const bNewAdd, DWORD const dwElapsedTime, DWORD const dwStartTime, DWORD const dwEndTime)
{
	GET_DEF(CEffectDefMgr, kEffectDefMgr);
	CEffectDef const* pDef = kEffectDefMgr.GetDef(iEffectID);
	if(pDef == NULL)
	{
		return;
	}

	if(!pDef->GetAbil(AT_DEF_RES_NO))
	{
		// 아이콘이 없으면 추가하지 않는다.
		return;
	}

	BM::CAutoMutex kLock(g_kBuffMutex);
	BuffInfoMapItor itor = g_BuffInfoMap.find(iEffectID);
	if(itor != g_BuffInfoMap.end() )	//이미 있는거면
	{
		if (true == bNewAdd)
		{
			itor->second.m_dwStartTime = dwStartTime;
		}

		int const iDurationTime = pDef->GetDurationTime();
		itor->second.m_dwMainTime = GetRemainTime(iDurationTime, dwStartTime, dwEndTime);
		itor->second.m_dwRestTime = itor->second.m_dwMainTime / 100;
		itor->second.m_bToggle = iDurationTime <= 0;
		itor->second.m_iValue = iValue;

		return;			 
	}

	XUI::CXUI_Wnd* pWnd = XUIMgr.Get(_T("FRM_BUFF"));
	if( pWnd == NULL )
	{
		return;
	}
	XUI::CXUI_Builder *pBuild = dynamic_cast<XUI::CXUI_Builder *>(pWnd->GetControl(_T("BLD_BUF")));
	if( pBuild == NULL )
	{
		return;
	}

	int const iMapSize = static_cast<int>(g_BuffInfoMap.size());

	if( iMapSize <= pBuild->CountX()*pBuild->CountY() )
	{
		BM::vstring kStr(BUFFICON_NAME);
		kStr += iMapSize;
		XUI::CXUI_Icon *pIcon = dynamic_cast<XUI::CXUI_Icon *>(pWnd->GetControl((std::wstring const&)kStr));
		if( pIcon != NULL )
		{
			SIconInfo kIconInfo(KUIG_EFFECT, iEffectID);
			pIcon->SetIconInfo(kIconInfo);
			
			const EEffectType eType = (EEffectType)pDef->GetAbil(AT_TYPE);			
			if( EFFECT_TYPE_BLESSED == eType 
				|| EFFECT_TYPE_CURSED == eType 
				|| EFFECT_TYPE_PENALTY == eType 
				|| EFFECT_TYPE_ITEM == eType
				|| EFFECT_TYPE_ABIL == eType
				|| EFFECT_TYPE_SYSTEM == eType
				)
			{
				DWORD const iDurationTime = static_cast<DWORD>(pDef->GetDurationTime());

				SBuffInfo tempInfo;
				tempInfo.m_dwStartTime = dwStartTime;
				tempInfo.m_dwMainTime = GetRemainTime(iDurationTime, dwStartTime, dwEndTime);
				tempInfo.m_dwRestTime = tempInfo.m_dwMainTime / 100;
				tempInfo.m_bToggle = iDurationTime <= 0;
				tempInfo.m_iKey = iEffectKey;
				tempInfo.m_iID  = iEffectID;
				tempInfo.m_iValue = iValue;
				tempInfo.m_szID = kStr;
				tempInfo.m_iIconNum = iMapSize;

				CheckTimeLimitEffect(tempInfo);

				std::pair<BuffInfoMap::iterator, bool> kRet = g_BuffInfoMap.insert(std::make_pair(iEffectID, tempInfo));
				if(kRet.second)
				{
					g_BuffInfoVec.push_back(tempInfo);
				}				
				pIcon->SetCustomData(&iEffectID, sizeof(iEffectID));
			}
			pIcon->SetInvalidate(true);
		}
	}
}

void lwDeleteBuff(int iEffectID)
{
	XUI::CXUI_Wnd* pWnd = XUIMgr.Get(_T("FRM_BUFF"));
	if( pWnd == NULL )
	{
		return;
	}
	BM::CAutoMutex kLock(g_kBuffMutex);
	BuffInfoMapItor it = g_BuffInfoMap.find(iEffectID);
	if( it != g_BuffInfoMap.end() )
	{
		XUI::CXUI_Icon *pIcon = dynamic_cast<XUI::CXUI_Icon *>(pWnd->GetControl(it->second.m_szID));
		if(pIcon != NULL)
		{//아이콘에서 지우기
			SIconInfo kIconInfo = pIcon->IconInfo();
			kIconInfo.iIconKey = 0;
			pIcon->SetIconInfo(kIconInfo);


			{
				pIcon->SetCustomData(&NONE_EFFECT_VALUE, sizeof(NONE_EFFECT_VALUE));
			}

			BM::vstring kStrCool(BUFFCOOL_NAME);
			kStrCool+=pIcon->BuildIndex();
			XUI::CXUI_Wnd* pkCool = dynamic_cast<XUI::CXUI_Wnd *>(pWnd->GetControl((std::wstring const&)kStrCool));
			if (pkCool)
			{
				pkCool->Text(_T(""));
			}
			pIcon->SetInvalidate(true);
		}
		
		it->second.Clear();
		g_BuffInfoMap.erase(it);
		RemoveFromBuffVec(iEffectID);

		ReOrderBuffWnd(pWnd);
	}
}

void lwUpdateBuffWnd()
{
	BM::CAutoMutex kLock(g_kBuffMutex);
	if( g_BuffInfoMap.empty() )
	{
		return;
	}
	XUI::CXUI_Wnd* pWnd = XUIMgr.Get(_T("FRM_BUFF"));
	if( pWnd == NULL )
	{
		return;
	}
	XUI::CXUI_Builder *pBuild = dynamic_cast<XUI::CXUI_Builder *>(pWnd->GetControl(_T("BLD_BUF")));
	if( pBuild == NULL )
	{
		return;
	}

	int iTotal = pBuild->CountX()*pBuild->CountY();

	BM::vstring kStr;
	BM::vstring kStrCool;
	BuffInfoMapItor itor;
	XUI::CXUI_Icon *pIcon = NULL;
	int iEffectID = NONE_EFFECT_VALUE;
	int iEffectKey = NONE_EFFECT_VALUE;
	bool bIsDelete = false;
	DWORD const dwServerElapsedTime32 = g_kEventView.GetServerElapsedTime();
	XUI::CXUI_Font* pkFont = NULL;
	//일단 지우기
	for(int i = 0; i < iTotal; ++i)
	{
		kStr = BUFFICON_NAME;
		kStr += i;
		pIcon = dynamic_cast<XUI::CXUI_Icon *>(pWnd->GetControl((std::wstring const&)kStr));
		if (!pIcon)
		{
			continue;
		}

		kStrCool = BUFFCOOL_NAME;
		kStrCool += i;
		XUI::CXUI_Wnd* pkCool = dynamic_cast<XUI::CXUI_Wnd *>(pWnd->GetControl((std::wstring const&)kStrCool));
		if (!pkCool)
		{
			continue;
		}

		{
			iEffectID = NONE_EFFECT_VALUE;
			pIcon->GetCustomData(&iEffectID, sizeof(iEffectID));
		}

		if(iEffectID != NONE_EFFECT_VALUE)
		{
			itor = g_BuffInfoMap.find(iEffectID);
			if(itor != g_BuffInfoMap.end())
			{
				if(itor->second.m_dwMainTime>0 || itor->second.m_dwStartTime == 0)
				{
					if(itor->second.Update(dwServerElapsedTime32))
					{
						int iTimeSetCount = 0;
						bool bEnd = false;
						kStr = _T("");
						DWORD dwTime = itor->second.m_dwRestTime;
						int iDay = dwTime / 864000;
						if( 0 < iDay )			//일 단위 이상이면
						{
							kStr+=iDay;
							kStr+=TTW(174);//일
							dwTime-=(iDay*864000);
							++iTimeSetCount;
						}
						
						iDay = dwTime / 36000;
						if( 0 < iDay )			//시간 단위 이상이면
						{
							bEnd = 0<kStr.size();
							if(bEnd)
							{
								kStr+=L"\n";
							}
							kStr+=iDay;
							kStr+=TTW(90004);//시간
							dwTime-=(iDay*36000);
							++iTimeSetCount;
						}

						iDay = dwTime / 600;
						if( !bEnd && 0 < iDay )			//분 단위 이상이면
						{
							bEnd = 0<kStr.size();
							if(bEnd)
							{
								kStr+=L"\n";
							}
							kStr+=iDay;
							kStr+=TTW(90003);//분
							dwTime-=(iDay*600);
							++iTimeSetCount;
						}

						if ( !bEnd && 0 < dwTime )
						{
							dwTime /= 10;
							if(0<kStr.size())
							{
								kStr+=L"\n";
							}
							else
							{
								if(dwTime <100)	//10초 이하면 빨간색~
								{
									kStr += "{C=0xFFFF0000/}";
								}
							}

							kStr+=dwTime;
							kStr += TTW(90002);//초
							++iTimeSetCount;
						}
						
						if(!pkFont)
						{
							pkFont = g_kFontMgr.GetFont(pkCool->Font());
						}

						if(pkFont)
						{
							--iTimeSetCount;
							std::max(0,iTimeSetCount);
							int h = iTimeSetCount ? pkFont->GetHeight() : 0;
							pkCool->TextPos(POINT2(pkCool->TextPos().x, pIcon->Height()-h*iTimeSetCount));
						}
						pkCool->Text((std::wstring const&)kStr);
					}
					else
					{
						SIconInfo kIconInfo = pIcon->IconInfo();
						
						kIconInfo.iIconKey = 0;
						pIcon->SetIconInfo(kIconInfo);

						{
							pIcon->SetCustomData(&NONE_EFFECT_VALUE, sizeof(NONE_EFFECT_VALUE));
						}
						g_BuffInfoMap.erase(itor);
						RemoveFromBuffVec(iEffectID);
						pkCool->Text(_T(""));

					//	leesg213 주석처리
					//	BM::GUID kGuid;
					//	if(g_kPilotMan.GetPlayerPilot(kGuid))
					//	{
					//		PgPilot *pkPilot = g_kPilotMan.FindPilot(kGuid);
					//		pkPilot->GetUnit()->DeleteEffect(iEffectID);
					//	}
						bIsDelete = true;
					}
				}
				else
				{
					pkCool->Text(_T(""));
					if (g_BuffInfoMap.end() != itor
						&& 0<itor->second.m_dwMainTime )	//원래 토탈시간이 0
					{
						SIconInfo kIconInfo = pIcon->IconInfo();

						kIconInfo.iIconKey = 0;
						pIcon->SetIconInfo(kIconInfo);

						{
							pIcon->SetCustomData(&NONE_EFFECT_VALUE, sizeof(NONE_EFFECT_VALUE));
						}
						g_BuffInfoMap.erase(itor);
						RemoveFromBuffVec(iEffectID);

						bIsDelete = true;
					}
				}
			}
			else
			{
				pkCool->Text(_T(""));
			}
		}	
		else
		{
			pkCool->Text(_T(""));
			SIconInfo kIconInfo = pIcon->IconInfo();
			if (kIconInfo.iIconKey)
			{
				bIsDelete = true;
			}
			kIconInfo.iIconKey = 0;
			pIcon->SetIconInfo(kIconInfo);
		}
	}

	//지워진게 없으면 여기서 끝내자.
	if( !bIsDelete )
	{
		return;
	}

	ReOrderBuffWnd(pWnd);
/*
	//빈칸 앞으로 땡기기
	BuffInfoMap tempMap;
	std::wstring wsValue;
	int iAdjust = 0;
	for(int i = 0; i < iTotal; ++i)
	{		
		kStr = BUFFICON_NAME;
		kStr += i;
		pIcon = dynamic_cast<XUI::CXUI_Icon *>(pWnd->GetControl((std::wstring const&)kStr));
		iEffectID = 0;
		pIcon->GetCustomData(&iEffectID, sizeof(iEffectID));
		if(iEffectID != NONE_EFFECT_VALUE)	//비지 않았으면
		{
			if(i != iAdjust)
			{
				//데이터를 옮기고
				kStr = BUFFICON_NAME;
				kStr += iAdjust;
				XUI::CXUI_Icon *pNewIcon = dynamic_cast<XUI::CXUI_Icon *>(pWnd->GetControl((std::wstring const&)kStr));
				pNewIcon->SetCustomData(&iEffectID, sizeof(iEffectID));
				
				SIconInfo kIconInfo = pIcon->IconInfo();

				SIconInfo kNewIconInfo = pNewIcon->IconInfo();
				kNewIconInfo.iIconKey = kIconInfo.iIconKey;
				
				pNewIcon->SetIconInfo(kNewIconInfo);

				//원래 자리를 비우자.
				kIconInfo.iIconKey = 0;
				pIcon->SetIconInfo(kIconInfo);

				int iTemp = NONE_EFFECT_VALUE;
				pIcon->SetCustomData(&iTemp, sizeof(iTemp));
				pIcon->Text(_T(""));
			}
			++iAdjust;
		}
	}*/
}


int	lwGetBuffEffectNo(int const index)
{
	BM::CAutoMutex kLock(g_kBuffMutex);
	if(0 <= index && index < g_BuffInfoVec.size())
	{
		return g_BuffInfoVec.at(index).m_iID;
	}

	return 0;
}

void lwUpdateBuffEndTime(int const iEffectID, DWORD const dwEndTime)
{
	BM::CAutoMutex kLock(g_kBuffMutex);
	BuffInfoMapItor it = g_BuffInfoMap.find(iEffectID);
	if( it != g_BuffInfoMap.end() )
	{
		BuffInfoMap::mapped_type & Info = (*it).second;
		Info.m_dwMainTime = GetRemainTime(dwEndTime-Info.m_dwStartTime, Info.m_dwStartTime, dwEndTime);
		Info.m_dwRestTime = Info.m_dwMainTime / 100;
	}
}