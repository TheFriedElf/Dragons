#include "stdafx.h"
#include "PgLocalManager.h"
#include "PgIInputObserver.h"
#include "PgXmlLoader.h"
#include "PgRenderer.h"
#include "PgRenderMan.h"
#include "PgPilotMan.h"
#include "PgMobileSuit.h"
#include "PgStat.h"
#include <WinUser.h>
#include "./NiApplication/NiApplication.h"
#include "PgInputSystem.H"

PgLocalManager *PgLocalManager::m_pkSelf = 0;
unsigned PgLocalManager::ms_uiLastSequence = 0;

// Input Guid
static const GUID s_kInputGUID = 
{ 
	0xf36353fd, 0xf0e4, 0x485a, 
	{ 0x91, 0x40, 0x8d, 0xd8, 0x47, 0x66, 0x1e, 0x72 } 
};

PgLocalManager::PgLocalManager()
{
	m_spActionMap = 0;
	m_spInputSystem = 0;
	m_spKeyboard = 0;
	m_spMouse = 0;
	m_spGamePad = 0;
	ClearInputDirReverse();
}

PgLocalManager::~PgLocalManager()
{
	m_spActionMap = 0;
	m_spInputSystem = 0;
	m_spKeyboard = 0;
	m_spMouse = 0;
	m_spGamePad = 0;
}

void PgLocalManager::DestroySingleton()
{
	if(m_pkSelf) 
	{
		InputSlotExContainer::iterator itr = m_pkSelf->m_kInputSlotExContainer.begin();
		while(itr != m_pkSelf->m_kInputSlotExContainer.end())
		{
			SAFE_DELETE(*itr);
			++itr;
		}
		SAFE_DELETE_NI(m_pkSelf);
	}
	m_pkSelf = 0;
}

bool PgLocalManager::Create(char const *pcID, void *pArg)
{
	m_pkSelf = dynamic_cast<PgLocalManager*>(PgXmlLoader::CreateObject(pcID));
	PG_ASSERT_LOG(m_pkSelf);

	if (m_pkSelf == NULL || m_pkSelf->Initialize(pArg) == false)
	{
		PgError("Local Input Manager : Instance Create Failed");
		return 0;
	}
	
	return true;
}

PgLocalManager *PgLocalManager::Get()
{
	return m_pkSelf;
}

bool PgLocalManager::Initialize(void *pArg)
{
	NiDI8InputSystem::DI8CreateParams kParams;

	kParams.SetOwnerInstance(g_pkApp->GetInstanceReference());
	kParams.SetOwnerWindow(g_pkApp->GetWindowReference());
	bool bNoWinKey = false;
	if (g_pkApp && g_pkApp->GetFullscreen())
		bNoWinKey = true;	// TODO: DISCL_NOWINKEY 적용하자.

	PG_ASSERT_LOG(NiRenderer::GetRenderer());
	kParams.SetRenderer(NiRenderer::GetRenderer());
#ifdef EXTERNAL_RELEASE
	kParams.SetKeyboardUsage(NiInputSystem::FOREGROUND | NiInputSystem::NONEXCLUSIVE);
#else
	kParams.SetKeyboardUsage(NiInputSystem::BACKGROUND | NiInputSystem::NONEXCLUSIVE);
#endif

#ifdef EXTERNAL_RELEASE
    kParams.SetMouseUsage(NiInputSystem::FOREGROUND | NiInputSystem::EXCLUSIVE);
#else
	kParams.SetMouseUsage(NiInputSystem::BACKGROUND | NiInputSystem::NONEXCLUSIVE);
#endif
    kParams.SetAxisRange(-100, +100);

	if (!m_spActionMap)
	{
		PgError("Local Input Manager : Action Map Init Failed");
		return false;
	}

	// Set Action Map
	kParams.SetActionMap(m_spActionMap);

	// Create the input system
	if(g_bUseDirectInput)
	{
		m_spInputSystem = NiInputSystem::Create(&kParams);
	}
	else
	{
		m_spInputSystem = NiNew PgInputSystem(&kParams);
	}
	
	if (!m_spInputSystem)
	{
		PgError("Local Input Manager : Input System Create Failed");
		return false;
	}

	// Sleep to give the system time to init.
	// We are just going to stall here...
	bool bGotDevices = false;
	while (!bGotDevices)
	{
		NiInputErr eErr = m_spInputSystem->CheckEnumerationStatus();
		switch(eErr)
		{
		case NIIERR_ENUM_NOTRUNNING:
			{
				PG_ASSERT_LOG(!"EnumerateDevices failed");
			}return false;
		case NIIERR_ENUM_FAILED:
			{
				PG_ASSERT_LOG(!"CheckEnumerationStatus! FAILED");
			}return false;
		case NIIERR_ENUM_COMPLETE:
			bGotDevices = true;
			break;
		case NIIERR_ENUM_NOTCOMPLETE:
		default:
			break;
		}
	}

	// Open the keyboard and mouse
	m_spKeyboard = m_spInputSystem->OpenKeyboard();
	m_spMouse = m_spInputSystem->OpenMouse();
	//Gamepad may not exist, but attempt to open any
    for (unsigned int iGamePad = 0; iGamePad < NiInputSystem::MAX_GAMEPADS; iGamePad++)
    {
       m_spGamePad = m_spInputSystem->OpenGamePad(iGamePad,0);
	   if(NULL != m_spGamePad)
	   {
		   break;
	   }
    }
	return true;
}
bool	PgLocalManager::HandleWindowMessage(unsigned int uiMsg,WPARAM wParam,LPARAM lParam)
{
	if(g_bUseDirectInput)
	{
		return	false;
	}

	PgInputSystem	*pkInputSystem = NiDynamicCast(PgInputSystem,m_spInputSystem);
	if(pkInputSystem)
	{
		return	pkInputSystem->HandleWindowMessage(uiMsg,wParam,lParam);
	}

	return	false;
}
/// Override
bool PgLocalManager::UpdateInput()
{
	bool bResult = false;
	
	if (m_spInputSystem == NULL)
		return false;

	PG_STAT(PgStatTimerF timerA(g_kLocalInputStatGroup.GetStatInfo("UpdateInput"), g_pkApp->GetFrameCount()));
	if (m_spInputSystem->GetMode() == NiInputSystem::MAPPED)
	{
		if (m_spInputSystem->UpdateActionMap() == NIIERR_DEVICELOST)
			m_spInputSystem->HandleDeviceChanges();
	}
	else
	{
		if (m_spInputSystem->UpdateAllDevices() == NIIERR_DEVICELOST)
			m_spInputSystem->HandleDeviceChanges();
	}

	PgInput kInput;
	int iActionDataCount = m_spInputSystem->GetActionDataCount();
	int iActionDataProcessed = 0;
	NiActionData* pkActionData = m_spInputSystem->PopActionData();		
	
    while(pkActionData)
    {
		NILOG(PGLOG_LOG, "[PgLocalManager] (%d) UpdateInput : %u\n", g_pkApp->GetFrameCount(), pkActionData->GetAppData());
		// 인풋을 생성
		CreateInput(pkActionData, kInput);

		//WriteToConsole("___Key : %u, Seq : %u\n", kInput.Data()->GetDataValue(), kInput.Data()->GetSequence());

		PG_STAT(char buf[64]; _itoa_s(kInput.GetUKey(), buf, 64, 10););
		PG_STAT(PgStatTimerF timerB(g_kLocalInputStatGroup.GetStatInfo(std::string("UpdateInput.") + buf), g_pkApp->GetFrameCount()));
			
		// Observer에게 Input을 처리하도록 넘김
		TransferToObserver(kInput);

		// 다음 인풋을 생성할 수 있게, 다음 입력을 꺼낸다.
		m_spInputSystem->ReleaseActionData(pkActionData);
        pkActionData = m_spInputSystem->PopActionData();
		iActionDataProcessed++;
    }

#ifndef EXTERNAL_RELEASE
	if (iActionDataProcessed != iActionDataCount)
	{
		NILOG(PGLOG_WARNING, "[PgLocalManager](%d) Action Data count doesn't match(%d, %d)\n", g_pkApp->GetFrameCount(), iActionDataCount, iActionDataProcessed);
	}

	if (iActionDataProcessed > 30 || iActionDataCount > 30)
	{
		NILOG(PGLOG_WARNING, "[PgLocalManager](%d) Action Data count is too many(%d, %d)\n", g_pkApp->GetFrameCount(), iActionDataCount, iActionDataProcessed);
	}
#endif

	return bResult;
}
void	PgLocalManager::DeactivateAllExtenedSlot()
{
	int	iTotal = m_kInputSlotExContainer.size();
	for(int i=0;i<iTotal;i++)
	{
		m_kInputSlotExContainer[i]->SetActivate(false);
	}	
}
void	PgLocalManager::ActivateExtendedSlot(int iSlotUKey,bool bActivate)
{
	int	iTotal = m_kInputSlotExContainer.size();
	for(int i=0;i<iTotal;i++)
	{
		if(m_kInputSlotExContainer[i]->GetUKey() == iSlotUKey)
		{
			m_kInputSlotExContainer[i]->SetActivate(bActivate);
		}
	}	
	
}

bool PgLocalManager::ProcessExtendedInput(PgInput &rkInput)
{
	PG_STAT(PgStatTimerF timerA(g_kLocalInputStatGroup.GetStatInfo("ProcessExtendedInput"), g_pkApp->GetFrameCount()));
	// 특수한 키에 대한 처리
	bool bResetFlag = false;

	InputSlotExContainer::iterator itr = m_kInputSlotExContainer.begin();
	while(itr != m_kInputSlotExContainer.end())
	{
		PgInputSlotEx *pkInputSlotEx = *itr;
		if(pkInputSlotEx->GetActivate() == false)
		{
			++itr;
			continue;
		}
		unsigned int uiReturedUKey = pkInputSlotEx->ProcessExInput(rkInput);

		if(uiReturedUKey)
		{
			if(pkInputSlotEx->KeepLastKey())
			{
				// 마지막에 누른 키를 살리겠다면
				// 기존의 rkInput은 Observer에게 Transfer하고
				// rkInput은 키 조합으로 인해 나온 Unique-Key로 바꾸어준다.
				TransferToObserver(rkInput);
				
				rkInput.SetUKey(uiReturedUKey);
				rkInput.SetPressed(true);
			}
			else
			{
				// 마지막에 누른 키를 버리겠다면, 
				// Observer에게 전달될 rkInput의 Unique-Key를 바꾸어버린다.
				rkInput.SetUKey(uiReturedUKey);
				rkInput.SetPressed(true);
			}
			
			if(!pkInputSlotEx->DoReset())
			{
				bResetFlag = true;
			}
			break;
		}
		++itr;
	}

	if(bResetFlag)
	{
		// Reset Flag는 키 조합이 비슷한 것들을 등록 했을 때 두 번 발동되는 경우가 있다.
		// 그래서 하나의 키 조합이 완성 되었을 때, 이전에 눌렀던 기록을 제거해야 할 때 쓰는 옵션이다.
		InputSlotExContainer::iterator itr = m_kInputSlotExContainer.begin();
		while(itr != m_kInputSlotExContainer.end())
		{
			(*itr)->Reset();
			++itr;
		}
	}

	return true;
}

bool PgLocalManager::CreateInput(NiActionData *pkActionData, PgInput& rkInput)
{
	PG_STAT(PgStatTimerF timerA(g_kLocalInputStatGroup.GetStatInfo("CreateInput"), g_pkApp->GetFrameCount()));
	PG_ASSERT_LOG(pkActionData);
	if (pkActionData == NULL)
		return false;


	BM::GUID kPilotGuid;
	if(g_kPilotMan.GetPlayerPilotGuid(kPilotGuid))
	{
		rkInput.SetPilotGuid(kPilotGuid);
	}
	
	unsigned int uiUKey = pkActionData->GetAppData();
	bool bPressed = pkActionData->GetDataValue() ? true : false;

	rkInput.SetPressed(bPressed);
	rkInput.Set(pkActionData);
	if(GetInputDirReverse())
	{
		uiUKey-=PgInput::UR_LOCAL_BEGIN;	//1000을 빼야 Define된 값이 됨

		switch(uiUKey)
		{
		case NiInputKeyboard::KEY_LEFT:
			{
				uiUKey = NiInputKeyboard::KEY_RIGHT;
			}break;
		case NiInputKeyboard::KEY_RIGHT:
			{
				uiUKey = NiInputKeyboard::KEY_LEFT;
			}break;
		case NiInputKeyboard::KEY_UP:
			{
				uiUKey = NiInputKeyboard::KEY_DOWN;
			}break;
		case NiInputKeyboard::KEY_DOWN:
			{
				uiUKey = NiInputKeyboard::KEY_UP;
			}break;
		default:
			{
			}break;
		}

		uiUKey += PgInput::UR_LOCAL_BEGIN;	//뺀 값 복구
	}
	rkInput.SetUKey(uiUKey);

	if(uiUKey == NiAction::MOUSE_BUTTON_LEFT || uiUKey == NiAction::MOUSE_BUTTON_RIGHT)
	{
		uiUKey = PgInput::UR_LOCAL_MOUSE_BEGIN + (uiUKey == NiAction::MOUSE_BUTTON_LEFT ? 1 : 3);			// xxxx todo mapping
		if(!bPressed)
		{
			++uiUKey;
		}
		rkInput.SetUKey(uiUKey);
		return true;
	}

	// 키보드에 대해서만 Sequence를 증가시킨다.
	if(bPressed && rkInput.Data()->GetDeviceID() == 1)
	{
		++ms_uiLastSequence;
	}

	// 확장 슬롯에 대한 인풋 처리 (마우스는 처리하지 않는다)
	return ProcessExtendedInput(rkInput);
}

void PgLocalManager::AddActionKeyboard(unsigned int uiUKey, NiInputKeyboard::KeyCode eKeyCode)
{
	if( eKeyCode == NiInputKeyboard::KEY_NOKEY )
	{
		return;
	}
	char szTemp[255] = {0, };//키이름
	errno_t eRet = _itoa_s((int)eKeyCode, szTemp, 255, 10);
	if(eRet == 0)
	{
		m_spActionMap->AddAction(szTemp, uiUKey, KEY_MAP(eKeyCode), NiAction::RETURN_BOOLEAN);
	}
	else
	{
		NILOG(PGLOG_ERROR, "Convert failed 10 radix integer to string");
		PG_ASSERT_LOG(0);
	}
}

//appData == UIKey, Semantic == KEY
bool PgLocalManager::ActionKeyboard_Set(char const *szName, const unsigned int iUKey)
{
	if(NULL == szName )
	{
		NILOG(PGLOG_ERROR, "Action name is NULL");
		PG_ASSERT_LOG(0);
		return false;
	}
	unsigned int iCount = m_spActionMap->GetActionCount();
	for(unsigned int i=0; i<iCount; ++i)
	{
		NiAction *pkAction = m_spActionMap->GetAction(i);
		if(!pkAction) 
		{
			NILOG(PGLOG_ERROR, "NiAction is NULL"); 
			PG_ASSERT_LOG(0);
		}
		if(0 == strcmp(szName, pkAction->GetName()))
		{
			pkAction->SetAppData(iUKey);
			return true;
		}
	}
	NILOG(PGLOG_ERROR, "Can't Found Key!!!!!");
	//PG_ASSERT_LOG(0);
	return false;
}

bool PgLocalManager::ActionKeyboard_Swap(char const *szFromName, char const *szToName)
{
	if(0 == strcmp(szFromName, szToName))
	{
		NILOG(PGLOG_LOG, "Original and Target Unique key is equal");
		return true;
	}

	NiAction *pkActionOrg = NULL, *pkActionTrg = NULL;

	unsigned int iCount = m_spActionMap->GetActionCount();
	for(unsigned int i=0; i<iCount;++i)
	{
		NiAction *pkTemp = m_spActionMap->GetAction(i);
		char const *szName = pkTemp->GetName();
		if(0 == strcmp(szFromName, szName))		{ pkActionOrg = pkTemp; }//From
		else if(0 == strcmp(szToName, szName))	{ pkActionTrg = pkTemp; }//To

		if(NULL != pkActionOrg && NULL != pkActionTrg)//둘다 찾은거다
		{
			unsigned int iTemp = pkActionOrg->GetAppData();//Set
			pkActionOrg->SetAppData(pkActionTrg->GetAppData());
			pkActionTrg->SetAppData(iTemp);
			return true;
		}
	}

	if(NULL == pkActionOrg)
	{
		NILOG(PGLOG_ERROR, "Can't found Orginal Unique key");
		PG_ASSERT_LOG(0);
	}
	if(NULL == pkActionTrg)
	{
		NILOG(PGLOG_ERROR, "Can't found Target Unique Key");
		PG_ASSERT_LOG(0);
	}
	return false;
}
int ExSlotCompareFunc(const void *arg1, const void *arg2)
{

	PgLocalManager::PgInputSlotEx	*pkSlot1 = *(PgLocalManager::PgInputSlotEx**)arg1;
	PgLocalManager::PgInputSlotEx	*pkSlot2 = *(PgLocalManager::PgInputSlotEx**)arg2;

	return	pkSlot1->GetNBKeys()<pkSlot2->GetNBKeys() ? 1 : -1;
}
void	PgLocalManager::SortExContainer()
{
	//	긴것이 앞에 오도록 정렬한다.

	int	iTotalSlot = m_kInputSlotExContainer.size();
	if(iTotalSlot<2)
	{
		return;
	}

	PgInputSlotEx	**pkSlotArray = new PgInputSlotEx*[iTotalSlot];

	for(int i=0;i<iTotalSlot;i++)
	{
		*(pkSlotArray+i) = m_kInputSlotExContainer[i];
	}

	::qsort(pkSlotArray,iTotalSlot,sizeof(PgInputSlotEx*),ExSlotCompareFunc);

	m_kInputSlotExContainer.clear();

	for(int i=0;i<iTotalSlot;i++)
	{
		m_kInputSlotExContainer.push_back(*(pkSlotArray+i));
	}

	SAFE_DELETE_ARRAY(pkSlotArray);

}
void PgLocalManager::AddActionMouseAxes(std::string const &kSlotName, int const KeyCode, BM::GUID const &kGuid)
{
	NiAction::Semantic ResultKey;

	switch(KeyCode)
	{
	case 1:	{ResultKey = NiAction::MOUSE_AXIS_X;	}break;
	case 2:	{ResultKey = NiAction::MOUSE_AXIS_Y;	}break;
	case 3:	{ResultKey = NiAction::MOUSE_AXIS_Z;	}break;
	default:
		{ 
			PG_ASSERT_LOG(0);
			return; 
		}break;
	}

	m_spActionMap->AddAction(kSlotName.c_str(), (int)ResultKey, ResultKey, 0);
}

void PgLocalManager::AddActionMouseButton(std::string const &kSlotName, int const KeyCode, BM::GUID const &kGuid)
{
	NiAction::Semantic ResultKey;

	switch(KeyCode)
	{
	case 1:	{ResultKey = NiAction::MOUSE_BUTTON_LEFT;	}break;
	case 2:	{ResultKey = NiAction::MOUSE_BUTTON_RIGHT;	}break;
	case 3:	{ResultKey = NiAction::MOUSE_BUTTON_MIDDLE;	}break;
	default:
		{
			PG_ASSERT_LOG(0);
			return; 
		}break;
	}

	m_spActionMap->AddAction(kSlotName.c_str(), (int)ResultKey, ResultKey, NiAction::RETURN_BOOLEAN);
}

bool PgLocalManager::ParseXml(const TiXmlNode *pkNode, void *pArg, bool bUTF8)
{
	// Create Action Map
	m_spActionMap = NiNew NiActionMap("PgLocalManager", (void*)&s_kInputGUID);

	pkNode = pkNode->FirstChildElement();

	int const iType = pkNode->Type();
	
	switch(iType)
	{
	case TiXmlNode::ELEMENT:
		{
			TiXmlElement *pkElement = (TiXmlElement *)pkNode;
			PG_ASSERT_LOG(pkElement);

			while(pkElement)
			{
				char const *pcTagName = pkElement->Value();

				if(strcmp(pcTagName, "SLOT") == 0)
				{
					TiXmlAttribute *pkAttr = pkElement->FirstAttribute();
					NiInputKeyboard::KeyCode eKeyCode = NiInputKeyboard::KEY_NOKEY;
					unsigned int uiUKey = 0;

					while(pkAttr)
					{
						char const *pcName = pkAttr->Name();
						char const *pcValue = pkAttr->Value();

						if(strcmp(pcName, "UKEY") == 0)
						{
							uiUKey = atoi(pcValue);
						}
						else if(strcmp(pcName, "KEY") == 0)
						{
							eKeyCode = (NiInputKeyboard::KeyCode)atoi(pcValue);
						}
						else
						{
							PgXmlError1(pkElement, "XmlParse: Incoreect Attr '%s'", pcName);
							break;
						}
						pkAttr = pkAttr->Next();
					}

					// ActionMap 에 Action 등록
					AddActionKeyboard(uiUKey, eKeyCode);
				}
				else if(strcmp(pcTagName, "SLOTEX") == 0)
				{
					TiXmlAttribute *pkAttr = pkElement->FirstAttribute();
					NiInputKeyboard::KeyCode eKeyCode = NiInputKeyboard::KEY_NOKEY;
					unsigned int uiUKey = 0;
					char acKeyValue[128] = {0};
					DWORD dwParam = 0;
					bool bKeepLastKey = false;
					bool bReset = false;

					while(pkAttr)
					{
						char const *pcName = pkAttr->Name();
						char const *pcValue = pkAttr->Value();

						if(strcmp(pcName, "UKEY") == 0)
						{
							uiUKey = atoi(pcValue);
						}
						else if(strcmp(pcName, "KEY") == 0)
						{
							strcpy_s(acKeyValue, 128, pcValue);
						}
						else if(strcmp(pcName, "PARAM") == 0)
						{
							dwParam = atoi(pcValue);
						}
						else if(strcmp(pcName, "KEEP") == 0)
						{
							bKeepLastKey = (strcmp(pcValue, "TRUE") == 0 ? true : false);
						}
						else if(strcmp(pcName, "RESET") == 0)
						{
							bReset = (strcmp(pcValue, "FALSE") == 0 ? true : false);
						}
						else
						{
							PgXmlError1(pkElement, "XmlParse: Incoreect Attr '%s'", pcName);
							break;
						}
						pkAttr = pkAttr->Next();
					}
	
					PgInputSlotEx *pkInputSlotEx = PgInputSlotEx::CreateInputSlotEx(uiUKey, acKeyValue, dwParam, bKeepLastKey, bReset);
					if(pkInputSlotEx)
					{
						m_kInputSlotExContainer.push_back(pkInputSlotEx);
					}
				}
				else if(strcmp(pcTagName, "AXES") == 0)
				{
					TiXmlAttribute *pkAttr = pkElement->FirstAttribute();
					std::string kSlotName;
					int iKeyCode = 0;
					BM::GUID kGuid;

					while(pkAttr)
					{
						char const *pcName = pkAttr->Name();
						char const *pcValue = pkAttr->Value();

						if(strcmp(pcName, "NAME") == 0)
						{
							kSlotName = pcValue;
						}
						else if(strcmp(pcName, "KEY") == 0)
						{
							iKeyCode = atoi(pcValue);
						}
						else if(strcmp(pcName, "GUID") == 0)
						{
							kGuid.Set(pcValue);
						}
						else
						{
							PgXmlError1(pkElement, "XmlParse: Incoreect Attr '%s'", pcName);
							break;
						}
						pkAttr = pkAttr->Next();
					}

					// ActionMap 에 Action 등록
					AddActionMouseAxes(kSlotName, iKeyCode, kGuid);
				}
				else if(strcmp(pcTagName, "BUTTON") == 0)
				{
					TiXmlAttribute *pkAttr = pkElement->FirstAttribute();
					std::string kSlotName;
					int iKeyCode = 0;
					BM::GUID kGuid;

					while(pkAttr)
					{
						char const *pcName = pkAttr->Name();
						char const *pcValue = pkAttr->Value();

						if(strcmp(pcName, "NAME") == 0)
						{
							kSlotName = pcValue;
						}
						else if(strcmp(pcName, "KEY") == 0)
						{
							iKeyCode = atoi(pcValue);
						}
						else if(strcmp(pcName, "GUID") == 0)
						{
							kGuid.Set(pcValue);
						}
						else
						{
							PgXmlError1(pkElement, "XmlParse: Incoreect Attr '%s'", pcName);
							break;
						}
						pkAttr = pkAttr->Next();
					}

					// ActionMap 에 Action 등록
					AddActionMouseButton(kSlotName, iKeyCode, kGuid);
				}
				else
				{
					PgXmlError1(pkElement, "XmlParse: Incoreect Tag '%s'", pcTagName);
				}

				// 다음 Element를 Parsing
				pkElement = pkElement->NextSiblingElement();
			}
		}
		break;

	default:
		break;
	}

	SortExContainer();

	return true;
}

PgLocalManager::PgBaseInput::PgBaseInput() : 
	m_uiKey(0),
	m_dwTimeStamp(0),
	m_iSequence(0),
	m_bPressing(0)
{
}

void PgLocalManager::PgBaseInput::Reset()
{
	m_dwTimeStamp = 0;
	m_iSequence = 0;
	m_bPressing = 0;
}

int PgLocalManager::PgInputSlotEx::ms_iMaxKey = 3;
int PgLocalManager::PgInputSlotEx::ms_iInvalidSequence = -1;
PgLocalManager::PgInputSlotEx::PgInputSlotEx() :
	m_uiUKey(0),
	m_iKeyOption(0),
	m_dwParam(0),
	m_eSlotType(UNKNOWN),
	m_iNbKeys(0),
	m_bReset(false),
	m_bKeepLastKey(false)
{
}

PgLocalManager::PgInputSlotEx::~PgInputSlotEx()
{

}

PgLocalManager::PgInputSlotEx *PgLocalManager::PgInputSlotEx::CreateInputSlotEx(
	unsigned int uiUKey, 
	char *pcInputSequence, 
	DWORD dwParam, 
	bool bKeepLastKey, 
	bool bReset)
{
	PgInputSlotEx *pkThis = new PgInputSlotEx();
	if(!pkThis)
	{
		return 0;
	}

	pkThis->m_uiUKey = uiUKey;
	pkThis->m_bKeepLastKey = bKeepLastKey;
	pkThis->m_bReset = bReset;

	int iIndex = 0;
	int iTotalNbOperator = 0;
	char aiOperatorList[MAX_KEY_SLOT] = {0};
	char const *pcOperator = "^|+*.";

	// 일단 모든 Operator에 대한 List를 얻는다.
	for(char pcChar = pcInputSequence[iIndex];
		(pcChar = pcInputSequence[iIndex]) != '\0';
		++iIndex)
	{
		if(isdigit(pcChar))
		{
			continue;
		}

		char pcCandidateOperator = 0;
		for(int iOpIndex = 0; (pcCandidateOperator = pcOperator[iOpIndex]) != '\0'; ++iOpIndex)
		{
			if(pcChar == pcCandidateOperator)
			{
				if(iTotalNbOperator < PgInputSlotEx::ms_iMaxKey)
				{
					aiOperatorList[iTotalNbOperator++] = pcChar;
				}
				else
				{
					// 제한된 형식이 아니면 파싱 중단.
					SAFE_DELETE(pkThis);
					return 0;
				}
			}
		}
	}

	// Operator로 Tokenizing해서 어떤 키를 어떻게 누르면 되는지 파싱한다.
	int iNbKeys = 0;
	char *pcNextToken = 0;
	char *pcToken = strtok_s(pcInputSequence, pcOperator, &pcNextToken);
	while(pcToken && iNbKeys < PgInputSlotEx::ms_iMaxKey)
	{
		pkThis->m_kKeySequences[iNbKeys++].m_uiKey = atoi(pcToken);
		pcToken = strtok_s(0, pcOperator, &pcNextToken);
	}

	pkThis->m_iNbKeys = iNbKeys;
	
	int iOperatorIndex = 0;
	while(iOperatorIndex < iTotalNbOperator)
	{
		char pcCombineType = aiOperatorList[iOperatorIndex++];
		switch(pcCombineType)
		{
		case '^':
			pkThis->m_eSlotType = DOUBLE_STROKE;
			break;
		case '|':
			pkThis->m_eSlotType = COMBINATION;
			break;
		case '+':
			pkThis->m_eSlotType = SEQUENCE;
			break;	
		case '*':
			pkThis->m_eSlotType = SUSTAIN;
			break;
		case '.':
			pkThis->m_iKeyOption = SUSTAIN;
			break;
		default:
			// 잘못된 오퍼레이터가 들어있으면 파싱 중단.
			SAFE_DELETE(pkThis);
			return 0;
			break;
		}
	}

	// Param설정
	if(pkThis->m_eSlotType != UNKNOWN)
	{
		pkThis->m_dwParam = dwParam;
	}

	return pkThis;
}


unsigned int PgLocalManager::PgInputSlotEx::ProcessExInput(PgInput &rkInput)
{
	PG_STAT(PgStatTimerF timerA(g_kLocalInputStatGroup.GetStatInfo("ProcessExInput"), g_pkApp->GetFrameCount()));
	PgInput *pkInput = &rkInput;

	for(int iIndex = 0; iIndex < m_iNbKeys; ++iIndex)
	{
		PgLocalManager::PgBaseInput *pkInputEx = &m_kKeySequences[iIndex];

		// 눌러진 키가, InputSlotEx에 등록된 키와 같다면
		if(pkInputEx->m_uiKey == rkInput.GetUKey())
		{
			if(!pkInputEx->m_iSequence && rkInput.GetPressed())
			{
				// 키 조합에 대해서 아무런 키도 눌러지지 않았고, 해당되는 키 중에 하나가 눌러졌으면
				pkInputEx->m_dwTimeStamp = BM::GetTime32();
				pkInputEx->m_iSequence = PgLocalManager::ms_uiLastSequence;
				pkInputEx->m_bPressing = true;
				//WriteToConsole("UKey : %u, Key : %u[%d], TimeStamp : %d, Seq = %d, Pressing = %s\n", m_uiUKey, pkInputEx->m_uiKey, iIndex, pkInputEx->m_dwTimeStamp, pkInputEx->m_iSequence, pkInputEx->m_bPressing ? "TRUE" : "FALSE");
				break;
			}
			else if(!rkInput.GetPressed() && pkInputEx->m_bPressing)
			{
				// 키가 떼어졌고, 이 키가 이전에 눌러졌었다면
				pkInputEx->m_bPressing = false;
				//WriteToConsole("UKey : %u, Key : %u[%d], Pressing = %s\n", m_uiUKey, pkInputEx->m_uiKey, iIndex, pkInputEx->m_bPressing ? "TRUE" : "FALSE");
				break;
			}
		}
		else if(m_eSlotType == SEQUENCE && !pkInputEx->m_iSequence)
		{
			break;
		}
	}

	bool bActivate = false;
	switch(m_eSlotType)
	{
	case DOUBLE_STROKE:
		bActivate = ProcessDoubleStrokeType();
		break;
	case COMBINATION:
		bActivate = ProcessCombinationType();
		break;
	case SEQUENCE:
		bActivate = ProcessSequenceType();
		break;
	case SUSTAIN:
		bActivate = ProcessSustainType();
		break;
	default:
		break;
	}

	if(!bActivate)
	{
		return 0;
	}

	Reset();

	return m_uiUKey;
}

void PgLocalManager::PgInputSlotEx::Reset()
{
	for(int iIndex = 0; iIndex < m_iNbKeys; ++iIndex)
	{
		PgLocalManager::PgBaseInput *pkInputEx = &m_kKeySequences[iIndex];
		pkInputEx->m_dwTimeStamp = 0;
		pkInputEx->m_iSequence = 0;
		pkInputEx->m_bPressing = 0;
	}
}

bool PgLocalManager::PgInputSlotEx::IsEmpty(int iIndex)
{
	if(iIndex >= PgInputSlotEx::ms_iMaxKey)
	{
		return false;
	}

	bool bEmpty = true;
	PgLocalManager::PgBaseInput *pkInputEx = &m_kKeySequences[iIndex];
	if(pkInputEx->m_dwTimeStamp != 0 ||
		pkInputEx->m_iSequence != 0 ||
		pkInputEx->m_bPressing != 0)
	{
		bEmpty = false;
	}

	return bEmpty;
}

bool PgLocalManager::PgInputSlotEx::KeepLastKey()
{
	return m_bKeepLastKey;
}

bool PgLocalManager::PgInputSlotEx::DoReset()
{
	return m_bReset;
}

void PgLocalManager::PgInputSlotEx::ShiftLeftKeySequence()
{
	m_kKeySequences[0] = m_kKeySequences[1];
	m_kKeySequences[1].Reset();
}

bool PgLocalManager::PgInputSlotEx::ProcessDoubleStrokeType()
{
	PgLocalManager::PgBaseInput *pkFirstInputEx = &m_kKeySequences[0];
	PgLocalManager::PgBaseInput *pkSecondInputEx = &m_kKeySequences[1];

	DWORD dwFirstInputTime = pkFirstInputEx->m_dwTimeStamp;
	DWORD dwSecondInputTime = pkSecondInputEx->m_dwTimeStamp;

	int iFirstInputSeq = pkFirstInputEx->m_iSequence;
	int iSecondInputSeq = pkSecondInputEx->m_iSequence;

	bool bExpired = false;

	if(iFirstInputSeq)
	{
		if(BM::GetTime32() - dwFirstInputTime > m_dwParam ||
			(dwSecondInputTime && 
			(dwSecondInputTime - dwFirstInputTime > m_dwParam || iSecondInputSeq - iFirstInputSeq != 1)))
		{
			bExpired = true;
			//WriteToConsole("Expired Slot Key : %u, SlotType : %s\n", m_uiUKey, m_eSlotType == DOUBLE_STROKE ? "DOUBLE_STROKE" 
			//														: m_eSlotType == COMBINATION ? "COMBINATION" :	\
			//															m_eSlotType == SUSTAIN ? "SUSTAIN" : "UNKNOWN");

		}
	}
	else
	{
		return false;
	}

	if(bExpired)
	{
		// 첫 번째 눌러진 키가 키 조합에 부합하지 않기 때문에, 왼쪽으로 밀어서 제거 해버린다.
		ShiftLeftKeySequence();
		return false;
	}

	if(dwSecondInputTime - dwFirstInputTime < m_dwParam &&
		iSecondInputSeq - iFirstInputSeq == 1)
	{
		bool bRet = (m_iKeyOption == SUSTAIN ? pkSecondInputEx->m_bPressing : !pkSecondInputEx->m_bPressing);
		if(bRet)
		{
			_PgOutputDebugString("Activate Slot(%d) : %u\n", g_pkApp->GetFrameCount(), m_uiUKey);
			return true;
		}
	}

	return false;
}

bool PgLocalManager::PgInputSlotEx::ProcessCombinationType()
{
	PgLocalManager::PgBaseInput *pkFirstInputEx = &m_kKeySequences[0];
	PgLocalManager::PgBaseInput *pkSecondInputEx = &m_kKeySequences[1];

	bool bSwap = false;
	if(pkFirstInputEx->m_iSequence != pkSecondInputEx->m_iSequence)
	{
		// 첫 번째 키가 눌러지지 않았거나, 두 개의 키가 다 눌러졋고 첫 번째 키가 더 나중에 눌러졌으면 스왑
		if(pkFirstInputEx->m_iSequence == 0 ||
			(pkFirstInputEx->m_iSequence > pkSecondInputEx->m_iSequence && pkSecondInputEx->m_iSequence))
		{
			bSwap = true;
		}
	}
	else
	{
		// 아무것도 안눌러 졌다.
		return false;
	}

	if(bSwap)
	{
		pkFirstInputEx = &m_kKeySequences[1];
		pkSecondInputEx = &m_kKeySequences[0];
	}

	DWORD dwFirstInputTime = pkFirstInputEx->m_dwTimeStamp;
	DWORD dwSecondInputTime = pkSecondInputEx->m_dwTimeStamp;

	bool bFirstInputPressing = pkFirstInputEx->m_bPressing;
	bool bSecondInputPressing = pkSecondInputEx->m_bPressing;

	bool bExpired = false;

	if(dwFirstInputTime)
	{
		if(!bFirstInputPressing ||
			BM::GetTime32() - dwFirstInputTime > m_dwParam)
		{
			bExpired = true;
		}
		else if(dwSecondInputTime && 
			(!bSecondInputPressing || dwSecondInputTime - dwFirstInputTime > m_dwParam))
		{
			bExpired = true;
		}
	}

	if(bExpired)
	{
		//WriteToConsole("Expired Slot Key : %u, SlotType : %s\n", m_uiUKey, m_eSlotType == DOUBLE_STROKE ? "DOUBLE_STROKE" :	\
		//																			m_eSlotType == COMBINATION ? "COMBINATION" :	\
		//																			m_eSlotType == SUSTAIN ? "SUSTAIN" : "UNKNOWN");
		Reset();
		return false;
	}

	// 어느 키를 먼저 눌렀든 상관 없다
	if(dwSecondInputTime - dwFirstInputTime < m_dwParam &&
		bFirstInputPressing &&  bSecondInputPressing)
	{
		_PgOutputDebugString("Activate Slot(%d) : %u\n", g_pkApp->GetFrameCount(), m_uiUKey);
		return true;
	}

	return false;
}

bool PgLocalManager::PgInputSlotEx::ProcessSequenceType()
{

	if(!g_pkLocalManager)
	{
		return	false;
	}

	PgInputSystem	*pkInputSystem = NiDynamicCast(PgInputSystem,g_pkLocalManager->GetInputSystem());
	if(!pkInputSystem)
	{
		return	false;
	}

	PgInputKeyboard	*pkKeyboard = (PgInputKeyboard*)pkInputSystem->GetKeyboard();
	if(!pkKeyboard)
	{
		return	false;
	}

	PgLocalManager::PgBaseInput *pkPrevInput = NULL;
	PgLocalManager::PgBaseInput	*pkCurrentInput = NULL;

	bool	bExpired = false;
	int	iIndex = 0;
	for(iIndex = 0; iIndex < m_iNbKeys; ++iIndex)
	{
		pkCurrentInput = &m_kKeySequences[iIndex];

		if(pkCurrentInput->m_iSequence == 0/* || (pkCurrentInput->m_bPressing && iIndex < m_iNbKeys-1)*/)
		{
			break;
		}

		if(pkPrevInput)
		{
			if(pkPrevInput->m_iSequence != pkCurrentInput->m_iSequence-1)
			{
				bExpired = true;
				break;
			}

			if(pkCurrentInput->m_dwTimeStamp - pkPrevInput->m_dwTimeStamp > m_dwParam)
			{
				bExpired = true;
				break;
			}
		}

		pkPrevInput = pkCurrentInput;

	}
	if(bExpired)
	{
		Reset();
	}

	if(!bExpired && iIndex<m_iNbKeys && pkCurrentInput && pkCurrentInput->m_iSequence != 0)
	{
		DWORD	dwTimeElapsed = BM::GetTime32() - pkCurrentInput->m_dwTimeStamp;
		if(dwTimeElapsed > m_dwParam)
		{
			bExpired = true;
		}
	}

	return	!bExpired && (iIndex == m_iNbKeys);


}

bool PgLocalManager::PgInputSlotEx::ProcessSustainType()
{
	PgLocalManager::PgBaseInput *pkFirstInputEx = &m_kKeySequences[0];
	DWORD kFirstInputTime = pkFirstInputEx->m_dwTimeStamp;
	int iFirstInputSeq = pkFirstInputEx->m_iSequence;

	return false;
}