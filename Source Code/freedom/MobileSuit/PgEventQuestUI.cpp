#include "stdafx.h"
#include "Variant/PgQuestInfo.h"
#include "Variant/PgEventQuestBase.h"
#include "Variant/PgStringUtil.h"
#include "ServerLib.h"
#include "PgPilotMan.h"
#include "PgQuestMan.h"
#include "PgNetwork.h"
#include "PgEventQuestUI.h"
#include "lwUI.h"
#include "Pg2DString.h"
#include "lwUIQuest.h"
#include "PgQuest.h"
#include "PgChatMgrClient.h"

extern bool FormatMoney(__int64 const iTotal, std::wstring &rkOut);

namespace PgEventQuestUI
{
	int const iCompleteToolTip = 450225;

	//
	class PgEventQuestStatusChecker
	{
		typedef std::map< int, int > ContCheck;
	public:
		PgEventQuestStatusChecker()
		{
		}
		~PgEventQuestStatusChecker()
		{
		}

		void Update(ContEventQuestTarget const& rkEventTarget, std::wstring const rkTitle)
		{
			bool const bOldComplete = IsComplete(rkEventTarget);

			ContCheck const kOld = m_kStatus;
			UpdateMine(rkEventTarget);

			ContCheck::const_iterator iter = m_kStatus.begin();
			while( m_kStatus.end() != iter )
			{
				int const iItemNo = (*iter).first;
				int const iCurCount = (*iter).second;
				ContCheck::const_iterator find_iter = kOld.find( iItemNo );
				if( kOld.end() != find_iter
				&&	(*find_iter).second != iCurCount )
				{
					ContEventQuestTarget::const_iterator target_iter = std::find(rkEventTarget.begin(), rkEventTarget.end(), iItemNo);
					if( rkEventTarget.end() != target_iter )
					{
						int const iTargetCount = (*target_iter).iCount;
						if( iTargetCount > iCurCount )
						{
							wchar_t const* pkItemName = NULL;
							if( ::GetItemName(iItemNo, pkItemName) )
							{
								g_kQuestMan.ShowQuestInfo(QS_Ing, std::wstring(pkItemName), iCurCount, iTargetCount);
							}
						}
					}
				}
				++iter;
			}

			if( !bOldComplete
			&&	IsComplete(rkEventTarget) )
			{
				PgActor* pkActor = g_kPilotMan.GetPlayerActor();
				if( pkActor )
				{
					pkActor->PlayNewSound(NiAudioSource::TYPE_3D, "QUEST_Complete1", 0.0f);
				}
				g_kQuestMan.ShowQuestInfo(QS_End, rkTitle);
			}
		}
		void Clear()
		{
			m_kStatus.clear();
			m_kFinished = false;
		}
		void CallToopTip(XUI::CXUI_Wnd const* pkTopWnd, ContEventQuestTarget const& rkEventTarget)
		{
			PgMiniIngToolTip kToolTip;
			if( !m_kFinished )
			{
				ContCheck::const_iterator iter = m_kStatus.begin();
				while( m_kStatus.end() != iter )
				{
					int const iItemNo = (*iter).first;
					int const iCurCount = (*iter).second;

					ContEventQuestTarget::const_iterator find_iter = std::find(rkEventTarget.begin(), rkEventTarget.end(), iItemNo);
					if( rkEventTarget.end() != find_iter )
					{
						wchar_t const* pkItemName = NULL;
						if( ::GetItemName(iItemNo, pkItemName) )
						{
							kToolTip.Add(std::wstring(pkItemName), iCurCount, (*find_iter).iCount);
						}
					}
					++iter;
				}
			}
			else
			{
				kToolTip.Add( std::wstring(L"{C=0xFF00FF00/}") + TTW(iCompleteToolTip) );
			}
			kToolTip.Call(pkTopWnd);
		}

	protected:
		void UpdateMine(ContEventQuestTarget const& rkEventTarget)
		{
			m_kStatus.clear();
			PgPlayer const* pkPlayer = g_kPilotMan.GetPlayerUnit();
			if( !pkPlayer )
			{
				return;
			}

			PgInventory const* pkInven = pkPlayer->GetInven();
			if( !pkInven )
			{
				return;
			}

			ContEventQuestTarget::const_iterator iter = rkEventTarget.begin();
			while( rkEventTarget.end() != iter )
			{
				ContEventQuestTarget::value_type const &rkTargetItem = (*iter);

				int const iHaveCount = static_cast< int >(pkInven->GetTotalCount(rkTargetItem.iItemNo));
				std::pair<ContCheck::iterator, bool> kRet = m_kStatus.insert( std::make_pair(rkTargetItem.iItemNo, iHaveCount) );
				if( !kRet.second )
				{
					(*kRet.first).second = iHaveCount;
				}
				++iter;
			}
		}

		bool IsComplete(ContEventQuestTarget const& rkEventTarget) const
		{
			{
				ContEventQuestTarget::const_iterator iter = rkEventTarget.begin();
				while( rkEventTarget.end() != iter )
				{
					ContEventQuestTarget::value_type const& rkEventTarget = (*iter);

					ContCheck::const_iterator find_iter = m_kStatus.find( rkEventTarget.iItemNo );
					if( m_kStatus.end() != find_iter )
					{
						if( rkEventTarget.iCount > (*find_iter).second )
						{
							return false; // 완성 못했음
						}
					}
					else
					{
						return false; // 완성 못했음
					}
					++iter;
				}
			}

			return true;
		}

	private:
		ContCheck m_kStatus;
		CLASS_DECLARATION_S(bool, Finished);
	};


	//
	PgEventQuest kSavedEventQuest;
	PgEventQuestStatusChecker m_kStatus;

	//
	void ClearInfo()
	{
		kSavedEventQuest.Clear();
		m_kStatus.Clear();
	}
	void ReadFromPacket(BM::Stream& rkPacket)
	{
		bool bIncludeCompleteInfo = false;
		bool bFinished = false;

		rkPacket.Pop( bIncludeCompleteInfo );
		if( bIncludeCompleteInfo )
		{
			rkPacket.Pop( bFinished );
			m_kStatus.Finished(bFinished);
		}

		kSavedEventQuest.ReadFromClientPacket( rkPacket );
	}

	std::wstring const kTopFormName( L"FRM_EVENT_QUEST" );
	std::wstring const kTitleFormName( L"FRM_TITLE" );
	std::wstring const kPrologueFormName( L"LST_PROLOGUE" );
	std::wstring const kInfoFormName( L"LST_INFO" );
	std::wstring const kInfoRewardTitleName ( L"FRM_REWARD_TEXT" );
	BM::vstring const kRewardFormName( L"FRM_REWARD" );
	BM::vstring const kRewardFormBgName( L"FRM_REWARD_BG" );
	std::wstring const kMiniBtnName( L"BTN_EVENT_QUEST_MINI" );
	void UpdateXuiListText(XUI::CXUI_List* pkPrologueList, std::wstring const& rkString)
	{
		if( !pkPrologueList )
		{
			return;
		}

		if( rkString.empty() )
		{
			pkPrologueList->ClearList();
		}
		else
		{
			XUI::SListItem* pkInfo = pkPrologueList->FirstItem();
			if( !pkInfo )
			{
				pkInfo = pkPrologueList->AddItem( rkString );
			}
			
			if( pkInfo )
			{
				XUI::CXUI_Wnd* pkPrologueForm = pkInfo->m_pWnd;
				if( pkPrologueForm )
				{
					pkPrologueForm->Text( rkString );
					XUI::CXUI_Style_String kStyleString = pkPrologueForm->StyleText();
					POINT2 const kTextSize(Pg2DString::CalculateOnlySize(kStyleString));
					pkPrologueForm->Size(kTextSize);
					pkPrologueForm->Invalidate();
				}
			}
		}
	}
	void UpdateRewardIcon(XUI::CXUI_Wnd *pkTopWnd, size_t const iCur, SEventQuestItem const& rkItem)
	{
		if( !pkTopWnd )
		{
			return;
		}

		BM::vstring const kCur(iCur);
		BM::vstring const kImgWndName( kRewardFormName + kCur);
		XUI::CXUI_Wnd* pkImgWnd = pkTopWnd->GetControl( kImgWndName.operator const std::wstring&() );
		if( pkImgWnd )
		{
			if( !rkItem.IsEmpty() )
			{
				SQuestItem const kRewardItem(rkItem.iItemNo, rkItem.iCount);

				pkImgWnd->SetCustomData(&kRewardItem, sizeof(SQuestItem));
				Quest::lwOnCallQuestRewardItemImage( lwUIWnd(pkImgWnd) );

				bool bIsAmountItem = true;
				GET_DEF(CItemDefMgr, kItemDefMgr);

				CItemDef const* pkItemDef = kItemDefMgr.GetDef(rkItem.iItemNo);
				if( pkItemDef )
				{
					bIsAmountItem = pkItemDef->IsAmountItem();
				}

				if( 1 < rkItem.iCount
				&&	bIsAmountItem )
				{
					BM::vstring const kItemCount(rkItem.iCount);
					pkImgWnd->Text( std::wstring(kItemCount) );
				}
				else
				{
					pkImgWnd->Text( std::wstring() );
				}
			}
			pkImgWnd->Visible( !rkItem.IsEmpty() );
		}

		BM::vstring const kImgBGWndName( kRewardFormBgName + kCur);
		XUI::CXUI_Wnd* pkImgBgWnd = pkTopWnd->GetControl( kImgBGWndName.operator const std::wstring&() );
		if( pkImgBgWnd )
		{
			pkImgWnd->Visible( !rkItem.IsEmpty() );
		}
	}
	void UpdateUI()
	{
		if( m_kStatus.Finished() )
		{
			g_kChatMgrClient.AddLogMessage(SChatLog(CT_EVENT_SYSTEM), TTW(iCompleteToolTip), true, 2);
			return;
		}

		XUI::CXUI_Wnd* pkTopWnd = XUIMgr.Get( kTopFormName );
		if( !pkTopWnd )
		{
			pkTopWnd = XUIMgr.Call( kTopFormName );
			if( !pkTopWnd )
			{
				return;
			}
		}

		{ // 제목
			XUI::CXUI_Wnd* pkTitleWnd = pkTopWnd->GetControl( kTitleFormName );
			if( pkTitleWnd )
			{
				pkTitleWnd->Text( kSavedEventQuest.Title() );
			}
		}
		{ // 프롤로그
			XUI::CXUI_List* pkPrologueList = dynamic_cast< XUI::CXUI_List* >(pkTopWnd->GetControl( kPrologueFormName ));
			UpdateXuiListText(pkPrologueList, kSavedEventQuest.Prologue());
		}
		{ // 내용
			XUI::CXUI_List* pkInfoList = dynamic_cast< XUI::CXUI_List* >(pkTopWnd->GetControl( kInfoFormName ));
			UpdateXuiListText(pkInfoList, kSavedEventQuest.Info());
		}
		{ // 보상 골드
			XUI::CXUI_Wnd* pkRewardTitle = pkTopWnd->GetControl( kInfoRewardTitleName );
			if( pkRewardTitle )
			{
				int const iDefaultTTW = 19931;
				bool bRet = (0 != kSavedEventQuest.RewardGold());
				if( bRet )
				{
					std::wstring kTempStr;
					bRet = FormatMoney(kSavedEventQuest.RewardGold(), kTempStr);
					if( bRet )
					{
						pkRewardTitle->Text( (TTW(19931) + std::wstring(L" : ") + kTempStr) );
					}
				}
				
				if( !bRet )
				{
					pkRewardTitle->Text( TTW(iDefaultTTW) );
				}
			}
		}
		{ // 보상
			size_t iCur = 0; // 보상 UI 시작
			int const iMaxCount = EQE_MAX_REWARD_ITEM_COUNT;
			ContEventQuestReward const& rkRewardList = kSavedEventQuest.RewardItem();
			ContEventQuestReward::const_iterator iter = rkRewardList.begin();
			while( rkRewardList.end() != iter )
			{
				ContEventQuestReward::value_type const& rkItem = (*iter);
				UpdateRewardIcon(pkTopWnd, iCur, rkItem);
				++iCur;
				++iter;
			}
			for( ; iMaxCount > iCur; ++iCur )
			{
				UpdateRewardIcon(pkTopWnd, iCur, SEventQuestItem());
			}
		}
	}
	void CloseUI()
	{
		XUIMgr.Close( kTopFormName );
	}
	void CloseMini()
	{
		XUIMgr.Close( kMiniBtnName );
	}
	void CallMini()
	{
		if( g_pkWorld && g_pkWorld->IsHaveAttr(GATTR_FLAG_MYHOME) )
		{
			CloseMini();
		}
		else
		{
			XUIMgr.Call( kMiniBtnName );
		}
	}
	bool IsNowEvent()
	{
		PgEventQuest const& rkEvent = kSavedEventQuest;
		return !(rkEvent.Title().empty() && rkEvent.Info().empty() && rkEvent.Prologue().empty());
	}
	void CheckNowEvent()
	{
		if( m_kStatus.Finished() ) // 완료 했는가?
		{
			Complete();
			CallMini();
		}
		else
		{
			if( IsNowEvent() )
			{
				XUI::CXUI_Wnd* pkTopWnd = XUIMgr.Get( kTopFormName );
				if( pkTopWnd )
				{
					UpdateUI();
				}
				else
				{
					CallMini();
				}
			}
			else
			{
				CloseUI();
				CloseMini();
			}
		}
	}

	bool IsInTargetItem(int const iItemNo)
	{
		return kSavedEventQuest.TargetItem().end() != std::find(kSavedEventQuest.TargetItem().begin(), kSavedEventQuest.TargetItem().end(), iItemNo);
	}
	void CheckTargetItem()
	{
		if( !m_kStatus.Finished() )
		{
			m_kStatus.Update(kSavedEventQuest.TargetItem(), kSavedEventQuest.Title());
		}
	}
	void Complete()
	{
		m_kStatus.Finished(true);
		CloseUI();
		//CloseMini();
	}

	void CheckEventQuestEnd(lwGUID kNpcGuid)
	{
		BM::Stream kPacket(PT_C_M_REQ_EVENT_QUEST_TALK);
		kPacket.Push( kNpcGuid() );
		NETWORK_SEND(kPacket);
	}
	lwWString GetNpcTalkPrologue()		{ return lwWString(kSavedEventQuest.NpcTalkPrologue()); }
	lwWString GetNpcTalkNotEnd()		{ return lwWString(kSavedEventQuest.NpcTalkNotEnd()); }
	lwWString GetNpcTalkEnd()			{ return lwWString(kSavedEventQuest.NpcTalkEnd()); }
	void CallStatusToolTip(lwUIWnd kTopWnd)
	{
		m_kStatus.CallToopTip(kTopWnd(), kSavedEventQuest.TargetItem());
	}

	// Regist Lua
	bool RegisterWrapper(lua_State *pkState)
	{
		using namespace lua_tinker;
		//Quest
		def(pkState, "CallEventQuestInfoUI", UpdateUI);
		def(pkState, "CheckEventQuest", CheckNowEvent);
		def(pkState, "IsNowEventQuest", IsNowEvent);
		def(pkState, "GetEventQuest_NpcTalkPrologue", GetNpcTalkPrologue);
		def(pkState, "GetEventQuest_NpcTalkNotEnd", GetNpcTalkNotEnd);
		def(pkState, "GetEventQuest_NpcTalkEnd", GetNpcTalkEnd);
		def(pkState, "CheckEventQuestEnd", CheckEventQuestEnd);
		def(pkState, "CallEventQuest_StatusToolTip", CallStatusToolTip);
		return true;
	}
};