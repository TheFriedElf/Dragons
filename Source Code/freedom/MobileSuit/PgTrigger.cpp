#include "StdAfx.h"
#include "Variant/PgStringUtil.h"
#include "PgNetwork.h"
#include "lwTrigger.h"
#include "PgActor.h"
#include "lwActor.h"
#include "PgPilotMan.h"
#include "PgParticleMan.h"
#include "PgMobileSuit.h"
#include "PgWorld.h"
#include "PgTrigger.h"
#include "PgHelpSystem.H"
#include "PgSoundMan.H"
#include "PgClientParty.H"
#include "PgPilot.h"
#include "PgActorUtil.h"
#include "Variant/PortalAccessInfo.h"

namespace TriggerUtil
{
	bool IsPlayerActor(PgActor* pkActor)
	{
		PgPilot *pkPilot = pkActor->GetPilot();
		if( pkPilot )
		{
			CUnit* pkUnit = pkPilot->GetUnit();
			if( pkUnit
			&&	pkUnit->IsUnitType(UT_PLAYER) )
			{
				return true;
			}
		}
		return false;
	}
}

PgTrigger::PgTrigger()
:m_iParam(0),
m_iParam2(0),
m_iTriggerTitleTextID(0),
m_ptTitleTextAdjust(NiPoint3::ZERO),
m_kTriggerTitleTextScale(1),
m_kTriggerTitleTextColor(NiColorA::WHITE),
m_eTriggerType(TRIGGER_TYPE_NONE),
m_fPhysXActiveTotalTime(0),
m_fParticleEmitTotalTime(0),
m_bIsEmitting(false),
m_fRotation(-9999.0),
m_bMinimapHide(false),
m_bUseWorldEventScript(false),
m_kContDisableInActor(),
m_bLoveFenceLife(false),
m_bDoOnLeaveScript(true),
m_eAddedType(ADD_NONE),
m_LoadingImageID(0),
m_RagUI(0),
m_iTriggerSkillID(0)
{
	Enable(true);
	SetGuid(BM::GUID::Create());
}

PgTrigger::~PgTrigger()
{
}

bool PgTrigger::Initialize()
{
	if(!g_pkWorld)
	{
		return false;
	}
	NiPhysXScene* pkPhysXScene = g_pkWorld->GetPhysXScene();
	if (!pkPhysXScene)
	{
		return false;
	}

	unsigned int uiSrcTotal = pkPhysXScene->GetSourcesCount();
	for (unsigned int ui=0 ; ui<uiSrcTotal ; ++ui)
	{
		NiPhysXRigidBodySrc* pkRigidBodySrc = NiDynamicCast(NiPhysXRigidBodySrc,pkPhysXScene->GetSourceAt(ui));
		if(!pkRigidBodySrc)
		{
			continue;
		}

		NiAVObject* pkSrcObject = pkRigidBodySrc->GetSource();
		if (!pkSrcObject)
		{
			continue;
		}
		if (pkSrcObject->GetName().Exists() == false)
		{
			continue;
		}

		std::string const strName((char const*)pkSrcObject->GetName());
		if(GetID() != strName)
		{
			continue;
		}

		SetSourceObjects(pkRigidBodySrc,pkSrcObject);
		break;
	}

	return true;
}

std::string &PgTrigger::GetConditionAction()
{
	return m_kConditionAction;
}

bool PgTrigger::OnEnter(PgActor *pkActor)
{
	if(!Enable())
	{
		if( pkActor
		&&	TriggerUtil::IsPlayerActor(pkActor) ) // 비 활성화 상 태 때, Player 기록
		{
			m_kContDisableInActor.insert( pkActor->GetGuid() );
		}
		return false;
	}
	if(!IsPhysXActiveTime())	{return false;}

	std::wstring wstrCompare = _T("SOUND_PLAY_COUNT");
	wstrCompare += _T('\0');
	bool bLimitedSoundPlay = (m_wstrParam == wstrCompare);
	if(m_kSoundIDOnEnter.empty() == false)
	{
		if(bLimitedSoundPlay == false || (bLimitedSoundPlay == true && m_iParam-- > 0))
		{
			g_kSoundMan.PlayAudioSourceByID(NiAudioSource::TYPE_3D, m_kSoundIDOnEnter.c_str(), 0, 0, 0,pkActor);
		}
	}

	if(pkActor && pkActor->IsMyActor())
	{
		g_kHelpSystem.ActivateByCondition(HELP_CONDITION_TRIGGER,GetID());
	}

	if(m_eConditionType == CT_TOUCH)
	{
		if (m_kScript.empty())
		{
			return false;
		}

		if(false == pkActor->GetIsInTrigger())
		{
			if( pkActor )
			{
				pkActor->SetCurrentTrigger(this);
			}

			if(m_bUseWorldEventScript)
			{
				lua_tinker::call<bool, lwTrigger, lwActor>("OnWorldEvent_PhysXTriggerEnter", lwTrigger(this), lwActor(pkActor));
			}
			return lua_tinker::call<bool, lwTrigger, lwActor>((m_kScript + "_OnEnter").c_str(), lwTrigger(this), lwActor(pkActor));
		}
		else
		{
			return false;
		}
	}
	else if(m_eConditionType == CT_ACTION)
	{
		if(pkActor)
		{
			pkActor->SetCurrentTrigger(this);
			if(TRIGGER_TYPE_JOB_SKILL == GetTriggerType())
			{
				PgActorUtil::UpdateColorShadow(pkActor, true);
			}
		}
		return true;
	}

	return false;
}

bool PgTrigger::OnUpdate(PgActor *pkActor)
{
	if(!Enable())	{return false;}
	if(!IsPhysXActiveTime())	{return false;}

	if(m_eConditionType == CT_TOUCH && m_kScript.length() > 0)
	{
		return lua_tinker::call<bool, lwTrigger, lwActor>((m_kScript + "_OnUpdate").c_str(), lwTrigger(this), lwActor(pkActor));
	}

	return false;
}

bool PgTrigger::OnLeave(PgActor *pkActor, PgTrigger *pNextTrigger)
{
	if(pNextTrigger && false==pNextTrigger->Enable())
	{
		pNextTrigger = NULL;
	}

	if(!Enable())
	{
		if( pkActor
		&&	TriggerUtil::IsPlayerActor(pkActor) )
		{
			m_kContDisableInActor.erase( pkActor->GetGuid() );
		}
		return false;
	}
	if(!IsPhysXActiveTime())	{return false;}

	if ( pkActor )
	{
		PgTrigger* pkActorCurTrigger = pkActor->GetCurrentTrigger();
		if(pkActorCurTrigger
			&& GetID() == pkActorCurTrigger->GetID()
			)
		{// 예외 처리, A트리거 Enter - B트리거 Enter - A트리거 Leave 하는 경우가 생겨
			// Actor에 B트리거로 세팅된 트리거 포인터가 NULL이 되어버리는 경우가 있으므로
			// 이름 체크를 통한 트리거 포인터 세팅을 함
			pkActor->SetCurrentTrigger(NULL);
		}

		if(m_eConditionType == CT_TOUCH && m_kScript.length() > 0)
		{
			return lua_tinker::call<bool, lwTrigger, lwTrigger, lwActor>((m_kScript + "_OnLeave").c_str(), lwTrigger(this), lwTrigger(pNextTrigger), lwActor(pkActor));
		}

		if(TRIGGER_TYPE_JOB_SKILL == GetTriggerType())
		{
			PgActorUtil::UpdateColorShadow(pkActor, true);

			if(pkActor->IsMyActor())
			{
				lua_tinker::call<void, lwActor>("JobSkill_Net_ReqCancelJobSkill", lwActor(pkActor));
			}
		}
	}
	return false;
}

bool PgTrigger::OnAction(PgActor *pkActor)
{
	if(!pkActor)
	{
		return false;
	}

	if(!IsActiveTime())
	{
		lwAddWarnDataTT(803, true);
		return false;
	}
	if(!Enable())	{return false;}

	if(TRIGGER_TYPE_JOB_SKILL == GetTriggerType() && pkActor->IsRidingPet())
	{ //탑승 중 채집생산을 시도하면 실패처리
		lwAddWarnDataTT(451004, true);
		return false;
	}

	if( true == pkActor->CheckOutLobby( this ) ) // 원정대 로비에서 나갈 것인지 체크함
	{
		return false;
	}

	if(m_eConditionType != CT_ACTION)
	{
		return false;
	}

	if(m_kScript.length() == 0)
	{
		return	false;
	}

	if(m_bUseWorldEventScript)
	{
		lua_tinker::call<bool, lwTrigger, lwActor>("OnWorldEvent_PhysXTriggerAction", lwTrigger(this), lwActor(pkActor));
	}

	if( TRIGGER_TYPE_JOB_SKILL == m_eTriggerType )
	{
		return lua_tinker::call<bool, lwTrigger, lwActor>(m_kScript.c_str(), lwTrigger(this), lwActor(pkActor), m_iParam2 );
	}
	return lua_tinker::call<bool, lwTrigger, lwActor>(m_kScript.c_str(), lwTrigger(this), lwActor(pkActor));
}

int PgTrigger::GetIndex()
{
	return m_iIndex;
}

void PgTrigger::SetIndex(int iIndex)
{
	m_iIndex = iIndex;
}

void PgTrigger::Enable(bool const bNewValue)
{
	if( m_kEnable != bNewValue )
	{
		if( !bNewValue )
		{
			PgActor* pkActor = g_kPilotMan.GetPlayerActor();
			if( pkActor && pkActor->GetCurrentTrigger()==this )
			{
				OnLeave(pkActor, NULL); // 비활성화 상태 일 때 내 플레이어는, 트리거에서 나간거로 처림
				m_kContDisableInActor.insert( pkActor->GetGuid() );
			}
		}

		m_kEnable = bNewValue;

		if( bNewValue )
		{
			CONT_DISABLE_IN_ACTOR::const_iterator iter = m_kContDisableInActor.begin();
			while( m_kContDisableInActor.end() != iter )
			{
				if( g_kPilotMan.IsMyPlayer((*iter)) )
				{
					PgActor* pkActor = g_kPilotMan.GetPlayerActor();
					if( pkActor )
					{
						OnEnter(pkActor); // 비활성화 상태 일 때 내 플레이어는, Enable 상태 일 때 Enter 한번 호출(별로 좋지는 않음 --;)
						break;
					}
				}
				++iter;
			}
			m_kContDisableInActor.clear();
		}
		if(g_pkWorld)
		{
			NiPoint3 const kTriggerPos = (m_spTriggerObject)? m_spTriggerObject->GetWorldTranslate(): NiPoint3(0.f,0.f,0.f);
			if( m_kEnable )
			{
				g_pkWorld->AddWorldText(kTriggerPos+GetTitleTextAdjust(), TTW(GetTriggerTitleTextID()), GetTriggerTitleTextScale(), GetTriggerTitleTextColor(), GetTriggerTitleTextFontName());
			}
			else
			{
				g_pkWorld->DelWorldText(kTriggerPos+GetTitleTextAdjust(), TTW(GetTriggerTitleTextID()), GetTriggerTitleTextScale(), GetTriggerTitleTextColor(), GetTriggerTitleTextFontName());
			}
		}
	}
}

char const*	PgTrigger::GetParamFromParamMap(char const *strKeyString)
{
	if(!strKeyString)
	{
		return NULL;
	}

	ParamMap::const_iterator itr = m_ParamMap.find( std::string(strKeyString) );
	if ( itr != m_ParamMap.end() )
	{
		return itr->second.c_str();
	}
	return NULL;
}

bool PgTrigger::SetParamFromParamMap(char const* szKeyString, char const* szValueString)
{
	if( !szKeyString
	||	!szValueString )
	{
		return false;
	}

	std::string const kKeyString = szKeyString;
	std::string const kValueString = szValueString;
	ParamMap::iterator iter = m_ParamMap.find(kKeyString);
	if( m_ParamMap.end() != iter )
	{
		(*iter).second = kValueString;
	}
	else
	{
		std::pair<ParamMap::iterator, bool> kRet = m_ParamMap.insert(std::make_pair(szKeyString, kValueString));
		if( !kRet.second )
		{
			return false;
		}
	}
	return true;
}

//! PgIWorldObject 재정의
bool PgTrigger::Update(float fAccumTime, float fFrameTime)
{
	if (m_kParticleEmitTimeCont.size() && m_fParticleEmitTotalTime != 0.0f)
	{
		//float const fAccumTime = g_pkApp->GetAccumTime();
		// Emit제어.
		unsigned __int64 iAccumTime = (unsigned __int64)(fAccumTime * 1000);
		unsigned __int64 iEmitTotal = (unsigned __int64)(m_fParticleEmitTotalTime * 1000);
		unsigned __int64 iTime = iAccumTime % iEmitTotal;
		float fTime = iTime * 0.001f;		// millisec -> sec

		bool bIsEmit = false;
		CONT_TIME::const_iterator itr = m_kParticleEmitTimeCont.begin();
		while(itr != m_kParticleEmitTimeCont.end())
		{
			float fStartTime = (*itr).first;
			float fEndTime = (*itr).second;
			if (fTime > fStartTime && fTime < fEndTime)
			{
				bIsEmit = true;
				break;
			}
			++itr;
		}

		if (m_bIsEmitting != bIsEmit)
		{
			m_bIsEmitting = bIsEmit;
			PgParticleMan::ChangeParticleGeneration(m_spTriggerObject, bIsEmit);
		}
	}

	return true;
}

bool PgTrigger::ParseXml(const TiXmlNode *pkNode, void *pArg, bool bUTF8)
{
	int const iType = pkNode->Type();
	
	switch(iType)
	{
	case TiXmlNode::ELEMENT:
		{
			TiXmlElement *pkElement = (TiXmlElement *)pkNode;
			assert(pkElement);
			
			char const *pcTagName = pkElement->Value();

			if (strcmp(pcTagName, "TRIGGER") == 0 ||
				strcmp(pcTagName, "OBJECT_PROPERTY") == 0)
			{
				TiXmlAttribute* pkAttr = pkElement->FirstAttribute();
				while(pkAttr)
				{
					char const *pcAttrName = pkAttr->Name();
					char const *pcAttrValue = pkAttr->Value();
				
					if(strcmp(pcAttrName, "ID") == 0)
					{
						if (!GetID().size())
						{
							SetID(pcAttrValue);
							//	if( NULL != pcAttrValue )
							//	{
							//		std::string kId(pcAttrValue);
							//		std::transform( kId.begin(), kId.end(), kId.begin(), toupper );
							//		SetID(kId.c_str());
							//	}							
						}
					}
					else if(strcmp(pcAttrName, "ENABLE") == 0)
					{
						m_kEnable = PgStringUtil::SafeAtob(pcAttrValue, true);
					}
					else if(strcmp(pcAttrName, "TITLE_TEXT_ID") == 0)
					{
						m_iTriggerTitleTextID = atoi(pcAttrValue);
					}
					else if(strcmp(pcAttrName, "TITLE_TEXT_ADJUST") == 0)
					{
						sscanf_s(pcAttrValue, "%f, %f, %f", 
							&m_ptTitleTextAdjust.x, 
							&m_ptTitleTextAdjust.y, 
							&m_ptTitleTextAdjust.z);
					}
					else if(strcmp(pcAttrName, "TITLE_TEXT_COLOR") == 0)
					{
						sscanf_s(pcAttrValue, "%f, %f, %f, %f", 
							&m_kTriggerTitleTextColor.r, 
							&m_kTriggerTitleTextColor.g, 
							&m_kTriggerTitleTextColor.b,
							&m_kTriggerTitleTextColor.a);
					}
					else if(strcmp(pcAttrName, "TITLE_TEXT_SCALE") == 0)
					{
						m_kTriggerTitleTextScale = (float)atof(pcAttrValue);
					}
					else if(strcmp(pcAttrName, "TITLE_TEXT_FONTNAME") == 0)
					{
						m_kTitleTextFontName = std::wstring(UNI(pcAttrValue));
					}
					else if(stricmp(pcAttrName, "SOUND_ID") == 0)
					{
						m_kSoundIDOnEnter = pcAttrValue;
					}
					else if(stricmp(pcAttrName, "HIDE_MINIMAP") == 0)
					{
						m_bMinimapHide = static_cast<bool>(atoi(pcAttrValue));
					}
					else if(stricmp(pcAttrName, "CHECK_EFFECT") == 0)
					{
					}
					else if(stricmp(pcAttrName, "ERROR_EFFECT_MSG") == 0)
					{
					}
					else if(strcmp(pcAttrName, "TYPE") == 0)
					{
						if (strcmp(pcAttrValue, "PORTAL") == 0)
						{
							m_eTriggerType = TRIGGER_TYPE_PORTAL;
						}
						else if (strcmp(pcAttrValue, "MISSION") == 0)
						{
							m_eTriggerType = TRIGGER_TYPE_MISSION;
						}
						else if (strcmp(pcAttrValue, "MISSION_NPC") == 0)
						{
							m_eTriggerType = TRIGGER_TYPE_MISSION_NPC;
						}
						else if (strcmp(pcAttrValue, "MISSION_EVENT_NPC") == 0)
						{
							m_eTriggerType = TRIGGER_TYPE_MISSION_EVENT_NPC;
						}
						else if (strcmp(pcAttrValue, "MISSION_EASY") == 0)
						{
							m_eTriggerType = TRIGGER_TYPE_MISSION_EASY;
						}
						else if (strcmp(pcAttrValue, "HIDDEN_PORTAL") == 0)
						{
							m_eTriggerType = TRIGGER_TYPE_HIDDEN_PORTAL;
						}
						else if (strcmp(pcAttrValue, "JUMP") == 0)
						{
							m_eTriggerType = TRIGGER_TYPE_JUMP;
						}
						else if (strcmp(pcAttrValue, "TELEJUMP") == 0)
						{
							m_eTriggerType = TRIGGER_TYPE_TELEJUMP;
						}
						else if (strcmp(pcAttrValue, "ROPE_RIDING") == 0)
						{
							m_eTriggerType = TRIGGER_TYPE_ROPE_RIDING;
						}
						else if (strcmp(pcAttrValue, "LOCATION") == 0)
						{
							m_eTriggerType = TRIGGER_TYPE_LOCATION;
						}
						else if (strcmp(pcAttrValue, "EVENT") == 0)
						{
							m_eTriggerType = TRIGGER_TYPE_EVENT;
						}
						else if (strcmp(pcAttrValue, "KEYEVENT") ==0)
						{
							m_eTriggerType = TRIGGER_TYPE_KEYEVENT;
						}
						else if ( !::strcmp(pcAttrValue, "BATTLEAREA") )
						{
							m_eTriggerType = TRIGGER_TYPE_BATTLEAREA;
						}
						else if ( !::strcmp(pcAttrValue, "EMPORIA") )
						{
							m_eTriggerType = TRIGGER_TYPE_PORTAL_EMPORIA;
						}
						else if ( !::strcmp(pcAttrValue, "TELEPORT") )
						{//이놈은 아이콘 생기면 안된다였는데 바꼈다.
							m_eTriggerType = TRIGGER_TYPE_TELEPORT;
						}
						else if ( !::strcmp(pcAttrValue, "HOME") )
						{
							m_eTriggerType = TRIGGER_TYPE_ENTER_HOME;
						}
						else if ( !::strcmp(pcAttrValue, "TRANSTOWER") )
						{
							m_eTriggerType = TRIGGER_TYPE_TRANSTOWER;
						}
						else if( !::strcmp(pcAttrValue, "SUPER_GROUND") )
						{
							m_eTriggerType = TRIGGER_TYPE_SUPERGROUND;
						}
						else if( !::strcmp(pcAttrValue, "PARTY_PORTAL") )
						{
							m_eTriggerType = TRIGGER_TYPE_PARTYPORTAL;
						}
						else if( !::strcmp(pcAttrValue, "PARTY_MEMBER_PORTAL") )
						{
							m_eTriggerType = TRIGGER_TYPE_PARTY_MEMBER_PORTAL;
						}
						else if( !::strcmp(pcAttrValue, "RAG_PARTY_MEMBER_PORTAL") )
						{
							m_eTriggerType = TRIGGER_TYPE_RAG_PARTY_MEMBER_PORTAL;
						}
						else if( !::strcmp(pcAttrValue, "JOB_SKILL") )
						{
							m_eTriggerType = TRIGGER_TYPE_JOB_SKILL;
						}
						else if( !::strcmp(pcAttrValue, "DOUBLE_UP") )
						{
							m_eTriggerType = TRIGGER_TYPE_DOUBLE_UP;
						}
						else if( !::strcmp(pcAttrValue, "ROADSIGN") )
						{
							m_eTriggerType = TRIGGER_TYPE_ROADSIGN;
						}
						else if( !::strcmp(pcAttrValue, "CHANNEL_PORTAL") )
						{
							m_eTriggerType = TRIGGER_TYPE_CHANNELPORTAL;
						}
						else if( !::strcmp(pcAttrValue, "KING_OF_HILL") )
						{
							m_eTriggerType = TRIGGER_TYPE_KING_OF_HILL;
						}
						else if( !::strcmp(pcAttrValue, "LOVE_FENCE") )
						{
							m_eTriggerType = TRIGGER_TYPE_LOVE_FENCE;
							m_bLoveFenceLife = true;
						}
						else if( !::strcmp(pcAttrValue, "SCORE") )
						{
							m_eTriggerType = TRIGGER_TYPE_SCORE;
						}
						else if( !::strcmp(pcAttrValue, "RADAR") )
						{
							m_eTriggerType = TRIGGER_TYPE_RADAR;
						}
						else if (strcmp(pcAttrValue, "MISSION_EVENT_NOT_HAVE_ARCADE") == 0)
						{
							m_eTriggerType = TRIGGER_TYPE_MISSION_EVENT_NOT_HAVE_ARCADE;
						}
						else if(strcmp(pcAttrValue, "PROGRESS") == 0)
						{
							m_eTriggerType = TRIGGER_TYPE_PROGRESS_SENSOR;
							m_kScript = "Progress_Sensor";
							m_eConditionType = CT_TOUCH;
						}
						else if(strcmp(pcAttrValue, "RACE_CHECK_POINT") == 0)
						{
							m_eTriggerType = TRIGGER_TYPE_RACE_CHECK_POINT;
						}
						else if(strcmp(pcAttrValue, "MISSION_UNUSED_GADACOIN") == 0)
						{
							m_eTriggerType = TRIGGER_TYPE_MISSION_UNUSED_GADACOIN;
						}
						else
						{
							PgXmlError1(pkElement, "XmlParse: Incoreect Attr '%s'", pcAttrName);
						}
					}
					else if (0 == ::strcmp(pcAttrName, CAMTRG_TYPE_NAME.c_str()) )
					{
						if( TRIGGER_TYPE_NONE != m_eTriggerType )
						{
							_PgMessageBox("CameraTriggerParsing Error", "%s\nTriggerType already setted\nBut now set CAMERA_TYPE", GetID().c_str() );
						}
						m_eTriggerType = TRIGGER_TYPE_CAMERA;
						if( NULL != pcAttrValue )
						{// 무조건 대문자로 바꿈
							std::string kTempCameraType(pcAttrValue);
							std::transform( kTempCameraType.begin(), kTempCameraType.end(), kTempCameraType.begin(), toupper );
							
							if(CAMTRG_OUT_TYPE  == kTempCameraType
								|| CAMTRG_IN_TYPE == kTempCameraType
								//|| CAMTRG_SINGLE_TYPE == kTempCameraType
							  )
							{
								SetParamFromParamMap(pcAttrName, kTempCameraType.c_str() );
							}
							else
							{
								std::string kErrMsg("Check CAMERA_TYPE value in ");
								kErrMsg += GetID() + std::string("\nNow use default set \'IN\'.");
								_PgMessageBox("CameraTriggerParsing Error", kErrMsg.c_str());
								SetParamFromParamMap(pcAttrName, CAMTRG_IN_TYPE.c_str());
							}
						}
					}
					else if(strcmp(pcAttrName, "IS_WORLD_EVENT_SCRIPT_USE") == 0)
					{
						m_bUseWorldEventScript = static_cast<bool>(atoi(pcAttrValue));
					}
					else if(strcmp(pcAttrName, "VALUE") == 0)
					{
						SetProgressID( static_cast<int>( atoi(pcAttrValue) ) );
					}
					else
					{
						PgXmlError1(pkElement, "XmlParse: Incoreect Attr '%s'", pcAttrName);
					}

					pkAttr = pkAttr->Next();
				}

				// 자식 노드들을 파싱한다.
				// 첫 자식만 여기서 걸어주면, 나머지는 NextSibling에 의해서 자동으로 파싱된다.
				const TiXmlNode * pkChildNode = pkNode->FirstChild();
				if(pkChildNode != 0)
				{
					if(!ParseXml(pkChildNode))
					{
						return false;
					}
				}

				return true;
			}
			else if(strcmp(pcTagName, "PROPERTY") == 0)
			{
				TiXmlAttribute *pkAttr = pkElement->FirstAttribute();
				while(pkAttr)
				{
					char const *pcAttrName = pkAttr->Name();
					char const *pcAttrValue = pkAttr->Value();

					if(strcmp(pcAttrName, "ID") == 0)
					{
						// XML파일을 열고 새로 Parse한다.
						TiXmlDocument* pkXmlDoc = PgXmlLoader::GetXmlDocumentByID(pcAttrValue);
						if (pkXmlDoc)
						{
							// 최상위 노드를 찾는다.
							TiXmlNode *pkRootNode = pkXmlDoc->FirstChild();
							while(pkRootNode && pkRootNode->Type() != TiXmlNode::ELEMENT)
							{
								pkRootNode = pkRootNode->NextSibling();
							}
							// 최상위 노드가 없다면,
							if(!pkRootNode)
							{
								PgError1("ID [%s] is incorrect", pcAttrValue);
							//	VERIFY_INFO_LOG(false, BM::LOG_LV5, _T("ID [%s] is incorrect"), UNI(pcAttrValue) );
								return 0;
							}
							ParseXml(pkRootNode, pArg);
						}
					}
					else
					{
						m_ParamMap.insert(std::make_pair(std::string(pcAttrName),
							std::string(pcAttrValue)));
					}
					pkAttr = pkAttr->Next();
				}
			}
			else if(strcmp(pcTagName, "CONDITION") == 0)
			{
				char const *pcScript = 0;

				TiXmlAttribute *pkAttr = pkElement->FirstAttribute();
				while(pkAttr)
				{
					char const *pcAttrName = pkAttr->Name();
					char const *pcAttrValue = pkAttr->Value();

					if(strcmp(pcAttrName, "TYPE") == 0)
					{
						if(strcmp(pcAttrValue, "touch") == 0)
						{
							m_eConditionType = CT_TOUCH;
						}
						else if(strcmp(pcAttrValue, "action") == 0)
						{
							m_eConditionType = CT_ACTION;
						}
						else if(strcmp(pcAttrValue, "script") == 0)
						{
							m_eConditionType = CT_SCRIPT;
						}
						else
						{
							PgXmlError1(pkElement, "Incorrect Trigger Type [%s]", pcAttrValue);
						}
					}
					else if(strcmp(pcAttrName, "ACTION") == 0)
					{
						m_kConditionAction = pcAttrValue;
					}
					else
					{
						PgXmlError1(pkElement, "XmlParse: Incoreect Attr '%s'", pcAttrName);
					}
					
					pkAttr = pkAttr->Next();
				}
			}
			else if(strcmp(pcTagName, "ACTION") == 0)
			{
				char acString[512] = {0,};

				TiXmlAttribute *pkAttr = pkElement->FirstAttribute();
				char const *pcAttrName = NULL;
				char const *pcAttrValue = NULL;
				while(pkAttr)
				{
					pcAttrName = pkAttr->Name();
					pcAttrValue = pkAttr->Value();

					if(strcmp(pcAttrName, "SCRIPT") == 0)
					{
						m_kScript = pcAttrValue;
					}
					else if(strcmp(pcAttrName, "PARAM") == 0)
					{
						m_iParam = atoi(pcAttrValue);
					}
					else if(strcmp(pcAttrName, "PARAM2") == 0)
					{
						m_iParam2 = atoi(pcAttrValue);
					}
					else if(strcmp(pcAttrName, "PARAM_STRING") == 0)
					{
						SAFE_STRNCPY(acString, pcAttrValue);
						m_wstrParam = UNI(acString);
						m_wstrParam += L'\0';
					}
					else if(strcmp(pcAttrName, "PARAM_STRING_QUEST1") == 0 ||
						strcmp(pcAttrName, "PARAM_STRING_QUEST2") == 0 ||
						strcmp(pcAttrName, "PARAM_STRING_QUEST3") == 0 ||
						strcmp(pcAttrName, "PARAM_STRING_QUEST4") == 0 ||
						strcmp(pcAttrName, "PARAM_STRING_QUEST5") == 0 ||
						strcmp(pcAttrName, "PARAM_STRING_QUEST6") == 0 ||
						strcmp(pcAttrName, "PARAM_STRING_QUEST7") == 0 ||
						strcmp(pcAttrName, "PARAM_STRING_QUEST8") == 0 ||
						strcmp(pcAttrName, "PARAM_STRING_QUEST9") == 0 )
					{
						typedef std::list< std::string > CONT_STR;
						CONT_STR kContList;
						BM::vstring::CutTextByKey( std::string( pcAttrValue ), std::string("/"), kContList );
						CONT_STR::const_iterator c_iter = kContList.begin();
						if( kContList.end() != c_iter )
						{
							SQuestTeleport kQuestTeleport;
							kQuestTeleport.iQuestID = PgStringUtil::SafeAtoi(*c_iter);					++c_iter;
							kQuestTeleport.kTeleportID = PgStringUtil::SafeUni( (*c_iter).c_str() );	++c_iter;

							m_kContQuestTeleport.push_back( kQuestTeleport );
						}
					}
					else if(strcmp(pcAttrName, "ERROR_MSG") == 0)
					{
						m_iErrorMsgID = atoi(pcAttrValue);
					}
					else if(strcmp(pcAttrName, "ENTITY_NO") == 0)
					{
					}
					else if(strcmp(pcAttrName, "INIT_ENTITY_TEAM") == 0)
					{
						m_iEntityTeam = atoi(pcAttrValue);
					}
					else if(strcmp(pcAttrName, "INIT_OBJECT_NO") == 0)
					{
						m_iObjectNo = atoi(pcAttrValue);
					}
					else if(strcmp(pcAttrName, "INIT_OBJECT_TEAM") == 0)
					{
						m_iObjectTeam = atoi(pcAttrValue);
					}
					else if(strcmp(pcAttrName, "LINK") == 0)
					{
						m_ContEntityLinkName.push_back(pcAttrValue);
					}
					else if(strcmp(pcAttrName, "ENTITY_LV") == 0)
					{
					}
					else if(strcmp(pcAttrName, "DO_LEAVE_SCRIPT") == 0)
					{
						m_bDoOnLeaveScript = atoi(pcAttrValue) ? true : false;
					}
					else if(strcmp(pcAttrName, "ADDED_TYPE") == 0)
					{
						if(strcmp(pcAttrValue, "RADAR") == 0)
						{
							m_eAddedType = ADD_RADAR;
						}
						else
						{
							PgXmlError1(pkElement, "Incorrect Trigger Added Type [%s]", pcAttrValue);
						}
					}
					else if(strcmp(pcAttrName, "RED_ING_EFFECT_NO") == 0)
					{
					}
					else if(strcmp(pcAttrName, "BLUE_ING_EFFECT_NO") == 0)
					{
					}
					else if(strcmp(pcAttrName, "RED_LV1_ENTITY_NO") == 0)
					{
					}
					else if(strcmp(pcAttrName, "RED_LV2_ENTITY_NO") == 0)
					{
					}
					else if(strcmp(pcAttrName, "RED_LV3_ENTITY_NO") == 0)
					{
					}
					else if(strcmp(pcAttrName, "BLUE_LV1_ENTITY_NO") == 0)
					{
					}
					else if(strcmp(pcAttrName, "BLUE_LV2_ENTITY_NO") == 0)
					{
					}
					else if(strcmp(pcAttrName, "BLUE_LV3_ENTITY_NO") == 0)
					{
					}
					else if(strcmp(pcAttrName, "LOADING_IMAGE_ID") == 0)
					{
						m_LoadingImageID = atoi(pcAttrValue);
					}
					else if(strcmp(pcAttrName, "RAG_UI") == 0)
					{
						m_RagUI = atoi(pcAttrValue);
					}
					else if(strcmp(pcAttrName, "ABSOLUTE_MOVE") == 0)
					{
					}
					else if(strcmp(pcAttrName, "NEXTFLOOR") == 0)
					{
					}
					else
					{
						m_ParamMap.insert(std::make_pair(std::string(pcAttrName),
							std::string(pcAttrValue)));
					}

					pkAttr = pkAttr->Next();
				}

				if ( TRIGGER_TYPE_PORTAL == m_eTriggerType )
				{
					CONT_DEFMAP const * pkContDefMap = NULL;
					g_kTblDataMgr.GetContDef(pkContDefMap);

					CONT_DEF_QUEST_REWARD const* pkQuestReward = NULL;
					g_kTblDataMgr.GetContDef(pkQuestReward);

					if ( pkContDefMap && pkQuestReward )
					{
						{
							CONT_PORTAL_ACCESS::value_type kPortal;
							if ( kPortal.Build( pkElement, *pkContDefMap, *pkQuestReward ) )
							{
								m_kContPortalAccess.push_back( kPortal );
							}
						}

						pkElement = pkElement->FirstChildElement();
						while ( pkElement )
						{
							if ( !::strcmp(pkElement->Value(), "ITEM") )
							{
								if ( m_kContPortalAccess.empty() )
								{
									m_kContPortalAccess.resize(1);
								}

								CONT_PORTAL_ACCESS::value_type kPortal;
								if ( kPortal.Build( pkElement, *pkContDefMap, *pkQuestReward ) )
								{
									m_kContPortalAccess.push_back( kPortal );
								}
							}
							pkElement = pkElement->NextSiblingElement();
						}

						/*
						int iListIndex = 0;
						TiXmlElement *pkChildElement = pkElement->FirstChildElement();
						while ( pkChildElement )
						{
							if ( !::strcmp( pkChildElement->Value(), "ITEM" ) )
							{
								pkAttr = pkChildElement->FirstAttribute();
								while ( pkAttr )
								{
									pcAttrName = pkAttr->Name();
									pcAttrValue = pkAttr->Value();

									if ( !::strcmp(pcAttrName, "NAME") )
									{
										::sprintf_s( acString, 512, "NAME_%d", ++iListIndex );
										m_ParamMap.insert( std::make_pair(std::string(acString), std::string(pcAttrValue)) );
									}
		// 							else if( !::strcmp(pcAttrName, "PARAM") )
		// 							{
		// 								::sprintf_s( acString, 512, "PARAM_%d", iListIndex );
		// 								m_ParamMap.insert( std::make_pair(std::string(acString), std::string(pcAttrValue)) );
		// 							}
		// 							else if( !::strcmp(pcAttrName, "PARAM2") )
		// 							{
		// 								::sprintf_s( acString, 512, "PARAM2_%d", iListIndex );
		// 								m_ParamMap.insert( std::make_pair(std::string(acString), std::string(pcAttrValue)) );
		// 							}

									pkAttr = pkAttr->Next();
								}	
							}

							pkChildElement = pkChildElement->NextSiblingElement();
						}

						if ( iListIndex )
						{
							::sprintf_s( acString, 512, "%d", iListIndex );
							m_ParamMap.insert( std::make_pair(std::string("MapCount"), std::string(acString)) );
						}
						*/
					}
					else
					{
						PG_ASSERT_LOG( pkContDefMap );
						_PgMessageBox("Error", "Not Found Def Map" );
					}
				}
				else if( TRIGGER_TYPE_PARTYPORTAL == m_eTriggerType
					|| TRIGGER_TYPE_PARTY_MEMBER_PORTAL == m_eTriggerType
					|| TRIGGER_TYPE_RAG_PARTY_MEMBER_PORTAL == m_eTriggerType )
				{
					CONT_DEFMAP const* pkContDefMap = NULL;
					g_kTblDataMgr.GetContDef(pkContDefMap);

					CONT_DEF_QUEST_REWARD const* pkQuestReward = NULL;
					g_kTblDataMgr.GetContDef(pkQuestReward);

					if ( pkContDefMap && pkQuestReward )
					{
						CONT_PORTAL_ACCESS::value_type kPortal;
						if ( kPortal.Build( pkElement, *pkContDefMap, *pkQuestReward ) )
						{
							m_kContPortalAccess.push_back( kPortal );
						}
					}
				}
			}
			else if(strcmp(pcTagName, "SKILL") == 0)
			{ //트랩이 사용할 액션 (트랩에 닿은 대상에게 리액션/이펙트 발동)
				TiXmlAttribute *pkAttr = pkElement->FirstAttribute();
				char const *pcAttrName = NULL;
				char const *pcAttrValue = NULL;
				while(pkAttr)
				{
					pcAttrName = pkAttr->Name();
					pcAttrValue = pkAttr->Value();

					if(strcmp(pcAttrName, "ID") == 0)
					{
						m_iTriggerSkillID = atoi(pcAttrValue);
					}
					pkAttr = pkAttr->Next();
				}
			}
			else if(strcmp(pcTagName, "PHYSX") == 0)
			{
				float fActiveStartTime = 0.0f;
				float fActiveEndTime = 0.0f;

				TiXmlAttribute *pkAttr = pkElement->FirstAttribute();
				while(pkAttr)
				{
					char const *pcAttrName = pkAttr->Name();
					char const *pcAttrValue = pkAttr->Value();

					if(strcmp(pcAttrName, "ACTIVE_START") == 0)
					{
						fActiveStartTime = (float)atof(pcAttrValue);
					}
					else if(strcmp(pcAttrName, "ACTIVE_END") == 0)
					{
						fActiveEndTime = (float)atof(pcAttrValue);
					}
					else if(strcmp(pcAttrName, "ACTIVE_TOTAL") == 0)
					{
						m_fPhysXActiveTotalTime = (float)atof(pcAttrValue);
					}					
					else
					{
						m_ParamMap.insert(std::make_pair(std::string(pcAttrName),
							std::string(pcAttrValue)));
					}
					
					pkAttr = pkAttr->Next();
				}

				if (fActiveStartTime != fActiveEndTime)
				{
					m_kPhysXActiveTimeCont.insert(std::make_pair(fActiveStartTime, fActiveEndTime));
				}
			}
			else if(strcmp(pcTagName, "PARTICLE") == 0)
			{
				float fEmitStartTime = 0.0f;
				float fEmitEndTime = 0.0f;

				TiXmlAttribute *pkAttr = pkElement->FirstAttribute();
				while(pkAttr)
				{
					char const *pcAttrName = pkAttr->Name();
					char const *pcAttrValue = pkAttr->Value();

					if(strcmp(pcAttrName, "EMIT_START") == 0)
					{
						fEmitStartTime = (float)atof(pcAttrValue);
					}
					else if(strcmp(pcAttrName, "EMIT_END") == 0)
					{
						fEmitEndTime = (float)atof(pcAttrValue);
					}
					else if(strcmp(pcAttrName, "EMIT_TOTAL") == 0)
					{
						m_fParticleEmitTotalTime = (float)atof(pcAttrValue);
					}
					else
					{
						m_ParamMap.insert(std::make_pair(std::string(pcAttrName),
							std::string(pcAttrValue)));
					}
					
					pkAttr = pkAttr->Next();
				}

				if (fEmitStartTime != fEmitEndTime)
				{
					m_kParticleEmitTimeCont.insert(std::make_pair(fEmitStartTime, fEmitEndTime));
				}
			}
			else if( 0 == ::strcmp(pcTagName, CAMTRG_CAMERA_ADJUST.c_str()) )
			{
				TiXmlAttribute const* pkCamADAttr = pkElement->FirstAttribute();
				float fValue = 0.0f;				

				while(pkCamADAttr)
				{
					fValue = static_cast<float>( ::atof(pkCamADAttr->Value()));

					if( 0 == ::strcmp(pkCamADAttr->Name(), CAMTRG_UP.c_str()) )
					{
						m_kContCamTrgInfo.insert(std::make_pair( pkCamADAttr->Name(), fValue));
					}
					else if( 0 == ::strcmp(pkCamADAttr->Name(), CAMTRG_WIDTH.c_str()) )
					{
						m_kContCamTrgInfo.insert(std::make_pair( pkCamADAttr->Name(), fValue));					
					}
					else if( 0 == ::strcmp(pkCamADAttr->Name(), CAMTRG_ZOOM.c_str()) )
					{// 카메라 트리거가 회전이 가능하므로 minus 값이 들어올 수 있음
						//if( 0 > fValue )
						//{
						//	_PgMessageBox("CameraTriggerParsing Error", "%s\nZOOM can't be minus value",GetID().c_str());
						//	fValue = 0.0f;
						//}
						m_kContCamTrgInfo.insert(std::make_pair( pkCamADAttr->Name(), fValue));
					}
					else if( 0 == ::strcmp(pkCamADAttr->Name(), CAMTRG_TARGET_Z.c_str()) )
					{
						m_kContCamTrgInfo.insert(std::make_pair( pkCamADAttr->Name(), fValue));						
					}
					else if( 0 == ::strcmp(pkCamADAttr->Name(), CAMTRG_TARGET_Y.c_str()) )
					{// 카메라 트리거가 회전이 가능하므로 minus 값이 들어올 수 있음
						//if( 0 > fValue )
						//{							
						//	_PgMessageBox("CameraTriggerParsing Error", "%s\nTARGET_Y can't be minus value",GetID().c_str());
						//	fValue = 0.0f;
						//}
						m_kContCamTrgInfo.insert(std::make_pair( pkCamADAttr->Name(), fValue));												
					}
					else if( 0 == ::strcmp(pkCamADAttr->Name(), CAMTRG_TARGET_X.c_str()) )
					{
						m_kContCamTrgInfo.insert(std::make_pair( pkCamADAttr->Name(), fValue));
					}
					else if( 0 == ::strcmp(pkCamADAttr->Name(), CAMTRG_GROUP.c_str()))
					{
						m_kContCamTrgInfo.insert(std::make_pair( pkCamADAttr->Name(), fValue));
					}
					
					pkCamADAttr = pkCamADAttr->Next();
				}
				// 최소한의 값이 들어 갔는지 체크함
				CheckCamTrgInfoAndFix();
			}
			else if(strcmp(pcTagName, "SIMPLE_TIME_EVENT") == 0)
			{
				SSIMPLETIMELIMIT kTimeLimit;
				TiXmlAttribute *pkAttr = pkElement->FirstAttribute();
				while(pkAttr)
				{
					char const *pcAttrName = pkAttr->Name();
					char const *pcAttrValue = pkAttr->Value();

					if(strcmp(pcAttrName, "FROM") == 0)
					{
						::sscanf(pcAttrValue,"%d,%d",&kTimeLimit.kBegin.byHour,&kTimeLimit.kBegin.byMin);
					}
					else if(strcmp(pcAttrName, "TO") == 0)
					{
						::sscanf(pcAttrValue,"%d,%d",&kTimeLimit.kEnd.byHour,&kTimeLimit.kEnd.byMin);
					}
					else if(strcmp(pcAttrName, "DAYOFWEEK") == 0)
					{
						kTimeLimit.kDayOfWeek.SetDay(EDOW_NONE);
						VEC_STRING kVec;
						PgStringUtil::BreakSep(pcAttrValue, kVec, "/");
						for(VEC_STRING::const_iterator it = kVec.begin(); it != kVec.end(); ++it)
						{
							if((*it) == "MON") { kTimeLimit.kDayOfWeek.AddDay(EDOW_MON); }
							else if((*it) == "TUE") { kTimeLimit.kDayOfWeek.AddDay(EDOW_TUE); }
							else if((*it) == "WED") { kTimeLimit.kDayOfWeek.AddDay(EDOW_WED); }
							else if((*it) == "THU") { kTimeLimit.kDayOfWeek.AddDay(EDOW_THU); }
							else if((*it) == "FRI") { kTimeLimit.kDayOfWeek.AddDay(EDOW_FRI); }
							else if((*it) == "SAT") { kTimeLimit.kDayOfWeek.AddDay(EDOW_SAT); }
							else if((*it) == "SUN") { kTimeLimit.kDayOfWeek.AddDay(EDOW_SUN); }
						}
					}
					pkAttr = pkAttr->Next();
				}

				m_kContTimeEvent.push_back(kTimeLimit);
			}
			else if(strcmp(pcTagName, "REMOTE_OBJECT") == 0)
			{
				ParseXML_RemoteObject(pkElement);
			}
			else
			{
				PgXmlError1(pkElement, "XmlParse: Incoreect Tag '%s'", pcTagName);
				break;
			}
		}

	default:
		break;
	}

	const TiXmlNode* pkNextNode = pkNode->NextSibling();
	if(pkNextNode)
	{
		if(!ParseXml(pkNextNode))
		{
			return false;
		}
	}

	return true;
}

NiTimeController* GetTimeController(NiObjectNET* pkObj)
{
    NiTimeController* pkControl = pkObj->GetControllers();
	if (pkControl)
	{
		return pkControl;
	}

    if (NiIsKindOf(NiAVObject, pkObj))
    {
        NiAVObject* pkAVObj = (NiAVObject*) pkObj;

        // recurse on properties
        NiTListIterator kPos = pkAVObj->GetPropertyList().GetHeadPos();
        while (kPos)
        {
            NiProperty* pkProperty = pkAVObj->GetPropertyList().GetNext(kPos);
            if (pkProperty && pkProperty->GetControllers())
			{
				NiTimeController* pkRetControl = GetTimeController(pkProperty);
				if (pkRetControl)
				{
					return pkRetControl;
				}
			}
        }
    }

    if (NiIsKindOf(NiNode, pkObj))
    {
        NiNode* pkNode = (NiNode*) pkObj;

        // recurse on children
        for (unsigned int i = 0; i < pkNode->GetArrayCount(); i++)
        {
            NiAVObject* pkChild;

            pkChild = pkNode->GetAt(i);
            if (pkChild)
			{
				NiTimeController* pkRetControl = GetTimeController(pkChild);
				if (pkRetControl)
				{
					return pkRetControl;
				}
			}
        }
    }

	return 0;
}

bool PgTrigger::IsPhysXActiveTime()
{
	if(!g_pkWorld)
	{
		return	false;
	}
	float const fAccumTime = g_pkWorld->GetAccumTime();

	CONT_TIME::const_iterator itr = m_kPhysXActiveTimeCont.begin();
    NiTimeController* pkControl = GetTimeController(m_spTriggerObject);

	if (itr != m_kPhysXActiveTimeCont.end() && pkControl && m_fPhysXActiveTotalTime != 0.0f)
	{
		// Emit제어.
		unsigned __int64 iAccumTime = (unsigned __int64)((fAccumTime + pkControl->GetPhase()) * 1000);
		unsigned __int64 iTotal = (unsigned __int64)(m_fPhysXActiveTotalTime * 1000);
		unsigned __int64 iTime = iAccumTime % iTotal;
		float fTime = iTime * 0.001f;		// millisec -> sec

		//float fTime = pkControl->GetLastScaledTime();
		while(itr != m_kPhysXActiveTimeCont.end())
		{
			float fStartTime = (*itr).first;
			float fEndTime = (*itr).second;
			if (fTime > fStartTime && fTime < fEndTime)
			{
				return true;
			}

			++itr;
		}
		return false;
	}
	else
	{
		return true;
	}
	return false;
}

bool PgTrigger::IsPortalAccess( size_t const iIndex )const
{
	if ( true == IsHavePortalAccess(iIndex) )
	{
		PgPlayer *pkPlayer = g_kPilotMan.GetPlayerUnit();
		if ( pkPlayer )
		{
			bool const bMaster = ( (g_kParty.PartyGuid() == BM::GUID::NullData()) ? true : (g_kParty.MasterGuid() == pkPlayer->GetID()) );
			return m_kContPortalAccess.at(iIndex).IsAccess( pkPlayer, bMaster, NULL );
		}
	}
	return false;
}

void PgTrigger::IsAccessInfo(size_t const iIndex, SPortalAccessInfo & rkAccessInfo)const
{
	if ( true == IsHavePortalAccess(iIndex) )
	{
		PgPlayer *pkPlayer = g_kPilotMan.GetPlayerUnit();
		if ( pkPlayer )
		{
			bool const bMaster = ( (g_kParty.PartyGuid() == BM::GUID::NullData()) ? true : (g_kParty.MasterGuid() == pkPlayer->GetID()) );
			return m_kContPortalAccess.at(iIndex).IsAccessInfo( pkPlayer, bMaster, rkAccessInfo );
		}
	}
}

float const PgTrigger::GetCamTrgInfoHeight() const
{
	CONT_CAMTRG_INFO::const_iterator itor = m_kContCamTrgInfo.find(CAMTRG_UP);
	if( itor != m_kContCamTrgInfo.end() )
	{
		return (*itor).second;
	}
	return 0.0f;
}

float const PgTrigger::GetCamTrgInfoWidth() const
{
	CONT_CAMTRG_INFO::const_iterator itor = m_kContCamTrgInfo.find(CAMTRG_WIDTH);
	if( itor != m_kContCamTrgInfo.end() )
	{
		return (*itor).second;
	}
	return 0.0f;
}

float const PgTrigger::GetCamTrgInfoZoom() const
{
	CONT_CAMTRG_INFO::const_iterator itor = m_kContCamTrgInfo.find(CAMTRG_ZOOM);
	if( itor != m_kContCamTrgInfo.end() )
	{
		return (*itor).second;
	}
	return 0.0f;
}

float const PgTrigger::GetCamTrgInfoTargetHeight() const
{	
	CONT_CAMTRG_INFO::const_iterator itor = m_kContCamTrgInfo.find(CAMTRG_TARGET_Z);
	if( itor != m_kContCamTrgInfo.end() )
	{
		return (*itor).second;
	}
	return 0.0f;
}

float const PgTrigger::GetCamTrgInfoTargetDepth() const
{	
	CONT_CAMTRG_INFO::const_iterator itor = m_kContCamTrgInfo.find(CAMTRG_TARGET_Y);
	if( itor != m_kContCamTrgInfo.end() )
	{
		return (*itor).second;
	}
	return 0.0f;
}

float const PgTrigger::GetCamTrgInfoTargetWidth() const
{	
	CONT_CAMTRG_INFO::const_iterator itor = m_kContCamTrgInfo.find(CAMTRG_TARGET_X);
	if( itor != m_kContCamTrgInfo.end() )
	{
		return (*itor).second;
	}
	return 0.0f;
}

int const PgTrigger::GetCamTrgInfoGroup() const
{	
	CONT_CAMTRG_INFO::const_iterator itor = m_kContCamTrgInfo.find(CAMTRG_GROUP);
	if( itor != m_kContCamTrgInfo.end() )
	{
		return static_cast<int>((*itor).second);
	}
	return 0;
}

bool PgTrigger::CheckCamTrgInfoAndFix()
{// 최소한으로 필요한 값이 들어왔는지 체크하고, 없으면 알려준다.
	bool bResult = true;

	std::string kErrMsg("Check CAMERA_ADJUST value in ");
	kErrMsg += GetID();	

	CONT_CAMTRG_INFO::const_iterator itor = m_kContCamTrgInfo.find(CAMTRG_UP);
	if( itor == m_kContCamTrgInfo.end() )
	{
		std::string kCopyErrMsg(kErrMsg);
		kCopyErrMsg += std::string("\n \'UP\' now use default set \'0\'.");
		//::MessageBox(NULL, UNI(kCopyErrMsg) , _T("CameraTriggerParsing Error"), MB_ICONERROR);
		_PgMessageBox("CameraTriggerParsing Error", "%s",kCopyErrMsg.c_str() );
		m_kContCamTrgInfo.insert( std::make_pair(CAMTRG_UP, 0.0f) );
		bResult = false;
	}
	
	itor = m_kContCamTrgInfo.find(CAMTRG_ZOOM);
	if( itor == m_kContCamTrgInfo.end() )
	{
		std::string kCopyErrMsg(kErrMsg);
		kCopyErrMsg += std::string("\n \'ZOOM\' now use default set \'0\'.");
		//::MessageBox(NULL, UNI(kCopyErrMsg) , _T("CameraTriggerParsing Error"), MB_ICONERROR);
		_PgMessageBox("CameraTriggerParsing Error", "%s",kCopyErrMsg.c_str() );
		m_kContCamTrgInfo.insert( std::make_pair(CAMTRG_ZOOM, 0.0f) );
		bResult = false;
	}

	return bResult;
}

int PgTrigger::GetPortalAccessName( size_t const iIndex )const
{
	if ( true == IsHavePortalAccess(iIndex) )
	{
		return m_kContPortalAccess.at(iIndex).GetName();
	}
	return 0;
}

std::wstring PgTrigger::FindQuestTeleport()
{
	std::wstring kTeleportID;

	std::list<SQuestTeleport>::const_iterator iter = m_kContQuestTeleport.begin();
	while( m_kContQuestTeleport.end() != iter)
	{
		int iQuestID = (*iter).iQuestID;
		PgPlayer* pPlayer = g_kPilotMan.GetPlayerUnit();
		if(pPlayer)
		{
			SUserQuestState const *pkQuestState = pPlayer->GetQuestState(iQuestID);
			if( pkQuestState )
			{
				if( QS_Ing == pkQuestState->byQuestState )
				{
					kTeleportID = (*iter).kTeleportID;
					break;
				}
			}
		}
		++iter;
	}

	return kTeleportID;
}

bool PgTrigger::IsActiveTime(void) const
{
	if(m_kContTimeEvent.empty())
	{ //시간 체크 컨테이너가 비어있다면 시간 체크를 하지 않고 언제나 활성상태
		return true;
	}
	else
	{
		SYSTEMTIME	kNow;
		g_kEventView.GetLocalTime(&kNow);
#ifndef EXTERNAL_RELEASE
		if(g_pkApp->IsSingleMode())
		{
			::GetSystemTime(&kNow);
		}
#endif
		for(CONT_TIME_EVENT::const_iterator iter = m_kContTimeEvent.begin(); iter != m_kContTimeEvent.end(); ++iter)
		{
			if((*iter).CheckTimeIsInDuration(kNow))
			{
				return true;
			}
		}
	}
	return false;
}

int PgTrigger::GetTriggerReactionSkillNo(CUnit* pkTargetUnit)
{
	if(m_iTriggerSkillID == 0)
	{
		return 0;
	}

	GET_DEF(CSkillDefMgr, kSkillDefMgr);
	CSkillDef const* pkSkillDef = kSkillDefMgr.GetDef(m_iTriggerSkillID);
	if (!pkSkillDef || !pkTargetUnit)
	{
		return 0;
	}
	return  pkSkillDef->GetAbil(AT_DAM_EFFECT_S + pkTargetUnit->GetAbil(AT_UNIT_SIZE) - 1);
}

std::wstring PgTrigger::GetTriggerReactionSkillName(CUnit* pkTargetUnit)
{
	int const iReactionSkillNum = GetTriggerReactionSkillNo(pkTargetUnit);
	if(iReactionSkillNum == 0)
	{
		return std::wstring();
	}

	GET_DEF(CSkillDefMgr, kSkillDefMgr);
	CSkillDef const* pkReactionSkillDef = kSkillDefMgr.GetDef(iReactionSkillNum);
	if(!pkReactionSkillDef)
	{
		return std::wstring();
	}
	return std::wstring( pkReactionSkillDef->GetActionName() );
}

//////////////////////접촉 시 애니 연출 기능 - 시작 - //////////////////////
void PgTrigger::ParseXML_RemoteObject(const TiXmlElement *pkElement)
{
	while( NULL != pkElement )
	{
		TiXmlAttribute const* pkAttribute = pkElement->FirstAttribute();
		SREMOTE_OBJECT kTemp_RemoteObject;
		while( NULL!= pkAttribute)
		{
			char const *pkName = pkAttribute->Name();
			char const *pkValue = pkAttribute->Value();
			if( 0 == strcmp( "TARGET_NAME", pkName) )
			{
				kTemp_RemoteObject.kTargetObjectName = pkValue;
			}
			else if( 0 == strcmp( "ANI_START_ID", pkName) )
			{
				kTemp_RemoteObject.kStartAniID = pkValue;
				LWR(kTemp_RemoteObject.kStartAniID);
			}
			else if( 0 == strcmp( "ANI_START_TIME", pkName) )
			{
				kTemp_RemoteObject.dwStartAniTime = atol(pkValue);
			}
			else if( 0 == strcmp( "USE_RANDOM_START", pkName) )
			{
				kTemp_RemoteObject.bUseRandomStart = ( 0==strcmp("TRUE", pkValue) ? true : false);
			}
			else if( 0 == strcmp( "ANI_NEXT_ID", pkName) )
			{
				kTemp_RemoteObject.kNextAniID = pkValue;
				LWR(kTemp_RemoteObject.kNextAniID);
			}
			pkAttribute = pkAttribute->Next();
		}
		if( 0 == kTemp_RemoteObject.kTargetObjectName.size() )
		{
			_PgMessageBox("Failed Insert Remote Object", " Not Have TargetObject Name ObjectID");
		}
		else if( 0 == kTemp_RemoteObject.kStartAniID.size() )
		{
			_PgMessageBox("Failed Insert Remote Object", " Not Have StartAniName : ObjectID : %s",kTemp_RemoteObject.kTargetObjectName.c_str() );
		}
		else
		{
			std::pair<CONT_MAP_REMOTE_OBJECT::iterator, bool> kRet = m_kContRemoteObject.insert( std::make_pair( kTemp_RemoteObject.kTargetObjectName, kTemp_RemoteObject ) );
			if( false == kRet.second )
			{//왜 중복 파싱하는지?
				//if( kRet.first != m_kContRemoteObject.end() )
				//{
				//	_PgMessageBox("Failed Insert Remote Object", " Already Insert ObjectID : %s",kTemp_RemoteObject.kTargetObjectName.c_str() );
				//}
				//else
				//{
				//	_PgMessageBox("Failed Insert Remote Object", "ObjectID : %s",kTemp_RemoteObject.kTargetObjectName.c_str() );
				//}
			}
		}
		pkElement = pkElement->NextSiblingElement();
	}
}
void PgTrigger::SetRemoteObject()
{
	if( NULL == g_pkWorld )
	{
		return;
	}
	CONT_MAP_REMOTE_OBJECT::const_iterator itor_Object = m_kContRemoteObject.begin();
	while( m_kContRemoteObject.end() != itor_Object )
	{
		g_pkWorld->SetCurrentRemoteObject( (*itor_Object).second );
		++itor_Object;
	}
	return;
}
bool PgTrigger::GetRemoteObject( std::string const &rkObjectName, CONT_MAP_REMOTE_OBJECT::mapped_type &rkObject )
{
	CONT_MAP_REMOTE_OBJECT::const_iterator itor_Object = m_kContRemoteObject.find( rkObjectName );
	if( m_kContRemoteObject.end() != itor_Object )
	{
		rkObject = (*itor_Object).second;
		return true;
	}
	return false;
}
//////////////////////접촉 시 애니 연출 기능 - 끝 - //////////////////////