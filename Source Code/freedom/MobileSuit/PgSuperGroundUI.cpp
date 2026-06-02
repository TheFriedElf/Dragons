#include "stdafx.h"
#include "PgNetwork.h"
#include "Variant/PgStringUtil.h"
#include "PgSuperGroundUI.h"
#include "lwUI.h"
#include "PgPilotMan.h"
#include "PgLegendItemMakeHelpMng.h"
#include "PgEventScriptSystem.h"
#include "PgChatMgrClient.h"

namespace PgSuperGroundUI
{
	int iSuperGroundNo = 0;
	int iModeNo = 0;
	int iNowFloor = 0;
	size_t iFloorCount = 0;
	size_t iRemainMonsterCount = 0;

	bool IsCanUseFloorMoveItem(int const iItemCustomValue)
	{
		int const iCanUseMinFloor = 1;
		if( g_pkWorld
		&&	0 == (g_pkWorld->GetAttr() & GATTR_SUPER_GROUND) ) // 슈퍼 그라운드 내에서만 사용 가능
		{
			lwAddWarnDataTT(20027);//사용할 수 없는 장소입니다.
			return false;
		}
		int const iResultFloor = (iNowFloor + iItemCustomValue);
		if( 0 >= iResultFloor
		||	iFloorCount < iResultFloor ) // 1층 이상
		{
			lwAddWarnDataTT(799226);//사용할 수 없는 층 입니다.
			return false;
		}
		return true;
	}

	void Clear()
	{
		iSuperGroundNo = 0;
		iModeNo = 0;
		iNowFloor = 0;
		iFloorCount = 0;
		iRemainMonsterCount = 0;
	}
	void ReadFromMapMove(BM::Stream& rkPacket)
	{
		rkPacket.Pop(iSuperGroundNo);
		rkPacket.Pop(iModeNo);
		rkPacket.Pop(iNowFloor);
		rkPacket.Pop(iRemainMonsterCount);
		rkPacket.Pop(iFloorCount);

		++iNowFloor; // 1증가 (서버에선 Index로 사용)

		XUIMgr.Call( std::wstring(L"FRM_SUPER_GND_TITLE") );
		lua_tinker::call< void, int, int, bool >("Call_SuperGroundFloor", iSuperGroundNo, iNowFloor, false);
		lwOnCallCenterMiniMap(lwGUID(BM::GUID()));
		g_kSuperGroundEnterUIMgr.CallMiniItemHelp(iSuperGroundNo, iModeNo);

		XUI::CXUI_Wnd *pkMapNameWnd = XUIMgr.Call( std::wstring(L"FRM_SUPER_GROUND_NAME") );
		if( pkMapNameWnd )
		{
			pkMapNameWnd->AliveTime( 5000 );
		}
	}
	void ProcessMsg(BM::Stream::DEF_STREAM_TYPE const wPacketType, BM::Stream& rkPacket)
	{
		switch( wPacketType )
		{
			case NFY_M_C_REMAIN_MONSTER:
			{
				rkPacket.Pop(iRemainMonsterCount);
			}break;
		case NFY_M_C_REMAIN_TIME_MSEC: // 시간제한 시작/종료
			{
				DWORD dwRemainTimeMSec = 0;
				rkPacket.Pop(dwRemainTimeMSec);
				if( dwRemainTimeMSec )
				{
					// 시작
					lua_tinker::call< void, int >("CallSuperGroundBossTimer", static_cast< int >(dwRemainTimeMSec));
				}
				else
				{
					// 종료
					lua_tinker::call< void >("CloseSuperGroundBossTimer");
				}
			}break;
		case NFY_M_C_LIMITED_TIME_MSEC: // 시간제한으로 실패
			{
				// 실패 처리
				lua_tinker::call< void >("CloseSuperGroundBossTimer");
				lua_tinker::call< void >("CallSuperGroundFailedUI");
			}break;
		}
	}

	//
	void CallSuperGroundEnterUI(int const iSuperGroundNo, char const* szTriggerID)
	{
		g_kSuperGroundEnterUIMgr.Call(iSuperGroundNo, szTriggerID);
	}
	void ReqEnterSuperGorundMode(lwUIWnd kSelf, int const iModeNo)
	{
		XUI::CXUI_Wnd* pkBtnWnd = kSelf();
		if( !pkBtnWnd )
		{
			return;
		}
		BM::Stream kPacket;
		pkBtnWnd->GetCustomData(kPacket.Data());
		kPacket.PosAdjust();
		int iSuperGroundNo = 0;
		std::string kTriggerID;
		kPacket.Pop( iSuperGroundNo );
		kPacket.Pop( kTriggerID );


		BM::Stream kSendPacket(PT_C_M_REQ_TRIGGER_ACTION);
		kSendPacket.Push(kTriggerID);
		kSendPacket.Push(iModeNo);
		NETWORK_SEND(kSendPacket);
	}

	int GetRemainMonsterCount()
	{
		return static_cast< size_t >(iRemainMonsterCount);
	}
	void ReqExitSuperGorund()
	{
		if( false == g_kEventScriptSystem.IsNowActivate() )
		{
			lwCallCommonMsgYesNoBox(MB(TTW(402013)), lwPacket(), false, MBT_EXIT_SUPER_GROUND);
		}
		else
		{
			SChatLog kChatLog(CT_EVENT_SYSTEM);
			g_kChatMgrClient.AddMessage(799223, kChatLog, true);
		}
	}

	void NextSuperGroundItem(lwUIWnd kTopWnd, int const iIncVal)
	{
		XUI::CXUI_Wnd* pkTopWnd = kTopWnd();
		if( !pkTopWnd )
		{
			return;
		}

		int iItemNo = 0;
		int iStepNo = 0;
		BM::Stream kPacket;
		pkTopWnd->GetCustomData(kPacket.Data());
		kPacket.PosAdjust();
		kPacket.Pop(iItemNo);
		kPacket.Pop(iStepNo);
		g_kSuperGroundEnterUIMgr.CallMiniItemHelp(iSuperGroundNo, iModeNo, iItemNo, iIncVal);
	}
	void NextSuperGroundItemStep(lwUIWnd kTopWnd, int const iIncVal)
	{
		XUI::CXUI_Wnd* pkTopWnd = kTopWnd();
		if( !pkTopWnd )
		{
			return;
		}
		int iItemNo = 0;
		int iStepNo = 0;
		BM::Stream kPacket;
		pkTopWnd->GetCustomData(kPacket.Data());
		kPacket.PosAdjust();
		kPacket.Pop(iItemNo);
		kPacket.Pop(iStepNo);
		g_kLegendItemMakeHelpMng.CallMakeHelpUI(iItemNo, iStepNo+iIncVal);
	}
	void CallCurrentSuperGroundItemMiniHelp()
	{
		g_kSuperGroundEnterUIMgr.CallMiniItemHelp(iSuperGroundNo, iModeNo);
	}
	bool lwIsHaveItemHelp()
	{
		return g_kSuperGroundEnterUIMgr.IsHaveItemHelp(iSuperGroundNo, iModeNo);
	}
	
	bool RegisterWrapper(lua_State *pkState)
	{
		using namespace lua_tinker;
		//
		def(pkState, "GetSG_RemainMonsterCount", GetRemainMonsterCount);
		def(pkState, "ReqSG_Exit", ReqExitSuperGorund);
		def(pkState, "CallSuperGroundEnterUI", CallSuperGroundEnterUI);
		def(pkState, "ReqEnterSuperGorundMode", ReqEnterSuperGorundMode);

		def(pkState, "NextSuperGroundItem", NextSuperGroundItem);
		def(pkState, "NextSuperGroundItemStep", NextSuperGroundItemStep);
		def(pkState, "CallCurrentSuperGroundItemMiniHelp", CallCurrentSuperGroundItemMiniHelp);
		def(pkState, "IsHaveItemHelp", lwIsHaveItemHelp);
		return true;
	}
};


//
SSuperGroundModeItem::SSuperGroundModeItem()
	: iClass(0), kItemNo()
{
}
SSuperGroundModeItem::SSuperGroundModeItem(SSuperGroundModeItem const& rhs)
	: iClass(rhs.iClass), kItemNo(rhs.kItemNo)
{
}
SSuperGroundModeItem::~SSuperGroundModeItem()
{
}
bool SSuperGroundModeItem::ParseXml(TiXmlElement const* pkNode)
{
	if( !pkNode )
	{
		return false;
	}
	iClass = PgStringUtil::SafeAtoi(pkNode->Attribute("CLASS_NO"));
	int iTempItemNo = PgStringUtil::SafeAtoi(pkNode->Attribute("ITEM_1")); if(iTempItemNo) { kItemNo.push_back(iTempItemNo); }
	iTempItemNo = PgStringUtil::SafeAtoi(pkNode->Attribute("ITEM_2")); if(iTempItemNo) { kItemNo.push_back(iTempItemNo); }
	iTempItemNo = PgStringUtil::SafeAtoi(pkNode->Attribute("ITEM_3")); if(iTempItemNo) { kItemNo.push_back(iTempItemNo); }
	iTempItemNo = PgStringUtil::SafeAtoi(pkNode->Attribute("ITEM_4")); if(iTempItemNo) { kItemNo.push_back(iTempItemNo); }
	return true;
}
//
SSuperGroundMode::SSuperGroundMode()
	: iTitleTextID(0), kBtnImgName(), kContItem(), kDefaultItem()
{
}
SSuperGroundMode::SSuperGroundMode(SSuperGroundMode const& rhs)
	: iTitleTextID(rhs.iTitleTextID), kBtnImgName(rhs.kBtnImgName), kContItem(rhs.kContItem), kDefaultItem(rhs.kDefaultItem)
{
}
bool SSuperGroundMode::ParseXml(TiXmlElement const* pkNode)
{
	if( !pkNode )
	{
		return false;
	}
	iTitleTextID = PgStringUtil::SafeAtoi(pkNode->Attribute("TITLE"));
	kBtnImgName = PgStringUtil::SafeUni(pkNode->Attribute("BTN_IMG"));
	bool bRet = true;
	TiXmlElement const* pkChild = pkNode->FirstChildElement();
	while( pkChild )
	{
		char const* szName = pkChild->Value();
		if( 0 == strcmp(szName, "CLASS_ITEM") )
		{
			CONT_SUPER_GROUND_MODE_ITEM::value_type kNew;
			bRet = kNew.ParseXml(pkChild) && bRet;
			kContItem.push_back( kNew );
		}
		else if( 0 == strcmp(szName, "DEFAULT_CLASS_ITEM") )
		{
			bRet = kDefaultItem.ParseXml(pkChild) && bRet;
		}
		else
		{
			bRet = false;
		}
		pkChild = pkChild->NextSiblingElement();
	}
	return bRet;
}
SSuperGroundModeItem SSuperGroundMode::GetItem() const
{
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( pkPlayer )
	{
		CONT_SUPER_GROUND_MODE_ITEM::const_iterator iter = kContItem.begin();
		while( kContItem.end() != iter )
		{
			CONT_SUPER_GROUND_MODE_ITEM::value_type const& rkItem = (*iter);
			if( rkItem.iClass == pkPlayer->GetAbil(AT_CLASS) )
			{
				return rkItem;
			}
			++iter;
		}
	}
	return kDefaultItem;
}
SSuperGroundEnterUI::SSuperGroundEnterUI()
	: iTitleTextID(0), iDescTextID(0), iSuperGroundNo(0)
{
}
SSuperGroundEnterUI::SSuperGroundEnterUI(SSuperGroundEnterUI const& rhs)
	: iTitleTextID(rhs.iTitleTextID), iDescTextID(rhs.iDescTextID), iSuperGroundNo(rhs.iSuperGroundNo)
{
	kMode[0] = rhs.kMode[0];
	kMode[1] = rhs.kMode[1];
	kMode[2] = rhs.kMode[2];
}

bool SSuperGroundEnterUI::ParseXml(TiXmlElement const* pkNode)
{
	if( !pkNode )
	{
		return false;
	}
	bool bRet = true;
	iTitleTextID = PgStringUtil::SafeAtoi(pkNode->Attribute("TITLE"));
	iDescTextID = PgStringUtil::SafeAtoi(pkNode->Attribute("DESC"));
	iSuperGroundNo = PgStringUtil::SafeAtoi(pkNode->Attribute("SUPER_GROUND_NO"));
	TiXmlElement const* pkChild = pkNode->FirstChildElement();
	while( pkChild )
	{
		char const* szName = pkChild->Value();
		if( 0 == strcmp("MODE_1", szName) )
		{
			bRet = kMode[0].ParseXml(pkChild) && bRet;
		}
		else if( 0 == strcmp("MODE_2", szName) )
		{
			bRet = kMode[1].ParseXml(pkChild) && bRet;
		}
		else if( 0 == strcmp("MODE_3", szName) )
		{
			bRet = kMode[2].ParseXml(pkChild) && bRet;
		}
		else
		{
			bRet = false;
		}
		pkChild = pkChild->NextSiblingElement();
	}
	return bRet;
}

//
PgSuperGroundEnterUIMgr::PgSuperGroundEnterUIMgr()
	: m_kContEnter()
{
}
PgSuperGroundEnterUIMgr::~PgSuperGroundEnterUIMgr()
{
}
void PgSuperGroundEnterUIMgr::Clear()
{
	m_kContEnter.clear();
}
bool PgSuperGroundEnterUIMgr::ParseXml(const TiXmlNode *pkNode, void *pArg, bool bUTF8)
{
	if( !pkNode )
	{
		return false;
	}
	if( 0 != strcmp(XML_ELEMENT_SUPER_GROUND_ENTER_SETTING, pkNode->Value()) )
	{
		return false;
	}
	bool bRet = true;
	TiXmlElement const* pkChild = pkNode->FirstChildElement("SUPER_GROUND");
	while( pkChild )
	{
		CONT_SUPER_GROUND_ENTER::mapped_type kNew;
		bRet = kNew.ParseXml(pkChild) && bRet;
		m_kContEnter.insert( std::make_pair(kNew.iSuperGroundNo, kNew) );
		pkChild = pkChild->NextSiblingElement();
	}
	return bRet;
}
void PgSuperGroundEnterUIMgr::Call(int const iSuperGroundNo, std::string const& rkTriggerID) const
{
	CONT_SUPER_GROUND_ENTER::const_iterator find_iter = m_kContEnter.find( iSuperGroundNo );
	if( m_kContEnter.end() == find_iter )
	{
		return;
	}

	UpdateEnterUI((*find_iter).second, rkTriggerID);
}
void PgSuperGroundEnterUIMgr::UpdateEnterUI(CONT_SUPER_GROUND_ENTER::mapped_type const& rkSuperGroundUI, std::string const& rkTriggerID) const
{
	std::wstring const kUIName(L"FRM_ENTER_SUPER_GROUND");
	XUI::CXUI_Wnd* pkTopWnd = XUIMgr.Get( kUIName );
	if( !pkTopWnd )
	{
		pkTopWnd = XUIMgr.Call(kUIName);
		if( !pkTopWnd )
		{
			return;
		}
	}

	BM::Stream kPacket;
	kPacket.Push( rkSuperGroundUI.iSuperGroundNo );
	kPacket.Push( rkTriggerID );
	pkTopWnd->SetCustomData(kPacket.Data());

	XUI::CXUI_Wnd* pkTitleWnd = pkTopWnd->GetControl( std::wstring(L"FRM_BG_01") );
	if( pkTitleWnd )
	{
		pkTitleWnd->Text( TTW(rkSuperGroundUI.iTitleTextID) );
	}
	XUI::CXUI_Wnd* pkDescWnd = pkTopWnd->GetControl( std::wstring(L"FRM_BG_02") );
	if( pkDescWnd )
	{
		XUI::CXUI_Wnd* pkTextWnd = pkDescWnd->GetControl( std::wstring(L"FRM_TEXT") );
		if( pkTextWnd )
		{
			pkTextWnd->Text( TTW(rkSuperGroundUI.iDescTextID) );
		}
	}
	for( int iCur = 0; MAX_SUPER_GROUND_MODE_NO > iCur; ++iCur )
	{
		UpdateModeBtn(pkTopWnd, iCur, rkSuperGroundUI.kMode[iCur], kPacket);
	}
	lwRegistUIAction(lwUIWnd(pkTopWnd), "CloseUI");
}
void PgSuperGroundEnterUIMgr::UpdateModeBtn(XUI::CXUI_Wnd* pkTopWnd, int const iModeNo, SSuperGroundMode const& rkMode, BM::Stream const& rkCustomData) const
{
	XUI::CXUI_Wnd* pkBtnWnd = pkTopWnd->GetControl( std::wstring(BM::vstring(L"BTN_MODE_") + iModeNo) );
	if( pkBtnWnd )
	{
		pkBtnWnd->Text(TTW(rkMode.iTitleTextID));
		pkBtnWnd->SetCustomData(rkCustomData.Data());
		XUI::CXUI_Button* pkButtonWnd = dynamic_cast< XUI::CXUI_Button* >(pkBtnWnd);
		if( pkButtonWnd
		&&	pkButtonWnd->BtnImgName() != rkMode.kBtnImgName )
		{
			pkButtonWnd->ButtonChangeImage(rkMode.kBtnImgName);
		}
	}
	XUI::CXUI_Wnd* pkSubFrmWnd = pkTopWnd->GetControl( std::wstring(BM::vstring(L"FRM_BTN_GROUP_") + iModeNo) );
	if( pkSubFrmWnd )
	{
		SSuperGroundModeItem const kItem = rkMode.GetItem();
		for( size_t iCur = 0; MAX_SUPER_GROUND_ITEM > iCur; ++iCur )
		{
			XUI::CXUI_Wnd* pkItemWnd = pkSubFrmWnd->GetControl( std::wstring(BM::vstring(L"BTN_ITEM_") + iCur) );
			if( pkItemWnd )
			{
				int const iItemNo = (kItem.kItemNo.size() > iCur)? kItem.kItemNo.at(iCur): 0;
				XUI::CXUI_Wnd* pkImgWnd = pkItemWnd->GetControl( std::wstring(L"FRM_ICON") );
				if( pkImgWnd )
				{
					SoulCraft::SetSimpleItemIcon(lwUIWnd(pkImgWnd), iItemNo);
					lwUIWnd(pkImgWnd).SetCustomData<int>(iItemNo);
				}
				pkItemWnd->Visible( (0 != iItemNo) );
				lwUIWnd(pkItemWnd).SetCustomData<int>(iItemNo);
			}
		}
	}
}
void PgSuperGroundEnterUIMgr::CallMiniItemHelp(int const iSuperGroundNo, int const iModeNo, int iItemNo, int iIncVal) const
{
	if( 0 > iModeNo
	||	MAX_SUPER_GROUND_MODE_NO <= iModeNo )
	{
		return;
	}
	CONT_SUPER_GROUND_ENTER::const_iterator iter = m_kContEnter.find(iSuperGroundNo);
	if( m_kContEnter.end() == iter )
	{
		return;
	}
	CONT_SUPER_GROUND_ENTER::mapped_type const& rkSuperGroundEnterInfo = (*iter).second;
	SSuperGroundMode const& rkMode = rkSuperGroundEnterInfo.kMode[iModeNo];
	SSuperGroundModeItem const kItem = rkMode.GetItem();
	CONT_ITEMNO::const_iterator loop_iter = kItem.kItemNo.end();
	if( 0 == iItemNo )
	{
		loop_iter = kItem.kItemNo.begin();
		iItemNo = (*loop_iter);
	}
	else
	{
		loop_iter = std::find(kItem.kItemNo.begin(), kItem.kItemNo.end(), iItemNo);
	}
	if( kItem.kItemNo.end() == loop_iter )
	{
		return;
	}
	if( 0 != iIncVal )
	{
		if( 0 < iIncVal )
		{
			++loop_iter;
			if( kItem.kItemNo.end() == loop_iter )
			{
				loop_iter = kItem.kItemNo.begin();
			}
		}
		else
		{
			if( kItem.kItemNo.begin() == loop_iter )
			{
				loop_iter = --kItem.kItemNo.end();
			}
			else
			{
				--loop_iter;
			}
		}
		iItemNo = (*loop_iter);
	}
	int const iInitStepNo = 1;
	bool const bRet = g_kLegendItemMakeHelpMng.CallMakeHelpUI(iItemNo, iInitStepNo);
	if( false == bRet
	&&	0 != iIncVal )
	{
		CallMiniItemHelp(iSuperGroundNo, iModeNo, iItemNo, iIncVal);
	}
}
bool PgSuperGroundEnterUIMgr::IsHaveItemHelp(int const iSuperGroundNo, int const iModeNo, int iItemNo, int iIncVal) const
{
	if( 0 > iModeNo
	||	MAX_SUPER_GROUND_MODE_NO <= iModeNo )
	{
		return false;
	}
	CONT_SUPER_GROUND_ENTER::const_iterator iter = m_kContEnter.find(iSuperGroundNo);
	if( m_kContEnter.end() == iter )
	{
		return false;
	}
	CONT_SUPER_GROUND_ENTER::mapped_type const& rkSuperGroundEnterInfo = (*iter).second;
	SSuperGroundMode const& rkMode = rkSuperGroundEnterInfo.kMode[iModeNo];
	SSuperGroundModeItem const kItem = rkMode.GetItem();
	CONT_ITEMNO::const_iterator loop_iter = kItem.kItemNo.end();
	if( 0 == iItemNo )
	{
		loop_iter = kItem.kItemNo.begin();
		iItemNo = (*loop_iter);
	}
	else
	{
		loop_iter = std::find(kItem.kItemNo.begin(), kItem.kItemNo.end(), iItemNo);
	}
	if( kItem.kItemNo.end() == loop_iter )
	{
		return false;
	}
	if( 0 != iIncVal )
	{
		if( 0 < iIncVal )
		{
			++loop_iter;
			if( kItem.kItemNo.end() == loop_iter )
			{
				loop_iter = kItem.kItemNo.begin();
			}
		}
		else
		{
			if( kItem.kItemNo.begin() == loop_iter )
			{
				loop_iter = --kItem.kItemNo.end();
			}
			else
			{
				--loop_iter;
			}
		}
		iItemNo = (*loop_iter);
	}
	return (0 != g_kLegendItemMakeHelpMng.GetStepSize(iItemNo));
}