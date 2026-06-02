#include "StdAfx.h"
#include "Lohengrin/packetstruct.h"
#include "Lohengrin/packettype.h"
#include "Lohengrin/PgRealmManager.h"
#include "Variant/PgEventView.h"
#include "Variant/PgBattleSquare.h"
#include "PgRenderMan.h"
#include "PgChatMgrClient.h"
#include "PgWorkerThread.h"
#include "PgGuild.h"
#include "PgContentsBase.h"
#include "PgNetwork.h"
#include "lwPacket.h"
#include "PgPilotMan.h"
#include "PgClientParty.h"
#include "PgClientExpedition.h"
#include "PgScripting.h"
#include "PgQuestMan.h"
#include "PgCoupleMgr.h"
#include "PgWorld.h"
#include "PgChatWindow.h"
#include "PgSystemInventory.h"
#include "lwMonsterKillUIMgr.h"
#include "lwNet.h"
#include "lwUI.h"
#include "lwCashShop.h"
#include "PgHelpSystem.h"
#include "PgMToMChat.h"
#include "PgSafeFoamMgr.h"
#include "PgBalloonEmoticonMgr.h"
#include "PgChainAttack.h"
#include "PgCmdlineParse.h"
#include "lwCharacterCard.h"
#include "PgUIDataMgr.h"
#include "lwUIFireLove.h"
#include "PgEventQuestUI.h"
#include "PgBattleSquare.h"
#include "PgActorPet.h"
#include "PgEventScriptSystem.h"
#include "lwInventoryUI.h"
#include "PgPOTParticle.h"
#include "Variant/PgJobSkillLocationItem.h"
#include "PgJobSkillLocationInfo.h"

extern bool SetNowGroundkey(int iLogicalChannelNo, int iMapNo, lwGUID kGroundGuid);
extern int GetNowGroundNo();
extern lwGUID GetNowGroundGuid();
extern void DisplayTime( bool bGameTime, bool bNoTime, bool bNoDate );
extern void lwClearQuake();
extern void CallMapMoveCause( BYTE const byCause );
extern void lwClearAllWorks();

SGroundKey g_kNowGroundKey;
bool g_bDisconnectByUser = false;
bool g_bDisconnectFlush = false;
std::wstring g_wstrDisconnectMessage;

// 루아로 연결되어 있고, 외부에서 부른다.
bool DisconnectFromServer(bool bDisConnectByUser)
{
	g_bDisconnectByUser = bDisConnectByUser;
	NILOG(PGLOG_NETWORK, "[lwNet] DisconnectFromServer\n");
	bool bResultSwitch = false;
	bool bResultLogin = false;
	bResultLogin = g_kNetwork.DisConnectLoginServer();
	bResultSwitch = g_kNetwork.DisConnectSwitchServer();
	
	//접속 종료 정리는 OnDisconnectFromSwitch()에서
	return bResultSwitch || bResultLogin;
}

void DisconnectFromServerByChannel()
{
	//DisconnectFromServer(true);
	g_kNetwork.DisConnectSwitchServer();
}

// PgClientNetwork의 이벤트에서 Flag를 세팅하고 그 Flag를 보고 PgMobileSuit::OnIdle에서 이 함수를 호출한다.
void OnDisconnectFlush()
{
	bool const bDisconnectByUser = g_bDisconnectByUser;

	//
	g_bDisconnectByUser = false;
	g_bDisconnectFlush = false;

	//
	g_kEventScriptSystem.DeactivateAll();

	// 
	g_kMToMMgr.Clear();
	g_kMonKillUIMgr.Clear();
	g_kBalloonEmoticonMgr.ClearInstance();
	g_kCharacterCardInfo = PgCharacterCard();
	// 커뮤니티
	g_kParty.Clear();
	g_kExpedition.Clear();
	g_kStatusEffectMan.ClearReserveEffectAll();

	NILOG(PGLOG_NETWORK, "[lwNet] OnDisconnectFlush\n");
	lwUIOff();
	lwClearAllWorks();
	PgScripting::Reset();

	SAFE_DELETE(PgContentsBase::ms_pkContents);
	SAFE_DELETE(PgActorPet::ms_pkMyPetInventory);
	SAFE_DELETE(PgActorPet::ms_pkTempPetInventory);

	//커뮤니티 Clear;
	g_kChatMgrClient.Clear();	//채팅 로그 삭제	
	g_kGuildMgr.Clear();
	g_kCoupleMgr.Clear();
	if( g_kChatWindow.GethWnd() )
	{
		g_kChatWindow.TerminateWindow();
	}
	g_kQuestMan.Clear();
	PgEventQuestUI::ClearInfo();

	g_kSystemInven.Clear();
//	g_kSafeFoamMgr.Clear();
	g_kSafeFoamMgr.RemoveFollow();
	g_kUIDataMgr.Clear();

	g_kRenderMan.RemoveAllScene();//모든 Scene 정리
	g_kNowGroundKey.Clear();
	g_kPilotMan.ClearPlayerInfo();
	g_kJobSkillLocationInfo.Clear();

	g_kHelpSystem.Clear();
	g_kBattleSquareMng.Clear();
	g_kBattleSquareMng.ClearNotice();
	
	g_kChainAttack.ClearAll();

	g_kCmdLineParse.SetLoginFuncOff(true);

	g_kRenderMan.AddScene("s_login", false);//로그인 신 추가
	
	lwUIFireLove::Clear_FireLoveRemainSMS();	// 사랑의 불꽃 데이터 모두 제거

	lwInventory::ClearCompInv();

	bool const bClearAuthData = g_kNetwork.ClearAuthData();
	lua_tinker::call<void, bool>( "OnDisconnectFlush", bClearAuthData );//Disconnect Flush 총괄 스크립트
	if( bClearAuthData )
	{
		g_kRealmMgr.Clear();
	}
	else
	{
		Sleep(100);// 슬립 살짝만 쓰자.
		lwTryLogin( lwWString(g_kNetwork.LoginID()), lwWString(g_kNetwork.LoginPW()), false, g_kNetwork.bSaveID() );
		return;
	}
	
	// 유저가 직접 끊지 않고, 서버에서도 먼저 끊지 않았으면
	if( g_wstrDisconnectMessage.empty()
	&&	!bDisconnectByUser )
	{
		if( LOCAL_MGR::NC_KOREA == g_kLocal.ServiceRegion() )
		{
			lua_tinker::call<void, char const*, bool >("QuitMsgBox_NC", MB(TTW(g_iMsgNoAnswerServer)), true);
		}
		else
		{
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", g_iMsgNoAnswerServer, true);
			XUI::CXUI_Wnd* pMsgBox = XUIMgr.Get(_T("SFRM_MSG_COMMON"));
			if( pMsgBox )
			{
				pMsgBox->SetCustomData(&g_iMsgNoAnswerServer, sizeof(g_iMsgNoAnswerServer));
			}
		}
	}

	if( !g_wstrDisconnectMessage.empty() )
	{
		if( LOCAL_MGR::NC_KOREA == g_kLocal.ServiceRegion() )
		{
			lua_tinker::call<void, char const*, bool >("QuitMsgBox_NC", MB(g_wstrDisconnectMessage), true);
		}
		else
		{
			lua_tinker::call<void, char const*, bool>("CommonMsgBox", MB(g_wstrDisconnectMessage), true);
		}
		
		g_wstrDisconnectMessage.clear();
	}
}

bool lwNet::RegisterWrapper(lua_State *pkState)
{
	using namespace lua_tinker;

	def(pkState, "DisconnectFromServer", &DisconnectFromServer);
	def(pkState, "Net_RecentMapMove", &Net_RecentMapMove);
	def(pkState, "ViewSelectCharacter", &ViewSelectCharacterDisplay);

	def(pkState, "SetNowGroundkey", &SetNowGroundkey);
	def(pkState, "GetNowGroundNo", &GetNowGroundNo);
	def(pkState, "GetNowGroundGuid", &GetNowGroundGuid);
	def(pkState, "DisplayTime", &DisplayTime);
	def(pkState, "DisconnectFromServerByChannel", &DisconnectFromServerByChannel);
	return true;
}

void NetReqMapMove( SGroundKey const &kGroundKey, short const nSpawnNo )
{
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( pkPlayer )
	{
		if( BM::GUID::IsNotNull(pkPlayer->GetID()) )
		{
			g_kHelpSystem.SaveHelpInfoFile(pkPlayer->GetID());
		}
	}

	g_kChaLevel.SetCharacterLevel();
}

void Net_RecentMapMove()
{
	if ( g_pkWorld )
	{
		if ( g_pkWorld->IsHaveAttr(GATTR_FLAG_VILLAGE) )
		{
			return;
		}

		if ( g_pkWorld->GetAttr() == GATTR_PVP )
		{
			return;
		}

		BM::Stream kPacket( PT_C_M_NFY_RECENT_MAP_MOVE );
		NETWORK_SEND(kPacket)
	}
}

void ViewSelectCharacterDisplay()
{
	if(!g_pkWorld)
	{
		return;
	}
	lwClearQuake();

	g_kNetwork.TryRealmNo(g_kNetwork.NowRealmNo());

	CONT_DEFMAP const *pDefMap;
	g_kTblDataMgr.GetContDef(pDefMap);
	if( NULL == pDefMap )
	{
		return;
	}

	int MapNo = g_pkWorld->MapNo();
	CONT_DEFMAP::const_iterator iter = pDefMap->find(MapNo);
	if( iter == pDefMap->end() )
	{
		return;
	}

	int Attr = iter->second.iAttr;

	if( (Attr & GATTR_EXPEDITION_GROUND) 
		|| (Attr & GATTR_EXPEDITION_LOBBY)
		|| (Attr & GATTR_EVENT_GROUND) )		// 원정대 로비 맵일 경우에는 채널을 원래 채널로 재접속해서 캐릭터 선택 화면으로 넘어가야한다.
	{												// 원래 채널 정보는 Player가 가지고 있다.
		SRecentPlayerNormalMapData MapData;
		PgPlayer * pPlayer = g_kPilotMan.GetPlayerUnit();
		pPlayer->GetRecentNormalMap(MapData);
		g_kNetwork.TryChannelNo(MapData.ChannelNo);
		g_kNetwork.SelectCharGuid(BM::GUID::NullData());		// 선택된 캐릭터를 초기화 시켜주지 않으면 일반 채널이동처럼 동작한다. 꼭 해줘야됨.
	}
	else
	{
		g_kNetwork.TryChannelNo(g_kNetwork.NowChannelNo());	// 아닌경우에는 그냥 하면됨.
	}

	BM::Stream kPacket(PT_C_L_TRY_LOGIN);
	kPacket.Push(g_kNetwork.TryRealmNo());
	kPacket.Push(g_kNetwork.TryChannelNo());
	kPacket.Push(false);
	g_kNetwork.SendToSwitch( kPacket );
}

/*
void Net_C_M_REQ_CREATE_ENTITY(lwGUID kGuid, int iClass, int iLevel, lwPoint3 kPoint3, bool bUniqueClass)
{
	_PgOutputDebugString("Net_C_M_REQ_CREATE_ENTITY kEntityGuid:%s iClass:%d iLevel:%d Pos(%.3f, %.3f, %.3f) UniqueClass:%d", kGuid.GetString(), iClass, iLevel, kPoint3.GetX(), kPoint3.GetY(), kPoint3.GetZ(), bUniqueClass);
	BM::Stream kPacket(PT_C_M_REQ_CREATE_ENTITY);

	SCreateEntity sCreateEntity( kGuid(), SClassKey( iClass, (short)iLevel) );
	sCreateEntity.bUniqueClass	= bUniqueClass;
	sCreateEntity.ptPos.Set( kPoint3.GetX(), kPoint3.GetY(), kPoint3.GetZ() );

	kPacket.Push(sCreateEntity);

	NETWORK_SEND(kPacket)
}
*/

// 리턴값은 맵로딩을 해야 하느냐 말아야 하느냐를 알려준다.
bool SetNowGroundkey(int iLogicalChannelNo, int iMapNo, lwGUID kGroundGuid)
{
	g_kNowGroundKey.Guid(kGroundGuid());
	if ( g_kNowGroundKey.GroundNo() == iMapNo )
	{
		return false;
	}
	g_kNowGroundKey.GroundNo(iMapNo);

	return true;
}

int GetNowGroundNo()
{
	return g_kNowGroundKey.GroundNo();
}

lwGUID GetNowGroundGuid()
{
	return lwGUID( g_kNowGroundKey.Guid() );
}

void DisplayTime( bool bGameTime, bool bNoTime, bool bNoDate )
{
	SYSTEMTIME kNowTime;
	if ( bGameTime )
	{
		g_kEventView.GetGameTime( &kNowTime );
	}
	else
	{
		g_kEventView.GetLocalTime( &kNowTime );
	}

	std::wstring wstrTime;
	if ( !bNoDate )
	{
		MakeStringForDate( wstrTime, kNowTime );
	}

	if ( !bNoTime )
	{
		if ( !bNoDate )
		{
			wstrTime += L" ";
		}
		MakeStringForTime( wstrTime, kNowTime );
	}

	if ( !wstrTime.empty() )
	{
		Notice_Show( wstrTime, EL_Normal );
	}
}

void CallMapMoveCause( BYTE const byCause )
{
	switch( byCause )
	{
	case MMET_GM:
		{
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 19015, true);
		}break;
	case MMET_GM_ReCall:
		{
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 19016, true);
		}break;
	case MMET_Failed:
		{
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 18998, true);
		}break;
	case MMET_TimeOut:
		{
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 18997, true);
		}break;
	case MMET_PublicChannelFailed:
		{
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 18055, true);
		}break;
	case MMET_Failed_Access:
		{
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 99, true);
			RegistUIAction(XUIMgr.Get(L"SFRM_MSG_COMMON"), L"CloseUI");
		}break;
	case MMET_Failed_Full:
		{
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 71011, true);
		}break;
	case MMET_Failed_JoinTime:
		{
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 803, true);
		}break;
	default:
		{
		}break;
	}
}
