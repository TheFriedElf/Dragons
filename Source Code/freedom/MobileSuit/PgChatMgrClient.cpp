#include "StdAfx.h"
#include "Variant/PgStringUtil.h"

#include "PgChatMgrClient.h"
#include "PgNetwork.h"
#include "PgCommandMgr.h"
#include "PgScripting.h"

#include "PgPilotMan.h"
#include "PgActor.h"
#include "PgPilot.h"
#include "lwUI.h"

#include "PgClientParty.h"
#include "PgClientExpedition.h"
#include "PgGuild.h"
#include "PgWorld.h"
#include "PgChatWindow.h"

#include "ServerLib.h"
#include "Pg2dString.h"
#include "PgOption.h"
#include "PgChatWindow.H"
#include "Variant/PgEventView.h"
#include "Variant/Global.h"
#include "PgFontSwitchTable.h"
#include "PgMToMChat.h"

#include "PgUIScene.H"
#include "lwUIMemTransCard.h"
#include "lwCashItem.h"

#include "PgUISound.h"
#include "PgMobileSuit.h"
#include "PgEmporiaMgr.h"
#include "PgEventTimer.h"
#include "PgHomeRenew.h"

#include "XUI/XUI_Font.h"

extern char const * EVENT_EM_AD_MERCENARY;
extern char const * EVENT_EM_START_JOIN;

extern void SetEmporiaBattleAlram(bool bClear=false);
extern void OnClickPvP_ReqDuel( lwGUID kCharGuid );

// CT_TRADE 때문에 수정
int const iMaxChatFilterBit = sizeof(__int64) * 8;

void tagChatLog::Clear()
{
	DelayCause(IMEPT_NONE);
	ChatType(CT_NONE);
	fTime = g_pkApp->GetAccumTime();
	//iLineCount = 0;
	bMine = false;
	kCharName.clear();
	kContents.clear();
	kCharGUID.Clear();
	dwColor = 0;
	kExtraDataPackInfo.ClearAllExtraDataPack();
}

//////////////////////////////////////////////////////////////////////////////////////////
// Global Function 
void Announce_Msg(std::wstring const &rkStr, EChatType const eChatType, ENoticeLevel const eNoticeLevel, bool const bNotice, bool const bUpper)
{
	if(!g_kChatMgrClient.Active())
	{
		return;
	}

	SChatLog kChatLog(eChatType);
	g_kChatMgrClient.AddLogMessage(kChatLog, rkStr, bNotice, eNoticeLevel, bUpper);
}

void Notice_Show(std::wstring const &rkString, int const iLevel, bool const bCheckEqualMessage)
{
	if(!g_kChatMgrClient.Active())
	{
		return;
	}

	SChatLog kChatLog(CT_EVENT_SYSTEM);
	g_kChatMgrClient.AddLogMessage(kChatLog, rkString, true, iLevel, true, bCheckEqualMessage);
}

void Notice_Show_ByDefStringNo(int const iDefStringNo, int const iLevel)
{
	std::wstring kString;
	{
		const wchar_t *pText = 0;
		if(!GetDefString(iDefStringNo, pText))
		{
			return;
		}
		std::wstring kString = pText;
	}

	Notice_Show(kString, iLevel);
}

void Notice_Show_ByTextTableNo(int const iTextTableNo, int const iLevel, bool const bCheckEqualMessage)
{
	Notice_Show(TTW(iTextTableNo), iLevel, bCheckEqualMessage);
}

void MakeStringForDate( std::wstring &wstr, SYSTEMTIME const &kTime )
{
	wchar_t wszTemp[MAX_PATH] = {0,};
	::swprintf_s( wszTemp, MAX_PATH, L"%04u-%02u-%02u", kTime.wYear, kTime.wMonth, kTime.wDay );
	wstr += wszTemp;
}

void MakeStringForTime( std::wstring &wstr, SYSTEMTIME const &kTime, bool const bMSEC )
{
	wchar_t wszTemp[MAX_PATH] = {0,};
	if ( bMSEC )
	{
		::swprintf_s( wszTemp, MAX_PATH, L"%02u:%02u:%02u %u\"", kTime.wHour, kTime.wMinute, kTime.wSecond, kTime.wMilliseconds );
	}
	else
	{
		::swprintf_s( wszTemp, MAX_PATH, L"%02u:%02u:%02u", kTime.wHour, kTime.wMinute, kTime.wSecond );
	}
	wstr += wszTemp;
}

void MakeStringForDateTT( std::wstring &wstr, SYSTEMTIME const &kTime, int const iTT )
{
	wchar_t wszTemp[MAX_PATH] = {0,};
	std::wstring ttw = TTW(iTT);
	BM::vstring vstr(ttw.c_str());
	swprintf_s( wszTemp, MAX_PATH, L"%04u", kTime.wYear );
	vstr.Replace(L"#YEAR#", wszTemp);
	swprintf_s( wszTemp, MAX_PATH, L"%02u", kTime.wMonth );
	vstr.Replace(L"#MONTH#", wszTemp);
	swprintf_s( wszTemp, MAX_PATH, L"%02u", kTime.wDay );
	vstr.Replace(L"#DAY#", wszTemp);
	wstr += (std::wstring const&)vstr;
}

void MakeStringForTimeTT( std::wstring &wstr, SYSTEMTIME const &kTime, int const iTT )
{
	wchar_t wszTemp[MAX_PATH] = {0,};
	std::wstring ttw = TTW(iTT);
	BM::vstring vstr(ttw.c_str());
	swprintf_s( wszTemp, MAX_PATH, L"%02u", kTime.wHour );
	vstr.Replace(L"#HOUR#", wszTemp);
	swprintf_s( wszTemp, MAX_PATH, L"%02u", kTime.wMinute );
	vstr.Replace(L"#MIN#", wszTemp);
	swprintf_s( wszTemp, MAX_PATH, L"%02u", kTime.wSecond );
	vstr.Replace(L"#SEC#", wszTemp);
	swprintf_s( wszTemp, MAX_PATH, L"%u", kTime.wMilliseconds );
	vstr.Replace(L"#MS#", wszTemp);
	wstr += (std::wstring const&)vstr;
}

void WstringFormat( std::wstring& rkOutMsg, size_t const iSize, wchar_t const *szMsg, ... )
{
	rkOutMsg = TEXT("");
	if (iSize <= 0)
	{
		return;
	}

	wchar_t* wszTemp = (wchar_t*)NiMalloc(sizeof(wchar_t)*(iSize+1));
	PG_ASSERT_LOG(wszTemp);
	if ( wszTemp )
	{
		::memset( wszTemp, 0, (iSize + 1) * sizeof(wchar_t) );

		va_list vargs;
		va_start( vargs, szMsg );
		::vswprintf_s( wszTemp, iSize, szMsg, vargs );
		va_end(vargs);
		rkOutMsg = wszTemp;
		NiFree(wszTemp);
	}
}

void RecvNoticePacket( BM::Stream& kPacket )
{
	std::wstring wstrNotice;
	int iLevel = EL_Normal;

	ENoticeType kType = NOTICE_ERROR;
	kPacket.Pop(kType);

	switch( kType )
	{
	case NOTICE_EM_CHALLENGE_RESULT:
		{
			bool bChallenge = false;
			kPacket.Pop(bChallenge);

			wstrNotice = bChallenge ? TTW(70091) : TTW(70092);
		}break;
	case NOTICE_EMPORIABATTLE_READY:
		{
			__int64 i64BattleTime = 0i64;
			int iIndex = -1;
			std::wstring kAttackGuildName;
			std::wstring kDefenceGuildName;
			kPacket.Pop(i64BattleTime);
			kPacket.Pop(iIndex);
			kPacket.Pop(kAttackGuildName);
			kPacket.Pop(kDefenceGuildName);

			if( g_kGuildMgr.GuildName()==kAttackGuildName || g_kGuildMgr.GuildName()==kDefenceGuildName )
			{
				SGuildEmporiaInfo kInfo = g_kGuildMgr.GetEmporiaInfo();
				kInfo.i64BattleTime = i64BattleTime;
				g_kGuildMgr.SetEmporiaInfo(kInfo);
			}

			if ( g_pkWorld && !g_pkWorld->IsHaveAttr(GATTR_FLAG_EMPORIABATTLE) )
			{
				//길드전 #GRADE#이 시작되었습니다.(8강, 4강, 준결승, 결승, 1성 방어전, 2성 방어전)
				int iGradeNo = 0;
				switch(iIndex)
				{
				case 101:
				case 100:
					{
						iGradeNo = 71046;
						iGradeNo += iIndex-PgEmporiaTournament::EMPORIA_BATTLE_TOURNAMENT_OWNERSHIP_STARTINDEX;
					}break;
				case 0:
					{
						iGradeNo = 71072;
					}break;
				case 1:case 2:
					{
						iGradeNo = 71073;
					}break;	
				case 3:case 4:case 5:case 6:
					{
						iGradeNo = 71074;
					}break;
				}

				if(iGradeNo)
				{
					BM::vstring vStr(TTW(71081));
					vStr.Replace(L"#GRADE#", TTW(iGradeNo));

					SChatLog kChatLog(CT_EVENT);
					g_kChatMgrClient.AddLogMessage( kChatLog, vStr, true, iLevel, true, true );
				}
			}

			//
			if( g_kEventTimer.IsHaveLocal(EVENT_EM_AD_MERCENARY) )
			{
				g_kEventTimer.RightNowLocal(EVENT_EM_AD_MERCENARY);
			}
			if( g_kEventTimer.IsHaveLocal(EVENT_EM_START_JOIN) )
			{
				g_kEventTimer.RightNowLocal(EVENT_EM_START_JOIN);
			}
			SetEmporiaBattleAlram(true);
		}break;
	case NOTICE_EMPORIABATTLE_START:
		{
		}break;
	case NOTICE_EMPORIABATTLE_RESULT:
		{
			std::wstring wstrResult;

			EEmporiaResultNoticeType eType = ERNT_NONE;
			kPacket.Pop(eType);
			switch(eType)
			{
			case ERNT_USEROUT:
			case ERNT_TIMEOVER:
				{
					//#GUILDNAME# 길드가 길드전에서 승리했습니다.
					std::wstring kWinGuild;
					kPacket.Pop(kWinGuild);

					BM::vstring vStr(TTW(70102));
					vStr.Replace(L"#GUILDNAME#", kWinGuild);
					wstrResult = static_cast<std::wstring>(vStr);
				}break;
			case ERNT_HAVEPOINT:
				{
					//#GUILDNAME# 길드가 포인트 '#POINT#'점을 획득하여 길드전에서 승리했습니다.
					std::wstring kWinGuild;
					int iPoint = 0;
					kPacket.Pop(kWinGuild);
					kPacket.Pop(iPoint);

					BM::vstring vStr(TTW(70103));
					vStr.Replace(L"#GUILDNAME#", kWinGuild);
					vStr.Replace(L"#POINT#", iPoint);
					wstrResult = static_cast<std::wstring>(vStr);
				}break;
			case ERNT_DESTORYCORE:
			case ERNT_KILLDRAGON:
				{
					std::wstring kKillerName;
					std::wstring kWinGuild;
					kPacket.Pop(kKillerName);
					kPacket.Pop(kWinGuild);

					//#GUILDNAME# 길드가 길드전에서 승리했습니다.
					int iNo = 70102;

					if( !kKillerName.empty() )
					{
						//'#USER#'님이 코어를 파괴하여 #GUILDNAME# 길드가 길드전에서 승리했습니다.
						//'#USER#'님이 거대수호용을 섬멸하여 #GUILDNAME# 길드가 길드전에서 승리했습니다.
						iNo = (ERNT_DESTORYCORE==eType) ? 70104 : 70105;
					}

					BM::vstring vStr(TTW(iNo));
					vStr.Replace(L"#USER#", kKillerName);
					vStr.Replace(L"#GUILDNAME#", kWinGuild);
					wstrResult = static_cast<std::wstring>(vStr);
				}break;
			}

			if( !wstrResult.empty() )
			{
				bool bShowNoti = true;
				if( g_pkWorld && g_pkWorld->IsHaveAttr(GATTR_FLAG_EMPORIABATTLE) )
				{
					bShowNoti = false;
				}

				SChatLog kChatLog(CT_EVENT);
				g_kChatMgrClient.AddLogMessage( kChatLog, wstrResult, bShowNoti, EL_PvPMsg );
			}
		}break;
	case NOTICE_PLUSE_UPGRADE_ITEM:
		{
			std::wstring kName;
			wchar_t const* pItemName = NULL;
			DWORD iItemNo = 0;
			int iNextLv = 0;
			bool bResult = false;

			kPacket.Pop(kName);
			kPacket.Pop(iItemNo);
			kPacket.Pop(iNextLv);
			kPacket.Pop(bResult);
			GET_DEF(CItemDefMgr, kItemDefMgr);
			CItemDef const* pDef = kItemDefMgr.GetDef(iItemNo);
			if( pDef )
			{
				if( GetDefString(pDef->NameNo(), pItemName) )
				{
					if(bResult)
					{// 인챈트 메세지는 성공 했을때만 보여준다
						WstringFormat( wstrNotice, MAX_PATH, TTW(404008).c_str(), kName.c_str(), pItemName, iNextLv, TTW(404010).c_str() );
					}
				}
			}
		}break;
	case NOTICE_EM_AD_MERCENARY:
		{
			g_kEmporiaMgr.ADMercenary( kPacket );
		}break;
	case NOTICE_REGEN_EVENT_MONSTER:
		{
			int MapNo = 0;
			kPacket.Pop(MapNo);
			kPacket.Pop(wstrNotice);

			if( 0 != MapNo )
			{
				std::wstring MapName = GetMapName( MapNo );

				wstrNotice = MapName + wstrNotice;
			}

			iLevel = EL_GMMsg;
		}break;
	default:
		{

		}break;
	}

	if ( !wstrNotice.empty() )
	{
		SChatLog kChatLog(CT_EVENT);
		g_kChatMgrClient.AddLogMessage( kChatLog, wstrNotice, true, iLevel );
	}
}


//////////////////////////////////////////////////////////////////////////////////////////
namespace PgChatMgrUtil
{

	std::wstring const kLogWndName(_T("ChatOut"));
	std::wstring const kSysLogWndName(_T("SysChatOut"));
	std::wstring const kExpeditionLogWndName(_T("FRM_EXPEDITION_REGISTER"));
	enum E_CHAT_PALAM
	{
		CP_MAX_All			=	30,//255,//전체 로그는 512개만 보여준다.
		CP_MAX_InputLog		=	32,//최대 입력 로그는 32개다
	};

	typedef struct tagChatColor
	{
		tagChatColor()
		{
			Set(0xFFFFFFFF, 0xFFFFFFFF);
		}

		tagChatColor(DWORD const dwInHead, DWORD const dwInBody)
		{
			Set(dwInHead, dwInBody);
		}
		
		void Set(DWORD const dwInHead, DWORD const dwInBody)
		{
			m_kHead = dwInHead;
			m_kBody = dwInBody;
		}
		DWORD m_kHead, m_kBody;
	}SChatColor;

	typedef std::map< int, CChatLogList > ContAllChatLog;
	typedef std::map< int, SNoticeLevelColor > ContNoticeLevelColor;

	ContAllChatLog kLogChat; //채팅 필터
	ContNoticeLevelColor kContNoticeLevelColor;//공지사항 레벨 컬러


	void HeaderName(const EChatType eType, std::wstring& rkOut, bool bChatOut = false)
	{
		switch(eType)
		{
		case CT_FRIEND:
			{
				rkOut = TTW(400104);
			}break;
		case CT_GUILD:
			{
				rkOut = TTW(400106);
			}break;
		case CT_NORMAL:
			{
				rkOut = TTW(400108);
			}break;
		case CT_TEAM:
			{
				rkOut = TTW(400273);
			}break;
		case CT_PARTY:
			{
				rkOut = TTW(400105);
			}break;
		case CT_WHISPER_BYNAME:
			{
			if( bChatOut )
			{
				rkOut = TTW(400107);//400107
			}
			else
			{
				rkOut = TTW(400904);//400107
			}
			}break;
		case CT_WHISPER_BYGUID:
			{
				rkOut = TTW(400107);//400107
			}break;
		case CT_MEGAPHONE_CH:
		case CT_MEGAPHONE_SV:
			{
				rkOut = TTW(401219);
			}break;
		case CT_TRADE:
			{
				rkOut = TTW(404003);
			}break;
		default:
			{
				rkOut = TTW(400161);
			}break;
		}
	}

	HRESULT GetChatModeColor(const EChatType eChatType, bool const bIsMine, SChatColor &rOutColor)
	{
		const static tagChatColor aColorSet[] = 
		{
			tagChatColor(0xFFFFFFFF, 0xFFFFFFFF),	// CT_ERROR
			tagChatColor(0xFFFFFFFF, 0xFFFFFFFF),	// CT_NONE
			tagChatColor(0xFFFFFFFF, 0xFFFFFFFF),	// CT_NORMAL
			tagChatColor(0xFF46F8FF, 0xFF46F8FF),	// CT_PARTY
			tagChatColor(0xFFFF8249, 0xFFFF8249),	// CT_WHISPER_BYNAME
			tagChatColor(0xFF88FF47, 0xFF88FF47),	// CT_FRIEND
			tagChatColor(0xFFFFFFFF, 0xFFFFFFFF),	// CT_MANTOMAN
			tagChatColor(0XFFFD90FE, 0XFFFD90FE),	// CT_GUILD
			tagChatColor(0xFF46F8FF, 0xFF46F8FF),	// CT_TEAM
			tagChatColor(0xFFFFFFFF, 0xFFFFFFFF),	// CT_Max
			tagChatColor(0xFFFF8249, 0xFFFF8249),	// CT_WHISPER_BYGUID
			tagChatColor(0xFFFFFFFF, 0xFFFFFFFF),	// CT_EVENT
			tagChatColor(0xFFFFFFFF, 0xFFFFFFFF),	// CT_EVENT_SYSTEM
			tagChatColor(0xFFFFFFFF, 0xFFFFFFFF),	// CT_EVENT_GAME
			tagChatColor(0xFFFFF440, 0xFFFFF440),	// CT_NOTICE
			tagChatColor(0xFFFFFFFF, 0xFFFFFFFF),	// CT_BATTLE
			tagChatColor(0xFF00FF00, 0xFF00FF00),	// CT_ITEM
			tagChatColor(0xFF99ACFF, 0xFF99ACFF),	// CT_COMMAND
			tagChatColor(0xFFFFFF00, 0xFFFFFF00),	// CT_MEGAPHONE_CH
			tagChatColor(0xFF7CEFFF, 0xFF7CEFFF),	// CT_MEGAPHONE_SV
			
			tagChatColor(0xFFFFFFFF, 0xFFFFFFFF),	// CT_RAREMONSTERGEN
			tagChatColor(0xFFFFFFFF, 0xFFFFFFFF),	// CT_RAREMONSTERDEAD
			tagChatColor(0xFFFFFFFF, 0xFFFFFFFF),	// CT_OXQUIZEVENT

			tagChatColor(0xFFFF0000, 0xFFFF0000),	// CT_ERROR_NOTICE	빨
			tagChatColor(0xFFFFFFFF, 0xFFFFFFFF),	// CT_NORMAL_NOTICE 흰
			tagChatColor(0xFF00FF00, 0xFF00FF00),	// CT_NOTICE1		녹
			tagChatColor(0xFF46F8FF, 0xFF46F8FF),	// CT_NOTICE2		하
			tagChatColor(0xFFFFF440, 0xFFFFF440),	// CT_NOTICE3		노
			tagChatColor(0xFFFFF440, 0xFFFFF440),	// CT_LOVE_SMS		노

			tagChatColor(0xFF000000, 0xFF000000),	// 쓰지않는 항목 : CT_CASHITEM_GIFT 공백 맞추기 위함 0xFF000000
			tagChatColor(0xFF00FF00, 0xFF00FF00),	// CT_GOLD
			tagChatColor(0xFF00FF00, 0xFF00FF00),	// CT_EXP
			tagChatColor(0xFFFFFFFF, 0xFFFFFFFF),	// CT_EMOTION
			tagChatColor(0xFF000000, 0xFF000000),	// 공백
			tagChatColor(0xFF000000, 0xFF000000),	// 공백
			tagChatColor(0xFF000000, 0xFF000000),	// 공백
			tagChatColor(0xFF000000, 0xFF000000),	// 공백
			tagChatColor(0xFF000000, 0xFF000000),	// 공백
			tagChatColor(0xFF000000, 0xFF000000),	// 공백
			tagChatColor(0xFFFFF440, 0xFFFFF440)	// CT_TRADE
		};
		
		{
			size_t const size = PgArrayUtil::GetArrayCount(aColorSet);			
			if(size < static_cast<size_t>(eChatType))
			{
				rOutColor = tagChatColor(0xFFFFFFFF, 0xFFFFFFFF);
				_PgMessageBox("PgChatMgrClient::GetChatModeColor","color array range over");
				return E_FAIL;
			}
		}

		const static tagChatColor kMyNormal(0xFFFFFCA2, 0xFFFFFCA2);
		const static tagChatColor kMyBattle(0xFFFFFF00, 0xFFFFFF00);
		const static tagChatColor kBlueTeam(0x0000FFFF, 0x0000FFFF);

		if(	eChatType >= CT_ERROR
			&&	eChatType <  CT_MAX)
		{
			const SChatColor *pElement = &aColorSet[eChatType];
			if( bIsMine )
			{	
				switch(eChatType)
				{
				case CT_NORMAL:
					{
						pElement = &kMyNormal;
					}break;
				case CT_BATTLE:
					{
						pElement = &kMyBattle;
					}break;
				default:
					break;
				}
			}

			if( pElement )
			{
				rOutColor = *pElement;
				return S_OK;
			}
		}
		assert(NULL);
		return E_FAIL;
	}


	DWORD ChatInputColor(EChatType const Type)
	{
		const static DWORD aColorSet[] = 
		{
			0xFFFFFFFF,	// CT_ERROR				0xFFFFFFFF
			0xFFFFFFFF,	// CT_NONE				0xFFFFFFFF
			0xFFFFFFFF,	// CT_NORMAL			0xFFFFFFFF
			0xFF46F8FF,	// CT_PARTY				0xFF00FFFF
			0xFFFF8249,	// CT_WHISPER_BYNAME	0xFFFFF440
			0xFF88FF47,	// CT_FRIEND			0xFF00FF00
			0XFFFD90FE,	// CT_GUILD				0xFFFFFF00
			0xFFFFFFFF,	// CT_MANTOMAN			0xFFFFFFFF
			0xFF46F8FF,	// CT_TEAM				0xFF0000FF
			0xFFFFFFFF,	// CT_Max				0xFFFFF440
			0xFFFF8249,	// CT_WHISPER_BYGUID	0xFFFF0000
			0xFFFFFFFF, // CT_EVENT
			0xFFFFFFFF,	// CT_EVENT_SYSTEM		0xFFFFFFFF
			0xFFFFFFFF,	// CT_EVENT_GAME		0xFFFFFFFF
			0xFFFFF440,	// CT_NOTICE			0xFFFFF440
			0xFFFFFFFF,	// CT_BATTLE			0xFFFFFFFF
			0xFF00FF00,	// CT_ITEM				0xFF00FF00
			0xFF99ACFF,	// CT_COMMAND			0xFF99ACFF
			0xFFFFFF00, // CT_MEGAPHONE_CH
			0xFF7CEFFF, // CT_MEGAPHONE_SV
			0xFFFFFFFF, // CT_RAREMONSTERGEN
			0xFFFFFFFF, // CT_RAREMONSTERDEAD
			0xFFFFFFFF, // CT_OXQUIZEVENT

			0xFFFF0000, // CT_ERROR_NOTICE	빨
			0xFFFFFFFF, // CT_NORMAL_NOTICE 흰
			0xFF00FF00, // CT_NOTICE1		녹
			0xFF46F8FF, // CT_NOTICE2		하
			0xFFFFF440, // CT_NOTICE3		노
			0xFFFFF440, // CT_LOVE_SMS		노
			0xFF000000,	// 쓰지않는 항목 : CT_CASHITEM_GIFT 공백 맞추기 위함 0xFF000000
			0xFF00FF00,	// CT_GOLD	0xFF00FF00
			0xFF00FF00,	// CT_EXP	0xFF00FF00
			0xFFFFFFFF,	// CT_EMOTION			0xFFFFFFFF
			0xFF000000,	// 공백
			0xFF000000,	// 공백
			0xFF000000,	// 공백
			0xFF000000,	// 공백
			0xFF000000,	// 공백
			0xFF000000,	// 공백
			0xFFFFF440, // CT_TRADE
		};
		size_t const size = PgArrayUtil::GetArrayCount(aColorSet);
		if(size < static_cast<size_t>(Type))
		{
			_PgMessageBox("PgChatMgrClient::ChatInputColor","color array range over");
			return (0xFFFFFFFF);
		}
		return aColorSet[Type];
	}


	//
	std::wstring const ColorHexToString(DWORD const dwColor)
	{
		int const iMaxArraySize = 128;
		wchar_t	szTemp[iMaxArraySize] = {0, };
		swprintf(szTemp, iMaxArraySize, L"{C=%#x/}", dwColor);
		return std::wstring(szTemp);
	}


	//
	void ChatFilterClear() // 채팅모드로 필터링 초기화
	{
		kLogChat.clear(); // 모든 로그를 지운다.
	}

	void ChatFilterAdd(int const iFilterSet, BYTE const cChatMode, std::wstring const &rkXuiListID) //채팅모드로 필더링 추가
	{
		//채팅 필터Set을 추가
		if(	cChatMode > iMaxChatFilterBit
		||	cChatMode < 0 )
		{
			NILOG(PGLOG_ERROR, "Out of range chat filter mode"); 
			PG_ASSERT_LOG(0 && _T(__FUNCTION__) );
			return;
		}

		DWORD const dwChatFilter = (1<<cChatMode);
		
		std::pair<ContAllChatLog::iterator, bool> kRet = kLogChat.insert(std::make_pair(iFilterSet, ContAllChatLog::mapped_type()));

		ContAllChatLog::mapped_type &rkElement = (*kRet.first).second;
		rkElement.ApplyFlag(dwChatFilter);
		
		if( !rkXuiListID.empty() ) //XUI ID 리스트 임시 저장
		{
			rkElement.WndName(rkXuiListID);
		}
	}

	void ChatFilterDel(int const iFilterSet, BYTE const cChatMode, std::wstring const &rkXuiListID) //채팅모드로 필더링 삭제
	{
		//채팅 필터Set을 추가
		if(	cChatMode > iMaxChatFilterBit
		||	cChatMode < 0 )
		{
			NILOG(PGLOG_ERROR, "Out of range chat filter mode"); 
			PG_ASSERT_LOG(0 && _T(__FUNCTION__) );
			return;
		}

		DWORD const dwChatFilter = (1<<cChatMode);

		std::pair<ContAllChatLog::iterator, bool> kRet = kLogChat.insert(std::make_pair(iFilterSet, ContAllChatLog::mapped_type()));

		ContAllChatLog::mapped_type &rkElement = (*kRet.first).second;
		rkElement.DisApplyFlag(dwChatFilter);
		
		if( !rkXuiListID.empty() ) //XUI ID 리스트 임시 저장
		{
			rkElement.WndName(rkXuiListID);
		}
	}

	//
	//
	void NoticeLevelColor_Add(int const iLevel, DWORD const dwFontColor, DWORD const dwOutlineColor, bool const bShowBG)
	{
		SNoticeLevelColor kNewLevelColor(dwFontColor, dwOutlineColor, bShowBG);
		std::pair<ContNoticeLevelColor::iterator, bool> kRet = kContNoticeLevelColor.insert( std::make_pair(iLevel, kNewLevelColor) );
		if( !kRet.second )
		{
			PG_ASSERT_LOG(0 && "Duplicated Level Color");
		}
	}

	void NoticeLevelColor_Clear()
	{
		kContNoticeLevelColor.clear();
	}


	//
	// 내부 채팅창
	void UpdateLogWndInternal(XUI::CXUI_Wnd* pkLogWnd, std::wstring const& rkWndName, SChatLog const& rkChatLog, std::wstring const& rkText, bool const bCheckEqualMessage, bool const bIsMonster)
	{
		if( !pkLogWnd )
		{
			return;
		}
	
		XUI::CXUI_List *pkListWnd = dynamic_cast<XUI::CXUI_List*>(pkLogWnd->GetControl(rkWndName));
		if( pkListWnd )
		{
			if(bCheckEqualMessage)
			{
				int const iTotalSize = pkListWnd->GetTotalItemCount();
				if(0 < iTotalSize)
				{
					XUI::SListItem *pItem = pkListWnd->GetItemAt(iTotalSize - 1); // List의 가장 최근 아이템을 얻어 온다.
					if(pItem && pItem->m_pWnd)
					{
						if(pItem->m_pWnd->Text() == rkText)
						{
							return;
						}
					}
				}
			}

			XUI::SListItem *pItem = pkListWnd->AddItem( std::wstring() ); //무조건 추가
			if( !pItem )
			{
				NILOG(PGLOG_ERROR, "XUI List Add item failed"); 
				PG_ASSERT_LOG(0);
				return;
			}

			XUI::CXUI_Wnd *pkWnd = pItem->m_pWnd;
			if( pkWnd )
			{
				XUI::CXUI_Wnd*	pChild = pkWnd->GetControl(_T("FRM_ID_RECT"));
				if( pChild )
				{
					if(((rkChatLog.ChatType() > CT_NONE)
						&&  (rkChatLog.ChatType() < CT_USERSELECT_MAX))
						||  (rkChatLog.ChatType() == CT_WHISPER_BYGUID)
						||	(rkChatLog.ChatType() == CT_MEGAPHONE_SV)
						||  (rkChatLog.ChatType() == CT_MEGAPHONE_CH)
						||  (rkChatLog.ChatType() == CT_MYHOME_PUBLIC)
						||  (rkChatLog.ChatType() == CT_MYHOME_PRIVATE)
						||  (rkChatLog.ChatType() == CT_LOVE_SMS)
						||	(rkChatLog.ChatType() == CT_TRADE ))
					{
						std::wstring kHeaderText;
						XUI::CXUI_Style_String HeaderStyle;
						HeaderName(rkChatLog.ChatType(), kHeaderText);
						kHeaderText = _T("[") + kHeaderText + _T("]");
						if( false == bIsMonster )
						{
							pChild->AdjustText(pChild->Font(), kHeaderText, HeaderStyle);
							POINT2 const kHeaderSize(Pg2DString::CalculateOnlySize(HeaderStyle));

							pChild->SetCustomData(rkChatLog.kCharName.c_str(), sizeof(std::wstring::value_type)*rkChatLog.kCharName.size());
							pChild->Text(rkChatLog.kCharName.c_str());
							XUI::CXUI_Style_String kStyleString = pChild->StyleText();
							POINT2 kTextSize(Pg2DString::CalculateOnlySize(kStyleString));

							kTextSize.x += kHeaderSize.x;
							kTextSize.y += kHeaderSize.y;

							pChild->Size(kTextSize);
						}
					}
					else
					{
						POINT2 kTextSize(0, 0);
						pChild->Size(kTextSize);
					}
					pChild->Text(_T(""));
					pChild->SetInvalidate();
				}

				int const Type = rkChatLog.ChatType();
				pkWnd->SetCustomData(&Type, sizeof(Type));
				pkWnd->Text(rkText);
				XUI::CXUI_Style_String kStyleString = pkWnd->StyleText();
				XUI::PgExtraDataPackInfo const &kExtraDataPackInfo = rkChatLog.kExtraDataPackInfo;
				kStyleString.SetExtraDataPackInfo(kExtraDataPackInfo);

				pkWnd->StyleText(kStyleString);

				pkWnd->Size(pkWnd->Size().x, pkWnd->TextRect().y+1);

			}

			while( CP_MAX_All < pkListWnd->GetTotalItemCount() )//갯수 초과 삭제
			{
				XUI::SListItem *pkFirstItem = pkListWnd->FirstItem();//새로운 개체가 아래에서 붙는 방식이기 때문.
				if( pkFirstItem )
				{
					pkListWnd->DeleteItem(pkFirstItem);
				}
				else
				{
					//pkFirstItem 없을 수는 없지만.
					assert(NULL);
					break;
				}
			}
		
			//Adjust List Scroll Position
			pkListWnd->DisplayStartPos(pkListWnd->GetTotalItemLen() - pkListWnd->Height());
			pkListWnd->AdjustMiddleBtnPos();
		}
	}

	// 외부 채팅창
	void UpdateLogWndExternal(int const iIdx, SChatLog const& rkChatLog, std::wstring const& rkText, bool const bCheckEqualMessage)
	{
		XUI::CXUI_Font *pFont = g_kFontMgr.GetFont(FONT_CHAT);
		if( !pFont )
		{
			return;
		}
		XUI::CXUI_Style_String kNewSytleText(XUI::PgFontDef(pFont, rkChatLog.dwColor), rkText);
		g_kChatWindow.AddNewString(iIdx, kNewSytleText, bCheckEqualMessage);
	}

	void UpdateLogWnd(XUI::CXUI_Wnd *pkLogWnd, SChatLog const& rkChatLog, bool const bCheckEqualMessage, bool const IsMonster)
	{
		ContAllChatLog::iterator total_iter = kLogChat.begin();//전체임.
		while( kLogChat.end() != total_iter )
		{
			ContAllChatLog::key_type const& iKey = (*total_iter).first;
			ContAllChatLog::mapped_type const &rkElement = (*total_iter).second;
			if( g_kChatMgrClient.CheckChattingOption(rkChatLog, rkElement) )
			{//추가 하자

				{	// 길드, 파티 대화의 경우 대화 탭이 점멸하도록 해야된다.
					XUI::CXUI_Wnd* pNewLog = XUIMgr.Get(_T("ChatOut"));
					if( !pNewLog )
					{
						return;
					}

					pNewLog = pNewLog->GetControl(_T("CHAT_BG"));
					if( !pNewLog )
					{
						return;
					}

					switch(rkChatLog.ChatType())
					{
					case CT_PARTY:
						{
							pNewLog = pNewLog->GetControl(_T("BTN_CHATFILTER_PARTY"));
							if( !pNewLog )
							{
								return;
							}
						}break;
					case CT_GUILD:
						{
							pNewLog = pNewLog->GetControl(_T("BTN_CHATFILTER_GUILD"));
							if( !pNewLog )
							{
								return;
							}
						}break;
					default:
						{
							pNewLog = NULL;
						}break;
					}

					if( pNewLog )
					{
						pNewLog = pNewLog->GetControl(_T("FRM_NEW_CHATLOG"));
						pNewLog->Visible(true);
					}
				}

				std::wstring kText = ColorHexToString(ChatInputColor(rkChatLog.ChatType()));

				std::wstring kHeader;
				HeaderName(rkChatLog.ChatType(), kHeader);

				kText += _T("[") + kHeader + _T("]");
				kText += ColorHexToString(rkChatLog.dwColor);

				if( rkChatLog.kCharName.size() )
				{
					TCHAR	str[1000];
					swprintf_s(str,1000,_T("{EP=%d/}"),rkChatLog.kExtraDataPackInfo.GetExtraDataPackCount()-1);

					if( false == IsMonster )
					{
						kText += _T("{O=U/}") + std::wstring(str);
						kText += rkChatLog.kCharName;
						kText += _T("{END=0/}");
						kText += _T("{O=0/}");
					}
					else
					{
						kText += std::wstring(str);
						kText += rkChatLog.kCharName;
					}

					kText += _T(" : ");
				}

				kText += rkChatLog.kContents;

				SChatColor kChatColor;
				const HRESULT hResult = GetChatModeColor(rkChatLog.ChatType(), rkChatLog.bMine, kChatColor);
				assert(S_OK == hResult);

				if( pkLogWnd )
				{
					UpdateLogWndInternal(pkLogWnd, rkElement.WndName(), rkChatLog, kText, bCheckEqualMessage, IsMonster);
				}
				else
				{
					UpdateLogWndExternal(iKey-1, rkChatLog, kText, bCheckEqualMessage);
					XUI::CXUI_Wnd *pkWnd = XUIMgr.Activate( PgChatMgrUtil::kLogWndName );
					if( pkWnd )
					{
						UpdateLogWndInternal(pkWnd, rkElement.WndName(), rkChatLog, kText, bCheckEqualMessage, IsMonster);
						XUIMgr.Close( PgChatMgrUtil::kLogWndName );
					}
				}
			}
			++total_iter;
		}
	}

	void UpdateExpeditionLogWnd(XUI::CXUI_Wnd *pkLogWnd, SChatLog const& rkChatLog, bool const bCheckEqualMessage, bool const IsMonster)
	{
		if( g_kExpedition.IsMember(rkChatLog.kCharGUID) )
		{
			ContAllChatLog::iterator total_iter = kLogChat.begin();//전체임.
			while( kLogChat.end() != total_iter )
			{
				ContAllChatLog::key_type const& iKey = (*total_iter).first;
				ContAllChatLog::mapped_type const &rkElement = (*total_iter).second;
				if( rkElement.CmpFilterFlag(rkChatLog.ChatFlag()) )
				{//추가 하자
					std::wstring kText = ColorHexToString(ChatInputColor(rkChatLog.ChatType()));

					SExpeditionMember const* pMember = PgClientExpeditionUtil::GetMember(rkChatLog.kCharGUID);
					if( pMember == NULL )
					{
						return;
					}

					if( g_kExpedition.IsExpeditionMaster(pMember->kCharGuid) )
					{
						kText += ColorHexToString(0xFFF76300);
					}
					else
					{
						int TeamNum = pMember->sTeam;
						switch( TeamNum )
						{
						case EET_TEAM_1:
							{
								kText += ColorHexToString(0xFFA0F70B);
							}break;
						case EET_TEAM_2:
							{
								kText += ColorHexToString(0xFFFEF967);
							}break;
						case EET_TEAM_3:
							{
								kText += ColorHexToString(0xFF90D8FF);
							}break;
						case EET_TEAM_4:
							{
								kText += ColorHexToString(0xFFBBA4FC);
							}break;
						default:
							{
								kText += ColorHexToString(rkChatLog.dwColor);
							}break;
						}
					}

					if( rkChatLog.kCharName.size() )
					{
						TCHAR	str[1000];
						swprintf_s(str,1000,_T("{EP=%d/}"),rkChatLog.kExtraDataPackInfo.GetExtraDataPackCount()-1);

						if( false == IsMonster )
						{
							kText += _T("{O=U/}") + std::wstring(str);
							kText += rkChatLog.kCharName;
							kText += _T("{END=0/}");
							kText += _T("{O=0/}");
						}
						else
						{
							kText += std::wstring(str);
							kText += rkChatLog.kCharName;
						}

						kText += _T(" : ");
					}

					kText += rkChatLog.kContents;

					SChatColor kChatColor;
					const HRESULT hResult = GetChatModeColor(rkChatLog.ChatType(), rkChatLog.bMine, kChatColor);
					assert(S_OK == hResult);

					if( pkLogWnd )
					{
						UpdateLogWndInternal(pkLogWnd, rkElement.WndName(), rkChatLog, kText, bCheckEqualMessage, IsMonster);
					}
					else
					{
						UpdateLogWndExternal(iKey-1, rkChatLog, kText, bCheckEqualMessage);
						XUI::CXUI_Wnd *pkWnd = XUIMgr.Activate( PgChatMgrUtil::kLogWndName );
						if( pkWnd )
						{
							UpdateLogWndInternal(pkWnd, rkElement.WndName(), rkChatLog, kText, bCheckEqualMessage, IsMonster);
							XUIMgr.Close( PgChatMgrUtil::kLogWndName );
						}
					}
				}
				++total_iter;
			}
		}
	}

	void ClearLogWnd()
	{
		XUI::CXUI_Wnd *pkWnd = XUIMgr.Get( kLogWndName );
		if( !pkWnd )
		{
			return;
		}

		ContAllChatLog::iterator total_iter = kLogChat.begin();//UI기록 삭제
		while( kLogChat.end() != total_iter )
		{
			ContAllChatLog::mapped_type const& rkElement = (*total_iter).second;
			XUI::CXUI_List *pkListWnd = dynamic_cast<XUI::CXUI_List*>(pkWnd->GetControl(rkElement.WndName()));
			if( pkListWnd )
			{
				pkListWnd->ClearList();
			}
			++total_iter;
		}
	}
}

//
//////////////////////////////////////////////////////////////////////////////////////////
//ChatInitUtil::SCHAT_LOC	const kChatLoc[ECS_END] = { { 0, 669, 384, 120 }, { 53, 695, 609, 120 }, { 12, 715, 685, 83}, {0, 732, 384, 120}, };
ChatInitUtil::SCHAT_LOC	kChatLoc[ECS_END] = { { 0, 669, 384, 120 }, { 53, 695, 609, 120 }, { 12, 715, 685, 83}, {0, 716, 384, 120}, };

void ChatInitUtil::ResizeChatWindow(int const iX, int const iY, int const iWidth, int const iHeight, bool bChangeLoc)
{
	XUI::CXUI_Wnd*	pChatBar = XUIMgr.Activate(L"ChatBar");
	XUI::CXUI_Wnd*	pChatOut = XUIMgr.Activate(L"ChatOut");
	XUI::CXUI_Wnd*	pSysChatOut = XUIMgr.Activate(L"SysChatOut");
	
	if( !pChatBar || !pChatOut || !pSysChatOut )
	{
		return;
	}
	
	if( (ECS_CASHSHOP == g_kChatMgrClient.ChatStation()) || (ECS_PVPLOBBY == g_kChatMgrClient.ChatStation()) )
	{
		pSysChatOut->Visible(false);
		g_kChatMgrClient.SysOutHide(true);
	}
	else
	{
		int SysOutConfigValue = lwConfig_GetValue("SYSTEMOUT_OPTION", "SYSTEMOUT_WINDOW");
		if( 2 == SysOutConfigValue  )
		{	// 시스템 메세지 창옵션이 켜진 경우에만.
			pSysChatOut->Visible(true);
			g_kChatMgrClient.SysOutHide(false);
		}
	}

	//	챗 바
	POINT2 ptSize = pChatBar->Size();
	ptSize.x = iWidth;
	pChatBar->Size(ptSize);
	if(bChangeLoc)
	{
		pChatBar->Location(iX, iY, pChatBar->Location().z);
	}
	XUI::CXUI_Wnd* pkChatBarBg = pChatBar->GetControl(L"SFRM_BAR_BG");
	if(pkChatBarBg)
	{
		pkChatBarBg->Size(pChatBar->Size());
	}

	XUI::CXUI_Wnd* pHead = pChatBar->GetControl(L"FRM_HEAD");
	XUI::CXUI_Wnd* pEdit = pChatBar->GetControl(L"EDT_CHAT");
	XUI::CXUI_Button* pDrop = dynamic_cast<XUI::CXUI_Button*>(pChatBar->GetControl(L"BTN_TELL_TYPE"));
	if( !pHead || !pEdit || !pDrop)
	{ 
		return; 
	}

	pHead->Size(iWidth - 158, pHead->Size().y);
	pEdit->Size(iWidth - 158, pEdit->Size().y);

	XUI::CXUI_Wnd* pInputBg = pChatBar->GetControl(L"SFRM_EDT_BG");
	if( pInputBg )
	{
		pInputBg->Size(iWidth - 156, pInputBg->Size().y);
		pInputBg = pInputBg->GetControl(L"SFRM_EDT_BG_SHADOW");
		if( pInputBg )
		{
			pInputBg->Size(iWidth - 156, pInputBg->Size().y);
		}
	}

	XUI::CXUI_Button* pBtnToggle = dynamic_cast<XUI::CXUI_Button*>(pChatBar->GetControl(L"BTN_TOGGLECHAT"));
	if( pBtnToggle )
	{
		pBtnToggle->Location(iWidth - 30, pBtnToggle->Location().y, pBtnToggle->Location().z);
	}
	XUI::CXUI_Form* pFormToggle = dynamic_cast<XUI::CXUI_Form*>(pChatBar->GetControl(L"FRM_TOGGLECHAT"));
	if( pFormToggle )
	{
		pFormToggle->Location(iWidth - 29, pBtnToggle->Location().y + 1, pBtnToggle->Location().z);
	}

	//	챗 아웃
	ptSize = pChatOut->Size();
	ptSize.x = iWidth;
	if(ECS_CASHSHOP == g_kChatMgrClient.ChatStation())
	{// 캐시샵시 채팅창 크기 줄어들때, 채팅 배경부분도 조정함
		if(0 < ptSize.y)
		{
			ptSize.y = iHeight;
		}
	}
	pChatOut->Size(ptSize);
	if(bChangeLoc)
	{
		pChatOut->Location(iX-1, iY - ptSize.y, pChatOut->Location().z);	
	}

	bool bIsSnap = g_kChatMgrClient.SysChatOutSnap();
	if( pSysChatOut && bIsSnap )
	{
		pSysChatOut->Location(POINT2(pChatOut->Location().x, pChatOut->Location().y - pSysChatOut->Size().y - 5));
	}

	XUI::CXUI_Wnd* pBtnParent = pChatOut->GetControl(L"CHAT_BG");
	if( pBtnParent )
	{
		XUI::CXUI_Button* pBtnOption = dynamic_cast<XUI::CXUI_Button*>(pBtnParent->GetControl(L"BTN_FILTER_OPTION"));
		XUI::CXUI_Button* pBtnTColor = dynamic_cast<XUI::CXUI_Button*>(pBtnParent->GetControl(L"BTN_TEXT_COLOR"));
		XUI::CXUI_Button* pBtnEmoticon = dynamic_cast<XUI::CXUI_Button*>(pBtnParent->GetControl(L"BTN_EMOTICON"));
		XUI::CXUI_Button* pBtnEmotion = dynamic_cast<XUI::CXUI_Button*>(pBtnParent->GetControl(L"BTN_EMOTION"));
		
		XUI::CXUI_Wnd* pMainBg = pBtnParent->GetControl(L"SFRM_CHATOUT_BG");
		if( !pBtnTColor|| !pBtnEmoticon || !pBtnEmotion || !pMainBg || !pBtnOption)
		{ 
			return; 
		}
		
		if(ECS_CASHSHOP == g_kChatMgrClient.ChatStation())
		{
			pBtnParent->Size(iWidth, 63);
			pMainBg->Size(iWidth, 63);
		}
		else
		{
			pBtnParent->Size(iWidth, pBtnParent->Size().y);
			pMainBg->Size(iWidth, pMainBg->Size().y);
		}
		pBtnOption->Location(iWidth - 118, pBtnOption->Location().y, pBtnOption->Location().z);
		pBtnTColor->Location(iWidth - 95, pBtnTColor->Location().y, pBtnTColor->Location().z);
		pBtnEmoticon->Location(iWidth - 71, pBtnEmoticon->Location().y, pBtnEmoticon->Location().z);
		pBtnEmotion->Location(iWidth - 48, pBtnEmotion->Location().y, pBtnEmotion->Location().z);
		XUI::CXUI_Wnd* pBtn = pBtnParent->GetControl(L"BTN_CALL_CHAT_DLG");
		if(NULL!=pBtn)
		{
			pBtn->Location(iWidth - 25, pBtn->Location().y, pBtn->Location().z);		
		}
	}

	XUI::CXUI_Wnd* pListNormal = pChatOut->GetControl(L"LIST_CHAT_NORMAL");
	XUI::CXUI_Wnd* pListGuild = pChatOut->GetControl(L"LIST_CHAT_GUILD");
	XUI::CXUI_Wnd* pListParty = pChatOut->GetControl(L"LIST_CHAT_PARTY");
	XUI::CXUI_Wnd* pListTrade = pChatOut->GetControl(L"LIST_CHAT_TRADE");
	XUI::CXUI_Wnd* pListWhisp = pChatOut->GetControl(L"LIST_CHAT_WHISPER");
	XUI::CXUI_Wnd* pDMY = pChatOut->GetControl(L"DMY_CHAT");
	if( !pListNormal || !pListGuild || !pListParty || !pListTrade || !pListWhisp || !pDMY )
	{ 
		return; 
	}
	if(ECS_CASHSHOP == g_kChatMgrClient.ChatStation())
	{// 캐시샵시 채팅창 크기 줄어들때, 스크롤바를 줄임
		int const iChatSize = 52;
		if(pListNormal)
		{
			pListNormal->Size(iWidth, iChatSize);	
			XUI::CXUI_Wnd* pScrUpBtn = pListNormal->GetControl(L"ScrUp");
			if(pScrUpBtn)
			{
				POINT3I const kCurrentPos = pScrUpBtn->Location();			
				pScrUpBtn->Location(iWidth - 21, -5);
			}
			XUI::CXUI_Wnd* pScrDnBtn = pListNormal->GetControl(L"ScrDown");
			if(pScrDnBtn)
			{
				POINT3I const kCurrentPos = pScrDnBtn->Location();			
				pScrDnBtn->Location(iWidth - 21, 40);
			}			
			XUI::CXUI_Wnd* pScrMid = pListNormal->GetControl(L"ScrMiddle");
			if(pScrMid)
			{
				POINT3I const kCurrentPos = pScrMid->Location();			
				pScrMid->Location(iWidth - 19, -5);
			}

			XUI::CXUI_List* pList = dynamic_cast<XUI::CXUI_List*>(pListNormal);
			if(pList)
			{
				lwUIWnd(pList).MoveMiddleBtnEndPos();
				pList->AdjustDownBtnPos();
				pList->AdjustMiddleBarSize();
				pList->AdjustMiddleBtnPos();
			}
		}

		if(pListGuild)
		{
			pListGuild->Size(iWidth, iChatSize);	
			XUI::CXUI_Wnd* pScrUpBtn = pListGuild->GetControl(L"ScrUp");
			if(pScrUpBtn)
			{
				POINT3I const kCurrentPos = pScrUpBtn->Location();			
				pScrUpBtn->Location(iWidth - 21, -5);
			}
			XUI::CXUI_Wnd* pScrDnBtn = pListGuild->GetControl(L"ScrDown");
			if(pScrDnBtn)
			{
				POINT3I const kCurrentPos = pScrDnBtn->Location();			
				pScrDnBtn->Location(iWidth - 21, 40);
			}			
			XUI::CXUI_Wnd* pScrMid = pListGuild->GetControl(L"ScrMiddle");
			if(pScrMid)
			{
				POINT3I const kCurrentPos = pScrMid->Location();			
				pScrMid->Location(iWidth - 19, -5);
			}

			XUI::CXUI_List* pList = dynamic_cast<XUI::CXUI_List*>(pListGuild);
			if(pList)
			{
				lwUIWnd(pList).MoveMiddleBtnEndPos();
				pList->AdjustDownBtnPos();
				pList->AdjustMiddleBarSize();
				pList->AdjustMiddleBtnPos();
			}
		}

		if(pListParty)
		{
			pListParty->Size(iWidth, iChatSize);	
			XUI::CXUI_Wnd* pScrUpBtn = pListParty->GetControl(L"ScrUp");
			if(pScrUpBtn)
			{
				POINT3I const kCurrentPos = pScrUpBtn->Location();			
				pScrUpBtn->Location(iWidth - 21, -5);
			}
			XUI::CXUI_Wnd* pScrDnBtn = pListParty->GetControl(L"ScrDown");
			if(pScrDnBtn)
			{
				POINT3I const kCurrentPos = pScrDnBtn->Location();			
				pScrDnBtn->Location(iWidth - 21, 40);
			}			
			XUI::CXUI_Wnd* pScrMid = pListParty->GetControl(L"ScrMiddle");
			if(pScrMid)
			{
				POINT3I const kCurrentPos = pScrMid->Location();			
				pScrMid->Location(iWidth - 19, -5);
			}

			XUI::CXUI_List* pList = dynamic_cast<XUI::CXUI_List*>(pListParty);
			if(pList)
			{
				lwUIWnd(pList).MoveMiddleBtnEndPos();
				pList->AdjustDownBtnPos();
				pList->AdjustMiddleBarSize();
				pList->AdjustMiddleBtnPos();
			}
		}

		if(pListTrade)
		{
			pListTrade->Size(iWidth, iChatSize);	
			XUI::CXUI_Wnd* pScrUpBtn = pListTrade->GetControl(L"ScrUp");
			if(pScrUpBtn)
			{
				POINT3I const kCurrentPos = pScrUpBtn->Location();			
				pScrUpBtn->Location(iWidth - 21, -5);
			}
			XUI::CXUI_Wnd* pScrDnBtn = pListTrade->GetControl(L"ScrDown");
			if(pScrDnBtn)
			{
				POINT3I const kCurrentPos = pScrDnBtn->Location();			
				pScrDnBtn->Location(iWidth - 21, 40);
			}			
			XUI::CXUI_Wnd* pScrMid = pListTrade->GetControl(L"ScrMiddle");
			if(pScrMid)
			{
				POINT3I const kCurrentPos = pScrMid->Location();			
				pScrMid->Location(iWidth - 19, -5);
			}

			XUI::CXUI_List* pList = dynamic_cast<XUI::CXUI_List*>(pListTrade);
			if(pList)
			{
				lwUIWnd(pList).MoveMiddleBtnEndPos();
				pList->AdjustDownBtnPos();
				pList->AdjustMiddleBarSize();
				pList->AdjustMiddleBtnPos();
			}
		}

		if(pListWhisp)
		{
			pListWhisp->Size(iWidth, iChatSize);	
			XUI::CXUI_Wnd* pScrUpBtn = pListWhisp->GetControl(L"ScrUp");
			if(pScrUpBtn)
			{
				POINT3I const kCurrentPos = pScrUpBtn->Location();			
				pScrUpBtn->Location(iWidth - 21, -5);
			}
			XUI::CXUI_Wnd* pScrDnBtn = pListWhisp->GetControl(L"ScrDown");
			if(pScrDnBtn)
			{
				POINT3I const kCurrentPos = pScrDnBtn->Location();			
				pScrDnBtn->Location(iWidth - 21, 40);
			}			
			XUI::CXUI_Wnd* pScrMid = pListWhisp->GetControl(L"ScrMiddle");
			if(pScrMid)
			{
				POINT3I const kCurrentPos = pScrMid->Location();			
				pScrMid->Location(iWidth - 19, -5);
			}

			XUI::CXUI_List* pList = dynamic_cast<XUI::CXUI_List*>(pListWhisp);
			if(pList)
			{
				lwUIWnd(pList).MoveMiddleBtnEndPos();
				pList->AdjustDownBtnPos();
				pList->AdjustMiddleBarSize();
				pList->AdjustMiddleBtnPos();
			}
		}
		
		pListGuild->Size(iWidth, iChatSize);
		pListParty->Size(iWidth, iChatSize);
		pListTrade->Size(iWidth, iChatSize);
		pListWhisp->Size(iWidth, iChatSize);
		pDMY->Size(iWidth,iChatSize);
	}
	else
	{
		if(pListNormal)
		{
			pListNormal->Size(iWidth, pListNormal->Size().y);
			XUI::CXUI_Wnd* pScrUpBtn = pListNormal->GetControl(L"ScrUp");
			if(pScrUpBtn)
			{
				POINT3I const kCurrentPos = pScrUpBtn->Location();			
				pScrUpBtn->Location(iWidth - 21, -5);
			}
			XUI::CXUI_Wnd* pScrDnBtn = pListNormal->GetControl(L"ScrDown");
			if(pScrDnBtn)
			{
				POINT3I const kCurrentPos = pScrDnBtn->Location();			
				pScrDnBtn->Location(iWidth - 21, 40);
			}			
			XUI::CXUI_Wnd* pScrMid = pListNormal->GetControl(L"ScrMiddle");
			if(pScrMid)
			{
				POINT3I const kCurrentPos = pScrMid->Location();			
				pScrMid->Location(iWidth - 19, -5);
			}

			XUI::CXUI_List* pList = dynamic_cast<XUI::CXUI_List*>(pListNormal);
			if(pList)
			{
				lwUIWnd(pList).MoveMiddleBtnEndPos();
				pList->AdjustDownBtnPos();
				pList->AdjustMiddleBarSize();
				pList->AdjustMiddleBtnPos();
			}
		}

		if(pListGuild)
		{
			pListGuild->Size(iWidth, pListGuild->Size().y);
			XUI::CXUI_Wnd* pScrUpBtn = pListGuild->GetControl(L"ScrUp");
			if(pScrUpBtn)
			{
				POINT3I const kCurrentPos = pScrUpBtn->Location();			
				pScrUpBtn->Location(iWidth - 21, -5);
			}
			XUI::CXUI_Wnd* pScrDnBtn = pListGuild->GetControl(L"ScrDown");
			if(pScrDnBtn)
			{
				POINT3I const kCurrentPos = pScrDnBtn->Location();			
				pScrDnBtn->Location(iWidth - 21, 40);
			}			
			XUI::CXUI_Wnd* pScrMid = pListGuild->GetControl(L"ScrMiddle");
			if(pScrMid)
			{
				POINT3I const kCurrentPos = pScrMid->Location();			
				pScrMid->Location(iWidth - 19, -5);
			}

			XUI::CXUI_List* pList = dynamic_cast<XUI::CXUI_List*>(pListGuild);
			if(pList)
			{
				lwUIWnd(pList).MoveMiddleBtnEndPos();
				pList->AdjustDownBtnPos();
				pList->AdjustMiddleBarSize();
				pList->AdjustMiddleBtnPos();
			}
		}

		if(pListParty)
		{
			pListParty->Size(iWidth, pListParty->Size().y);
			XUI::CXUI_Wnd* pScrUpBtn = pListParty->GetControl(L"ScrUp");
			if(pScrUpBtn)
			{
				POINT3I const kCurrentPos = pScrUpBtn->Location();			
				pScrUpBtn->Location(iWidth - 21, -5);
			}
			XUI::CXUI_Wnd* pScrDnBtn = pListParty->GetControl(L"ScrDown");
			if(pScrDnBtn)
			{
				POINT3I const kCurrentPos = pScrDnBtn->Location();			
				pScrDnBtn->Location(iWidth - 21, 40);
			}			
			XUI::CXUI_Wnd* pScrMid = pListParty->GetControl(L"ScrMiddle");
			if(pScrMid)
			{
				POINT3I const kCurrentPos = pScrMid->Location();			
				pScrMid->Location(iWidth - 19, -5);
			}

			XUI::CXUI_List* pList = dynamic_cast<XUI::CXUI_List*>(pListParty);
			if(pList)
			{
				lwUIWnd(pList).MoveMiddleBtnEndPos();
				pList->AdjustDownBtnPos();
				pList->AdjustMiddleBarSize();
				pList->AdjustMiddleBtnPos();
			}
		}

		if(pListTrade)
		{
			pListTrade->Size(iWidth, pListTrade->Size().y);
			XUI::CXUI_Wnd* pScrUpBtn = pListTrade->GetControl(L"ScrUp");
			if(pScrUpBtn)
			{
				POINT3I const kCurrentPos = pScrUpBtn->Location();			
				pScrUpBtn->Location(iWidth - 21, -5);
			}
			XUI::CXUI_Wnd* pScrDnBtn = pListTrade->GetControl(L"ScrDown");
			if(pScrDnBtn)
			{
				POINT3I const kCurrentPos = pScrDnBtn->Location();			
				pScrDnBtn->Location(iWidth - 21, 40);
			}			
			XUI::CXUI_Wnd* pScrMid = pListTrade->GetControl(L"ScrMiddle");
			if(pScrMid)
			{
				POINT3I const kCurrentPos = pScrMid->Location();			
				pScrMid->Location(iWidth - 19, -5);
			}

			XUI::CXUI_List* pList = dynamic_cast<XUI::CXUI_List*>(pListTrade);
			if(pList)
			{
				lwUIWnd(pList).MoveMiddleBtnEndPos();
				pList->AdjustDownBtnPos();
				pList->AdjustMiddleBarSize();
				pList->AdjustMiddleBtnPos();
			}
		}

		if(pListWhisp)
		{
			pListWhisp->Size(iWidth, pListWhisp->Size().y);
			XUI::CXUI_Wnd* pScrUpBtn = pListWhisp->GetControl(L"ScrUp");
			if(pScrUpBtn)
			{
				POINT3I const kCurrentPos = pScrUpBtn->Location();			
				pScrUpBtn->Location(iWidth - 21, -5);
			}
			XUI::CXUI_Wnd* pScrDnBtn = pListWhisp->GetControl(L"ScrDown");
			if(pScrDnBtn)
			{
				POINT3I const kCurrentPos = pScrDnBtn->Location();			
				pScrDnBtn->Location(iWidth - 21, 40);
			}			
			XUI::CXUI_Wnd* pScrMid = pListWhisp->GetControl(L"ScrMiddle");
			if(pScrMid)
			{
				POINT3I const kCurrentPos = pScrMid->Location();			
				pScrMid->Location(iWidth - 19, -5);
			}

			XUI::CXUI_List* pList = dynamic_cast<XUI::CXUI_List*>(pListWhisp);
			if(pList)
			{
				lwUIWnd(pList).MoveMiddleBtnEndPos();
				pList->AdjustDownBtnPos();
				pList->AdjustMiddleBarSize();
				pList->AdjustMiddleBtnPos();
			}
		}

		pListGuild->Size(iWidth, pListGuild->Size().y);
		pListParty->Size(iWidth, pListParty->Size().y);
		pListTrade->Size(iWidth, pListTrade->Size().y);
		pListWhisp->Size(iWidth, pListWhisp->Size().y);
		pDMY->Size(iWidth, pDMY->Size().y);
	}
}

//
//////////////////////////////////////////////////////////////////////////////////////////
PgChatMgrClient::tagInputChatLog::tagInputChatLog(std::wstring const &rkChat)
{
	fInputTime = g_pkApp->GetAccumTime();
	kChat = rkChat;
	fTermTime = 0.0f;
}

PgChatMgrClient::PgChatMgrClient()
	:	m_bShowNoticeAble(true)
	,	m_bIsUseColor(false)
	,	m_iFontColor(0xFFFFFFFF)
	,	m_kBallonState(BS_Not_Open)
	,	m_kChatStation(ECS_COMMON)
	,	m_bToggleConsecutiveChat(false)
	,	m_bExistWhisperList(false)
	,	m_bSysChatOutSnap(false)
	,	m_bSysOutHide(false)
{
	m_iChatMode		= CT_NORMAL;
	InputChatLog_Reset();
	Active(true);

	{// 도배 금지 관련 변수 초기화(아래 값은 lua에서 재초기화 됨) init.lua
		m_stMaxInputLog = PgChatMgrUtil::CP_MAX_InputLog;
		BlockTime(0.0f);
		MaxBlockTime(5.0f);
		StartBlockTime(0.0f);
		
		// 스팸 체크 스테이트
		m_iSpamChkFlag = 0;
						//ESCS_ChainInput
						// |ESCS_LooseChainInput
						// |ESCS_MacroInput
						// |ESCS_SameInput;
	
		//SetBlockSpamChat_ChainInput
		ChainWatchT(0.77f);
		ChainBlockTime(0.0f);

		//SetBlockSpamChat_LooseChainInput
		LChainWatchT(0.21f);
		LChainCnt(2);
		LChainBlockTime(0.0f);

		//SetBlockSpamChat_MacroInput
		SameWatchT(1.17f);
		SameCnt(2);
		SameBlockTime(0.0f);

		//SetBlockSpamChat_SameInput
		MacroCnt(10);
		MacroRangeTime(0.2);
		MacroBlockTime(0.0f);
	}
	//이전 채팅창번호 저장
	{
		OldChatMode(1);
	}
	
	////아래 부분은 init.lua에서 초기화 한후 다시 넣고 있으니 init.lua에서 추가하여 사용할것
	//PgChatMgrUtil::NoticeLevelColor_Clear();
	//PgChatMgrUtil::NoticeLevelColor_Add(EL_Normal, 0xFFFFF954, 0xFF000000, false);		//Level 0
	//PgChatMgrUtil::NoticeLevelColor_Add(EL_Warning, 0xFFFF2A2F, 0xFF000000, true);		//Level 1
	//PgChatMgrUtil::NoticeLevelColor_Add(EL_Level2, 0xFF01FFAE, 0xFF000000, false);		//Level 2
	//PgChatMgrUtil::NoticeLevelColor_Add(EL_PvPMsg, 0xFFF6D576, 0xFF000000, true);		//Level PvP
	//PgChatMgrUtil::NoticeLevelColor_Add(EL_GMMsg, 0xFFFFA934, 0xFF000000, true);		//GMMsg
	//PgChatMgrUtil::NoticeLevelColor_Add(EL_Notice1, 0xFF00FF00, 0xFF000000, true);		// 중요정보 갱신
	//PgChatMgrUtil::NoticeLevelColor_Add(EL_Notice2, 0xFF46F8FF, 0xFF000000, true);		// 유저 관계된 공지
	//PgChatMgrUtil::NoticeLevelColor_Add(EL_Notice3, 0xFFFFFF00, 0xFF000000, true);		// 정보갱신, 유저에게 알림	
	PgChatMgrUtil::ChatFilterClear();
	//채팅모드 인식기능
	{
		OldChatTime(0);
	}

	m_kSavedPos.Set(-1,-1);
	m_kSystemSavedPos.Set(-1, -1);

	// 거래 채팅 대기 시간.
	TradeModeChatTime(0.0f);
}

PgChatMgrClient::~PgChatMgrClient()
{
	Active(false);
}

void PgChatMgrClient::Clear()
{
	BM::CAutoMutex kLock(m_kMutex);
	//좀 특수한 Clear();
	//로그아웃. 서버 접속 연결이 끊겼을 때 이전 사용자 정보만 싹 지워준다.

	PgChatMgrUtil::ClearLogWnd();

	m_kLog_Input.clear();
	m_kLog_InputChat.clear();
	m_kLog_InputChatForBlock.clear();
	InputChatLog_Reset();
	ClearBlockTime();

	m_kLog_Name2Guid.clear();
	m_kLog_Guid2Name.clear();
	m_kCache_Name2Guid.clear();
	m_kTextColorList.clear();
}

void PgChatMgrClient::ShowNoticeUI(std::wstring const kContents, int iLevel, bool bUpper, bool const bCheckEqualMessage)
{
	BM::CAutoMutex kLock(m_kMutex);
	if( PgChatMgrUtil::kContNoticeLevelColor.empty() )
	{
		return;
	}

	PgChatMgrUtil::ContNoticeLevelColor::iterator iter = PgChatMgrUtil::kContNoticeLevelColor.find(iLevel);
	if( PgChatMgrUtil::kContNoticeLevelColor.end() == iter )
	{
		iter = PgChatMgrUtil::kContNoticeLevelColor.find(0);//Default
	}

	PgChatMgrUtil::ContNoticeLevelColor::mapped_type& rkNoticeColor = (*iter).second;

	m_kNoticeWaitList.push( SNoticeWaitItem(kContents, rkNoticeColor) );
	ShowNotice(bUpper, bCheckEqualMessage);
}

void PgChatMgrClient::ShowNotice(bool bUpper, bool const bCheckEqualMessage)
{ 
	BM::CAutoMutex kLock(m_kMutex);
	if ( !IsShowNoticeAble() )
	{
		return;
	}

	DWORD const dwCurTime = BM::GetTime32();
	while( !m_kNoticeWaitList.empty() )
	{
		QueueNoticeWait::value_type& rkItem = m_kNoticeWaitList.front();
		if(bUpper)
		{
			ShowNotice(0, rkItem.kContents, rkItem, dwCurTime, 0, bCheckEqualMessage);
		}
		else
		{// 하단 출력
			ShowNotice(100, rkItem.kContents, rkItem, dwCurTime, 0, bCheckEqualMessage);
		}
		m_kNoticeWaitList.pop();
	}
}

void PgChatMgrClient::LogMsgBox(int const iTTW)
{
	BM::CAutoMutex kLock(m_kMutex);

	LogMsgBox(TTW(iTTW));
}

void PgChatMgrClient::LogMsgBox(std::wstring const &rkMsg)
{
	BM::CAutoMutex kLock(m_kMutex);

	if( rkMsg.empty() )
	{
		return;
	}

	SChatLog kChatLog(CT_EVENT);
	AddLogMessage(kChatLog, rkMsg);
	lua_tinker::call<void, char const*, bool>("CommonMsgBox", MB(rkMsg), true);
}

void PgChatMgrClient::ClearNotice()
{
	BM::CAutoMutex kLock(m_kMutex);
	while( !m_kNoticeWaitList.empty() )
	{
		m_kNoticeWaitList.pop();
	}

	int iIndex = 0;
	while(true)
	{
		TCHAR wszTemp[MAX_PATH] = {0,};
		_stprintf_s(wszTemp, MAX_PATH, _T("WarnOut_%d"), iIndex);
		XUI::CXUI_Wnd*	pkWnd = XUIMgr.Get(wszTemp);
		if( pkWnd )
		{
			pkWnd->Close();
			++iIndex;
		}
		else
		{
			break;
		}
	}
}

void PgChatMgrClient::DisableNotice()
{
	BM::CAutoMutex kLock(m_kMutex);
	TCHAR wszTemp[MAX_PATH] = {0,};

	int iIndex = 0;
	XUI::CXUI_Wnd *pkWnd = NULL;
	DWORD const dwTime = BM::GetTime32();
	while ( 1 )
	{
		_stprintf_s(wszTemp, MAX_PATH, _T("WarnOut_%d"), iIndex++ );

		if ( !XUIMgr.IsActivate( std::wstring(wszTemp), pkWnd ) )
		{
			break;
		}

		if(dwTime - pkWnd->CalledTime() < 1000)
		{
			SNoticeLevelColor kCurColor;
			pkWnd->GetCustomData(&kCurColor, sizeof(kCurColor));

			m_kNoticeWaitList.push( SNoticeWaitItem( pkWnd->Text(), kCurColor) );
		}
		else
		{
			pkWnd->Close();
		}
	}
}

void PgChatMgrClient::ShowNotice(int const iIndex, std::wstring const &rkContents, const SNoticeLevelColor& rkColor, DWORD const dwCallTime, int const iY, bool const bCheckEqualMessage)
{
	TCHAR wszTemp[MAX_PATH] = {0,};
	_stprintf_s(wszTemp, MAX_PATH, _T("WarnOut_%d"), iIndex);

	int iEleapsedY = iY;
	SNoticeWaitItem kOldItem;
	DWORD dwOldCalledTime = 0;

	XUI::CXUI_Wnd *pkWnd = XUIMgr.Get(wszTemp);
	if( pkWnd )//Old
	{
		SNoticeLevelColor kCurColor;
		pkWnd->GetCustomData(&kCurColor, sizeof(kCurColor));
		//ShowNotice(iIndex+1, pkWnd->Text(), kCurColor, pkWnd->CalledTime(), iY+pkWnd->Size().y);// 재귀호출

		kOldItem = SNoticeWaitItem(pkWnd->Text(), kCurColor);
		dwOldCalledTime = pkWnd->CalledTime();
	}
	else
	{
		pkWnd = XUIMgr.Call(wszTemp);
	}
	
	if(bCheckEqualMessage)
	{
		if(kOldItem.kContents == rkContents)
		{
			// 이전 메시지와 내용이 같을 경우 추가하지 않음
			return;
		}
	}

	if( pkWnd )
	{
		int const iUpSideBaseLocationY = 214;
		int const iDownSideBaseLocationY = 565;
		pkWnd->Text(rkContents);//contents
		pkWnd->CalledTime(dwCallTime);

		pkWnd->Size(pkWnd->Size().x, Pg2DString::CalculateOnlySize((XUI::CXUI_Style_String)pkWnd->StyleText()).y + 2);
		if(100 > iIndex)
		{// 상단 표시 
			pkWnd->Location(pkWnd->Location().x, iUpSideBaseLocationY - iY - pkWnd->Size().y);
		}
		else
		{// 하단 표시
			int const iDownSideLimit = iDownSideBaseLocationY * pkWnd->Size().y*3;
			pkWnd->Location(pkWnd->Location().x, iDownSideBaseLocationY + iY + pkWnd->Size().y);
			if(iDownSideLimit < pkWnd->Location().y) 
			{
				pkWnd->Close();
				return;
			}
		}

		pkWnd->FontColor(rkColor.dwFontColor);//decorate
		pkWnd->OutLineColor(rkColor.dwOutlineColor);
		XUI::CXUI_Wnd* pkBgWnd = pkWnd->GetControl(_T("IMG_BG"));
		if( pkBgWnd )
		{
			pkBgWnd->Visible(rkColor.bShowBG);
			pkBgWnd->Size(pkBgWnd->Size().x, pkWnd->Size().y);
			pkBgWnd->ImgSize(pkBgWnd->Size());
		}
		pkWnd->SetCustomData(&rkColor, sizeof(rkColor));
		
		iEleapsedY += pkWnd->Size().y;		
	}

	if( pkWnd
	&&	!kOldItem.kContents.empty() )//이전 아이템이 있으면 재귀 호출로
	{
		ShowNotice(iIndex+1, kOldItem.kContents, kOldItem, dwOldCalledTime, iEleapsedY);
	}
}

void ReplaceEnterChar(std::wstring::value_type &rkChar)
{
	if( L'\r' == rkChar )
	{
		rkChar = L' ';
	}
}

void PgChatMgrClient::AddMessage(int const iTextTable, SChatLog &rkChatLog, bool bNotice, int iLevel, bool bUpper, bool const bCheckEqualMessage)
{//지정된 메시지 번호로 이벤트 메시지 발생
	BM::CAutoMutex kLock(m_kMutex);
	PgStringUtil::EraseStr<std::wstring>(TTW(iTextTable), _T("\n"), rkChatLog.kContents);
	std::for_each(rkChatLog.kContents.begin(), rkChatLog.kContents.end(), ReplaceEnterChar);

	rkChatLog.dwColor = PgChatMgrUtil::ChatInputColor(rkChatLog.ChatType());
	//rkChatLog.kContents = TTW(iTextTable);
	//rkChatLog.iLineCount = BreakLine(rkChatLog.kContents);;

	if( rkChatLog.IsDelayLog() )
	{
		AddDelayLog(rkChatLog);
		return;
	}

	RecvChatLog_Add(rkChatLog, bCheckEqualMessage);

	if(bNotice)
	{
		ShowNoticeUI(rkChatLog.kContents, iLevel, bUpper, bCheckEqualMessage);
	}
};

void PgChatMgrClient::AddLogMessage(SChatLog &rkChatLog, std::wstring const &rkLogMessage, bool bNotice, int iLevel, bool bUpper, bool const bCheckEqualMessage, bool const IsMonster)
{
	BM::CAutoMutex kLock(m_kMutex);

	PgStringUtil::EraseStr<std::wstring>(rkLogMessage, _T("\n"), rkChatLog.kContents);
	std::for_each(rkChatLog.kContents.begin(), rkChatLog.kContents.end(), ReplaceEnterChar);

	rkChatLog.dwColor = PgChatMgrUtil::ChatInputColor(rkChatLog.ChatType());
	//rkChatLog.kContents = rkLogMessage;
	//rkChatLog.iLineCount = BreakLine(rkChatLog.kContents);

	if( rkChatLog.IsDelayLog() )
	{
		AddDelayLog(rkChatLog);
		return;
	}

	RecvChatLog_Add(rkChatLog, bCheckEqualMessage, IsMonster);

	if(bNotice)
	{
		ShowNoticeUI(rkLogMessage, iLevel, bUpper, bCheckEqualMessage);
	}
}

//채팅 커맨드 XML로 부터 로딩
bool PgChatMgrClient::ParseXml(const TiXmlNode *pkNode, void *pArg, bool bUTF8)
{
	BM::CAutoMutex kLock(m_kMutex);

	int const iType = pkNode->Type();

	while(pkNode)
	{
		switch(iType)
		{
		case TiXmlNode::ELEMENT:
			{
				TiXmlElement *pkElement = (TiXmlElement *)pkNode;
				PG_ASSERT_LOG(pkElement);
				
				char const *pcTagName = pkElement->Value();

				if(strcmp(pcTagName, "CHAT_COMMAND")==0)
				{
					const TiXmlNode * pkChildNode = pkNode->FirstChild();
					
					pkNode = pkChildNode;
					continue;
				}
				else if(strcmp(pcTagName, "TEXT") == 0)
				{
					const TiXmlAttribute *pkAttr = pkElement->FirstAttribute();
					int	iModeNo	= 0;
					unsigned int iMinCount = 0;
					char const *szText = 0;
					while(pkAttr)
					{
						char const *pcAttrName = pkAttr->Name();
						char const *pcAttrValue = pkAttr->Value();

						if(0 == strcmp(pcAttrName, "TEXT"))
						{
							szText = pcAttrValue;
						}
						else if(0 == strcmp(pcAttrName, "MODE"))
						{
							iModeNo = atoi(pcAttrValue);
						}
						else if(0 == strcmp(pcAttrName, "MINCOUNT"))
						{
							iMinCount = (unsigned int)atoi(pcAttrValue);
						}
						else
						{
							PG_ASSERT_LOG(!"invalid attribute");
						}
						pkAttr = pkAttr->Next();
					}

					if( szText )
					{
						std::wstring kTemp = UNI(szText);
						UPR(kTemp);
						RegChatCommand(kTemp, (EChatType)iModeNo, iMinCount, std::string(""));
					}
				}
			}
		}

		const TiXmlNode* pkNextNode = pkNode->NextSibling();
		pkNode = pkNextNode;
	}

	return true;
}

bool PgChatMgrClient::RegChatCommand(std::wstring const &rkCommand, const EChatType eChatType, const unsigned int iMinCount, const std::string& rkLuaScript)
{
	BM::CAutoMutex kLock(m_kMutex);

	SChatCommand kChatCommand;
	kChatCommand.kChatCommand = rkCommand;
	kChatCommand.eChatType = eChatType;
	kChatCommand.iMinCount = iMinCount;
	kChatCommand.kLuaScript = rkLuaScript;

	m_kChatCommand.insert(std::make_pair(rkCommand, kChatCommand));
	return true;
}


//
void BreakSpace(std::wstring const &rkText, std::vector< std::wstring > &rkVec)
{
	std::wstring kTemp = rkText;
	std::wstring kSep = L" ";
	TCHAR *pToken = NULL;
	TCHAR *pNextToken = NULL;
	pToken = _tcstok_s(&kTemp[1], kSep.c_str(), &pNextToken);
	if(NULL == pToken)
	{
		rkVec.push_back(rkText);
		return;
	}
	rkVec.push_back(pToken);

	while(1)
	{
		pToken = _tcstok_s(NULL, kSep.c_str(), &pNextToken);
		if( pToken != NULL )
		{
			rkVec.push_back( pToken );
		}
		else
		{
			break;
		}
	}
}

//채팅 커맨드 및 이모션
bool PgChatMgrClient::Command_Chat(std::wstring const &rkChat, int &iChatMode, std::wstring &rkName, std::wstring &rkContents)
{
	BM::CAutoMutex kLock(m_kMutex);

	if(rkChat[0] == _T('/'))
	{
		if(rkChat.size() < 1)
		{//채팅내용이 한글자면 에러!
			return false;
		}

		std::vector< std::wstring > kVec;
		BreakSpace(rkChat, kVec);//공백 문자로 채팅 문을 분해

		SChatCommand const* kTargetCmd = GetCommandType(kVec[0]);
		if( !kTargetCmd )
		{
			return false;
		}

		iChatMode = kTargetCmd->eChatType;

		switch( iChatMode )
		{
		case CT_WHISPER_BYNAME://사용자 입력은 이걸로 들어온다.
			{

				if(kTargetCmd->iMinCount != 3)// /r
				{
					if( !m_kWhisperList.size() )
					{
						return false;
					}

					ContWhisper::reverse_iterator	rit = m_kWhisperList.rbegin();
					if( rit != m_kWhisperList.rend() )
					{
						rkName = (*rit);
					}

					if( kVec.size() > 1 )
					{
						size_t nPos = rkChat.find(kVec[1], 0);
						rkContents = rkChat.substr(nPos, rkChat.size() - nPos);
					}
				}
				else// /w
				{
					if( kVec.size() < 2 )
					{
						return false;
					}
					rkName = kVec[1];
					InsertWhisper(rkName, false);

					if( kVec.size() > 2 )
					{
						size_t nPos = rkChat.find(kVec[2], 0);
						rkContents = rkChat.substr(nPos, rkChat.size() - nPos);
					}
				}
			}break;
		default: // /p /f /g /s
			{
				rkName = L"";
				if( kVec.size() > 1 )
				{
					size_t nPos = rkChat.find(kVec[1], 0);
					rkContents = rkChat.substr(nPos, rkChat.size() - nPos);
				}
			}break;
		}
		return true;
	}

	return false;
}

bool PgChatMgrClient::Command_Emotion(std::wstring const &rkChat)
{
	BM::CAutoMutex kLock(m_kMutex);

	//특정 단어에 대한 캐릭터 반응 처리용(ㅋㅋ -> 웃음)
	int iID = 0;
	if( g_kPgEmotiMgr.FindEmotionByKeyward(rkChat, iID))
	{
		PgPlayer* pPlayer = g_kPilotMan.GetPlayerUnit();
		if( pPlayer )
		{
			::CONT_EMOTION const * pContEmotion = NULL;
			g_kTblDataMgr.GetContDef(pContEmotion);	//맵 데프
			if( !pContEmotion )
			{
				return false;
			}

			::CONT_EMOTION::const_iterator c_iter = pContEmotion->find(SEMOTIONKEY(ET_EMOTION, iID));
			if( c_iter == pContEmotion->end() )
			{
				return false;
			}

			PgBitArray<MAX_DB_EMOTICON_SIZE> kCont;
			if( pPlayer->GetContEmotion(ET_EMOTION, kCont) )
			{
				if( c_iter->second == 0 || kCont.Get(iID) )
				{
					if(CheckCanEmotion(iID))
					{
						SendEmotionPacket(g_kPilotMan.PlayerActor(), ET_EMOTION, iID);
					}
					return true;
				}
			}
		}
	}

	return false;
}

bool PgChatMgrClient::Command_Filter(std::wstring &rkChat)
{
	BM::CAutoMutex kLock(m_kMutex);

	std::wstring kTemp = rkChat;
	if(!g_kClientFS.Filter(kTemp, true))
	{
		return false;
	}
	rkChat = kTemp;
	return true;
}

bool PgChatMgrClient::SeperateUserCommand(std::wstring const& rkCmdText, std::wstring::size_type kNextPos, std::wstring &rkNormalText, std::map<size_t, std::wstring> &kContCommand)
{
	rkNormalText.clear();
	const	int	iLength = rkCmdText.length();
	if(iLength == 0)
	{
		return	false;
	}
	const int iMaxColorCommand = 9;
	const WCHAR	*strColorCommandConverting[iMaxColorCommand]=
	{
		{ _T("R") },
		{ _T("G") },
		{ _T("B") },
		{ _T("Y") },
		{ _T("O") },
		{ _T("P") },
		{ _T("GR") },
		{ _T("V") },
		{ _T("W") }
	};

	const	WCHAR wFirstChar = rkCmdText[0];
	//	wFirstChar 가 문자이면 Color 커멘드
	if(iswalpha(wFirstChar) != 0 )
	{
		for(int iCommandPos=0; iCommandPos<iMaxColorCommand;iCommandPos++)
		{
			std::wstring kCommandText = _T("#");
			const WCHAR* strCommand = strColorCommandConverting[iCommandPos];
			int	iCmdLen = wcslen(strCommand);
			bool bCorrect = true;
			int kSrcPos = 0;
			for(kSrcPos=0; kSrcPos<iCmdLen; kSrcPos++)
			{
				WCHAR	wSrcChar = rkCmdText[kSrcPos];
				WCHAR	wCmdChar = *(strCommand+kSrcPos);

				if(wSrcChar != wCmdChar)
				{
					bCorrect = false;
					break;
				}
				else
				{
					kCommandText += wSrcChar;
				}
			}

			if(bCorrect)
			{
				rkNormalText = rkCmdText.substr(kSrcPos);
				kContCommand.insert( std::make_pair(kNextPos, kCommandText) );
				return	true;
			}
		}
		return	false;
	}

	//	wFirstChar 가 숫자이면 이모티콘 커멘드
	if(iswdigit(wFirstChar) != 0 )
	{
		std::wstring kNumberString = _T("#");
		int kSrcPos = 0;
		for(kSrcPos=0; kSrcPos<iLength; kSrcPos++)
		{
			WCHAR	wChar = rkCmdText[kSrcPos];
			if( iswdigit(wChar) == 0)
			{
				break;
			}

			kNumberString += wChar;
		}

		if(kNumberString.length() == 0)
		{
			return	false;
		}

		rkNormalText = rkCmdText.substr(kSrcPos);
		kContCommand.insert( std::make_pair(kNextPos, kNumberString) );

		return	true;
	}

	return	false;
}
bool PgChatMgrClient::SeperateCommandString(std::wstring const& rkOrgText, std::wstring &rkNormalText, std::map<size_t, std::wstring> &kContCommand)
{
	const	int	iLength = rkOrgText.length();
	if(iLength == 0)
	{
		return	false;
	}
	rkNormalText.clear();
	//#있는지 확인
	wchar_t const kCommandWord = L'#';
	std::wstring::size_type kBeforePos = 0;
	std::wstring::size_type kNextPos = rkOrgText.find(kCommandWord);
	if( kNextPos == std::wstring::npos )
	{
		return false;
	}
	//있으면 분리 동작
	std::wstring kNewString;
	while(kNextPos != std::wstring::npos)
	{
		kNewString += rkOrgText.substr(kBeforePos,kNextPos-kBeforePos);
		//	# 다음부터 Space or '#' 까지 읽는다.
		
		std::wstring::size_type	kNextSpacePos = rkOrgText.find(_T(' '),kNextPos+1);
		std::wstring::size_type	kNextCommandPos = rkOrgText.find(kCommandWord,kNextPos+1);

		if( kNextSpacePos == std::wstring::npos) 
		{//공백 위치 저장
			kNextSpacePos = iLength;
		}
		if( kNextCommandPos == std::wstring::npos) 
		{//다음 #의 위치 저장
			kNextCommandPos = iLength;
		}
		//커맨드의 길이 값 = 공백이랑 다음# 중 빠른 쪽 위치값 - 현재 #위치값+1
		const int iCmdTextLength = ((kNextCommandPos<kNextSpacePos) ? kNextCommandPos : kNextSpacePos) - (kNextPos+1);
		std::wstring kCmdText = rkOrgText.substr(kNextPos+1,iCmdTextLength);

		std::wstring kConvertedText;
		//추출한 커맨드에 딸려들어간 일반 문자 분리 및 분리한 커맨드 저장
		if(SeperateUserCommand(kCmdText, kNextPos, kConvertedText, kContCommand))
		{
			kNewString += kConvertedText;
			kNextPos += kCmdText.length();
		}
		else
		{
			kNewString += kCommandWord;
		}

		kBeforePos = kNextPos+1;
		kNextPos = rkOrgText.find(kCommandWord,kBeforePos);
	
	}

	rkNormalText = kNewString;

	return true;
}

//이모티콘을 섞은 금지어 검사 및 변환(2010. 07. 14 조현건)
bool PgChatMgrClient::TransEmoticonFilter(std::wstring &rkChat)
{
	std::wstring kNormal;
	std::map<size_t, std::wstring> kContCommand;

	if( false == SeperateCommandString(rkChat, kNormal, kContCommand) )
	{
		return false;
	}
	//새로 적용된 필터링 적용
	bool bFilter = false;
	if( false == g_kClientFS.Filter(rkChat, false) )
	{ //기존 필터에 걸리지 않았을 경우, 레직스 필터결과 저장
		bFilter = g_kClientFS.Filter(kNormal, true);
	}
	
	if(true == bFilter)
	{//레직스 필터에 걸렸을 경우, 임시 이모티콘 컨테이너에저장된 이모티콘 원래 문장 위치에 끼워넣기(되나?문제없나?)
		std::wstring kFilterString;
		size_t kNormalPos = 0;
		for( size_t kCurrentPos = 0; kCurrentPos < rkChat.length(); ++kCurrentPos)
		{
			std::map<size_t, std::wstring>::const_iterator const_iter = kContCommand.find(kCurrentPos);
			if( const_iter != kContCommand.end() )
			{
				std::wstring kCmdText = (*const_iter).second;
				kFilterString += kCmdText;
				kCurrentPos += kCmdText.length() - 1;
			}
			else 
			{
				if(kNormalPos < kNormal.length() )
				{
					wchar_t const kNormChar = kNormal.at(kNormalPos);
					kFilterString += kNormChar;
					++kNormalPos;
				}
			}
		}
		rkChat = kFilterString;
	}

	return bFilter;
}

//채팅 기능 제한
void PgChatMgrClient::SetBlockSpamChat_ChainInput(float const fWatchT, float const fBlockTime)
{
	ChainWatchT(fWatchT);
	ChainBlockTime(fBlockTime);
}

void PgChatMgrClient::SetBlockSpamChat_LooseChainInput(float const fWatchT, int const iCnt, float const fBlockTime)
{
	LChainWatchT(fWatchT);
	LChainCnt(iCnt);
	LChainBlockTime(fBlockTime);
}

void PgChatMgrClient::SetBlockSpamChat_MacroInput(int const iCnt, float const fRangeTime, float const fBlockTime)
{
	MacroCnt(iCnt);
	MacroRangeTime(fRangeTime);
	MacroBlockTime(fBlockTime);
}

void PgChatMgrClient::SetBlockSpamChat_SameInput(float const fWatchT, int iCnt, float const fBlockTime)
{
	SameWatchT(fWatchT);
	SameCnt(iCnt);
	SameBlockTime(fBlockTime);
}

bool PgChatMgrClient::Command_UserFunction(std::wstring const& rkChat)
{
	BM::CAutoMutex kLock(m_kMutex);

	if ( true == Command_EmotiAction( rkChat ) )
	{
		return true;
	}

	if( rkChat[0] == L'/' )
	{// 다른곳에서 Command_UserUse 함수 호출 금지
		if( !Command_UserUse(rkChat) )
		{
			std::wstring const kCommand = rkChat.substr(1);
			return Command_God(kCommand);
		}
	}

	return false;
}

bool PgChatMgrClient::Command_EmotiAction(std::wstring const& rkChat)
{//여기서 커맨드 형식의 풍선 이모티콘과 이모션을 패킷화하여 서버로 전송한다.
	BM::CAutoMutex kLock(m_kMutex);

	bool bExistEmoti = false;
	BYTE bEmotiType = ET_EMOTICON;
	int iEmotiID = 0;
	if( rkChat[0] == L'/' )
	{//이모션 체크
		//g_kPgEmotiMgr
		bEmotiType = ET_EMOTION;
		bExistEmoti = g_kPgEmotiMgr.FindEmotionByKeyward( rkChat, iEmotiID, 1 );
	}
	else if( rkChat[0] == L'@' )
	{//풍선이모티콘 체크
		//g_kPgEmotiMgr
		bEmotiType = ET_BALLOON;
		bExistEmoti = g_kPgEmotiMgr.FindBalloonByKeyward( rkChat, iEmotiID, 1 );
	}

	if( bExistEmoti )
	{
		if(ET_EMOTION == bEmotiType)
		{
			if(CheckCanEmotion(iEmotiID))
			{
				SendEmotionPacket(g_kPilotMan.PlayerActor(), bEmotiType, iEmotiID);
			}
		}
		else
		{
			SendEmotionPacket(g_kPilotMan.PlayerActor(), bEmotiType, iEmotiID);
		}
	}
	return bExistEmoti;
}

bool PgChatMgrClient::Command_God(std::wstring const &rkChat)
{
#ifndef USE_INB
	lua_tinker::dostring(*lua_wrapper_user(g_kLuaTinker), MB(rkChat));
#else
	SChatLog kChatLog(CT_EVENT);//잘못된 명령어 입니다.
	AddMessage(EM_Invalid_Command, kChatLog, true);
#endif
	return true;
}

bool PgChatMgrClient::Command_UserUse(std::wstring const &rkChat)
{// 유저가 사용할수 있는 명령어	
	static std::wstring const SAVE_USER_PORTAL_CMD(L"/MEMORIZE");

	std::wstring kTempStr=rkChat;
	UPR(kTempStr);
	
	switch(g_kLocal.ServiceRegion())
	{// 국가 중에
	case LOCAL_MGR::NC_TAIWAN:
		{// 대만은
			if(SAVE_USER_PORTAL_CMD == kTempStr)
			{// 위치 기억 명령을 사용 할수 있다
				g_kMemTransCard.SendRegMemPortalReq();
				return true;
			}
		}break;
	default:
		{
#ifndef USE_INB
			if(SAVE_USER_PORTAL_CMD == kTempStr)
			{// 배포 버전이 아니면, 위치 기억 명령을 사용 가능
				g_kMemTransCard.SendRegMemPortalReq();
				return true;
			}
#endif
		}break;
	}
	return false;
}

bool	PgChatMgrClient::GetUserCommandConvertedString(std::wstring const &kStringSrc,std::wstring &kConvertedString, std::wstring const & kFont)	
{
	kConvertedString.clear();

	const	int	iLength = kStringSrc.length();

	if(iLength == 0)
	{
		return	false;
	}

	const	int	iMaxColorCommand = 9;
	
	const	WCHAR	*strColorCommandConverting[iMaxColorCommand][2]=
	{
		{ _T("R"),_T("{C=0xffff0000/}") },
		{ _T("G"),_T("{C=0xff00ff00/}") },
		{ _T("B"),_T("{C=0xff00000ff/}") },
		{ _T("Y"),_T("{C=0xffffff00/}") },
		{ _T("O"),_T("{C=0xffff8000/}") },
		{ _T("P"),_T("{C=0xffff0080/}") },
		{ _T("GR"),_T("{C=0xff808080/}") },
		{ _T("V"),_T("{C=0xff8000ff/}") },
		{ _T("W"),_T("{C=0xffffffff/}") }
	};

	const	WCHAR wFirstChar = kStringSrc[0];

	//	wFirstChar 가 문자이면 Color 커멘드
	if(iswalpha(wFirstChar) != 0 )
	{
		for(int j=0;j<iMaxColorCommand;j++)
		{
			const	WCHAR*	strCommand = strColorCommandConverting[j][0];

			int	iCmdLen = wcslen(strCommand);
			bool	bCorrect = true;
			for(int k=0;k<iCmdLen;k++)
			{
				WCHAR	wSrcChar = kStringSrc[k];
				WCHAR	wCmdChar = *(strCommand+k);

				if(wSrcChar != wCmdChar)
				{
					bCorrect = false;
					break;
				}
			}

			if(bCorrect)
			{
				kConvertedString += std::wstring(strColorCommandConverting[j][1]);
				kConvertedString += kStringSrc.substr(1);
				return	true;
			}
		}
		return	false;
	}

	//	wFirstChar 가 숫자이면 이모티콘 커멘드
	if(iswdigit(wFirstChar) != 0 )
	{
		std::wstring kNumberString;
		for(int i=0; i<iLength; i++)
		{
			WCHAR	wChar = kStringSrc[i];
			if( iswdigit(wChar) == 0)
			{
				break;
			}

			kNumberString += wChar;
		}

		if(kNumberString.length() == 0)
		{
			return	false;
		}

		int	iEmoticonID = 0;
		if( !g_kEmoFontMgr.Trans_key_value(atoi(MB(kNumberString)), iEmoticonID) )
		{
			return false;
		}
		const	WCHAR	wEmoticonIDChar = iEmoticonID;
		
		kConvertedString += _T("{T=");
		kConvertedString += XUI::XUI_SAVE_FONT;
		kConvertedString += _T("/C=");
		kConvertedString += XUI::XUI_SAVE_COLOR;
		kConvertedString += _T("/}");

		kConvertedString += _T("{C=0xffffffff/T=");
		kConvertedString += kFont;
		kConvertedString += _T("/}");
		kConvertedString += wEmoticonIDChar;

		kConvertedString += _T("{T=");
		kConvertedString += XUI::XUI_RESTORE_FONT;
		kConvertedString += _T("/C=");
		kConvertedString += XUI::XUI_RESTORE_COLOR;
		kConvertedString += _T("/}");

		kConvertedString += kStringSrc.substr(kNumberString.length());

		return	true;
	}

	return	false;
}

std::wstring PgChatMgrClient::ConvertUserCommand(std::wstring const &kString,std::wstring const & kFont)
{
	BM::CAutoMutex kLock(m_kMutex);

	const	int	iLength = kString.length();
	if(iLength == 0)
	{
		return	kString;
	}

	std::wstring	kNewString;

	const	WCHAR	kCommandChar = _T('#');

	std::wstring::size_type kBeforePos = 0;
	std::wstring::size_type kNextPos = kString.find(kCommandChar);
	if( kNextPos == std::wstring::npos )
	{
		//	커멘드가 하나도 없음
		return kString;
	}

	

	while(kNextPos != std::wstring::npos)
	{
		
		kNewString += kString.substr(kBeforePos,kNextPos-kBeforePos);

		//	# 다음부터 Space or '#' 까지 읽는다.
		
		std::wstring::size_type	kNextSpacePos = kString.find(_T(' '),kNextPos+1);
		std::wstring::size_type	kNextCommandPos = kString.find(kCommandChar,kNextPos+1);

		if( kNextSpacePos == std::wstring::npos) kNextSpacePos = iLength;
		if( kNextCommandPos == std::wstring::npos) kNextCommandPos = iLength;

		const	int	iCmdTextLength = ((kNextCommandPos<kNextSpacePos) ? kNextCommandPos : kNextSpacePos) - (kNextPos+1);
		std::wstring kCmdText = kString.substr(kNextPos+1,iCmdTextLength);

		std::wstring kConvertedText;
		if(GetUserCommandConvertedString(kCmdText,kConvertedText,kFont))
		{
			kNewString += kConvertedText;
			kNextPos += kCmdText.length();
		}
		else
		{
			kNewString += kCommandChar;
		}

		kBeforePos = kNextPos+1;
		kNextPos = kString.find(kCommandChar,kBeforePos);
	
	}

	kNewString += kString.substr(kBeforePos,iLength-kBeforePos);

	return	kNewString;
}

bool PgChatMgrClient::SendMToMChat(std::wstring& rkChat, DWORD& Color,std::wstring const& wstrName)
{
	BM::CAutoMutex	kLock(m_kMutex);

	if( rkChat.empty() )
	{
		return true;
	}

	if( !wstrName.size() )
	{
		return false;
	}

	if( !SendChat_MuteCheck() )
	{
		return false;
	}
	
	if(!g_kUnicodeFilter.IsCorrect(UFFC_CHAT, rkChat))
	{// 사용 가능한 유니코드 범주 체크
		SChatLog kChatLog(CT_EVENT);
		g_kChatMgrClient.AddMessage(700407, kChatLog, true);
		return false;
	}

	bool const bFilter = Command_Filter(rkChat);

	BM::Stream kPacket(PT_C_M_REQ_CHAT);
	kPacket.Push((BYTE)CT_MANTOMAN);

	Color = (true == UseColor())?(m_iFontColor):(PgChatMgrUtil::ChatInputColor(CT_MANTOMAN));
	kPacket.Push(rkChat);//2
	kPacket.Push(Color);//3

	XUI::PgExtraDataPackInfo kExtraDataPackInfo;
	kExtraDataPackInfo.PushToPacket(kPacket);//4

	BM::GUID kRecvGuid;
	if(Name2Guid_Find_ByName(wstrName, kRecvGuid))
	{//Guid로
		kPacket.Push((BYTE)0);
		kPacket.Push(kRecvGuid);
	}
	else
	{//이름으로
		kPacket.Push((BYTE)1);
		kPacket.Push(wstrName);
	}

	kPacket.Push(static_cast<BYTE>(MTM_MSG_DEFAULT));

	NETWORK_SEND(kPacket)//패킷전송

	return bSendRet;
}

//채팅 메시지 외부 창 사용
bool PgChatMgrClient::SendChat(std::wstring const &rkChat, std::wstring const &rkName)
{
	BM::CAutoMutex kLock(m_kMutex);

	if( rkChat.empty() )
	{
		return true;
	}
	
	InputChatLog_Add(rkChat);//무조건 채팅 입력 로그 추가
	
	//채팅 금지
	if( !SendChat_MuteCheck() )
	{
		return false;
	}

	//	커맨드
	std::wstring kContents = rkChat;
	std::wstring kName = m_kWhisperTarget;
	int iChatMode = CT_NORMAL;
 	bool const bCommand = Command_Chat(rkChat, iChatMode, kName, kContents);//채팅커멘드 인지 검사.(전송모드가 바뀔 수 있다.)
	//	커맨드
	if( !bCommand 
	&&	Command_UserFunction(rkChat) )
	{
		return true;
	}
	else
	{
		if( bCommand && kContents.empty() )
		{//타입변경 명령어만 있고 내용이 없다면 전송하지 않는다.
			SChatLog kChatLog(CT_EVENT);
			AddMessage(EM_Invalid_Command, kChatLog);
			return false;
		}
	}

	if(CheckSpamChat(rkChat))
	{// 도배이면
		return false;
	}

	XUI::CXUI_Font* pFont = g_kFontMgr.GetFont(FONT_CHAT);
	if( pFont )
	{
		if( m_kTextColorList.empty() )
		{
			m_iFontColor = PgChatMgrUtil::ChatInputColor(static_cast<EChatType>(iChatMode));
		}
		else
		{
			ContTextColorList::iterator it = m_kTextColorList.begin();
			while( it != m_kTextColorList.end() )
			{
				if( (*it).eChatType == m_iChatMode )
				{
					break;
				}
				++it;
			}
			if( it == m_kTextColorList.end() )
			{
				m_iFontColor = PgChatMgrUtil::ChatInputColor(static_cast<EChatType>(iChatMode));
			}
			else
			{
				m_iFontColor = (*it).TextColor;
			}
		}
		
		XUI::CXUI_Style_String	kText(XUI::PgFontDef(pFont, m_iFontColor), kContents);
		return SendChat_Message(iChatMode, kText, kName);
	}
	return false;
}

// 귓속말, 메시지 지연일 경우는 bCheckEqualMessage 체크 하지 않음
void PgChatMgrClient::RecvChatLog_Add(const SChatLog &rkChatLog, bool const bCheckEqualMessage, bool const IsMonster)
{
#if !defined(USE_INB) && defined(PG_LOG_ENABLED)
	if (NiLogger::GetOutputToLogFile(PGLOG_MESSAGE))
	{
		std::wstring kTextFinal;
		std::wstring kText;
		std::wstring kHeader;
		PgChatMgrUtil::HeaderName(rkChatLog.ChatType(), kHeader);
		kText += _T("[") + kHeader + _T("]");

		if( rkChatLog.kCharName.size() )
		{
			kText += rkChatLog.kCharName;
			kText += _T(" : ");
		}

		kText += rkChatLog.kContents;

		kText += _T("\n");

		std::wstring::iterator itor = kText.begin();
		while(itor != kText.end())
		{
			if((*itor) == _T('%'))
			{
				kTextFinal.append(_T("%%"));
			}
			else
			{
				kTextFinal.append(1, *(itor));
			}
			++itor;
		}

		NILOG(PGLOG_MESSAGE, MB(kTextFinal));
	}
#endif	

	XUI::CXUI_Wnd* pNewLog = XUIMgr.Get(_T("ChatOut"));
	if( !pNewLog )
	{
		std::back_inserter(m_kChatLog) = rkChatLog; // 지연
		return;
	}

	if( CT_WHISPER_BYGUID == rkChatLog.ChatType()
	&&	!rkChatLog.kCharName.empty()
	&&	!rkChatLog.bMine )
	{
		InsertWhisper(rkChatLog.kCharName);
	}

	if( CT_NORMAL == rkChatLog.ChatType() )
	{
		XUI::CXUI_Wnd *pkExpeditionWnd = XUIMgr.Get( PgChatMgrUtil::kExpeditionLogWndName );
		if( pkExpeditionWnd )
		{
			XUI::CXUI_Wnd *pkExpeditionLogWnd = pkExpeditionWnd->GetControl(L"FRM_TAIL");
			if( PgClientExpeditionUtil::IsInExpedition() )
			{
				if( pkExpeditionLogWnd || g_kChatWindow.GethWnd() )
				{
					PgChatMgrUtil::UpdateExpeditionLogWnd(pkExpeditionLogWnd, rkChatLog, bCheckEqualMessage, IsMonster);
				}
			}
		}
	}

	XUI::CXUI_Wnd *pkLogWnd = XUIMgr.Get( PgChatMgrUtil::kLogWndName );
	XUI::CXUI_Wnd *pkSysLogWnd = XUIMgr.Get( PgChatMgrUtil::kSysLogWndName );
	if( (pkLogWnd && pkSysLogWnd) || g_kChatWindow.GethWnd() )
	{
		PgChatMgrUtil::UpdateLogWnd(pkLogWnd, rkChatLog, bCheckEqualMessage, IsMonster);
		
		if( (rkChatLog.ChatType() > CT_USERSELECT_MAX)
			&& (rkChatLog.ChatType() != CT_WHISPER_BYGUID)
			&& (rkChatLog.ChatType() != CT_TRADE) )
		{
			if( !m_bSysOutHide )
			{	// 시스템 메세지 창이 보일때만 업데이트.
				PgChatMgrUtil::UpdateLogWnd(pkSysLogWnd, rkChatLog, bCheckEqualMessage, IsMonster);				
			}
		}
	}
	else
	{
		if( PgChatMgrUtil::CP_MAX_All <= m_kChatLog.size() )
		{
			m_kChatLog.pop_front();
		}
		std::back_inserter(m_kChatLog) = rkChatLog; // 지연
	}
}

void PgChatMgrClient::Update()
{
	BM::CAutoMutex kLock(m_kMutex);
	if( m_kChatLog.empty() )
	{
		return;
	}

	XUI::CXUI_Wnd *pkLogWnd = XUIMgr.Get( PgChatMgrUtil::kLogWndName );
	XUI::CXUI_Wnd *pkSysLogWnd = XUIMgr.Get( PgChatMgrUtil::kSysLogWndName );
	if( !pkLogWnd && !pkSysLogWnd && !g_kChatWindow.GethWnd() )
	{
		return;
	}

	ContDelayList::iterator iter = m_kChatLog.begin();
	while( m_kChatLog.end() != iter )
	{
		PgChatMgrUtil::UpdateLogWnd(pkLogWnd, (*iter), false, false);
		
		if( !m_bSysOutHide 
			&& ((*iter).ChatType() > CT_USERSELECT_MAX)
			&& ((*iter).ChatType() != CT_WHISPER_BYGUID)
			&& ((*iter).ChatType() != CT_TRADE) )
		{
			PgChatMgrUtil::UpdateLogWnd(pkSysLogWnd, (*iter), false, false);
		}
		++iter;
	}
	m_kChatLog.clear();
}

void PgChatMgrClient::InsertWhisper(std::wstring const& kName, bool const bIsSort)
{
	BM::CAutoMutex kLock(m_kMutex);

	ContWhisper::iterator	it = m_kWhisperList.begin();

	while( it != m_kWhisperList.end() )
	{
		if( bIsSort )
		{
			if( it->compare(kName) == 0 )
			{
				m_kWhisperList.erase(it);
				break;
			}
		}
		else
		{
			if( it->compare(kName) == 0 )
			{
				return;
			}
		}
		++it;
	}
	m_kWhisperList.push_back(kName);
	if( m_kWhisperList.size() > MAX_WHISPER )
	{
		m_kWhisperList.pop_front();
	}
}

//채팅 관련 Recv
bool PgChatMgrClient::RecvChat(BM::Stream* pkPacket)
{
	BM::CAutoMutex kLock(m_kMutex);

	static std::string const kEtcHeadKey = "ETC";
	static std::string const kSubKey_DenyWhisper = "DENY_CHAT_WHISPER";
	static std::string const kSubKey_DenyParty = "DENY_CHAT_PARTY";
	static std::string const kSubKey_DenyGuild = "DENY_CHAT_GUILD";
	static std::string const kSubKey_DenyFriend = "DENY_CHAT_FRIEND";
	static std::string const kSubKey_DenyTrade = "DENY_CHAT_TRADE";

	BM::GUID kCharGuid;
	BYTE cChatMode = 0;
	pkPacket->Pop(cChatMode);//ChatMode

	bool bShowBallon = true;
	bool bCanGo = true;
	switch(cChatMode)
	{
	case CT_WHISPER_BYGUID:
	case CT_WHISPER_BYNAME:
		{
			bCanGo = 0 == g_kGlobalOption.GetValue(kEtcHeadKey, kSubKey_DenyWhisper);
		}break;
	case CT_PARTY:
		{
			bCanGo = 0 == g_kGlobalOption.GetValue(kEtcHeadKey, kSubKey_DenyParty);
		}break;
	case CT_FRIEND:
		{
			bCanGo = 0 == g_kGlobalOption.GetValue(kEtcHeadKey, kSubKey_DenyFriend);
		}break;
	case CT_GUILD_LOINGUSER:
		{
			bShowBallon = false;
			cChatMode = CT_GUILD;
		}//No break
	case CT_GUILD:
		{
			bCanGo = 0 == g_kGlobalOption.GetValue(kEtcHeadKey, kSubKey_DenyGuild);
		}break;
	case CT_TRADE:
		{
			bCanGo = 0 == g_kGlobalOption.GetValue(kEtcHeadKey, kSubKey_DenyTrade);
		}break;
	default:
		{
			bCanGo = true;
		}break;
	}

	switch(cChatMode)
	{
	case CT_NORMAL:
	case CT_FRIEND:
	case CT_PARTY:
	case CT_GUILD:
	case CT_TEAM:
	case CT_WHISPER_BYGUID:
	case CT_WHISPER_BYNAME:
	case CT_EMOTION:
	case CT_TRADE:
		{
			if( bCanGo )
			{
				SChatLog kChatLog((EChatType)cChatMode);
				pkPacket->Pop(kChatLog.kCharGUID);//CharGuid
				pkPacket->Pop(kChatLog.kCharName);
				pkPacket->Pop(kChatLog.kContents);
				pkPacket->Pop(kChatLog.dwColor);

				{	// 3가지 채팅 타입만 채팅 차단함.
					if( (cChatMode == CT_NORMAL) 
						|| (cChatMode == CT_WHISPER_BYGUID) 
						|| (cChatMode == CT_WHISPER_BYNAME) )
					{
						bool bBlocked = CheckChatBlockCharacter(kChatLog.kCharName, ECBT_NORMAL);
						if( bBlocked )
						{
							return false;
						}
					}
				}

				kChatLog.kContents = ConvertUserCommand(kChatLog.kContents);

				kChatLog.kExtraDataPackInfo.PopFromPacket(*pkPacket);

				XUI::PgExtraDataPack	kCharNameData;
				kCharNameData.AddExtraData(LINK_EDT_CHAR, kChatLog.kCharGUID.str());

				kChatLog.kExtraDataPackInfo.AddExtraDataPack(kCharNameData);

				kChatLog.bMine = g_kPilotMan.IsMyPlayer(kChatLog.kCharGUID);
				if( !kChatLog.bMine )
				{
					Name2Guid_Add(CT_NORMAL, kChatLog.kCharName, kChatLog.kCharGUID); // 캐슁
				}
				if(cChatMode != CT_EMOTION)
				{
					RecvChatLog_Add(kChatLog);	
				}

				if(bShowBallon)
				{
					std::wstring strContent(PgChatMgrUtil::ColorHexToString(kChatLog.dwColor));
					strContent += kChatLog.kContents;
					Character_ChatBallon_Set(kChatLog.kCharGUID, strContent, cChatMode);//캐릭터가 있으면 풍선을 띄운다.
				}
			}
		}break;
	case CT_MANTOMAN:
		{
			BYTE	IsResult;
			std::wstring wstrName;
			SMtoMChatData	Data;

			pkPacket->Pop(kCharGuid);
			pkPacket->Pop(IsResult);

			if( IsResult == 2 )
			{
//				Data.kType = MTM_MSG_DEFAULT;
				Data.kName = TTW(400161);
				Data.kContants = TTW(551003);
				Data.dwColor = 0xFFFFFF99;
//				Data.kLinkItem.clear();
				g_kMToMMgr.Add(kCharGuid, L"",  MTM_MSG_DEFAULT, Data, false );
			}
			else
			{
				BYTE kType = MTM_MSG_DEFAULT;

				pkPacket->Pop(wstrName);
				Name2Guid_Add(CT_NORMAL, wstrName, kCharGuid); // 캐슁

				// 따로 놀자
				pkPacket->Pop(kType);
				pkPacket->Pop(Data.kContants);
				pkPacket->Pop(Data.dwColor);

				if ( MTM_MSG_EMPORIABATTLE == kType )
				{
					kType = MTM_MSG_DEFAULT;
				}

//				PU::TLoadArray_M(*pkPacket, Data.kLinkItem);

				Data.kContants = ConvertUserCommand(Data.kContants);

				if( !IsResult )
				{
					Data.kName = wstrName;
					g_kMToMMgr.Add(kCharGuid, wstrName, kType, Data, false);
				}
				else
				{
					PgPlayer* Player = g_kPilotMan.GetPlayerUnit();
					Data.kName = Player->Name();
					g_kMToMMgr.Add(kCharGuid, Data);
				}
			}
		}break;
	case CT_NOTICE:
		{
			std::wstring kContents;
			pkPacket->Pop(kContents);
			Notice_Show(kContents, EL_Warning);
		}break;
	case CT_ERROR:
		{//에러처리!
			{
				BYTE cErrorCode;
				pkPacket->Pop(cErrorCode);
				if(WCR_NotFoundName==cErrorCode || WCR_NotFoundGuid==cErrorCode)
				{
					//SChatLog kChatLog((EChatType)cChatMode);
					SChatLog kChatLog(CT_EVENT);
					AddMessage(EM_FreindFailed, kChatLog, true);
				}
			}
		}break;
	case CT_RAREMONSTERGEN:
		{
			int iMonsterNo = 0;
			BM::GUID kGuid;
			pkPacket->Pop(kGuid);
			pkPacket->Pop(iMonsterNo);

			GET_DEF(CMonsterDefMgr, kMonsterDefMgr);
			CMonsterDef const * pDef = kMonsterDefMgr.GetDef(iMonsterNo);
			if(pDef)
			{
				wchar_t const * pkMonName = NULL;
				if(g_pkWorld
					&& true == GetDefString(pDef->NameNo(),pkMonName)
					)
				{
					std::wstring kMsg;
					WstringFormat( kMsg, MAX_PATH, TTW(4001000).c_str(),GetMapName(g_pkWorld->MapNo()).c_str(), pkMonName);
					Notice_Show(kMsg, EL_Normal);
				}
			}
		}break;
	case CT_RAREMONSTERDEAD:
		{
			int iMonsterNo = 0;
			std::wstring kOwnerName;

			BM::GUID kGuid;
			pkPacket->Pop(kGuid);
			pkPacket->Pop(iMonsterNo);
			pkPacket->Pop(kOwnerName);

			GET_DEF(CMonsterDefMgr, kMonsterDefMgr);
			CMonsterDef const * pDef = kMonsterDefMgr.GetDef(iMonsterNo);
			if(pDef)
			{
				wchar_t const * pkMonName = NULL;
				if(g_pkWorld
					&& true == GetDefString(pDef->NameNo(),pkMonName)
					)
				{
					std::wstring kMsg;
					WstringFormat( kMsg, MAX_PATH, TTW(4001001).c_str(), kOwnerName.c_str(),GetMapName(g_pkWorld->MapNo()).c_str(),pkMonName);
					Notice_Show(kMsg, EL_Normal);
				}
			}
		}break;
	case CT_OXQUIZEVENT:
		{
			BM::GUID kEventGuid;
			std::wstring kContents;
			pkPacket->Pop(kEventGuid);
			pkPacket->Pop(kContents);
			Notice_Show(kContents,EL_Normal);
		}break;
	case CT_CASHITEM_GIFT:
		{
			int iMsgNo = 0;
			int iEventNo = 0;
			pkPacket->Pop(iMsgNo);
			pkPacket->Pop(iEventNo);

			std::wstring kMsg;
			WstringFormat( kMsg, MAX_PATH, TTW(iMsgNo).c_str(), iEventNo);
			Notice_Show(kMsg, EL_Normal);
		}break;
	case CT_MYHOME_PUBLIC:
		{
			SChatLog kChatLog((EChatType)cChatMode);
			pkPacket->Pop(kChatLog.kCharGUID);//CharGuid
			pkPacket->Pop(kChatLog.kCharName);
			pkPacket->Pop(kChatLog.kContents);
			pkPacket->Pop(kChatLog.dwColor);

			kChatLog.kContents = ConvertUserCommand(kChatLog.kContents);

			kChatLog.kExtraDataPackInfo.PopFromPacket(*pkPacket);

			XUI::PgExtraDataPack	kCharNameData;
			kCharNameData.AddExtraData(LINK_EDT_CHAR, kChatLog.kCharGUID.str());

			kChatLog.kExtraDataPackInfo.AddExtraDataPack(kCharNameData);

			kChatLog.bMine = g_kPilotMan.IsMyPlayer(kChatLog.kCharGUID);

			lwHomeRenew::AddMyhomeChatAllChatContent(kChatLog);
		}break;
	case CT_MYHOME_PRIVATE:
		{
		}break;
	//case CT_Max:
	//case CT_NONE:
	//case CT_EVENT:
	//case CT_EVENT_SYSTEM:
	//case CT_EVENT_GAME:
	default:
		{
		}break;
	}

	return true;
}

//채팅 입력 로그 관련
void PgChatMgrClient::InputChatLog_Add(std::wstring const &rkChat)//입력로그 추가
{
	BM::CAutoMutex kLock(m_kMutex);

	ContInputList::iterator kIter = m_kLog_InputChat.begin();
	for(; m_kLog_InputChat.end() != kIter; ++kIter)
	{
		if((*kIter) == rkChat)//중복 하면 이전걸 지운다.
		{
			kIter = m_kLog_InputChat.erase(kIter);
			break;
		}
	}

	// 채팅 기록이 Max 이상이면 제거
	if( MaxInputLog() < m_kLog_Input.size()+1 )
	{
		m_kLog_Input.erase(m_kLog_Input.begin());
	}
	m_kLog_Input.push_back(SInputChatLog(rkChat));

	if( MaxInputLog() < m_kLog_InputChat.size()+1 )
	{
		m_kLog_InputChat.erase(m_kLog_InputChat.begin());
	}

	m_kLog_InputChat.push_back(rkChat);
	InputChatLog_Reset();
}

//채팅 입력 로그 관련
bool PgChatMgrClient::CheckSpamChat(std::wstring const &rkChat)//입력로그 추가
{
	BM::CAutoMutex kLock(m_kMutex);
	
	if(1 >= m_kLog_Input.size()) 
	{
		return false;
	}
	
	//최근에 들어온것은 자신의 것이므로 잠시 빼둔다
	SInputChatLog const kBackEle = m_kLog_Input.back();
	m_kLog_Input.pop_back();

	bool bBadInput = false;
	float fCurTime = g_pkApp->GetAccumTime();
	float fDistTimeToPrevInput = 0;
	
	if(SpamChkFlag() & ESCS_ChainInput
		&& !m_kLog_Input.empty())
	{// Block ChainInput
		fDistTimeToPrevInput = fCurTime-m_kLog_Input.rbegin()->fInputTime;
		if(m_fChainWatchT >= fDistTimeToPrevInput)
		{//1. 바로 이전과 비교해서 m_fChainWatchT 보다 작으면 금지
			bBadInput = true;
			IncBlockChatTime(m_fChainBlockTime);
			//WriteToConsole("1타입 : %f 증가\n", m_fChainBlockTime);
		}
	}

	if(!bBadInput
		&& SpamChkFlag() & ESCS_LooseChainInput
		&&m_kLog_Input.size() >= m_iLChainCnt)
	{// Block LooseChainInput
		float fOldTime = 0.f;
		ContInputLog::reverse_iterator kReverse_Iter = m_kLog_Input.rbegin();
		
		for(unsigned int i=0; i < m_iLChainCnt; ++i)
		{// 마지막 입력으로부터 N개 만큼 이전에 입력된 문장의
			if(kReverse_Iter != m_kLog_Input.rend())
			{// 시간 값을 가져와
				fOldTime = kReverse_Iter->fInputTime;
			}
			++kReverse_Iter;
		}
		if(m_fLChainWatchT >= fCurTime-fOldTime)
		{// N개가 특정 시간 이내에 입력 되었다면 도배로 판명 한다
			bBadInput = true;
			IncBlockChatTime(m_fLChainBlockTime);
			//WriteToConsole("2타입 : %f 증가\n", m_fLChainBlockTime);
		}
	}

	if(!bBadInput
		&& SpamChkFlag() & ESCS_MacroInput)
	{// Block MacroInput
		////1. 현재 입력 문장과 같은 문장을 찾음
		/////if(찾았을때)
		//{
		//	//1-1. 사이시간 = 현재 입력 문장의 입력시간 - 찾은 문장의 입력시간
		//	///if(사이시간이 오차범주 이내이면)
		//	{
		//		//1-1-1. 현재 입력 문장의 입력 시간 = 찾은 문장의 입력 시간
		//		//1-1-2. ++카운트
		//		///if(카운트가 블록해야할만큼 이면)
		//		{
		//			//도배 판정!
		//		}
		//	}
		//}
		unsigned int uiCnt=0;
		float fCurStrInputTime = fCurTime;
		float fTermTime = 0.0f;
		ContInputLog::reverse_iterator kReverse_Iter = m_kLog_Input.rbegin();
		for(;m_kLog_Input.rend() != kReverse_Iter; ++kReverse_Iter)
		{
			if(rkChat == kReverse_Iter->kChat)
			{//1. 현재 입력 문장과 같은 문장을 찾음
				//1-1. 사이시간 = 현재 입력 문장의 입력시간 - 찾은 문장의 입력시간
				float const fDiffTime = fCurStrInputTime - kReverse_Iter->fInputTime;
				if(0.0f == fTermTime)
				{// 같은 문장 2개의 차이 시간 1개로는 분별을 할수 없으므로 차이값만 저장하고 다음을 찾음
					fTermTime = fDiffTime;
				}
				else
				{
					float const fMinTime = fTermTime-m_fMacroRangeTime;
					float const fMaxTime = fTermTime+m_fMacroRangeTime;
					//WriteToConsole("Min %2.3f, MaxL %2.3f\n", fMinTime, fMaxTime);
					//WriteToConsole("Tem %2.3f\n", fTermTime);
					//WriteToConsole("Dif %2.3f\n", fDiffTime);
					if(fMinTime <= fDiffTime
						&& fDiffTime <= fMaxTime)
					{///사이시간이 오차범주 이내이면
						//1-1-2. ++카운트
						++uiCnt;
						if(uiCnt >= m_iMaroCnt)
						{/// 카운트가 블록해야할만큼 이면 도배 판정!
							bBadInput = true;
							IncBlockChatTime(m_fMacroBlockTime);
							//WriteToConsole("3타입 : %f 증가\n", m_fMacroBlockTime);
							break;
						}
					}
					else
					{// 사이시간이 같지 않으면 그만둠
						break;
					}
				}
				//1-1-1. 현재 입력 문장의 입력 시간 = 찾은 문장의 입력 시간
				fCurStrInputTime = kReverse_Iter->fInputTime;
			}
		}
		//if(bBadInput)
		//{// 체크한후 다시 횟수를 채우게 할필요는 없으므로 일단 제거함(후에 필요할수도?)
		//	ContInputLog::iterator itor = m_kLog_Input.begin();
		//	while(m_kLog_Input.end() != itor)
		//	{// 모두 삭제
		//		if((*itor) == rkChat)
		//		{
		//			itor = m_kLog_Input.erase(itor);
		//		}
		//		else
		//		{
		//			++itor;
		//		}
		//	}
		//}
		//WriteToConsole("\n\n");
	}
	

	if(!bBadInput
		&& SpamChkFlag() & ESCS_SameInput)
	{// Block SameInput
		unsigned int uiCount = 0;
		ContInputLog::reverse_iterator kReverse_Iter = m_kLog_Input.rbegin();
		for(;m_kLog_Input.rend() != kReverse_Iter; ++kReverse_Iter)
		{
			float const fDiffTime = fCurTime - kReverse_Iter->fInputTime;
			if(m_fSameWatchT < fDiffTime)
			{// m_fSameInput초 이내에
				break;
			}
			if(rkChat == kReverse_Iter->kChat)
			{
				++uiCount;
				if(m_iSameCnt <= uiCount && m_fSameWatchT >= fDiffTime)
				{// 4. 같은 입력이 m_iSameCnt개 이상이면 금지
					bBadInput = true;
					IncBlockChatTime(m_fSameBlockTime);
					//WriteToConsole("4타입 : %f 증가\n", m_fSameBlockTime);
					break;
				}
			}
		}
	}

	//모두 사용했으므로 다시 추가
	m_kLog_Input.push_back(kBackEle);
	
	float const fElapsedBlockTime = g_pkApp->GetAccumTime() - StartBlockTime();
	//WriteToConsole("지연시간 : %f\n Block시간:%f\n\n", fElapsedBlockTime, BlockTime());

	if(bBadInput)
	{
		SChatLog kChatLog(CT_EVENT);
		AddMessage(EM_DoNot_Keyboard_Warior, kChatLog, true);
		StartBlockTime(g_pkApp->GetAccumTime());				//채팅 금지 시작
		//return ECB_BLOCK;//도배이면
		return true;
	}
	else if(fElapsedBlockTime <= BlockTime())
	{// 도배로 인한 채팅 금지이면 채팅을 할 수 없음
		SChatLog kChatLog(CT_EVENT);
		std::wstring const kFormStr(TTW(790630));
		wchar_t szBuf[MAX_PATH] ={0,};
		int const iRemainBlockTime = static_cast<int>(BlockTime() - fElapsedBlockTime);
		wsprintfW(szBuf, kFormStr.c_str(), iRemainBlockTime);
		AddLogMessage(kChatLog, szBuf, true);
		return true;
	}
	else
	{
		BlockTime(0.0f);
	}

	return false;
}

bool PgChatMgrClient::InputChatLog_Get(std::wstring &rkChat, bool bPrev)//입력로그 순차얻기
{
	BM::CAutoMutex kLock(m_kMutex);

	if( m_kLog_InputChat.empty() )
	{
		return false;
	}

	if(-1 == m_nLog_InputChat_Old)
	{//처음일 때
		rkChat = *m_kLog_InputChatIter;
		m_nLog_InputChat_Old = bPrev;
		return true;
	}

	if(bPrev)
	{//Prev(UP)
		++m_kLog_InputChatIter;
		if(m_kLog_InputChat.rend() == m_kLog_InputChatIter)
		{
			--m_kLog_InputChatIter;
		}
	}
	else
	{//Next(DOWN)
		if(m_kLog_InputChat.rbegin() != m_kLog_InputChatIter)
		{
			--m_kLog_InputChatIter;
		}
	}

	rkChat = *m_kLog_InputChatIter;//
	return true;
}

void PgChatMgrClient::InputChatLog_Reset()
{
	BM::CAutoMutex kLock(m_kMutex);

	m_kLog_InputChatIter = m_kLog_InputChat.rbegin();
	m_nLog_InputChat_Old = -1;
}

//이모션 관련 (Send)
bool PgChatMgrClient::SendChat_InputNow(bool bPopup)
{
	BM::CAutoMutex kLock(m_kMutex);

	BYTE	ChatMode = (BYTE)m_iChatMode;

	if( bPopup )
	{
		switch(m_iChatMode)
		{
		case CT_NORMAL://일반채팅, 파티만 보낸다.
			{
				m_kBallonState = BS_Normal_Open;
				break;
			}
		case CT_PARTY:
			{
				if( PgClientPartyUtil::IsInParty() )
				{
					m_kBallonState = BS_Party_Open;
				}
				else
				{
					return false;
				}
			}break;
		case CT_GUILD:
			{
				if( g_kGuildMgr.IamHaveGuild() )
				{
					m_kBallonState = BS_Guild_Open;
				}
				else
				{
					return false;
				}
			}break;
		default:
			{
				return false;
			};
		}
	}
	else
	{
		switch( m_kBallonState )
		{
		case BS_Normal_Open:{ ChatMode = (BYTE)CT_NORMAL;	} break;
		case BS_Party_Open:	{ ChatMode = (BYTE)CT_PARTY;	} break;
		case BS_Guild_Open: { ChatMode = (BYTE)CT_GUILD;	} break;
		default:
			{
				return false;
			} break;
		}
		m_kBallonState = BS_Not_Open;
	}

	BM::Stream kPacket(PT_C_M_REQ_CHAT_INPUTNOW);
	kPacket.Push(ChatMode);
	kPacket.Push(bPopup);
	
	NETWORK_SEND(kPacket)
	
	return bSendRet;
}

bool PgChatMgrClient::RecvChat_InputNow(BM::Stream* pkPacket)
{
	BM::CAutoMutex kLock(m_kMutex);

	BM::GUID kCharGuid;
	BYTE cChatMode;
	bool bPopup;

	pkPacket->Pop(kCharGuid);
	pkPacket->Pop(cChatMode);
	pkPacket->Pop(bPopup);
	
	PgPilot	*pkPilot = g_kPilotMan.FindPilot(kCharGuid);
	if(pkPilot)
	{
		PgActor *pkActor = dynamic_cast<PgActor *>(pkPilot->GetWorldObject());
		if(pkActor)
		{
			if(bPopup)
			{
				pkActor->ShowChatBalloon((EChatType)cChatMode, TTW(799251), 0, true);
				//ShowChatBalloon_InputNow();
			}
			else
			{
				pkActor->ShowChatBalloon_Clear();//(...)을 감춘다
			}
			return true;
		}
	}
	return false;
}

bool PgChatMgrClient::Self_InputNow(bool const bPopup, int const iChatMode)
{
	PgActor *pkActor = g_kPilotMan.GetPlayerActor();
	if(pkActor)
	{
		if(bPopup)
		{
			pkActor->ShowChatBalloon((EChatType)iChatMode, TTW(799251), 0, true);
		}
		else
		{
			pkActor->ShowChatBalloon_Clear();//(...)을 감춘다
		}
		return true;
	}
	return false;
}

void PgChatMgrClient::ClearChatBalloon()
{
	PgActor *pkActor = g_kPilotMan.GetPlayerActor();
	if(pkActor)
	{
		pkActor->ShowChatBalloon_Clear();//(...)을 감춘다
	}
}

//
void PgChatMgrClient::Character_ChatBallon_Set(BM::GUID &rkCharGuid, std::wstring const &rkWString, int const iChatType)
{//지정된 Guid캐릭터 말풍선 삽입
	BM::CAutoMutex kLock(m_kMutex);

	PgPilot	*pkPilot = g_kPilotMan.FindPilot(rkCharGuid);
	if(pkPilot)
	{
		PgActor *pkActor = dynamic_cast<PgActor *>(pkPilot->GetWorldObject());
		if(pkActor)
		{
			switch(iChatType)
			{
			case CT_NORMAL://일반 채팅만
			case CT_PARTY://파티 채팅만
			case CT_EMOTION://이모션(단축키 사용)
			case CT_GUILD:// 길드 채팅만
				{
					pkActor->ShowChatBalloon((EChatType)iChatType, rkWString);//말풍선 표시
				}break;
			default:
				{//그외 채팅은 표시 하지 않는다.
				}break;
			}
		}
	}
	return;
}

//Name 2 Guid 관련 함수
void PgChatMgrClient::Name2Guid_Add(EChatType const eType, std::wstring const &rkCharName, BM::GUID const &rkCharGuid)
{
	BM::CAutoMutex kLock(m_kMutex);

	//ContName2GuidList::value_type *pkNewName2Guid = NULL;
	ContName2GuidList::iterator result_iter = m_kCache_Name2Guid.end();

	ContName2Guid::const_iterator find_iter1 = m_kLog_Name2Guid.find( rkCharName );
	ContGuid2Name::const_iterator find_iter2 = m_kLog_Guid2Name.find( rkCharGuid );
	if( m_kLog_Name2Guid.end() == find_iter1
	&&	m_kLog_Guid2Name.end() == find_iter2 )
	{
		result_iter = m_kCache_Name2Guid.insert( m_kCache_Name2Guid.end(), ContName2GuidList::value_type(eType, rkCharName, rkCharGuid) );
	}

	if( m_kCache_Name2Guid.end() != result_iter )
	{
		std::pair<ContName2Guid::iterator, bool> kRet1 = m_kLog_Name2Guid.insert( std::make_pair(rkCharName, result_iter) );
		std::pair<ContGuid2Name::iterator, bool> kRet2 = m_kLog_Guid2Name.insert( std::make_pair(rkCharGuid, result_iter) );
	}
}

bool PgChatMgrClient::Name2Guid_Find_ByName(std::wstring const &rkCharName, BM::GUID &rkCharGuid) const
{
	BM::CAutoMutex kLock(m_kMutex);

	ContName2Guid::const_iterator find_iter =  m_kLog_Name2Guid.find( rkCharName );
	if( m_kLog_Name2Guid.end() == find_iter )
	{
		return false;
	}
	ContName2GuidList::value_type const &rkElement = *(*find_iter).second;
	rkCharGuid = rkElement.kCharGuid;
	return true;
}

bool PgChatMgrClient::Name2Guid_Find_ByGuid(BM::GUID const &rkCharGuid, std::wstring &rkCharName) const
{
	BM::CAutoMutex kLock(m_kMutex);

	ContGuid2Name::const_iterator find_iter = m_kLog_Guid2Name.find( rkCharGuid );
	if( m_kLog_Guid2Name.end() == find_iter )
	{
		return false;
	}
	ContName2GuidList::value_type const &rkElement = *(*find_iter).second;
	rkCharName = rkElement.kCharName;
	return true;
}

PgChatMgrClient::ContName2GuidList::iterator PgChatMgrClient::Name2Guid_Remove(ContName2GuidList::iterator iter)
{
	if( m_kCache_Name2Guid.end() == iter )
	{
		return iter;
	}

	ContName2GuidList::value_type const &rkElement = (*iter);
	m_kLog_Name2Guid.erase( rkElement.kCharName );
	m_kLog_Guid2Name.erase( rkElement.kCharGuid );
	return m_kCache_Name2Guid.erase( iter );
}

void PgChatMgrClient::Name2Guid_Remove(EChatType const eType)
{
	ContName2GuidList::iterator iter = m_kCache_Name2Guid.begin();
	while( m_kCache_Name2Guid.end() != iter )
	{
		if( eType == (*iter).eType )
		{
			(*iter).eType = CT_NORMAL;//삭제 대신에 맵이동시 삭제 가능한 것으로 만든다.
		}
		++iter;
	}
}

bool PgChatMgrClient::Name2Guid_Remove(BM::GUID const &rkCharGuid)
{
	BM::CAutoMutex kLock(m_kMutex);
	ContGuid2Name::const_iterator find_iter = m_kLog_Guid2Name.find( rkCharGuid );
	if( m_kLog_Guid2Name.end() == find_iter )
	{
		return false;
	}
	Name2Guid_Remove( (*find_iter).second );
	return true;
}

bool PgChatMgrClient::Name2Guid_Remove(std::wstring const &rkCharName)
{
	BM::CAutoMutex kLock(m_kMutex);
	ContName2Guid::const_iterator find_iter = m_kLog_Name2Guid.find( rkCharName );
	if( m_kLog_Name2Guid.end() == find_iter )
	{
		return false;
	}
	Name2Guid_Remove( (*find_iter).second );
	return true;
}

void PgChatMgrClient::Name2Guid_Clear()
{
	BM::CAutoMutex kLock(m_kMutex);

	ContName2GuidList::iterator iter = m_kCache_Name2Guid.begin();
	while( m_kCache_Name2Guid.end() != iter )
	{
		ContName2GuidList::value_type &rkElement = (*iter);

		switch( rkElement.eType )
		{
		case CT_PARTY:
		case CT_FRIEND:
		case CT_GUILD:
			{
				// Don't Clear
			}break;
		default:
			{
				m_kLog_Name2Guid.erase( rkElement.kCharName );
				m_kLog_Guid2Name.erase( rkElement.kCharGuid );
				iter = m_kCache_Name2Guid.erase( iter );

				continue;
			}break;
		}

		++iter;
	}
}


bool PgChatMgrClient::CheckChatOut()
{ 
	BM::CAutoMutex kLock(m_kMutex);
	return (g_kChatWindow.GethWnd() == NULL)?(false):(true);
}

bool PgChatMgrClient::SetChatOutFocus()
{
	BM::CAutoMutex kLock(m_kMutex);
	return g_kChatWindow.SetChatOutFocus();
}

// Tab으로 귓속말 대상 변경.
std::wstring PgChatMgrClient::SetWhisperByTab(void)
{
	BM::CAutoMutex kLock(m_kMutex);

	// 귓속말 리스트가 있을 때만.
	if( m_kWhisperList.size() )
	{
		ContWhisper::reverse_iterator rit = m_kWhisperList.rbegin();
		
		// 귓속말 대상이 없으면 귓속말 리스트 맨처음 이름으로.
		if( 0 == m_kWhisperTarget.size() )
		{
			return *rit;
		}

		while( rit != m_kWhisperList.rend() )
		{
			if( 0 == rit->compare(m_kWhisperTarget) )
			{
				// 현재 귓속말 대화중인 이름의 다음을 선택한다.
				++rit;
				
				// 귓속말 리스트의 맨 마지막 이름이었으면, 다시 처음으로.
				if( rit == m_kWhisperList.rend() )
				{
					rit = m_kWhisperList.rbegin();
				}

				return *rit;
			}
			++rit;
		}
	}

	return std::wstring(_T(""));
}

bool PgChatMgrClient::SetWhisperDlg(XUI::CXUI_Wnd* pkWnd)
{
	BM::CAutoMutex kLock(m_kMutex);

	if( m_kWhisperList.size() )
	{
		XUI::CXUI_List* pList = dynamic_cast<XUI::CXUI_List*>(pkWnd->GetControl(_T("LST_SELECT_TYPE")));
		if( pList )
		{
			pList->DeleteAllItem();
			ContWhisper::const_iterator	iter = m_kWhisperList.begin();
			LONG ItemSizeY = 0;

			while( iter != m_kWhisperList.end() )
			{
				XUI::SListItem* pItem = pList->AddItem(_T("FRM_WHISPER_NAME"));
				if( pItem )
				{
					XUI::CXUI_Wnd* pkItemWnd = pItem->m_pWnd;
					pkItemWnd->Text((*iter));
					ItemSizeY = pkItemWnd->Size().y;
				}
				++iter;
			}

			int const iItemCnt = pList->GetTotalItemCount();

			pList->Size(POINT2(pList->Size().x, (ItemSizeY * iItemCnt)));
			pkWnd->Size(POINT2(pkWnd->Size().x, (ItemSizeY * iItemCnt)));
		}

		XUI::CXUI_Wnd* pChatBar = XUIMgr.Get(_T("ChatBar"));
		if( pChatBar )
		{
			XUI::CXUI_Wnd* pTemp = pChatBar->GetControl(_T("BTN_WHISPER"));
			if( pTemp )
			{
				pkWnd->Location(pChatBar->Location().x + pTemp->Location().x, pChatBar->Location().y + pTemp->Location().y - pkWnd->Size().y);
			}
		}
	}
	else
	{
		//SChatLog kChatLog(CT_EVENT);//잘못된 명령어 입니다.
		//AddMessage(404006, kChatLog);
		pkWnd->Close();
		return true;
	}

	return false;
}

PgChatMgrClient::SChatCommand const* PgChatMgrClient::GetCommandType(std::wstring const& Cmd)
{
	std::wstring kTemp = Cmd;
	UPR(kTemp);
	ContChatCommand::iterator	iter = m_kChatCommand.find(kTemp);
	if(m_kChatCommand.end() != iter)
	{
		return &iter->second;
	}

	return NULL;
}

PgChatMgrClient::SChatCommand const* PgChatMgrClient::GetCommandTypeToMode(EChatType const Mode)
{
	ContChatCommand::iterator	iter = m_kChatCommand.begin();
	while( iter != m_kChatCommand.end() )
	{
		if( iter->second.eChatType == Mode )
		{
			if( Mode != CT_WHISPER_BYNAME || iter->second.iMinCount != 2)
			{
				return &iter->second;
			}
		}
		++iter;
	}
	return NULL;
}

bool PgChatMgrClient::SendChatDlg(std::wstring const& Text)
{
	BM::CAutoMutex kLock(m_kMutex);
	return SendChat(Text, L"");
//CreateChatDlg()
}


bool PgChatMgrClient::CheckSysCommand(std::wstring const& kText)
{
	BM::CAutoMutex kLock(m_kMutex);

	if( kText[0] == _T('/'))
	{
		if( kText.size() < 2 )
		{
			return false;
		}

		std::vector< std::wstring >	kVec;
		BreakSpace(kText, kVec);
		std::wstring	kContants;

		SChatCommand const*	kTargetCmd = NULL;

		std::vector< std::wstring >::iterator	iter = kVec.begin();
		if( iter == kVec.end() )
		{
			return false;
		}

		if( iter->compare(L"MODE") == 0 )
		{
			if( ++iter != kVec.end() )
			{
				kTargetCmd = GetCommandTypeToMode((EChatType)_wtoi(iter->c_str()));
				kVec.erase(iter);
			}
		}
		else
		{
			kTargetCmd = GetCommandType(*iter);
		}


		if( !kTargetCmd )
		{	
			return false;
		}
		switch( kTargetCmd->eChatType )
		{
		case CT_DUEL:
			{
				if( kVec.size() < 2 )
				{
					return false;
				}

				PgPilot* pkPilot = g_kPilotMan.FindPilot(kVec.at(1));
				if(pkPilot)
				{
					BM::GUID kID = pkPilot->GetGuid();
					OnClickPvP_ReqDuel( lwGUID(kID) );
				}
				else
				{
					::Notice_Show_ByTextTableNo(201212, EL_Warning);
				}
			}return true;
		}
	}
	return false;

}

// 스페이스에 의한 채팅 모드 변경
bool PgChatMgrClient::CheckChatCommand(std::wstring const& kText)
{
	BM::CAutoMutex kLock(m_kMutex);

	if( kText[0] == _T('/'))
	{
		if( kText.size() < 2 )
		{
			return false;
		}

		std::vector< std::wstring >	kVec;
		BreakSpace(kText, kVec);
		std::wstring	kContants;

		SChatCommand const*	kTargetCmd = NULL;

		std::vector< std::wstring >::iterator	iter = kVec.begin();
		if( iter == kVec.end() )
		{
			return false;
		}

		if( iter->compare(L"MODE") == 0 )
		{
			if( ++iter != kVec.end() )
			{
				kTargetCmd = GetCommandTypeToMode((EChatType)_wtoi(iter->c_str()));
				kVec.erase(iter);
			}
		}
		else
		{
			kTargetCmd = GetCommandType(*iter);
		}


		if( !kTargetCmd )
		{	
			return false;
		}

		XUI::CXUI_Wnd* pChatOut = XUIMgr.Get(_T("ChatOut"));
		if( !pChatOut )
		{
			return false;
		}
		XUI::CXUI_Wnd* pBtnParent = pChatOut->GetControl(L"CHAT_BG");
	//	if( !pBtnParent )
	//	{
	//		return false;
	//	}
		switch( kTargetCmd->eChatType )
		{
		case CT_DUEL:
			{
				if( kVec.size() < 2 )
				{
					return false;
				}
			}return true;

		case CT_WHISPER_BYNAME://사용자 입력은 이걸로 들어온다.
			{
				if(kTargetCmd->iMinCount != 3)// /r
				{
					if( !m_kWhisperList.size() )
					{
						return false;
					}

					ContWhisper::reverse_iterator	rit = m_kWhisperList.rbegin();
					if( rit != m_kWhisperList.rend() )
					{
						m_kWhisperTarget = (*rit);
						m_iChatMode = kTargetCmd->eChatType;
						if( m_kTextColorList.empty() )
						{
							m_iFontColor = PgChatMgrUtil::ChatInputColor(static_cast<EChatType>(m_iChatMode));
						}
						else
						{
							ContTextColorList::iterator it = m_kTextColorList.begin();
							while( it != m_kTextColorList.end() )
							{
								if( (*it).eChatType == m_iChatMode )
								{
									break;
								}
								++it;
							}
							if( it == m_kTextColorList.end() )
							{
								m_iFontColor = PgChatMgrUtil::ChatInputColor(static_cast<EChatType>(m_iChatMode));
							}
							else
							{
								m_iFontColor = (*it).TextColor;
							}
						}
					}

					if( kVec.size() > 1 )
					{
						size_t nPos = kText.find(kVec[1], 0);
						kContants = kText.substr(nPos, kText.size() - nPos);
					}
				}
				else// /w
				{
					if( kVec.size() < 2 )
					{
						return false;
					}
					
					if( MAX_CHARACTERNAME_LEN < (kVec[1].size() * sizeof(std::wstring::value_type)) )
					{
						lwAddWarnDataTT(404024);
						return true;
					}

					m_kWhisperTarget = kVec[1];
					m_iChatMode = kTargetCmd->eChatType;
					if( m_kTextColorList.empty() )
					{
						m_iFontColor = PgChatMgrUtil::ChatInputColor(static_cast<EChatType>(m_iChatMode));
					}
					else
					{
						ContTextColorList::iterator it = m_kTextColorList.begin();
						while( it != m_kTextColorList.end() )
						{
							if( (*it).eChatType == m_iChatMode )
							{
								break;
							}
							++it;
						}
						if( it == m_kTextColorList.end() )
						{
							m_iFontColor = PgChatMgrUtil::ChatInputColor(static_cast<EChatType>(m_iChatMode));
						}
						else
						{
							m_iFontColor = (*it).TextColor;
						}
					}

					PgPlayer *pkPC = g_kPilotMan.GetPlayerUnit();
					if( pkPC )
					{
						InsertWhisper(m_kWhisperTarget, false);
					}
					if( kVec.size() > 2 )
					{
						size_t nPos = kText.find(kVec[2], 0);
						kContants = kText.substr(nPos, kText.size() - nPos);
					}
				}
			}break;
		default: // /p /f /g /s
			{
				m_kWhisperTarget = L"";
				m_iChatMode = kTargetCmd->eChatType;
				if( m_kTextColorList.empty() )
				{
					m_iFontColor = PgChatMgrUtil::ChatInputColor(static_cast<EChatType>(m_iChatMode));
				}
				else
				{
					ContTextColorList::iterator it = m_kTextColorList.begin();
					while( it != m_kTextColorList.end() )
					{
						if( (*it).eChatType == m_iChatMode )
						{
							break;
						}
						++it;
					}
					if( it == m_kTextColorList.end() )
					{
						m_iFontColor = PgChatMgrUtil::ChatInputColor(static_cast<EChatType>(m_iChatMode));
					}
					else
					{
						m_iFontColor = (*it).TextColor;
					}
				}

				if( kVec.size() > 1 )
				{
					size_t nPos = kText.find(kVec[1], 0);
					kContants = kText.substr(nPos, kText.size() - nPos);
				}

				int nChatTab = 1;
				switch( kTargetCmd->eChatType )
				{
				case CT_NORMAL:
				case CT_TEAM:
					{
						nChatTab = 1;
					}break;
				case CT_GUILD:
					{
						nChatTab = 2;
					}break;
				case CT_PARTY:
					{
						nChatTab = 3;
					}break;
				case CT_TRADE:
					{
						nChatTab = 4;
					}break;
				default:
					{
						nChatTab = 1;
					}break;
				}
				if( pBtnParent )
				{
					lua_tinker::call<void, lwUIWnd, int>("ChatMode_FilterSet", lwUIWnd(pBtnParent), nChatTab);
				}
			}break;
		}

		if( m_kTextColorList.empty() )
		{
			m_iFontColor = PgChatMgrUtil::ChatInputColor(static_cast<EChatType>(m_iChatMode));
		}
		else
		{
			ContTextColorList::iterator it = m_kTextColorList.begin();
			while( it != m_kTextColorList.end() )
			{
				if( (*it).eChatType == m_iChatMode )
				{
					break;
				}
				++it;
			}
			if( it == m_kTextColorList.end() )
			{
				m_iFontColor = PgChatMgrUtil::ChatInputColor(static_cast<EChatType>(m_iChatMode));
			}
			else
			{
				m_iFontColor = (*it).TextColor;
			}
		}

		SetChatControl(kTargetCmd->eChatType, m_kWhisperTarget, kContants);
	}
	else
	{
		return false;
	}

	return true;
}

//	전송 전 갓커맨드 체크
bool PgChatMgrClient::SendChat_Check(std::wstring const& kText, bool const bCheckSpamChat)
{
	BM::CAutoMutex kLock(m_kMutex);

	if( !kText.size() )
	{
		return true;
	}
	
	if(bCheckSpamChat)
	{// 도배 필터링 하지 않는 경우는 로그에 기억하지 않는다
		InputChatLog_Add(kText);
	}
	
	if( CheckChatCommand( kText ) )
	{
		return false;
	}
	if( Command_UserFunction(kText) )
	{//	커맨드
		return true;
	}
	if( !SendChat_MuteCheck() )
	{//	채금
		return true;
	}

	if(bCheckSpamChat
		&& CheckSpamChat(kText))
	{// 도배체크를 사용하고, 도배 이면
		return true;
	}

	return SendChat_Message(m_iChatMode, XUI::CXUI_Style_String(XUI::PgFontDef(g_kFontMgr.GetCurrentFont()),kText), m_kWhisperTarget);
}
bool PgChatMgrClient::SendChat_CheckSS(XUI::CXUI_Style_String const& kText, bool const bCheckSpamChat)
{
	BM::CAutoMutex kLock(m_kMutex);

	if( !kText.Length() )
	{
		switch( m_iChatMode )
		{
			// 귓속말 상태에서 빈칸이면 일반 대화로 변경
		case CT_WHISPER_BYGUID:
		case CT_WHISPER_BYNAME:
			{
				std::wstring	kContants(L"");
				m_kWhisperTarget = L"";
				
				m_iChatMode = CT_NORMAL;
				if( m_kTextColorList.empty() )
				{
					m_iFontColor = PgChatMgrUtil::ChatInputColor(static_cast<EChatType>(m_iChatMode));
				}
				else
				{
					ContTextColorList::iterator it = m_kTextColorList.begin();
					while( it != m_kTextColorList.end() )
					{
						if( (*it).eChatType == m_iChatMode )
						{
							break;
						}
						++it;
					}
					if( it == m_kTextColorList.end() )
					{
						m_iFontColor = PgChatMgrUtil::ChatInputColor(static_cast<EChatType>(m_iChatMode));
					}
					else
					{
						m_iFontColor = (*it).TextColor;
					}
				}

				SetChatControl(CT_NORMAL, m_kWhisperTarget, kContants);
			} break;
		default:
			{	
				std::wstring	kContants(L"");
				
				SetChatControl((EChatType)m_iChatMode, m_kWhisperTarget, kContants);
				
				m_bToggleConsecutiveChat = false;
			} break;
		}
		return true;
	}

	switch( m_iChatMode )
	{
	case CT_TRADE:
		{
			// 거래 채팅은 30초에 한번만 가능.
			float CurTime = g_pkApp->GetAccumTime();
			if( CurTime > m_TradeModeChatTime )
			{
				m_TradeModeChatTime = g_pkApp->GetAccumTime() + 30;
			}
			else
			{
				// 에러 메세지(30초에 한번만 가능합니다).
				lwAddWarnDataTT(799393);
				return true;
			}				
		}break;
	}
	
	if(bCheckSpamChat)
	{// 도배 필터링 하지 않는 경우는 로그에 기억하지 않는다
		InputChatLog_Add(kText.GetNormalString());
	}
	if( CheckSysCommand( kText.GetOriginalString() ))
	{
		return true;
	}

	if( CheckChatCommand( kText.GetOriginalString() ) )
	{
		return false;
	}

	//	커맨드
	if( Command_UserFunction(kText.GetOriginalString()) )
	{
		return true;
	}

	//	채금
	if( !SendChat_MuteCheck() )
	{
		return true;
	}

	if(bCheckSpamChat
		&& CheckSpamChat(kText.GetNormalString()))
	{// 도배체크를 사용하고 도배이면
		return true;
	}

	return SendChat_Message(m_iChatMode, kText, m_kWhisperTarget);
}

bool PgChatMgrClient::SendChat_Message(int const iChatType, XUI::CXUI_Style_String const& kText, std::wstring const& kName)
{
	if( g_pkApp->IsSingleMode() )
	{
		AddLogMessage( SChatLog(CT_NORMAL),kText.GetOriginalString());
		return true;
	}

	BM::CAutoMutex kLock(m_kMutex);

	std::wstring OrginalString = kText.GetOriginalString();
	XUI::PgExtraDataPackInfo const &kExtraDataPackInfo = kText.GetExtraDataPackInfo();

	// 채팅 텍스트에 '{', 'T', '='가 사용된 부분이 있으면 채팅 태그 사용으로 간주함.
	bool const UseChatTag = CheckChatTag(OrginalString);
	if( UseChatTag )
	{
		bool IsItemExtraData = IsItemLinkContainData(kExtraDataPackInfo);
		if (false==IsItemExtraData)
		{
			SChatLog kChatLog((EChatType)CT_EVENT);
			g_kChatMgrClient.AddMessage(799394, kChatLog, true, 11);
			return false;
		}
	}

	BYTE byChatType = static_cast<BYTE>(iChatType);

	BM::Stream	kPacket( PT_C_M_REQ_CHAT );

	size_t const iWrPos = kPacket.WrPos();
	kPacket.Push( byChatType );//1

	//	# 계열 커멘드(#1,#2,#R...) 를 파싱하여 XUI의 StyleString 에 맞게 치환한다.
	std::wstring kContents = kText.GetOriginalString();
	//커맨드가 포함된 문장에 대한 필터링 추가
	TransEmoticonFilter(kContents);
	if( !g_kUnicodeFilter.IsCorrect(UFFC_CHAT, kContents) )
	{// 사용 가능한 유니코드 범주 체크
		SChatLog kChatLog(CT_EVENT);
		g_kChatMgrClient.AddMessage(700407, kChatLog, true);
		return true;
	}

	Command_Emotion(kContents);//xxx 임시 이모션 검사
	Command_Filter(kContents);

	kPacket.Push(kContents);//2
	if( CT_MYHOME_PUBLIC == byChatType || CT_MYHOME_PRIVATE == byChatType )
	{
		kPacket.Push(lwHomeRenew::GetMyhomeChatFontColor());//3
	}
	else
	{
		kPacket.Push(m_iFontColor);//3
	}

	kExtraDataPackInfo.PushToPacket(kPacket);//4

	//	채팅 모드에 따른 검사.
	switch( byChatType )
	{
	case CT_WHISPER_BYGUID:
	case CT_WHISPER_BYNAME:
		{
			if( kName.empty() )	// 보낼 사람 이름을 썻는가
			{
				SChatLog kChatLog(CT_EVENT_SYSTEM);
				AddMessage(EM_Whisper_InputError, kChatLog, true);
				return true;
			}
			else
			{
				BM::GUID kRecvGuid;
				if( Name2Guid_Find_ByName(kName, kRecvGuid) )
				{
					if( kRecvGuid != g_kPilotMan.GetPlayerUnit()->GetID() )
					{
						if ( CT_WHISPER_BYGUID != byChatType )
						{
							byChatType = CT_WHISPER_BYGUID;
							kPacket.ModifyData( iWrPos, &byChatType, sizeof(byChatType) );
						}
						kPacket.Push(kRecvGuid);
					}
					else
					{
						SChatLog kChatLog(CT_EVENT_SYSTEM);
						AddMessage(799340, kChatLog, true);
						return true;
					}
				}
				else
				{
					if ( CT_WHISPER_BYNAME != byChatType )
					{
						if( 0 != kName.compare(g_kPilotMan.GetPlayerUnit()->Name()) )
						{
							byChatType = CT_WHISPER_BYNAME;
							kPacket.ModifyData( iWrPos, &byChatType, sizeof(byChatType) );
						}
						else
						{
							SChatLog kChatLog(CT_EVENT_SYSTEM);
							AddMessage(799340, kChatLog, true);
							return true;
						}
					}
					kPacket.Push(kName);
				}
			}

			SendChat_InputNow(false);
		}break;
	case CT_PARTY:
		{
			if( !PgClientPartyUtil::IsInParty() )	// 파티에 속해 있는가
			{
				SChatLog kChatLog(CT_EVENT_SYSTEM);
				AddMessage(22002, kChatLog, true);
				return true;
			}
		}break;
	case CT_GUILD:
		{
			if( !g_kGuildMgr.IamHaveGuild() )	// 길드에 속해 있는가
			{
				SChatLog kChatLog(CT_EVENT_SYSTEM);
				AddMessage(700092, kChatLog, true);
				return true;
			}

			SendChat_InputNow(false);
		}break;
	case CT_TEAM:
	case CT_NORMAL:
	case CT_EMOTION:
	case CT_TRADE:
		{
		}break;
	case CT_FRIEND:
		{
			SendChat_InputNow(false);
		}break;
	case CT_MYHOME_PUBLIC:
	case CT_MYHOME_PRIVATE:
		{
		}break;
	default:
		{
			//	여기는 유저가 쓸 수 없다.
			PG_ASSERT_LOG( 0 && _T(__FUNCTION__) && _T("Not Support Chat Mode") );
			return false;
		}break;
	}

	NETWORK_SEND(kPacket)
	return bSendRet;//패킷전송
}

void PgChatMgrClient::SetChatControl(EChatType const Type, std::wstring const& kName, std::wstring  const& Contants)
{
	BM::CAutoMutex kLock(m_kMutex);

	int const MAX_EDIT_SIZE = kChatLoc[ChatStation()].iWidth  - 208;
	int const EDIT_BASE_LOC = 123;

	std::wstring	kHead;
	DWORD const		Color = PgChatMgrUtil::ChatInputColor(Type);

	PgChatMgrUtil::HeaderName(Type, kHead, CheckChatOut());

	if(Type == CT_WHISPER_BYNAME)
	{
		wchar_t	szName[30] = {0,};
		if( !CheckChatOut() )
		{
			swprintf(szName, 29, kHead.c_str(), kName.c_str());
			kHead = szName;
		}
		else
		{
			kHead += std::wstring(L" ") + kName;
		}
	}
	//kHead += _T(":");

	if( CheckChatOut() )
	{
		kHead = std::wstring(_T("/")) + kHead + _T(" ");
		g_kChatWindow.SetChatControl(Type, kHead, Color);
		return;
	}

	XUI::CXUI_Wnd* pParent = XUIMgr.Get(_T("ChatBar"));
	if( !pParent )
	{
		return; 
	}

	XUI::CXUI_Wnd* pHead = pParent->GetControl(_T("FRM_HEAD"));
	if( !pHead )
	{
		return; 
	}

	XUI::CXUI_Edit* pEdit = dynamic_cast<XUI::CXUI_Edit*>(pParent->GetControl(_T("EDT_CHAT")));
	if( !pEdit )
	{
		return; 
	}

	XUI::CXUI_Wnd* pTypeBtn = pParent->GetControl(_T("BTN_TELL_TYPE"));
	if( pTypeBtn )
	{
		std::wstring	kTemp;
		if(Type == CT_WHISPER_BYNAME)
		{
			PgChatMgrUtil::HeaderName(CT_WHISPER_BYGUID, kTemp);
		}
		else
		{
			PgChatMgrUtil::HeaderName(Type, kTemp);
		}
		pTypeBtn->Text(kTemp);
	}

	switch(Type)
	{
	case CT_WHISPER_BYNAME:
		{
			kHead += _T(":");

			pHead->Size(POINT2(MAX_EDIT_SIZE, pHead->Size().y));
			pHead->Visible(true);
			pHead->Text(kHead);
			pHead->FontColor(Color);

			XUI::CXUI_Style_String kStyleString = pHead->StyleText();
			POINT2 const kTextSize(Pg2DString::CalculateOnlySize(kStyleString));
			pHead->Size(POINT2(kTextSize.x, pHead->Size().y));

			pEdit->Location(POINT2(EDIT_BASE_LOC + kTextSize.x, pEdit->Location().y));
			pEdit->Size(POINT2(MAX_EDIT_SIZE - kTextSize.x, pEdit->Size().y));
		}break;
	default:
		{
			pHead->Visible(false);
			pEdit->Location(POINT2(EDIT_BASE_LOC, pEdit->Location().y));
			pEdit->Size(POINT2(MAX_EDIT_SIZE, pEdit->Size().y));
		}break;
	}
	pEdit->EditText(Contants);
	pEdit->SetEditFocus(true);
}

bool PgChatMgrClient::SendChat_MuteCheck()
{
	BM::CAutoMutex kLock(m_kMutex);

	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( pkPlayer )
	{
		CEffect *pkEffect = pkPlayer->GetEffect(EFFECTNO_MUTECHAT);
		if( pkEffect )	//채금되었으면
		{
			__int64 const i64NowTime = g_kEventView.GetLocalSecTime(CGameTime::SECOND);
			__int64 i64EndTime = 0i64;
			CGameTime::DBTimeEx2SecTime( pkEffect->ExpireTime(), i64EndTime, CGameTime::SECOND );

			__int64 const i64GapTime = std::max( i64EndTime - i64NowTime, 1i64 );
		
			wchar_t wstrTime[50] = L"***";
			if ( (CGameTime::OneYear / CGameTime::SECOND) >= i64GapTime )
			{
				int iSec = static_cast<int>(i64GapTime % 60i64);
				int iMin = static_cast<int>(i64GapTime / 60i64);
				int iHour = iMin / 60;
				iMin %= 60;

				swprintf(wstrTime, 50, L"%d:%02d:%02d",iHour, iMin, iSec);
			}

			std::wstring kMessage;
				
			SChatLog kChatLog(CT_EVENT_SYSTEM);
			FormatTTW(kMessage, 700122, wstrTime);
			g_kChatMgrClient.AddLogMessage(kChatLog, kMessage, true, 1);
			return false;
		}

		return true;
	}
	return false;
}

void PgChatMgrClient::AddDelayLog(SChatLog const& rkLog)
{
	BM::CAutoMutex kLock(m_kMutex);

	if( !rkLog.IsDelayLog() )
	{
		return ;
	}

	ContDelayLogMap::iterator find_iter = m_kDelayLog.find(rkLog.DelayCause());
	if( m_kDelayLog.end() == find_iter )
	{
		std::pair<ContDelayLogMap::iterator, bool> kRet = m_kDelayLog.insert( std::make_pair(rkLog.DelayCause(), ContDelayLogMap::mapped_type()) );
		find_iter = kRet.first;
	}

	if( m_kDelayLog.end() != find_iter )
	{
		ContDelayLogMap::mapped_type& rkDelayLog = (*find_iter).second;
		std::back_inserter(rkDelayLog) = rkLog;
	}
}

void PgChatMgrClient::FlushDelayLog(EItemModifyParentEventType const eCause)
{
	BM::CAutoMutex kLock(m_kMutex);
	ContDelayLogMap::iterator find_iter = m_kDelayLog.find(eCause);
	if( m_kDelayLog.end() == find_iter )
	{
		return ;
	}

	ContDelayLogMap::mapped_type const& rkVec = (*find_iter).second;
	ContDelayLogMap::mapped_type::const_iterator log_iter = rkVec.begin();
	while( rkVec.end() != log_iter )
	{
		ContDelayLog::value_type const& rkDelayLog = (*log_iter);
		RecvChatLog_Add(rkDelayLog);
		++log_iter;
	}
	m_kDelayLog.erase(find_iter);
}

void PgChatMgrClient::SetMaxBlockTime(float const fMaxBlockTime)
{
	m_fMaxBlockTime = fMaxBlockTime;
}

void PgChatMgrClient::ClearBlockTime()
{// 채팅 금지 시간 초기화
	m_fBlockTime = m_fStartBlockTime = 0;
}
void PgChatMgrClient::IncBlockChatTime(float const fIncTime)
{
	m_fBlockTime+=fIncTime;
	if(m_fBlockTime > m_fMaxBlockTime)
	{
		m_fBlockTime = m_fMaxBlockTime;
	}
}

void PgChatMgrClient::ChatAram(const std::wstring & strAramSound, const bool bShowNow)
{
	const UINT64 iNewTime = BM::GetTime64();
	//10초 보다 많이 지났고, 게임 시작이 아니라면
	if( 10000 <= (iNewTime - m_iOldChatTime) || bShowNow)
	{
		PgPlayer *pkPlayer = g_kPilotMan.GetPlayerUnit();
		if(!pkPlayer)
		{
			return;
		}

		PgPilot	*pkPilot = g_kPilotMan.FindPilot(pkPlayer->GetID());
		if(pkPilot)
		{
			PgActor *pkActor = dynamic_cast<PgActor *>(pkPilot->GetWorldObject());
			if(!pkActor)
			{
				return;	
			}
			//채팅창 활성화 말풍선 메세지 출력
			pkActor->ShowChatBalloon(CT_NORMAL, TTW(799250), 10000, true);

			//사운드 추가
			g_kUISound.PlaySoundByID( strAramSound );

			//'안녕' 이모션 추가
			PgEmotiMgr::tagEmotion const &kEmotion = g_kPgEmotiMgr.GetEmotion(1);
			if( CheckCanEmotion(1) && !kEmotion.IsEmpty() )
			{
				if( !kEmotion.ParamCont().empty() )
				{
					PgEmotiMgr::tagEmotion::CONT_PARAM::const_iterator iter = kEmotion.ParamCont().begin();
					while( iter != kEmotion.ParamCont().end() )
					{
						pkActor->SetParam(MB(iter->second.KEY), MB(iter->second.VAL));
						++iter;
					}
				}
				pkActor->ReserveTransitAction(MB(kEmotion.ActionID()));
			}
		}
		m_iOldChatTime = BM::GetTime64();
	}
}

void PgChatMgrClient::SetChatOutDefaultPos(POINT2 const& rkPos)
{
	if(ECS_COMMON == ChatStation())
	{
		SetSavedPos(rkPos);
	}
}

void PgChatMgrClient::SetSysChatOutDefaultPos(POINT2 const& rkPos)
{
	if( ECS_COMMON == ChatStation() )
	{
		SetSystemSavedPos(rkPos);
	}
}

void PgChatMgrClient::SetSavedPos(POINT2 const& rkPos)
{
	m_kSavedPos.Set(rkPos.x, rkPos.y);
}

void PgChatMgrClient::SetSystemSavedPos(POINT2 const& rkPos)
{
	m_kSystemSavedPos.Set(rkPos.x, rkPos.y);
}

POINT2 const& PgChatMgrClient::GetChatOutDefaultPos() const
{
	return m_kSavedPos;
}

POINT2 const& PgChatMgrClient::GetSysChatOutDefaultPos() const
{
	return m_kSystemSavedPos;
}

void PgChatMgrClient::ApplyChatOutSavedPos(XUI::CXUI_Wnd* pkWnd)
{
	if(NULL==pkWnd)
	{
		return;
	}
	if(ECS_COMMON == ChatStation())
	{
		POINT2 const& rkSavedPos = GetChatOutDefaultPos();
		if(-1<rkSavedPos.x && -1<rkSavedPos.y)
		{
			pkWnd->Location(rkSavedPos);
		}
	}
	pkWnd->CanDrag(ECS_COMMON == ChatStation());
}

void PgChatMgrClient::ApplySystemChatOutSavedPos(XUI::CXUI_Wnd* pkWnd)
{
	if( NULL == pkWnd )
	{
		return;
	}

	if( ECS_COMMON == ChatStation() )
	{
		POINT2 const& rkSavedPos = GetSysChatOutDefaultPos();
		if( -1 < rkSavedPos.x && -1 < rkSavedPos.y )
		{
			pkWnd->Location(rkSavedPos);
			CheckSnapPos(rkSavedPos);
		}
	}
	pkWnd->CanDrag(ECS_COMMON == ChatStation());
}


void PgChatMgrClient::ChatStation(EChatStation eStation)
{
	if(m_kChatStation!=m_kPastChatStation)
	{
		m_kPastChatStation = m_kChatStation;
	}

	m_kChatStation = eStation;
}

EChatStation PgChatMgrClient::ChatStation() const
{
	return m_kChatStation;
}

EChatStation PgChatMgrClient::PastChatStation() const
{
	return m_kPastChatStation;
}

bool PgChatMgrClient::TogglingConsecutiveChat(void)
{
	return (m_bToggleConsecutiveChat = (!m_bToggleConsecutiveChat));
}

void PgChatMgrClient::ToggleConsecutiveChatUI(void)
{
	CXUI_Wnd* pkWnd = XUIMgr.Get(L"ChatBar");
	if( pkWnd )
	{
		CXUI_Wnd* pkStroke =  pkWnd->GetControl(L"FRM_TOGGLECHAT");
		if(pkStroke)
		{
			pkStroke->Visible(m_bToggleConsecutiveChat);
		}	
	}
}

void PgChatMgrClient::ShowNotifyConnectInfo(ENotifyConnectInfo const NfyInfo, std::wstring const& kName, int nClassNo, SUVInfo& kUVInfo, BM::GUID const& rkGuid)
{
	BM::CAutoMutex kLock(m_kMutex);

	ContNotifyConnectInfoList::iterator it = m_kNotifyList.begin();
	while( it != m_kNotifyList.end() )
	{
		if( 0 == kName.compare((*it).kName) )
		{
			return;
		}

		++it;
	}

	std::wstring kHeader(_T(""));
	std::wstring kText(_T(""));

	switch(NfyInfo)
	{
	case NCI_FRIEND:
		{
			kHeader = _T("{C=0xFF88FF47/}") + TTW(400104);
		}break;
	case NCI_GUILD:
		{
			kHeader = _T("{C=0xFFFD90FE/}") + TTW(400106);
		}break;
	case NCI_COUPLE:
		{
			kHeader = _T("{C=0xFFFF8E49/}") + TTW(700519);
		}break;
	default:
		{
		}break;
	}

	kText += kHeader + _T(" : ");
	kText += _T("{O=U/}") + kName + _T("{O=0/}") + TTW(799333);
	SNotifyConnectInfo kNfyInfo(NfyInfo, kText, kName, nClassNo, kUVInfo, rkGuid);

	m_kNotifyList.push_back(kNfyInfo);

	ShowNotifyConnectInfoUI();
}

BM::GUID& PgChatMgrClient::GetNotifyGuid(void)
{
	ContNotifyConnectInfoList::iterator it = m_kNotifyList.begin();
	return (*it).kGuid;
}

void PgChatMgrClient::PopNotifyList(void)
{
	if( m_kNotifyList.empty() )
	{
		return;
	}

	m_kNotifyList.pop_front();
}

void PgChatMgrClient::ShowNotifyConnectInfoUI(void)
{
	BM::CAutoMutex kLock(m_kMutex);

	if( m_kNotifyList.empty() )
	{
		return;
	}

	XUI::CXUI_Wnd* pkNfyWnd = XUIMgr.Get(_T("FRM_NFY_CONN"));
	if( pkNfyWnd )
	{
		return;
	}

	ContNotifyConnectInfoList::iterator it = m_kNotifyList.begin();

	pkNfyWnd = XUIMgr.Call(_T("FRM_NFY_CONN"));
	if( !pkNfyWnd )
	{
		return;
	}

	XUI::CXUI_Wnd* pkGuildMarkWnd = pkNfyWnd->GetControl(_T("IMG_GUILD_MARK"));
	XUI::CXUI_Wnd* pkClassMarkWnd = pkNfyWnd->GetControl(_T("IMG_CLASS"));
	XUI::CXUI_Wnd* pkCoupleWnd = pkNfyWnd->GetControl(_T("IMG_COUPLE"));
	if( !pkGuildMarkWnd || !pkClassMarkWnd || !pkCoupleWnd )
	{
		return;
	}

	switch( (*it).eNotifyInfo )
	{
	case NCI_FRIEND:
		{
			pkGuildMarkWnd->Visible(false);
			pkClassMarkWnd->Visible(true);
			pkCoupleWnd->Visible(false);
		}break;
	case NCI_COUPLE:
		{
			pkGuildMarkWnd->Visible(false);
			pkClassMarkWnd->Visible(false);
			pkCoupleWnd->Visible(true);
		}break;
	case NCI_GUILD:
		{
			pkGuildMarkWnd->Visible(true);
			pkClassMarkWnd->Visible(false);
			pkCoupleWnd->Visible(false);
		}break;
	default:
		{
			pkGuildMarkWnd->Visible(false);
			pkClassMarkWnd->Visible(false);
			pkCoupleWnd->Visible(false);
		}break;
	}

	lwSetMiniClassIconIndex( pkClassMarkWnd, (*it).nClassNo );

	(*it).kUVInfo.U = pkGuildMarkWnd->UVInfo().U;
	(*it).kUVInfo.V = pkGuildMarkWnd->UVInfo().V;
	pkGuildMarkWnd->UVInfo( (*it).kUVInfo );

	XUI::CXUI_Wnd* pChild = pkNfyWnd->GetControl(_T("FRM_ID_RECT"));
	if( pChild)
	{
		pChild->SetCustomData((*it).kName.c_str(), sizeof(std::wstring::value_type) * (*it).kName.size());
		pChild->Text((*it).kName.c_str());
		XUI::CXUI_Style_String kStyleString = pChild->StyleText();
		POINT2 const kTextSize(Pg2DString::CalculateOnlySize(kStyleString));
		pChild->Size(kTextSize);
		pChild->Text(_T(""));
		pChild->SetInvalidate();
	}

	pkNfyWnd->Text( (*it).kOutString );
}

void PgChatMgrClient::CheckSnapPos(POINT2 const& rkPos)
{
	XUI::CXUI_Wnd* pSysChatOut = XUIMgr.Activate(_T("SysChatOut"));
	XUI::CXUI_Wnd* pChatOut = XUIMgr.Activate(_T("ChatOut"));
	if( !pSysChatOut || ! pChatOut )
	{
		return;
	}

	POINT3I const& rkSysChatOutLoc = pSysChatOut->Location();
	POINT3I const& rkChatOutLoc = pChatOut->Location();
	int nHeight = pSysChatOut->Height();

	if( (rkSysChatOutLoc.x < rkChatOutLoc.x + 50) 
		&& (rkSysChatOutLoc.x > rkChatOutLoc.x - 50)
		&& (rkSysChatOutLoc.y < rkChatOutLoc.y - nHeight + 50)
		&& (rkSysChatOutLoc.y) > rkChatOutLoc.y - nHeight - 50 )
	{
		pSysChatOut->Location(POINT2(pChatOut->Location().x, pChatOut->Location().y - pSysChatOut->Size().y - 5));
		SysChatOutSnap(true);
	}
}

void PgChatMgrClient::UseFontColor(unsigned int iColor)
{
	ContTextColorList::iterator it = m_kTextColorList.begin();
	while ( it != m_kTextColorList.end() )
	{
		if( (*it).eChatType == m_iChatMode )
		{
			(*it).TextColor = iColor;
			m_iFontColor = iColor;
			return;
		}
		++it;
	}

	STextColorMap kTextColor(m_iChatMode, iColor);
	m_kTextColorList.push_back(kTextColor);
	m_iFontColor = iColor;
}

// 유저가 채팅 Tag를 사용했는지 검사.
// 채팅 Tag를 사용했다면 보내지말고 그냥 무시해야 됨.
bool PgChatMgrClient::CheckChatTag(std::wstring const & Text)
{
	// '{'로 시작하는 부분이 있는지 검사.
	std::wstring::size_type TagStartPos = Text.find(_T("{"));
	if( TagStartPos == std::wstring::npos )
	{	// '{' 로 시작하는 부분이 없음(채팅태그 사용하지 않음).
		return false;
	}

	// Font나 Font-Size 변경하려는 경우만 막는다. { T =
	std::wstring::size_type TagTextChangePos = Text.find(_T("T"));
	if( TagTextChangePos == std::wstring::npos )
	{	// 'T'를 포함하지 않는다. (크기변환)
		TagTextChangePos = Text.find(_T("C"));
		if( TagTextChangePos == std::wstring::npos )
		{	// 'C'를 포함하지 않는다. (컬러변환)
			return false;
		}
	}

	// '{'가 포함되어 있다면 '='가 있는지 다시 검사.
	std::wstring::size_type TagEndPos = Text.find(_T("="));
	if( TagEndPos == std::wstring::npos )
	{	// '{'는 있는데 '='가 없으면 채팅 태그를 사용하지 않는 것으로 간주.
		return false;
	}

	if( (TagStartPos < TagTextChangePos) && (TagTextChangePos < TagEndPos) )
	{	// 채팅 텍스트에 '{', 'T', '='가 사용된 부분이 있으면 채팅 태그 사용으로 간주함.
		return true;
	}
	
	//T 바로 다음에 '=' 붙었을 경우엔 폰트 크기 변경이다. 'T' 와 '=' 사이의 공백은 무시해야 한다.
	std::wstring::size_type Tag_T_Pos = (TagTextChangePos+1);
	while( Tag_T_Pos != std::wstring::npos )
	{
		wchar_t const szWord = Text.at(Tag_T_Pos);
		if( szWord == L' ' )
		{
			++Tag_T_Pos;
			continue;
		}
		if( szWord == L'=' )
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	return false;
}

void PgChatMgrClient::Regist_ChatBlockUser(std::wstring & Name)
{
	BM::CAutoMutex Lock(m_kMutex);

	PgPlayer * pPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pPlayer )
	{
		return;
	}

	if( Name.empty() )
	{
		return;
	}

	if( Name == pPlayer->Name() )
	{	// 자기 자신은 추가할 수 없다.
		BM::vstring vText(TTW(404027));
		SChatLog ChatLog(CT_EVENT_SYSTEM);
		AddLogMessage(ChatLog, vText, true, 1);
		return;
	}

	if( PgClientPartyUtil::IsInParty() )
	{	// 파티 멤버는 차단 할 수 없다.
		SPartyMember const * pMember = NULL;
		if( g_kParty.GetMember(Name, pMember) )
		{	// 파티 멤버에 포함된 캐릭터다.
			BM::vstring vText(TTW(404028));
			SChatLog ChatLog(CT_EVENT_SYSTEM);
			AddLogMessage(ChatLog, vText, true, 1);
			return;
		}
	}

	if( m_MyChatBlockList.size() >= MAX_BLOCKCHAT_LIST )
	{	// 더이상 추가할 수 없다.
		BM::vstring vText(TTW(404026));
		SChatLog ChatLog(CT_EVENT_SYSTEM);
		AddLogMessage(ChatLog, vText, true, 1);
		return;
	}
	
	// 처음 추가 할 때는 기본으로 모든 채팅을 차단.
	BYTE BlockType = ECBT_BOTH;

	// 차단 리스트에 추가.
	bool bDuplicate = CheckChatBlockList(Name);
	if( bDuplicate )
	{	// 이미 차단 된 캐릭터.
		BM::vstring vText(TTW(404025));
		vText.Replace(L"#Name#", Name);
		SChatLog ChatLog(CT_EVENT_SYSTEM);
		AddLogMessage(ChatLog, vText, true, 1);
		return;
	}

	// 서버로 차단 정보 패킷 보냄.
	BM::Stream Packet(PT_C_M_REQ_REGIST_CHAT_BLOCK);
	Packet.Push(Name);
	Packet.Push(BlockType);

	NETWORK_SEND(Packet);
}

void PgChatMgrClient::UnRegist_ChatBlockUser(std::wstring & Name)
{
	BM::CAutoMutex Lock(m_kMutex);
	
	// 차단 리스트에 있는지 확인.
	bool bExist = CheckChatBlockList(Name);
	if( !bExist )
	{	// 목록에 없는데 삭제 시도.
		BM::vstring vText(TTW(404038));
		SChatLog ChatLog(CT_EVENT_SYSTEM);
		AddLogMessage(ChatLog, vText, true, 1);
		return;
	}

	RemoveChatBlockList(Name);

	// 서버로 차단 정보 패킷 보냄.
	BM::Stream Packet(PT_C_M_REQ_UNREGIST_CHAT_BLOCK);
	Packet.Push(Name);

	NETWORK_SEND(Packet);
}

void PgChatMgrClient::Modify_ChatBlockMode(std::wstring & Name, BYTE BlockMode)
{
	BM::CAutoMutex Lock(m_kMutex);
	
	BYTE SendBlockType = 0;
	// 차단 리스트에 있는지 확인.
	bool bExist = ModifyChatBlockList(Name, BlockMode, SendBlockType);
	if( !bExist )
	{	// 목록에 없는데 변경 시도.
		// 목록에 없는데 삭제 시도.
		BM::vstring vText(TTW(404038));
		SChatLog ChatLog(CT_EVENT_SYSTEM);
		AddLogMessage(ChatLog, vText, true, 1);
		return;
	}

	// 서버로 차단 옵션 변경 패킷 보냄.
	BM::Stream Packet(PT_C_M_REQ_CHANGE_CHAT_BLOCK_OPTION);
	Packet.Push(Name);
	Packet.Push(SendBlockType);

	NETWORK_SEND(Packet);
}

void PgChatMgrClient::UpdateChatBlockList(XUI::CXUI_Wnd * pWnd)
{
	BM::CAutoMutex Lock(m_kMutex);

	if( NULL == pWnd )
	{
		return;
	}

	XUI::CXUI_List * pList = dynamic_cast<XUI::CXUI_List*>(pWnd->GetControl(L"LST_BLOCK_LIST"));
	if( NULL == pList )
	{
		return;
	}

	pList->ClearList();
	
	int ListNum = 0;
	CONT_BLOCKCHAT_LIST::const_iterator list_iter = m_MyChatBlockList.begin();
	for( ; list_iter != m_MyChatBlockList.end() ; ++list_iter)
	{
		XUI::SListItem * pItem = pList->AddItem(L"");	// 새로운 항목 추가.
		if( pItem )
		{
			XUI::CXUI_Wnd * pItemWnd = pItem->m_pWnd;
			if( pItemWnd )
			{
				pItemWnd->SetCustomData((list_iter->first).c_str(), sizeof(std::wstring::value_type) * (list_iter->first.size()));
				++ListNum;

				XUI::CXUI_Form * pName = dynamic_cast<XUI::CXUI_Form *>(pItemWnd->GetControl(L"FRM_BLOCK_NAME"));
				if( pName )
				{
					std::wstring CharName = list_iter->first;
					pName->Text(CharName);
				}

				XUI::CXUI_CheckButton * pNormalBtn = dynamic_cast<XUI::CXUI_CheckButton *>(pItemWnd->GetControl(L"CHK_BLOCK_NORMAL"));
				if( pNormalBtn )
				{
					pNormalBtn->Check(false);
					if( (list_iter->second & ECBT_NORMAL) )
					{
						pNormalBtn->Check(true);
					}
				}

				XUI::CXUI_CheckButton * pWorldBtn = dynamic_cast<XUI::CXUI_CheckButton *>(pItemWnd->GetControl(L"CHK_BLOCK_WORLD"));
				if( pWorldBtn )
				{
					pWorldBtn->Check(false);
					if( (list_iter->second & ECBT_CASHCHAT) )
					{
						pWorldBtn->Check(true);
					}
				}
			}
		}
	}
}

void PgChatMgrClient::SetChatBlockList(CONT_BLOCKCHAT_LIST & BlockList)
{
	BM::CAutoMutex Lock(m_kMutex);

	m_MyChatBlockList.swap(BlockList);
}

void PgChatMgrClient::GetChatBlockList(CONT_BLOCKCHAT_LIST & BlockList)
{
	BM::CAutoMutex Lock(m_kMutex);

	BlockList.insert(m_MyChatBlockList.begin(), m_MyChatBlockList.end());
}

bool PgChatMgrClient::CheckChatBlockCharacter(std::wstring & CharName, EChatBlockType BlockType)
{
	CONT_BLOCKCHAT_LIST::iterator find_iter = m_MyChatBlockList.find(CharName);
	if( find_iter != m_MyChatBlockList.end() )
	{
		if( find_iter->second & BlockType )
		{
			return true;
		}
	}

	return false;
}

bool PgChatMgrClient::CheckChatBlockList(std::wstring & CharName)
{	// 차단 리스트에 등록 되어 있는지 확인.
	CONT_BLOCKCHAT_LIST::iterator find_iter = m_MyChatBlockList.find( CharName );
	if( find_iter != m_MyChatBlockList.end() )
	{	// 찾았으면 true.
		return true;
	}
	// 없으면 false.
	return false;
}

bool PgChatMgrClient::AddChatBlockList(std::wstring & CharName, BYTE BlockType)
{
	std::pair<CONT_BLOCKCHAT_LIST::iterator, bool> ret = m_MyChatBlockList.insert( std::make_pair(CharName, BlockType) );
	
	return ret.second;
}

void PgChatMgrClient::RemoveChatBlockList(std::wstring & CharName)
{
	m_MyChatBlockList.erase(CharName);
}

bool PgChatMgrClient::ModifyChatBlockList(std::wstring & CharName, BYTE BlockType, BYTE & ModifyBlockType)
{	// 채팅 차단 방법 변경.
	CONT_BLOCKCHAT_LIST::iterator find_iter = m_MyChatBlockList.find( CharName );
	if( find_iter != m_MyChatBlockList.end() )
	{	// 찾아서 변경.
		find_iter->second = find_iter->second ^ BlockType;
		ModifyBlockType = find_iter->second;
		return true;
	}

	return false;
}

bool PgChatMgrClient::CheckChattingOption(SChatLog const & ChatLog, PgChatMgrUtil::CChatLogList const & ChatLogList)
{
	if( (ChatStation() == ECS_PVPLOBBY) || (ChatStation() == ECS_CASHSHOP) )
	{	// PvP로비이거나 캐시샵 상태에서는 시스템 창 표시 유무 옵션과, 다른 탭의 옵션에 따라 출력해줄 내용이 다르다.
		if( (ChatLog.ChatType() > CT_USERSELECT_MAX)
			&& (ChatLog.ChatType() != CT_WHISPER_BYGUID)
			&& (ChatLog.ChatType() != CT_TRADE) )
		{	// 시스템 메세지의 경우..
			int SysOutConfigValue = lwConfig_GetValue("SYSTEMOUT_OPTION", "SYSTEMOUT_WINDOW");
			int ChatOutConfigValue = lwConfig_GetValue("CHATMODE_NORMAL", "FILTER_SYSTEM");
			if( 2 == SysOutConfigValue || 2 == ChatOutConfigValue )
			{	// 시스템 메세지 창이 켜져 있거나, 일반채팅창에 시스템메세지 옵션이 켜져있는 경우에만 출력.
				return true;
			}
		}
		else
		{	// 다른 타입의 메세지는 자신의 채팅탭에서 옵션을 확인한 후에 일반 채팅로그로 출력시킨다.
			// 해당 탭의 옵션 중 자신의 옵션만 확인(길드탭이면 길드메세지 옵션이 켜져있는지만 본다).
			int ConfigValue = 0;
			switch( ChatLog.ChatType() )
			{
			case CT_NORMAL:
			case CT_FRIEND:
			case CT_TEAM:
			case CT_WHISPER_BYGUID:
			case CT_WHISPER_BYNAME:
				{
					ConfigValue = lwConfig_GetValue("CHATMODE_NORMAL", "FILTER_NORMAL");
				}break;
			case CT_PARTY:
				{
					ConfigValue = lwConfig_GetValue("CHATMODE_PARTY", "FILTER_PARTY");
				}break;
			case CT_GUILD:
				{
					ConfigValue = lwConfig_GetValue("CHATMODE_GUILD", "FILTER_GUILD");
				}break;
			case CT_TRADE:
				{
					ConfigValue = lwConfig_GetValue("CHATMODE_TRADE", "FILTER_TRADE");
				}break;
			default:
				{
					ConfigValue = 2;
				}break;
			}

			if( 2 == ConfigValue )
			{
				return true;
			}
		}
	}
	else
	{	// 일반 상태나 마이홈에서의 채팅은 기존과 동일함.
		return ChatLogList.CmpFilterFlag(ChatLog.ChatFlag());
	}

	return false;
}

void PgChatMgrClient::LoveModeMessageDelay(std::wstring & Message, int nLevel)
{
	PgChatMgrUtil::ContNoticeLevelColor::iterator iter = PgChatMgrUtil::kContNoticeLevelColor.find(nLevel);
	if( PgChatMgrUtil::kContNoticeLevelColor.end() == iter )
	{
		iter = PgChatMgrUtil::kContNoticeLevelColor.find(0);//Default
	}

	PgChatMgrUtil::ContNoticeLevelColor::mapped_type & NoticeColor = (*iter).second;

	m_kNoticeWaitList.push( SNoticeWaitItem(Message, NoticeColor) );
}

bool PgChatMgrClient::IsItemLinkContainData(XUI::PgExtraDataPackInfo const &rkExtraDataPackInfo) const
{
	size_t const extraDataCount = rkExtraDataPackInfo.GetExtraDataPackCount();
	for (size_t i = 0; i < extraDataCount; ++i)
	{
		PgExtraDataPack const* pInfo = rkExtraDataPackInfo.GetExtraDataPack(i);
		CONT_EXTRA_DATA::const_iterator it = pInfo->m_kExtraDataCont.begin();
		while(pInfo->m_kExtraDataCont.end() != it)
		{
			if((*it).m_kType==LINK_EDT_ITEM)
			{
				return true;
			}
			++it;
		}
	}
	return false;
}