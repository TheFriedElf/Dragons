#include "Stdafx.h"
#include "Lohengrin/GameTime.h"
#include "Variant/PgStringUtil.h"
#include "ServerLib.h"
#include "PgNetwork.h"
#include "PgGuild.h"
#include "PgChatMgrClient.h"
#include "PgPilot.h"
#include "PgPilotMan.h"
#include "PgWorld.h"
#include "PgActor.h"
#include "lwUI.h"
#include "PgOption.h"
#include "lwGuildUI.h"
#include "PgQuestMan.h"
#include "lwQuestMan.h"
#include "PgParticleMan.h"
#include "PgContentsBase.h"
#include "PgEmporiaMgr.h"
#include "PgCommandMgr.h"
#include "lwWorld.h"
#include "lwUIQuest.h"
#include "PgDailyQuestui.h"
#include "PgMToMChat.h"

extern bool BreakMoney_WStr(__int64 const iTotal, VEC_WSTRING &rkVec);

namespace PgGuildMgrUtil
{
	int const iMsgOnlyMaster = 700080;
	int const iUseOnlyOwner = 400854;
	std::wstring const kChangeGuildMarkUIName( _T("FRM_GUILD_EMBLEM_SELECT1") );


	inline bool IsCanReqInOutState(int const iGroundAttr)
	{
		if( GATTR_PVP == iGroundAttr
		||	GATTR_EMPORIABATTLE == iGroundAttr )
		{
			return false;
		}
		return true;
	}

	class PgUpdateGuildName : public PgPickObjectNullChecker
	{
	public:
		PgUpdateGuildName(BM::GUID const& rkGuildGuid)
			: m_kGuildGuid(rkGuildGuid)
		{
		}

		virtual ~PgUpdateGuildName() {};
		bool Test(PgIWorldObject* pkObject)
		{
			if( !pkObject )
			{
				return false;
			}

			PgActor *pkActor = dynamic_cast<PgActor*>(pkObject);
			if( pkActor )
			{
				PgPilot *pkPilot = pkActor->GetPilot();
				if( pkPilot )
				{
					PgPlayer* pkPlayer = dynamic_cast<PgPlayer*>(pkPilot->GetUnit());
					if( pkPlayer
					&&	m_kGuildGuid == pkPlayer->GuildGuid() )
					{
						pkActor->UpdateName();
					}
				}
			}

			return false;
		}
	private:
		BM::GUID const& m_kGuildGuid;
	};

	void AvgBreakColor(NiColorA& kColor, size_t const &iTempColor)
	{
		float const fLimitAvg = 0.063654f;
		float const fAvgR = abs(((kColor.g-kColor.r) + (kColor.b-kColor.r))/2.f);
		float const fAvgG = abs(((kColor.r-kColor.g) + (kColor.b-kColor.g))/2.f);
		float const fAvgB = abs(((kColor.r-kColor.b) + (kColor.g-kColor.b))/2.f);

		float fAvgAll = (fAvgR+fAvgG+fAvgB)/3.f;
		if( fLimitAvg > fAvgAll )
		{
			int const iCur = iTempColor % 3;
			switch(iCur)
			{
			case 0:{kColor.r += fAvgAll;}break;
			case 1:{kColor.g += fAvgAll;}break;
			case 2:{kColor.b += fAvgAll;}break;
			default:{assert(0);}break;
			}
		}
	}

	void AutoFloatMinMax(float const &fCur, float& fOut)
	{
		float const fMax = 0.867f, fMin = 0.75171f;
		float const fCutMin = 0.24829f;

		if( fCutMin > fCur )
		{
			fOut = fCur + fMin;
		}
		else if( fMin > fCur )
		{
			fOut = fMin;
		}
		else if( fMax < fCur )
		{
			fOut = fMax;
		}
	}

	BYTE ColorAtomToByte(float fAtom)
	{
		return static_cast< BYTE >(UCHAR_MAX * fAtom);
	}

	void Color32ToStr(BYTE const cA, BYTE const cR, BYTE const cG, BYTE const cB, std::wstring& rkOut)
	{
		wchar_t szTemp[MAX_PATH] = {0, };
		_stprintf_s(szTemp, L"0x%2X%2X%2X%2X", cA, cR, cG, cB);
		rkOut = std::wstring(szTemp);
	}

	void NiColorAToStr(NiColorA const& rkColor, std::wstring& rkOut)
	{
		Color32ToStr(ColorAtomToByte(rkColor.a), ColorAtomToByte(rkColor.r), ColorAtomToByte(rkColor.g), ColorAtomToByte(rkColor.b), rkOut);
	}

	void CalcSizetToColor(size_t const iTempColor, NiColorA& rkOutColor)
	{
		float fR = ((iTempColor&0x00FF0000)>>16) / (float)0xFF;
		float fG = ((iTempColor&0x0000FF00)>>8) / (float)0xFF;
		float fB = (iTempColor&0x000000FF) / (float)0xFF;

		AutoFloatMinMax(fR, fR);
		AutoFloatMinMax(fG, fG);
		AutoFloatMinMax(fB, fB);

		rkOutColor = NiColorA(fR, fG, fB, 1.0f);

		AvgBreakColor(rkOutColor, iTempColor);
	}

	void CalcGuidToColor(BM::GUID const &rkGuid, NiColorA& rkOutColor)
	{
		size_t const iTempColor = BM::CGuid_hash_compare()(rkGuid);
		CalcSizetToColor(iTempColor, rkOutColor);
	}

	void CalcSizetToColor(size_t const iSize, std::wstring& rkOut)
	{
		NiColorA kOutColor;
		CalcSizetToColor(iSize, kOutColor);
		NiColorAToStr(kOutColor, rkOut);
		rkOut = (BM::vstring(L"{C=")<<rkOut<< L"/}").operator const std::wstring &();
	}


	void CallGuildChangeMark1(int const iItemNo, SItemPos const& rkItemInvPos)
	{
		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( !pkPlayer )
		{
			return;
		}

		if( !g_kGuildMgr.IamHaveGuild()
		||	!g_kGuildMgr.IsOwner(pkPlayer->GetID()) )
		{
			g_kChatMgrClient.LogMsgBox(iUseOnlyOwner);
			return;
		}

		CXUI_Wnd* pkTopWnd = XUIMgr.Get(kChangeGuildMarkUIName);
		if( !pkTopWnd )
		{
			pkTopWnd = XUIMgr.Call(kChangeGuildMarkUIName);
			if( !pkTopWnd )
			{
				return;
			}
		}

		BM::Stream kPacket;
		kPacket.Push( iItemNo );
		kPacket.Push( rkItemInvPos );

		pkTopWnd->SetCustomData(kPacket.Data());
	}

	int GetMaxGuildMaxCount()
	{
		return cMaxGuildEmblem;
	}

	bool CallChangeMarkYesNo(lwUIWnd kTopWnd, BYTE const cNewGuildMark)
	{
		CXUI_Wnd* pkTopWnd = kTopWnd();
		if( !pkTopWnd )
		{
			return false;
		}

		BM::Stream kCustomData;
		int iItemNo = 0;
		SItemPos kItemInvPos;

		pkTopWnd->GetCustomData( kCustomData.Data() );
		pkTopWnd->ClearCustomData();
		kCustomData.PosAdjust();

		kCustomData.Pop( iItemNo );
		kCustomData.Pop( kItemInvPos );

		if( 0 == iItemNo )
		{
			return false;
		}

		if( cMaxGuildEmblem <= cNewGuildMark )
		{
			return false;
		}

		if( g_kGuildMgr.Emblem() == cNewGuildMark )
		{
			return false;
		}

		BM::GUID kMyCharGuid;
		if( !g_kPilotMan.GetPlayerPilotGuid(kMyCharGuid) )
		{
			return false;
		}

		if( !g_kGuildMgr.IamHaveGuild() )
		{
			return false;
		}

		if( !g_kGuildMgr.IsOwner(kMyCharGuid) )
		{
			return false;
		}

		XUI::CXUI_MsgBox* pMsgBox = dynamic_cast<XUI::CXUI_MsgBox*>(XUIMgr.Call(std::wstring(_T("OK_CANCEL_GUILD_MARK_CHANGE1"))));
		if( !pMsgBox )
		{
			return false;
		}
		BM::Stream kNewCustomData;
		kNewCustomData.Push( iItemNo );
		kNewCustomData.Push( kItemInvPos );
		kNewCustomData.Push( cNewGuildMark );

		XUI::CXUI_Wnd* pkImgWnd = dynamic_cast<XUI::CXUI_Image*>( pMsgBox->GetControl( std::wstring(_T("IMG_MARK")) ) );
		if( !pkImgWnd )
		{
			return false;
		}

		SUVInfo kUVInfo = pkImgWnd->UVInfo();
		kUVInfo.Index = cNewGuildMark + 1;
		pkImgWnd->UVInfo( kUVInfo );

		XUI::PgMessageBoxInfo Info = pMsgBox->BoxInfo();
		Info.iBoxType = MBT_CHANGE_GUILD_MARK1;
		Info.kOrderGuid = kMyCharGuid;
		pMsgBox->BoxInfo(Info);
		pMsgBox->SetCustomData( kNewCustomData.Data() );
		return true;
	}

	void SendReqChangeGuildMark(CXUI_Wnd* pkTopWnd)
	{
		if( !pkTopWnd )
		{
			return;
		}

		BM::Stream kCustomData;
		int iItemNo = 0;
		SItemPos kItemInvPos;
		BYTE cNewGuildMark = 0;

		pkTopWnd->GetCustomData( kCustomData.Data() );
		pkTopWnd->ClearCustomData();
		kCustomData.PosAdjust();

		kCustomData.Pop( iItemNo );
		kCustomData.Pop( kItemInvPos );
		kCustomData.Pop( cNewGuildMark );

		if( 0 == iItemNo )
		{
			return;
		}

		if( cMaxGuildEmblem <= cNewGuildMark )
		{
			return;
		}

		if( g_kGuildMgr.Emblem() == cNewGuildMark )
		{
			return;
		}

		if( SItemPos(0,0) == kItemInvPos )
		{
			return;
		}

		BM::Stream kPacket( PT_C_N_REQ_GUILD_COMMAND );
		kPacket.Push( static_cast< BYTE >(GC_M_AuthOwner) );
		kPacket.Push( static_cast< BM::Stream::DEF_STREAM_TYPE >(PT_C_M_REQ_ITEM_ACTION) );
		kPacket.Push( kItemInvPos );
		kPacket.Push( lwGetServerElapsedTime32() );
		kPacket.Push( cNewGuildMark );
		NETWORK_SEND(kPacket);
	}
};


PgGuildMgr::PgGuildMgr()
{
	Clear();

	{
		//길드 추방당하면 Clear() 호출된다.
		// Clear() 안에 포함하지 않는다.
		m_kGuildEntranceApplicant.clear();
		m_kEntranceOpenGuild.clear();
		m_kSearchEntranceOpenGuild.clear();
		m_kApplicationState.Init();
		m_iRecommendGuildRank = 0;
		m_iGuildInvView = 0;
	}
	
}

PgGuildMgr::~PgGuildMgr()
{
}

void PgGuildMgr::Clear()
{
	ListMode(GLM_Grade);
	m_bWait = false;
	m_kBasicInfo.Clear();
	m_kEmporiaInfo.Clear();
	m_kMember.clear();
	m_kMemberGrade.clear();
	m_kOtherInfo.clear();
	m_kRequester.clear();

	m_kTempGuildName.clear();
	m_kTempGuildEmblem = -1;

	m_kGuildSkill.Init();
	
	m_kInv.Clear();
	m_kInv.Init(m_kBasicInfo.abyInvExtern, m_kBasicInfo.abyInvExternIdx);

	g_kChatMgrClient.Name2Guid_Remove(CT_GUILD);

	m_kGuildNotice.clear();

	m_kApplicationState.Init();
	ClearEntranceApplicant();
}


bool PgGuildMgr::PreCreateGuild(std::wstring &rkGuildName)
{
	if( !rkGuildName.size() )
	{
		return false;
	}

	if( g_kClientFS.Filter(rkGuildName, false, FST_ALL) 
		|| !g_kUnicodeFilter.IsCorrect(UFFC_GUILD_NAME, rkGuildName)
		|| g_kChatMgrClient.CheckChatTag(rkGuildName)
		)
	{
		int const iTTWBadGuildName = 700106;
		//CallGuildNpcFunctionUI(QDT_NpcTalk, iTTWBadGuildName);
		SChatLog kChatLog(CT_EVENT);
		g_kChatMgrClient.AddMessage(iTTWBadGuildName, kChatLog, true);
		return false;
	}

	m_kTempGuildName = rkGuildName;//임시
	BM::Stream kPacket(PT_C_N_REQ_GUILD_COMMAND, (BYTE)GC_PreCreate);
	kPacket.Push(m_kTempGuildName);
	NETWORK_SEND(kPacket)
	return true;
}

bool PgGuildMgr::IsHaveEntranceGuild() const
{
	return m_kApplicationState.kGuildGuid.IsNotNull();
}

bool PgGuildMgr::CreateGuild()
{
	if( !m_kTempGuildName.size() )
	{
		return false;
	}

	if( 0 > m_kTempGuildEmblem )
	{
		return false;
	}

	BM::Stream kPacket(PT_C_N_REQ_GUILD_COMMAND, (BYTE)GC_Create);
	kPacket.Push(m_kTempGuildName);
	kPacket.Push((BYTE)m_kTempGuildEmblem);
	NETWORK_SEND(kPacket)
	return true;
}

bool PgGuildMgr::DestroyGuild()
{
	if( IamHaveGuild() )
	{
		BM::Stream kPacket(PT_C_N_REQ_GUILD_COMMAND, (BYTE)GC_M_Destroy);
		kPacket.Push(m_kBasicInfo.kGuildGuid);
		NETWORK_SEND(kPacket)
		return true;
	}
	return false;
}

bool PgGuildMgr::LeaveGuild()
{
	if( !IamHaveGuild() )//
	{
		return false;
	}

	BM::GUID kMyCharGuid;
	bool const bFindPilot = g_kPilotMan.GetPlayerPilotGuid(kMyCharGuid);
	if( !bFindPilot
	||	IsOwner(kMyCharGuid) )
	{
		return false;//오너는 안되
	}

	CallYesNoMsgBox(TTW(400619), BM::GUID::NullData(), MBT_CONFIRM_GUILD_LEAVE);
	return true;
}

bool PgGuildMgr::KickUser(BM::GUID const &rkCharGuid)
{
	if( !IamHaveGuild() )//
	{
		return false;
	}

	BM::GUID kMyCharGuid;
	bool const bFindPilot = g_kPilotMan.GetPlayerPilotGuid(kMyCharGuid);
	if( !bFindPilot
	&&	!IsMaster(kMyCharGuid) )
	{
		return false;//마스터 아님 안되
	}

	if( kMyCharGuid == rkCharGuid )
	{
		SChatLog kChatLog(CT_EVENT);
		g_kChatMgrClient.AddMessage(700081, kChatLog, true);
		return false;//나는 안되
	}

	SGuildMemberInfo kMemberInfo;
	if( !GetMemberByGuid(rkCharGuid, kMemberInfo) )
	{
		return false;
	}

	std::wstring kMessage;
	if( FormatTTW(kMessage, 400618, kMemberInfo.kCharName.c_str()) )
	{
		CallYesNoMsgBox(kMessage ,rkCharGuid, MBT_CONFIRM_GUILD_KICK_USER);
	}
	return true;
}

bool PgGuildMgr::ChangeOwner(BM::GUID const &rkNewOwnerGuid)
{
	BM::GUID kMyCharGuid;
	if( !g_kPilotMan.GetPlayerPilotGuid(kMyCharGuid) )
	{
		return false;
	}

	if( !IamHaveGuild() )
	{
		return false;
	}

	if( !IsOwner(kMyCharGuid) )
	{
		return false;
	}

	BM::Stream kPacket(PT_C_N_REQ_GUILD_COMMAND, (BYTE)GC_M_ChangeOwner);
	kPacket.Push( rkNewOwnerGuid );
	NETWORK_SEND(kPacket)
	return true;
}

//
bool PgGuildMgr::ReqOtherGuildInfo(BM::GUID const &rkGuildGuid, BM::GUID const &rkActorGuid)
{
	BM::CAutoMutex kLock(m_kMutex);
	if( !g_pkWorld )
	{
		return false;
	}
	if( PgWorldUtil::IsCharacterSelectStage() )
	{
		return false; // 캐릭터 선택창에선 보낼 수 없다.
	}
	
	if( !m_bWait
	&&	g_kPilotMan.IsMyPlayer(rkActorGuid)//내 캐릭터고
	&&	BM::GUID::NullData() == m_kBasicInfo.kGuildGuid )//내 길드 정보가 아직 없으면
	{
		m_bWait = true;
		ReqBasicInfo(rkGuildGuid);//Basic요청
		ReqMemberInfo(rkGuildGuid);//멤버요청
		ReqGuildEntranceApplicantList(rkGuildGuid);//길드가입 신청자 리스트
	}

	bool bNew = false;
	ContWaiter::iterator kWait_iter = m_kRequester.find(rkGuildGuid);
	if(m_kRequester.end() == kWait_iter)
	{//새로운 길드에 대기열 생성
		ContWaiter::mapped_type kElement;
		std::pair<ContWaiter::iterator, bool> ibRet = m_kRequester.insert(std::make_pair(rkGuildGuid, kElement));
		if(!ibRet.second)
		{
			return false;//실패
		}
		kWait_iter = ibRet.first;
		bNew = true;
	}
	ContWaiter::mapped_type& rkElement = kWait_iter->second;
	rkElement.insert(rkActorGuid);

	//
	if(bNew)
	{
		BM::Stream kPacket(PT_C_N_REQ_GUILD_COMMAND, (BYTE)GC_OtherInfo);
		kPacket.Push(rkGuildGuid);
		NETWORK_SEND(kPacket)
	}
	return true;//성공
}

bool PgGuildMgr::OtherGuildInfo(const SGuildOtherInfo& rkInfo)
{
	BM::CAutoMutex kLock(m_kMutex);
	ContWaiter::iterator kWait_iter = m_kRequester.find(rkInfo.kGuid);
	if(m_kRequester.end() != kWait_iter)
	{
		ContWaiter::mapped_type &rkElement = kWait_iter->second;
		ContWaiter::mapped_type::iterator kActor_iter = rkElement.begin();
		while(rkElement.end() != kActor_iter)
		{
			//여기서 Actor를 찾아서 처리한다.
			UpdateActorGuildName(*kActor_iter);

			kActor_iter = rkElement.erase(kActor_iter);//무조건 삭제
		}
		m_kRequester.erase(kWait_iter);
		return true;
	}
	return false;
}

bool PgGuildMgr::GetGuildInfo(BM::GUID const &rkGuildGuid, SGuildOtherInfo& rkOut) const
{
	BM::CAutoMutex kLock(m_kMutex);
	//찾아서 rkOut에 셋팅해주자.
	ContOtherGuild::const_iterator kGuild_iter = m_kOtherInfo.find(rkGuildGuid);
	if(m_kOtherInfo.end() != kGuild_iter)
	{
		rkOut.Set(kGuild_iter->second);
		return true;
	}
	return false;
}

bool PgGuildMgr::GetGuildInfo(BM::GUID const &rkGuildGuid, BM::GUID const& rkCharGuid, SGuildOtherInfo& rkOut)
{
	BM::CAutoMutex kLock(m_kMutex);
	//찾아서 rkOut에 셋팅해주자.
	ContOtherGuild::iterator kGuild_iter = m_kOtherInfo.find(rkGuildGuid);
	if(m_kOtherInfo.end() != kGuild_iter)
	{
		ContOtherGuild::mapped_type& rkInfo = kGuild_iter->second;
		rkOut.Set(rkInfo);
		return true;
	}
	return false;
}

bool PgGuildMgr::AddGuildInfo(const SGuildOtherInfo& rkInfo)
{
	BM::CAutoMutex kLock(m_kMutex);
	ContOtherGuild::iterator kGuild_iter = m_kOtherInfo.find(rkInfo.kGuid);
	if(m_kOtherInfo.end() != kGuild_iter)
	{
		ContOtherGuild::mapped_type &rkElement = kGuild_iter->second;
		//중복된 내용은 업데이트 하도록 한다.

		rkElement.Set(rkInfo);
		return true;
	}
	std::pair<ContOtherGuild::iterator, bool> ibRet = m_kOtherInfo.insert(std::make_pair(rkInfo.kGuid, ContOtherGuild::mapped_type(rkInfo)));
	return ibRet.second;
}

bool PgGuildMgr::DelGuildInfo(BM::GUID const &rkGuildGuid, BM::GUID const& rkCharGuid)
{
	BM::CAutoMutex kLock(m_kMutex);
	ContOtherGuild::iterator kGuild_iter = m_kOtherInfo.find(rkGuildGuid);
	if(m_kOtherInfo.end() != kGuild_iter)
	{
		ContOtherGuild::mapped_type& rkElement = (*kGuild_iter).second;
		rkElement.Del(rkCharGuid);
		if( rkElement.Empty() )
		{
			m_kOtherInfo.erase(kGuild_iter);
		}
		return true;
	}
	return false;
}

bool PgGuildMgr::ProcessPacket(BM::Stream *pkPacket)
{
	BM::CAutoMutex kLock(m_kMutex);
	BYTE cGuildCmd = GC_None;
	BYTE cRetCode = GCR_None;
	if(!pkPacket->Pop(cGuildCmd)) {return false;}
	if(!pkPacket->Pop(cRetCode)) {return false;}

	switch(cGuildCmd)
	{
	case GC_SetGuildEntranceOpen:
	case GC_SetMercenary:
		{
			int iTTW = 0;
			switch( cRetCode )
			{
			case GCR_Success:
				{
					if( cGuildCmd ==  GC_SetGuildEntranceOpen)
					{
						m_kGuildEntranceOpen.ReadFromPacket(*pkPacket);

						std::wstring wstrNotice(TTW(401085)); // 길드 가입 조건 설정이 변경됨.
						SChatLog kChatLog(CT_GUILD);
						g_kChatMgrClient.AddLogMessage(kChatLog, wstrNotice);
					}
					else if( cGuildCmd == GC_SetMercenary )
					{
						m_kSetMercenary.ReadFromPacket(*pkPacket);
					}
					else
					{

					}
				}break;
			case GCR_NotAuth:
				{
					iTTW = PgGuildMgrUtil::iMsgOnlyMaster;
				}break;
			default:
				{
				}break;
			}
			if( iTTW )
			{
				SChatLog kChatLog(CT_EVENT);
				g_kChatMgrClient.AddMessage(iTTW, kChatLog, true);
			}
		}break;
	case GC_M_AuthOwner:
	case GC_M_AuthMaster:
		{
			switch( cRetCode )
			{
			case GCR_Failed:
			default:
				{
					g_kChatMgrClient.LogMsgBox( PgGuildMgrUtil::iMsgOnlyMaster );
				}break;
			}
		}break;
	case GC_PreCreate_Test:
		{
			int iTTW = 0;
			switch( cRetCode )
			{
			case GCR_Success:
				{
					CallGuildNpcFunctionUI(QDT_Guild_Mark, 400837);
				}break;
			case GCR_Level:
				{
					iTTW = 400553;
				}break;
			case GCR_Money:
			default:
				{
					iTTW = 400554;
				}break;
			}
			if( iTTW )
			{
				CallGuildNpcFunctionUI(QDT_NpcTalk, iTTW);
			}
		}break;
	case GC_PreCreate:	//Step1 Check
		{
			int iTTW = 0;
			switch(cRetCode)
			{
			case GCR_Success:
				{
					//생성 성공 SFRM_GUILD_MASTER_COMMON_YES_NO
					TCHAR szTemp[512] = {0, };
					CallGuildNpcFunctionUI(QDT_Guild_Create, 400550);
				}break;
			case GCR_Level:
				{
					iTTW = 400553;
				}break;
			case GCR_Money:
				{
					iTTW = 400554;
				}break;
			case GCR_Member:
				{
					iTTW = 400547;
				}break;
			case GCR_BadName:
				{
					iTTW = 700106;
				}break;
			case GCR_Duplicate:
			default:
				{
					iTTW = 400551;
				}break;
			}
			if( iTTW )
			{
				CallGuildNpcFunctionUI(QDT_NpcTalk, iTTW);
			}
		}break;
	case GC_Create:	//Step2 Create Command
		{
			int iTTW = 0;
			switch(cRetCode)
			{
			case GCR_Success:
				{
					BM::GUID kCharGuid;
					SGuildOtherInfo kInfo;
					pkPacket->Pop(kCharGuid);
					kInfo.ReadFromPacket(*pkPacket);

					if( g_kPilotMan.IsMyPlayer(kCharGuid) )
					{
						iTTW = 400552;
						//강제로 Guild 정보 업데이트

						ReqBasicInfo(kInfo.kGuid);//내 길드 정보를 요청
						ReqMemberInfo(kInfo.kGuid);
					}
					
					AddGuildInfo(kInfo);
					SetPlayerGuildGuid(kCharGuid, kInfo.kGuid);

					if( g_kPilotMan.IsMyPlayer(kCharGuid) )
					{
						RandomTacticsQuestUI::CallRandomTacticsQuestBtn(true);
					}
				}break;
			case GCR_BadName:
				{
					iTTW = 700106;
				}break;
			case GCR_Max:
			default://어떤 실패던지 중복되었다고 찍는다 -_-;
				{
					iTTW = 400551;
				}break;
			}

			if( iTTW )
			{
				CallGuildNpcFunctionUI(QDT_NpcTalk, iTTW);
			}
		}break;
	case GC_OtherInfo:
		{
			if(GCR_Success == cRetCode)
			{
				SGuildOtherInfo kInfo;
				kInfo.ReadFromPacket(*pkPacket);
				if(AddGuildInfo(kInfo))
				{
					OtherGuildInfo(kInfo);
				}
			}
			else//실패
			{
				BM::GUID kGuildGuid;
				pkPacket->Pop(kGuildGuid);
				m_kRequester.erase(kGuildGuid);
			}
		}break;
	case GC_Join://이건 맵서버로 부터 온다.
		{
			int iTTW = 0;
			switch(cRetCode)
			{
			case GCR_Success:
				{
					BM::GUID kCharGuid;
					SGuildOtherInfo kInfo;

					pkPacket->Pop(kCharGuid);
					kInfo.ReadFromPacket(*pkPacket);

					AddGuildInfo(kInfo);
					SetPlayerGuildGuid(kCharGuid, kInfo.kGuid);//Pilot을 찾아 Actor에 이름을 Update 해준다.

					if( g_kPilotMan.IsMyPlayer( kCharGuid ) )
					{
						ReqBasicInfo(kInfo.kGuid);
						ReqMemberInfo(kInfo.kGuid);

						RandomTacticsQuestUI::CallRandomTacticsQuestBtn(true);

						PgPlayer *pkPC = g_kPilotMan.GetPlayerUnit();
						if( pkPC )
						{
							g_kSkillTree.CreateSkillTree( pkPC );
						}

						if( !g_kGuildMgr.IamHaveGuild() )
						{// 길드에 가입되는 유저
							BM::vstring vStr(TTW(400476));
							vStr.Replace(L"#GUILD#", kInfo.kName);
							::Notice_Show( static_cast<std::wstring>(vStr), 0 ); // ## 길드에 가입 되었습니다.

							if( m_kApplicationState.kGuildGuid.IsNotNull() )
							{// 길드게시판을 통해 지원했다면.
								InitApplicantState();
								if( XUIMgr.Get(L"FRM_GUILDWAR") )
								{
									XUIMgr.Close(L"FRM_GUILDWAR");
								}
							}
						}
					}
				}break;
			case GCR_Notify:
				{
					//누구가 가입 했습니다.
					SGuildMemberInfo kMember;
					kMember.ReadFromPacket(*pkPacket);
					std::wstring kMessage = kMember.kCharName + TTW(700087);
					SChatLog kChatLog(CT_EVENT);
					g_kChatMgrClient.AddLogMessage(kChatLog, kMessage, true);

					AddMember(kMember);
				}break;
			case GCR_CantMe:
				{
					iTTW = 400845; //내가 초대/탈퇴/추방 명령어를 할수 없는 장소에 있습니다.
				}break;
			case GCR_Failed:
			case GCR_Error:
				{
					iTTW = 400847;
				}break;
			case GCR_Max:
				{
					iTTW = 700077;//최대 길드원 초과
				}break;
			default:
				break;
			}

			if( iTTW )
			{
				SChatLog kChatLog(CT_EVENT);
				g_kChatMgrClient.AddMessage(iTTW, kChatLog, true);
			}
		}break;
	case GC_M_Destroy:
	case GC_M_Kick:
	case GC_Leave:
		{
			BM::GUID kCharGuid;
			pkPacket->Pop(kCharGuid);
			bool const bOwner = IsOwner(kCharGuid);
			bool const bMine = g_kPilotMan.IsMyPlayer(kCharGuid);

			g_kChatMgrClient.Name2Guid_Remove( kCharGuid );

			if( bMine )
			{
				PgPlayer *pkPC = g_kPilotMan.GetPlayerUnit();
				if( pkPC )
				{
					PgMySkill *pkMySkill = pkPC->GetMySkill();
					if( pkMySkill )
					{
						pkMySkill->EraseSkill(SDT_Guild);
						g_kSkillTree.CreateSkillTree( pkPC );
					}
				}
			}

			int iTTW = 0;
			switch(cRetCode)
			{
			case GCR_Success://성공
				{//kCharGuid 캐릭터와 같은 그라운드에 근처에 있으면 무조건 온다
					SetPlayerGuildGuid(kCharGuid, BM::GUID::NullData());

					if( bMine )//나면
					{
						Clear();//길드 정보 정리
						lwGuild::lwSendGuildInvClose();
						RandomTacticsQuestUI::CallRandomTacticsQuestBtn(false);
						XUIMgr.Close(RandomQuestUI::kRandomQuestUIName);
						XUIMgr.Close(L"SFRM_GUILD_INVENTORY");
						XUIMgr.Close(L"SFRM_GUILD_SETTING");
						XUIMgr.Close(L"SFRM_GUILD_APPLICANT_ACCEPT_REJECT");
						
						if( cGuildCmd == GC_M_Kick )
						{
							iTTW = 700084;
						}
						else if( cGuildCmd == GC_M_Destroy )
						{
							iTTW = 700086;//길드가 해체
							m_kGuildEntranceApplicant.clear(); // 길드가입 신청자들 정리
						}
						else if( cGuildCmd == GC_Leave )
						{
							iTTW = 700085;
						}
					}
				}break;
			case GCR_Notify://길드 원일 때에만 온다
				{
					SetPlayerGuildGuid(kCharGuid, BM::GUID::NullData());

					if( bMine )
					{
						Clear();//길드 정보 정리
					}
					else
					{
						int iNfyTTW = 0;
						std::wstring kCharName;
						pkPacket->Pop(kCharName);

						if( cGuildCmd == GC_Leave )
						{
							iNfyTTW = 700088;//님이 길드에서 탈퇴
						}
						else if( cGuildCmd == GC_M_Kick )
						{
							iNfyTTW = 700089;//님이 길드에서 추방
						}

						DelMember(kCharGuid);

						if( iNfyTTW )
						{
							SChatLog kChatLog(CT_EVENT);
							std::wstring kMessage = kCharName + TTW(iNfyTTW);
							g_kChatMgrClient.AddLogMessage(kChatLog, kMessage, true, 0);
						}
					}
				}break;
			case GCR_None://탈퇴는 오너 안되
				{
					iTTW = 700079;
				}break;
			case GCR_NotAuth://킥은 마스터 아니면 안되
				{
					iTTW = PgGuildMgrUtil::iMsgOnlyMaster;
				}break;
			case GCR_DoNotMe://킥은 자신은 안되
				{
					iTTW = 700081;
				}break;
			case GCR_CantMe:
				{
					iTTW = 400845; //내가 초대/탈퇴/추방 명령어를 할수 없는 장소에 있습니다.
				}break;
			case GCR_CantHim:
				{
					iTTW = 400846; //상대방이 초대/탈퇴/추방 명령어를 할수 없는 장소에 있습니다.
				}break;
			case GCR_Failed:
			case GCR_Error:
				{
					iTTW = 700082;//에러
				}break;
			case GCR_NotEmpty:
				{
					iTTW = 400558;
				}break;
			case GCR_HaveEmporia:
				{
					iTTW = 700140;
				}break;
			case GCR_Same:
				{
					iTTW = 700403; // 같은 등급의 길드원은 추방 할 수 없습니다.
				}break;
			default:
				break;
			}

			if( iTTW )
			{
				if( GC_M_Destroy == cGuildCmd
				&&	bOwner )
				{
					CallGuildNpcFunctionUI(QDT_NpcTalk, iTTW);
				}
				else
				{
					SChatLog kChatLog(CT_EVENT);
					g_kChatMgrClient.AddMessage(iTTW, kChatLog, true);
				}
			}
		}break;
	case GC_M_MemberGrade:
			{
				int iTTW = 0;
				switch( cRetCode )
				{
				case GCR_Success:
					{
						BM::GUID kCharGuid;
						BYTE cNewMemberGrade = GMG_Membmer;
						bool bIsMaster = false;
						pkPacket->Pop( kCharGuid );
						pkPacket->Pop( cNewMemberGrade );

						ContGuildMember::iterator find_iter = m_kMember.find( kCharGuid );
						if( m_kMember.end() != find_iter )
						{
							bool const bMine = g_kPilotMan.IsMyPlayer(kCharGuid);
							if( bMine ) // 내가
							{
								if( !IsMaster( kCharGuid ) ) 
								{// 길드원이였으면 길드 가입 신청서 요청
									BM::Stream kPacket(PT_C_N_REQ_GUILD_COMMAND, (BYTE)GC_GuildEntranceApplicant_List);
									kPacket.Push(GuildGuid());
									NETWORK_SEND(kPacket)
								}
								else
								{
									ClearEntranceApplicant();
									XUIMgr.Close(L"SFRM_GUILD_SETTING");
									XUIMgr.Close(L"SFRM_GUILD_APPLICANT_ACCEPT_REJECT");
								}
							}

							(*find_iter).second.cGrade = cNewMemberGrade;
						}

						SGuildMemberInfo kMemberInfo;
						if( GetMemberByGuid(kCharGuid, kMemberInfo) )
						{
							std::wstring kGradeName;
							if( GetGradeName(kMemberInfo.cGrade, kGradeName) )
							{
								SChatLog kChatLog(CT_EVENT);
								if( FormatTTW(kChatLog.kContents, 400626, kMemberInfo.kCharName.c_str(), kGradeName.c_str()) )
								{
									g_kChatMgrClient.AddLogMessage(kChatLog, kChatLog.kContents, true);
								}
							}
						}
					}break;
				case GCR_NotAuth:
					{
						iTTW = PgGuildMgrUtil::iMsgOnlyMaster;
					}break;
				case GCR_Max: // 부길마는 최대 2명까지
					{
						iTTW = 700404;
					}break;
				case GCR_Failed:
				case GCR_Member:
				default:
					{
						iTTW = 700091;
					}break;
				}

				if( iTTW )
				{
					SChatLog kChatLog(CT_EVENT);
					g_kChatMgrClient.AddMessage(iTTW, kChatLog, true);
				}
			}break;
	case GC_M_Notice:
		{
			int iTTW = 0;
			switch( cRetCode )
			{
			case GCR_Success:
				{
					BM::GUID kSetterGuid;

					pkPacket->Pop( m_kGuildNotice );
					pkPacket->Pop( kSetterGuid );

					SChatLog kChatLog(CT_EVENT);
					if( FormatTTW(kChatLog.kContents, 400632, m_kGuildNotice.c_str()) )
					{
						g_kChatMgrClient.AddLogMessage(kChatLog, kChatLog.kContents, true);
					}
				}break;
			case GCR_Max:
				{
					iTTW = 400633;
				}break;
			case GCR_BadName:
				{
					iTTW = 700144;
				}break;
			case GCR_NotAuth:
			default:
				{
					iTTW = PgGuildMgrUtil::iMsgOnlyMaster;
				}break;
			}
			if( iTTW )
			{
				SChatLog kChatLog(CT_EVENT);
				g_kChatMgrClient.AddMessage(iTTW, kChatLog, true);
			}
		}break;
	case GC_Info:
		{
			if(GCR_Success == cRetCode)
			{
				m_kBasicInfo.ReadFromPacket(*pkPacket);
				m_kSetMercenary.ReadFromPacket(*pkPacket);
				m_kGuildEntranceOpen.ReadFromPacket(*pkPacket);
				pkPacket->Pop( m_kGuildNotice );
				ReadEmporiaInfoFromPacket(*pkPacket);

				if( m_kMember.size() )//맴버 정보가 있으면
				{
					m_bWait = false;
				}

				PgInventory* pkInv = g_kGuildMgr.GetInven();
				if( pkInv )
				{
					pkInv->Init(m_kBasicInfo.abyInvExtern, m_kBasicInfo.abyInvExternIdx);
				}
			}
			else
			{
				//why failed ?
			}	
		}break;
	case GC_List:
	case GC_UpdateGuildList:
		{
			switch ( cRetCode )
			{
			case GCR_Success:
				{
					ReadListFromPacket(*pkPacket, false);
				}break;
			case GCR_Moved:
				{
					ReadListFromPacket(*pkPacket, true);
				}break;
			default:
				{
					return true;
				}break;
			}

			if( BM::GUID::NullData() != m_kBasicInfo.kGuildGuid )
			{
				m_bWait = false;
			}
		}break;
	case GC_GuildEntranceApplicant_State:
		{// 길드오너, 마스터가 수락/거부 한 결과
			BM::GUID kCharGuid;			
			pkPacket->Pop(kCharGuid);			

			switch ( cRetCode )
			{
			case GCR_Success:
				{
					BYTE byState;
					pkPacket->Pop(byState);

					CONT_MY_GUILD_ENTRANCE_APPLICANTS::iterator find_iter = m_kGuildEntranceApplicant.find(kCharGuid);
					if( find_iter != m_kGuildEntranceApplicant.end() )
					{
						if( byState == AS_REJECT )
						{// 길드 오너/마스터가 가입신청을 거부했으니 목록에서 지워준다.
							m_kGuildEntranceApplicant.erase(find_iter);
						}
						else
						{
							CONT_MY_GUILD_ENTRANCE_APPLICANTS::mapped_type& kApplicant =(*find_iter).second;
							kApplicant.byState = byState;							
						}

						lwGuild::GuildApplicantSetting(); // UI 갱신
					}
				}break;
			case GCR_NotFoundUser:
				{// 대상을 찾을 수 없습니다.
					CONT_MY_GUILD_ENTRANCE_APPLICANTS::iterator find_iter = m_kGuildEntranceApplicant.find(kCharGuid);
					if( find_iter != m_kGuildEntranceApplicant.end() )
					{
						m_kGuildEntranceApplicant.erase(find_iter);
					}
					lwGuild::GuildApplicantSetting();
					::Notice_Show( TTW(201791), EL_Warning );
				}break;
			default:
				{
				}break;
			}
		}break;
	case GC_GuildEntranceApplicant_List:
		{
			switch ( cRetCode )
			{
			case GCR_Success:
				{
					ReadGuildEntranceApplicantListFromPacket(*pkPacket);
				}break;
			default:
				{

				}break;
			}
		}break;
	case GC_ReqJoin://
		{
			if( GCR_None == cRetCode )//초대 요청을 받음
			{
				std::wstring kMasterName;
				BM::GUID kGuildGuid;
				std::wstring kGuildName;

				pkPacket->Pop(kMasterName);
				pkPacket->Pop(kGuildGuid);
				pkPacket->Pop(kGuildName);

				if( 0 != g_kGlobalOption.GetValue("ETC", "DENY_GUILD") )//자동 거부이면
				{
					SChatLog kChatLog(CT_EVENT);
					g_kChatMgrClient.AddMessage(700117, kChatLog, true, 1);

					ReqJoinAnswer(kGuildGuid, false);//거부
					break;
				}

				//
				std::wstring kMessage;
				bool const bRet = FormatTTW(kMessage, 400561, kMasterName.c_str(), kGuildName.c_str());//초대
				if( bRet )
				{
					CallYesNoMsgBox(kMessage, kGuildGuid, MBT_GUILD_JOIN);
				}
			}
			else//길드장 명령어 처리
			{
				int iTTW = 0;
				switch(cRetCode)
				{
				case GCR_Success:
					{
					}break;
				case GCR_JoinWait://Join Wait
					{
						iTTW = 700142;
					}break;
				case GCR_Duplicate://Other guild
					{
						iTTW = 700075;
					}break;
				case GCR_Member://same member
					{
						iTTW = 700076;
					}break;
				case GCR_Max://Full member
					{
						iTTW = 700077;
					}break;
				case GCR_Failed://Can't find
				case GCR_Error://default failed
					{
						iTTW = 700013;
					}break;
				case GCR_Reject:
					{
						std::wstring kMessage, kRejectUserName;

						pkPacket->Pop(kRejectUserName);

						FormatTTW(kMessage, 200129, kRejectUserName.c_str());
						SChatLog kChatLog(CT_EVENT);
						g_kChatMgrClient.AddLogMessage(kChatLog, kMessage, true);
					}break;
				case GCR_Date_Limit:
					{
						__int64 iValue = 0;
						pkPacket->Pop( iValue );

						// 길드게시판을 통해 신청했다면
						if( m_kApplicationState.kGuildGuid.IsNotNull() )
						{
							m_kApplicationState.Init();
						}

						std::wstring kMessage;
						PgStringUtil::ReplaceStr< std::wstring >(TTW(700510), L"$HOUR$", BM::vstring(iValue), kMessage);
						g_kChatMgrClient.AddLogMessage(SChatLog(CT_EVENT), kMessage, true);
					}break;
				case GCR_CantMe:
					{
						iTTW = 400845; //내가 초대/탈퇴/추방 명령어를 할수 없는 장소에 있습니다.
					}break;
				case GCR_CantHim:
					{
						iTTW = 400846; //상대방이 초대/탈퇴/추방 명령어를 할수 없는 장소에 있습니다.
					}break;
				default:
					iTTW = 0;//
					break;
				}
				
				if( iTTW )
				{
					SChatLog kChatLog(CT_EVENT);
					g_kChatMgrClient.AddMessage(iTTW, kChatLog, true);
				}
			}
		}break;
	case GC_M_Rename:
	case GC_GM_Rename:
		{
			if( GCR_Success == cRetCode )
			{
				BM::GUID kGuildGuid;
				std::wstring kNewGuildName;

				pkPacket->Pop(kGuildGuid);
				pkPacket->Pop(kNewGuildName);

				SGuildOtherInfo kOtherInfo;
				if( GetGuildInfo(kGuildGuid, kOtherInfo) )//다른 길드
				{
					kOtherInfo.kName = kNewGuildName;
					AddGuildInfo(kOtherInfo);//다시 업데이트
				}

				if( m_kBasicInfo.kGuildGuid == kGuildGuid )//자신의 길드면
				{
					m_kBasicInfo.kGuildName = kNewGuildName;
				}

				// 월드의 길드 Guid
				if( NULL != g_pkWorld)
				{
					g_pkWorld->PickObject(PgGuildMgrUtil::PgUpdateGuildName(kGuildGuid));
				}

			}
			else
			{
				//
			}
		}break;
	case GC_AddExp:
		{		
			__int64 iResultExp = 0;
			
			pkPacket->Pop( iResultExp );
			RefreshExp( iResultExp );
		}break;
	case GC_M_ChangeMark1:
		{
			int iTTW = 0;
			switch( cRetCode )
			{
			case GCR_Success:
				{
					BM::GUID kGuildGuid, kCharGuid;
					BYTE cNewGuildEmblem = 0;
					pkPacket->Pop( kCharGuid );
					pkPacket->Pop( kGuildGuid );
					pkPacket->Pop( cNewGuildEmblem );

					SGuildOtherInfo kOtherInfo;
					if( GetGuildInfo(kGuildGuid, kOtherInfo) )
					{
						kOtherInfo.cEmblem = cNewGuildEmblem;
						AddGuildInfo(kOtherInfo);
					}

					if( g_kPilotMan.IsMyPlayer(kCharGuid)
					&&	g_kGuildMgr.GuildGuid() == kGuildGuid )
					{
						// 메시지
						SChatLog kCharLog(CT_EVENT);
						g_kChatMgrClient.AddMessage(400853, kCharLog, true);
					}
					if(g_pkWorld)
					{
						PgActor* pkActor = dynamic_cast< PgActor* >(g_pkWorld->FindObject(kCharGuid));
						if( pkActor )
						{
							pkActor->UpdateName();
						}
					}
				}break;
			case GCR_Failed:
			default:
				{
				}break;
			}

		}break;
	case GC_M_SetLv:
	case GC_M_LvUp:
		{
			int iTTW = 0;
			switch(cRetCode)
			{
			case GCR_Success://길드에 레벨이 올랐습니다.
				{
					SGuildBasicInfo kBasicInfo;
					BM::GUID kOperatorGuid;
					pkPacket->Pop(kOperatorGuid);
					kBasicInfo.ReadFromPacket(*pkPacket);

					m_kBasicInfo = kBasicInfo;

					PgPlayer* pkPC = g_kPilotMan.GetPlayerUnit();
					if( pkPC )
					{
						if( IsMaster(pkPC->GetID())
						&&	kOperatorGuid == pkPC->GetID()  )
						{
							//NPC 대화상자 출력
							iTTW = lwGuild::GetGuildLevelResultText(m_kBasicInfo.sLevel);
						}
						else
						{
							//공지 출력
							SChatLog kChatLog(CT_EVENT);
							std::wstring kNoticeText;
							if( FormatTTW(kNoticeText, 400809, m_kBasicInfo.sLevel) )
							{
								g_kChatMgrClient.AddLogMessage(kChatLog, kNoticeText, true);
							}
						}
					}

					PgActor *pkActor = g_kPilotMan.GetPlayerActor();
					if( pkActor )
					{
						pkActor->PlayNewSound(NiAudioSource::TYPE_3D, "Guild_LV_UP", 0.0f);	// Sound
						pkActor->AddNewParticle("e_guild_level_up", 8383, "char_root", pkActor->GetEffectScale());
					}
				}break;
			case GCR_Max://현재 최대 레벨 입니다.
				{
					iTTW = 400819;
				}break;
			case GCR_NotEnoughExp://경험치 부족
				{
					iTTW = 400840;
				}break;
			case GCR_Same:
				{
				}break;
			case GCR_Money://돈, 아이템 부족
			case GCR_Failed://실패
			default:
				{
					iTTW = 400810;//부족
				}break;
			}

			if( iTTW )
			{
				CallGuildNpcFunctionUI(QDT_NpcTalk, iTTW);
			}
		}break;
	case GC_InventoryOpen:
		{// 길드 인벤토리 아이템 리스트
			BYTE byInvType = 0;
			__int64 i64Money = 0i64;
			
			pkPacket->Pop(byInvType);
			m_kInv.Clear(static_cast<EInvType>(byInvType)); // 해당 인벤토리 클리어 하고 새로 받은 데이터로 셋팅.

			m_kInv.ReadFromPacket(static_cast<EInvType>(byInvType), *pkPacket);
			pkPacket->Pop(i64Money);
			m_kInv.Money(i64Money);
			lwGuild::CallGuildInv(byInvType);
		}break;
	case GC_InventoryMoney:
		{// 길드 Gold
			__int64 i64Money = 0i64;
			pkPacket->Pop(i64Money);
			m_kInv.Money(i64Money);
		}break;
	case GC_InventoryLog:
		{
			lwGuild::CallGuildLog(pkPacket);
		}break;
	case GC_InventoryUpdate:
		{
			switch(cRetCode)
			{
			case GCR_Success:
				{
					std::wstring kCharName;
					pkPacket->Pop(kCharName);

					DB_ITEM_STATE_CHANGE_ARRAY kArray;
					PU::TLoadArray_M( *pkPacket, kArray);

					bool bChangeView = false;

					BYTE byInvType = 0;
					DB_ITEM_STATE_CHANGE_ARRAY::const_iterator item_itor = kArray.begin();
					while(kArray.end() != item_itor )
					{
						DB_ITEM_STATE_CHANGE_ARRAY::value_type const &rkCurModify = (*item_itor);

						if( rkCurModify.State() == DISCT_MODIFY_GUILD_MONEY )
						{
							BM::Stream kAddonData = rkCurModify.kAddonData;
							__int64 i64NewMoney;
							kAddonData.Pop(i64NewMoney);
							m_kInv.Money(i64NewMoney);
						}
						else
						{
							PgItemWrapper const &rkItemWrapper = rkCurModify.ItemWrapper();

							byInvType = rkItemWrapper.Pos().x;
							bChangeView = true;

							if( false == m_kInv.Modify(rkItemWrapper.Pos(), rkItemWrapper) )
							{// 길드금고 수정 실패하면 새로 요청하자.
								PgPlayer* pkPC = g_kPilotMan.GetPlayerUnit();
								if( !pkPC )
								{
									::Notice_Show( TTW(3075), EL_Warning );
									XUIMgr.Close(L"SFRM_GUILD_INVENTORY");
									return false;
								}

								BM::Stream kPacket(PT_C_N_REQ_GUILD_COMMAND, static_cast<BYTE>(GC_InventoryOpen));
								kPacket.Push(pkPC->GuildGuid());
								kPacket.Push(byInvType);
								NETWORK_SEND(kPacket);
								return false;
							}
						}

						++item_itor;
					}

					PgPlayer * pkMyPlayer = g_kPilotMan.GetPlayerUnit();
					if( !pkMyPlayer ){ break; }

					if( pkMyPlayer->Name() == kCharName
					&&	bChangeView )
					{
						lwGuild::CallGuildInv(byInvType);
					}
				}break;
			case GCR_NotInvAuth:
				{//권한없음
					::Notice_Show( TTW(201506), EL_Warning );
				}break;
			case GCR_NotInvMove:
				{// 이동할 수 없는 위치
					::Notice_Show( TTW(20025), EL_Warning );
				}break;
			case GCR_Money:
				{// 골드가 부족합니다.
					::Notice_Show( TTW(403078), EL_Warning );
				}break;
			case GCR_Error:
				{// 1골드 이상 입력해야 합니다.
					::Notice_Show( TTW(403408), EL_Warning );
				}break;
			default:
				{
				}break;
			}
		}break;
	case GC_M_InventorySupervision:
		{
			BYTE abyAuthority[MAX_DB_GUILD_INV_AUTHORITY] = {0, };
			pkPacket->Pop(abyAuthority);

			switch(cRetCode)
			{
			case GCR_Success:
				{
					SetGuildInvAuthority(abyAuthority[1],abyAuthority[3]);
					std::wstring wstrNotice(TTW(403406));
					SChatLog kChatLog(CT_GUILD);
					g_kChatMgrClient.AddLogMessage(kChatLog, wstrNotice);
				}break;
			default:
				{
				}break;
			}
		}break;
	case GC_M_InventoryCreate:
		{
			int iTTW = 0;
			switch(cRetCode)
			{
			case GCR_Success:
				{	
					BYTE abyInvExtern[MAX_DB_INVEXTEND_SIZE] = {0,};
					BYTE abyInvExternIdx[MAX_DB_INVEXTEND_SIZE] = {0,};
					BYTE abyInvAuthority[MAX_DB_GUILD_INV_AUTHORITY] = {0,};

					pkPacket->Pop(abyInvExtern);
					pkPacket->Pop(abyInvExternIdx);
					pkPacket->Pop(abyInvAuthority);

					PgInventory* pkInv = g_kGuildMgr.GetInven();
					if( pkInv )
					{
						pkInv->Init(abyInvExtern, abyInvExternIdx);
					}
					
					SetGuildInvAuthority(abyInvAuthority[1], abyInvAuthority[3]);

					PgPlayer* pkPC = g_kPilotMan.GetPlayerUnit();
					if( pkPC )
					{
						if( IsMaster( pkPC->GetID() ) )
						{// 길드창고가 개설 되었습니다.
							iTTW = 401071;
						}
					}
				}break;
			case GCR_Level:
				{// 길드창고는 길드레벨 2이상부터 생성할 수 있다.
					iTTW = 401078;
				}break;
			case GCR_Money:
				{// 돈이 부족해.
					iTTW = 403078;
				}break;
			case GCR_Failed:
			default:
				{
				}break;
			}
			if( iTTW )
			{
				CallGuildNpcFunctionUI(QDT_NpcTalk, iTTW);
			}
		}break;
	case GC_M_AddSkill:
		{
			int iTTW = 0;
			switch(cRetCode)
			{
			case GCR_Success:
				{
					//성공
					int iSkillNo = 0;
					SGuildBasicInfo kBasicInfo;
					pkPacket->Pop(iSkillNo);
					kBasicInfo.ReadFromPacket(*pkPacket);

					m_kBasicInfo = kBasicInfo;

					m_kGuildSkill.Init();
					m_kGuildSkill.Add(MAX_DB_GUILD_SKILL_SIZE, m_kBasicInfo.cGuildSkill);

					PgPlayer *pkPC = g_kPilotMan.GetPlayerUnit();
					if( pkPC )
					{
						PgMySkill *pkMySkill = pkPC->GetMySkill();
						if( pkMySkill )
						{
							pkMySkill->LearnNewSkill(iSkillNo);
							g_kSkillTree.CreateSkillTree( pkPC );
						}
					}

					GET_DEF(CSkillDefMgr, kSkillDefMgr);
					const CSkillDef* pkSkillDef = kSkillDefMgr.GetDef(iSkillNo);
					if( pkSkillDef )
					{
						std::wstring const *pkSkillName = NULL;
						if( ::GetDefString(pkSkillDef->NameNo(), pkSkillName) )
						{
							std::wstring const &rkNoticeText = TTW(400820);

							TCHAR szTemp[512] = {0, };
							_stprintf_s(szTemp, sizeof(szTemp), _T("{C=0xFFFF0000/}[%s %s %d]"), pkSkillName->c_str(), TTW(224).c_str(), pkSkillDef->GetAbil(AT_LEVEL));

							SChatLog kChatLog(CT_EVENT);
							g_kChatMgrClient.AddLogMessage(kChatLog, rkNoticeText);	// Message

							SIconInfo kSkillIconInfo(KUIG_SKILLTREE, iSkillNo);
							kSkillIconInfo.iIconResNumber = pkSkillDef->RscNameNo();
							lwGuild::ShowSingleResultWindow(kSkillIconInfo, rkNoticeText, std::wstring(szTemp));

							PgActor *pkActor = g_kPilotMan.GetPlayerActor();
							if( pkActor )
							{
								pkActor->PlayNewSound(NiAudioSource::TYPE_3D, "Guild_Skill_UP", 0.0f);	// Sound
							}
						}
					}

					if( IsMaster(pkPC->GetID()) )
					{
						XUI::CXUI_Wnd *pkTopWnd = XUIMgr.Get( PgQuestManUtil::kFullQuestWndName );
						if( pkTopWnd )
						{
							XUI::CXUI_Wnd *pkGuildBuySkillWnd = pkTopWnd->GetControl( _T("SFRM_GUILD_BUY_SKILL") );
							lwGuild::UpdateGuildSkillBuyUI(pkGuildBuySkillWnd);
						}
					}
				}break;
			case GCR_NotAuth:
				{
					//길마만 가능
					iTTW = PgGuildMgrUtil::iMsgOnlyMaster;
				}break;
			case GCR_NotEnoughExp:
				{
					iTTW = 400448;
				}break;
			case GCR_Pre:
				{
					//이전 배워야 할 스킬이 없음
					iTTW = 400822;
				}break;
			case GCR_Failed:
			case GCR_Money:
				{
					//돈이나, 레벨, 스킬포인트, 아이템 부족
					iTTW = 400821;
				}break;
			}

			if( iTTW )
			{
				SChatLog kChatLog(CT_EVENT);
				g_kChatMgrClient.AddMessage(iTTW, kChatLog, true);
			}
		}break;
	case GC_M_ChangeOwner:
		{
			int iTTW = 0;
			switch( cRetCode )
			{
			case GCR_Success:
			case GCR_Notify:
				{
					//성공
					BM::GUID kNewOwnerGuid;
					pkPacket->Pop( kNewOwnerGuid );

					bool const bNewMine = g_kPilotMan.IsMyPlayer(kNewOwnerGuid);
					bool const bOldMine = g_kPilotMan.IsMyPlayer(m_kBasicInfo.kMasterGuid);

					if( bNewMine )
					{
						if( !IsMaster(kNewOwnerGuid) )
						{// 길드원이였다면
							BM::Stream kPacket(PT_C_N_REQ_GUILD_COMMAND, (BYTE)GC_GuildEntranceApplicant_List);
							kPacket.Push(GuildGuid());
							NETWORK_SEND(kPacket)
						}
					}

					if( bOldMine )
					{
						ClearEntranceApplicant();
						XUIMgr.Close(L"SFRM_GUILD_SETTING");
						XUIMgr.Close(L"SFRM_GUILD_APPLICANT_ACCEPT_REJECT");
					}

					SGuildMemberInfo kNewOwnerInfo;
					if( GetMemberByGuid(kNewOwnerGuid, kNewOwnerInfo) )
					{
						SChatLog kChatLog(CT_EVENT);
						if( FormatTTW(kChatLog.kContents, 700130, kNewOwnerInfo.kCharName.c_str()) )
						{
							g_kChatMgrClient.AddLogMessage(kChatLog, kChatLog.kContents, true);
						}
					}
					m_kBasicInfo.kMasterGuid = kNewOwnerGuid;
		
					ReqMemberInfo(m_kBasicInfo.kGuildGuid); //맴버 목록 재 요청
				}break;
			case GCR_NotAuth:
				{
					iTTW = PgGuildMgrUtil::iMsgOnlyMaster; // 길드마스터 아니면 안되
				}break;
			case GCR_None:
			case GCR_Failed:
			default:
				{
					iTTW = 700131; // 실패 상대편이 길드에 없거나 현재 접속중이지 않음, 둘다 로그인 해야 변경 가능
				}break;
			}

			if( iTTW )
			{
				SChatLog kChatLog(CT_EVENT);
				g_kChatMgrClient.AddMessage(iTTW, kChatLog, true);
			}
		}break;
	case GC_ReserveBattleForEmporia:
		{
			int iTTW = 0;
			switch ( cRetCode )
			{
			case GCR_Success:
			case GCR_System:
				{
 					ReadEmporiaInfoFromPacket( *pkPacket );
					g_kEmporiaMgr.RefreshUI( false, NULL );

					__int64 i64ChallengeMoney = 0;
					__int64 i64CurrentMoney = 0i64;
                    pkPacket->Pop( i64CurrentMoney );
                    pkPacket->Pop( i64ChallengeMoney );
					
					if ( GCR_Success == cRetCode )
					{
						switch ( m_kEmporiaInfo.byType )
						{
						case EMPORIA_KEY_NONE:
							{
								iTTW = 71001;
							}break;
						case EMPORIA_KEY_MINE:
							{
							}break;
						case EMPORIA_KEY_BATTLERESERVE:
							{
								PgPlayer * pkMyPlayer = g_kPilotMan.GetPlayerUnit();
								if( !pkMyPlayer ){ break; }
								
								__int64 const i64ChallengeEndTime = m_kEmporiaInfo.i64BattleTime - PgEmporiaTournament::EMPORIA_BATTLE_CHALLENGE_ENDTIME_BEFORE_BATTLETIME;

								SYSTEMTIME kSystime;
								CGameTime::SecTime2SystemTime( i64ChallengeEndTime, kSystime, CGameTime::DEFAULT );

								BM::vstring vstrTemp(TTW(71056));
								vstrTemp += L" ";
								vstrTemp += TTW(71057);
								vstrTemp.Replace( L"#YEAR#", kSystime.wYear );
								vstrTemp.Replace( L"#MONTH#", BM::vstring(kSystime.wMonth, L"%02u") );
								vstrTemp.Replace( L"#DAY#", BM::vstring(kSystime.wDay, L"%02u") );
								vstrTemp.Replace( L"#HOUR#", BM::vstring(kSystime.wHour, L"%02u") );
								vstrTemp.Replace( L"#MIN#", BM::vstring(kSystime.wMinute, L"%02u") );								
								std::wstring kDay = static_cast<std::wstring>(vstrTemp);

                                int const iChallengeGold = static_cast<int>(i64ChallengeMoney / 10000);
								if( XUIMgr.Get(L"FRM_GUILDWAR") )
								{
									BM::vstring vStr(TTW(70093));
									vStr.Replace(L"#CHALLENGE_POINT#", iChallengeGold);
									vStr.Replace(L"#DAY#", kDay);

									lua_tinker::call<void, lwWString, bool>("CommonMiddleMsgBoxByText", lwWString(static_cast<std::wstring>(vStr)), true);
								}
								else
								{
									BM::vstring vStr(TTW(71002));
									vStr.Replace(L"#CHALLENGE_POINT#", iChallengeGold);
									vStr.Replace(L"#DAY#", kDay);

									SChatLog kChatLog(CT_EVENT);
									g_kChatMgrClient.AddLogMessage(kChatLog, vStr, true);
								}
							}break;
						case EMPORIA_KEY_BATTLECONFIRM:
						case EMPORIA_KEY_BATTLE_ATTACK:
						case EMPORIA_KEY_BATTLE_DEFENCE:
							{
							}break;
						}
					}
				}break;
			case GCR_Level:
				{
					iTTW = 71044;
				}break;
			case GCR_Duplicate:
				{
					iTTW = 71003;
				}break;
			case GCR_NotAuth:
				{
					iTTW = PgGuildMgrUtil::iMsgOnlyMaster;
				}break;
			case GCR_NotEnoughExp:
				{// 경험치 부족
					iTTW = 71041;
				}break;
            case GCR_Money:
                {
                    iTTW = 71577;
                }break;
			case GCR_LimitCost:
				{
					iTTW = 71578;
				}break;
            case GCR_NotInvAuth://길드금고를 생성해 주세요.
                {
                    iTTW = 71579;
                }break;
			}

			if( iTTW )
			{
				SChatLog kChatLog(CT_EVENT);
				g_kChatMgrClient.AddMessage(iTTW, kChatLog, true);
			}
		}break;
	case GC_EmporiaInfo:
		{
			ReadEmporiaInfoFromPacket(*pkPacket);

			PgActor * pkMyActor = g_kPilotMan.GetPlayerActor();
			if ( pkMyActor )
			{
				pkMyActor->UpdateName();
			}
		}break;
	case GC_None:
	default:
		{
			return false;
		}break;
	}

	UpdateGuildUI();
	UpdateGuildSkillUI();
	return true;
}

bool PgGuildMgr::ReqBasicInfo(BM::GUID const &rkGuildGuid)
{
	if(BM::GUID::NullData() != rkGuildGuid)
	{
		BM::Stream kPacket(PT_C_N_REQ_GUILD_COMMAND, (BYTE)GC_Info);
		kPacket.Push(rkGuildGuid);
		NETWORK_SEND(kPacket)
		return true;
	}
	return false;
}

bool PgGuildMgr::ReqMemberInfo(BM::GUID const &rkGuildGuid)
{
	if( BM::GUID::NullData() != rkGuildGuid )
	{
		BM::Stream kPacket(PT_C_N_REQ_GUILD_COMMAND, (BYTE)GC_List);
		kPacket.Push(rkGuildGuid);
		NETWORK_SEND(kPacket)
		return true;
	}
	return false;
}

bool PgGuildMgr::ReqGuildEntranceApplicantList(BM::GUID const & rkGuildGuid)
{
	if( BM::GUID::NullData() != rkGuildGuid )
	{
		BM::Stream kPacket(PT_C_N_REQ_GUILD_COMMAND, (BYTE)GC_GuildEntranceApplicant_List);
		kPacket.Push(rkGuildGuid);
		NETWORK_SEND(kPacket);
		return true;
	}
	return false;
}


bool PgGuildMgr::SetPlayerGuildGuid(BM::GUID const &rkCharGuid, BM::GUID const &rkGuildGuid)
{
	PgPilot* pkPilot = g_kPilotMan.FindPilot(rkCharGuid);
	if(pkPilot)
	{
		PgPlayer* pkPC = dynamic_cast<PgPlayer*>(pkPilot->GetUnit());
		if(pkPC)
		{
			if( BM::GUID::IsNull(rkGuildGuid) )
			{
				DelGuildInfo(pkPC->GuildGuid(), rkCharGuid);
			}

			pkPC->GuildGuid(rkGuildGuid);
			UpdateActorGuildName(rkCharGuid);
			return true;
		}
	}
	return false;
}

bool PgGuildMgr::UpdateActorGuildName(BM::GUID const &rkActorGuid)//Actor의 이름을 Refresh한다.
{
	if(!g_pkWorld)
	{
		return false;
	}
	PgActor *pkActor = dynamic_cast<PgActor*>(g_pkWorld->FindObject(rkActorGuid));
	if(pkActor)
	{
		return pkActor->UpdateName();
	}
	return false;
}

void PgGuildMgr::ReqJoinGuild(BM::GUID const &rkCharGuid)//Guid로 초대
{
	if(!g_pkWorld)
	{
		return;
	}
	BM::GUID kMyCharGuid;
	bool const bFindPilot = g_kPilotMan.GetPlayerPilotGuid(kMyCharGuid);
	if( !bFindPilot
	&&	!IsMaster(kMyCharGuid) )
	{
		SChatLog kChatLog(CT_EVENT);
		g_kChatMgrClient.AddMessage(700083, kChatLog, true);
		return ;//자기 자신 초대는 안되
	}
	
	if( !PgGuildMgrUtil::IsCanReqInOutState(g_pkWorld->GetAttr()) )
	{
		SChatLog kChatLog(CT_EVENT);
		g_kChatMgrClient.AddMessage(400845, kChatLog, true);
		return;
	}

	BM::Stream kPacket(PT_C_N_REQ_GUILD_COMMAND, (BYTE)GC_ReqJoin);
	kPacket.Push(rkCharGuid);
	NETWORK_SEND(kPacket)

	std::wstring kCharName;
	bool bFindName = false;
	if( !bFindName )
	{
		bFindName = g_kChatMgrClient.Name2Guid_Find_ByGuid(rkCharGuid, kCharName);
	}
	if( !bFindName )
	{
		PgPilot *pkPilot = g_kPilotMan.FindPilot(rkCharGuid);
		if( pkPilot )
		{
			kCharName = pkPilot->GetName();
		}
		bFindName = NULL != pkPilot;
	}
	
	std::wstring kTempMsg;
	if( FormatTTW(kTempMsg, 700078, kCharName.c_str()) )
	{
		SChatLog kChatLog(CT_EVENT);
		g_kChatMgrClient.AddLogMessage(kChatLog, kTempMsg, true);
	}
}

void PgGuildMgr::ReqJoinGuild(std::wstring const &rkCharName)//이름으로 초대
{
	if(!g_pkWorld)
	{
		return;
	}
	BM::GUID kMyCharGuid;
	CUnit* pkUnit = g_kPilotMan.GetPlayerUnit();
	if( pkUnit
	&&	pkUnit->Name() == rkCharName )
	{
		SChatLog kChatLog(CT_EVENT);
		g_kChatMgrClient.AddMessage(700083, kChatLog, true);
		return ;//자기 자신 초대는 안되
	}

	if( !PgGuildMgrUtil::IsCanReqInOutState(g_pkWorld->GetAttr()) )
	{
		SChatLog kChatLog(CT_EVENT);
		g_kChatMgrClient.AddMessage(400845, kChatLog, true);
		return;
	}

	BM::Stream kPacket(PT_C_N_REQ_GUILD_COMMAND, (BYTE)GC_ReqJoin_Name);
	kPacket.Push(rkCharName);
	NETWORK_SEND(kPacket)

	std::wstring kTempMsg;
	if( FormatTTW(kTempMsg, 700078, rkCharName.c_str()) )
	{
		SChatLog kChatLog(CT_EVENT);
		g_kChatMgrClient.AddLogMessage(kChatLog, kTempMsg, true);
	}
}

void PgGuildMgr::ReqJoinAnswer(BM::GUID const &rkGuildGuid, bool const bJoin)
{
	BM::Stream kPacket(PT_C_N_REQ_GUILD_COMMAND, (BYTE)GC_Join);
	kPacket.Push(rkGuildGuid);
	kPacket.Push(bJoin);
	NETWORK_SEND(kPacket)
}

bool SortGuildMember(SGuildMemberInfo const &rkLeft, SGuildMemberInfo const &rkRight)
{
	//if( 0 != rkLeft.kPlace.iNo && 0 == rkRight.kPlace.iNo )				{ return true; }
	//if( 0 == rkLeft.kPlace.iNo && 0 != rkRight.kPlace.iNo )				{ return false; }
	if( 0 != rkLeft.kGndKey.GroundNo() && 0 == rkRight.kGndKey.GroundNo() )	{ return true; }
	if( 0 == rkLeft.kGndKey.GroundNo() && 0 != rkRight.kGndKey.GroundNo() )	{ return false; }
	if( rkLeft.cGrade > rkRight.cGrade )									{ return false; }
	if( rkLeft.cGrade < rkRight.cGrade )									{ return true; }
	if( rkLeft.kCharName < rkRight.kCharName )								{ return true; }
	return false;
}

bool PgGuildMgr::UpdateGuildUI(XUI::CXUI_Wnd* pkGuildTopWnd) const
{
	bool const bHaveGuild = IamHaveGuild();
	if( bHaveGuild//길드가 있고
	&& Wait() )//대기중이면
	{
		return false;//다음에 업데이트
	}

	PgContentsBase::Status kStatus = PgContentsBase::GetContentsStatus();
	if( PgContentsBase::STATUS_LOBBY != kStatus
	&&	PgContentsBase::STATUS_ROOM != kStatus )
	{
		if ( !pkGuildTopWnd )
		{
			if ( !XUIMgr.IsActivate( _T("SFRM_MAIN_GUILD"), pkGuildTopWnd ) )
			{
				return false;
			}
		}

		PgPlayer *pkPC = g_kPilotMan.GetPlayerUnit();
		if( !pkPC )
		{
			return false;
		}

		PgMySkill *pkMySkill = pkPC->GetMySkill();
		if( !pkMySkill )
		{
			return false;
		}

		XUI::CXUI_Wnd *pkGuildNameWnd = pkGuildTopWnd->GetControl(_T("FRM_NAME"));//길드 이름
		XUI::CXUI_Wnd *pkGuildLevelWnd = pkGuildTopWnd->GetControl(_T("FRM_LEVEL"));//길드 레벨
		XUI::CXUI_Wnd *pkGuildEmblemWnd = pkGuildTopWnd->GetControl(_T("IMG_GUILD_MARK"));//길드 엠블렘
		XUI::CXUI_AniBar *pkGuildExpBarWnd = dynamic_cast<XUI::CXUI_AniBar*>(pkGuildTopWnd->GetControl(_T("ANIBAR_GUILD_EXP")));//길드 경험치 바
		XUI::CXUI_List *pkGuildMemberListWnd = dynamic_cast<XUI::CXUI_List*>(pkGuildTopWnd->GetControl(_T("LST_GUILD_LIST")));
		XUI::CXUI_Button* pkGuildSkillBtn = dynamic_cast<XUI::CXUI_Button*>(pkGuildTopWnd->GetControl(_T("BTN_SKILL")));
		XUI::CXUI_Button* pkGuildEntityBtn = dynamic_cast<XUI::CXUI_Button*>(pkGuildTopWnd->GetControl(_T("BTN_ENTITY")));
		XUI::CXUI_Button *pkGuildKickBtn = dynamic_cast<XUI::CXUI_Button*>(pkGuildTopWnd->GetControl(_T("BTN_GUILD_KICK")));
		XUI::CXUI_Button *pkGuildLeaveBtn = dynamic_cast<XUI::CXUI_Button*>(pkGuildTopWnd->GetControl(_T("BTN_GUILD_LEAVE")));
		XUI::CXUI_Button *pkGuildInviteBtn = dynamic_cast<XUI::CXUI_Button*>(pkGuildTopWnd->GetControl(_T("BTN_GUILD_INVITE")));
		XUI::CXUI_Button* pkGuildEmproiaBoard = dynamic_cast<XUI::CXUI_Button*>(pkGuildTopWnd->GetControl(_T("BTN_EMPORIA_BOARD")));
		XUI::CXUI_Button* pkGuildEBattleJoin = dynamic_cast<XUI::CXUI_Button*>(pkGuildTopWnd->GetControl(_T("BTN_EMBATTLE_JOIN")));
		XUI::CXUI_CheckButton *pkGradeBtn = dynamic_cast<XUI::CXUI_CheckButton*>( pkGuildTopWnd->GetControl(_T("BTN_GRADE")) );
		XUI::CXUI_CheckButton *pkLocationBtn = dynamic_cast<XUI::CXUI_CheckButton*>( pkGuildTopWnd->GetControl(_T("BTN_LOCATION")) );
		XUI::CXUI_Wnd *pkListTitleLevel = pkGuildTopWnd->GetControl( _T("SFRM_TITLE_LEVEL") );//계급정보 모드
		XUI::CXUI_Wnd *pkListTitleGrade = pkGuildTopWnd->GetControl( _T("SFRM_TITLE_GRADE") );
		XUI::CXUI_Wnd *pkListTitleChannel = pkGuildTopWnd->GetControl( _T("SFRM_TITLE_CHANNEL") );//위치정보 모드
		XUI::CXUI_Wnd *pkListTitleLocation = pkGuildTopWnd->GetControl( _T("SFRM_TITLE_LOCATION") );

		if( pkGuildInviteBtn ) {pkGuildInviteBtn->Visible(false);}//버튼들 기본 비활성화
		if( pkGuildLeaveBtn ) {pkGuildLeaveBtn->Visible(false);}
		if( pkGuildKickBtn ) {pkGuildKickBtn->Visible(false);}
		if( !pkGuildMemberListWnd ) {return false;}

		if( !bHaveGuild )
		{
			pkGuildMemberListWnd->ClearList();//길드 멤버 초기화
		}

		if( pkGuildNameWnd )//이름
		{
			if( bHaveGuild )
			{
				pkGuildNameWnd->Text(m_kBasicInfo.kGuildName);
			}
			else
			{
				pkGuildNameWnd->Text(TTW(400562));//길드 없음
			}
		}

		if( pkGuildLevelWnd )//길드 레벨
		{
			if( bHaveGuild )
			{
				BM::vstring kLevel(m_kBasicInfo.sLevel);
				pkGuildLevelWnd->Text(kLevel);
			}
			else
			{
				pkGuildLevelWnd->Text(TTW(400222));
			}
		}
		if( pkGuildExpBarWnd )//길드 경험치
		{
			//pkGuildExpBarWnd->Visible(bHaveGuild);
			int const iMax = 10000;
			int iCur = 0;
			if( bHaveGuild )
			{
				__int64 iNextExp = 0;
				CONT_DEF_GUILD_LEVEL const *pkDefGuildLevel = NULL;
				g_kTblDataMgr.GetContDef(pkDefGuildLevel);
				if( pkDefGuildLevel )
				{
					//다음 레벨이 없다
					CONT_DEF_GUILD_LEVEL::const_iterator def_iter = pkDefGuildLevel->find(m_kBasicInfo.sLevel);
					if( pkDefGuildLevel->end() != def_iter )
					{
						CONT_DEF_GUILD_LEVEL::const_iterator nextdef_iter = pkDefGuildLevel->find(m_kBasicInfo.sLevel + 1);
						if( pkDefGuildLevel->end() != nextdef_iter )
						{
							iNextExp = (*nextdef_iter).second.iExp;
						}
						else
						{
							iNextExp = (*def_iter).second.iExp;
						}
					}
				}

				if( iNextExp )
				{
					iCur = (int)(((double)m_kBasicInfo.iExperience / iNextExp) * iMax);
				}
				pkGuildExpBarWnd->Max(iMax);
				pkGuildExpBarWnd->Now(__min(iMax, iCur));

				float const fMaxPercent = 100.f;
				TCHAR szTemp[512] = {0, };
				_stprintf_s(szTemp, _T("%I64d / %I64d (%.2f%%)"), m_kBasicInfo.iExperience, iNextExp, __min(iCur / fMaxPercent, fMaxPercent));

				pkGuildExpBarWnd->Text( std::wstring(szTemp) );
			}
			else
			{
				pkGuildExpBarWnd->Max(iMax);
				pkGuildExpBarWnd->Now(0);
				pkGuildExpBarWnd->Text( std::wstring() );
			}
			pkGuildExpBarWnd->SetInvalidate();
		}

		if( pkGuildEmblemWnd )//길드 엠블렘
		{
			SUVInfo kUVInfo = pkGuildEmblemWnd->UVInfo();
			kUVInfo.Index = m_kBasicInfo.cEmblem + 1;
			pkGuildEmblemWnd->UVInfo(kUVInfo);
			pkGuildEmblemWnd->Visible(bHaveGuild);
			pkGuildEmblemWnd->SetInvalidate();
		}

		//길마인지?
		BM::GUID rkMyCharGuid;
		bool const bFindMyPilot = g_kPilotMan.GetPlayerPilotGuid(rkMyCharGuid);
		if( !bFindMyPilot )
		{
			return false;//내캐릭터가 없어서 무효
		}

		bool const bOwner = IsOwner(rkMyCharGuid);
		bool const bMaster = IsMaster(rkMyCharGuid);

		XUI::CXUI_Wnd *pkGuildNotice = pkGuildTopWnd->GetControl(_T("SFRM_GUILD_NOTICE"));
		if( pkGuildNotice )
		{//길드 공지
			XUI::CXUI_Wnd* pkNotice = pkGuildNotice->GetControl(L"FRM_NOTICE");
			if( pkNotice )
			{
				pkNotice->Text(GuildNotice());
			}

			XUI::CXUI_Wnd* pkEdit = pkGuildNotice->GetControl(L"BTN_EDIT");
			if( pkEdit )
			{//길마나 마스터만 보여준다.
				pkEdit->Visible( bOwner || bMaster );	
			}
		}

		//기능 버튼
		if( pkGuildSkillBtn )		{ pkGuildSkillBtn->Visible(true);		pkGuildSkillBtn->Disable(!bHaveGuild); }
		if( pkGuildEntityBtn )		{ pkGuildEntityBtn->Visible(true);		pkGuildEntityBtn->Disable( (!bOwner) && (!bMaster) ); }//길드설정은 오너, 마스터 권한
		if( pkGuildKickBtn )		{ pkGuildKickBtn->Visible(bOwner);		pkGuildKickBtn->Disable(!bOwner); } //추방은 오너 권한
		if( pkGuildLeaveBtn )		{ pkGuildLeaveBtn->Visible(!bOwner);	pkGuildLeaveBtn->Disable(bOwner || !bHaveGuild); }
		if( pkGuildInviteBtn )		{ pkGuildInviteBtn->Visible(true);		pkGuildInviteBtn->Disable(!bMaster); } //초대 마스터 권한
		if( pkGuildEmproiaBoard )	{ pkGuildEmproiaBoard->Visible(true);	pkGuildEmproiaBoard->Disable(false); }
		if( pkGuildEBattleJoin )
		{ 
			pkGuildEBattleJoin->Visible(true);	
			lua_tinker::call< void, lwUIWnd, BYTE >("RefreshEmporiaJoinBtn", lwUIWnd(pkGuildEBattleJoin), m_kEmporiaInfo.byType );
		}

		if( !bHaveGuild ) {return false;}//길드가 없으면 아래는 무시

		if( m_kMember.empty()//멤버가 비고
		||	m_kMemberGrade.empty()//멤버 등급표가 비었으면
		||	!pkGuildMemberListWnd )
		{
			//ReqMemberInfo(m_kBasicInfo.kGuildGuid);//요청
			return false;//실패
		}

		//
		bool const bListGradeMode = ListMode() == GLM_Grade;
		bool const bListLocationMode = ListMode() == GLM_Location;
		if( pkGradeBtn )			{ pkGradeBtn->Check(bListGradeMode); }
		if( pkLocationBtn )			{ pkLocationBtn->Check(bListLocationMode); }
		if( pkListTitleLevel )		{ pkListTitleLevel->Visible(bListGradeMode); }
		if( pkListTitleGrade )		{ pkListTitleGrade->Visible(bListGradeMode); }
		if( pkListTitleChannel )	{ pkListTitleChannel->Visible(bListLocationMode); }
		if( pkListTitleLocation )	{ pkListTitleLocation->Visible(bListLocationMode); }

		typedef std::list< SGuildMemberInfo > ContGuildMemberList;
		ContGuildMemberList kSortedMemberList;
		//맴버 목록 (소트)
		ContGuildMember::const_iterator member_iter = m_kMember.begin();
		while(m_kMember.end() != member_iter)
		{
			const ContGuildMember::mapped_type& rkElement = (*member_iter).second;
			std::back_inserter(kSortedMemberList) = rkElement;
			++member_iter;
		}
		kSortedMemberList.sort( SortGuildMember );

		typedef std::list< XUI::SListItem* > CONT_XUI_LIST_ITEM;
		CONT_XUI_LIST_ITEM kContMemberWnd;
		{
			XUI::SListItem* pkBegin = pkGuildMemberListWnd->FirstItem();
			while( NULL != pkBegin )
			{
				kContMemberWnd.push_back( pkBegin );
				pkBegin = pkGuildMemberListWnd->NextItem(pkBegin);
			}
		}

		//맴버 목록 업데이트
		ContGuildMemberList::const_iterator sorted_iter = kSortedMemberList.begin();
		while(kSortedMemberList.end() != sorted_iter)
		{
			const ContGuildMemberList::value_type& rkElement = (*sorted_iter);

			XUI::SListItem* pkNewItem = NULL;
			if( !kContMemberWnd.empty() )
			{
				pkNewItem = kContMemberWnd.front();
				kContMemberWnd.pop_front();
			}
			else
			{
				pkNewItem = pkGuildMemberListWnd->AddItem(rkElement.kCharName);
			}
			if( pkNewItem )
			{
				XUI::CXUI_Wnd* pkNewItemWnd = pkNewItem->m_pWnd;
				if( pkNewItemWnd )
				{
					bool const bLoingedCharacter = 0 != rkElement.kGndKey.GroundNo();
					pkNewItemWnd->OwnerGuid( rkElement.kCharGuid );

					XUI::CXUI_Wnd *pkCharNameWnd = pkNewItemWnd->GetControl(_T("FRM_NAME"));
					XUI::CXUI_Wnd *pkClassImgWnd = pkNewItemWnd->GetControl(_T("IMG_CLASS"));
					XUI::CXUI_Wnd *pkLevelWnd = pkNewItemWnd->GetControl(_T("FRM_LEVEL"));
					XUI::CXUI_Wnd *pkGradeWnd = pkNewItemWnd->GetControl(_T("FRM_GRADE"));
					XUI::CXUI_Wnd *pkChannelWnd = pkNewItemWnd->GetControl(_T("FRM_CHANNEL"));
					XUI::CXUI_Wnd *pkLocationWnd = pkNewItemWnd->GetControl(_T("FRM_LOCATION"));
					//XUI::CXUI_Button* pkMyHomeBtn = dynamic_cast<XUI::CXUI_Button*>(pkNewItemWnd->GetControl(L"BTN_GO_HOME"));

					if( pkCharNameWnd )
					{
						pkCharNameWnd->Text( rkElement.kCharName );
					}
					if( pkClassImgWnd )
					{
						lwSetMiniClassIconIndex( pkClassImgWnd, static_cast<int>(rkElement.cClass) );
						pkClassImgWnd->GrayScale( !bLoingedCharacter );
					}

					if( pkLevelWnd )
					{
						BM::vstring kLevel(rkElement.sLevel);
						pkLevelWnd->Text( kLevel );
						pkLevelWnd->Visible( bListGradeMode );
					}
					std::wstring kGradeName;
					bool const bFindGrade = GetGradeName(rkElement.cGrade, kGradeName);
					if( pkGradeWnd )
					{
						pkGradeWnd->Text( (bFindGrade)? kGradeName: std::wstring() );
						pkGradeWnd->Visible( bListGradeMode );
					}

					if( pkChannelWnd )
					{
						BM::vstring kChannel(rkElement.sChannel);
						pkChannelWnd->Text( (bLoingedCharacter)? kChannel: std::wstring() );
						pkChannelWnd->Visible( bListLocationMode );
					}
					if( pkLocationWnd )
					{
						std::wstring const kMapName = (rkElement.kGndKey.GroundNo())? GetMapName(rkElement.kGndKey.GroundNo()): TTW(27);						
						int const iWidth = pkLocationWnd->Width()+REGULATION_UI_WIDTH;
						Quest::SetCutedTextLimitLength(pkLocationWnd,kMapName, _T("..."));						
						pkLocationWnd->Visible( bListLocationMode );
					}
					XUI::CXUI_Wnd* pkMyHomeGoWnd = pkNewItemWnd->GetControl( L"BTN_MYHOME_GO_NONE" );
					if( !pkMyHomeGoWnd )
					{
						return false;
					}
					XUI::CXUI_Button* pkMyHomeGoBtn = dynamic_cast<XUI::CXUI_Button*>( pkMyHomeGoWnd );
					if( !pkMyHomeGoBtn )
					{
						return false;
					}
					pkMyHomeGoBtn->Disable( rkElement.kHomeAddr.IsNull() );
				}
			}

			++sorted_iter;
		}

		{
			CONT_XUI_LIST_ITEM::iterator erase_iter = kContMemberWnd.begin();
			while( kContMemberWnd.end() != erase_iter )
			{
				pkGuildMemberListWnd->DeleteItem((*erase_iter));
				++erase_iter;
			}
		}
	}
	else
	{
		/*
		if ( !pkCommunityTopWnd )
		{
			if ( PgContentsBase::STATUS_LOBBY == kStatus )
			{
				if ( !XUIMgr.IsActivate( PgContentsBase::ms_FRM_PVP_LOBBY, pkCommunityTopWnd ) )
				{
					return false;
				}
			}
			else
			{
 				if ( !XUIMgr.IsActivate( PgContentsBase::ms_FRM_PVP_ROOM[PVP_MODE_TEAM], pkCommunityTopWnd ) )
 				{
					if ( !XUIMgr.IsActivate( PgContentsBase::ms_FRM_PVP_ROOM[PVP_MODE_PERSONAL], pkCommunityTopWnd ) )
					{
						return false;
					}
 				}
			}

			pkCommunityTopWnd = pkCommunityTopWnd->GetControl(_T("FRM_COMMUNITY"));
			if ( pkCommunityTopWnd )
			{
				pkCommunityTopWnd = pkCommunityTopWnd->GetControl(_T("FRM_2"));
				if ( pkCommunityTopWnd && !pkCommunityTopWnd->Visible() )
				{
					pkCommunityTopWnd = NULL;
				}
			}

			if ( !pkCommunityTopWnd )
			{
				return false;
			}
		}

		XUI::CXUI_List *pList = dynamic_cast<XUI::CXUI_List*>(pkCommunityTopWnd->GetControl(_T("LST_LIST")));
		if ( !pList )
		{
			return false;
		}

		pList->ClearList();

		//맴버 목록 업데이트
		ContGuildMember::const_iterator member_itr;
		for(member_itr=m_kMember.begin();member_itr!=m_kMember.end();++member_itr)
		{
			const ContGuildMember::mapped_type& rkElement = member_itr->second;
			if ( g_kPilotMan.IsMyPlayer(rkElement.kCharGuid) )
			{
				continue;
			}

			XUI::SListItem *pItem = pList->AddItem(rkElement.kCharName);
			if ( pItem )
			{
				pItem->m_pWnd->Text(rkElement.kCharName);
				pItem->m_pWnd->Enable(false);

				std::wstring wstrText;

				XUI::CXUI_Wnd* pkStatusWnd = pItem->m_pWnd->GetControl(_T("IMG_STATUS"));
				if ( pkStatusWnd )
				{
					switch( rkElement.kGndKey.GroundNo() )
					{
					case 0:
						{
							pkStatusWnd->UVUpdate(FLS_OFFLINE);

							wstrText = TTW(200004);

							int iText = 200116;
							pItem->m_pWnd->SetCustomData( &iText, sizeof(int) );
						}break;
					case PvP_Lobby_GroundNo:
						{
							pkStatusWnd->UVUpdate(FLS_ONLINE);
							wstrText = TTW(200001);
							pItem->m_pWnd->Enable(true);

							if ( PgContentsBase::STATUS_LOBBY == kStatus )
							{
								BM::Stream kPacket(PT_C_T_REQ_WITH_PVP, rkElement.kCharGuid );
								pItem->m_pWnd->SetCustomData( &(kPacket.Data().at(0)), kPacket.Size() );
							}
							else
							{
								BM::Stream kPacket(PT_C_T_REQ_INVITE_PVP, rkElement.kCharGuid );
								pItem->m_pWnd->SetCustomData( &(kPacket.Data().at(0)), kPacket.Size() );
							}
						}break;
					default:
						{
							pkStatusWnd->UVUpdate(FLS_ONLINE);
							if ( GATTR_PVP == lwGetMapAttrByNo( rkElement.kGndKey.GroundNo() ) )
							{
								wstrText = TTW(200002);

								if ( PgContentsBase::STATUS_LOBBY == kStatus )
								{
									BM::Stream kPacket(PT_C_T_REQ_WITH_PVP, rkElement.kCharGuid );
									pItem->m_pWnd->SetCustomData( &(kPacket.Data().at(0)), kPacket.Size() );
								}
							}
							else
							{
								wstrText = TTW(200004);

								int iText = 200116;
								pItem->m_pWnd->SetCustomData( &iText, sizeof(int) );
							}
						}break;
					}		
				}


				XUI::CXUI_Wnd* pkWnd = pItem->m_pWnd->GetControl(_T("FRM_POS"));
				if ( pkWnd )
				{
					pkWnd->Text(wstrText);
				}
			}
		}
		*/
	}

	return true;//업데이트 성공
}

bool PgGuildMgr::UpdateGuildSkillUI(XUI::CXUI_Wnd* pkSkillTopWnd) const
{
	bool const bHaveGuild = IamHaveGuild();

	if( !pkSkillTopWnd )
	{
		pkSkillTopWnd = XUIMgr.Get( _T("SFRM_GUILD_SKILL") );
		if( !pkSkillTopWnd )
		{
			return false;
		}
	}

	PgPlayer *pkPC = g_kPilotMan.GetPlayerUnit();
	if( !pkPC )
	{
		return false;
	}

	PgMySkill *pkMySkill = pkPC->GetMySkill();
	if( !pkMySkill )
	{
		return false;
	}

	typedef std::vector< XUI::CXUI_Icon* > ContGuildSkillWnd;
	ContGuildSkillWnd kSkillWnd;
	std::back_inserter(kSkillWnd) = dynamic_cast<XUI::CXUI_Icon*>(pkSkillTopWnd->GetControl(_T("ICON_GUILD_SKILL_1")));//길드스킬
	std::back_inserter(kSkillWnd) = dynamic_cast<XUI::CXUI_Icon*>(pkSkillTopWnd->GetControl(_T("ICON_GUILD_SKILL_2")));//길드스킬
	std::back_inserter(kSkillWnd) = dynamic_cast<XUI::CXUI_Icon*>(pkSkillTopWnd->GetControl(_T("ICON_GUILD_SKILL_3")));//길드스킬
	std::back_inserter(kSkillWnd) = dynamic_cast<XUI::CXUI_Icon*>(pkSkillTopWnd->GetControl(_T("ICON_GUILD_SKILL_4")));//길드스킬

	//if( bHaveGuild )//길드 스킬
	{
		GET_DEF(CSkillDefMgr, kSkillDefMgr);
		//ContSkillNo kGuildSkills;
		//pkMySkill->GetHaveSkills(SDT_Guild, kGuildSkills);
		size_t iCur = 0;
		ContGuildSkillWnd::iterator wnd_iter = kSkillWnd.begin();
		while(kSkillWnd.end() != wnd_iter)
		{
			ContGuildSkillWnd::value_type pkSkillIconWnd = (*wnd_iter);
			if( pkSkillIconWnd )
			{
				int const iSkillNo = pkSkillIconWnd->IconInfo().iIconKey;
				const CSkillDef* pkSkillDef = kSkillDefMgr.GetDef(iSkillNo);
				if( pkSkillDef )
				{
					SIconInfo kSkillIconInfo(KUIG_SKILLTREE, iSkillNo);
					kSkillIconInfo.iIconResNumber = pkSkillDef->RscNameNo();
					pkSkillIconWnd->SetIconInfo(kSkillIconInfo);
				}

				pkSkillIconWnd->GrayScale( !pkMySkill->IsExist(iSkillNo) );
			}
			
			++wnd_iter;
		}
	}
	return true;
}

void PgGuildMgr::ReadEntranceOpenGuildFromPacket(BM::Stream& rkPacket)
{
	BM::CAutoMutex kLock(m_kMutex);

	PgPlayer *pkPC = g_kPilotMan.GetPlayerUnit();
	if( !pkPC )
	{
		return ;
	}

	m_kEntranceOpenGuild.clear();

	size_t iCount = 0;
	rkPacket.Pop( iCount );

	while( iCount-- )
	{
		SEntranceOpenGuild kEntranceOpenGuild;
		kEntranceOpenGuild.ReadFromPacket(rkPacket);
		m_kEntranceOpenGuild.push_back(kEntranceOpenGuild);
	}

	rkPacket.Pop(m_iRecommendGuildRank); // 추천길드의 랭킹
	WrapperUpdateEntranceOpenGuildUI();
}

void PgGuildMgr::AddEntranceApplicant(const SGuildEntranceApplicant& kNewApplicant)
{
	m_kGuildEntranceApplicant.insert(std::make_pair(kNewApplicant.kCharGuid, kNewApplicant));
}

void PgGuildMgr::DelEntranceApplicant(BM::GUID const& rkCharGuid)
{
	CONT_MY_GUILD_ENTRANCE_APPLICANTS::iterator find_iter = m_kGuildEntranceApplicant.find(rkCharGuid);
	if( find_iter != m_kGuildEntranceApplicant.end() )
	{
		m_kGuildEntranceApplicant.erase(find_iter);
	}

	lwGuild::GuildApplicantSetting();
}

void PgGuildMgr::ReadGuildEntranceApplicantListFromPacket(BM::Stream& rkPacket)
{
	BM::CAutoMutex kLock(m_kMutex);

	PgPlayer *pkPC = g_kPilotMan.GetPlayerUnit();
	if( !pkPC )
	{
		return ;
	}

	//Contents 서버의 PgGuildMgr::WriteGuildEntranceApplicantListToPacket()과 한쌍
	size_t iCount = 0;
	rkPacket.Pop( iCount );
	int iNewApplicantCount = 0;

	m_kGuildEntranceApplicant.clear();
	
	while( iCount-- )
	{
		SGuildEntranceApplicant kGuildEntranceApplicant;
		kGuildEntranceApplicant.ReadFromPacket(rkPacket);
		AddEntranceApplicant(kGuildEntranceApplicant);
	}

	CONT_MY_GUILD_ENTRANCE_APPLICANTS::iterator loop_iter = m_kGuildEntranceApplicant.begin();
	while( m_kGuildEntranceApplicant.end() != loop_iter )
	{
		CONT_MY_GUILD_ENTRANCE_APPLICANTS::mapped_type kElement = (*loop_iter).second;
		if( kElement.byState == AS_NONE )
		{// 처리하지 않은 길드가입 신청서의 수
			++iNewApplicantCount;
		}		
		++loop_iter;
	}

	if( iNewApplicantCount
	&&	IsMaster( pkPC->GetID() ) )
	{
		BM::vstring vStr(TTW(400464));
		vStr.Replace(L"#COUNT#", iNewApplicantCount);	
		SChatLog kChatLog(CT_GUILD);
		g_kChatMgrClient.AddLogMessage(kChatLog, vStr);
	}
}

void PgGuildMgr::ReadListFromPacket(BM::Stream& rkPacket, bool const bUpdate)
{
	BM::CAutoMutex kLock(m_kMutex);

	//Center 서버에 PgGuild::WriteListToPacket()과 한쌍
	BYTE cGradeCount = 0;
	if ( !bUpdate )
	{
		m_kMember.clear();
		m_kMemberGrade.clear();

		rkPacket.Pop(cGradeCount);
		ContGuildGrade::value_type kTempGrade;
		for(unsigned int iCur=0; cGradeCount>iCur; ++iCur)
		{
			kTempGrade.ReadFromPacket(rkPacket);
			m_kMemberGrade.push_back(kTempGrade);
		}
	}

	size_t iCount = 0;
	rkPacket.Pop( iCount );

	ContGuildMember::key_type kKey;
	ContGuildMember::mapped_type kMemberInfo;
	while( iCount-- )
	{
		kKey.ReadFromPacket( rkPacket );
		kMemberInfo.ReadFromPacket( rkPacket );

		std::pair<ContGuildMember::iterator, bool> kRet = m_kMember.insert( std::make_pair(kKey, kMemberInfo) );
		if( !kRet.second
		&&	bUpdate )
		{
			bool bLogin = bUpdate && (0 == (*kRet.first).second.kGndKey.GroundNo() && 0 != kMemberInfo.kGndKey.GroundNo());
			bool bLogout = bUpdate && (0 != (*kRet.first).second.kGndKey.GroundNo() && 0 == kMemberInfo.kGndKey.GroundNo());
			
			(*kRet.first).second = kMemberInfo;

			std::wstring kMsg;
			if( bUpdate && bLogin )
			{
				FormatTTW(kMsg, 400627, kMemberInfo.kCharName.c_str());
				SUVInfo kUVInfo;
				kUVInfo.Index = m_kBasicInfo.cEmblem + 1;
				if( (0 != g_kGlobalOption.GetValue(XML_ELEMENT_ETC, "NOTIFY_GUILD"))
					&& m_kBasicInfo.kGuildGuid.IsNotNull() )
				{
					g_kChatMgrClient.ShowNotifyConnectInfo(NCI_GUILD, kMemberInfo.kCharName, kMemberInfo.cClass, kUVInfo, kMemberInfo.kCharGuid);
				}
			}
			else if( bUpdate && bLogout )
			{
				FormatTTW(kMsg, 400628, kMemberInfo.kCharName.c_str());
			}
			if( !kMsg.empty() )
			{
				SChatLog kChatLog(CT_EVENT);
				g_kChatMgrClient.AddLogMessage(kChatLog, kMsg);
			}
		}
	}

	ContGuildMember::const_iterator iter = m_kMember.begin();
	while( m_kMember.end() != iter )
	{
		ContGuildMember::mapped_type const &rkMember = (*iter).second;
		if( 0 != rkMember.kGndKey.GroundNo() )
		{
			g_kChatMgrClient.Name2Guid_Add( CT_GUILD, rkMember.kCharName, rkMember.kCharGuid );
		}
		++iter;
	}
}

void PgGuildMgr::ReadEmporiaInfoFromPacket( BM::Stream &rkPacket )
{
	rkPacket.Pop( m_kEmporiaInfo );

	XUI::CXUI_Wnd *pkCommunityWnd = XUIMgr.Get(_T("SFRM_COMMUNITY") );
	if ( pkCommunityWnd )
	{
		pkCommunityWnd = pkCommunityWnd->GetControl( _T("FRM_GUILD") );
		if ( pkCommunityWnd )
		{
			pkCommunityWnd = pkCommunityWnd->GetControl( _T("BTN_EMBATTLE_JOIN") );
			if ( pkCommunityWnd )
			{
				lua_tinker::call< void, lwUIWnd, BYTE >("RefreshEmporiaJoinBtn", lwUIWnd(pkCommunityWnd), m_kEmporiaInfo.byType );
			}
		}
	}

	if ( true == m_kEmporiaInfo.IsNowBattle() )
	{
		__int64 const i64BattleTime = m_kEmporiaInfo.i64BattleTime;
		__int64 const i64NowTime = g_kEventView.GetLocalSecTime(CGameTime::DEFAULT);
		DWORD const iTime = (i64BattleTime-i64NowTime) / CGameTime::SECOND;
		std::wstring kRemainTime;
		TimeToString(iTime, kRemainTime);
		BM::vstring vTime(TTW(73099));
		vTime.Replace(L"#TIME#", kRemainTime);

		SMtoMChatData kChatData;
		kChatData.kName = TTW(400445);
		kChatData.kContants = std::wstring(vTime);
		kChatData.dwColor = 0xFFFFFF99;
		g_kMToMMgr.Add( m_kEmporiaInfo.kID, kChatData.kContants, MTM_MSG_EMPORIABATTLE, kChatData, true );
	}
	else
	{
		g_kMToMMgr.DelID( m_kEmporiaInfo.kID );
		g_kMToMMgr.Del( m_kEmporiaInfo.kID );
	}

	g_kEmporiaMgr.RefreshThrowButton( m_kEmporiaInfo.byType );
}

bool PgGuildMgr::GetGradeName(int const iGrade, std::wstring& rkOut) const
{
	int const iMaxGrade = (m_kMemberGrade.size() * MAX_GUILDGRADENAME) - 1;
	int const iCurGrade = NiMin(iGrade, iMaxGrade);

	ContGuildGrade::const_iterator grade_iter = m_kMemberGrade.begin();
	while(m_kMemberGrade.end() != grade_iter)
	{
		const ContGuildGrade::value_type& rkElement = (*grade_iter);
		bool const bFindGrade = rkElement.Get(iCurGrade, rkOut);
		if( bFindGrade )
		{
			return true;//찾았다
		}
		++grade_iter;
	}
	return false;//실패
}

bool PgGuildMgr::IsOwner(BM::GUID const &rkCharGuid) const
{
	return m_kBasicInfo.kMasterGuid == rkCharGuid;//오너
}

bool PgGuildMgr::IsMaster(BM::GUID const &rkCharGuid) const
{
	ContGuildMember::const_iterator iter = m_kMember.find(rkCharGuid);
	if( m_kMember.end() != iter )
	{
		const ContGuildMember::mapped_type& rkElement = (*iter).second;
		return rkElement.cGrade <= GMG_Master;//마스터, 오너는 마스터
	}
	return false;//마스터 아님
}

bool PgGuildMgr::IamHaveGuild() const
{
	return BM::GUID::NullData() != m_kBasicInfo.kGuildGuid;
}

bool PgGuildMgr::HaveGuildInventory()
{
	if( !IamHaveGuild() )
	{
		return false;
	}

	if( 0 == m_kInv.GetMaxIDX(IT_EQUIP) )	
	{
		return false;
	}

	return true;
}

bool PgGuildMgr::AddMember(const SGuildMemberInfo& rkMember)
{
	ContGuildMember::iterator member_iter = m_kMember.find(rkMember.kCharGuid);
	if( m_kMember.end() == member_iter )
	{
		std::pair<ContGuildMember::iterator, bool> kRet = m_kMember.insert(std::make_pair(rkMember.kCharGuid, rkMember));
		if( kRet.second )
		{
			if( 0 != rkMember.kGndKey.GroundNo() )
			{
				g_kChatMgrClient.Name2Guid_Add( CT_GUILD, rkMember.kCharName, rkMember.kCharGuid );
			}
			return true;
		}
	}
	return false;
}

bool PgGuildMgr::DelMember(BM::GUID const &rkCharGuid)
{
	ContGuildMember::iterator member_iter = m_kMember.find(rkCharGuid);
	if( m_kMember.end() != member_iter )
	{
		m_kMember.erase(member_iter);
		return true;
	}
	return false;
}

bool PgGuildMgr::GetMemberByName(std::wstring const &rkName, SGuildMemberInfo& rkMemberInfo) const
{
	ContGuildMember::const_iterator member_iter = m_kMember.begin();
	while(m_kMember.end() != member_iter)
	{
		const ContGuildMember::mapped_type& rkElement = (*member_iter).second;
		if( rkElement.kCharName == rkName )
		{
			rkMemberInfo = rkElement;
			return true;
		}
		++member_iter;
	}
	return false;
}

bool PgGuildMgr::GetMemberByGuid(BM::GUID const &rkCharGuid, SGuildMemberInfo& rkMemberInfo) const
{
	ContGuildMember::const_iterator member_iter = m_kMember.find(rkCharGuid);
	if(m_kMember.end() != member_iter)
	{
		rkMemberInfo = (*member_iter).second;
		return true;
	}
	return false;
}

void PgGuildMgr::ReqGuildAddSkill(int const iSkillNo)
{
	if( !iSkillNo )
	{
		return;
	}

	if( !IamHaveGuild() )
	{
		return;
	}

	PgPlayer *pkPC = g_kPilotMan.GetPlayerUnit();
	if( !pkPC )
	{
		return;
	}

	if( !IsMaster( pkPC->GetID() ) )
	{
		return;
	}

	BM::Stream kPacket(PT_C_N_REQ_GUILD_COMMAND, (BYTE)GC_M_AddSkill);
	kPacket.Push(iSkillNo);
	kPacket.Push(true);//Test
	NETWORK_SEND(kPacket)
}

void PgGuildMgr::RefreshExp( __int64 const i64Exp )
{
	m_kBasicInfo.iExperience = i64Exp;
	if ( m_kBasicInfo.iExperience < 0i64 )
	{
		m_kBasicInfo.iExperience = 0i64;
	}

	XUI::CXUI_Wnd *pkCommunityWnd = NULL;
	if ( XUIMgr.IsActivate( _T("SFRM_COMMUNITY"), pkCommunityWnd ) )
	{
		UpdateGuildUI( pkCommunityWnd );
	}
}

void PgGuildMgr::SetGuildApplicationState(BM::GUID const &rkGuildGuid, BYTE byState, __int64 i64EntranceFee)
{
	m_kApplicationState.kGuildGuid = rkGuildGuid;
	m_kApplicationState.byApplicantionState = byState;

	if( i64EntranceFee > 0i64 )
	{
		m_kApplicationState.i64GuildEntranceFee = i64EntranceFee;
	}

	if( byState != AS_NONE )
	{// 길드가입 신청결과가 도착하였습니다.
		lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 401082, true);
	}

	if( XUIMgr.Get(L"FRM_GUILDWAR") )
	{// 게시판이 떠있는 상태에서만 UI 업데이트
		WrapperUpdateEntranceOpenGuildUI();
	}
}

void PgGuildMgr::ReqGuildEntrance()
{
	PgPlayer *pkPC = g_kPilotMan.GetPlayerUnit();
	if( !pkPC )
	{
		return;
	}

	CXUI_Wnd* pkMainUI = XUIMgr.Get(L"SFRM_GUILD_APPLICANT");
	if( !pkMainUI )
	{
		return ;
	}

	CXUI_Wnd* pkCustomData = pkMainUI->GetControl(L"FRM_HIDDEN_CUSTOMDATA");
	if( !pkCustomData )
	{
		return ;
	}

	CXUI_Edit_MultiLine* pkEditMsg = dynamic_cast<CXUI_Edit_MultiLine*>(pkMainUI->GetControl(L"MEDT_CONTENTS"));
	if( !pkEditMsg )
	{
		return ;
	}

	int iCustomData = 0;
	pkCustomData->GetCustomData(&iCustomData, sizeof(iCustomData));

	// 골드 체크
	__int64 i64Money = pkPC->GetAbil64(AT_MONEY);
	if( i64Money < m_kEntranceOpenGuild.at(iCustomData).i64GuildEntranceFee )
	{
		::Notice_Show( TTW(403078), EL_Warning );
		return ;
	}

	// 레벨 체크
	short sLevel = static_cast<short>(pkPC->GetAbil(AT_LEVEL));
	if( sLevel < m_kEntranceOpenGuild.at(iCustomData).sGuildEntranceLevel )
	{
		::Notice_Show( TTW(400482), EL_Warning );
		return ;
	}

	// 클래스 체크
	BYTE byClass = static_cast<BYTE>(pkPC->GetAbil(AT_CLASS));
	__int64 const i64ClassFlag = GET_CLASS_LIMIT( byClass );
	if( false == (i64ClassFlag & m_kEntranceOpenGuild.at(iCustomData).i64GuildEntranceClass) )
	{
		::Notice_Show( TTW(400483), EL_Warning );
		return ;
	}

	// 메세지 체크
	std::wstring wstrMsg(pkEditMsg->XUI::CXUI_Edit::EditText());
	if( wstrMsg.empty() )
	{
		::Notice_Show( TTW(400485), EL_Warning );
		return ;
	}
	else
	{
		if(g_kClientFS.Filter(wstrMsg, false))
		{// 욕설 필터링		
			::Notice_Show( TTW(600037), EL_Warning );
			return ;
		}
	}

	if( IamHaveGuild() )
	{// 넌 이미 길드가 있어
		::Notice_Show( TTW(400494), EL_Warning );
		return ;
	}

	BM::Stream kPacket(PT_C_N_REQ_GUILD_ENTRANCE);
	kPacket.Push(m_kEntranceOpenGuild.at(iCustomData).kGuildGuid);
	kPacket.Push(pkPC->Name());
	kPacket.Push(sLevel);
	kPacket.Push(byClass);
	kPacket.Push(wstrMsg);
	kPacket.Push(m_kEntranceOpenGuild.at(iCustomData).i64GuildEntranceFee);
	NETWORK_SEND(kPacket);

	// 가입 신청한 길드 상태 저장
	m_kApplicationState.SetState(m_kEntranceOpenGuild.at(iCustomData).kGuildGuid, AS_NONE, m_kEntranceOpenGuild.at(iCustomData).i64GuildEntranceFee);
}

void PgGuildMgr::ResultGuildEntrance(const bool bRet)
{
	if( bRet )
	{
		SChatLog kChatLog(CT_EVENT);
		g_kChatMgrClient.AddLogMessage(kChatLog, TTW(400486), true);
	}
	else
	{
		m_kApplicationState.Init();
		lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 400495, true);
	}

	WrapperUpdateEntranceOpenGuildUI();
	XUIMgr.Close(L"SFRM_GUILD_APPLICANT");
}

void PgGuildMgr::CallGuildApplicationUI(int const iCustomDataAsInt)
{
	if( IamHaveGuild() )
	{// 넌 이미 길드가 있어.
		::Notice_Show( TTW(400494), EL_Warning );
		return ;
	}

	if( m_kEntranceOpenGuild.at(iCustomDataAsInt).kGuildGuid == m_kApplicationState.kGuildGuid )
	{
		switch( m_kApplicationState.byApplicantionState )
		{
		case AS_NONE:
			{// 신청 취소다.
				BM::Stream kPacket(PT_C_N_REQ_GUILD_ENTRANCE_CANCEL);
				lwCallCommonMsgYesNoBox(MB(TTW(400487)), lwPacket(&kPacket), true, MBT_CONFIRM_GUILD_APPLICATION_CANCEL);
				return ;
			}break;
		case AS_ACCEPT:
			{// 최종적으로 길드 가입비용 지불할 것인지 확인
				if( m_kApplicationState.i64GuildEntranceFee > 0 )
				{//가입비용이 있다면 의사를 물어보고
					BM::Stream kPacket(PT_N_C_REQ_PAYMENT_GUILD_ENTRANCE_FEE);
					kPacket.Push(m_kApplicationState.kGuildGuid);
					kPacket.Push(m_kApplicationState.i64GuildEntranceFee);

					typedef std::vector< std::wstring > ContWstr;
					ContWstr kMoneyVec;
					if(BreakMoney_WStr(m_kApplicationState.i64GuildEntranceFee, kMoneyVec))
					{
						BM::vstring vStr(TTW(401087));
						vStr.Replace(L"#GOLD#", kMoneyVec.at(0));
						vStr.Replace(L"#SILVER#", kMoneyVec.at(1));
						vStr.Replace(L"#COOPER#", kMoneyVec.at(2));
						lwCallCommonMsgYesNoBox(MB(vStr), lwPacket(&kPacket), true, MBT_CONFIRM_GUILD_ENTRANCE_FEE, NULL, false);
					}
				}
				else
				{//없으면 바로 가입 패킷 날림
					BM::Stream kPacket(PT_C_N_REQ_GUILD_COMMAND, (BYTE)GC_Join);
					kPacket.Push(m_kApplicationState.kGuildGuid);
					kPacket.Push(true);
					NETWORK_SEND(kPacket);
				}
				return ;
			}break;
		case AS_REJECT: // 거절당했다.
			{// 취소를 날려서 DB에서 삭제를 해야 한다.
				BM::Stream kPacket(PT_C_N_REQ_GUILD_ENTRANCE_CANCEL);
				NETWORK_SEND(kPacket);
				
				InitApplicantState();
				WrapperUpdateEntranceOpenGuildUI();
				::Notice_Show( TTW(400475), 0 ); // 가입신청 거절당했다.
				return ;
			}
		default:
			{

			}break;
		}
	}
	else if( m_kApplicationState.kGuildGuid != BM::GUID::NullData() )
	{//동시에 여러 길드 신청은 안된다.
		lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 400489, true);
		return ;
	}	
	else
	{
		CXUI_Wnd* pkMainUI = XUIMgr.Get(L"SFRM_GUILD_APPLICANT");
		if( !pkMainUI )
		{
			pkMainUI = XUIMgr.Call(L"SFRM_GUILD_APPLICANT");
			if( !pkMainUI)
			{
				return ;
			}
		}

		CXUI_Wnd* pkCustomData = pkMainUI->GetControl(L"FRM_HIDDEN_CUSTOMDATA");
		CXUI_Wnd* pkNameTitle = pkMainUI->GetControl(L"FRM_NAME_TITLE");
		CXUI_Wnd* pkLevelTitle = pkMainUI->GetControl(L"FRM_LEVEL_TITLE");
		CXUI_Wnd* pkClassTitle = pkMainUI->GetControl(L"FRM_CLASS_TITLE");
		CXUI_Wnd* pkFeeTitle = pkMainUI->GetControl(L"FRM_FEE_TITLE");
		if(	!pkCustomData 
		||	!pkNameTitle 
		||	!pkLevelTitle 
		||	!pkClassTitle
		||	!pkFeeTitle )
		{
			return ;
		}

		pkCustomData->SetCustomData(&iCustomDataAsInt, sizeof(iCustomDataAsInt)); // 길드가입 신청할 때 필요한 인덱스

		CXUI_Wnd* pkName = pkNameTitle->GetControl(L"FRM_NAME");
		CXUI_Wnd* pkLevel = pkLevelTitle->GetControl(L"FRM_LEVEL");

		CXUI_Image* pkFighter = dynamic_cast<CXUI_Image*>(pkClassTitle->GetControl(L"IMG_CLASS_FIGHTER"));
		CXUI_Image* pkMagician = dynamic_cast<CXUI_Image*>(pkClassTitle->GetControl(L"IMG_CLASS_MAGICIAN"));
		CXUI_Image* pkArcher = dynamic_cast<CXUI_Image*>(pkClassTitle->GetControl(L"IMG_CLASS_ARCHER"));
		CXUI_Image* pkThief = dynamic_cast<CXUI_Image*>(pkClassTitle->GetControl(L"IMG_CLASS_THIEF"));
		CXUI_Image* pkShaman = dynamic_cast<CXUI_Image*>(pkClassTitle->GetControl(L"IMG_CLASS_SHAMAN"));
		CXUI_Image* pkDoubleFighter = dynamic_cast<CXUI_Image*>(pkClassTitle->GetControl(L"IMG_CLASS_DOUBLE_FIGHTER"));

		CXUI_Wnd* pkGold = pkFeeTitle->GetControl(L"FRM_GOLD_TEXT");
		CXUI_Edit_MultiLine* pkEditMsg = dynamic_cast<CXUI_Edit_MultiLine*>(pkMainUI->GetControl(L"MEDT_CONTENTS"));

		if( !pkName
		||	!pkLevel
		||	!pkFighter || !pkMagician || !pkArcher || !pkThief || !pkShaman
		||	!pkGold
		||	!pkEditMsg )
		{
			return ;
		}

		pkName->Text(m_kEntranceOpenGuild.at(iCustomDataAsInt).wstrGuildName);
		BM::vstring vStr(TTW(400469));
		vStr.Replace( L"#LEVEL#", m_kEntranceOpenGuild.at(iCustomDataAsInt).sGuildEntranceLevel );
		pkLevel->Text(vStr);

		pkFighter->GrayScale(!((m_kEntranceOpenGuild.at(iCustomDataAsInt).i64GuildEntranceClass & UCLIMIT_MARKET_FIGHTER) == UCLIMIT_MARKET_FIGHTER));
		pkMagician->GrayScale(!((m_kEntranceOpenGuild.at(iCustomDataAsInt).i64GuildEntranceClass & UCLIMIT_MARKET_MAGICIAN) == UCLIMIT_MARKET_MAGICIAN));
		pkArcher->GrayScale(!((m_kEntranceOpenGuild.at(iCustomDataAsInt).i64GuildEntranceClass & UCLIMIT_MARKET_ARCHER) == UCLIMIT_MARKET_ARCHER));
		pkThief->GrayScale(!((m_kEntranceOpenGuild.at(iCustomDataAsInt).i64GuildEntranceClass & UCLIMIT_MARKET_THIEF) == UCLIMIT_MARKET_THIEF));
		pkShaman->GrayScale(!((m_kEntranceOpenGuild.at(iCustomDataAsInt).i64GuildEntranceClass & UCLIMIT_MARKET_SHAMAN) == UCLIMIT_MARKET_SHAMAN));
		if( pkDoubleFighter )
		{
			pkDoubleFighter->GrayScale(!((m_kEntranceOpenGuild.at(iCustomDataAsInt).i64GuildEntranceClass & UCLIMIT_MARKET_DOUBLE_FIGHTER) == UCLIMIT_MARKET_DOUBLE_FIGHTER));
		}

		typedef std::vector< std::wstring > ContWstr;
		ContWstr kMoneyVec;
		if(BreakMoney_WStr(m_kEntranceOpenGuild.at(iCustomDataAsInt).i64GuildEntranceFee, kMoneyVec))
		{
			pkGold->Text(kMoneyVec.at(0));
		}
	}
}

void PgGuildMgr::SearchGuild(lwWString lwSearchGuildName)
{
	const int iLeastLength = 1;

	if( lwSearchGuildName.GetWString().size() < iLeastLength )
	{
		BM::vstring vStr(TTW(400498));
		vStr.Replace(L"#COUNT#", iLeastLength);		
		lua_tinker::call<void, lwWString, bool>("CommonMsgBoxByText", lwWString(static_cast<std::wstring>(vStr)), true);
		return ;
	}
	
	InitSearchGuild();
	m_kSearchEntranceOpenGuild.reserve(10); // 검색 결과 한페이지 정도 확보.
	
	CONT_ENTRANCE_OPEN_GUILD_LIST::const_iterator loop_iter = m_kEntranceOpenGuild.begin();
	while( loop_iter != m_kEntranceOpenGuild.end() )
	{
		CONT_ENTRANCE_OPEN_GUILD_LIST::value_type const kElement = (*loop_iter);
		if( wcsstr( kElement.wstrGuildName.c_str(), lwSearchGuildName.GetWString().c_str() ) )
		{
			m_kSearchEntranceOpenGuild.push_back(kElement);
		}
		
		++loop_iter;
	}

	if( m_kSearchEntranceOpenGuild.empty() )
	{
		lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 400499, true);
		return ;
	}

	WrapperUpdateEntranceOpenGuildUI();
}

void PgGuildMgr::WrapperUpdateEntranceOpenGuildUI(int const iCurrentPage)
{
	if( m_kSearchEntranceOpenGuild.empty() )
	{
		UpdateEntranceOpenGuildUI(iCurrentPage, m_kEntranceOpenGuild);
	}
	else
	{
		UpdateEntranceOpenGuildUI(iCurrentPage, m_kSearchEntranceOpenGuild);
	}
}

void PgGuildMgr::UpdateEntranceOpenGuildUI(int iCurrentPage, const CONT_ENTRANCE_OPEN_GUILD_LIST& rkContGuild)
{
	int const MAX_PAGE_COLUMN = 10; // 한페이지에 표시 될 개수
	int const MAX_PAGE_COUNT = 10; // 표시해야 할 최대 페이지 수

	int const iTotalPage = (rkContGuild.size() % MAX_PAGE_COLUMN) ? ( rkContGuild.size() / MAX_PAGE_COLUMN ) + 1 : ( rkContGuild.size() / MAX_PAGE_COLUMN ); // 총 페이지 수

	if(	iCurrentPage < 0
	||	iTotalPage <= iCurrentPage )
	{
		return ;
	}

	XUI::CXUI_Wnd* pkMainUI = XUIMgr.Get(L"FRM_GUILDWAR");
	if( !pkMainUI )
	{
		pkMainUI = XUIMgr.Call(L"FRM_GUILDWAR");
		if( !pkMainUI )
		{
			return ;
		}		
	}

	XUI::CXUI_Wnd* pkEntranceBoard = pkMainUI->GetControl(L"FRM_ENTRANCE_BOARD");
	if( !pkEntranceBoard )
	{
		return ;
	}

	XUI::CXUI_Wnd* pkFrmPage = pkEntranceBoard->GetControl(L"FRM_PAGE");
	if( !pkFrmPage )
	{
		return ;
	}

	XUI::CXUI_Wnd* pkHiddenCurrentPage = pkEntranceBoard->GetControl(L"FRM_HIDDEN_CURRENT_PAGE");
	if( !pkHiddenCurrentPage )
	{
		return ;
	}
	pkHiddenCurrentPage->SetCustomData(&iCurrentPage, sizeof(iCurrentPage)); // 현재 페이지 저장

	XUI::CXUI_Builder* pkBuildPage = dynamic_cast<XUI::CXUI_Builder*>(pkFrmPage->GetControl(L"BLD_PAGE"));
	if( !pkBuildPage )
	{
		return ;
	}
	
	int iDisplayTotalPage = MAX_PAGE_COUNT < iTotalPage ? MAX_PAGE_COUNT : iTotalPage; // 현재 표시할 페이지 수

	int iTempSize = 0; // 버튼의 가로 크기를 구한다.
	CXUI_CheckButton* pkTemp = dynamic_cast<XUI::CXUI_CheckButton*>(pkFrmPage->GetControl(L"CBTN_PAGE0"));
	if( pkTemp )
	{
		iTempSize = pkTemp->Size().x;
	}
	
	// 페이지 수에 따른 가운데 정렬	
	int iCenterPos = pkEntranceBoard->Width() / 2;
	int iGab = pkBuildPage->GabX();
	int iLocationX = iCenterPos - ( ( iGab / 2 ) * ( iDisplayTotalPage - 1 ) + (iTempSize / 2 ) );	
	pkFrmPage->Location(iLocationX, pkFrmPage->Location().y);

	for( int i = 0; i < MAX_PAGE_COUNT; ++i )
	{
		BM::vstring vStr(L"CBTN_PAGE");
		vStr += i;

		CXUI_CheckButton* pkPageBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pkFrmPage->GetControl(vStr));
		if( pkPageBtn )
		{
			{// 페이지 번호
				BM::vstring vStrPageNum; 
				vStrPageNum = (iCurrentPage / MAX_PAGE_COUNT) * MAX_PAGE_COUNT + 1 + i;
				pkPageBtn->Text(vStrPageNum);
			}

			{// 버튼 클릭 상태 설정
				if( pkPageBtn->BuildIndex() == iCurrentPage )
				{
					pkPageBtn->Check(true);
					pkPageBtn->ClickLock(true);
				}
				else
				{
					pkPageBtn->ClickLock(false);
					pkPageBtn->Check(false);
				}
			}			

			{// 페이지 수만큼만 버튼 보이게
				if( iDisplayTotalPage <= pkPageBtn->BuildIndex() )
				{
					pkPageBtn->Visible(false);
				}
				else
				{
					pkPageBtn->Visible(true);
				}
			}
		}
	}
	
	// 신청한 길드가 있다면 보여주고, 없으면 추천길드를 보여준다.
	CXUI_Wnd* pkTopGuild = pkEntranceBoard->GetControl(L"FRM_TOP_GUILD");
	if( !pkTopGuild )
	{
		return ;
	}

	CXUI_Wnd* pkTopGuildRank = pkTopGuild->GetControl(L"FRM_TOP_RANK");
	CXUI_Wnd* pkTopGuildMark = pkTopGuild->GetControl(L"FRM_TOP_GUILD_MARK");
	CXUI_Wnd* pkTopRecommend = pkTopGuild->GetControl(L"FRM_TOP_RECOMMEND");

	CXUI_Image* pkTopMarkImg = NULL;
	if( pkTopGuildMark )
	{
		pkTopMarkImg = dynamic_cast<CXUI_Image*>(pkTopGuildMark->GetControl(L"IMG_TOP_GUILD_MARK"));
	}

	CXUI_Image* pkTopRecommendImg = NULL;
	if( pkTopRecommend )
	{
		pkTopRecommendImg = dynamic_cast<CXUI_Image*>(pkTopRecommend->GetControl(L"IMG_TOP_RECOMMEND"));
	}

	CXUI_Wnd* pkTopGuildName = pkTopGuild->GetControl(L"FRM_TOP_GUILD_NAME");			
	CXUI_Wnd* pkTopGuildLevel = pkTopGuild->GetControl(L"FRM_TOP_GUILD_LEVEL");
	CXUI_Wnd* pkTopGuildMaster = pkTopGuild->GetControl(L"FRM_TOP_GUILD_MASTER");
	CXUI_Wnd* pkTopGuildMember = pkTopGuild->GetControl(L"FRM_TOP_GUILD_MEMBER");
	CXUI_Wnd* pkTopGuildPR = pkTopGuild->GetControl(L"FRM_TOP_GUILD_PR");
	CXUI_Button* pkTopGuildApplication = dynamic_cast<CXUI_Button*>(pkTopGuild->GetControl(L"BTN_TOP_APPLICATION"));

	if( !pkTopGuildRank
	||	!pkTopMarkImg
	||	!pkTopRecommendImg
	||	!pkTopGuildName
	||	!pkTopGuildLevel
	||	!pkTopGuildMaster
	||	!pkTopGuildMember
	||	!pkTopGuildPR
	||	!pkTopGuildApplication )
	{
		return ;
	}
	
	if( m_kApplicationState.kGuildGuid.IsNotNull() )
	{// 내가 신청한 길드
		bool bFindGuild = false;
		CONT_ENTRANCE_OPEN_GUILD_LIST::iterator iter = m_kEntranceOpenGuild.begin();
		while( m_kEntranceOpenGuild.end() != iter )
		{
			CONT_ENTRANCE_OPEN_GUILD_LIST::value_type kMyApplicantGuild = (*iter); 
			if( kMyApplicantGuild.kGuildGuid == m_kApplicationState.kGuildGuid )
			{
				bFindGuild = true;
				int const iCustomData = kMyApplicantGuild.iRank - 1;

				pkTopRecommend->Visible(false);
				pkTopGuildRank->Text(BM::vstring(kMyApplicantGuild.iRank));
				pkTopGuildRank->Visible(true);
				SUVInfo kUVInfo = pkTopMarkImg->UVInfo();
				kUVInfo.Index = kMyApplicantGuild.byEmblem + 1;
				pkTopMarkImg->UVInfo(kUVInfo);
				pkTopMarkImg->SetInvalidate();
				pkTopGuildName->Text(kMyApplicantGuild.wstrGuildName);
				pkTopGuildLevel->Text(BM::vstring(kMyApplicantGuild.sGuildLevel));
				pkTopGuildMaster->Text(kMyApplicantGuild.wstrMasterName);
				BM::vstring vStrMemberCount(TTW(400467));
				vStrMemberCount.Replace( L"#COUNT#", kMyApplicantGuild.iGuildMemberCount );
				pkTopGuildMember->Text(vStrMemberCount);
				pkTopGuildPR->Text(kMyApplicantGuild.wstrGuildPR);
				pkTopGuildApplication->Visible(true);
				pkTopGuildApplication->SetCustomData(&iCustomData, sizeof(iCustomData));

				switch( m_kApplicationState.byApplicantionState )
				{
				case AS_NONE:
					{// 신청취소
						pkTopGuildApplication->Text(TTW(401080));
					}break;
				case AS_ACCEPT:
				case AS_REJECT:
					{//지원결과확인
						pkTopGuildApplication->Text(TTW(401081));
					}break;				
				default:
					{
					}break;
				}

				break; //지우면 안됨.
			}

			++iter;
		}

		if( !bFindGuild )
		{// 신청한 길드는 있는데 그 길드를 못찾았다? 길드를 해산 한 것임.
			BM::Stream kPacket(PT_C_N_REQ_GUILD_ENTRANCE_CANCEL);
			NETWORK_SEND(kPacket);

			InitApplicantState();
			WrapperUpdateEntranceOpenGuildUI();
			::Notice_Show( TTW(403407), 0 ); // 가입 신청한 길드가 해체 되었습니다.
			return ;
		}
	}
	else
	{// 신청한 길드가 없으면 추천 길드를 보여 줌
		if( m_iRecommendGuildRank )
		{
			int const iCustomData = m_iRecommendGuildRank - 1;
			pkTopGuildRank->Visible(false);
			pkTopRecommend->Visible(true);
			SUVInfo kUVInfo = pkTopMarkImg->UVInfo();
			kUVInfo.Index = m_kEntranceOpenGuild.at(iCustomData).byEmblem + 1;
			pkTopMarkImg->UVInfo(kUVInfo);
			pkTopMarkImg->SetInvalidate();
			pkTopGuildName->Text(m_kEntranceOpenGuild.at(iCustomData).wstrGuildName);
			pkTopGuildLevel->Text(BM::vstring(m_kEntranceOpenGuild.at(iCustomData).sGuildLevel));
			pkTopGuildMaster->Text(m_kEntranceOpenGuild.at(iCustomData).wstrMasterName);
			BM::vstring vStrMemberCount(TTW(400467));
			vStrMemberCount.Replace( L"#COUNT#", m_kEntranceOpenGuild.at(iCustomData).iGuildMemberCount );
			pkTopGuildMember->Text(vStrMemberCount);
			pkTopGuildPR->Text(m_kEntranceOpenGuild.at(iCustomData).wstrGuildPR);
			pkTopGuildApplication->Visible(true);
			pkTopGuildApplication->SetCustomData(&iCustomData, sizeof(iCustomData));
			pkTopGuildApplication->Text(TTW(400466));
		}
	}
	
	pkTopGuildApplication->Disable( IamHaveGuild() );

	XUI::CXUI_List* pkListGuild = dynamic_cast<XUI::CXUI_List*>(pkEntranceBoard->GetControl(L"LST_GUILD_OPEN"));
	if( !pkListGuild )
	{
		return ;
	}

	pkListGuild->ClearList();
	
	// 한페이지에 최대 10개
	int const iStart = iCurrentPage * MAX_PAGE_COLUMN;
	int const iEnd = ( rkContGuild.size() < ( iStart + MAX_PAGE_COLUMN ) ?  rkContGuild.size() : ( iStart + MAX_PAGE_COLUMN ) );

	for( int i = iStart; i < iEnd; ++i )
	{
		XUI::SListItem* pkNewItemElement = pkListGuild->AddItem(L"");
		if( pkNewItemElement )
		{
			CXUI_Wnd* pkNewItemWnd = pkNewItemElement->m_pWnd;

			CXUI_Wnd* pkGuildRank = pkNewItemWnd->GetControl(L"FRM_RANK");
			CXUI_Wnd* pkGuildMark = pkNewItemWnd->GetControl(L"FRM_GUILD_MARK");
			
			CXUI_Image* pkMarkImg = NULL;
			if( pkGuildMark )
			{
				pkMarkImg = dynamic_cast<CXUI_Image*>(pkGuildMark->GetControl(L"IMG_GUILD_MARK"));
			}
			
			CXUI_Wnd* pkGuildName = pkNewItemWnd->GetControl(L"FRM_GUILD_NAME");			
			CXUI_Wnd* pkGuildLevel = pkNewItemWnd->GetControl(L"FRM_GUILD_LEVEL");
			CXUI_Wnd* pkGuildMaster = pkNewItemWnd->GetControl(L"FRM_GUILD_MASTER");
			CXUI_Wnd* pkGuildMember = pkNewItemWnd->GetControl(L"FRM_GUILD_MEMBER");
			CXUI_Wnd* pkGuildPR = pkNewItemWnd->GetControl(L"FRM_GUILD_PR");
			CXUI_Button* pkGuildApplication = dynamic_cast<CXUI_Button*>(pkNewItemWnd->GetControl(L"BTN_APPLICATION"));

			if( !pkGuildRank
			||	!pkMarkImg
			||	!pkGuildName
			||	!pkGuildLevel
			||	!pkGuildMaster
			||	!pkGuildMember
			||	!pkGuildPR
			||	!pkGuildApplication )
			{
				return ;
			}

			pkGuildRank->Text(BM::vstring(rkContGuild.at(i).iRank));
			SUVInfo kUVInfo = pkMarkImg->UVInfo();
			kUVInfo.Index = rkContGuild.at(i).byEmblem + 1;
			pkMarkImg->UVInfo(kUVInfo);
			pkMarkImg->SetInvalidate();
			pkGuildName->Text(rkContGuild.at(i).wstrGuildName);
			pkGuildLevel->Text(BM::vstring(rkContGuild.at(i).sGuildLevel));
			pkGuildMaster->Text(rkContGuild.at(i).wstrMasterName);
			BM::vstring vStrMemberCount(TTW(400467));
			vStrMemberCount.Replace( L"#COUNT#", rkContGuild.at(i).iGuildMemberCount );
			pkGuildMember->Text(vStrMemberCount);
			pkGuildPR->Text(rkContGuild.at(i).wstrGuildPR);

			int iCustomData = rkContGuild.at(i).iRank - 1;
			pkGuildApplication->SetCustomData(&iCustomData, sizeof(iCustomData));

			if( !IamHaveGuild() )
			{
				if( m_kApplicationState.kGuildGuid == rkContGuild.at(i).kGuildGuid )
				{// 내가 신청한 길드
					switch( m_kApplicationState.byApplicantionState )
					{
					case AS_NONE:
						{
							pkGuildApplication->Text(TTW(401080)); // 신청 취소
						}break;
					case AS_ACCEPT:
					case AS_REJECT:
						{
							pkGuildApplication->Text(TTW(401081)); // 지원결과 확인
						}break;
					default:
						{
						}break;
					}
				}
			}
			else
			{// 길드가 있으면 길드신청 누르지 조차 못하게
				pkGuildApplication->Disable(true);
			}
		}
	}
}

bool PgGuildMgr::Member_Find_ByGuid(BM::GUID const& kCharGuid, SGuildMemberInfo& kMemberInfo)
{
	BM::CAutoMutex kLock(m_kMutex);

	if (BM::GUID::NullData() == kCharGuid)
	{
		return false;
	}

	ContGuildMember::iterator kIter = m_kMember.begin();
	while(m_kMember.end() != kIter)
	{
		if((*kIter).second.kCharGuid == kCharGuid)
		{
			kMemberInfo = (*kIter).second;
			return true;
		}
		++kIter;
	}

	return false;
}