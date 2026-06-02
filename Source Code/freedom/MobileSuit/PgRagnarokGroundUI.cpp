#include "stdafx.h"
#include "PgRagnarokGroundUI.h"
#include "Variant/PgStringUtil.h"
#include "lwUI.h"
#include "PgPilotMan.h"
#include "PgNetwork.h"
#include "PgClientParty.h"
#include "lwTrigger.h"
#include "PgTrigger.h"
#include "PgChatMgrClient.h"
#include "PgWorld.h"

namespace PgRagnarokGroundUI
{
	void ProcessMsg(BM::Stream::DEF_STREAM_TYPE const wPacketType, BM::Stream& rkPacket)
	{
		switch( wPacketType )
		{
		/*case:
			{
			}break;*/
		}
	}

	bool RegisterWrapper(lua_State *pState)
	{
		using namespace lua_tinker;

		def(pState, "CallRagUI", lwCallRagUI);
		def(pState, "EnterRagDungeon", lwEnterRagDungeon);
		def(pState, "ReqRagnarokBless", lwReqRagnarokBless);

		return true;
	}
	
	void lwCallRagUI(int const GroundKey, char const* TriggerName)
	{
		g_kRagnarokGroundEnterUIMgr.Call(GroundKey, TriggerName);
	}

	void lwEnterRagDungeon(int const ModeNo)
	{
		if( !CheckCanEnterDungeon(ModeNo) )
		{
			return;
		}
		
		lua_tinker::call<void, lwWString, int>("OnRagDungeonEnter", lwWString(g_kRagnarokGroundEnterUIMgr.GetTriggerName()), ModeNo);
	}

	void lwReqRagnarokBless(lwGUID lwNpcGuid)
	{
		BM::GUID const & NpcGuid = lwNpcGuid();

		if( BM::GUID::IsNull(NpcGuid) )
		{
			g_kChatMgrClient.AddMessage(527, SChatLog(CT_EVENT), true);
			return;
		}

		PgPlayer* pPlayer = g_kPilotMan.GetPlayerUnit();
		if( !pPlayer )
		{
			return;
		}

		SGroundKey const& GroundKey = pPlayer->GroundKey();
		SWorldNpc world_npc = g_kWorldNpcMgr.FindNpc(NpcGuid, GroundKey.GroundNo());
		int const EffectNo = world_npc.GiveEffectNo;

		GET_DEF(CEffectDefMgr, EffectDefMgr);
		const CEffectDef *pDef = EffectDefMgr.GetDef(EffectNo);
		if ( !pDef )
		{
			return;
		}

		BM::Stream Packet(PT_C_M_REQ_RAGNAROK_EFFECT);
		Packet.Push( NpcGuid );
		NETWORK_SEND( Packet );
	}

	bool CheckCanEnterDungeon(int const ModeNo)
	{
		PgPlayer* pPlayer = g_kPilotMan.GetPlayerUnit();
		if(!pPlayer)
		{
			return false;
		}

		int const MyLevel = pPlayer->GetAbil(AT_LEVEL);

		if( PgClientPartyUtil::IsInParty() )
		{// 파티에 속해 있으면
			if( PgClientPartyUtil::IsPartyMaster(pPlayer->GetID()) )
			{// 파티장만 신청 할 수 있다.
				if( false == CheckCanEnterLevel(MyLevel, ModeNo) )
				{
					lwAddWarnDataTT(200106);
					return false;
				}
				ContPartyMember ContParty;
				g_kParty.GetPartyMemberList(ContParty);
				ContPartyMember::iterator iter = ContParty.begin();
				for( ; ContParty.end() != iter; ++iter)
				{
					if( false == CheckCanEnterLevel(static_cast<int>((*iter)->sLevel), ModeNo) )
					{//입장레벨이 맞지 않음
						lwAddWarnDataTT(799938);
						return false;
					}
				}
			}
			else
			{
				lwAddWarnDataTT(2);
				return false;
			}
		}
		else
		{
			if( false == CheckCanEnterLevel(MyLevel, ModeNo) )
			{//입장레벨이 맞지 않음
				lwAddWarnDataTT(200106);
				return false;
			}
		}

		PgInventory * pInv = pPlayer->GetInven();
		if( !pInv )
		{
			return false;
		}

		// 입장 아이템 체크
		ContHaveItemNoCount ContHaveItemNoCount;
		if( S_OK == pInv->GetItems(UICT_RAGNAROK_ENTER, ContHaveItemNoCount ) )
		{// 한 종류의 입장아이템으로 여러개의 던전 입장검사해야 되기 때문에, 입장 아이템 보유여부만 검사한다.
			if( false == FindCousumItem(ContHaveItemNoCount) )
			{
				lwAddWarnDataTT(310009);
				return false;
			}
		}
		else
		{
			lwAddWarnDataTT(310009);
			return false;
		}
		return true;
	}

	bool CheckCanEnterLevel(int const PlayerLevel, int const ModeNo)
	{
		return g_kRagnarokGroundEnterUIMgr.CheckCanEnterLevel( PlayerLevel, ModeNo );
	}
	
	bool FindCousumItem(ContHaveItemNoCount & ContItemNoCount)
	{
		GET_DEF(CItemDefMgr, ItemDefMgr);

		ContHaveItemNoCount::const_iterator itor = ContItemNoCount.begin();
		while( ContItemNoCount.end() != itor )
		{
			int const iItemNo = itor->first;
			CItemDef const * pItemDef = ItemDefMgr.GetDef(iItemNo);
			if( pItemDef )
			{
				if( pItemDef->GetAbil(AT_USE_ITEM_CUSTOM_VALUE_1) )
				{//찾았다
					return true;
				}
			}
			++itor;
		}
		return false;
	}
}

//
SRagnarokGroundModeItem::SRagnarokGroundModeItem()
	: ClassNo(0), Cont_ItemNo()
{
}

SRagnarokGroundModeItem::SRagnarokGroundModeItem(SRagnarokGroundModeItem const& rhs)
	: ClassNo(rhs.ClassNo), Cont_ItemNo(rhs.Cont_ItemNo)
{
}

SRagnarokGroundModeItem::~SRagnarokGroundModeItem()
{
}

bool SRagnarokGroundModeItem::ParseXml(TiXmlElement const* pNode)
{
	if( !pNode )
	{
		return false;
	}
	ClassNo = PgStringUtil::SafeAtoi(pNode->Attribute("CLASS_NO"));
	int TempItemNo = 0;
	TempItemNo = PgStringUtil::SafeAtoi(pNode->Attribute("ITEM_1")); Cont_ItemNo.push_back(TempItemNo);
	TempItemNo = PgStringUtil::SafeAtoi(pNode->Attribute("ITEM_2")); Cont_ItemNo.push_back(TempItemNo);
	TempItemNo = PgStringUtil::SafeAtoi(pNode->Attribute("ITEM_3")); Cont_ItemNo.push_back(TempItemNo);
	TempItemNo = PgStringUtil::SafeAtoi(pNode->Attribute("ITEM_4")); Cont_ItemNo.push_back(TempItemNo);
	TempItemNo = PgStringUtil::SafeAtoi(pNode->Attribute("ITEM_5")); Cont_ItemNo.push_back(TempItemNo);
	TempItemNo = PgStringUtil::SafeAtoi(pNode->Attribute("ITEM_6")); Cont_ItemNo.push_back(TempItemNo);
	TempItemNo = PgStringUtil::SafeAtoi(pNode->Attribute("ITEM_7")); Cont_ItemNo.push_back(TempItemNo);
	TempItemNo = PgStringUtil::SafeAtoi(pNode->Attribute("ITEM_8")); Cont_ItemNo.push_back(TempItemNo);
	TempItemNo = PgStringUtil::SafeAtoi(pNode->Attribute("ITEM_9")); Cont_ItemNo.push_back(TempItemNo);
	return true;
}
//
SRagnarokGroundMode::SRagnarokGroundMode()
: TitleTextID(0)
, MinLevel(0)
, MaxLevel(0)
, BtnImgName()
, ContItem()
, DefaultItem()
{
}

SRagnarokGroundMode::SRagnarokGroundMode(SRagnarokGroundMode const& rhs)
: TitleTextID(rhs.TitleTextID)
, MinLevel(rhs.MinLevel)
, MaxLevel(rhs.MaxLevel)
, BtnImgName(rhs.BtnImgName)
, ContItem(rhs.ContItem)
, DefaultItem(rhs.DefaultItem)
{
}

bool SRagnarokGroundMode::ParseXml(TiXmlElement const* pNode)
{
	if( !pNode )
	{
		return false;
	}
	TitleTextID = PgStringUtil::SafeAtoi(pNode->Attribute("TITLE_NO"));
	MinLevel = PgStringUtil::SafeAtoi(pNode->Attribute("MIN_LEVEL"));
	MaxLevel = PgStringUtil::SafeAtoi(pNode->Attribute("MAX_LEVEL"));
	BtnImgName = PgStringUtil::SafeUni(pNode->Attribute("BTN_IMG"));
	bool bRet = true;
	TiXmlElement const* pChild = pNode->FirstChildElement();
	while( pChild )
	{
		char const* szName = pChild->Value();
		if( 0 == strcmp(szName, "CLASS_ITEM") )
		{
			CONT_RAGNAROK_GROUND_MODE_ITEM::value_type NewElement;
			bRet = NewElement.ParseXml(pChild) && bRet;
			ContItem.push_back( NewElement );
		}
		else if( 0 == strcmp(szName, "DEFAULT_CLASS_ITEM") )
		{
			bRet = DefaultItem.ParseXml(pChild) && bRet;
		}
		else
		{
			bRet = false;
		}
		pChild = pChild->NextSiblingElement();
	}
	return bRet;
}

SRagnarokGroundModeItem SRagnarokGroundMode::GetItem() const
{
	PgPlayer* pPlayer = g_kPilotMan.GetPlayerUnit();
	if( pPlayer )
	{
		CONT_RAGNAROK_GROUND_MODE_ITEM::const_iterator iter = ContItem.begin();
		while( ContItem.end() != iter )
		{
			CONT_RAGNAROK_GROUND_MODE_ITEM::value_type const& Item = (*iter);
			if( Item.ClassNo == pPlayer->GetAbil(AT_CLASS) )
			{
				return Item;
			}
			++iter;
		}
	}
	return DefaultItem;
}

SRagnarokGroundEnterUI::SRagnarokGroundEnterUI()
	: TitleTextNo(0), ContentTextNo(0), GroundKey(0)
{
}

SRagnarokGroundEnterUI::SRagnarokGroundEnterUI(SRagnarokGroundEnterUI const& rhs)
	: TitleTextNo(rhs.TitleTextNo), ContentTextNo(rhs.ContentTextNo), GroundKey(rhs.GroundKey)
{
	m_mode[0] = rhs.m_mode[0];
	m_mode[1] = rhs.m_mode[1];
	m_mode[2] = rhs.m_mode[2];
}

bool SRagnarokGroundEnterUI::ParseXml(TiXmlElement const* pNode)
{
	if( !pNode )
	{
		return false;
	}
	bool bRet = true;
	TitleTextNo = PgStringUtil::SafeAtoi(pNode->Attribute("TITLE_NO"));
	ContentTextNo = PgStringUtil::SafeAtoi(pNode->Attribute("CONTENT_NO"));
	GroundKey = PgStringUtil::SafeAtoi(pNode->Attribute("GROUND_KEY"));
	TiXmlElement const* pChild = pNode->FirstChildElement();
	while( pChild )
	{
		char const* szName = pChild->Value();
		if( 0 == strcmp("MODE_1", szName) )
		{
			bRet = m_mode[ERagGroundMode::RGM_NORMAL].ParseXml(pChild) && bRet;
		}
		else if( 0 == strcmp("MODE_2", szName) )
		{
			bRet = m_mode[ERagGroundMode::RGM_RARE].ParseXml(pChild) && bRet;
		}
		else if( 0 == strcmp("MODE_3", szName) )
		{
			bRet = m_mode[ERagGroundMode::RGM_HERO].ParseXml(pChild) && bRet;
		}
		else
		{
			bRet = false;
		}
		pChild = pChild->NextSiblingElement();
	}
	return bRet;
}

PgRagnarokGroundEnterUIMgr::PgRagnarokGroundEnterUIMgr()
	: m_GroundKey(0), m_ContEnter()
{
}
PgRagnarokGroundEnterUIMgr::~PgRagnarokGroundEnterUIMgr()
{
}
void PgRagnarokGroundEnterUIMgr::Clear()
{
	m_GroundKey = 0;
	m_TriggerName.clear();
	m_ContEnter.clear();
}
bool PgRagnarokGroundEnterUIMgr::ParseXml(const TiXmlNode *pNode, void *pArg, bool bUTF8)
{
	if( !pNode )
	{
		return false;
	}
	if( 0 != strcmp(XML_ELEMENT_RAGNAROK_GROUND_ENTER_SETTING, pNode->Value()) )
	{
		return false;
	}
	bool bRet = true;
	TiXmlElement const* pChild = pNode->FirstChildElement("RAGNAROK_GROUND");
	while( pChild )
	{
		CONT_RAGNAROK_GROUND_ENTER::mapped_type NewElement;
		bRet = NewElement.ParseXml(pChild) && bRet;
		m_ContEnter.insert( std::make_pair(NewElement.GroundKey, NewElement) );
		pChild = pChild->NextSiblingElement();
	}
	return bRet;
}

void PgRagnarokGroundEnterUIMgr::Call(int const GroundKey, char const* TriggerName)
{
	CONT_RAGNAROK_GROUND_ENTER::const_iterator find_iter = m_ContEnter.find( GroundKey );
	if( m_ContEnter.end() == find_iter )
	{
		return;
	}

	m_GroundKey = GroundKey;
	if( TriggerName )
	{
		BM::vstring vStr;
		vStr = TriggerName ;
		m_TriggerName = (std::wstring)vStr;
	}

	UpdateEnterUI((*find_iter).second);
}

bool PgRagnarokGroundEnterUIMgr::CheckCanEnterLevel(int const PlayerLevel, int const ModeNo)
{
	CONT_RAGNAROK_GROUND_ENTER::const_iterator find_iter = m_ContEnter.find( m_GroundKey );
	if( m_ContEnter.end() == find_iter )
	{
		return false;
	}

	CONT_RAGNAROK_GROUND_ENTER::mapped_type RagnarokGroundUI = find_iter->second;
	if( PlayerLevel < RagnarokGroundUI.m_mode[ModeNo].MinLevel )
	{
		return false;
	}
	if( RagnarokGroundUI.m_mode[ModeNo].MaxLevel )
	{
		if( PlayerLevel > RagnarokGroundUI.m_mode[ModeNo].MaxLevel )
		{
			return false;
		}
	}
	return true;
}

std::wstring PgRagnarokGroundEnterUIMgr::GetTriggerName() const
{
	return m_TriggerName;
}

void PgRagnarokGroundEnterUIMgr::UpdateEnterUI(CONT_RAGNAROK_GROUND_ENTER::mapped_type const& RagnarokGroundUI) const
{
	std::wstring const UIName(L"FRM_ENTER_RAGNAROK_GROUND");
	XUI::CXUI_Wnd* pTopWnd = XUIMgr.Get( UIName );
	if( !pTopWnd )
	{
		pTopWnd = XUIMgr.Call( UIName );
		if( !pTopWnd )
		{
			return;
		}
	}

	XUI::CXUI_Wnd * pTitleWnd = pTopWnd->GetControl(L"FRM_TITLE");
	if( pTitleWnd )
	{
		pTitleWnd->Text( TTW(RagnarokGroundUI.TitleTextNo) );
	}

	XUI::CXUI_Wnd * pContentWnd = pTopWnd->GetControl(L"FRM_RAGNAROKUI_CONTENT");
	if( pContentWnd )
	{
		pContentWnd->Text( TTW(RagnarokGroundUI.ContentTextNo) );
	}

	int MyLevel = 0;
	int MyClass = 0;
	PgPlayer * pPlayer = g_kPilotMan.GetPlayerUnit();
	if( pPlayer )
	{
		MyLevel = pPlayer->GetAbil(AT_LEVEL);
		MyClass = pPlayer->GetAbil(AT_CLASS);
	}

	XUI::CXUI_Wnd * pNormal = pTopWnd->GetControl(L"BTN_NORMAL");
	if( pNormal )
	{
		UpdateModeBtn( pNormal, MyLevel, ERagGroundMode::RGM_NORMAL, RagnarokGroundUI.m_mode[ERagGroundMode::RGM_NORMAL] );
	}

	XUI::CXUI_Wnd * pRare = pTopWnd->GetControl(L"BTN_RARE");
	if( pRare )
	{
		UpdateModeBtn( pRare, MyLevel, ERagGroundMode::RGM_RARE, RagnarokGroundUI.m_mode[ERagGroundMode::RGM_RARE] );
	}
	
	XUI::CXUI_Wnd * pHero = pTopWnd->GetControl(L"BTN_HERO");
	if( pHero )
	{
		UpdateModeBtn( pHero, MyLevel, ERagGroundMode::RGM_HERO, RagnarokGroundUI.m_mode[ERagGroundMode::RGM_HERO] );
	}
	
	UpdateModeItem( pTopWnd, MyClass, ERagGroundMode::RGM_NORMAL, RagnarokGroundUI.m_mode[ERagGroundMode::RGM_NORMAL] );
	UpdateModeItem( pTopWnd, MyClass, ERagGroundMode::RGM_RARE, RagnarokGroundUI.m_mode[ERagGroundMode::RGM_RARE] );
	UpdateModeItem( pTopWnd, MyClass, ERagGroundMode::RGM_HERO, RagnarokGroundUI.m_mode[ERagGroundMode::RGM_HERO] );
}

void PgRagnarokGroundEnterUIMgr::UpdateModeBtn(XUI::CXUI_Wnd * pWnd, int const MyLevel, ERagGroundMode const& Mode, SRagnarokGroundMode const& RagnarokGroundUI) const
{
	if( !pWnd )
	{
		return;
	}

	SUVInfo UVInfo;
	pWnd->Enable(true);

	XUI::CXUI_Wnd * pTitleWnd = pWnd->GetControl(L"FRM_TITLE");
	if( pTitleWnd )
	{
		pTitleWnd->Text( TTW(RagnarokGroundUI.TitleTextID) );
	}
	
	XUI::CXUI_Wnd * pLvWnd = pWnd->GetControl(L"FRM_LV");
	if( pLvWnd )
	{
		BM::vstring vStr;
		vStr = TTW(224);
		vStr += ".";
		vStr += RagnarokGroundUI.MinLevel;
		vStr += " ~ ";
		if( RagnarokGroundUI.MaxLevel )
		{
			vStr += TTW(224);
			vStr += ".";
			vStr += RagnarokGroundUI.MaxLevel;
		}
		pLvWnd->Text(vStr);

		UVInfo = pLvWnd->UVInfo();
		UVInfo.Index = 2;
		pLvWnd->UVInfo(UVInfo);
		pLvWnd->SetInvalidate();
		pLvWnd->Enable(true);

		bool bLevelOver = false;
		if( MyLevel < RagnarokGroundUI.MinLevel )
		{
			bLevelOver = true;
		}
		if( RagnarokGroundUI.MaxLevel )
		{
			if( RagnarokGroundUI.MaxLevel < MyLevel )
			{
				bLevelOver = true;
			}
		}
		if( bLevelOver )
		{//해당 입장레벨에 맞지 않으면 버튼을 비활성화 시킨다.
			UVInfo = pLvWnd->UVInfo();
			UVInfo.Index = 1;
			pLvWnd->UVInfo(UVInfo);
			pLvWnd->SetInvalidate();
			pLvWnd->Enable(false);	// 레벨표시 비활성화
			pWnd->Enable(false);	// 전체 버튼 비활성화
		}
	}
}

void PgRagnarokGroundEnterUIMgr::UpdateModeItem(XUI::CXUI_Wnd * pWnd, int const MyClass, ERagGroundMode const& Mode, SRagnarokGroundMode const& RagnarokGroundUI) const
{
	if( !pWnd )
	{
		return;
	}

	BM::vstring FormName(_T("FRM_ITEM_"));
	switch( Mode )
	{
	case ERagGroundMode::RGM_NORMAL:
		{
			FormName += _T("NORMAL");
		}break;
	case ERagGroundMode::RGM_RARE:
		{
			FormName += _T("RARE");
		}break;
	case ERagGroundMode::RGM_HERO:
		{
			FormName += _T("HERO");
		}break;
	}

	for( int i = 0; i < MAX_RAGNAROK_GROUND_ITEM; ++i )
	{
		BM::vstring vStr(FormName);
		vStr += i;
		XUI::CXUI_Wnd * pItemWnd = pWnd->GetControl(vStr);
		if( pItemWnd )
		{
			bool DefaultSetting = true;
			CONT_RAGNAROK_GROUND_MODE_ITEM::const_iterator iter = RagnarokGroundUI.ContItem.begin();
			for( ; iter != RagnarokGroundUI.ContItem.end(); ++iter )
			{
				if( iter->ClassNo == MyClass )
				{
					DefaultSetting = false;
					if( iter->Cont_ItemNo[i] )
					{
						pItemWnd->Visible( true );
						XUI::CXUI_Wnd * pIconWnd = pItemWnd->GetControl(L"IMG_ICON");
						if( pIconWnd )
						{
							SoulCraft::SetSimpleItemIcon(lwUIWnd(pIconWnd), iter->Cont_ItemNo[i]);
							lwUIWnd(pIconWnd).SetCustomData<int>(iter->Cont_ItemNo[i]);
						}
					}
					else
					{
						pItemWnd->Visible( false );
					}
				}
			}
			if( DefaultSetting )
			{
				if( RagnarokGroundUI.DefaultItem.Cont_ItemNo[i] )
				{
					pItemWnd->Visible( true );
					XUI::CXUI_Wnd * pIconWnd = pWnd->GetControl(L"IMG_ICON");
					if( pIconWnd )
					{
						SoulCraft::SetSimpleItemIcon(lwUIWnd(pIconWnd), iter->Cont_ItemNo[i]);
						lwUIWnd(pIconWnd).SetCustomData<int>(iter->Cont_ItemNo[i]);
					}
				}
				else
				{
					pItemWnd->Visible( false );
				}
			}
		}
	}
}