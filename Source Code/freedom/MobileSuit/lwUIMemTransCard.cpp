#include "StdAfx.h"
#include "PgNetwork.h"
#include "lwUIMemTransCard.h"
#include "PgPilotMan.h"
#include "PgUIScene.h"
#include "lwWorld.h"

namespace lwUIMemTransCard
{
	bool RegisterWrapper(lua_State *pkState)
	{
		using namespace lua_tinker;
		def(pkState, "SendRegMemPortalReq", &lwSendRegMemPortalReq);
	//	def(pkState, "SendUseMemPortalReq", &lwSendUseMemPortalReq);
		
		return true;
	}

	void lwSendRegMemPortalReq()
	{// 위치 등록 요청 패킷 전송
		g_kMemTransCard.SendRegMemPortalReq();
	}

	bool IsCanSaveAndWarpGround(int const iGAttr)
	{
		return 0 == (iGAttr & (GATTR_FLAG_CANT_WARP));
	}

	//void lwSendUseMemPortalReq()
	//{// 포탈 사용 요청 패킷 전송
	//}
}

PgMemTransCard::PgMemTransCard()
{
	ClearAll();
}

PgMemTransCard::~PgMemTransCard()
{
	ClearAll();
}

void PgMemTransCard::SendRegMemPortalReq()
{// 위치 등록 요청 패킷 전송
	if(!g_pkWorld)
	{
		return;
	}
	// 위치저장이 안되는 장소는
	if( !lwUIMemTransCard::IsCanSaveAndWarpGround(g_pkWorld->GetAttr()) )
	{// 위치를 기억할 수 없음
		::Notice_Show(TTW(790333), EL_Warning);
		return;
	}

	BM::Stream kPacket(PT_C_M_REQ_REG_PORTAL);
	std::wstring kComment(L"");
	kPacket.Push(kComment);
	NETWORK_SEND(kPacket);
	// 플레이어에게 위치 저장됨을 알림	
	::Notice_Show(TTW(790334), EL_Normal);
}

void PgMemTransCard::SendUseMemPortalReq()
{// 포탈 사용 요청 패킷 전송
	//GET_DEF(CItemDefMgr, kItemDefMgr);
	//CItemDef const *pItemDef = kItemDefMgr.GetDef(98000440);	
	//if(ICMET_Cant_UsePVP & pItemDef->GetAbil(AT_ATTRIBUTE)){}
	if(!g_pkWorld)
	{
		return;
	}
	switch( g_pkWorld->GetAttr() )
	{// 다음 맵에서는
	case GATTR_PVP:					// PVP
	case GATTR_EMPORIABATTLE:		// 엠포리아
		{// 기억 전송 카드를 사용할수 없음			
			::Notice_Show(TTW(20027), EL_Warning);
			return;
		}break;
	}

	PgPlayer *pkPlayer = dynamic_cast<PgPlayer*>(g_kPilotMan.GetPlayerUnit());
	if( !pkPlayer )
	{
		return;
	}

	int const iWrongPortalInfo = 790332;
	CONT_USER_PORTAL const& rContUserPortal = pkPlayer->ContPortal();
	if(rContUserPortal.empty())
	{// 저장된 포탈 정보를 얻어와
		::Notice_Show(TTW(iWrongPortalInfo), EL_Warning);
		return;
	}

	CONT_USER_PORTAL::const_iterator iter = rContUserPortal.begin();
	CONT_USER_PORTAL::key_type const& kPortalGuid = (*iter).first;
	CONT_USER_PORTAL::mapped_type const& rkPortalInfo = (*iter).second;

	CONT_DEFMAP const* pkDefMap = NULL;
	g_kTblDataMgr.GetContDef(pkDefMap);
	if( !pkDefMap )
	{
		::Notice_Show(TTW(iWrongPortalInfo), EL_Warning);
		return;
	}

	CONT_DEFMAP::const_iterator find_iter = pkDefMap->find(rkPortalInfo.iGroundNo);
	if( pkDefMap->end() == find_iter )
	{
		::Notice_Show(TTW(iWrongPortalInfo), EL_Warning);
		return;
	}

	if( !lwUIMemTransCard::IsCanSaveAndWarpGround((*find_iter).second.iAttr) )
	{
		::Notice_Show(TTW(iWrongPortalInfo), EL_Warning);
		return;
	}

	BM::Stream kPacket(PT_C_M_REQ_USE_PORTAL);
	kPacket.Push(m_kCashItemPos);				// 포탈 캐쉬템의 인벤위치
	kPacket.Push(lwGetServerElapsedTime32());
	kPacket.Push(kPortalGuid);
	// 포탈 사용 패킷을 보낸다
	NETWORK_SEND(kPacket);	
}

void PgMemTransCard::RecvPortalReqResult(WORD const wPacketType, BM::Stream& rkPacket)
{// 요청 결과 패킷
	switch(wPacketType)
	{
	case PT_M_C_ANS_REG_PORTAL: // 포탈 등록 요청 결과
	case PT_M_C_ANS_USE_PORTAL: // 포탈 사용 요청 결과	
		{
			HRESULT kError;
			rkPacket.Pop(kError);	
			switch(kError)
			{
			case E_PORTAL_FULL:
				{// 더이상 저장불가
					::Notice_Show(TTW(790330), EL_Warning);
				}break;
			case E_PORTAL_ALREADY_EXISTS:
				{// 이미저장된 포탈
					::Notice_Show(TTW(790331), EL_Warning);
				}break;
			case E_NOT_FOUND_PORTAL:
				{// 포탈 정보를 찾을수 없다
					::Notice_Show(TTW(790332), EL_Warning);
				}break;
			case E_CANNOT_SAVE:
				{// 저장할수 없는 맵
					::Notice_Show(TTW(790333), EL_Warning);
				}break;
			default:{}break;
			}
		}break;
	}
	ClearAll();
}

void PgMemTransCard::ClearAll()
{
	m_kCashItemPos.Clear();
}
 
bool PgMemTransCard::GetSavedMapName(std::wstring& kMapName_out)
{// 맵 이름 얻어오기	
	kMapName_out.clear();
	PgPlayer *pkPlayer = dynamic_cast<PgPlayer*>(g_kPilotMan.GetPlayerUnit());
	if( !pkPlayer )
	{
		return false;
	}
	CONT_USER_PORTAL const& rContUserPortal = pkPlayer->ContPortal();
	if(rContUserPortal.empty())
	{
		return false;
	}

	CONT_USER_PORTAL::const_iterator iter = rContUserPortal.begin();
	int const iGroundNo = iter->second.iGroundNo;
	kMapName_out = ::GetMapName(iGroundNo);

	return true;
}
