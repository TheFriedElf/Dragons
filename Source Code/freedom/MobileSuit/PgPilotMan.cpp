#include "StdAfx.h"
#include "Variant/Global.h"
#include "Variant/PgEventView.h"
#include "PgMobileSuit.h"
#include "PgPilotMan.h"
#include "PgIWorldObject.h"
#include "PgPilot.h"
#include "PgActor.h"
#include "PgActorMonster.h"
#include "PgInput.h"
#include "PgNetwork.h"
#include "PgXmlLoader.h"
#include "mmsystem.h"
#include "PgRemoteManager.h"
#include "PgWorld.h"
#include "PgStat.h"
#include "Variant/PgPlayer.H"
#include "Variant/PgMonster.h"
#include "Variant/PgEntity.h"
#include "PgActionEffect.H"
#include "bm/point.h"
#include "PgProjectileMan.H"
#include "lwGUID.H"
#include "lwWorld.h"
#include "PgWorldObjectPool.H"
#include "PgActionPool.H"
#include "PgHelpSystem.h"
#include "PgAction.h"
#include "PgCustomUI_Summmoner.h"
#include "PgSelectStage.h"
#include "Variant/PgJumpingCharEventMgr.h"
#include "PgOption.h"

PgPilotMan::PgPilotMan() : PgIInputObserver(100)
	, m_uiStateCount(0)
	, m_bHideBalloon(false)
{
	m_dwLastActionBroadcastTime = 0;
	m_ulLast_S_KeyPressedTime = 0;
	m_kPlayerPilotGuid = BM::GUID::NullData();

	ReadClassID("ClassNo.xml");
	m_bLockMyInput = false;
	//m_pkFindPilotCache = NULL;
	m_iCacheHitCount = 0;
	m_pkPlayerPilot = NULL;
	m_pkPlayerUnit = NULL;
	m_pkPlayerActor = NULL;
	m_pkPlayerWorld = NULL;
	m_bSet = false;
}

PgPilotMan::~PgPilotMan()
{
	Destroy();
}

void	PgPilotMan::Destroy()
{
	{
		BM::CAutoMutex kLock(m_kRemoveLock, true);
		Container::iterator itr = m_kContPilot.begin();
		while(itr != m_kContPilot.end())
		{
			SAFE_DELETE_NI(itr->second);
			++itr;
		}
		m_kContPilot.clear();
	}

	ClearPlayerInfo();

	NILOG(PGLOG_MINOR, "[PgPilotMan] %d count cache hit\n", m_iCacheHitCount);
}

void PgPilotMan::ReadClassID(char const *pcXmlPath)
{
	TiXmlDocument kXmlDoc(pcXmlPath);
	if(!PgXmlLoader::LoadFile(kXmlDoc, UNI(pcXmlPath)))
	{
		PgError1("Parse Failed [%s]", pcXmlPath);
		return;
	}
	
	// Find Root
	const TiXmlElement *pkElement = kXmlDoc.FirstChildElement();
	PG_ASSERT_LOG(strcmp(pkElement->Value(), "CLASS") == 0);

	pkElement = pkElement->FirstChildElement();
	while(pkElement)
	{
		char const *pcTagName = pkElement->Value();

		if(strcmp(pcTagName, "ITEM") == 0)
		{
			const TiXmlAttribute *pkAttr = pkElement->FirstAttribute();

			int iNo = 0;
			std::string kActorID;
			std::string kPilotXmlPath;

			while(pkAttr)
			{
				std::string pcAttrName(pkAttr->Name());
				std::string pcAttrValue(pkAttr->Value());

				if(pcAttrName == "NO")
				{
					iNo = atoi(pcAttrValue.c_str());
				}
				else if(pcAttrName == "ID")
				{
					kActorID = pcAttrValue;
				}
				else if(pcAttrName == "PILOT_PATH")
				{
					kPilotXmlPath = pcAttrValue;
				}
				else
				{
					PgXmlError1(pkElement, "XmlParse: Incoreect Attr '%s'", pcAttrName);
				}

				pkAttr = pkAttr->Next();
			}

			if(!iNo || kActorID.empty())
			{
				PG_ASSERT_LOG(!"Not Enough Class Data");
			}
			else
			{
				m_kClassContainer.insert(std::make_pair(SClassGenderKey(0,iNo),stClassInfo(kActorID,kPilotXmlPath)));
			}
		}
		else
		{
			PgXmlError1(pkElement, "XmlParse: Incoreect Tag '%s'", pcTagName);
		}

		pkElement = pkElement->NextSiblingElement();
	}
}

bool PgPilotMan::ProcessInput(PgInput *pkInput)
{
	PG_ASSERT_LOG(pkInput);
	if (!pkInput)
	{
		return true;
	}

	// TODO : Frozen Pilot처리하는 부분, 코드가 중복이다. 깔끔하게 빼는 방법 없을까.
	PgPilot *pkPilot = 0;
	unsigned int uiUKey = pkInput->GetUKey();
	if(pkInput->IsRemote())
	{
		BM::Stream *pkPacket = pkInput->GetPacket();
		PG_ASSERT_LOG(pkPacket);
		
		int iMessage = uiUKey - PgInput::UR_REMOTE_BEGIN;
		switch(iMessage)
		{
		case PT_M_C_NFY_ACTION2:
			{
				//DWORD dwFirst = BM::GetTime32();
				//WriteToConsole("[SyncInfo] Synced Time : %u, Local : %u, [In.PilotMan] ", BM::GetTime32(), PgActor::GetSynchronizedTime());
				SActionInfo kActionInfo;
				kActionInfo.ReceiveFromServer(*pkPacket);
				
				pkPilot = FindPilot(kActionInfo.guidPilot);
				if(!pkPilot || pkPilot->IsFrozen())
				{
					if (pkPilot == NULL)
					{
						if (BufferReservedPilotPacket(kActionInfo.guidPilot, pkPacket, pkInput->GetUKey()))
						{
							return true;
						}
					}
					NILOG(PGLOG_NETWORK, "[PgPilotMan] %s pilot action(0x%0X) packet received but not process(%s)(%d)\n", MB(kActionInfo.guidPilot.str()), pkInput, pkPilot == NULL ? "pilot null" : "pilot frozen", g_pkApp->GetFrameCount());
					return true;
				}
		
				pkPilot->RecvNfyAction(kActionInfo, pkPacket);

				PgActorMonster* pActor = dynamic_cast<PgActorMonster*>(FindActor(kActionInfo.guidPilot));
				if( pActor )
				{
					pActor->DoSpeech(static_cast<EUnitState>(kActionInfo.iUnitState));
				}

				//WriteToConsole("ActionNo : %d, Delay : %u ActionInstanceNo : %d, app Time : %u\n", kActionInfo.iActionID, BM::GetTime32() - dwFirst, kActionInfo.iActionInstanceID, BM::GetTime32());
			}break;
		case PT_M_C_NFY_UPDATE_DIRECTION:
			{
				BM::GUID kPilotGuid;
				BYTE byDirection;
				DWORD dwDirectionTerm;
				POINT3 ptCurPos;

				pkPacket->Pop(kPilotGuid);
				pkPacket->Pop(byDirection);
				pkPacket->Pop(dwDirectionTerm);
				pkPacket->Pop(ptCurPos);

				_PgOutputDebugString("PT_M_C_NFY_UPDATE_DIRECTION dwDirectionTerm:%d\n",dwDirectionTerm);

				PgPilot *pkPilot = FindPilot(kPilotGuid);
				if(!pkPilot || pkPilot->IsFrozen())
				{
					if (pkPilot == NULL)
					{
						if(BufferReservedPilotPacket(kPilotGuid, pkPacket, pkInput->GetUKey()))
						{
							return true;
						}
					}
					NILOG(PGLOG_NETWORK, "[PgPilotMan] %s pilot action(0x%0X) packet received but not process(%s)(%d)\n", MB(kPilotGuid.str()), pkInput, pkPilot == NULL ? "pilot null" : "pilot frozen", g_pkApp->GetFrameCount());
					return true;
				}

				NiPoint3 kCurPos(ptCurPos.x, ptCurPos.y, ptCurPos.z);
				pkPilot->SetDirection(static_cast<Direction>((byDirection & 0xF0) >> 4), dwDirectionTerm, kCurPos);
			}break;
		case PT_M_C_NFY_PACTION:
			{
				g_kProjectileMan.Notify_PAction(pkPacket);
			}
			break;
		default:
			break;
		}			
	}
	else if(pkInput->IsLocal())
	{


		// Local Input
		if(m_bLockMyInput || pkInput->GetPilotGuid() == BM::GUID::NullData())
		{
			return true;
		}

		NiInputKeyboard::KeyCode const eKeyCode = static_cast<NiInputKeyboard::KeyCode>(pkInput->GetUKey() - PgInput::UR_LOCAL_BEGIN);
		switch(eKeyCode)
		{
		case NiInputKeyboard::KEY_SPACE:
			{
				if( !pkInput->GetPressed() && !XUIMgr.GetFocusedEdit() ) //Space키가 Up 상태면
				{
					PgActor* pkMyActor = GetPlayerActor();
					if(pkMyActor)
					{
						//액터 전용 UI
						if(g_kUnitCustomUI.CallMenu())
						{
							return true;
						}

						//SkillSet Cancel
						if(pkMyActor->SkillSetAction().ReserveActionCancel())
						{
							lwAddWarnDataTT(401620);
							return true;
						}
					}
				}
				else
				{
					if(g_kUnitCustomUI.GetShowMenu())
					{
						return true;
					}
					if( g_pkWorld )
					{//스페이스 바 눌렀을 경우 주변에 선택 가능한 NPC가 있는지 우선 체크
						PgIWorldObject *pkObject = g_pkWorld->PickObjectByKeyboard();
						if(pkObject)
						{
							if( PgIXmlObject::ID_NPC == pkObject->GetObjectID() )
							{
								return false;
							}
						}
					}
				}
			}break;
		case NiInputKeyboard::KEY_UP:
		case NiInputKeyboard::KEY_LEFT:
		case NiInputKeyboard::KEY_RIGHT:
		case NiInputKeyboard::KEY_DOWN:
			{
				if(g_kUnitCustomUI.GetShowMenu())
				{
					if(	!pkInput->GetPressed() )//Space키가 Up 상태면
					{
						Direction eDir = DIR_NONE;
						switch(eKeyCode)
						{
						case NiInputKeyboard::KEY_UP:		{ eDir = DIR_UP;		}break;
						case NiInputKeyboard::KEY_LEFT:		{ eDir = DIR_LEFT;		}break;
						case NiInputKeyboard::KEY_RIGHT:	{ eDir = DIR_RIGHT;		}break;
						case NiInputKeyboard::KEY_DOWN:		{ eDir = DIR_DOWN;		}break;
						}
						g_kUnitCustomUI.SelectMenu(eDir);
						return true;
					}
					return true;
				}
			}break;
		case 2001:
		case 2002:
		case 2013:
		case 2014:
			{
				if(g_kUnitCustomUI.GetShowMenu())
				{
					return true;
				}
			}break;
		default:
			{
				g_kUnitCustomUI.DoCancel();
			}break;
		}

#ifndef EXTERNAL_RELEASE
		// 내 Actor가 없거나, Frozen되어 있을 수 있으려나?
		if (pkInput->GetPilotGuid() == BM::GUID::NullData())
		{
			return false;
		}
		
		pkPilot = FindPilot(pkInput->GetPilotGuid());
		PG_WARNING_LOG(pkPilot == GetPlayerPilot());
#endif
		pkPilot = GetPlayerPilot();
		
		if(!pkPilot || pkPilot->IsFrozen())
		{
			PG_WARNING_LOG(!"PlayerPilot is frozen or null");
			if (pkPilot == NULL)
			{
				if (BufferReservedPilotPacket(pkInput->GetPilotGuid(), pkInput->GetPacket(), pkInput->GetUKey()))
				{
					return true;
				}
			}
			NILOG(PGLOG_NETWORK, "[PgPilotMan] %s pilot action packet received but not process(%s)(%d)\n", MB(pkInput->GetPilotGuid().str()), pkPilot == NULL ? "pilot null" : "pilot frozen", g_pkApp->GetFrameCount());
			return true;
		}


		PgActor	*pkActor = dynamic_cast<PgActor*>(pkPilot->GetWorldObject());
		if(!pkActor)
		{
			return false;
		}
		PgAction *pkCurrentAction = pkActor->GetAction();
		if(g_pkWorld && pkCurrentAction)
		{//콤보키 적용
			NiInputKeyboard	*pkKeyboard = g_pkLocalManager->GetInputSystem()->GetKeyboard();
			if( pkKeyboard)
			{
				if( 0 != strcmp("null", pkCurrentAction->GetParam(40001) ) )
				{
					int const iTat_Start =atoi(pkCurrentAction->GetParam(40001));
					if( 1 == iTat_Start )
					{
						int const iKeyNum = g_kGlobalOption.GetUKeyToKey(uiUKey);
						bool bKeyDown = pkKeyboard->KeyIsDown((NiInputKeyboard::KeyCode)iKeyNum);
						pkPilot->InsertInputKey( uiUKey, g_pkWorld->GetAccumTime(), bKeyDown);
						return true;
					}
				}
			}
		}

		PgAction *pkMyAction = pkActor->GetAction();
		if(!pkMyAction)
		{
			return false;
		}
		if(pkMyAction->IsChangeToNextActionOnNextUpdate())
		{
			return true;
		}
		if( 0 == strcmp("TRUE" , pkMyAction->GetScriptParam("USE_COMBO")) )
		{//콤보스킬은 여기서 처리하지 않는다.
			return false;
		}

		UpdateDirectionSlot(pkInput->GetUKey(),pkInput->GetPressed());
		
		PgAction *pkAction = pkPilot->CreateAction(pkInput);

		if(pkActor && pkAction && pkAction->GetEnable())
		{
			//	따라다니기 중지
			if( pkPilot->GetGuid() != pkActor->GetFollowingTargetGUID() )
			{
				pkActor->RequestFollowActor(pkActor->GetFollowingTargetGUID(),EFollow_Cancel);
			}
		}
		return pkPilot->ProcessAction(pkAction);
	}

	return true;
}

void PgPilotMan::ClearDirectionSlot(bool bReset)
{
	if(bReset)
	{
		for(DirectionSlot::iterator itr = m_kDirectionSlot.begin();
			itr != m_kDirectionSlot.end();
			++itr)
		{
			itr->second = 0;
		}
	}
	else
	{
		m_kDirectionSlot.clear();
	}	

}
void	PgPilotMan::UpdateDirectionSlot(unsigned int uiUKey,bool bEnable)
{

	BYTE	byDir = DIR_NONE;
	switch(uiUKey)
	{
	case	ACTIONKEY_LEFT:
		{
			byDir = DIR_LEFT;
		};
		break;
	case	ACTIONKEY_RIGHT:
		{
			byDir = DIR_RIGHT;
		};
		break;
	case	ACTIONKEY_UP:
		{
			byDir = DIR_UP;
		};
		break;
	case	ACTIONKEY_DOWN:
		{
			byDir = DIR_DOWN;
		};
		break;
	}

	_PgOutputDebugString("PgPilotMan::UpdateDirectionSlot uiUKey:%u bEnable:%d\n",uiUKey,bEnable);

	m_kDirectionSlot[byDir] = (bEnable ? ++m_uiStateCount : false);

	PgActor *pkActor = GetPlayerActor();
	if(pkActor)
	{
		pkActor->InvalidateDirection();
	}
}
unsigned int PgPilotMan::GetDirectionSlot(BYTE byDirection)
{
	DirectionSlot::iterator itr = m_kDirectionSlot.find(byDirection);
	if(itr != m_kDirectionSlot.end())
	{
		return itr->second;
	}

	return 0;
}
void	PgPilotMan::ModifyActionResult(PgPilot *pkCaster,int iActionInstanceID,PgActionTargetList &kTargetList,DWORD dwTimeStamp)
{
	PgPilot	*pkPilot;
	PgActor	*pkActor;
	for(ActionTargetList::iterator itor = kTargetList.begin(); itor != kTargetList.end(); ++itor)
	{
		pkPilot = FindPilot((*itor).GetTargetPilotGUID());
		if(pkPilot)
		{
			pkActor = dynamic_cast<PgActor *>(pkPilot->GetWorldObject());
			if(pkActor)
			{
				PgActionResult	*pkActionResult = &(*itor).GetActionResult();
				for(unsigned int i=0;i<pkActionResult->GetEffectNum(); ++i)
				{
					pkActor->AddEffect(pkActionResult->GetEffect(i),0, 0,pkCaster->GetGuid(), iActionInstanceID, dwTimeStamp, false);
				}
			}
		}
	}
}

bool PgPilotMan::GetClassInfo(int const iClass, int const iGender,stClassInfo &koutClassInfo)
{
	const SClassGenderKey kKey(iGender, iClass);

	ClassContainer::iterator itr = m_kClassContainer.find(kKey);
	if(itr != m_kClassContainer.end())
	{
		koutClassInfo = itr->second;
		return true;
	}
	else
	{
		const SClassGenderKey kSubKey(0, iClass);
		ClassContainer::iterator itr = m_kClassContainer.find(kSubKey);
		if(itr != m_kClassContainer.end())
		{
			char const *pcGender = (iGender == 1 ? "_male" : 
				iGender == 2 ? "_female" : "");
			std::string kActorID(itr->second.m_kActorID);
			std::string kPilotPath(itr->second.m_kPilotPath);
			kActorID += pcGender;

			if(kPilotPath.empty() == false)
			{
				kPilotPath += pcGender;
				kPilotPath += ".xml";
			}

			std::pair<ClassContainer::iterator, bool> iRet = m_kClassContainer.insert(std::make_pair(kKey, stClassInfo(kActorID,kPilotPath)));

			koutClassInfo = (iRet.first)->second;
			return	true;
		}
	}
	return false;
}

PgPilot *PgPilotMan::NewPilot(BM::GUID const &rkGuid, int iClass, int iGender, char const* pcObjectType)
{
	if(FindPilot(rkGuid))
	{
		NILOG(PGLOG_LOG, "[PgPilotMan] Request NewPilot(%s) but already exists.(%d,%d)\n", MB(rkGuid.str()), iClass, iGender);
		return 0;
	}

	stClassInfo	kClassInfo;
	if(!GetClassInfo(iClass,iGender,kClassInfo))
	{
		PgError2("[PgPilotMan] GetClassID failed(%d,%d)\n", iClass, iGender);
	//	VERIFY_INFO_LOG(false, BM::LOG_LV5, _T("[PgPilotMan] GetClassID failed(%d,%d)\n"), iClass, iGender);
		return 0;
	}

	//! xxx 임시로 만든 XmlObjectID
	PgIXmlObject::XmlObjectID eID = PgIXmlObject::ID_PC;

	//PgIWorldObject *pkWorldObject = dynamic_cast<PgIWorldObject *>(PgXmlLoader::CreateObject(pcActorID, (void*)&eID, NULL, pcObjectType));
	PgIWorldObject *pkWorldObject = g_kWorldObjectPool.CreateObject(kClassInfo.m_kActorID.c_str(),eID,pcObjectType);
	if(!pkWorldObject)
	{
		PgError4("[PgPilotMan] Create NewWorldObject(%s)(%s) failed(%d,%d)\n", MB(rkGuid.str()), kClassInfo.m_kActorID.c_str(), iClass, iGender);
	//	VERIFY_INFO_LOG(false, BM::LOG_LV5, _T("[PgPilotMan] Create NewWorldObject(%s)(%s) failed(%d,%d)\n"), rkGuid.str().c_str(), UNI(kClassInfo.m_kActorID.c_str()), iClass, iGender);
		return 0;
	}

	PgPilot *pkPilot = pkWorldObject->GetPilot();
	if(!pkPilot)
	{
		if(kClassInfo.m_kPilotPath.empty() == false)
		{
			pkPilot = dynamic_cast<PgPilot *>(PgXmlLoader::CreateObjectFromFile(kClassInfo.m_kPilotPath.c_str()));
		}

		if(!pkPilot)
		{
			// 따로 지정된 파일럿이 없어도 하나 있어야 한다.
			pkPilot = new PgPilot;
		}

		pkPilot->SetWorldObject(pkWorldObject);
		pkWorldObject->SetPilot(pkPilot);
	}

	pkPilot->SetObjectID(eID);
	pkWorldObject->SetObjectID(eID);

	CUnit *pkUnit = g_kTotalObjMgr.CreateUnit(UT_PLAYER, rkGuid);//임시로 유닛을 붙여주는것이다. 알아서 없어질꺼다.
	if(pkUnit)
	{
		pkUnit->CUnit::SetAbil(AT_CLASS, iClass);
		pkUnit->SetAbil(AT_GENDER, iGender);

		//SPlayerDBSimpleData	kSimpleData;
		//kSimpleData.iClass = iClass;
		//kSimpleData.wLv = 1;
		//kSimpleData.byGender = iGender;
		//kSimpleData.guidCharacter = rkGuid;

		//PgPlayer	*pkPlayer = (PgPlayer*)pkUnit;

		//pkPlayer->SetAbil(AT_ATTACK_SPEED,100);
		//pkPlayer->SetInfo(kSimpleData,NULL);

		pkPilot->SetUnit(pkUnit);
		//if( pkUnit->IsUnitType(UT_PLAYER) )
		//{// 플레이어의 경우에만
		//	PgActor* pkActor = dynamic_cast<PgActor*>( pkWorldObject );
		//	if(pkActor)
		//	{// 애니메이션을 모두 읽어 오고
		//		pkActor->PreLoadAllAnimation();
		//	}
		//}
	}
	else
	{
		_PgOutputDebugString("g_kTotalObjMgr.CreateUnit(UT_PLAYER, rkGuid) Failed\n");
		NILOG(PGLOG_WARNING, "[PgPilotMan] pkUnit is NULL\n");
		return NULL;
	}

	if(!g_iUseAddUnitThread)
	{//스레드 아니면 직접 넣음.
		BM::CAutoMutex kLock(m_kRemoveLock, true);
		std::pair<Container::iterator, bool> ret = m_kContPilot.insert(std::make_pair(rkGuid, pkPilot));
		if(!ret.second)//이미 있음.
		{
			NILOG(PGLOG_WARNING, "[PgPilotMan] %s pilot is already exist in list - By NoThread\n", MB(rkGuid.str()));
			assert(NULL);
		}
	}

	return pkPilot;
}

bool PgPilotMan::InsertPilot(BM::GUID const &rkGuid, PgPilot* pkPilot)
{
	PG_ASSERT_LOG(g_iUseAddUnitThread == 1);
	BM::CAutoMutex kLock(m_kRemoveLock, true);
	std::pair<Container::iterator, bool> ret = m_kContPilot.insert(std::make_pair(rkGuid, pkPilot));
	if(!ret.second )
	{
		NILOG(PGLOG_WARNING, "[PgPilotMan] %s pilot is already exist in list - By Thread\n", MB(rkGuid.str()));
		return false;
	}
	return true;
}

bool PgPilotMan::InsertReservedPilot( BM::GUID const &rkGuid, EUnitType const kUnitType )
{
	BM::CAutoMutex kLock(m_kReserveLock, true);
	ReservedPilot::iterator iter = m_kReservedPilot.find(rkGuid);
	if (iter != m_kReservedPilot.end())
	{
		// 이미 있어?
		NILOG(PGLOG_WARNING, "[PgPilotMan] %s pilot is already exist in reserved list\n", MB(rkGuid.str()));
		return false;
	}

	std::pair<ReservedPilot::iterator, bool> ret = m_kReservedPilot.insert(std::make_pair(rkGuid, SReservedPilot(kUnitType)));
	if (ret.second == false)
	{
		// 실패하면 어쩌라구;
		NILOG(PGLOG_WARNING, "[PgPilotMan] %s pilot insert to reserved list failed\n", MB(rkGuid.str()));
		return false;
	}

	NILOG(PGLOG_LOG, "[PgPilotMan] InsertReservedPilot : %s\n", MB(rkGuid.str()));
	return true;
}

bool PgPilotMan::RemoveReservedPilot(BM::GUID const &rkGuid, bool bRemoveFromWorldAddUnitQueue)
{
	_PgOutputDebugString("PgPilotMan::RemoveReservedPilot() GUID : %s\n",MB(rkGuid.str()));

	bool ret = false;
	BM::CAutoMutex kLock(m_kReserveLock, true);
	ReservedPilot::iterator iter = m_kReservedPilot.find(rkGuid);
	if (iter != m_kReservedPilot.end())
	{
		iter->second.Release();	
		m_kReservedPilot.erase(iter);
		ret = true;
		NILOG(PGLOG_LOG, "[PgPilotMan] RemoveReservedPilot : %s\n", MB(rkGuid.str()));
	}
	else
	{
		NILOG(PGLOG_WARNING, "[PgPilotMan] RemoveReservedPilot : %s pilot is not inside list\n", MB(rkGuid.str()));
	}	
/*
	if (bRemoveFromWorldAddUnitQueue)
	{
		g_kWorldLock);
		if (g_pkWorld)
		{
			NILOG(PGLOG_LOG, "[PgPilotMan] RemoveReservedPilot from world AddUnitQueue: %s\n", MB(rkGuid.str()));
			g_pkWorld->RemoveUnitFromAddUnitQueue(rkGuid);
		}
		g_kWorldLock.UnlockReader();
	}
*/
	return ret;
}

void PgPilotMan::ClearReservedPilot()
{
	BM::CAutoMutex kLock(m_kReserveLock, true);
	ReservedPilot::iterator iter = m_kReservedPilot.begin();
	for(;iter != m_kReservedPilot.end(); ++iter)
	{
		iter->second.Release();	
	}
	m_kReservedPilot.clear();
}

bool PgPilotMan::IsReservedPilot( BM::GUID const &rkGuid, EUnitType *pOutUnitType )const
{
	bool ret = false;
	BM::CAutoMutex kLock(m_kReserveLock);
	ReservedPilot::const_iterator iter = m_kReservedPilot.find(rkGuid);
	if (iter != m_kReservedPilot.end())
	{
		ret = true;
		if ( pOutUnitType )
		{
			*pOutUnitType = iter->second.kUnitType;
		}
	}
	NILOG(PGLOG_LOG, "[PgPilotMan] IsReservedPilot(%d) : %s\n", ret, MB(rkGuid.str()));
	return ret;
}

bool PgPilotMan::BufferReservedPilotPacket(BM::GUID const &rkGuid, BM::Stream* pPacket, unsigned int ukey )
{
	if (pPacket == NULL)
		return false;
{
	BM::CAutoMutex kLock(m_kReserveLock);
	ReservedPilot::iterator iter = m_kReservedPilot.find(rkGuid);
	if (iter != m_kReservedPilot.end())
	{
		pPacket->PosAdjust();
		BM::Stream* pNewPacket = new BM::Stream( *pPacket );
		iter->second.kPacketBuffer.push_back(pNewPacket);	// Buffering자체는 mainthread에서만 하므로 buffer를 lock걸 필요는 없다.
		NILOG(PGLOG_LOG, "[PgPilotMan] BufferReservedPilotPacket(%d,true) : %s\n", ukey, MB(rkGuid.str()));
	}
	else
	{
		NILOG(PGLOG_WARNING, "[PgPilotMan] BufferReservedPilotPacket(%d,false) : %s\n", ukey, MB(rkGuid.str()));
		return false;
	}
}

	SetFirstPacketBufferedTime(rkGuid);
	return true;
}

void	PgPilotMan::SetFirstPacketBufferedTime(BM::GUID const &rkGuid)
{
	PacketBufferTimeCont::iterator itor = m_kFirstPacketBufferedTimeCont.find(rkGuid);
	if(itor == m_kFirstPacketBufferedTimeCont.end())
	{
		m_kFirstPacketBufferedTimeCont.insert(std::make_pair(rkGuid,NiGetCurrentTimeInSec()));
	}
}

float PgPilotMan::GetFirstPacketBufferedTime(BM::GUID const &rkGuid)
{
	PacketBufferTimeCont::iterator itor = m_kFirstPacketBufferedTimeCont.find(rkGuid);
	if(itor != m_kFirstPacketBufferedTimeCont.end())
	{
		float fTime = itor->second;
		return fTime;
	}

	return	-1.0f;
}
bool PgPilotMan::FlushReservedPilotPacket(BM::GUID const &rkGuid)
{
	bool ret = false;
	unsigned int packetNum = 0;
{
	BM::CAutoMutex kLock(m_kReserveLock);
	ReservedPilot::iterator iter = m_kReservedPilot.find(rkGuid);
	if (iter != m_kReservedPilot.end())
	{
		packetNum = iter->second.kPacketBuffer.size();
		PacketBuffer::iterator packetIter = iter->second.kPacketBuffer.begin();
		while (packetIter != iter->second.kPacketBuffer.end())
		{
			g_pkRemoteManager->AddPacket( *(*packetIter) );
			SAFE_DELETE(*packetIter);
			++packetIter;
		}
		iter->second.kPacketBuffer.clear();
		ret = true;
		NILOG(PGLOG_LOG, "[PgPilotMan] FlushReservedPilotPacket(%d)(%d count) : %s\n", ret, packetNum, MB(rkGuid.str()));
	}
	else
	{
		NILOG(PGLOG_WARNING, "[PgPilotMan] FlushReservedPilotPacket(%d)(%d count) : %s\n", ret, packetNum, MB(rkGuid.str()));
	}
	
}
	m_kFirstPacketBufferedTimeCont.erase(rkGuid);
	return ret;
}

PgPilot *PgPilotMan::FindPilot(std::wstring &kName, bool bFindPassPlayerPilot)
{
	PgPilot* pkPlayer = NULL;
	if(bFindPassPlayerPilot)
	{
		pkPlayer = GetPlayerPilot();
	}

	PG_STAT(PgStatTimerF timer(g_kPilotStatGroup.GetStatInfo("PilotMan.FindPilot(std::wstring &kName)"), g_pkApp->GetFrameCount()));
	//if (m_pkFindPilotCache && m_pkFindPilotCache->GetName() == kName && pkPlayer != m_pkFindPilotCache)
	//{
	//	m_iCacheHitCount++;
	//	return m_pkFindPilotCache;
	//}

	BM::CAutoMutex kLock(m_kRemoveLock);
	for(Container::iterator itr = m_kContPilot.begin(); itr != m_kContPilot.end(); ++itr)
	{
		PgPilot	*pkPilot = itr->second;
		if(pkPilot->GetName() == kName && pkPlayer != pkPilot)
		{
			//m_pkFindPilotCache = itr->second;
			return	pkPilot;
		}
	}
	return	NULL;
}
PgPilot *PgPilotMan::FindPilot(BM::GUID const &rkGuid)const
{
	PG_STAT(PgStatTimerF timer(g_kPilotStatGroup.GetStatInfo("PilotMan.FindPilot"), g_pkApp->GetFrameCount()));
	//if (m_pkFindPilotCache && m_pkFindPilotCache->GetGuid() == rkGuid)
	//{
	//	m_iCacheHitCount++;
	//	return m_pkFindPilotCache;
	//}

	BM::CAutoMutex kLock(m_kRemoveLock);
	return Unlocked_FindPilot(rkGuid);
}

PgPilot* PgPilotMan::Unlocked_FindPilot(BM::GUID const &rkGuid)const
{
	Container::const_iterator itr = m_kContPilot.find(rkGuid);	
	if(itr == m_kContPilot.end())
	{
		return 0; 
	}
	
	return itr->second;
}
PgPilot *PgPilotMan::FindPilotInRange(EUnitType eType, NiPoint3 const& ptPos, int const iRange)
{
	BM::CAutoMutex kLock(m_kRemoveLock);
	return Unlocked_FindPilotInRange(eType, ptPos, iRange);
}

PgPilot *PgPilotMan::Unlocked_FindHouse(BM::GUID const &rkOnwerGuid)
{
	Container::iterator itr = m_kContPilot.begin();
	for(; itr != m_kContPilot.end(); ++itr )
	{
		PgPilot* pkPilot = itr->second;
		if( pkPilot )
		{
			PgMyHome* pkMyHome = dynamic_cast<PgMyHome*>(pkPilot->GetUnit());
			if( pkMyHome )
			{
				if( pkMyHome->OwnerGuid() == rkOnwerGuid )
				{
					return pkPilot;
				}
			}
		}
	}
	
	return NULL;
}
PgPilot *PgPilotMan::FindHouse(BM::GUID const &rkOnwerGuid)
{
	BM::CAutoMutex kLock(m_kRemoveLock);
	return Unlocked_FindHouse(rkOnwerGuid);
}
PgPilot *PgPilotMan::Unlocked_FindPilotInRange(EUnitType eType, NiPoint3 const& ptPos, int const iRange)
{
	Container::iterator itr = m_kContPilot.begin();
	for( ; itr != m_kContPilot.end(); ++itr )
	{
		PgPilot* pkPilot = itr->second;
		if(pkPilot)
		{
			CUnit* pkUnit = pkPilot->GetUnit();
			if(pkUnit)
			{
				if( pkUnit->UnitType() == eType )
				{
					PgActor* pkActor = dynamic_cast<PgActor*>(pkPilot->GetWorldObject());	//유닛과 실제 액터랑 위치 차이가 엄청남. 클라이언트 기준이므로 액터 중심으로 검사
					if(NULL==pkActor)	{return NULL;}
					NiPoint3 const ptUnitPos = pkActor->GetPos();
					NiPoint3 kLen = ptUnitPos - ptPos;
					float iDis = kLen.Length();
					if( iDis < iRange)
					{
						return pkPilot;
					}
				}
			}
		}
	}

	return NULL;
}

bool PgPilotMan::FindUnit(EUnitType const eType, UNIT_PTR_ARRAY& rkUnitArray)const
{
    Container::const_iterator itr = m_kContPilot.begin();
	for( ; itr != m_kContPilot.end(); ++itr )
	{
		PgPilot* pkPilot = itr->second;
		if(pkPilot)
		{
			CUnit* pkUnit = pkPilot->GetUnit();
			if(pkUnit && pkUnit->IsUnitType(eType))
			{
                rkUnitArray.Add(pkUnit);
			}
		}
	}

	return !rkUnitArray.empty();
}

bool PgPilotMan::FindUnitInRange(EUnitType eType, NiPoint3 const& ptPos, int const iRange, UNIT_PTR_ARRAY& rkUnitArray)
{
	BM::CAutoMutex kLock(m_kRemoveLock);
	return 0<Unlocked_FindUnitInRange(eType, ptPos, iRange, rkUnitArray);
}

size_t PgPilotMan::Unlocked_FindUnitInRange(EUnitType eType, NiPoint3 const& ptPos, int const iRange, UNIT_PTR_ARRAY& rkUnitArray)
{
	Container::iterator itr = m_kContPilot.begin();
	for( ; itr != m_kContPilot.end(); ++itr )
	{
		PgPilot* pkPilot = itr->second;
		if(pkPilot)
		{
			CUnit* pkUnit = pkPilot->GetUnit();
			if(pkUnit)
			{
				if( pkUnit->UnitType() == eType )
				{
					PgActor* pkActor = dynamic_cast<PgActor*>(pkPilot->GetWorldObject());	//유닛과 실제 액터랑 위치 차이가 엄청남. 클라이언트 기준이므로 액터 중심으로 검사
					if(NULL==pkActor)	{return NULL;}
					NiPoint3 const ptUnitPos = pkActor->GetPos();
					NiPoint3 kLen = ptUnitPos - ptPos;
					float iDis = kLen.Length();
					if( iDis < iRange)
					{
						rkUnitArray.Add(pkUnit);
					}
				}
			}
		}
	}

	return rkUnitArray.size();
}

PgActor* PgPilotMan::FindActor(BM::GUID const &rkGuid)
{
	PgPilot *pkPilot = FindPilot(rkGuid);
	if ( pkPilot )
	{
		return dynamic_cast<PgActor*>(pkPilot->GetWorldObject());
	}
	return NULL;
}

PgActor* PgPilotMan::FindActorByClassNo(int const iClass)
{
	stClassInfo	kClassInfo;
	if(!GetClassInfo(iClass,0,kClassInfo))
	{
		return NULL;
	}

	for(Container::iterator itr = m_kContPilot.begin(); itr != m_kContPilot.end(); ++itr)
	{
		PgPilot	*pkPilot = itr->second;
		if(pkPilot)
		{
			PgActor* pkActor = dynamic_cast<PgActor*>(pkPilot->GetWorldObject());
			if(pkActor)
			{
				if(pkActor->GetID() == kClassInfo.m_kActorID)
				{
					return pkActor;
				}
			}
		}
	}

	return NULL;
}

PgActor* PgPilotMan::FindNpcActor(std::wstring const& kActorName) const
{
	for(Container::const_iterator kPilot_itor = m_kContPilot.begin(); kPilot_itor != m_kContPilot.end(); ++kPilot_itor)
	{
		PgPilot	*pkPilot = kPilot_itor->second;
		if(pkPilot)
		{			
			PgNpc* pkNpc = dynamic_cast<PgNpc*>(pkPilot->GetUnit());
			if(pkNpc)
			{
				if(pkNpc->ActorName() == kActorName)
				{
					PgActor* pkActor = dynamic_cast<PgActor*>(pkPilot->GetWorldObject());
					return pkActor;
				}
			}
		}
	}
	return	NULL;
}

PgPilot* PgPilotMan::GetPlayerPilot()const
{
#ifndef EXTERNAL_RELEASE
	if (m_bSet)
	{
		PG_ASSERT_LOG(m_pkPlayerPilot);
		PG_ASSERT_LOG(m_pkPlayerActor);
		PG_ASSERT_LOG(m_pkPlayerWorld);
		PG_ASSERT_LOG(m_pkPlayerUnit);
		PG_ASSERT_LOG(FindPilot(PlayerPilotGuid()) == m_pkPlayerPilot);
		PG_ASSERT_LOG(m_pkPlayerPilot && m_pkPlayerPilot->GetWorldObject() == m_pkPlayerActor);
		PG_ASSERT_LOG(m_pkPlayerPilot && m_pkPlayerPilot->GetUnit() == m_pkPlayerUnit);
	}
#endif
	return m_pkPlayerPilot;
}

PgPlayer* PgPilotMan::GetPlayerUnit()const
{
#ifndef EXTERNAL_RELEASE
	if (m_bSet)
	{
		PG_ASSERT_LOG(m_pkPlayerPilot);
		PG_ASSERT_LOG(m_pkPlayerActor);
		PG_ASSERT_LOG(m_pkPlayerWorld);
		PG_ASSERT_LOG(m_pkPlayerUnit);
		PG_ASSERT_LOG(FindPilot(PlayerPilotGuid()) == m_pkPlayerPilot);
		PG_ASSERT_LOG(m_pkPlayerPilot && m_pkPlayerPilot->GetWorldObject() == m_pkPlayerActor);
		PG_ASSERT_LOG(m_pkPlayerPilot && m_pkPlayerPilot->GetUnit() == m_pkPlayerUnit);
	}
#endif
	return m_pkPlayerUnit;
}

PgActor* PgPilotMan::GetPlayerActor()const
{
#ifndef EXTERNAL_RELEASE
	if (m_bSet)
	{
		PG_ASSERT_LOG(m_pkPlayerPilot);
		PG_ASSERT_LOG(m_pkPlayerActor);
		PG_ASSERT_LOG(m_pkPlayerWorld);
		PG_ASSERT_LOG(m_pkPlayerUnit);
		PG_ASSERT_LOG(FindPilot(PlayerPilotGuid()) == m_pkPlayerPilot);
		PG_ASSERT_LOG(m_pkPlayerPilot && m_pkPlayerPilot->GetWorldObject() == m_pkPlayerActor);
		PG_ASSERT_LOG(m_pkPlayerPilot && m_pkPlayerPilot->GetUnit() == m_pkPlayerUnit);
	}
#endif
	return m_pkPlayerActor;
}

PgWorld* PgPilotMan::GetPlayerWorld()const
{
#ifndef EXTERNAL_RELEASE
	if (m_bSet)
	{
		PG_ASSERT_LOG(m_pkPlayerPilot);
		PG_ASSERT_LOG(m_pkPlayerActor);
		PG_ASSERT_LOG(m_pkPlayerWorld);
		PG_ASSERT_LOG(FindPilot(PlayerPilotGuid()) == m_pkPlayerPilot);
		PG_ASSERT_LOG(m_pkPlayerPilot && m_pkPlayerPilot->GetWorldObject() == m_pkPlayerActor);
		PG_ASSERT_LOG(m_pkPlayerPilot && m_pkPlayerPilot->GetUnit() == m_pkPlayerUnit);
		PG_ASSERT_LOG(m_pkPlayerActor && m_pkPlayerActor->GetWorld() == m_pkPlayerWorld);
	}
#endif
	if (m_pkPlayerActor)
		return m_pkPlayerActor->GetWorld();
	return NULL;
}

bool PgPilotMan::RemovePilot(BM::GUID const &rkGuid)
{
	Container::iterator itr;
{
	BM::CAutoMutex kLock(m_kRemoveLock);
	itr= m_kContPilot.find(rkGuid);

	if (itr == m_kContPilot.end())
	{
		return false;
	}
}
{
	BM::CAutoMutex kLock(m_kRemoveLock, true);

	if (itr->second == m_pkPlayerPilot)
	{
		NILOG(PGLOG_LOG, "[PgPilotMan] RemovePilot(my actor,%s)\n", MB(rkGuid.str()));
		m_pkPlayerPilot = 0;
		m_pkPlayerActor = 0;
		m_pkPlayerWorld = 0;
		m_pkPlayerUnit = 0;
		m_bSet = false;
	}

	//if (itr->second == m_pkFindPilotCache)
	//{
	//	m_pkFindPilotCache = NULL;
	//}

	PgPilot* removingPilot = itr->second;
	m_kContPilot.erase(itr);

	PG_ASSERT_LOG(removingPilot);
	SAFE_DELETE_NI(removingPilot);

	return true;
}
}

bool PgPilotMan::GetPlayerPilotGuid(BM::GUID &rkPilotGuid_out)const
{
	if (m_kPlayerPilotGuid != BM::GUID::NullData())
	{
#ifndef EXTERNAL_RELEASE
		BM::CAutoMutex kLock(m_kRemoveLock);
		Container::const_iterator itr = m_kContPilot.find(m_kPlayerPilotGuid);
		bool bInside = itr == m_kContPilot.end();

		if (bInside)
		{
			if (IsReservedPilot(m_kPlayerPilotGuid) == false)
			{
				PG_ASSERT_LOG(NULL);
				NILOG(PGLOG_WARNING, "[PgPilotMan] GetPlayerPilotGuid : %s player is not exist in list\n", MB(m_kPlayerPilotGuid.str()));
				return false;
			}
		}

		if (itr->second->GetGuid() != m_kPlayerPilotGuid)
		{
			PG_ASSERT_LOG(NULL);
			NILOG(PGLOG_WARNING, "[PgPilotMan] GetPlayerPilotGuid : player is not equal(%s, %s)\n", MB(m_kPlayerPilotGuid.str()), MB(itr->second->GetGuid().str()));
			return false;
		}
#endif
		rkPilotGuid_out = m_kPlayerPilotGuid;
		return true;		
	}	
	return false;
}

bool PgPilotMan::IsMyPlayer(BM::GUID const &rkGuid)const
{
	// QUESTION: 둘다 NullData일때를 비교해야 하는가?
	if (m_kPlayerPilotGuid == rkGuid)
	{
		PG_ASSERT_LOG(m_bSet == false || m_kPlayerPilotGuid != BM::GUID::NullData());
		return true;
	}

	return false;
}

bool PgPilotMan::IsMySummoned(CUnit * pkUnit)const
{
	if(m_pkPlayerUnit && pkUnit && pkUnit->IsUnitType(UT_SUMMONED))
	{
		VEC_SUMMONUNIT const& kContSummonUnit = m_pkPlayerUnit->GetSummonUnit();
		PgPilot * pkPilot = NULL;
		for(VEC_SUMMONUNIT::const_iterator c_it=kContSummonUnit.begin(); c_it!=kContSummonUnit.end(); ++c_it)
		{
			if(pkUnit->GetID() == (*c_it).kGuid)
			{
				return true;
			}
		}
	}

	return false;
}

bool PgPilotMan::IsMySummoned(BM::GUID const& rkGuid)const
{
	if(m_pkPlayerUnit && rkGuid.IsNotNull())
	{
		VEC_SUMMONUNIT const& kContSummonUnit = m_pkPlayerUnit->GetSummonUnit();
		PgPilot * pkPilot = NULL;
		for(VEC_SUMMONUNIT::const_iterator c_it=kContSummonUnit.begin(); c_it!=kContSummonUnit.end(); ++c_it)
		{
			//소환체인지 검사를 할 필요가 있을까?
			if(rkGuid == (*c_it).kGuid)
			{
				return true;
			}
		}
	}

	return false;
}

void PgPilotMan::ClearPlayerInfo()
{

	if( BM::GUID::IsNotNull(m_kPlayerPilotGuid) )
	{
		g_kHelpSystem.SaveHelpInfoFile(m_kPlayerPilotGuid);
	}

	m_dwLastActionBroadcastTime = 0;
	m_ulLast_S_KeyPressedTime = 0;
	m_kPlayerPilotGuid = BM::GUID::NullData();

	m_kLockedType.clear();
	m_kActionLockedType.clear();
	m_bLockMyInput = false;
	//m_pkFindPilotCache = NULL;
	m_pkPlayerPilot = NULL;
	m_pkPlayerUnit = NULL;
	m_pkPlayerActor = NULL;
	m_pkPlayerWorld = NULL;
	m_bSet = false;
}

bool PgPilotMan::SetPlayerPilot(BM::GUID const &rkGuid)
{
	if(rkGuid != BM::GUID::NullData())
	{
		m_kPlayerPilotGuid = rkGuid;
	}
	else
	{
		PG_ASSERT_LOG(NULL);
	}
	
	{
		BM::CAutoMutex kLock(m_kRemoveLock);
		Container::iterator itr = m_kContPilot.find(rkGuid);
		if (itr == m_kContPilot.end())
		{
			NILOG(PGLOG_ERROR, "[PgPilotMan] SetPlayerPilot: player is not exit(%s)\n", MB(m_kPlayerPilotGuid.str()));
			return false;
		}
		else
		{
			m_pkPlayerPilot = itr->second;
		}
	}

	
	PG_ASSERT_LOG(m_pkPlayerPilot);
	if (m_pkPlayerPilot)
	{
		m_pkPlayerUnit = dynamic_cast<PgPlayer*>(m_pkPlayerPilot->GetUnit());
		PG_ASSERT_LOG(m_pkPlayerUnit);
		m_pkPlayerActor = dynamic_cast<PgActor*>(m_pkPlayerPilot->GetWorldObject());
		PG_ASSERT_LOG(m_pkPlayerActor);
		if (m_pkPlayerActor)
		{
			m_pkPlayerWorld = m_pkPlayerActor->GetWorld();
			if(IS_CLASS_LIMIT(UCLIMIT_DRAGON_FIGHTER, m_pkPlayerUnit->GetAbil(AT_CLASS)) )
			{// 드래곤 파이터만 애니 선로딩 
				m_pkPlayerActor->PreLoadAllAnimation();			// 내 플레이어 애니를 선로딩 해둔다(액션할떄 애니 로딩으로 인한 끊김 없이 하기위해)
			}
		}
	}
	else
	{
		return false;
	}

	PgPilot::PlayerPilotGuid(rkGuid);
	m_bSet = true;

	m_pkPlayerPilot->ActivateExtendedSlot();

	ClearDirectionSlot(true);

	if( BM::GUID::IsNotNull(rkGuid) )
	{
		g_kHelpSystem.LoadHelpInfoFile(rkGuid);
	}

	if(m_pkPlayerUnit && m_pkPlayerActor)
	{
		m_pkPlayerActor->SkillSetAction().ContSkillSet( m_pkPlayerUnit->GetMySkill()->ContSkillSet() );
	}

	if( g_pkWorld )
	{
		g_pkWorld->ProcessNpcEvent();
	}

	return true;
}

bool PgPilotMan::BroadcastDirection(PgPilot *pkPilot, BYTE byDirection)
{
	if(!pkPilot)
	{
		return false;
	}

	DWORD dwNow = BM::GetTime32();
	DWORD dwActionTerm = 0;
	if(m_dwLastActionBroadcastTime != 0)
	{
		dwActionTerm = dwNow - m_dwLastActionBroadcastTime;
	}
		
	BM::Stream kPacket(PT_C_M_REQ_UPDATE_DIRECTION);
	
	POINT3BY ptDirection;
	POINT3 kCurPos;
	POINT3BY ptPathNormal(0, 0, 0);

	//방향은 0~15 값을 사용하므로 4바이트로 해결된다.
	//패킷을 늘리지 않고 2개의 방향을 하나의 BYTE에 대해서 처리
	PgActor* pkActor = dynamic_cast<PgActor *>(pkPilot->GetWorldObject());
	if (pkActor != NULL)
	{
		BYTE btNewDirection = byDirection << 4;

		// 하위 4바이트 바라보는 Direction
		if(pkActor->IsLockBidirection())
		{
			BYTE btDirection = btDirection = pkActor->GetDirFromMovingVector(pkActor->GetLookingDir());
			btNewDirection |= btDirection;
		}
		else
		{
			btNewDirection |= btNewDirection >> 4;
		}

		kPacket.Push(btNewDirection);

		NiPoint3& rkPoint3 = pkActor->GetDirectionVector(byDirection);
		POINT3_2_POINT3BY(POINT3(rkPoint3.x, rkPoint3.y, rkPoint3.z), ptDirection);
		NiPoint3 kActorPos = pkActor->GetPosition(true);
		kCurPos.Set(kActorPos.x, kActorPos.y, kActorPos.z);

		const NiPoint3& rkPathNormal = pkActor->GetPathNormal();
		POINT3_2_POINT3BY(POINT3(rkPathNormal.x, rkPathNormal.y, 0), ptPathNormal);
	}
	else
	{
	//	VERIFY_INFO_LOG(false, BM::LOG_LV5, _T("[%s] Cannot get MyActor Guid[%d]"), __FUNCTIONW__, pkPilot->GetGuid().str().c_str());
		kPacket.Push(byDirection);
		ptDirection.Set(0, 0, 0);
		ptPathNormal.Set(0, 0, 0);
	}

	kPacket.Push(ptDirection);
	kPacket.Push(POINT2BY(ptPathNormal.x, ptPathNormal.y));
	kPacket.Push(dwActionTerm);
	kPacket.Push(kCurPos);	


	PgActionEntity kActionEntity(0, byDirection);
	kActionEntity.SetDirectionTerm(dwActionTerm);
	kActionEntity.SetDirectionStartPos(NiPoint3(kCurPos.x,kCurPos.y,kCurPos.z));

	pkActor->AddActionEntityToFollowers(kActionEntity);

	NETWORK_SEND(kPacket)

	_PgOutputDebugString("[PgPilotMan.BroadcastDirection(Monitor)] Direction : %d , ActionTerm : %u\n", byDirection, dwActionTerm);
	return true;
}

void PgPilotMan::BroadcastSimulate(PgPilot *pkPilot, bool bSimulate)
{
	if(!pkPilot)
	{
		return;
	}

	PgActor *pkActor = dynamic_cast<PgActor *>(pkPilot->GetWorldObject());
	if(!pkActor)
	{
		return;
	}

	NiPoint3 kPos = pkActor->GetPosition();
	BM::Stream kPacket(PT_C_M_REQ_ACTOR_SIMULATE);
	kPacket.Push(bSimulate);
	kPacket.Push(POINT3(kPos.x, kPos.y, kPos.z - PG_CHARACTER_Z_ADJUST));
	NETWORK_SEND(kPacket)
}

bool PgPilotMan::Broadcast(PgPilot *pkPilot, PgAction *pkAction, bool bForceActionTermToZero)
{
	PG_ASSERT_LOG(pkPilot);
	PG_ASSERT_LOG(pkAction);
	
	if(!pkPilot || !pkAction)
	{
		return false;
	}

	int iActionNo = pkAction->GetActionNo();
	int iActionInstanceID = pkAction->GetActionInstanceID();
	BYTE byActionParam = pkAction->GetActionParam();
	
	PG_WARNING_LOG(iActionNo>0);
	PG_ASSERT_LOG(iActionInstanceID>=0);

	if(iActionNo <= 0)
	{
		return false;
	}

	DWORD dwActionBroadCastTerm = BM::GetTime32() - m_dwLastActionBroadcastTime ;
	if(bForceActionTermToZero || m_dwLastActionBroadcastTime == 0 /*|| iActionBroadCastTerm > 10000*/)
	{
		dwActionBroadCastTerm = 0;
	}

	//if(dwActionBroadCastTerm == 0)
	//{
	//	PG_ASSERT_LOG(dwActionBroadCastTerm == 0);
	//	return false;
	//}

	PgActor* pkActor = dynamic_cast<PgActor*>(pkPilot->GetWorldObject());
	if(NULL == pkActor || !pkActor->IsUnderMyControl())
	{
		return false;
	}

	NiPoint3 kPos = pkActor->GetPosition(true);
	NiPoint3 kPathNormal = pkActor->GetPathNormal();
	POINT3BY ptPathNormal;
			
	BM::Stream kPacket(PT_C_M_REQ_ACTION2);

	SActionInfo	kActionInfo;
	kActionInfo.guidPilot = pkPilot->GetGuid();
	kActionInfo.bIsDown = pkAction->GetEnable();
	//방향은 0~15 값을 사용하므로 4바이트로 해결된다.
	//패킷을 늘리지 않고 2개의 방향을 하나의 BYTE에 대해서 처리

	//방향 디렉션에 실제 이동 Direction + 바라보는 Direction을 같이 날린다.
	kActionInfo.byDirection = (pkAction->GetActionOptionEnable(PgAction::AO_CAN_CHANGE_ACTOR_POS) ? pkActor->GetDirection() : pkActor->GetLastDirection());
	//kActionInfo.byDirection = pkActor->GetDirection();
	
	// 상위 4바이트 실제 이동 Direction
	kActionInfo.byDirection <<= 4;


	// 하위 4바이트 바라보는 Direction
	if(pkActor->IsLockBidirection())
	{
		BYTE btDirection = pkActor->GetDirFromMovingVector( pkActor->GetLookingDir() );
		kActionInfo.byDirection |= btDirection;
	}
	else
	{
		BYTE btDirection = pkActor->GetDirFromMovingVector8Way( pkActor->GetLookingDir() );
		kActionInfo.byDirection |= btDirection;
	}
	
	NiPoint3& rkPoint3 = pkActor->GetDirectionVector(kActionInfo.byDirection);
	POINT3_2_POINT3BY(POINT3(pkActor->GetLookingDir().x, pkActor->GetLookingDir().y, pkActor->GetLookingDir().z), kActionInfo.ptDirection);
	if( POINT3BY::NullData()==kActionInfo.ptDirection )
	{
		POINT3_2_POINT3BY(POINT3(rkPoint3.x, rkPoint3.y, rkPoint3.z), kActionInfo.ptDirection);
	}

	POINT3_2_POINT3BY(POINT3(kPathNormal.x, kPathNormal.y, 0), ptPathNormal);
	kActionInfo.ptPathNormal.x = ptPathNormal.x;
	kActionInfo.ptPathNormal.y = ptPathNormal.y;
	kActionInfo.ptPos.x = kPos.x;
	kActionInfo.ptPos.y = kPos.y;
	kActionInfo.ptPos.z = kPos.z - PG_CHARACTER_Z_ADJUST;
	kActionInfo.dwActionTerm = dwActionBroadCastTerm;
	kActionInfo.iActionID = iActionNo;
	kActionInfo.iActionInstanceID = iActionInstanceID;
	kActionInfo.byActionParam = byActionParam;
	kActionInfo.dwTimeStamp = lwGetServerElapsedTime32();

	pkActor->AddActionEntityToFollowers(PgActionEntity(pkAction,DIR_NONE));

	kActionInfo.SendToServer(kPacket);

	_PgOutputDebugString("[PgPilotMan.BroadCast] Broadcasted ActionID : %d ActionInstance : %d Direction : %d %d \tActionTerm : %u (Can Change Actor Pos : %s)\n", kActionInfo.iActionID,kActionInfo.iActionInstanceID, (kActionInfo.byDirection >> 4), (kActionInfo.byDirection & 0x0F), dwActionBroadCastTerm, (pkAction->GetActionOptionEnable(PgAction::AO_CAN_CHANGE_ACTOR_POS) ? "TRUE" : "FALSE"));


#ifdef PG_SYNC_ENTIRE_TIME
	// 클라(PC)의 경우 서버에서의 Time을 simulation해서 절대 시간을 맞추고
	// 서버에서 액션이 발생할 경우, 그냥 서버의 시간을 보내주면 된다.
	kPacket.Push(PgActor::GetSynchronizedTime());
#endif

	//Client의 
	if(pkPilot->GetUnit())
	{
//		kPacket.Push(pkPilot->GetUnit()->RandomSeedCallCounter());
		kPacket.Push(pkAction->StartRandomSeedCallCounter());
		//BM::vstring vStr("pkAction->StartRandomSeedCallCounter():");
		//vStr+=pkAction->StartRandomSeedCallCounter();
		//vStr+= "Unit CallCounter:";
		//vStr+=pkPilot->GetUnit()->RandomSeedCallCounter();
		//vStr+="\n\n";
		//OutputDebugStringA(static_cast<std::string>(vStr).c_str());

		//CHECK_INFO_LOG(false, BM::LOG_LV5, _T("[%s] Broadcast ActionID %d (Counter %d)"), pkPilot->GetUnit()->Name().c_str(), iActionNo, pkPilot->GetUnit()->RandomSeedCallCounter());
	}
	else
	{
		kPacket.Push(0);
	}	

	//_PgOutputDebugString("PT_C_M_REQ_ACTION ActionNo : %u Pos(%f,%f,%f) TargetNum : %d\n",iActionNo,kActionInfo.ptPos.x,kActionInfo.ptPos.y,kActionInfo.ptPos.z,);
	PgActionTargetList *pkTargetList = pkAction->GetTargetList();
	BYTE byTargetNum = (pkTargetList != 0 ? pkTargetList->size() : 0);

	if(pkAction->GetActionOptionEnable(PgAction::AO_NO_BROADCAST_TARGETLIST))
	{
		byTargetNum = 0;
	}

	kPacket.Push(byTargetNum); 
	if(byTargetNum>0)
	{
		for(ActionTargetList::iterator itor = pkTargetList->begin(); itor != pkTargetList->end(); ++itor)
		{
			kPacket.Push((*itor).GetTargetPilotGUID());
			kPacket.Push((*itor).GetSphereIndex());
		}
	}

	if(pkAction->GetActionOptionEnable(PgAction::AO_OVERRIDE_PACKET))
	{
		kPacket.Push((bool)true);
		pkAction->OverridePacket(pkActor, &kPacket);
	}
	else
	{
		kPacket.Push((bool)false);
	}

	NILOG(PGLOG_LOG, "PT_C_M_REQ_ACTION(%d) ActionNo : %u byActionParam : %d byTargetNum : %d InstanceID : %d ActionPos:%f,%f,%f ActionTerm  : %d \n", g_pkApp->GetFrameCount(), iActionNo, byActionParam, byTargetNum, iActionInstanceID, kPos.x, kPos.y, kPos.z, dwActionBroadCastTerm);

	NETWORK_SEND(kPacket)
	g_pkRemoteManager->SetLastSendPositionTime();

	if(pkAction->GetEnable())
	{
		m_dwLastActionBroadcastTime = BM::GetTime32();
	}

/*	//WriteToConsole("[PgPilotMan.BroadcastAction] Action : %s, ActionInstance No : %d, Action Term : %d \n", \
		pkAction->GetID().c_str(),\
		iActionInstanceID,\
		dwActionBroadCastTerm);
*/
	return true;
}

PgIXmlObject::XmlObjectID PgPilotMan::GetObjectIDFromString(char const* pcType)
{
	PgIXmlObject::XmlObjectID kXOI = PgIXmlObject::ID_NONE;
	if(pcType)
	{
		if (_strnicmp("pc", pcType, strlen("pc")) == 0)
		{
			kXOI = PgIXmlObject::ID_PC;
		}
		else if (_strnicmp("monster", pcType, strlen("monster")) == 0)
		{
			kXOI = PgIXmlObject::ID_MONSTER;
		}
		else if (_strnicmp("npc", pcType, strlen("npc")) == 0)
		{
			kXOI = PgIXmlObject::ID_NPC;
		}
		else if (_strnicmp("pet", pcType, strlen("pet")) == 0)
		{
			kXOI = PgIXmlObject::ID_PET;
		}
		else if (_strnicmp("boss", pcType, strlen("boss")) == 0)
		{
			kXOI = PgIXmlObject::ID_BOSS;
		}
		else
		{
			NILOG(PGLOG_WARNING, "[PgIXmlObject] Unknown %s Pilot id\n", pcType);
		}
	}

	return kXOI;
}

void PgPilotMan::SetLockMyInput(bool bLock)
{
	m_bLockMyInput = bLock;

	ClearDirectionSlot(false);
}


void PgPilotMan::LockPlayerInput(SReqPlayerLock const& rkReqLock) // Lock/Unlock 시에 type 번호가 한쌍이어야 한다.
{
	if( rkReqLock.bActionLock )
	{
		// action 으로 거는 락
		PgActor* pkActor = GetPlayerActor();
		if( pkActor )
		{
			if( PgPilotManUtil::LockPlayerInput(m_kActionLockedType, rkReqLock.eLockType) )
			{
				pkActor->ReserveTransitAction("a_lock_move");
				pkActor->ClearActionState();
				pkActor->SetDirection(DIR_NONE);

			}
		}
	}
	else
	{
		// 입력만 막는 락
		if( PgPilotManUtil::LockPlayerInput(m_kLockedType, rkReqLock.eLockType) )
		{
			lwReleaseAllKey();
			SetLockMyInput(true);
		}
	}
}


void PgPilotMan::UnlockPlayerInput(SReqPlayerLock const& rkReqLock)
{
	if( rkReqLock.bActionLock )
	{
		// action 으로 거는 락
		if( PgPilotManUtil::UnlockPlayerInput(m_kActionLockedType, rkReqLock.eLockType) 
			&& !lwCheckReservedClassPromotionAction()
			)
		{
			PgActor* pkActor = GetPlayerActor();
			if( pkActor )
			{
				pkActor->ReserveTransitAction("a_unlock_move");
			}
		}
	}
	else
	{
		// 입력만 막는 락
		if( PgPilotManUtil::UnlockPlayerInput(m_kLockedType, rkReqLock.eLockType) )
		{
			SetLockMyInput(false);
		}
	}
}

//점핑 캐릭터 이벤트
int PgPilotMan::SJumpingCharEvent::BaseClassNo()const
{
	PgActor const* pkActor = g_kSelectStage.GetNewActor();
	if( !pkActor )
	{
		return 0;
	}

	if( pkActor->GetUnit() )
	{
		return pkActor->GetUnit()->GetAbil(AT_CLASS);
	}
	return 0;
}

void PgPilotMan::SetJumpingEvent(int const iEventNo, int const iRemainRewardCount)
{
	m_kJumpingCharEvent.iNo = iEventNo;
	m_kJumpingCharEvent.iRemain = iRemainRewardCount;
}

void PgPilotMan::UseJumpingEvent(bool const bUse)
{
	m_kJumpingCharEvent.bUse = (bUse && m_kJumpingCharEvent.iNo);
}

int PgPilotMan::GetJumpingCreateCharCount()const
{
	if( m_kJumpingCharEvent.iNo>0 )
	{
		int const iBaseClassNo = m_kJumpingCharEvent.BaseClassNo();
		
		VEC_INT kNextClass;
		g_kJumpingCharEventMgr.GetClass(m_kJumpingCharEvent.iNo, iBaseClassNo, kNextClass);
		return kNextClass.size();
	}
	return 0;
}

void PgPilotMan::SetJumpingSelectIdx(int const iIdx)
{
	int const iBaseClassNo = m_kJumpingCharEvent.BaseClassNo();
	m_kJumpingCharEvent.iClass = g_kJumpingCharEventMgr.GetClass(m_kJumpingCharEvent.iNo, iBaseClassNo, iIdx);
}

bool PgPilotMan::IsCreateJumpingCharClass(int const iClassNo)const
{
	VEC_INT kContClass;
	g_kJumpingCharEventMgr.GetClass(m_kJumpingCharEvent.iNo, iClassNo, kContClass);
	return false==kContClass.empty();
}

int PgPilotMan::GetCreateJumpingCharClass(int const iIdx)const
{
	int const iBaseClassNo = m_kJumpingCharEvent.BaseClassNo();
	return g_kJumpingCharEventMgr.GetClass(m_kJumpingCharEvent.iNo, iBaseClassNo, iIdx);
}

int PgPilotMan::GetCreateJumpingCharLevel(int const iIdx)const
{
	int const iBaseClassNo = m_kJumpingCharEvent.BaseClassNo();
	return g_kJumpingCharEventMgr.GetLevel(m_kJumpingCharEvent.iNo, iBaseClassNo, iIdx);
}

void PgPilotMan::SetHideBalloon(bool const bHide)
{
	m_bHideBalloon = bHide;

	BM::CAutoMutex kLock(m_kRemoveLock, true);
	Container::iterator itr = m_kContPilot.begin();
	while(itr != m_kContPilot.end())
	{
		if(itr->second)
		{
			if( PgActor * pkActor = dynamic_cast<PgActor*>(itr->second->GetWorldObject()) )
			{
				pkActor->SetHideBalloon(bHide);
			}
		}
		++itr;
	}
}

bool PgPilotMan::IsHideBalloon()const
{
	return m_bHideBalloon;
}

namespace PgPilotManUtil
{
	bool LockPlayerInput(ContLockType &rkLockCont, EPlayerLockType const eLockType)
	{
		if( EPLT_None >= eLockType )
		{
#ifndef EXTERNAL_RELEASE
			PgError1("[%d] is unknown lock type", eLockType);
#endif
			return false;
		}

		ContLockType::iterator find_iter = std::find(rkLockCont.begin(), rkLockCont.end(), eLockType);
		if( rkLockCont.end() != find_iter )
		{
			return false;
		}

		bool const bRet = rkLockCont.empty(); // 아무것도 없는 상태에서 진입시에만 락을 잠금

		std::back_inserter(rkLockCont) = eLockType;

		return bRet; // true --> lock, false --> don't lock
	}

	bool UnlockPlayerInput(ContLockType &rkLockCont, EPlayerLockType const eLockType)
	{
		if( EPLT_None >= eLockType )
		{
#ifndef EXTERNAL_RELEASE
			PgError1("[%d] is unknown lock type", eLockType);
#endif
			return false;
		}

		ContLockType::iterator find_iter = std::find(rkLockCont.begin(), rkLockCont.end(), eLockType);
		if( rkLockCont.end() != find_iter )
		{
			rkLockCont.erase(find_iter);
		}
		else
		{
			return false; // 락을 건적이 없는데 풀을 수는 없는것이다
		}
		return rkLockCont.empty(); // 잠금 상태가 아무것도 없어야 풀어줌
	}

	void RemoveReservePilotUnit(BM::GUID const& rkCharacterGuid, char const* szFunc, size_t const iLine)
	{
		NILOG(PGLOG_LOG, "[%s][%u]-[%s][%u] Try To Find From Reserved Pilot\n", szFunc, iLine, __FUNCTION__, __LINE__);
		bool bReservedPilot = false;
		if( g_kPilotMan.IsReservedPilot(rkCharacterGuid) )
		{
			NILOG(PGLOG_LOG, "[%s][]-[][%u] Pilot Found From Reserved Pilot\n", szFunc, __LINE__);
			bReservedPilot = true;
			g_kPilotMan.RemoveReservedPilot(rkCharacterGuid);
		}

		if( g_pkWorld )
		{
			NILOG(PGLOG_LOG, "[%s][]-[][%u] Try To Remove Pilot From AddUnitQueue\n", szFunc, __LINE__);
			if( g_pkWorld->RemoveUnitFromAddUnitQueue(rkCharacterGuid) )
			{
				PG_ASSERT_LOG(bReservedPilot);	// QUESTION: AddUnitQueue에는 있는데 ReservedPilot이 아닐 수 있을까?
			}
		}
		else
		{
			NILOG(PGLOG_LOG, "[%s][]-[][%u] World Is Null\n", szFunc, __LINE__);
		}

		g_kStatusEffectMan.ClearReserveEffect(rkCharacterGuid);
	}

	void BufferReservedPilotPacket_NotUnitTypeRemove( BM::GUID const& rkCharacterGuid, EUnitType const kUnitType, BM::Stream* pkPacket, char const* szFunc, size_t const iLine )
	{
		NILOG(PGLOG_LOG, "[%s][%u]-[%s][%u] Try To Find From Reserved Pilot UnitType = %d\n", szFunc, iLine, __FUNCTION__, __LINE__, kUnitType );
		
		bool bReservedPilot = false;
		EUnitType kCheckUnitType = UT_NONETYPE;
		if( g_kPilotMan.IsReservedPilot( rkCharacterGuid, &kCheckUnitType ) )
		{
			NILOG(PGLOG_LOG, "[%s][]-[][%u] Pilot Found From Reserved Pilot ParamType=%d, ReserveType=%d\n", szFunc, __LINE__, kUnitType, kCheckUnitType );
			bReservedPilot = true;
			if ( kUnitType == kCheckUnitType )
			{
				g_kPilotMan.BufferReservedPilotPacket( rkCharacterGuid, pkPacket, 0 );
				return;
			}
		}

		if( g_pkWorld )
		{
			NILOG(PGLOG_LOG, "[%s][]-[][%u] Try To Remove Pilot From AddUnitQueue\n", szFunc, __LINE__);
			if( g_pkWorld->RemoveUnitFromAddUnitQueue(rkCharacterGuid) )
			{
				PG_ASSERT_LOG(bReservedPilot);	// QUESTION: AddUnitQueue에는 있는데 ReservedPilot이 아닐 수 있을까?
			}
		}
		else
		{
			NILOG(PGLOG_LOG, "[%s][]-[][%u] World Is Null\n", szFunc, __LINE__);
		}

		g_kStatusEffectMan.ClearReserveEffect(rkCharacterGuid);
	}
	
	bool RemoveConnectionWithSubPlayer( BM::GUID const& rkPlayerGuid, BM::GUID& rkOutSubPlayer )
	{
		rkOutSubPlayer = BM::GUID::NullData();
		PgPilot* pkPiot =  g_kPilotMan.FindPilot( rkPlayerGuid ); 
		if( !pkPiot )
		{
			return false;
		}
		CUnit* pkUnit = pkPiot->GetUnit();
		if( !pkUnit )
		{
			return false;
		}
		switch( pkUnit->UnitType() )
		{
		case UT_PLAYER:
			{
				PgPlayer* pkPlayer = static_cast<PgPlayer*>( pkUnit );
				if(!pkPlayer)
				{
					return false;
				}
				BM::GUID const& rkSubPlayerGuid =  pkPlayer->SubPlayerID();
				if( rkSubPlayerGuid.IsNull() )
				{
					return false;
				}
				PgPilot* pkSubPilot = g_kPilotMan.FindPilot(rkSubPlayerGuid);
				if(!pkSubPilot)
				{
					return false;
				}
				CUnit* pkSubUnit = pkSubPilot->GetUnit();
				if(!pkSubUnit)
				{
					return false;
				}
				if(!pkSubUnit->IsUnitType(UT_SUB_PLAYER))
				{
					return false;
				}
				PgSubPlayer* pkSubPlayer = static_cast<PgSubPlayer*>( pkSubUnit );
				if(!pkSubPlayer)
				{
					return false;
				}
				rkOutSubPlayer = rkSubPlayerGuid;
				pkSubPlayer->SetCallerUnit(NULL);
				pkSubPlayer->Caller( BM::GUID::NullData() );
				pkPlayer->SubPlayerID( BM::GUID::NullData() ); // 으으으흠...
			}break;
		default:
			{
				return false;
			}break;
		}
		return true;
	}
};