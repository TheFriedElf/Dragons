#include "stdafx.h"
#include "lwUI.h"
#include "PgMobileSuit.h"
#include "PgMToMChat.h"
#include "lwMToMChat.h"
#include "PgWorld.h"

int const MAX_MToM_CHAT_LIST = 30;
int const MAX_DLG_COUNT = 50;
//float const FLASH_UPDATE_TICK_TIME = 0.5f;
wchar_t const* const szDefault_SmallFormName = L"FRM_MTOM_CHAT_MINI";
wchar_t const* const szDefault_BigFormName = L"SFRM_MTOM_CHAT_DLG";
wchar_t const* const szDefault_QuestionName = L"SFRM_MTOM_QUESTION_DLG";

//-----------------------------------------------------------------------------------------
//	PgMToMChat
//-----------------------------------------------------------------------------------------
PgMToMChat::PgMToMChat()
:	m_kType(MTM_MSG_DEFAULT)
,	m_kArriveNewChat(true)
,	m_kIsDlgOpen(false)
{
//	m_kChatContainer.clear();
//	m_kMToMName = _T("");
//	m_kMToMGuid = BM::GUID::NullData();
}

PgMToMChat::PgMToMChat( BM::GUID const& kGuid, std::wstring const& kName, BYTE const kType )
:	m_kType(MTM_MSG_DEFAULT)
,	m_kArriveNewChat(true)
,	m_kIsDlgOpen(false)
{
	Init( kGuid, kName, kType );
}

PgMToMChat::~PgMToMChat()
{
	m_kChatContainer.clear();
}

void PgMToMChat::Init(BM::GUID const& Guid, std::wstring const& kName, BYTE const kType )
{
	m_kMToMName = kName;
	m_kMToMGuid = Guid;
	m_kType = kType;
}

void PgMToMChat::AddChatMsg( BYTE const kType, SMtoMChatData const& ChatData, std::wstring const& kID, bool const bIsOpen )
{
	m_kChatContainer.push_back(ChatData);
	while( m_kChatContainer.size() > MAX_MToM_CHAT_LIST )
	{
		m_kChatContainer.pop_front();
	}

	if( m_kIsDlgOpen )
	{
		//메세지 바로 추가
		XUI::CXUI_Wnd* pSelf = XUIMgr.Get(kID);
		if( pSelf )
		{
			lwUIWnd UISelf(pSelf);
			MToMChatUIListAdd(UISelf, m_kMToMGuid, ChatData);
		}
	}
	else
	{
		//윈도우 반짝 반짝
		m_kArriveNewChat = true;
		XUI::CXUI_Wnd *pkWnd = MToMChatNewDlgMini( m_kMToMGuid, kType );
		if ( pkWnd && bIsOpen )
		{
			lwMToMChatDlgStateChange( lwUIWnd(pkWnd) );
		}
	}
}

void PgMToMChat::DrawAllChatMsg(std::wstring const& kID)
{
	XUI::CXUI_Wnd* pWnd = XUIMgr.Get(kID);
	if( !pWnd )
	{
		return;
	}
	lwUIWnd	UISelf(pWnd);
	MToMChatUIListClear(UISelf);
	kChatContainer::iterator	iter = m_kChatContainer.begin();
	while( iter != m_kChatContainer.end() )
	{
		MToMChatUIListAdd(UISelf, m_kMToMGuid, (*iter));
		++iter;
	}

	m_kArriveNewChat = false;
}

void PgMToMChat::Show()
{
	m_kIsDlgOpen = true;
}

void PgMToMChat::Hide()
{
	m_kIsDlgOpen = false;
}

//-----------------------------------------------------------------------------------------
//	PgMToMChatMgr
//-----------------------------------------------------------------------------------------
PgMToMChatMgr::PgMToMChatMgr()
{
	m_kMToMCont.clear();
}

PgMToMChatMgr::~PgMToMChatMgr()
{
	m_kMToMCont.clear();
}

void PgMToMChatMgr::Add( BM::GUID const& Guid, std::wstring const& kName, BYTE const kType )
{
	kMToMContainer::iterator iter = m_kMToMCont.find(Guid);
	if( iter != m_kMToMCont.end() )
	{
		return;
	}

	PgMToMChat	NewChatDlg;
	std::pair<kMToMContainer::iterator, bool> Rst = m_kMToMCont.insert(std::make_pair(Guid, NewChatDlg));
	if( Rst.second )
	{
		Rst.first->second.Init( Guid, kName, kType);
	}
}

void PgMToMChatMgr::Add(BM::GUID const& Guid, std::wstring const& kName, BYTE const kType, SMtoMChatData const& ChatData, bool const bIsOpen )
{
	std::wstring kID;

	kMToMContainer::iterator	iter = m_kMToMCont.find(Guid);
	if( iter != m_kMToMCont.end() )
	{
		if( !FindID(iter->first, kID) )
		{
			return;
		}

		iter->second.Type( kType );
		iter->second.AddChatMsg( kType, ChatData, kID, bIsOpen );
		return;
	}

	PgMToMChat	NewChatDlg( Guid, kName, kType );
	std::pair<kMToMContainer::iterator, bool> Rst = m_kMToMCont.insert(std::make_pair(Guid, NewChatDlg));
	if( Rst.second )
	{
		if( !AddID(Guid, kID) )
		{
			return;
		}

		Rst.first->second.AddChatMsg( kType, ChatData, kID, bIsOpen );
	}
}

void PgMToMChatMgr::Add( BM::GUID const& Guid, SMtoMChatData const& ChatData )
{
	kMToMContainer::iterator	iter = m_kMToMCont.find(Guid);
	if( iter != m_kMToMCont.end() )
	{
		std::wstring kID;
		if( !FindID(iter->first, kID) )
		{
			return;
		}
		iter->second.AddChatMsg( iter->second.Type(), ChatData, kID, false );
	}
}

void PgMToMChatMgr::Del(BM::GUID const& Guid)
{
	kMToMContainer::iterator	iter = m_kMToMCont.find(Guid);
	if( iter == m_kMToMCont.end() )
	{
		// error
		return;
	}
	m_kMToMCont.erase(iter);

// 	kFlashTimeContainer::iterator c_iter = m_kTimeCont.find(Guid);
// 	if( c_iter == m_kTimeCont.end() )
// 	{
// 		return;
// 	}
// 	m_kTimeCont.erase(c_iter);
}

void PgMToMChatMgr::AddAllList(BM::GUID const& Guid)
{
	kMToMContainer::iterator	iter = m_kMToMCont.find(Guid);
	if( iter == m_kMToMCont.end() )
	{
		// error
		return;
	}

	std::wstring kID;
	if( FindID(Guid, kID) )
	{
		iter->second.DrawAllChatMsg(kID);
	}
}

bool PgMToMChatMgr::IsChatDlgState(BM::GUID const& Guid)
{
	kMToMContainer::iterator	iter = m_kMToMCont.find(Guid);
	if( iter == m_kMToMCont.end() )
	{
		// error
		return false;
	}
	return iter->second.IsDlgOpen();
}

bool PgMToMChatMgr::SetChatDlgState(BM::GUID const& Guid, bool const IsState)
{
	kMToMContainer::iterator	iter = m_kMToMCont.find(Guid);
	if( iter == m_kMToMCont.end() )
	{
		// error
		return false;
	}
	iter->second.IsDlgOpen(IsState);
	return true;
}

bool PgMToMChatMgr::IsArriveNew(BM::GUID const& Guid)
{
	kMToMContainer::iterator	iter = m_kMToMCont.find(Guid);
	if( iter == m_kMToMCont.end() )
	{
		return false;
	}

	return iter->second.ArriveNewChat();
}

bool PgMToMChatMgr::SetArriveNew(BM::GUID const& Guid, bool const IsNew)
{
	kMToMContainer::iterator	iter = m_kMToMCont.find(Guid);
	if( iter == m_kMToMCont.end() )
	{
		return false;
	}
	iter->second.ArriveNewChat(IsNew);
	return true;
}

/*
bool PgMToMChatMgr::FlashUpdate(BM::GUID const& Guid, int& NextUV)
{
	kMToMContainer::iterator	c_iter = m_kMToMCont.find(Guid);
	if( c_iter == m_kMToMCont.end() )
	{
		return false;
	}
	
	kFlashTimeContainer::iterator	iter = m_kTimeCont.find(Guid);
	if( iter == m_kTimeCont.end() )
	{
		SFlashUpdateData	Data;
		Data.bIsLight = false;
		Data.fTime = g_pkApp->GetAccumTime();
		std::pair<kFlashTimeContainer::iterator, bool> Rst = m_kTimeCont.insert(std::make_pair(Guid, Data));
		if( Rst.second )
		{
			NextUV = c_iter->second.GetRelationType();
			return true;
		}
	}
	else
	{
		if( iter->second.fTime < g_pkApp->GetAccumTime() )
		{
			iter->second.fTime = g_pkApp->GetAccumTime() + FLASH_UPDATE_TICK_TIME;
			iter->second.bIsLight = !iter->second.bIsLight;
			NextUV = c_iter->second.GetRelationType() + (BYTE)iter->second.bIsLight;
			return true;
		}
	}

	return false;
}
*/

bool PgMToMChatMgr::GetName(BM::GUID const& Guid, std::wstring& Name)const
{
	kMToMContainer::const_iterator	iter = m_kMToMCont.find(Guid);
	if( iter == m_kMToMCont.end() )
	{
		return false;
	}

	Name = iter->second.MToMName();
	return true;
}


bool PgMToMChatMgr::GetChatDlgList(kMToMChatUserContainer& kCont)const
{
	if( !m_kMToMCont.size() )
	{
		return false;
	}

	kMToMContainer::const_iterator	iter = m_kMToMCont.begin();
	for( ; iter != m_kMToMCont.end() ; ++iter )
	{
		kCont.push_back( kMToMChatUserContainer::value_type(iter->first, iter->second.Type()) );
	}

	return true;
}

bool PgMToMChatMgr::AddID(BM::GUID const& Guid, std::wstring& wstrID)
{
	kMToMContainer::const_iterator	c_iter = m_kMToMCont.find(Guid);
	if( c_iter == m_kMToMCont.end() )
	{
		return false;
	}

	if( c_iter->second.IsDlgOpen() )
	{
		for(int i = 0; i < MAX_DLG_COUNT; ++i)
		{
			BM::vstring	vStr(szDefault_BigFormName);
			vStr+= i;
			wstrID = (std::wstring const&)vStr;
			kIDContainer::iterator	ID_iter = m_kBigIDCont.find(wstrID);
			if( ID_iter != m_kBigIDCont.end() )
			{
				continue;
			}

			ID_iter = m_kBigIDCont.insert(m_kBigIDCont.end(), wstrID);
			if( ID_iter == m_kBigIDCont.end() )
			{
				return false;
			}

			SNameData	Data;
			std::pair<kDlgIDContainer::iterator, bool> Rst = m_kGuidToDlgIDCont.insert(std::make_pair(Guid, Data));
			if( Rst.second )
			{
				Rst.first->second.bIsOpen = c_iter->second.IsDlgOpen();
				Rst.first->second.iter = ID_iter;
				return true;
			}
		}
	}
	else
	{
		for(int i = 0; i < MAX_DLG_COUNT; ++i)
		{
			BM::vstring	vStr(szDefault_SmallFormName);
			vStr+= i;
			wstrID = (std::wstring const&)vStr;
			kIDContainer::iterator	ID_iter = m_kSmallIDCont.find(wstrID);
			if( ID_iter != m_kSmallIDCont.end() )
			{
				continue;
			}

			ID_iter = m_kSmallIDCont.insert(m_kSmallIDCont.end(), wstrID);
			if( ID_iter == m_kSmallIDCont.end() )
			{
				return false;
			}

			SNameData	Data;
			std::pair<kDlgIDContainer::iterator, bool> Rst = m_kGuidToDlgIDCont.insert(std::make_pair(Guid, Data));
			if( Rst.second )
			{
				Rst.first->second.bIsOpen = c_iter->second.IsDlgOpen();
				Rst.first->second.iter = ID_iter;
				return true;
			}
		}
	}

	return false;
}

bool PgMToMChatMgr::DelID(BM::GUID const& Guid)
{
	kDlgIDContainer::iterator	iter = m_kGuidToDlgIDCont.find(Guid);
	if( iter != m_kGuidToDlgIDCont.end() )
	{
		if( iter->second.bIsOpen )
		{
			m_kBigIDCont.erase(iter->second.iter);
		}
		else
		{
			m_kSmallIDCont.erase(iter->second.iter);
		}
		m_kGuidToDlgIDCont.erase(iter);

		return true;
	}

	return false;
}

bool PgMToMChatMgr::FindID(BM::GUID const& Guid, std::wstring& wstrID)
{
	kDlgIDContainer::iterator	iter = m_kGuidToDlgIDCont.find(Guid);
	if( iter != m_kGuidToDlgIDCont.end() )
	{
		wstrID = iter->second.iter->c_str();
		return true;
	}
	return false;
}

void PgMToMChatMgr::GetIDList(bool const bIsOpen, kIDContainer& kList)
{
	if( bIsOpen )
	{
		kList.insert(m_kBigIDCont.begin(), m_kBigIDCont.end());
	}
	else
	{
		kList.insert(m_kSmallIDCont.begin(), m_kSmallIDCont.end());
	}
}

void PgMToMChatMgr::Clear()
{
//	m_kTimeCont.clear();
	m_kMToMCont.clear();
	m_kBigIDCont.clear();
	m_kSmallIDCont.clear();
	m_kGuidToDlgIDCont.clear();
}
