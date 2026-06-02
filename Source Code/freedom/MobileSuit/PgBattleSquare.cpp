#include "stdafx.h"
#include "Variant/PgQuestInfo.h"
#include "Variant/PgStringUtil.h"
#include "Variant/PgBattleSquare.h"
#include "Variant/TableDataManager.h"
#include "PgNetwork.h"
#include "PgBattleSquare.h"

#include "ServerLib.h"
#include "lwUI.h"
#include "lwUIQuest.h"
#include "PgChatMgrClient.h"
#include "PgPilotMan.h"
#include "PgSoundMan.h"
#include "PgOption.h"
#include "PgObserverMode.h"
#include "PgMobileSuit.h"
#include "PgQuestMan.h"
#include "PgMoveAnimateWnd.h"
#include "PgUIScene.h"
#include "PgMinimap.h"
#include "PgUISound.h"

extern bool BreakMoney_WStr(__int64 const iTotal, VEC_WSTRING &rkVec);
namespace BattleSquareUtil
{
	void AddObserver(PgBSTeam const& rkBSTeam, BM::GUID const& rkMyPlayerGuid)
	{
		CONT_BS_TEAM_GUID_MEMBER::const_iterator red_iter = rkBSTeam.ContMember().begin();
		while( rkBSTeam.ContMember().end() != red_iter )
		{
			CONT_BS_TEAM_GUID_MEMBER::key_type const& rkGuid = (*red_iter).first;
			if( rkMyPlayerGuid != rkGuid )
			{
				g_kOBMode.AddTarget( rkGuid );
			}
			++red_iter;
		}
	}
	void DelObserver(PgBSTeam const& rkOldTeam, PgBSTeam const& rkNewTeam)
	{
		CONT_BS_TEAM_GUID_MEMBER::const_iterator red_iter = rkOldTeam.ContMember().begin();
		while( rkOldTeam.ContMember().end() != red_iter )
		{
			CONT_BS_TEAM_GUID_MEMBER::key_type const& rkGuid = (*red_iter).first;
			if( rkNewTeam.ContMember().end() == rkNewTeam.ContMember().find(rkGuid) )
			{
				g_kOBMode.RemoveTarget( rkGuid );
			}
			++red_iter;
		}
	}
	void UpdateNameEquip(PgBSTeam const& rkBSTeam)
	{
		if( g_pkWorld )
		{
			CONT_BS_TEAM_GUID_MEMBER::const_iterator iter = rkBSTeam.ContMember().begin();
			while( rkBSTeam.ContMember().end() != iter )
			{
				CONT_BS_TEAM_GUID_MEMBER::key_type const& rkGuid = (*iter).first;
				PgActor* pkActor = dynamic_cast< PgActor* >( g_pkWorld->FindObject(rkGuid) );
				if( pkActor )
				{
					pkActor->UpdateName();
					pkActor->EquipAllItem();
				}
				++iter;
			}
		}
	}
}

namespace BattleSquareUI
{
	bool GetMemberInfo(BM::GUID const& rkCharGuid, PgBSTeam const& rkRedTeam, PgBSTeam const& rkBlueTeam, SBSTeamMember& rkOut)
	{
		if( rkRedTeam.IsTeamMember(rkCharGuid) )
		{
			return rkRedTeam.GetMember(rkCharGuid, rkOut);
		}
		else if( rkBlueTeam.IsTeamMember(rkCharGuid) )
		{
			return rkBlueTeam.GetMember(rkCharGuid, rkOut);
		}
		return false;
	}
	EBattleSquareTeam GetTeam(BM::GUID const& rkChar, PgBSTeam const& rkRedTeam, PgBSTeam const& rkBlueTeam)
	{
		if( rkRedTeam.IsTeamMember(rkChar) )
		{
			return rkRedTeam.Team();
		}
		if( rkBlueTeam.IsTeamMember(rkChar) )
		{
			return rkBlueTeam.Team();
		}
		return BST_NONE;
	}

	EBattleSquareTeam GetMyTeam(PgBSTeam const& rkRedTeam, PgBSTeam const& rkBlueTeam)
	{
		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( !pkPlayer )
		{
			return BST_NONE;
		}
		return GetTeam(pkPlayer->GetID(), rkRedTeam, rkBlueTeam);
	}
	bool GetMyMemberInfo(PgBSTeam const& rkRedTeam, PgBSTeam const& rkBlueTeam, SBSTeamMember& rkOut)
	{
		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( !pkPlayer )
		{
			return false;
		}
		EBattleSquareTeam const eTeam = GetMyTeam(rkRedTeam, rkBlueTeam);
		switch( eTeam )
		{
		case BST_RED:
			{
				return rkRedTeam.GetMember(pkPlayer->GetID(), rkOut);
			}break;
		case BST_BLUE:
			{
				return rkBlueTeam.GetMember(pkPlayer->GetID(), rkOut);
			}break;
		default:
			{
			}break;
		}
		return false;
	}

	bool InitPenalty( VEC_DEFDEATHPENALTY::const_iterator itor_vec, int const iDeathCount)
	{
		XUI::CXUI_Wnd* pkDeathPenaltyUI = XUIMgr.Get(L"FRM_BS_DEATH_PENALTY");

		BM::vstring strBuff(L"BS_BuffIcon");
		strBuff = strBuff + BM::vstring(itor_vec->bySuccessionalDeath - 1);
		
		bool bGrayScale = ( (int)itor_vec->bySuccessionalDeath != iDeathCount );

		if(pkDeathPenaltyUI)
		{
			XUI::CXUI_Wnd* pkIconWnd = pkDeathPenaltyUI->GetControl(strBuff);
			if(pkIconWnd)
			{
				XUI::CXUI_Icon* pkIcon = dynamic_cast<XUI::CXUI_Icon*>(pkIconWnd);
				if(pkIcon)
				{
					int const iEffect = itor_vec->iEffect[0];
					SIconInfo kIconInfo = pkIcon->IconInfo();
					kIconInfo.iIconKey = iEffect;
					pkIcon->SetIconInfo(kIconInfo);
					pkIcon->GrayScale(bGrayScale);
					pkIcon->SetCustomData( &iEffect, sizeof(iEffect) );
				}
			}
		}
		return bGrayScale;
	}
	bool CheckPenalty( VEC_DEFDEATHPENALTY::const_iterator itor_vec)
	{
		if ( !g_pkWorld )
		{
			return false;
		}
		if( itor_vec->iGroundAttr != 0 && g_pkWorld->GetAttr() != itor_vec->iGroundAttr )
		{
			return false;
		}
		if (itor_vec->iGroundNo != 0 && g_pkWorld->MapNo() != itor_vec->iGroundNo )
		{
			return false;
		}
		return true;
	}
	void UpdateDeathPenalty(int const iEffect)
	{
		XUI::CXUI_Wnd* pkDeathPenaltyUI = XUIMgr.Get(L"FRM_BS_DEATH_PENALTY");
		BM::vstring strBuff(L"BS_BuffIcon");
		if(!pkDeathPenaltyUI)
		{
			return;
		}
		int iBuff = 0;
		XUI::CXUI_Icon* pkIcon = NULL;
		do
		{
			BM::vstring strIcon = strBuff + BM::vstring(iBuff);
			XUI::CXUI_Wnd* pkIconWnd = pkDeathPenaltyUI->GetControl(strIcon);
			if(pkIconWnd)
			{
				pkIcon = dynamic_cast<XUI::CXUI_Icon*>(pkIconWnd);
				if(pkIcon)
				{
					SIconInfo kIconInfo = pkIcon->IconInfo();
					if( iEffect == kIconInfo.iIconKey)
					{
						pkIcon->GrayScale(false);
						return;
					}
				}
			}
			++iBuff;
		}
		while(pkIcon);
	}
	void CallDeathPenalty(int iDeathCount)
	{
		if(!g_pkWorld)
		{
			return;
		}

		XUIMgr.Activate(L"FRM_BS_DEATH_PENALTY");
		int const iCountMax = 8;

		if( iCountMax < iDeathCount)
		{
			iDeathCount = iCountMax;
		}


		CONT_DEFDEATHPENALTY const* pkContDefDeathPenalty;
		g_kTblDataMgr.GetContDef(pkContDefDeathPenalty);
		if( 0 == pkContDefDeathPenalty->size())
		{
			return;
		}

		CONT_DEFDEATHPENALTY::const_iterator itor_death = pkContDefDeathPenalty->find( g_pkWorld->MapNo() );
		if( itor_death == pkContDefDeathPenalty->end() )
		{
			return;
		}

		VEC_DEFDEATHPENALTY const rkVec = (*itor_death).second;
		if( 0 == rkVec.size())
		{
			return;
		}

		int iSecondEffect = 0;
		VEC_DEFDEATHPENALTY::const_iterator itor_vec = rkVec.begin();
		while (rkVec.end() != itor_vec)
		{
			if ( true == CheckPenalty(itor_vec))
			{
				if(false == InitPenalty(itor_vec, iDeathCount) )
				{
					iSecondEffect = itor_vec->iEffect[1];
				}
			}
			++itor_vec;
		}
		if( 0 != iSecondEffect )
		{
			UpdateDeathPenalty(iSecondEffect);
		}
	}

	std::wstring const kTeamSelectUI(L"FRM_BS_PLAYER_LIST");
	std::wstring const kTeamStateUI(L"FRM_BS_PRESENT_LIST");
	class PgBSTeamUI
	{
	public:
		PgBSTeamUI(PgBSTeam const& rkRedTeam, PgBSTeam const& rkBlueTeam, PgBSGame const& rkBSGame, bool const bHaveFreePVPAttr)
			: m_kRedTeam(rkRedTeam), m_kBlueTeam(rkBlueTeam), m_kBSGame(rkBSGame), m_bHaveFreePVPAttr(bHaveFreePVPAttr)
		{
		}
		~PgBSTeamUI()
		{
		}

		void Close()
		{
			XUIMgr.Close(kTeamSelectUI);
			XUIMgr.Close(kTeamStateUI);
		}
		bool IsUIOn() const
		{
			EBattleSquareTeam const eTeam = GetMyTeam(m_kRedTeam, m_kBlueTeam);
			bool bJoinTeam = (BST_NONE == eTeam);
			if( bJoinTeam )
			{
				return NULL != XUIMgr.Get(kTeamSelectUI);
			}
			else
			{
				return NULL != XUIMgr.Get(kTeamStateUI);
			}
		}
		void Call()
		{
			EBattleSquareTeam const eTeam = GetMyTeam(m_kRedTeam, m_kBlueTeam);
			bool bJoinTeam = (BST_NONE == eTeam);
			if( bJoinTeam )
			{
				XUI::CXUI_Wnd* pkTop = XUIMgr.Get(kTeamSelectUI);
				if( !pkTop )
				{
					pkTop = XUIMgr.Call(kTeamSelectUI);
					if( !pkTop )
					{
						return;
					}
				}
				UpdateTeamSelectUI(pkTop);
			}
			else
			{
				XUI::CXUI_Wnd* pkTop = XUIMgr.Get(kTeamStateUI);
				if( !pkTop )
				{
					pkTop = XUIMgr.Call(kTeamStateUI);
					if( !pkTop )
					{
						return;
					}
				}
				UpdateTeamStateUI(pkTop);
			}
		}
		void Update()
		{
			EBattleSquareTeam const eTeam = GetMyTeam(m_kRedTeam, m_kBlueTeam);
			bool bJoinTeam = (BST_NONE == eTeam);
			if( bJoinTeam )
			{
				XUI::CXUI_Wnd* pkTop = XUIMgr.Get(kTeamSelectUI);
				if( !pkTop )
				{
					return;
				}
				UpdateTeamSelectUI(pkTop);
			}
			else
			{
				XUI::CXUI_Wnd* pkTop = XUIMgr.Get(kTeamStateUI);
				if( !pkTop )
				{
					return;
				}
				UpdateTeamStateUI(pkTop);
			}
		}

	private:
		void UpdateTeamSelectUI(XUI::CXUI_Wnd* pkTop)
		{
			EBattleSquareTeam const eTeam = GetMyTeam(m_kRedTeam, m_kBlueTeam);
			bool bVisibleBG = (BST_NONE == eTeam);

			XUI::CXUI_Wnd* pkBG1Wnd = pkTop->GetControl( std::wstring(L"IMG_T_BG1") );
			if( pkBG1Wnd )
			{
				pkBG1Wnd->Visible( bVisibleBG );
			}
			XUI::CXUI_Wnd* pkBG2Wnd = pkTop->GetControl( std::wstring(L"IMG_T_BG2") );
			if( pkBG2Wnd )
			{
				pkBG2Wnd->Visible( bVisibleBG );
			}
			XUI::CXUI_Wnd* pkBG3Wnd = pkTop->GetControl( std::wstring(L"SFRM_BG_1") );
			if( pkBG3Wnd )
			{
				pkBG3Wnd->Visible( bVisibleBG );
			}
			XUI::CXUI_Wnd* pkTitleWnd = pkTop->GetControl( std::wstring(L"FRM_TITLE") );
			if( pkTitleWnd )
			{
				pkTitleWnd->Visible( bVisibleBG );
			}
			XUI::CXUI_Wnd* pkTitleDescWnd = pkTop->GetControl( std::wstring(L"FRM_TITLE_DESC") );
			if( pkTitleDescWnd )
			{
				pkTitleDescWnd->Visible( bVisibleBG );
			}
			XUI::CXUI_Wnd* pkContentsDescWnd = pkTop->GetControl( std::wstring(L"FRM_CONTENTS_DESC") );
			if( pkContentsDescWnd )
			{
				pkContentsDescWnd->Visible( bVisibleBG );
			}

			UpdateTeamList( dynamic_cast< XUI::CXUI_List* >(pkTop->GetControl(L"LST_RED")), m_kRedTeam );
			UpdateTeamList( dynamic_cast< XUI::CXUI_List* >(pkTop->GetControl(L"LST_BLUE")), m_kBlueTeam );

			XUI::CXUI_Wnd* pkRedBtn1 = pkTop->GetControl( std::wstring(L"BTN_RED_JOIN_1") );
			if( pkRedBtn1 )
			{
				//pkRedBtn1->Enable(false);
			}
			XUI::CXUI_Wnd* pkBlueBtn1 = pkTop->GetControl( std::wstring(L"BTN_BLUE_JOIN_1") );
			if( pkBlueBtn1 )
			{
				//pkBlueBtn1->Enable(false);
			}

			XUI::CXUI_Wnd* pkRedBtn2 = pkTop->GetControl( std::wstring(L"BTN_RED_JOIN_2") );
			if( pkRedBtn2 )
			{
				pkRedBtn2->Visible( BST_NONE == eTeam ); // 팀이 없을 때만
			}
			XUI::CXUI_Wnd* pkBlueBtn2 = pkTop->GetControl( std::wstring(L"BTN_BLUE_JOIN_2") );
			if( pkBlueBtn2 )
			{
				pkBlueBtn2->Visible( BST_NONE == eTeam );
			}
			XUI::CXUI_Wnd* pkSelectRedWnd = pkTop->GetControl( std::wstring(L"IMG_RED_SELECTED") );
			XUI::CXUI_Wnd* pkSelectBlueWnd = pkTop->GetControl( std::wstring(L"IMG_BLUE_SELECTED") );
			if( pkSelectRedWnd
			&&	pkSelectBlueWnd )
			{
				switch( eTeam )
				{
				case BST_RED:
					{
						pkSelectRedWnd->Visible(true);
						pkSelectBlueWnd->Visible(false);
					}break;
				case BST_BLUE:
					{
						pkSelectRedWnd->Visible(false);
						pkSelectBlueWnd->Visible(true);
					}break;
				default:
					{
						pkSelectRedWnd->Visible(false);
						pkSelectBlueWnd->Visible(false);
					}break;
				}
			}
		}
		
		void UpdateTeamStateUI(XUI::CXUI_Wnd* pkTop)
		{
			//그림두장
			XUI::CXUI_Wnd* pkBG1Wnd = pkTop->GetControl( std::wstring(L"FRM_IMG1") );
			if(!pkBG1Wnd )
			{
				return;
			}
			XUI::CXUI_Wnd* pkBG2Wnd = pkTop->GetControl( std::wstring(L"FRM_IMG2") );
			if(!pkBG2Wnd )
			{
				return;
			}
			//팀 타이틀			
			XUI::CXUI_Wnd* pkTitleRedWnd = pkTop->GetControl( std::wstring(L"FRM_TEAM_RED_TITLE") );
			if(!pkTitleRedWnd )
			{
				return;
			}
			XUI::CXUI_Wnd* pkTitleBlueWnd = pkTop->GetControl( std::wstring(L"FRM_TEAM_BLUE_TITLE") );
			if(!pkTitleBlueWnd )
			{
				return;
			}
			//텍스트 Red
			XUI::CXUI_Wnd* pkRankRedWnd = pkTop->GetControl( std::wstring(L"FRM_TEAM_RED_TITLE") );
			if(!pkRankRedWnd )
			{
				return;
			}
			XUI::CXUI_Wnd* pkClassRedWnd = pkTop->GetControl( std::wstring(L"FRM_TEAM_RED_CLASS") );
			if(!pkClassRedWnd )
			{
				return;
			}
			XUI::CXUI_Wnd* pkNameRedWnd = pkTop->GetControl( std::wstring(L"FRM_TEAM_RED_NAME") );
			if(!pkNameRedWnd )
			{
				return;
			}
			XUI::CXUI_Wnd* pkPointRedWnd = pkTop->GetControl( std::wstring(L"FRM_TEAM_BLUE_POINT") );
			if(!pkPointRedWnd )
			{
				return;
			}
			//텍스트 Blue
			XUI::CXUI_Wnd* pkRankBlueWnd = pkTop->GetControl( std::wstring(L"FRM_TEAM_BLUE_TITLE") );
			if(!pkRankBlueWnd )
			{
				return;
			}
			XUI::CXUI_Wnd* pkClassBlueWnd = pkTop->GetControl( std::wstring(L"FRM_TEAM_BLUE_CLASS") );
			if(!pkClassBlueWnd )
			{
				return;
			}
			XUI::CXUI_Wnd* pkNameBlueWnd = pkTop->GetControl( std::wstring(L"FRM_TEAM_BLUE_NAME") );
			if(!pkNameBlueWnd )
			{
				return;
			}
			XUI::CXUI_Wnd* pkPointBlueWnd = pkTop->GetControl( std::wstring(L"FRM_TEAM_BLUE_POINT") );
			if(!pkPointBlueWnd )
			{
				return;
			}

			UpdateTeamList( dynamic_cast< XUI::CXUI_List* >(pkTop->GetControl(L"LST_RED")), m_kRedTeam );
			UpdateTeamList( dynamic_cast< XUI::CXUI_List* >(pkTop->GetControl(L"LST_BLUE")), m_kBlueTeam );

		}
		void UpdateTeamList(XUI::CXUI_List* pkListWnd, PgBSTeam const& rkTeam)
		{
			if( !pkListWnd )
			{
				return;
			}

			CONT_BS_TEAM_MEMBER kTeamMember;
			{
				CONT_BS_TEAM_GUID_MEMBER::const_iterator iter = rkTeam.ContMember().begin();
				while( rkTeam.ContMember().end() != iter )
				{
					std::back_inserter(kTeamMember) = (*iter).second;
					++iter;
				}
			}
			kTeamMember.sort(); // 성적대로 정렬

			typedef std::list< XUI::SListItem* > CONT_XUI_LIST_ITEM;
			CONT_XUI_LIST_ITEM kContListItem;
			XUI::SListItem* pkBegin = pkListWnd->FirstItem();
			while( pkBegin )
			{
				kContListItem.push_back(pkBegin);
				pkBegin = pkListWnd->NextItem(pkBegin);
			}

			// 팀 맴버
			int iCurRank = 1, iCount = 1;
			CONT_BS_TEAM_MEMBER::const_iterator prev_iter = kTeamMember.end();
			CONT_BS_TEAM_MEMBER::const_iterator iter = kTeamMember.begin();
			while( kTeamMember.end() != iter )
			{
				XUI::SListItem* pkNewListItem = NULL;
				if( kContListItem.empty() )
				{
					pkNewListItem = pkListWnd->AddItem( std::wstring() );
				}
				else
				{
					pkNewListItem = kContListItem.front();
					kContListItem.pop_front();
				}
				if( pkNewListItem )
				{
					XUI::CXUI_Wnd* pkListItem = pkNewListItem->m_pWnd;
					if( pkListItem )
					{
						if( kTeamMember.end() != prev_iter )
						{
							if( *prev_iter == *iter )
							{
								// 순위 증가 없음
							}
							else
							{
								iCurRank = iCount;
							}
						}
						EBattleSquareTeam const eTeam = GetMyTeam(m_kRedTeam, m_kBlueTeam);
						bool bJoinTeam = (BST_NONE == eTeam);
						if( bJoinTeam )
						{
							UpdateTeamSelectItem(pkListItem, (*iter), iCurRank);
						}
						else
						{
							UpdateTeamStateItem(pkListItem, (*iter), iCurRank);
						}
					}
				}

				prev_iter = iter;
				++iter;
				++iCount;
			}

			// 대기자
			iter = rkTeam.ContWaiter().begin();
			while( rkTeam.ContWaiter().end() != iter )
			{
				XUI::SListItem* pkNewListItem = NULL;
				if( kContListItem.empty() )
				{
					pkNewListItem = pkListWnd->AddItem( std::wstring() );
				}
				else
				{
					pkNewListItem = kContListItem.front();
					kContListItem.pop_front();
				}

				if( pkNewListItem )
				{
					XUI::CXUI_Wnd* pkListItem = pkNewListItem->m_pWnd;
					if( pkListItem )
					{
						int const iCurRank = 0;						
						EBattleSquareTeam const eTeam = GetMyTeam(m_kRedTeam, m_kBlueTeam);
						bool bJoinTeam = (BST_NONE == eTeam);
						if( bJoinTeam )
						{
							UpdateTeamSelectItem(pkListItem, (*iter), iCurRank);
						}
						else
						{
							UpdateTeamStateItem(pkListItem, (*iter), iCurRank);
						}
					}
				}
				++iter;
			}

			while( !kContListItem.empty() )
			{
				pkListWnd->DeleteItem( kContListItem.front() );
				kContListItem.pop_front();
			}
		}
		void UpdateTeamSelectItem(XUI::CXUI_Wnd* pkItemWnd, SBSTeamMember const& rkTeamMember, int const iCurRank)
		{
			if( !pkItemWnd )
			{
				return;
			}

			XUI::CXUI_Wnd* pkRankWnd = pkItemWnd->GetControl( std::wstring(L"SFRM_RANK") );
			if( pkRankWnd )
			{
				if( 0 != iCurRank )
				{
					pkRankWnd->Text( BM::vstring(iCurRank) );
				}
				else
				{
					pkRankWnd->Text( std::wstring(L"-") );
				}
			}
			//Level표기 삭제
			/*XUI::CXUI_Wnd* pkLevelWnd = pkItemWnd->GetControl( std::wstring(L"SFRM_LV") );
			if( pkLevelWnd )
			{
				std::wstring kText;
				if( FormatTTW(kText, 460078, rkTeamMember.usLevel) )
				{
					pkLevelWnd->Text( kText );
				}
			}*/
			//직업표기도 삭제
			/*XUI::CXUI_Wnd* pkClassWnd = pkItemWnd->GetControl( std::wstring(L"IMG_CLASS") );
			if( pkClassWnd )
			{
				SUVInfo kUVInfo = pkClassWnd->UVInfo();
				kUVInfo.Index = static_cast< size_t >(rkTeamMember.usClass);
				pkClassWnd->UVInfo( kUVInfo );
			}
			*/
			XUI::CXUI_Wnd* pkNameWnd = pkItemWnd->GetControl( std::wstring(L"SFRM_NAME") );
			if( pkNameWnd )
			{
				pkNameWnd->Text( rkTeamMember.kCharName );
			}
			XUI::CXUI_Wnd* pkMyItemWnd = pkItemWnd->GetControl( std::wstring(L"IMG_MY_ITEM") );
			if( pkMyItemWnd )
			{
				bool const bIsMyItem = g_kPilotMan.IsMyPlayer(rkTeamMember.kCharGuid);
				pkMyItemWnd->Visible( bIsMyItem );
			}
		}

		void UpdateTeamStateItem(XUI::CXUI_Wnd* pkItemWnd, SBSTeamMember const& rkTeamMember, int const iCurRank)
		{
			if( !pkItemWnd )
			{
				return;
			}
			//3위까지 폰트 색 변경
			DWORD dwRank = 0xFFFFF568;
			DWORD dwNorm = 0xFFFFEEB9;

			XUI::CXUI_Wnd* pkRankWnd = pkItemWnd->GetControl( std::wstring(L"SFRM_RANK") );
			if( pkRankWnd )
			{
				if( 0 != iCurRank )
				{
					if( 3 >= iCurRank)
					{
						pkRankWnd->FontColor(dwRank);
					}
					else
					{
						pkRankWnd->FontColor(dwNorm);
					}
					pkRankWnd->Text( BM::vstring(iCurRank) );
				}
				else
				{
					pkRankWnd->Text( std::wstring(L"-") );
				}
			}
			{// 직업 아이콘 설정
				XUI::CXUI_Wnd* pkClassWnd = pkItemWnd->GetControl( std::wstring(L"IMG_CLASS") );
				lwSetMiniClassIconIndex(pkClassWnd, static_cast<int>(rkTeamMember.usClass) );
			}
			XUI::CXUI_Wnd* pkNameWnd = pkItemWnd->GetControl( std::wstring(L"SFRM_NAME") );
			if( pkNameWnd )
			{					
				if( 3 >= iCurRank)
				{
					pkNameWnd->FontColor(dwRank);
				}
				else
				{
					pkNameWnd->FontColor(dwNorm);
				}
				pkNameWnd->Text( rkTeamMember.kCharName );
			}
			XUI::CXUI_Wnd* pkPointWnd = pkItemWnd->GetControl( std::wstring(L"SFRM_POINT") );
			if( pkPointWnd )
			{				
				if( 3 >= iCurRank)
				{
					pkPointWnd->FontColor(dwRank);
				}
				else
				{
					pkPointWnd->FontColor(dwNorm);
				}
				pkPointWnd->Text( BM::vstring(rkTeamMember.iPoint) );
			}
			XUI::CXUI_Wnd* pkMedalWnd = pkItemWnd->GetControl( std::wstring(L"SFRM_MEDAL") );
			if( pkMedalWnd )
			{
				if( 3 >= iCurRank)
				{
					pkMedalWnd->FontColor(dwRank);
				}
				else
				{
					pkMedalWnd->FontColor(dwNorm);
				}
				pkMedalWnd->Text( BM::vstring(rkTeamMember.usIconCount) );
			}
		}
	private:
		PgBSTeam const& m_kRedTeam;
		PgBSTeam const& m_kBlueTeam;
		PgBSGame const& m_kBSGame;
		bool const m_bHaveFreePVPAttr;
	};

	//
	std::wstring const kRedTeamPointWndName(L"FRM_BS_TEAM_RED_POINT");
	std::wstring const kBlueTeamPointWndName(L"FRM_BS_TEAM_BLUE_POINT");
	std::wstring const kRedTeamMedalWndName(L"FRM_BS_TEAM_RED_MEDAL");
	std::wstring const kBlueTeamMedalWndName(L"FRM_BS_TEAM_BLUE_MEDAL");
	std::wstring const kCenterNoticeWndName(L"FRM_BS_CENTER_NOTICE");
	std::wstring const kMyPointWndName(L"FRM_BS_MY_POINT");
	std::wstring const kWinTeamPointItemWnd(L"FRM_BS_WIN_TEAM_ITEM");
	std::wstring const kMyPointItemWnd(L"FRM_BS_MY_ITEM");
	class PgBSMainUI
	{
	public:
		PgBSMainUI(PgBSTeam const& rkRedTeam, PgBSTeam const& rkBlueTeam, PgBSGame const& rkBSGame, bool const bHaveFreePVPAttr)
			: m_kRedTeam(rkRedTeam), m_kBlueTeam(rkBlueTeam), m_kBSGame(rkBSGame), m_bHaveFreePVPAttr(bHaveFreePVPAttr)
		{
		}
		~PgBSMainUI()
		{
		}

		void Close()
		{
			XUIMgr.Close(kRedTeamPointWndName);
			XUIMgr.Close(kBlueTeamPointWndName);
			XUIMgr.Close(kCenterNoticeWndName);
			XUIMgr.Close(kMyPointWndName);
			XUIMgr.Close(kWinTeamPointItemWnd);
			XUIMgr.Close(kMyPointItemWnd);

			XUI::CXUI_Wnd* pkBuff = XUIMgr.Get(L"FRM_BUFF");
			if(pkBuff)
			{
				pkBuff->Location(0,0);
			}
			g_kQuestMan.ShowMiniQuest(true);
		}
		void Call()
		{
			XUI::CXUI_Wnd* pkRedTeamPointWnd = XUIMgr.Activate(kRedTeamPointWndName);
			if( 0 == m_kRedTeam.TeamPoint())
			{
				UpdatePointUI(pkRedTeamPointWnd, m_kRedTeam.TeamPoint());
			}
			XUI::CXUI_Wnd* pkBlueTeamPointWnd = XUIMgr.Activate(kBlueTeamPointWndName);
			if( 0 == m_kBlueTeam.TeamPoint())
			{
				UpdatePointUI(pkBlueTeamPointWnd, m_kBlueTeam.TeamPoint());
			}
			XUI::CXUI_Wnd* pkRedTeamMedalWnd = XUIMgr.Activate(kRedTeamMedalWndName);
			XUI::CXUI_Wnd* pkBlueTeamMedalWnd = XUIMgr.Activate(kBlueTeamMedalWndName);
			//캐릭터 머리위에 용자의 증표 갯수 표시
			CONT_BS_TEAM_GUID_MEMBER const& rkRedTeam = m_kRedTeam.ContMember();
			CONT_BS_TEAM_GUID_MEMBER::const_iterator iter_Red = rkRedTeam.begin();
			while( rkRedTeam.end() != iter_Red)
			{
				PgActor* pPlayer = g_kPilotMan.FindActor( (*iter_Red).first );
				if(pPlayer)
				{
					SBSTeamMember const kMember = (*iter_Red).second;
					pPlayer->UpdateCustomCount( kMember.usIconCount );
				}
				++iter_Red;
			}
			CONT_BS_TEAM_GUID_MEMBER const& rkBlueTeam = m_kBlueTeam.ContMember();
			CONT_BS_TEAM_GUID_MEMBER::const_iterator iter_Blue = rkBlueTeam.begin();
			while( rkBlueTeam.end() != iter_Blue)
			{
				PgActor* pPlayer = g_kPilotMan.FindActor( (*iter_Blue).first );
				if(pPlayer)
				{
					SBSTeamMember const kMember = (*iter_Blue).second; 
					pPlayer->UpdateCustomCount( kMember.usIconCount );
				}
				++iter_Blue;
			}
			//
			EBattleSquareTeam const eTeam = GetMyTeam(m_kRedTeam, m_kBlueTeam);
			SBSTeamMember kMyTeamMemberInfo;
			if( m_bHaveFreePVPAttr
			&&	BST_NONE != eTeam
			&&	GetMyMemberInfo(m_kRedTeam, m_kBlueTeam, kMyTeamMemberInfo) )
			{
				XUIMgr.Close( kCenterNoticeWndName );
				XUI::CXUI_Wnd* pkMyPointWnd = XUIMgr.Activate( kMyPointWndName );
				if( 0 == kMyTeamMemberInfo.iPoint )
				{
					UpdatePointUI(pkMyPointWnd, kMyTeamMemberInfo.iPoint);
				}

				XUI::CXUI_Wnd* pkWinTeamPointItemWnd = XUIMgr.Activate( kWinTeamPointItemWnd );
				if( pkWinTeamPointItemWnd )
				{
					SBSRewardItem kCurItem, kNextItem;
					int const iTeamPoint = (BST_RED == eTeam)? m_kRedTeam.TeamPoint(): m_kBlueTeam.TeamPoint();
					bool const bFindItem = FindPointItem(m_kBSGame.ContWinTeamRewardItem(), iTeamPoint, kCurItem, kNextItem);
					UpdatePointRewardUI(pkWinTeamPointItemWnd, kCurItem, kNextItem);
				}
				XUI::CXUI_Wnd* pkMyPointItemWnd = XUIMgr.Activate( kMyPointItemWnd );
				if( pkMyPointItemWnd )
				{
					SBSRewardItem kCurItem, kNextItem;
					bool const bFindItem = FindPointItem(m_kBSGame.ContPrivateRewardItem(), kMyTeamMemberInfo.iPoint, kCurItem, kNextItem);
					UpdatePointRewardUI(pkMyPointItemWnd, kCurItem, kNextItem);
				}
			}
			else
			{
				XUIMgr.Activate( kCenterNoticeWndName );
				XUIMgr.Close( kMyPointWndName );
				XUIMgr.Close( kWinTeamPointItemWnd );
				XUIMgr.Close( kMyPointItemWnd );
			}
		}
	private:
		void UpdatePointUI(XUI::CXUI_Wnd* pkTopWnd, int const iPoint)
		{
			if( !pkTopWnd )
			{
				return;
			}
			XUI::CXUI_Wnd* pkPointWnd = pkTopWnd->GetControl( std::wstring(L"FRM_POINT") );
			if( pkPointWnd )
			{
				pkPointWnd->Text( BM::vstring(iPoint) );
			}
		}
		void UpdatePointRewardUI(XUI::CXUI_Wnd* pkTopWnd, SBSRewardItem const& rkCurItem, SBSRewardItem const& rkNextItem)
		{
			if( !pkTopWnd )
			{
				return;
			}
			XUI::CXUI_Wnd* pkIconCurWnd = pkTopWnd->GetControl( std::wstring(L"ICON_CUR") );
			if( pkIconCurWnd )
			{
				SQuestItem kRewardItemOld;
				pkIconCurWnd->GetCustomData(&kRewardItemOld, sizeof(kRewardItemOld));

				SQuestItem const kRewardItem(rkCurItem.iItemNo1, rkCurItem.iCount1);
				pkIconCurWnd->SetCustomData(&kRewardItem, sizeof(kRewardItem));
				if(kRewardItemOld.iItemNo != kRewardItem.iItemNo 
					|| kRewardItemOld.iCount != kRewardItem.iCount)
				{
					if( 0 != kRewardItemOld.iCount)
					{
						pkTopWnd->OwnerState(ECRS_PREV_EFFECT_STARTED);
					}
				}
				Quest::lwOnCallQuestRewardItemImage( lwUIWnd(pkIconCurWnd) );
				if( 1 < rkCurItem.iCount1 )
				{
					pkIconCurWnd->Text( BM::vstring(rkCurItem.iCount1) );
				}
				else
				{
					pkIconCurWnd->Text( std::wstring() );
				}
			}
			XUI::CXUI_Wnd* pkIconNextWnd = pkTopWnd->GetControl( std::wstring(L"ICON_NEXT") );
			if( pkIconNextWnd )
			{
				SQuestItem const kRewardItem(rkNextItem.iItemNo1, rkNextItem.iCount1);
				pkIconNextWnd->SetCustomData(&kRewardItem, sizeof(kRewardItem));
				Quest::lwOnCallQuestRewardItemImage( lwUIWnd(pkIconNextWnd) );
				if( 1 < rkNextItem.iCount1 )
				{
					pkIconNextWnd->Text( BM::vstring(rkNextItem.iCount1) );
				}
				else
				{
					pkIconNextWnd->Text( std::wstring() );
				}
			}
			XUI::CXUI_Wnd* pkIconCurPointWnd = pkTopWnd->GetControl( std::wstring(L"FRM_CUR_POINT") );
			if( pkIconCurPointWnd )
			{
				pkIconCurPointWnd->Text( BM::vstring(rkCurItem.iMinPoint) );
			}
			XUI::CXUI_Wnd* pkIconNextPointWnd = pkTopWnd->GetControl( std::wstring(L"FRM_NEXT_POINT") );
			if( pkIconNextPointWnd )
			{
				pkIconNextPointWnd->Text( BM::vstring(rkNextItem.iMinPoint) );
			}


		}
		bool FindPointItem(std::list< SBSRewardItem > const& rkCont, int const iPoint, SBSRewardItem& rkCurItem, SBSRewardItem& rkNextItem)
		{
			typedef std::list< SBSRewardItem > CONT_POINT_ITEM;
			CONT_POINT_ITEM::const_iterator cur_iter = rkCont.end();
			CONT_POINT_ITEM::const_iterator next_iter = rkCont.end();
			CONT_POINT_ITEM::const_iterator iter = rkCont.begin();
			while( rkCont.end() != iter )
			{
				if( (*iter).iMinPoint <= iPoint )
				{
					cur_iter = next_iter = iter;
					++next_iter;
				}
				++iter;
			}
			if( rkCont.end() != cur_iter )
			{
				rkCurItem = (*cur_iter);
				if( rkCont.end() == next_iter )
				{
					rkNextItem = (*cur_iter);
				}
				else
				{
					rkNextItem = (*next_iter);
				}
				return true;
			}
			return false;
		}

	private:
		PgBSTeam const& m_kRedTeam;
		PgBSTeam const& m_kBlueTeam;
		PgBSGame const& m_kBSGame;
		bool const m_bHaveFreePVPAttr;
	};

	//
	void JoinChannel(int const iGameIDX)
	{
		if( !g_kBattleSquareMng.GetCanEnterGameIDX(iGameIDX) )
		{
			return;
		}

		BM::Stream kPacket(PT_C_T_REQ_WANT_JOIN_BS_CHANNEL);
		kPacket.Push( iGameIDX );
		NETWORK_SEND( kPacket );
	}

	//
	int const iBSRewardStartIndex = 1;
	int const iBSRewardIconCount = 4;
	void CallBSReward(int iPos, int const iItemNo, int const iCount, int const iTotalCount)
	{
		GET_DEF(CItemDefMgr, kItemDefMgr);
		CItemDef const* pkItemDef = kItemDefMgr.GetDef(iItemNo);
		if( NULL == pkItemDef )
		{
			return;
		}

		XUI::CXUI_Wnd* pkTopWnd = XUIMgr.Activate( std::wstring(L"FRM_BS_RESULT_ITEM") );
		if( !pkTopWnd )
		{
			return;
		}
		XUI::CXUI_Wnd* pkMsgWnd = pkTopWnd->GetControl( std::wstring(L"FRM_MSG") );
		if( !pkMsgWnd )
		{
			return;
		}

		int const iMinCount = 2;
		bool const bUseFourBG = (iMinCount < iTotalCount);
		XUI::CXUI_Wnd* pkBG4Wnd = pkTopWnd->GetControl( std::wstring(L"FRM_BG_4") );
		XUI::CXUI_Wnd* pkBG2Wnd = pkTopWnd->GetControl( std::wstring(L"FRM_BG_2") );
		if( pkBG4Wnd
		&&	pkBG2Wnd )
		{
			pkBG4Wnd->Visible( bUseFourBG );
			pkBG2Wnd->Visible( !bUseFourBG );
		}

		std::wstring kTempStr;
		wchar_t const* pkItemName = NULL;
		if( GetDefString(pkItemDef->NameNo(), pkItemName) )
		{
			if( FormatTTW(kTempStr, 460032, pkItemName) )
			{
				kTempStr += std::wstring(L"\n");
			}
		}
		std::wstring kIconWndName(L"ICN_ITEM_");
		if( iBSRewardStartIndex == iPos )
		{
			pkMsgWnd->Text(std::wstring());
			for( int iCur = iBSRewardStartIndex; iBSRewardIconCount >= iCur; ++iCur )
			{
				XUI::CXUI_Wnd* pkIconWnd = pkTopWnd->GetControl( kIconWndName+std::wstring(BM::vstring(iCur)) );
				if( pkIconWnd )
				{
					pkIconWnd->DefaultImgName(std::wstring());
					pkIconWnd->SetInvalidate();
					pkIconWnd->Text( std::wstring() );
				}
			}
		}
		pkMsgWnd->Text( pkMsgWnd->Text() + kTempStr );
		if( iMinCount >= iTotalCount )
		{
			iPos += 1; // 2개 이하 일 땐, 2,3 번에 찍도록
		}

		XUI::CXUI_Wnd* pkIconWnd = pkTopWnd->GetControl( kIconWndName+std::wstring(BM::vstring(iPos)) );
		if( pkIconWnd )
		{
			SQuestItem const kQuestItem(iItemNo, iCount);
			pkIconWnd->SetCustomData(&kQuestItem, sizeof(SQuestItem));
			Quest::lwOnCallQuestRewardItemImage( lwUIWnd(pkIconWnd) );
			pkIconWnd->Scale( 1.5f ); // 1.5배

			if( pkItemDef->CanEquip() )
			{
				pkIconWnd->Text( std::wstring() );
			}
			else
			{
				pkIconWnd->Text( BM::vstring(iCount) );
			}
		}
	}

	//
	void SendJoinBSTeam(int const iJoinTeam)
	{
		BM::Stream kPacket(PT_C_M_REQ_WANT_JOIN_BS);
		kPacket.Push( static_cast< EBattleSquareTeam >(iJoinTeam) );
		NETWORK_SEND(kPacket);
	}
	void SendLeaveBSGround()
	{
		lwCallYesNoMsgBox(TTW(460094), BM::GUID::NullData(), MBT_CONFIRM_LEAVE_BATTLESQUARE);
	}
	void AddBSNotice(int const iTextTableNo)
	{
		g_kBattleSquareMng.AddNotice(iTextTableNo);
	}
	void CallChannelUI()
	{
		BM::Stream kPacket(PT_C_T_REQ_BS_CHANNEL_INFO);
		NETWORK_SEND(kPacket);
	}

	void lwOnEffectView(lwUIWnd kSelf, char const* szChangeRewardModel)
	{
		BM::vstring SZ_CHANGE_REWARD_MODEL(szChangeRewardModel);
		wchar_t const* const SZ_CHANGE_REWARD_EFF1 = L"CraftSpin";
		wchar_t const* const SZ_CHANGE_REWARD_EFF2 = L"CraftFlash";

		wchar_t const* const SZ_CHANGE_REWARD_PROGRESS_SND = L"Item-try";
		wchar_t const* const SZ_CHANGE_REWARD_SUCCESS_SND = L"Item-Success";

		static float fStartTime = 0.0f;
		float const RARITY_PROGRESS_TIME = 0.667f; //진행 시간은 1초
		float const RARITY_FINISHED_TIME = 0.53f; //진행 시간은 1초

		XUI::CXUI_Wnd* pkMain = kSelf.GetSelf();
		if( pkMain )
		{
			PgUIModel* pkModel = g_kUIScene.FindUIModel(SZ_CHANGE_REWARD_MODEL);
			if( pkModel )
			{
				E_CHANGE_REWARD_STATE const eState = static_cast<E_CHANGE_REWARD_STATE>(pkMain->OwnerState());
				if( ECRS_NOTHING == eState )
				{
					return;
				}

				if( ECRS_NEXT_EFFECT_STARTED == pkMain->OwnerState() )
				{
					if( 0.0f == fStartTime )
					{
						fStartTime = g_pkApp->GetAccumTime();
						pkModel->SetNIFEnableUpdate(MB(SZ_CHANGE_REWARD_EFF1), true);
						pkModel->ResetNIFAnimation(MB(SZ_CHANGE_REWARD_EFF2));
						pkModel->SetEnableUpdate(true);
						g_kUISound.PlaySoundByID( SZ_CHANGE_REWARD_PROGRESS_SND );
					}
					else
					{
						float fPassTime = g_pkApp->GetAccumTime() - fStartTime;
						if( fPassTime < RARITY_FINISHED_TIME )
						{
							return;
						}

						pkModel->SetNIFEnableUpdate(MB(SZ_CHANGE_REWARD_EFF2), false);
						pkModel->SetEnableUpdate(false);
						pkMain->OwnerState(ECRS_NOTHING);
						fStartTime = 0.0f;
					}
				}
				else
				{
					if( 0.0f == fStartTime )
					{
						fStartTime = g_pkApp->GetAccumTime();
						pkModel->SetNIFEnableUpdate(MB(SZ_CHANGE_REWARD_EFF1), true);
						pkModel->ResetNIFAnimation(MB(SZ_CHANGE_REWARD_EFF2));
						pkModel->SetEnableUpdate(true);
						g_kUISound.PlaySoundByID( SZ_CHANGE_REWARD_SUCCESS_SND );
					}
					else
					{
						float fPassTime = g_pkApp->GetAccumTime() - fStartTime;
						if( fPassTime < RARITY_PROGRESS_TIME )
						{
							return;
						}

						pkModel->SetNIFEnableUpdate(MB(SZ_CHANGE_REWARD_EFF1), false);
						pkModel->SetEnableUpdate(false);
						pkMain->OwnerState(ECRS_NOTHING);
						fStartTime = 0.0f;
					}
				}
			}
		}
	}
	bool RegisterWrapper(lua_State *pkState)
	{
		using namespace lua_tinker;
		//
		def(pkState, "CallChannelUI",	CallChannelUI);
		def(pkState, "JoinBattleSquareChannel",	JoinChannel);
		def(pkState, "Call_BSReward", CallBSReward);
		def(pkState, "SendJoinBSTeam", SendJoinBSTeam);
		def(pkState, "SendLeaveBSGround", SendLeaveBSGround);
		def(pkState, "Add_BSNotice", AddBSNotice);
		def(pkState, "OnEffectView", lwOnEffectView);
		return true;
	}
};


//
PgBSNoticeMng::PgBSNoticeMng()
	: m_fLastNoticeTime(0), m_kContNotice(), m_iCurNotice(0), m_kEnable(true), m_fInitTime(0)
{
}
PgBSNoticeMng::~PgBSNoticeMng()
{
}

void PgBSNoticeMng::AddNotice(int const iTTW)
{
	m_kContNotice.push_back( TTW(iTTW) );
}
void PgBSNoticeMng::Init()
{
	m_kContNotice.clear();
	m_fInitTime = m_fLastNoticeTime = g_pkApp->GetAccumTime();
	m_iCurNotice = 0;
	m_kEnable = true;
}
void PgBSNoticeMng::Update()
{
	if( m_kContNotice.empty()
	||	false == m_kEnable )
	{
		return;
	}
	float const fAccumTime = g_pkApp->GetAccumTime();

/*	float const fServiceTime = 120.f; // 2분 동안만 제공
	if( fServiceTime < (fAccumTime - m_fInitTime) )
	{
		return;
	}
*/
	float const fResetTime = 5.f;
	if( fResetTime < (fAccumTime - m_fLastNoticeTime) )
	{
		m_fLastNoticeTime = fAccumTime;
		//g_kChatMgrClient.AddLogMessage(SChatLog(CT_EVENT), m_kContNotice.at(m_iCurNotice), true, 0);

		XUI::CXUI_Wnd* pkInfo = XUIMgr.Get(L"FRM_BS_INFO");
		if( !pkInfo || pkInfo->IsClosed() )
		{
			pkInfo = XUIMgr.Call(L"FRM_BS_INFO");
		}
		if(pkInfo)
		{
			pkInfo->Text( m_kContNotice.at(m_iCurNotice) );
		}

		++m_iCurNotice;
		if( m_kContNotice.size() <= m_iCurNotice )
		{
			m_iCurNotice = 0;
		}
	}
}

//
PgBSKillSoundMng::PgBSKillSoundMng()
	: m_iKillCount(0), m_fLastKillTime(0.f), m_pkNowAudio(NULL), m_kNextAudio()
{
}
PgBSKillSoundMng::~PgBSKillSoundMng()
{
}
void PgBSKillSoundMng::Update()
{
	float const fResetTime = 3.f;
	float const fAccumTime = g_pkApp->GetAccumTime();
	if( 0 != m_kNextAudio.size() )
	{
		if( NULL == m_pkNowAudio
		||	m_pkNowAudio->GetStatus() == NiAudioSource::DONE )
		{
			PgActor* pkMyActor = g_kPilotMan.GetPlayerActor();
			if( pkMyActor )
			{
				m_pkNowAudio = g_kSoundMan.PlayAudioSourceByID(NiAudioSource::TYPE_3D, m_kNextAudio.c_str(), 0.0f, 0.0f, 0.0f, pkMyActor);
			}
			m_kNextAudio.clear();
		}
	}
	if( fResetTime <= (fAccumTime - m_fLastKillTime) )
	{
		m_iKillCount = 0;
		m_fLastKillTime = fAccumTime;
	}
}
void PgBSKillSoundMng::AddKill()
{
	++m_iKillCount;
	if( g_pkApp )
	{
		m_fLastKillTime = g_pkApp->GetAccumTime();
	}

	switch( m_iKillCount )
	{
	case 1:		{ ; }break;
	case 2:		{ m_kNextAudio = std::string("PVP_Double_Kill"); }break;
	case 3:		{ m_kNextAudio = std::string("PVP_Triple_Kill"); }break;
	case 4:		{ m_kNextAudio = std::string("PVP_Excellent"); }break;
	case 5:		{ m_kNextAudio = std::string("PVP_Unbelievable"); }break;
	default:
		{
			if( 6 <= m_iKillCount )
			{
				m_kNextAudio = std::string("PVP_Oh_My_God");
			}
		}break;
	}
}
void PgBSKillSoundMng::Clear()
{
	//if( m_pkNowAudio )
	//{
	//	m_pkNowAudio->Stop();
	//}
	m_iKillCount = 0;
	m_pkNowAudio = NULL;
	m_kNextAudio.clear();
}

//
tagBSClientGameInfo::tagBSClientGameInfo(BM::Stream& rkPacket)
{
	rkPacket.Pop( eStatus );
	rkPacket.Pop( iCurUserCount );
	kBSGame.ReadFromPacket( rkPacket );
}
tagBSClientGameInfo::tagBSClientGameInfo(tagBSClientGameInfo const& rhs)
	: eStatus(rhs.eStatus), iCurUserCount(rhs.iCurUserCount), kBSGame(rhs.kBSGame)
{
}
tagBSClientGameInfo::~tagBSClientGameInfo()
{
}
bool tagBSClientGameInfo::operator < (tagBSClientGameInfo const& rhs)
{
	BM::PgPackedTime const& rkStartTime = kBSGame.kStartTime;
	BM::PgPackedTime const& kRhsTime = rhs.kBSGame.kStartTime;
	if( rkStartTime.Hour() < kRhsTime.Hour() )
	{
		return true;
	}
	if( rkStartTime.Hour() > kRhsTime.Hour() )
	{
		return false;
	}

	if( rkStartTime.Min() < kRhsTime.Min() )
	{
		return true;
	}
	if( rkStartTime.Min() > kRhsTime.Min() )
	{
		return false;
	}

	if( rkStartTime.Sec() < kRhsTime.Sec() )
	{
		return true;
	}
	if( rkStartTime.Sec() > kRhsTime.Sec() )
	{
		return false;
	}
	
	if( rkStartTime.Hour() == kRhsTime.Hour()
	&&	rkStartTime.Min() == kRhsTime.Min()
	&&	rkStartTime.Sec() == kRhsTime.Sec()
	&&	kBSGame.iLevelMax < rhs.kBSGame.iLevelMax )
	{
		return true;
	}
	return false;
}

//
tagBSItemUnitPos::tagBSItemUnitPos(BM::Stream& rkPacket)
{
	POINT3 kTempPos;
	rkPacket.Pop( kGuid );
	rkPacket.Pop( kTempPos );
	rkPacket.Pop( iIconType );
			
	kPos = NiPoint3(kTempPos.x, kTempPos.y, kTempPos.z);
}

//
tagBSGameStatusInfo::tagBSGameStatusInfo()
{
	Clear();
}
tagBSGameStatusInfo::tagBSGameStatusInfo(tagBSGameStatusInfo const& rhs)
	: iGameIDX(rhs.iGameIDX), eStatus(rhs.eStatus), iChannelNameTextID(rhs.iChannelNameTextID)
	, iLevelMin(rhs.iLevelMin), iLevelMax(rhs.iLevelMax), iDiffTime(rhs.iDiffTime),
	iCurUserCount(rhs.iCurUserCount), iMaxUser(rhs.iMaxUser)
{
}
tagBSGameStatusInfo::~tagBSGameStatusInfo()
{
}
void tagBSGameStatusInfo::Clear()
{
	iGameIDX = 0;
	eStatus = BSGS_NONE;
	iChannelNameTextID = 0;
	iLevelMin = 0;
	iLevelMax = 0;
	iCurUserCount = 0;
	iMaxUser = 0;
	iDiffTime = 0;
}
void tagBSGameStatusInfo::ReadFromPacket(BM::Stream& rkPacket)
{
	rkPacket.Pop( iGameIDX );
	rkPacket.Pop( eStatus );
	rkPacket.Pop( iChannelNameTextID );
	rkPacket.Pop( iLevelMin );
	rkPacket.Pop( iLevelMax );
	rkPacket.Pop( iDiffTime );
}
bool tagBSGameStatusInfo::operator ==(tagBSGameStatusInfo const& rhs) const
{
	return iChannelNameTextID == rhs.iChannelNameTextID;
}

//
tagBSUnitPos::tagBSUnitPos()
	: iTeamNo(0), kPos(), sIconCount(0)
{
}
tagBSUnitPos::tagBSUnitPos(tagBSUnitPos const& rhs)
: iTeamNo(rhs.iTeamNo), kPos(rhs.kPos), sIconCount(rhs.sIconCount)
{
}
tagBSUnitPos::tagBSUnitPos(BM::Stream& rkPacket)
	: iTeamNo(0), kPos(), sIconCount(0)
{
	ReadFromPacket(rkPacket);
}
tagBSUnitPos::~tagBSUnitPos()
{
}
void tagBSUnitPos::ReadFromPacket(BM::Stream& rkPacket)
{
	POINT3 kTempPos;
	rkPacket.Pop( kTempPos );
	rkPacket.Pop( iTeamNo );
	rkPacket.Pop( sIconCount );
	kPos = NiPoint3(kTempPos.x, kTempPos.y, kTempPos.z);
}

//
tagBSMedalPos::tagBSMedalPos()
	: kPos()
{
}
tagBSMedalPos::tagBSMedalPos(tagBSMedalPos const& rhs)
	: kPos(rhs.kPos)
{
}
tagBSMedalPos::tagBSMedalPos(BM::Stream& rkPacket)
	: kPos()
{
	ReadFromPacket(rkPacket);
}
tagBSMedalPos::~tagBSMedalPos()
{
}
void tagBSMedalPos::ReadFromPacket(BM::Stream& rkPacket)
{
	POINT3 kTempPos;
	rkPacket.Pop( kTempPos );
	kPos = POINT3I(kTempPos.x, kTempPos.y, kTempPos.z);
}

//
PgBattleSquareMng::PgBattleSquareMng()
: m_kBSGame(), m_kManCount(0), m_kContItemUnitPos(), m_kContUnitPos(), m_kContMedalPos(), m_kKillCountMng(), m_kContNoticeInfo(), m_kRedTeam(BST_RED),  m_kBlueTeam(BST_BLUE)
, m_kInGameNotice(), m_bPreTabKeyDownStatus(false), m_eLeadTeam(E_LEAD_PARE)
{
}
PgBattleSquareMng::~PgBattleSquareMng()
{
}
void PgBattleSquareMng::Clear()
{
	m_kManCount = 0;
	m_kContItemUnitPos.clear();
	m_kContUnitPos.clear();
	m_kContMedalPos.clear();
	m_kBSGame.Clear();
	m_kKillCountMng.Clear();
	m_kRedTeam.Clear();
	m_kBlueTeam.Clear();
	m_bPreTabKeyDownStatus = false;
	m_eLeadTeam = E_LEAD_PARE;
}
void PgBattleSquareMng::ClearNotice()
{
	m_kContNoticeInfo.clear();
}
void PgBattleSquareMng::Update()
{
	if( !g_pkWorld
		||	!g_pkWorld->IsHaveAttr(GATTR_FLAG_BATTLESQUARE) )
	{
		return;
	}

	if( NULL == XUIMgr.GetFocusedEdit() )
	{
		if( g_pkLocalManager )
		{
			NiInputKeyboard* pkKeyboard = g_pkLocalManager->GetInputSystem()->GetKeyboard();
			if( pkKeyboard )
			{
				bool bDownTabKey = pkKeyboard->KeyIsDown(static_cast<NiInputKeyboard::KeyCode>(g_kGlobalOption.GetUKeyToKey(NiInputKeyboard::KEY_TAB+PgInput::UR_LOCAL_BEGIN)));
				if( false == m_bPreTabKeyDownStatus
					&& m_bPreTabKeyDownStatus != bDownTabKey)
				{
					FlipTeamUI();
				}
				m_bPreTabKeyDownStatus = bDownTabKey;
			}
		}
	}

	bool const bHaveFreePVPAttr = (0 != (g_pkWorld->DynamicGndAttr()&DGATTR_FLAG_FREEPVP));
	BattleSquareUI::PgBSMainUI kMainUI(m_kRedTeam, m_kBlueTeam, m_kBSGame, bHaveFreePVPAttr);
	kMainUI.Call();

	if( bHaveFreePVPAttr )
	{
		m_kKillCountMng.Update();
	}
	m_kInGameNotice.Update();
}
void PgBattleSquareMng::OnRecvPacket(WORD const wPacketType, BM::Stream& rkPacket)
{
	switch( wPacketType )
	{
	case PT_M_C_ANS_BS_EXIT:
		{
			g_kPilotMan.UnlockPlayerInput(SReqPlayerLock(EPLT_BattleSquare, true)); // 움직임 해제
			g_kPilotMan.UnlockPlayerInput(SReqPlayerLock(EPLT_BattleSquare, false));
		}break;
	case PT_M_C_NFY_BS_DEAD:
		{
			BM::GUID kDeadMan, kKiller;
			std::wstring kDeadManName, kKillerName;

			rkPacket.Pop( kDeadMan );
			rkPacket.Pop( kDeadManName );
			rkPacket.Pop( kKiller );
			rkPacket.Pop( kKillerName );

			if( g_kPilotMan.IsMyPlayer(kDeadMan) )
			{
				if( !kKillerName.empty() )
				{
					std::wstring kTempStr;
					if( FormatTTW(kTempStr, 460038, kKillerName.c_str()) )
					{
						g_kChatMgrClient.AddLogMessage(SChatLog(CT_EVENT), kTempStr, true, 1);
					}
				}
				m_kKillCountMng.Clear();
			}
			if( g_kPilotMan.IsMyPlayer(kKiller) )
			{
				m_kKillCountMng.AddKill();
			}

			XUI::CXUI_List* pkListWnd = dynamic_cast< XUI::CXUI_List* >(XUIMgr.Get( L"LST_BS_KILL_MSG" ));
			if( !pkListWnd )
			{
				pkListWnd = dynamic_cast< XUI::CXUI_List* >(XUIMgr.Call( L"LST_BS_KILL_MSG" ));
				if( !pkListWnd )
				{
					break;
				}
			}

			XUI::SListItem* pkNewItem = pkListWnd->AddItem(std::wstring());
			if( pkNewItem
			&&	pkNewItem->m_pWnd )
			{
				float const fAccumTime = g_pkApp->GetAccumTime();
				pkNewItem->m_pWnd->SetCustomData( &fAccumTime, sizeof(fAccumTime) );
				XUI::CXUI_Wnd* pkKillerWnd = pkNewItem->m_pWnd->GetControl(L"FRM_KILLER");
				if( pkKillerWnd )
				{
					pkKillerWnd->Text( (( kKillerName.empty() )? std::wstring(): kKillerName) );
				}

				XUI::CXUI_Wnd* pkIconWnd = pkNewItem->m_pWnd->GetControl(L"FRM_ICON");
				if( pkIconWnd )
				{
					SUVInfo kUVInfo = pkIconWnd->UVInfo();
					kUVInfo.Index = (kKillerName.empty())? 2: 1;
					pkIconWnd->UVInfo(kUVInfo);
				}

				XUI::CXUI_Wnd* pkVictimWnd = pkNewItem->m_pWnd->GetControl(L"FRM_VICTIM");
				if( pkVictimWnd )
				{
					pkVictimWnd->Text( kDeadManName );
				}
			}
		}break;
	case PT_M_C_NFY_BS_DEATHCOUNT:
		{
			int iDeathCount = 0;
			rkPacket.Pop( iDeathCount );
			BattleSquareUI::CallDeathPenalty(iDeathCount);
		}break; 
	case PT_T_C_ANS_WANT_JOIN_BS_CHANNEL:
	case PT_M_C_ANS_WANT_JOIN_BS:
		{
			EBS_JOIN_RETURN eReturn = BSJR_SUCCESS;

			rkPacket.Pop( eReturn );

			//
			int iTTW = 0;
			switch( eReturn )
			{
			case BSJR_SUCCESS: // 입장 성공
				{
					iTTW = 0;
					if( g_pkWorld 
						&& PT_M_C_ANS_WANT_JOIN_BS == wPacketType 
						)
					{
						bool const bHaveFreePVPAttr = (0 != (g_pkWorld->DynamicGndAttr()&DGATTR_FLAG_FREEPVP));
						BattleSquareUI::PgBSTeamUI kTeamUI(m_kRedTeam, m_kBlueTeam, m_kBSGame, bHaveFreePVPAttr);

						kTeamUI.Close();

						PgActor* pkMyActor = g_kPilotMan.GetPlayerActor();
						if( pkMyActor )
						{
							pkMyActor->EquipAllItem();
						}

						XUI::CXUI_Wnd* pkRedPoint = XUIMgr.Activate(L"FRM_BS_TEAM_RED_POINT");
						if(pkRedPoint)
						{
							XUI::CXUI_Wnd* pkRedText = pkRedPoint->GetControl(L"FRM_POINT");
							if(pkRedText)
							{
								pkRedText->Text( BM::vstring(m_kRedTeam.TeamPoint() ) );
							}
						}

						XUI::CXUI_Wnd* pkBluePoint = XUIMgr.Activate(L"FRM_BS_TEAM_BLUE_POINT");
						if(pkBluePoint)
						{
							XUI::CXUI_Wnd* pkBlueText = pkBluePoint->GetControl(L"FRM_POINT");
							if(pkBlueText)
							{
								pkBlueText->Text( BM::vstring(m_kBlueTeam.TeamPoint() ) );
							}
						}
					}
					g_kPilotMan.UnlockPlayerInput(SReqPlayerLock(EPLT_BattleSquare, true)); // 움직임 해제
					g_kPilotMan.UnlockPlayerInput(SReqPlayerLock(EPLT_BattleSquare, false));
				}break;
			case BSJR_LEVEL: // 입장 레벨이 안되
				{
					iTTW = 460018;
				}break;
			case BSJR_BALANCE:
				{
					iTTW = 460079;
				}break;
			case BSJR_DUPLICATE:
			case BSJR_NORESERVE: // 예약되지 않은 사람이야
			case BSJR_MAX: // 인원이 꽉 찼어
				{
					if( PT_M_C_ANS_WANT_JOIN_BS == wPacketType )
					{
						iTTW = 460092;
					}
					else
					{
						iTTW = 460020;
					}
				}break;
			//case BSJR_GOLD: // 돈이 부족해
			//	{
			//		iTTW = 460017;
			//	}break;
			case BSJR_PARTY: // 파티 상태는 안되
				{
					iTTW = 460035;
				}break;
			case BSJR_NOTOPEN: // 입장 불가능 상태야
				{
					iTTW = 460019;
				}break;
			default:
				{
				}break;
			}

			//
			if( 0 != iTTW )
			{
				g_kChatMgrClient.AddMessage(iTTW, SChatLog(CT_EVENT), true);
			}
		}break;
	//case PT_T_C_ANS_BS_CHANNER_INFO:
	//	{
	//		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	//		if( !pkPlayer )
	//		{
	//			break;
	//		}
	//		BM::GUID kNpcGuid;
	//		size_t iCount = 0;
	//		rkPacket.Pop( kNpcGuid );
	//		rkPacket.Pop( iCount );
	//		CONT_BS_CLIENT_INFO kChannelList;
	//		for( size_t iCur = 0; iCount > iCur; ++iCur )
	//		{
	//			CONT_BS_CLIENT_INFO::value_type const kNewElement(rkPacket);
	//			kChannelList.push_back( kNewElement );
	//		}
	//		kChannelList.sort();
	//	}break;
	case PT_M_C_NFY_BS_REWARD:
		{
			m_kInGameNotice.Enable(false);

			CONT_BS_USER_REWARD kContWinTeamBonusReward;
			CONT_BS_REWARD kContReward;

			m_kRedTeam.ReadFromPacket(rkPacket);
			m_kBlueTeam.ReadFromPacket(rkPacket);
			PU::TLoadArray_A(rkPacket, kContReward);
			PU::TLoadTable_AA(rkPacket, kContWinTeamBonusReward);

			EBattleSquareTeam const eMyTeam = BattleSquareUI::GetMyTeam(m_kRedTeam, m_kBlueTeam);
			bool bWin = false;
			switch( eMyTeam )
			{
			case BST_RED:		{ bWin = m_kRedTeam.TeamPoint() > m_kBlueTeam.TeamPoint(); }break;
			case BST_BLUE:		{ bWin = m_kBlueTeam.TeamPoint() > m_kRedTeam.TeamPoint(); }break;
			default:
				{
				}break;
			}
			bool const bDraw = m_kRedTeam.TeamPoint() == m_kBlueTeam.TeamPoint();

			SBSTeamMember kMyMemberInfo;
			BattleSquareUI::GetMyMemberInfo(m_kRedTeam, m_kBlueTeam, kMyMemberInfo);

			lua_tinker::call< void >("ClearBSReward");
			
			typedef std::map< std::wstring, std::wstring > CONT_ITEM_MESSAGE;
			CONT_ITEM_MESSAGE kContItemMsg;

			CONT_BS_USER_REWARD::const_iterator iter = kContWinTeamBonusReward.begin();
			while( kContWinTeamBonusReward.end() != iter )
			{
				CONT_BS_USER_REWARD::key_type const& rkKey = (*iter).first;
				SBSTeamMember kMemberInfo;
				if( BattleSquareUI::GetMemberInfo(rkKey, m_kRedTeam, m_kBlueTeam, kMemberInfo) )
				{
					CONT_BS_USER_REWARD::mapped_type const& rkContReward = (*iter).second;
					CONT_BS_USER_REWARD::mapped_type::const_iterator item_iter = rkContReward.begin();
					while( rkContReward.end() != item_iter )
					{
						int const iItemNo = (*item_iter).iItemNo;
						wchar_t const* pkItemName = NULL;
						if( GetItemName(iItemNo, pkItemName) )
						{
							std::wstring const kItemName(pkItemName);
							CONT_ITEM_MESSAGE::iterator iter = kContItemMsg.find(kItemName);
							if( kContItemMsg.end() == iter )
							{
								std::pair<CONT_ITEM_MESSAGE::iterator, bool> kRet = kContItemMsg.insert( std::make_pair(kItemName, CONT_ITEM_MESSAGE::mapped_type()) );
								if( kRet.second )
								{
									iter = kRet.first;
								}
							}
							if( kContItemMsg.end() != iter )
							{
								if( (*iter).second.empty() )
								{
									(*iter).second += kMemberInfo.kCharName;
								}
								else
								{
									(*iter).second += std::wstring(L", ") + kMemberInfo.kCharName;
								}
							}
						}
						++item_iter;
					}
				}
				++iter;
			}
			int iCount = BattleSquareUI::iBSRewardStartIndex;
			CONT_ITEM_MESSAGE::const_iterator msg_iter = kContItemMsg.begin();
			/*	//시스템 공지 삭제 - 이미지를 가려서....
			while( kContItemMsg.end() != msg_iter )
			{
				std::wstring kTemp( TTW(460091) );
				PgStringUtil::ReplaceStr(kTemp, std::wstring(L"$Names$"), (*msg_iter).second, kTemp);
				PgStringUtil::ReplaceStr(kTemp, std::wstring(L"$ItemName$"), (*msg_iter).first, kTemp);
				lua_tinker::call< void, int, lwWString >("AddBSBonusRewardMsg", iCount, lwWString(kTemp));
				++iCount;
				++msg_iter;
			}
			*/
			iCount = BattleSquareUI::iBSRewardStartIndex;
			int const iTotalCount = static_cast< int >(kContReward.size());
			CONT_BS_REWARD::const_iterator item_iter = kContReward.begin();
			while( kContReward.end() != item_iter )
			{
				lua_tinker::call< void, int, int, int >("AddBSRewardItem", iCount, (*item_iter).iItemNo, (*item_iter).iCount, iTotalCount);
				++iCount;
				++item_iter;
			}

			BattleSquareUI::PgBSMainUI kMainUI(m_kRedTeam, m_kBlueTeam, m_kBSGame, false);
			kMainUI.Close(); // 메인 UI 닫기
			lua_tinker::call< void, bool, bool, unsigned short, unsigned short, int >("Call_BSGameEnd", bWin, bDraw, kMyMemberInfo.usKill, kMyMemberInfo.usDead, kMyMemberInfo.iPoint);		
		}break;
	case PT_N_C_NFY_BS_NOTICE:
		{
			size_t iCount = 0;
			CONT_BS_GAMES_STATUS_INFO kContNfyInfo;
			bool bNotice = false;

			rkPacket.Pop( iCount );
			for( size_t iCur = 0; iCount > iCur; ++iCur )
			{
				CONT_BS_GAMES_STATUS_INFO::value_type kNewNfyInfo;
				kNewNfyInfo.ReadFromPacket(rkPacket);
				kContNfyInfo.push_back( kNewNfyInfo );
			}
			rkPacket.Pop( bNotice );

			CONT_BS_GAMES_STATUS_INFO::const_iterator iter = kContNfyInfo.begin();
			while( kContNfyInfo.end() != iter )
			{
				CONT_BS_GAMES_STATUS_INFO::iterator find_iter = std::find(m_kContNoticeInfo.begin(), m_kContNoticeInfo.end(), (*iter));
				if( m_kContNoticeInfo.end() != find_iter )
				{
					(*find_iter) = (*iter);
				}
				else
				{
					m_kContNoticeInfo.push_back(*iter);
				}
				++iter;
			}

			if( bNotice )
			{
				ProcessNfyInfo(kContNfyInfo);	//배틀스퀘어 공지
			}
		}break;
	case PT_M_C_NFY_BS_ITEM_USER_POS:
		{
			g_kUIScene.SetRefreshAllMiniMap();

			CONT_BS_ITEM_UNIT_POS kTemp;
			size_t iCount = 0;

			rkPacket.Pop( iCount );
			for( size_t iCur = 0; iCount > iCur; ++iCur )
			{
				CONT_BS_ITEM_UNIT_POS::value_type const kNewUnitPos(rkPacket);
				kTemp.push_back( kNewUnitPos );
			}
			m_kContItemUnitPos.swap( kTemp );
		}break;
	case PT_M_C_NFY_WANT_JOIN_BS:
		{
			if(g_pkWorld)
			{
				PgBSTeam const kTempRed(m_kRedTeam);
				PgBSTeam const kTempBlue(m_kBlueTeam);
				m_kRedTeam.ReadFromPacket(rkPacket);
				m_kBlueTeam.ReadFromPacket(rkPacket);

				PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
				BM::GUID const kMyPlayerGuid = (pkPlayer)? pkPlayer->GetID(): BM::GUID::NullData();
				BattleSquareUtil::AddObserver(m_kRedTeam, kMyPlayerGuid);
				BattleSquareUtil::AddObserver(m_kBlueTeam, kMyPlayerGuid);
				BattleSquareUtil::DelObserver(kTempRed, m_kRedTeam);
				BattleSquareUtil::DelObserver(kTempBlue, m_kBlueTeam);
				BattleSquareUtil::UpdateNameEquip(m_kRedTeam);
				BattleSquareUtil::UpdateNameEquip(m_kBlueTeam);

				//
				bool const bHaveFreePVPAttr = (0 != (g_pkWorld->DynamicGndAttr()&DGATTR_FLAG_FREEPVP));
				BattleSquareUI::PgBSTeamUI kTeamUI(m_kRedTeam, m_kBlueTeam, m_kBSGame, bHaveFreePVPAttr);
				kTeamUI.Update();
			}
		}break;
	case PT_M_C_NFY_BS_MONSTER_POS:
		{
			g_kUIScene.SetRefreshAllMiniMap();
			size_t iCount = 0;
			rkPacket.Pop( iCount );
			CONT_BS_MEDAL_POS kContTemp;
			for( size_t iCur = 0; iCount > iCur; ++iCur )
			{
				BM::GUID kCharGuid;
				rkPacket.Pop( kCharGuid );
				SBSMedalPos kMedalPos;
				CONT_BS_MEDAL_POS::mapped_type kTemp(rkPacket);
				std::pair<CONT_BS_MEDAL_POS::iterator, bool> kRet = kContTemp.insert( std::make_pair(kCharGuid, kTemp) );
			}
			m_kContMedalPos.swap( kContTemp );
		}break;
	case PT_T_C_ANS_BS_CHANNEL_INFO:
		{
			UpdateBattleSquareChannel(rkPacket);
		}break;
	case PT_M_C_NFY_BS_SCORE:
		{
			SBSTeamMember kMyInfoOld;
			BattleSquareUI::GetMyMemberInfo(m_kRedTeam, m_kBlueTeam, kMyInfoOld);
			const int iRedPointOld = m_kRedTeam.TeamPoint();
			const int iBluePointOld = m_kBlueTeam.TeamPoint();
			// 정보 갱신
			m_kRedTeam.ReadFromScorePacket(rkPacket);
			m_kBlueTeam.ReadFromScorePacket(rkPacket);
			
			const int iRedPointNew = m_kRedTeam.TeamPoint();
			const int iBluePointNew = m_kBlueTeam.TeamPoint();

			SBSTeamMember kMyInfoNew;
			BattleSquareUI::GetMyMemberInfo(m_kRedTeam, m_kBlueTeam, kMyInfoNew);

			//Point
			int const iMyPoint =  kMyInfoNew.iPoint - kMyInfoOld.iPoint;
			int const iTeamRedPoint_Compare = iRedPointNew - iRedPointOld;
			int const iTeamBluePoint_Compare = iBluePointNew - iBluePointOld;
			//Medal
			int const iMyMedal = kMyInfoNew.usIconCount;
			int const iTeamRedMedal = m_kRedTeam.IconCount();
			int const iTeamBlueMedal = m_kBlueTeam.IconCount();

			//용자의 증표(팀/나)
			UpdateMedalCount(iMyMedal, iTeamRedMedal, iTeamBlueMedal);
			//포인트 이동
			UpdateMovePoint(iMyPoint, iRedPointNew, iBluePointNew);

			//승리중인 팀
			UpdateLeadTeam(iRedPointNew, iBluePointNew);
			
		}break;
	case PT_M_C_NFY_BS_UNIT_POS:
		{
			size_t iCount = 0;
			rkPacket.Pop( iCount );
			CONT_BS_UNIT_POS kContTemp;
			for( size_t iCur = 0; iCount > iCur; ++iCur )
			{
				BM::GUID kCharGuid;
				rkPacket.Pop( kCharGuid );
				CONT_BS_UNIT_POS::mapped_type kTemp(rkPacket);
				std::pair<CONT_BS_UNIT_POS::iterator, bool> kRet = kContTemp.insert( std::make_pair(kCharGuid, kTemp) ); // 일단 모두 저장
			}

			int const iMyTeamNo = BattleSquareUI::GetMyTeam(m_kRedTeam, m_kBlueTeam);
			CONT_BS_UNIT_POS::iterator iter = kContTemp.begin();
			while( kContTemp.end() != iter )
			{
				CONT_BS_UNIT_POS::mapped_type const& rkUnitPos = (*iter).second;
				bool bErase = true;
				if( 0 != iMyTeamNo )
				{
					if( iMyTeamNo != rkUnitPos.iTeamNo )
					{
						// 남팀
						CONT_BS_UNIT_POS::const_iterator check_iter = kContTemp.begin();
						while( kContTemp.end() != check_iter )
						{
							float const fLimitDistance = 250;
							CONT_BS_UNIT_POS::mapped_type const& rkCheckPos = (*check_iter).second;
							if( iMyTeamNo == rkCheckPos.iTeamNo
							&&	fLimitDistance > (rkUnitPos.kPos - rkCheckPos.kPos).Length() )
							{
								bErase = false; // 내 팀원과 일정 거리내면 보여준다
								break;
							}
							else if( 0 != rkCheckPos.sIconCount)
							{
								bErase = false;	// 거리가 멀어도 용자의 증표를 가지고 있으면 보여준다.
								break;
							}
							++check_iter;
						}
					}
					else
					{
						// 내 팀
						bErase = false;
					}
				}

				if( bErase )
				{
					iter = kContTemp.erase( iter );
				}
				else
				{
					++iter;
				}
			}
			m_kContUnitPos.swap( kContTemp );
		}
	default:
		{
		}break;
	}
}

void PgBattleSquareMng::ReadFromPacket(BM::Stream& rkPacket)
{
	__int64 iRemainTimeSec = 0;
	__int64 iRemainStartTimeSec = 0;

	rkPacket.Pop( iRemainStartTimeSec );
	rkPacket.Pop( iRemainTimeSec );
	m_kBSGame.ReadFromPacket(rkPacket);
	m_kRedTeam.ReadFromPacket( rkPacket );
	m_kBlueTeam.ReadFromPacket( rkPacket );

	if( g_pkWorld )
	{
		if( 0 != (g_pkWorld->DynamicGndAttr()&DGATTR_FLAG_FREEPVP) )
		{
			lua_tinker::call< void, int, bool >("CallBattleSquareTimer", static_cast< int >(iRemainTimeSec), true); // 시간 동기화
		}
		else
		{
			lua_tinker::call< void, int, bool >("CallBattleSquareTimer", static_cast< int >(iRemainStartTimeSec+1), false); // 시간 동기화
		}
	}
}
bool PgBattleSquareMng::GetCanEnterGameIDX(int const& iGameIDX) const
{
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkPlayer )
	{
		return false;
	}

	int const iPlayerLevel = pkPlayer->GetAbil(AT_LEVEL);
	CONT_BS_GAMES_STATUS_INFO::const_iterator iter = m_kContNoticeInfo.begin();
	while( m_kContNoticeInfo.end() != iter )
	{
		CONT_BS_GAMES_STATUS_INFO::value_type const& rkNfyInfo = (*iter);
		if( iGameIDX == rkNfyInfo.iGameIDX )
		{
			if( rkNfyInfo.iCurUserCount <= rkNfyInfo.iMaxUser )
			{
				//if( rkNfyInfo.iLevelMin <= iPlayerLevel
				//	&&	rkNfyInfo.iLevelMax >= iPlayerLevel )
				{
					switch( rkNfyInfo.eStatus )
					{
					case BSGS_NOW_PREOPEN:
					case BSGS_REQ_START:
					case BSGS_NOW_GAME:
						{
							return true; // 입장 가능
						}break;
					default:
						{
							// none
						}break;
					}
				}
			}
		}
		else if( 0 == iGameIDX)	//열려있는 채널이 있는지만 확인
		{
			//if( rkNfyInfo.iLevelMin <= iPlayerLevel
			//	&&	rkNfyInfo.iLevelMax >= iPlayerLevel )
			{
				switch( rkNfyInfo.eStatus )
				{
				case BSGS_NOW_PREOPEN:
				case BSGS_REQ_START:
				case BSGS_NOW_GAME:
					{
						return true; // 입장 가능
					}break;
				default:
					{
						// none
					}break;
				}
			}
		}
		++iter;
	}
	return false;
}
bool PgBattleSquareMng::IsEnterGameIDX(int& iGameIDX) const
{
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkPlayer )
	{
		return false;
	}

	int const iPlayerLevel = pkPlayer->GetAbil(AT_LEVEL);
	CONT_BS_GAMES_STATUS_INFO::const_iterator iter = m_kContNoticeInfo.begin();
	while( m_kContNoticeInfo.end() != iter )
	{
		CONT_BS_GAMES_STATUS_INFO::value_type const& rkNfyInfo = (*iter);
		if( iGameIDX = rkNfyInfo.iGameIDX )
		{
			if( rkNfyInfo.iLevelMin <= iPlayerLevel
				&&	rkNfyInfo.iLevelMax >= iPlayerLevel )
			{
				switch( rkNfyInfo.eStatus )
				{
				case BSGS_NOW_PREOPEN:
				case BSGS_REQ_START:
				case BSGS_NOW_GAME:
					{
						return true; // 입장 가능
					}break;
				default:
					{
						// none
					}break;
				}
			}
		}
		else
		{
			return false;
		}
		++iter;
	}
	return false;
}
void PgBattleSquareMng::ProcessNfyInfo(CONT_BS_GAMES_STATUS_INFO const& kContNfyInfo)
{
	if(!g_pkWorld)
	{
		return;
	}

	__int64 const iOneMinute = 60i64;
	CONT_BS_GAMES_STATUS_INFO::value_type kNfyInfo = kContNfyInfo.front();
	std::wstring kMsg, kTimeMsg;
	FormatTTW(kMsg, 460041, TTW(kNfyInfo.iChannelNameTextID).c_str());

	switch( kNfyInfo.eStatus )
	{
	case BSGS_WAIT_START:
		{
			if( -iOneMinute > kNfyInfo.iDiffTime )
			{
				FormatTTW(kTimeMsg, 460043, (-(kNfyInfo.iDiffTime / iOneMinute))+1); // 시작 x분 전입니다
			}
			else
			{
				FormatTTW(kTimeMsg, 460044, -kNfyInfo.iDiffTime); // 시작 x초 전입니다
			}
			if( g_pkWorld
				&&	g_pkWorld->IsHaveAttr(GATTR_FLAG_BATTLESQUARE) )
			{
				lua_tinker::call< void, int, bool >("CallBattleSquareTimer", static_cast< int >(-kNfyInfo.iDiffTime), false); // 시간 동기화
			}
		}break;
	case BSGS_NOW_GAME:
		{
			__int64 const iMinuteTime = kNfyInfo.iDiffTime / iOneMinute;
			if( 15 >= iMinuteTime )
			{
				if( iOneMinute < kNfyInfo.iDiffTime )
				{
					FormatTTW(kTimeMsg, 460047, iMinuteTime+1); // 종료 x분 전입니다
				}
				else
				{
					FormatTTW(kTimeMsg, 460048, kNfyInfo.iDiffTime); // 종료 x초 전입니다
				}
			}
			else
			{
				kTimeMsg = TTW(460046); // 진행 중입니다
			}
			
			if( g_pkWorld
			&&	0 != (g_pkWorld->DynamicGndAttr()&DGATTR_FLAG_FREEPVP) )
			{
				lua_tinker::call< void, int, bool >("CallBattleSquareTimer", static_cast< int >(kNfyInfo.iDiffTime), true); // 시간 동기화
			}
		}break;
	case BSGS_NOW_PREOPEN:
		{
			kNfyInfo.iDiffTime = 0;
			kMsg = TTW(460063); // [배틀 스퀘어]
			kTimeMsg = TTW(460042); // 개방 되었습니다
		}break;
	case BSGS_REQ_START:
		{
			kNfyInfo.iDiffTime = 0;
			kMsg = TTW(460063); // [배틀 스퀘어]
			kTimeMsg = TTW(460045); // 시작 되었습니다
		}break;
	case BSGS_REQ_END:
		{
//			kNfyInfo.iDiffTime = 0;
//			kMsg = TTW(460063); // [배틀 스퀘어]
//			kTimeMsg = TTW(460049); // 종료 되었습니다
			return;
		}break;
	}

	kMsg = kMsg + kTimeMsg;
	g_kChatMgrClient.AddLogMessage(SChatLog(CT_EVENT), kMsg, true, 4);

	lua_tinker::call< void, int >("Check_BSTimeCount", static_cast< int >(kNfyInfo.iDiffTime));

	{
		int const iGameIDX = kNfyInfo.iGameIDX;
		if( !g_pkWorld->IsHaveAttr(GATTR_FLAG_PUBLIC_CHANNEL)
		&&	!g_pkWorld->IsHaveAttr(GATTR_MISSION)
		&&	GetCanEnterGameIDX(iGameIDX) ) // 입장 가능하면 PVP 버튼을 깜빡 거리게 만든다
		{
			XUI::CXUI_Wnd *pkSysMenuWnd = XUIMgr.Get( std::wstring(L"SFRM_SYSTEMMENU") );
			if( pkSysMenuWnd )
			{
				XUI::CXUI_Wnd* pkPvpBtnWnd = pkSysMenuWnd->GetControl( std::wstring(L"BTN_PVP") );
				if( pkPvpBtnWnd )
				{
					DWORD const dwTwinkleTime = 180000;	//3분간 깜빡거리도록
					DWORD const dwTwinkleInter = 100;
					pkPvpBtnWnd->SetTwinkle(true);
					pkPvpBtnWnd->TwinkleTime(dwTwinkleTime);
					pkPvpBtnWnd->TwinkleInterTime(dwTwinkleInter);
				}
			}
		}
	}
}
void PgBattleSquareMng::CallTeamUI()
{
	if(!g_pkWorld)
	{
		return;
	}
	bool const bHaveFreePVPAttr = (0 != (g_pkWorld->DynamicGndAttr()&DGATTR_FLAG_FREEPVP));
	BattleSquareUI::PgBSTeamUI kTeamUI(m_kRedTeam, m_kBlueTeam, m_kBSGame, bHaveFreePVPAttr);
	kTeamUI.Call();
}
void PgBattleSquareMng::FlipTeamUI()
{
	if(!g_pkWorld)
	{
		return;
	}
	bool const bHaveFreePVPAttr = (0 != (g_pkWorld->DynamicGndAttr()&DGATTR_FLAG_FREEPVP));
	BattleSquareUI::PgBSTeamUI kTeamUI(m_kRedTeam, m_kBlueTeam, m_kBSGame, bHaveFreePVPAttr);
	bool const bHaveTeamUI = kTeamUI.IsUIOn();
	if( bHaveTeamUI )
	{
		kTeamUI.Close();
	}
	else
	{
		kTeamUI.Call();
	}
}
int PgBattleSquareMng::GetTeam(BM::GUID const& rkCharGuid) const
{
	return BattleSquareUI::GetTeam(rkCharGuid, m_kRedTeam, m_kBlueTeam);
}
void PgBattleSquareMng::Init()
{
	m_kInGameNotice.Init();
	lua_tinker::call< void >( "InitBSNotice" );
}
void PgBattleSquareMng::AddNotice(int const iTextTableNo)
{
	m_kInGameNotice.AddNotice( iTextTableNo );
}
void PgBattleSquareMng::RegistMemberTargetObserver()
{
	PgPlayer* pkMyPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkMyPlayer )
	{
		return;
	}
	BattleSquareUtil::AddObserver(m_kRedTeam, pkMyPlayer->GetID());
	BattleSquareUtil::AddObserver(m_kBlueTeam, pkMyPlayer->GetID());
}

void PgBattleSquareMng::SetAnimateWnd(XUI::CXUI_Wnd*& pkAnimateWnd, std::wstring& strOriginal, int const iLimit)
{
	int iNum = 0;
	BM::vstring strClone(strOriginal);
	while( !pkAnimateWnd || iLimit > iNum)
	{
		strClone = strOriginal;
		strClone += iNum;

		pkAnimateWnd = XUIMgr.Get(strClone);
		if( !pkAnimateWnd )
		{
			pkAnimateWnd = XUIMgr.Call(strOriginal, false, strClone);
			if(pkAnimateWnd)
			{
				return;
			}
			else //복사호출 실패
			{
			}
		}
		++iNum;
	}
}

void PgBattleSquareMng::UpdateBattleSquareChannel(BM::Stream& rkPacket)
{
	// Update UI
	XUI::CXUI_Wnd *pkTopWnd = XUIMgr.Call( std::wstring(L"FRM_BATTLE_SQUARE_JOIN") );
	if( !pkTopWnd )
	{
		return ;
	}
	lwRegistUIAction(lwUIWnd(pkTopWnd), "CloseUI");

	XUI::CXUI_List* pkListWnd = dynamic_cast< XUI::CXUI_List* >(pkTopWnd->GetControl(std::wstring(L"LST_CHANNEL")));
	if( !pkListWnd )
	{
		return ;
	}

	pkListWnd->ClearList();

	size_t iCount = 0;
	rkPacket.Pop(iCount);

	for( size_t iCur = 0; iCount > iCur; ++iCur )
	{
		EBS_GAME_STATUS eStatus;
		rkPacket.Pop(eStatus);
		int iCurUserCount = 0;
		rkPacket.Pop(iCurUserCount);
		SBSGame kTemp;
		kTemp.ReadFromPacket(rkPacket);

		XUI::SListItem* pkNewItem = pkListWnd->AddItem( std::wstring() );
		if( pkNewItem )
		{
			XUI::CXUI_Wnd* pkItemWnd = pkNewItem->m_pWnd;
			if( pkItemWnd )
			{
				bool bHide = false;
				{
					XUI::CXUI_Wnd* pkNameWnd = pkItemWnd->GetControl( std::wstring(L"SFRM_CHANNEL_BG") );
					if( pkNameWnd )
					{
						BM::vstring kTempStr;
						switch( eStatus )
						{
						case BSGS_NOW_PREOPEN:
						case BSGS_NOW_GAME:
							{
								kTempStr = BM::vstring(TTW(kTemp.iChannelNameTextID));
							}break;
						default:
							{
								kTempStr = TTW(kTemp.iChannelNameTextID);
							}break;
						}

#ifndef USE_INB
						if( g_pkApp->VisibleClassNo() )
						{
							kTempStr += L" (I:";
							kTempStr += kTemp.iGameIDX;
							kTempStr += L")";
						}
#endif

						pkNameWnd->Text( kTempStr );
					}
				}
				{
					XUI::CXUI_Wnd* pkLvWnd = pkItemWnd->GetControl( std::wstring(L"SFRM_JOINLV_BG") );
					if( pkLvWnd )
					{
						std::wstring kTempStr;
						if( FormatTTW(kTempStr, 460022, kTemp.iLevelMin, kTemp.iLevelMax) )
						{
							pkLvWnd->Text( kTempStr );
						}
					}
				}
				{
					XUI::CXUI_Wnd* pkMemberWnd = pkItemWnd->GetControl( std::wstring(L"SFRM_MEMBER_BG") );
					if( pkMemberWnd )
					{
						BM::vstring kTempStr;
						kTempStr = BM::vstring(L"(") + BM::vstring(iCurUserCount) + BM::vstring(L"/") + BM::vstring(kTemp.iMaxUser) + BM::vstring(L")");
						pkMemberWnd->Text( kTempStr );
					}
				}
				{
					XUI::CXUI_Wnd* pkEventWnd = pkItemWnd->GetControl( std::wstring(L"BTN_JOIN") );
					if( pkEventWnd )
					{
						int const iPreEnter = 460040;
						int const iCantEnter = 460016;
						int const iCanEnter = 460015;
						bool bEnable = false;
						int iButtonText = 0;
						switch( eStatus )
						{
						case BSGS_NOW_PREOPEN:
						case BSGS_NOW_GAME:
							{
								if( iCurUserCount < kTemp.iMaxUser )
								{
									iButtonText = (BSGS_NOW_PREOPEN == eStatus)? iPreEnter: iCanEnter;
									bEnable = true;
								}
								else
								{
									iButtonText = iCantEnter;
									bEnable = false;
								}
							}break;
						default:
						case BSGS_NOW_END:
							{
								iButtonText = iCantEnter;
								bEnable = false;
								bHide = true;
							}break;
						}

						PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
						int const iMyLevel = pkPlayer->GetAbil(AT_LEVEL);
						if( iMyLevel > kTemp.iLevelMax
							||	iMyLevel < kTemp.iLevelMin ) // 레벨 안되면 입장 불가 표시
						{
							bEnable = false;
							iButtonText = iCantEnter;
							//bHide = true;
						}

						pkEventWnd->Text( TTW(iButtonText) );
						pkEventWnd->Enable( bEnable );

						pkEventWnd->SetCustomData(&kTemp.iGameIDX, sizeof(kTemp.iGameIDX));
					}
				}
				if (bHide)
				{
					pkListWnd->DeleteItem(pkNewItem);
				}
			}
		}
	}
}

void PgBattleSquareMng::UpdateMedalCount(int const iMyMedal, int const iTeamRedMedal, int const iTeamBlueMedal) const
{
	//Team
	XUI::CXUI_Wnd* pkRedMedal = XUIMgr.Activate(L"FRM_BS_TEAM_RED_MEDAL");
	pkRedMedal->SetCustomData(&iTeamRedMedal, sizeof(iTeamRedMedal) );
	XUI::CXUI_Wnd* pkBlueMedal = XUIMgr.Activate(L"FRM_BS_TEAM_BLUE_MEDAL");
	pkBlueMedal->SetCustomData(&iTeamBlueMedal, sizeof(iTeamBlueMedal) );

	//Me
	XUI::CXUI_Wnd* pkMyPointWnd = XUIMgr.Get(L"FRM_BS_MY_POINT");
	if(!pkMyPointWnd)
	{
		return;
	}
	XUI::CXUI_Wnd* pkMyMedalText = pkMyPointWnd->GetControl(L"FRM_MEDAL");
	if(pkMyMedalText)
	{
		wchar_t szTemp[MAX_PATH] = {0,};
		swprintf(szTemp, MAX_PATH, _T("%d"), iMyMedal);
		pkMyMedalText->Text(szTemp);
	}
}

void PgBattleSquareMng::UpdateMovePoint(int const iMyPoint, int const iTeamRedPoint, int const iTeamBluePoint)
{
	//--Init
	//기본 이동포인트UI
	std::wstring strMyMovePoint(L"FORM_BS_MY_MOVE_POINT");
	std::wstring strTeamRedMovePoint = L"FORM_BS_RED_MOVE_POINT";
	std::wstring strTeamBlueMovePoint = L"FORM_BS_BLUE_MOVE_POINT";
	//이동할 위치UI
	std::wstring strMyPoint(L"FRM_BS_MY_POINT");
	std::wstring strTeamRedPoint = L"FRM_BS_TEAM_RED_POINT";
	std::wstring strTeamBluePoint = L"FRM_BS_TEAM_BLUE_POINT";

	std::wstring strTeamMovePoint;
	std::wstring strTeamPoint;
	std::wstring strTeamMovePoint_Other;
	std::wstring strTeamPoint_Other;

	int iTeamPoint = 0;
	int iTeamPoint_Other = 0;
	PgActor* pkPlayer = g_kPilotMan.GetPlayerActor();
	if(!pkPlayer)
	{
		return;
	}
	//--팀설정
	if( m_kRedTeam.IsTeamMember(pkPlayer->GetGuid()) )
	{
		strTeamPoint = strTeamRedPoint;
		strTeamMovePoint = strTeamRedMovePoint;
		iTeamPoint = iTeamRedPoint;

		strTeamPoint_Other = strTeamBluePoint;
		strTeamMovePoint_Other = strTeamBlueMovePoint;
		iTeamPoint_Other = iTeamBluePoint;
	}
	else if( m_kBlueTeam.IsTeamMember(pkPlayer->GetGuid()) )
	{
		strTeamPoint = strTeamBluePoint;
		strTeamMovePoint = strTeamBlueMovePoint;
		iTeamPoint = iTeamBluePoint;

		strTeamPoint_Other = strTeamRedPoint;
		strTeamMovePoint_Other = strTeamRedMovePoint;
		iTeamPoint_Other = iTeamRedPoint;
	}
	XUI::CXUI_Wnd* pkTeamPointWnd = XUIMgr.Get(strTeamPoint);
	XUI::CXUI_Wnd* pkTeamPointWnd_Other = XUIMgr.Get(strTeamPoint_Other);
	XUI::CXUI_Wnd* pkMyPointWnd = XUIMgr.Get(strMyPoint);
	//--값 설정
	if(pkTeamPointWnd && pkMyPointWnd && pkTeamPointWnd_Other)
	{
		POINT3I kLocStart;;
		XUI::CXUI_Wnd* pkMiniMapUI = XUIMgr.Get(L"SFRM_BIG_MAP");
		if(pkMiniMapUI)
		{
			kLocStart = pkMiniMapUI->GetTotalLocation();
		}
		POINT2 kStartPos;
		PgAlwaysMiniMap* pkMinimap = dynamic_cast<PgAlwaysMiniMap*>(g_kUIScene.GetMiniMapUI("MiniMap_Whole"));
		if(pkMinimap)
		{
			kStartPos = pkMinimap->ActorToMinimapScreenPos( pkPlayer, POINT2(kLocStart.x, kLocStart.y) );
		}
		wchar_t szTemp[MAX_PATH] = {0,};
		swprintf(szTemp, MAX_PATH, _T("%d"), iMyPoint);
		
		//내팀
		{
			XUI::CXUI_Wnd* pkMovePoint_TeamWnd = NULL;
			SetAnimateWnd(pkMovePoint_TeamWnd, strTeamMovePoint, 10);
			PgMoveAnimateWnd* pkMovePoint_Team = dynamic_cast<PgMoveAnimateWnd*>(pkMovePoint_TeamWnd);
			if( !pkMovePoint_Team )
			{
				return;
			}
			pkMovePoint_Team->SetCustomData(&iTeamPoint, sizeof(iTeamPoint));	
			XUI::CXUI_Wnd* pkTeamPointText = pkTeamPointWnd->GetControl(L"FRM_POINT");
			if(pkTeamPointText)
			{
				if( 0 < iMyPoint )
				{
					pkMovePoint_Team->Text(szTemp);
				}
				else
				{
					pkMovePoint_Team->Text(L"");
				}
				pkMovePoint_Team->StartPoint(NiPoint3(kStartPos.x, kStartPos.y, 0.0f));

				POINT2 kEndPos;
				POINT3I kLocation = pkTeamPointWnd->GetTotalLocation();
				kEndPos = POINT2( kLocation.x + pkTeamPointText->TextPos().x+5, kLocation.y + pkTeamPointText->TextPos().y+10);

				NiPoint3 kPoint1( (kStartPos.x+kEndPos.x)/2, (kStartPos.y+kEndPos.y)/2, 0.0f );

				pkMovePoint_Team->ModifyPoint1( kPoint1 );
				pkMovePoint_Team->EndPoint(NiPoint3(kEndPos.x, kEndPos.y, 0.0f));
				pkMovePoint_Team->StartTime(g_pkApp->GetAccumTime());
			}
		}
		//다른팀
		{
			XUI::CXUI_Wnd* pkMovePoint_TeamWnd_Other = NULL;
			SetAnimateWnd(pkMovePoint_TeamWnd_Other, strTeamMovePoint_Other, 10);
			PgMoveAnimateWnd* pkMovePoint_Team_Other = dynamic_cast<PgMoveAnimateWnd*>(pkMovePoint_TeamWnd_Other);
			if( !pkMovePoint_Team_Other )
			{
				return;
			}
			pkMovePoint_Team_Other->SetCustomData(&iTeamPoint_Other, sizeof(iTeamPoint_Other));
			XUI::CXUI_Wnd* pkTeamPointText_Other = pkTeamPointWnd->GetControl(L"FRM_POINT");
			if(pkTeamPointText_Other)
			{
				pkMovePoint_Team_Other->StartTime(g_pkApp->GetAccumTime());
			}
		}
		//나
		if( 0 < iMyPoint)
		{
			XUI::CXUI_Wnd* pkMovePoint_MyWnd = NULL;
			SetAnimateWnd(pkMovePoint_MyWnd, strMyMovePoint, 10);
			PgMoveAnimateWnd* pkMovePoint_My = dynamic_cast<PgMoveAnimateWnd*>(pkMovePoint_MyWnd);
			if( !pkMovePoint_My )
			{
				return;
			}
			XUI::CXUI_Wnd* pkMyPointText = pkMyPointWnd->GetControl(L"FRM_POINT");
			if(pkMyPointText)
			{
				pkMovePoint_My->Text(szTemp);
				pkMovePoint_My->StartPoint(NiPoint3(kStartPos.x, kStartPos.y, 0.0f));

				POINT2 kEndPos;
				POINT3I kLocation = pkMyPointWnd->GetTotalLocation();
				kEndPos = POINT2( kLocation.x + pkMyPointText->TextPos().x+5, kLocation.y + pkMyPointText->TextPos().y+10);

				NiPoint3 kPoint1( (kStartPos.x+kEndPos.x)/2, (kStartPos.y+kEndPos.y)/2, 0.0f );

				pkMovePoint_My->ModifyPoint1( kPoint1 );
				pkMovePoint_My->EndPoint(NiPoint3(kEndPos.x, kEndPos.y, 0.0f));
				pkMovePoint_My->StartTime(g_pkApp->GetAccumTime());

				SBSTeamMember kMyInfoNew;
				BattleSquareUI::GetMyMemberInfo(m_kRedTeam, m_kBlueTeam, kMyInfoNew);
				int const iPoint = kMyInfoNew.iPoint;
				pkMovePoint_My->SetCustomData(&iPoint, sizeof(iPoint));
			}
		}
	}
}

void PgBattleSquareMng::UpdateLeadTeam(int const iTeamRedPoint, int const iTeamBluePoint)
{
	E_LeadTeam eLeadTeam = E_LEAD_PARE;
	if(iTeamRedPoint > iTeamBluePoint)
	{
		eLeadTeam = E_LEAD_RED;
	}
	else if(iTeamRedPoint < iTeamBluePoint)
	{
		eLeadTeam = E_LEAD_BLUE;
	}
	else if(iTeamRedPoint == iTeamBluePoint)
	{
		eLeadTeam = E_LEAD_PARE;
	}

	if( m_eLeadTeam == eLeadTeam && E_LEAD_PARE != eLeadTeam )
	{
		return;
	}

	switch( eLeadTeam )
	{
	case E_LEAD_RED:
		{
			XUI::CXUI_Wnd* pkTeamBlueWnd = XUIMgr.Get(L"FRM_BS_TEAM_BLUE_POINT");
			if(!pkTeamBlueWnd)
			{
				return;
			}
			XUI::CXUI_Wnd* pkTeamBlueLeadWnd = pkTeamBlueWnd->GetControl(L"FRM_WIN");
			if(!pkTeamBlueLeadWnd)
			{
				return;
			}
			pkTeamBlueLeadWnd->Visible(false);

			XUI::CXUI_Wnd* pkTeamRedWnd = XUIMgr.Get(L"FRM_BS_TEAM_RED_POINT");
			if(!pkTeamRedWnd)
			{
				return;
			}
			XUI::CXUI_Wnd* pkTeamRedLeadWnd = pkTeamRedWnd->GetControl(L"FRM_WIN");
			if(!pkTeamRedWnd)
			{
				return;
			}
			pkTeamRedLeadWnd->Visible(true);
//			XUIMgr.Call(L"FRM_BS_TEAM_RED_POINT");
//			XUIMgr.Call(L"FRM_BS_TEAM_RED_POINT_ANI");

		}break;
	case E_LEAD_BLUE:
		{
			XUI::CXUI_Wnd* pkTeamRedWnd = XUIMgr.Get(L"FRM_BS_TEAM_RED_POINT");
			if(!pkTeamRedWnd)
			{
				return;
			}
			XUI::CXUI_Wnd* pkTeamRedLeadWnd = pkTeamRedWnd->GetControl(L"FRM_WIN");
			if(!pkTeamRedWnd)
			{
				return;
			}
			pkTeamRedLeadWnd->Visible(false);
			
			XUI::CXUI_Wnd* pkTeamBlueWnd = XUIMgr.Get(L"FRM_BS_TEAM_BLUE_POINT");
			if(!pkTeamBlueWnd)
			{
				return;
			}
			XUI::CXUI_Wnd* pkTeamBlueLeadWnd = pkTeamBlueWnd->GetControl(L"FRM_WIN");
			if(!pkTeamBlueLeadWnd)
			{
				return;
			}
			pkTeamBlueLeadWnd->Visible(true);
//			XUIMgr.Call(L"FRM_BS_TEAM_BLUE_POINT");
//			XUIMgr.Call(L"FRM_BS_TEAM_BLUE_POINT_ANI");

		}break;
	case E_LEAD_PARE:
		{
//			XUIMgr.Close(L"FRM_BS_TEAM_RED_POINT_ANI");
//			XUIMgr.Close(L"FRM_BS_TEAM_BLUE_POINT_ANI");

			XUI::CXUI_Wnd* pkTeamRedWnd = XUIMgr.Get(L"FRM_BS_TEAM_RED_POINT");
			if(!pkTeamRedWnd)
			{
				return;
			}
			XUI::CXUI_Wnd* pkTeamRedLeadWnd = pkTeamRedWnd->GetControl(L"FRM_WIN");
			if(!pkTeamRedWnd)
			{
				return;
			}
			pkTeamRedLeadWnd->Visible(false);

			XUI::CXUI_Wnd* pkTeamBlueWnd = XUIMgr.Get(L"FRM_BS_TEAM_BLUE_POINT");
			if(!pkTeamBlueWnd)
			{
				return;
			}
			XUI::CXUI_Wnd* pkTeamBlueLeadWnd = pkTeamBlueWnd->GetControl(L"FRM_WIN");
			if(!pkTeamBlueLeadWnd)
			{
				return;
			}
			pkTeamBlueLeadWnd->Visible(false);
		}break;
	default:
		{
		}break;
	}

	m_eLeadTeam = eLeadTeam;
}