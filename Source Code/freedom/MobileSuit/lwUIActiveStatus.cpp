#include "StdAfx.h"
#include "Variant/Global.h"
#include "PgNetwork.h"
#include "PgPilotMan.h"
#include "PgChatMgrclient.h"
#include "lwUI.h"
#include "PgUISound.h"
#include "lwUIActiveStatus.h"
#include "lwWorld.h"

int const INIT_STR_ITEMNO = 98004350; // STR 스텟 초기화
int const INIT_INT_ITEMNO = 98004360; // INT 스텟 초기화
int const INIT_CON_ITEMNO = 98004370; // CON 스텟 초기화
int const INIT_DEX_ITEMNO = 98004380; // DEX 스텟 초기화
int const INIT_ALL_ITEMNO = 98004390; // ALL 스텟 초기화

PgUIActiveStatus::PgUIActiveStatus()
:m_iRemainStatus(0)
{
}

PgUIActiveStatus::~PgUIActiveStatus()
{
}

void PgUIActiveStatus::InitUIData()
{
	m_iStr = 0;
	m_iInt = 0;
	m_iDex = 0;
	m_iCon = 0;
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkPlayer)
	{
		m_iRemainStatus=0;
		return;
	}
	else
	{
		m_iRemainStatus = pkPlayer->GetAbil(AT_STATUS_BONUS);
	}
}

void PgUIActiveStatus::CallUI()
{	
	InitUIData();
	XUIMgr.Call(L"SFRM_ACTIVE_STATUS");
	UpdateUI();
}

void PgUIActiveStatus::UpdateUI()
{
	for(int i=E_ACTSTAT_STR; i < E_ACTSTAT_MAX; ++i)
	{
		UpdateElem(static_cast<eActiveStatus>(i));
	}
}

void PgUIActiveStatus::UpdateElem(eActiveStatus const eType)
{
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkPlayer)
	{
		return;
	}
	XUI::CXUI_Wnd* pkMain = XUIMgr.Get(L"SFRM_ACTIVE_STATUS");
	if(!pkMain)
	{
		return;
	}
	BM::vstring vStr(L"FRM_ELEM");
	vStr += static_cast<int>(eType)-1;
	XUI::CXUI_Wnd* pkElem = pkMain->GetControl(vStr);
	if(!pkElem)
	{
		return;
	}

	int iAddingValue = 0;
	int iAccAddValue = 0;

	switch(eType)
	{
	case E_ACTSTAT_STR:
		{
			iAddingValue = m_iStr;
			iAccAddValue = pkPlayer->GetAbil(AT_STR_ADD);
		}break;
	case E_ACTSTAT_INT:
		{
			iAddingValue = m_iInt;
			iAccAddValue = pkPlayer->GetAbil(AT_INT_ADD);
		}break;
	case E_ACTSTAT_DEX:
		{
			iAddingValue = m_iDex;
			iAccAddValue = pkPlayer->GetAbil(AT_DEX_ADD);
		}break;
	case E_ACTSTAT_CON:
		{
			iAddingValue = m_iCon;
			iAccAddValue = pkPlayer->GetAbil(AT_CON_ADD);
		}break;
	default:
		{
			return;
		}break;
	}
		
	XUI::CXUI_Wnd* pkBonus = pkElem->GetControl(L"SFRM_BNS");
	if(pkBonus)
	{// 현재 UI상에서 수정중인 status 값
		pkBonus->Text(BM::vstring(iAddingValue));
	}

	XUI::CXUI_Wnd* pkAccBonus= pkElem->GetControl(L"SFRM_ACC_BNS");
	if(pkAccBonus)
	{// 해당 status 누적 수량
		iAccAddValue+=iAddingValue;		//누적 수량 계산
		pkAccBonus->Text(BM::vstring(iAccAddValue));
	}	

	XUI::CXUI_Wnd* pkRemain= pkMain->GetControl(L"FRM_REMAIN_STAT_VALUE");
	if(pkRemain)
	{// UI상의 남은 스탯 사용 수치
		pkRemain->Text(BM::vstring(m_iRemainStatus));
	}
}

void PgUIActiveStatus::ModifyStat(eActiveStatus const eType, bool const bInc)
{
	XUI::CXUI_Wnd* pkMain = XUIMgr.Get(L"SFRM_ACTIVE_STATUS");
	if(!pkMain)
	{
		return;
	}
	BM::vstring vStr(L"FRM_ELEM");
	vStr += static_cast<int>(eType)-1;
	XUI::CXUI_Wnd* pkElem = pkMain->GetControl(vStr);
	if(!pkElem)
	{
		return;
	}

	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkPlayer)
	{
		return;
	}

	GET_DEF(PgClassDefMgr, kClassDef);
	int const iDelta = bInc ? 1 : -1;
	SClassKey kClassKey(pkPlayer->GetAbil(AT_CLASS), pkPlayer->GetAbil(AT_LEVEL));
	
	int* piAddingValue = NULL;
	int iAccAddValue = 0;
	int iStatLimit = 0;
	switch(eType)
	{
	case E_ACTSTAT_STR:
		{			
			piAddingValue = &m_iStr;
			iAccAddValue = pkPlayer->GetAbil(AT_STR_ADD);
			iStatLimit = kClassDef.GetAbil(kClassKey, AT_STR_ADD_LIMIT); //pkPlayer->GetAbil(AT_STR_ADD_LIMIT);
		}break;
	case E_ACTSTAT_INT:
		{
			piAddingValue = &m_iInt;
			iAccAddValue = pkPlayer->GetAbil(AT_INT_ADD);
			iStatLimit = kClassDef.GetAbil(kClassKey, AT_INT_ADD_LIMIT); //pkPlayer->GetAbil(AT_INT_ADD_LIMIT);
		}break;
	case E_ACTSTAT_DEX:
		{
			piAddingValue = &m_iDex;
			iAccAddValue = pkPlayer->GetAbil(AT_DEX_ADD);
			iStatLimit = kClassDef.GetAbil(kClassKey, AT_DEX_ADD_LIMIT); //pkPlayer->GetAbil(AT_DEX_ADD_LIMIT);
		}break;
	case E_ACTSTAT_CON:
		{
			piAddingValue = &m_iCon;
			iAccAddValue = pkPlayer->GetAbil(AT_CON_ADD);
			iStatLimit = kClassDef.GetAbil(kClassKey, AT_CON_ADD_LIMIT);// pkPlayer->GetAbil(AT_CON_ADD_LIMIT);
		}break;
	default:
		{
			return;
		}break;
	}
	int& riAddingValue = *piAddingValue;
	int const iSum = riAddingValue+iAccAddValue+iDelta;
	int const iCalcRemainStatus = m_iRemainStatus-iDelta;
	if(iSum < 0
		|| (riAddingValue+iDelta) < 0
		)
	{// 감소 시킬수는 없다.
		return;
	}

	if(iStatLimit < iSum)
	{// 스탯 제한에 걸림
		lwAddWarnDataTT(790652);
		return;
	}

	if(iCalcRemainStatus < 0)
	{// 보너스 스탯 부족
		lwAddWarnDataTT(790651);
		return;
	}

	// 값 수정
	riAddingValue+=iDelta;
	m_iRemainStatus-=iDelta;
	// UI 갱신
	UpdateElem(eType);
}
bool PgUIActiveStatus::SendModifyAddStatus()
{	
	BM::Stream kPacket(PT_C_M_REQ_STATUS_CHANGE);
	VEC_ABILINFO kCont;
	kCont.push_back(SAbilInfo(AT_STR_ADD, m_iStr));
	kCont.push_back(SAbilInfo(AT_INT_ADD, m_iInt));
	kCont.push_back(SAbilInfo(AT_DEX_ADD, m_iDex));
	kCont.push_back(SAbilInfo(AT_CON_ADD, m_iCon));		
	kPacket.Push(kCont);
	NETWORK_SEND(kPacket);
	InitUIData();	//패킷 보내면 초기화
	return true;
}

bool PgUIActiveStatus::RecvPacket(BM::Stream& rkPacket)
{
	HRESULT kResult = S_OK;
	rkPacket.Pop(kResult);
	switch(kResult)
	{
	case S_OK:
		{// 성공
			SChatLog kChatLog(CT_NOTICE);
			g_kChatMgrClient.AddLogMessage(kChatLog, TTW(790653), true);
		}break;
	case E_NOT_ENOUGH:
		{// 스탯 부족
			lwAddWarnDataTT(790651);
		}break;
	case E_LIMITED_STR:
	case E_LIMITED_INT:
	case E_LIMITED_CON:
	case E_LIMITED_DEX:
		{// 스탯 제한에 걸림
			lwAddWarnDataTT(790652);
		}break;
	//case E_초기화불가:
	//	{// 초기화 할 수 없음
	//		lwAddWarnDataTT(790657);			
	//	}break;
	default:
		{
			return false;
		}break;
	}
	return true;
}

namespace lwUIActiveStatus
{
	void RegisterWrapper(lua_State *pkState)
	{	
		using namespace lua_tinker;		
		def(pkState, "InitUIData_AS", lwUIActiveStatus::InitUIData_AS);
		def(pkState, "Call_AS", lwUIActiveStatus::Call_AS);
		def(pkState, "Update_AS", lwUIActiveStatus::Update_AS);
		def(pkState, "ModifyStat_AS", lwUIActiveStatus::ModifyStat_AS);
		def(pkState, "SendModifyAddStatus_AS", lwUIActiveStatus::SendModifyAddStatus_AS);
		def(pkState, "UseInitStatusItem_AS", lwUIActiveStatus::UseInitStatusItem_AS);
		def(pkState, "UseInitAllStatusItem_AS", lwUIActiveStatus::UseInitAllStatusItem_AS);
		def(pkState, "GetAddtionlaStatus_AS", lwUIActiveStatus::GetAddtionlaStatus_AS);
		def(pkState, "IsModifiedStatusInUI_AS", lwUIActiveStatus::IsModifiedStatusInUI_AS);		
	}
	
	void InitUIData_AS()
	{
		g_kUIActiveStatus.InitUIData();
	}

	void Call_AS()
	{
		g_kUIActiveStatus.CallUI();
	}

	void Update_AS()
	{
		g_kUIActiveStatus.UpdateUI();
	}

	void ModifyStat_AS(int const iType, bool const bInc)
	{
		g_kUIActiveStatus.ModifyStat(static_cast<eActiveStatus>(iType), bInc);
	}

	bool SendModifyAddStatus_AS()
	{
		return g_kUIActiveStatus.SendModifyAddStatus();
	}

	bool UseInitStatusItem_AS(int const iType)
	{
		int iItemNo = 0;
		switch(iType)
		{
		case E_ACTSTAT_STR: { iItemNo = INIT_STR_ITEMNO;}break;
		case E_ACTSTAT_INT:	{ iItemNo = INIT_INT_ITEMNO;}break;
		case E_ACTSTAT_CON:	{ iItemNo = INIT_CON_ITEMNO;}break;
		case E_ACTSTAT_DEX:	{ iItemNo = INIT_DEX_ITEMNO;}break;		
		default:
			{				
				return false;
			}break;
		}
		int iTextNo = 7041+(10*(iType-1));

		// 아이템 존재 하는지 확인
		SItemPos kItemPos;
		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if(!pkPlayer)
		{
			return false;
		}
		PgInventory* pkInv = pkPlayer->GetInven();
		if(!pkInv)
		{
			return false;
		}
		HRESULT const kResult = pkInv->GetFirstItem(iItemNo, kItemPos);

		std::wstring kStr;
		GetItemName(iItemNo, kStr);
		wchar_t szBuf[MAX_PATH]={0,};
		if(S_OK== kResult)
		{// 아이템이 있으면
			::swprintf_s(szBuf, MAX_PATH, TTW(790650).c_str(), kStr.c_str());
			BM::Stream kPacket(PT_C_M_REQ_ITEM_ACTION, kItemPos);
			kPacket.Push(lwGetServerElapsedTime32());
			lwCallCommonMsgYesNoBox(MB(szBuf), lwPacket(&kPacket), true, MBT_COMMON_YESNO_TO_PACKET);
		}
		else
		{// 아이템이 없으면
			::swprintf_s(szBuf, MAX_PATH, TTW(790649).c_str(), kStr.c_str());
			XUI::CXUI_Wnd* pkWnd = XUIMgr.Call(L"SFRM_ACTIVE_MSG_BOX", true);
			if(pkWnd)
			{
				XUI::CXUI_Wnd* pkTxtWnd = pkWnd->GetControl(L"SFR_SDW");
				if(pkTxtWnd)
				{
					pkTxtWnd->Text(szBuf);
				}
			}
		}
		return true;
	}

	bool UseInitAllStatusItem_AS()
	{
		SItemPos kItemPos;
		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if(!pkPlayer)
		{
			return false;
		}
		PgInventory* pkInv = pkPlayer->GetInven();
		if(!pkInv)
		{
			return false;
		}
		std::wstring kStr;
		GetItemName(INIT_ALL_ITEMNO, kStr);
		wchar_t szBuf[MAX_PATH]={0,};
		HRESULT const kResult = pkInv->GetFirstItem(INIT_ALL_ITEMNO, kItemPos);
		if(S_OK == kResult)
		{// 아이템이 있으면
			BM::Stream kPacket(PT_C_M_REQ_ITEM_ACTION, kItemPos);
			::swprintf_s(szBuf, MAX_PATH, TTW(790650).c_str(), kStr.c_str());
			kPacket.Push(lwGetServerElapsedTime32());
			lwCallCommonMsgYesNoBox(MB(szBuf), lwPacket(&kPacket), true, MBT_COMMON_YESNO_TO_PACKET);
		}
		else
		{
			::swprintf_s(szBuf, MAX_PATH, TTW(790649).c_str(), kStr.c_str());
			XUI::CXUI_Wnd* pkWnd = XUIMgr.Call(L"SFRM_ACTIVE_MSG_BOX", true);
			if(pkWnd)
			{
				XUI::CXUI_Wnd* pkTxtWnd = pkWnd->GetControl(L"SFR_SDW");
				if(pkTxtWnd)
				{
					pkTxtWnd->Text(szBuf);
				}
			}		
		}
		return true;
	}

	int GetAddtionlaStatus_AS(int const iType)
	{
		switch(iType)
		{
		case E_ACTSTAT_STR:
			{
				return g_kUIActiveStatus.Str();
			}break;
		case E_ACTSTAT_INT:
			{
				return g_kUIActiveStatus.Int();
			}break;
		case E_ACTSTAT_DEX:
			{
				return g_kUIActiveStatus.Dex();
			}break;
		case E_ACTSTAT_CON:
			{
				return g_kUIActiveStatus.Con();
			}break;
		default:
			{
			}break;
		}
		return 0;
	}

	bool IsModifiedStatusInUI_AS()
	{
		if(0 == g_kUIActiveStatus.Str()
			&& 0 == g_kUIActiveStatus.Int()
			&& 0 == g_kUIActiveStatus.Dex()
			&& 0 == g_kUIActiveStatus.Con()
			)
		{
			return false;
		}
		return true;
	}

	bool UseInitStatusItem(int const iItemNo)
	{
		int iType = 0;
		switch(iItemNo)
		{
		case INIT_STR_ITEMNO:	{ iType = E_ACTSTAT_STR;}break;
		case INIT_INT_ITEMNO:	{ iType = E_ACTSTAT_INT;}break;
		case INIT_CON_ITEMNO:	{ iType = E_ACTSTAT_CON;}break;
		case INIT_DEX_ITEMNO:	{ iType = E_ACTSTAT_DEX;}break;		
		case INIT_ALL_ITEMNO:	
			{
				return UseInitAllStatusItem_AS();
			}break;
		default:
			{				
				return false;
			}break;
		}
		return UseInitStatusItem_AS(iType);
	}
}