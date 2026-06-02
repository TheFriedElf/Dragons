#include "stdafx.h"
#include "PgStatusEffect.H"
#include "PgPilot.H"
#include "PgPilotMan.H"
#include "lwUI.H"
#include "PgActor.H"
#include "PgParticleMan.H"
#include "PgHeadBuffIcon.H"
#include "PgSoundMan.h"
#include "PgLocalManager.H"
#include "PgAction.h"
#include "PgChatMgrClient.h"
#include "PgSafeFoamMgr.h"
#include "PgOption.h"
#include "PgNifMan.h"
#include "PgScreenEffect.h"
#include "lwQuaternion.h"
#include "PgEventScriptSystem.h"
#include "NewWare/Renderer/DrawWorkflow.h"
#include "NewWare/Renderer/Kernel/RenderStateTagExtraData.h"


PgStatusEffectMan	g_kStatusEffectMan;
extern bool GetDefString(int const iTextNo, std::wstring const *&pkOut);

void SetInputDirReverseCount(bool bIsMyActor, bool bSet)
{
	if(bIsMyActor && g_pkLocalManager)
	{
		if(bSet)
		{
			g_pkLocalManager->SetInputDirReverse();
		}
		else
		{
			g_pkLocalManager->ClearInputDirReverse();
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	class	PgStatusEffect
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool PgStatusEffect::ParseXml(TiXmlNode const* pkNode, void* pArg, bool bUTF8)
{
	int const iType = pkNode->Type();
	
	switch(iType)
	{
	case TiXmlNode::ELEMENT:
		{
			TiXmlElement* pkElement = (TiXmlElement *)pkNode;
			assert(pkElement);
			
			char const* pcTagName = pkElement->Value();

			if(0 == _stricmp(pcTagName, "STATUS_EFFECT"))
			{
				TiXmlAttribute* pkAttr = pkElement->FirstAttribute();
				while(pkAttr)
				{
					char const* pcName = pkAttr->Name();
					char const* pcValue = pkAttr->Value();

					if(0 == _stricmp(pcName, "TYPE"))
					{
						m_kType = std::string(pcValue);
					}
					else if(0 == _stricmp(pcName, "HIDE_ENABLE"))
					{
						m_iHideEnable = (atoi(pcValue));
					}
					else if(0 == _stricmp(pcName, "ONLY_CASTER"))
					{
						m_bOnlyCaster = static_cast<bool>( (atoi(pcValue)) );
					}
				
					pkAttr = pkAttr->Next();
				}


				// 자식 노드들을 파싱한다.
				// 첫 자식만 여기서 걸어주면, 나머지는 NextSibling에 의해서 자동으로 파싱된다.
				TiXmlNode* pkChildNode = pkElement->FirstChild();
				if(0 != pkChildNode)
				{

					if(!ParseXml(pkChildNode))
					{
						return false;
					}
				}
			}
			else if(0 == _stricmp(pcTagName, "REDIRECTION_BY_EFFECTVALUE"))
			{
				// 자식 노드들을 파싱한다.
				// 첫 자식만 여기서 걸어주면, 나머지는 NextSibling에 의해서 자동으로 파싱된다.
				TiXmlNode* pkChildNode = pkElement->FirstChild();
				if(0 != pkChildNode)
				{

					if(!ParseXml(pkChildNode))
					{
						return false;
					}
				}
			}
			else if(0 == _stricmp(pcTagName, "REDIRECTION"))
			{
				TiXmlAttribute* pkAttr = pkElement->FirstAttribute();

				int	iValue = 0;
				std::string	kXMLID;
				while(pkAttr)
				{
					char const* pcName = pkAttr->Name();
					char const* pcValue = pkAttr->Value();

					if(0 == _stricmp(pcName, "VALUE"))
					{
						iValue = atoi(pcValue);
					}
					else if(0 == _stricmp(pcName, "XML_ID"))
					{
						kXMLID = std::string(pcValue);
					}
				
					pkAttr = pkAttr->Next();
				}

				if(NULL == GetRedirectionXMLID(iValue))
				{
					m_RedirectionMap.insert(std::make_pair(iValue,kXMLID));
				}
			}
			else if(0 == _stricmp(pcTagName, "ITEM"))
			{
				TiXmlAttribute* pkAttr = pkElement->FirstAttribute();
				stStatusEffectParticle* pkParticle = NULL;
				stStatusEffectSkillText	*pkSkillText = NULL;
												
				int	iInputSlotID = -1;
				bool bInputSlotEnable = true;

				int iEquipPos = -1;
				int iEquipItemNo = -1;

				std::string kChangeActionFrom;
				std::string kChangeActionTo;

				while(pkAttr)
				{
					char const* pcName = pkAttr->Name();
					char const* pcValue = pkAttr->Value();

					if(0 == _stricmp(pcName, "TYPE"))
					{
						if(0 == _stricmp(pcValue,"COLOR"))						{ m_ValidItems |= IT_COLOR; }
						else if(0 == _stricmp(pcValue,"ALPHA"))					{ m_ValidItems |= IT_ALPHA; }
						else if(0 == _stricmp(pcValue,"HEAD_SIZE"))				{ m_ValidItems |= IT_HEAD_SIZE; }
						else if(0 == _stricmp(pcValue,"PARTICLE"))	
						{
							pkParticle = new stStatusEffectParticle();							
							m_kParticleContainer.push_back(pkParticle);

							m_ValidItems|=IT_PARTICLE;
						}
						else if(0 == _stricmp(pcValue,"DETACH_PARTICLE"))	
						{
							pkParticle = new stStatusEffectParticle();							
							m_kDetachParticleContainer.push_back(pkParticle);

							m_ValidItems|=IT_DETACH_PARTICLE;
						}
						else if(0 == _stricmp(pcValue,"INVISIBLE"))				{ m_ValidItems |= IT_INVISIBLE; }
						else if(0 == _stricmp(pcValue,"STUN"))					{ m_ValidItems |= IT_STUN; }
						else if(0 == _stricmp(pcValue,"ONLY_MOVE_ACTION"))		{ m_ValidItems |= IT_ONLY_MOVE_ACTION; }
						else if(0 == _stricmp(pcValue,"FREEZED"))				{ m_ValidItems |= IT_FREEZED; }
						else if(0 == _stricmp(pcValue,"HIDE_PARTS"))			{ m_ValidItems |= IT_HIDE_PARTS; }
						else if(0 == _stricmp(pcValue,"TRANSFORM"))				{ m_ValidItems |= IT_TRANSFORM; }
						else if(0 == _stricmp(pcValue,"ONLY_DEFAULT_ATTACK"))	{ m_ValidItems |= IT_ONLY_DEFAULT_ATTACK; }
						else if(0 == _stricmp(pcValue,"HIDE"))					{ m_ValidItems |= IT_HIDE; }
						else if(0 == _stricmp(pcValue,"HIDE_NODE"))				{ m_ValidItems |= IT_HIDE_NODE; }
						else if(0 == _stricmp(pcValue,"HIDE_SHADOW"))			{ m_ValidItems |= IT_HIDE_SHADOW; }
						else if(0 == _stricmp(pcValue,"CHANGE_DEFAULT_ATTACK")) { m_ValidItems |= IT_CHANGE_DEFAULT_ATTACK; }
						else if(0 == _stricmp(pcValue,"INPUT_SLOT")) 			{ m_ValidItems |= IT_INPUT_SLOT; }
						else if(0 == _stricmp(pcValue,"DETACH_ACTION")) 		{ m_ValidItems |= IT_DETACH_ACTION; }
						else if(0 == _stricmp(pcValue,"START_ACTION"))			{ m_ValidItems |= IT_START_ACTION; }
						else if(0 == _stricmp(pcValue,"CHANGE_ACTION"))			{ m_ValidItems |= IT_CHANGE_ACTION; }
						else if(0 == _stricmp(pcValue,"INVINCIBLE"))			{ m_ValidItems |= IT_INVINCIBLE; }
						else if(0 == _stricmp(pcValue,"SKILL_TEXT")) 
						{
							pkSkillText = new stStatusEffectSkillText();
							m_kSkillTextContainer.push_back(pkSkillText);

							m_ValidItems|=IT_SKILL_TEXT;
						}
						else if(0 == _stricmp(pcValue,"PLAY_SOUND")) 			{ m_ValidItems |= IT_PLAY_SOUND; }
						else if(0 == _stricmp(pcValue,"BODY_SIZE"))				{ m_ValidItems |= IT_BODY_SIZE; }
						else if(0 == _stricmp(pcValue,"DARK_SIGHT")) 			{ m_ValidItems|=IT_DARK_SIGHT; }
						else if(0 == _stricmp(pcValue,"TEXTURE_CHANGE")) 		{ m_ValidItems|=IT_TEXTURE_CHANGE; }
						else if(0 == _stricmp(pcValue,"SHOW_WORLD_FOCUS_FILTER")) { m_ValidItems|=IT_WORLD_FOCUS_FILTER; }
						else if(0 == _stricmp(pcValue,"INPUTDIR_REVERSE"))		{ m_ValidItems|=IT_INPUTDIR_REVERSE; }						
						else if(0 == _stricmp(pcValue,"EQUIP_ITEM"))			{ m_ValidItems|=IT_EQUIP_ITEM; }
						else if(0 == _stricmp(pcValue,"WHITE_OUT"))				{ m_ValidItems|=IT_WHITE_OUT; }
						else if(0 == _stricmp(pcValue,"NOT_ACTION_SHIFT"))		{ m_ValidItems|=IT_NOT_ACTION_SHIFT; }
						else if(0 == _stricmp(pcValue,"EVENT_SCRIPT"))			{ m_ValidItems|=IT_EVENT_SCRIPT; }
						else 
						{
							//::MessageBox(NULL,UNI("Unknown Status Effect Item Type"),UNI(pcValue),MB_OK);
							_PgMessageBox( pcValue, "Unknown Status Effect Item Type" );
						}
					}
					else if(0 == _stricmp(pcName,"INPUT_ID"))
					{
						iInputSlotID = atoi(pcValue);
					}
					else if(0 == _stricmp(pcName,"INPUT_ENABLE"))
					{
						bInputSlotEnable = (0 == stricmp(pcValue,"TRUE"));
					}
					else if(0 == _stricmp(pcName,"NEW_DEFAULT_ATTACK"))
					{
						m_kNewDefaultAttack = std::string(pcValue);
					}
					else if(0 == _stricmp(pcName,"ORIGINAL_DEFAULT_ATTACK"))
					{
						m_kOriginalDefaultAttack = std::string(pcValue);
					}
					else if(0 == _stricmp(pcName, "TRANSFORM_ID"))	//	
					{
						m_kTransformID = std::string(pcValue);
					}
					else if(0 == _stricmp(pcName, "TRANSFORM_ID_FOR_MALE"))	//남자 캐릭터가 변신 할 때
					{
						m_kTransformIDForMale = std::string(pcValue);
					}
					else if(0 == _stricmp(pcName, "TRANSFORM_ID_FOR_FEMALE"))	//여자 캐릭터가 변신 할 때
					{
						m_kTransformIDForFemale = std::string(pcValue);
					}
					else if(0 == _stricmp(pcName, "TRANSFORM_NEXT_ACTION"))	//	
					{
						m_kTransformNextAction = std::string(pcValue);
					}
					else if(0 == _stricmp(pcName, "TRANSFORM_NAMENO"))		//변신했을 때 이름을 바꿀건가. DefString 번호로 하자
					{
						m_iTransformNameNo = atoi(pcValue);
					}
					else if(0 == _stricmp(pcName, "HIDE_PARTS_POS"))	//	
					{
						m_kHidePartContainer.push_back(atoi(pcValue));
					}
					else if(0 == _stricmp(pcName, "HEAD_SIZE"))
					{
						m_fHeadSize = static_cast<float>(atof(pcValue));
					}
					else if(0 == _stricmp(pcName, "BODY_SIZE"))
					{
						m_fBodySize = static_cast<float>(atof(pcValue));
					}
					else if(0 == _stricmp(pcName, "BODY_CHANGE_TIME"))
					{
						m_fBodyChangeTime = static_cast<float>(atof(pcValue));
					}
					else if( (0==_stricmp(pcName, "A")) || (0==_stricmp(pcName, "ALPHA_START_A")) )
					{
						m_kColorAlpha.a = static_cast<float>(atof(pcValue));
					}
					else if(0 == _stricmp(pcName, "ALPHA_END_A"))
					{
						m_fAlphaEnd = static_cast<float>(atof(pcValue));
					}
					else if(0 == _stricmp(pcName, "ALPHA_START_TRANSITIONTIME"))
					{
						m_fAlphaStartTransitionTime = static_cast<float>(atof(pcValue));
					}
					else if(0 == _stricmp(pcName, "ALPHA_END_TRANSITIONTIME"))
					{
						m_fAlphaEndTransitionTime = static_cast<float>(atof(pcValue));
					}
					else if(0 == _stricmp(pcName, "DIFFUSE"))
					{
						sscanf_s(pcValue,"%f,%f,%f",&m_kColorAlpha.r,&m_kColorAlpha.g,&m_kColorAlpha.b);
					}
					else if(0 == _stricmp(pcName,"SPECULAR"))
					{
						sscanf_s(pcValue,"%f,%f,%f",&m_kSpecular.r,&m_kSpecular.g,&m_kSpecular.b);
					}
					else if(0 == _stricmp(pcName, "COLOR_TRANSIT_SPEED"))
					{
						m_fColorTransitSpeed = static_cast<float>(atof(pcValue));
					}
					else if(0 == _stricmp(pcName, "COLOR_TRANSIT_TIME"))
					{
						m_fColorTransitTime = static_cast<float>(atof(pcValue));
					}
					else if(0 == _stricmp(pcName,"PARTICLE_ID"))
					{
						if(pkParticle)
						{
							pkParticle->m_kParticleID = std::string(pcValue);
						}
					}
					else if(0 == _stricmp(pcName,"PARTICLE_ATTACH_TARGET_NODE"))
					{
						if(pkParticle)
						{
							pkParticle->m_kAttachTargetNodeID = std::string(pcValue);
						}
					}
					else if(0 == _stricmp(pcName,"PARTICLE_ATTACH_TO_WORLD"))
					{
						if(pkParticle)
						{
							pkParticle->m_bAttachToWorld = (0 == stricmp(pcValue,"TRUE"));
						}
					}
					else if(0 == _stricmp(pcName,"PARTICLE_SEE_FRONT_WHEN_ONLY_ATTACH_POS"))
					{// 파티클을 위치점에 붙이는게 아니라면 적용되지 않음에 유의
						if(pkParticle)
						{
							pkParticle->m_bSeeFront = (0 == stricmp(pcValue,"TRUE"));
						}
					}
					else if(0 == _stricmp(pcName,"PARTICLE_RANDOM_OFFSET_MIN"))
					{
						if(pkParticle)
						{
							float fX = 0, fY = 0, fZ = 0;
							sscanf_s(pcValue,"%f,%f,%f",&fX,&fY,&fZ);
							pkParticle->m_kRandomOffsetMin.x = fX;
							pkParticle->m_kRandomOffsetMin.y = fY;
							pkParticle->m_kRandomOffsetMin.z = fZ;
						}
					}
					else if(0 == _stricmp(pcName,"PARTICLE_RANDOM_OFFSET_MAX"))
					{
						if(pkParticle)
						{
							float fX = 0, fY = 0, fZ = 0;
							sscanf_s(pcValue,"%f,%f,%f",&fX,&fY,&fZ);
							pkParticle->m_kRandomOffsetMax.x = fX;
							pkParticle->m_kRandomOffsetMax.y = fY;
							pkParticle->m_kRandomOffsetMax.z = fZ;
						}
					}
					else if(0 == _stricmp(pcName,"PARTICLE_FOLLOW_ROTATION_NODE"))
					{
						if(pkParticle)
						{
							pkParticle->m_kFollowRotationTargetNodeID = std::string(pcValue);
						}
					}
					else if(0 == _stricmp(pcName,"ATTACH_TO_POINT"))
					{
						if(pkParticle)
						{
							pkParticle->m_bAttachToPoint = (0 == _stricmp("TRUE", pcValue));
						}
					}
					else if(0 == _stricmp(pcName,"NO_FOLLOW_PARENT_ROTATION"))
					{
						if(pkParticle)
						{
							pkParticle->m_bNoFollowParentRotation = (0 == _stricmp("TRUE", pcValue));
						}
					}
					else if(0 == _stricmp(pcName,"PARTICLE_SCALE"))
					{
						if(pkParticle)
						{
							pkParticle->m_fScale = static_cast<float>(atof(pcValue));
						}
					}
					else if(0 == _stricmp(pcName,"PARTICLE_USE_AUTO_SCALE"))
					{
						if(pkParticle)
						{
							pkParticle->m_bAutoScale = (0==_stricmp("TRUE", pcValue));
						}
					}
					else if(0 == _stricmp(pcName,"NOT_DETACH_PARTICLE"))
					{
						if(pkParticle)
						{
							pkParticle->m_bNotDetachParticle = (0==_stricmp("TRUE", pcValue));
						}
					}
					else if(0 == _stricmp(pcName, "PHASE"))
					{
						if(pkParticle)
						{
							pkParticle->m_fPhase = static_cast<float>(atof(pcValue));
						}
					}
					else if(0 == _stricmp(pcName,"SHOW_OPTION"))
					{
						if(pkParticle)
						{
							// 1 : Me, 2 : MyTeam, 4 : Enemy or로 조합 가능 (7 : All)
							pkParticle->m_byShowOption = atoi(pcValue);
						}
					}
					else if(0 == _stricmp(pcName,"PARTICLE_SOUND"))
					{
						if(pkParticle
							&& pcValue
							)
						{
							pkParticle->m_kParticleSoundID = std::string(pcValue);
						}
					}
					else if(0 == _stricmp(pcName,"SKILL_TEXT_TYPE"))
					{
						if(pkSkillText)
						{
							int const SKILL_TEXT_MAX_INDEX = 11;
							char const* strSkillText[SKILL_TEXT_MAX_INDEX] = {	"POWER","DEF","MDEF","ATK","MATK",
																	"MOVE","MAXHP","MAXMP","CRITICAL","FLEE",
																	"ACCURY"
																  };

							for(int i = 0; i < SKILL_TEXT_MAX_INDEX; ++i)
							{
								if(0 == _stricmp(strSkillText[i], pcValue))
								{
									pkSkillText->m_iSkillTextType = i;
									pkSkillText->m_bySkillTextType = stStatusEffectSkillText::SESTT_SKILL_TEXT;
								}
							}

							int const SIMPLE_TEXT_MAX_INDEX = 3;
							char const* strSimpleText[SIMPLE_TEXT_MAX_INDEX] = {"MISS","DODGE","BLOCK"
																  };

							for(int i = 0; i < SIMPLE_TEXT_MAX_INDEX; ++i)
							{
								if(0 == _stricmp(strSimpleText[i], pcValue))
								{
									pkSkillText->m_iSkillTextType = i;
									pkSkillText->m_bySkillTextType = stStatusEffectSkillText::SESTT_SIMPLE_TEXT;
								}
							}

							if(-1 == pkSkillText->m_iSkillTextType)
							{
								//::MessageBox(NULL, UNI(pcValue), UNI("Unknown Skill Text ID"), MB_OK);
								_PgMessageBox("Unknown Skill Text ID", pcValue);
							}
						}
					}
					else if(0 == _stricmp(pcName,"SKILL_TEXT_UP"))
					{
						if(pkSkillText)
						{
							pkSkillText->m_bIsUp = (0 == _stricmp("TRUE", pcValue));
						}
					}
					else if(0 == _stricmp(pcName,"SOUND_ID"))
					{
						m_kSoundID = std::string(pcValue);
					}
					else if(0 == _stricmp(pcName,"LOOP_COUNT"))
					{
						m_iLoopCount = atoi(pcValue);
					}
					else if(0 == _stricmp(pcName,"DETACH_ACTION_ID"))
					{
						if(IT_DETACH_ACTION & m_ValidItems)
						{
							m_kDetachActionID = std::string(pcValue);
						}
					}
					else if(0 == _stricmp(pcName,"START_ACTION_ID"))
					{
						if(IT_START_ACTION & m_ValidItems)
						{
							m_kStartActionID = std::string(pcValue);
						}
					}
					else if(0 == _stricmp(pcName,"CHANGE_ACTION_FROM"))
					{
						if(IT_CHANGE_ACTION & m_ValidItems)
						{
							kChangeActionFrom = std::string(pcValue);
						}
					}
					else if(0 == _stricmp(pcName,"CHANGE_ACTION_TO"))
					{
						if(IT_CHANGE_ACTION & m_ValidItems)
						{
							kChangeActionTo = std::string(pcValue);
						}
					}
					else if(0 == _stricmp(pcName,"START_ACTION_SHOW_OPTION"))
					{
						m_eStartActionOption = static_cast<EStartActionOptionType>(atoi(pcValue));
					}
					else if(0 == _stricmp(pcName,"PARAM_INDEX"))
					{
						m_kParamIndex = static_cast<int>(atoi(pcValue));
					}
					else if(0 == _stricmp(pcName,"PARAM_VALUE"))
					{
						m_kParamID = std::string(pcValue);
					}	 
					else if(0 == _stricmp(pcName,"SET_EFFECT_ID"))
					{
						m_kEffectSave = static_cast<int>(atoi(pcValue));
					}
					else if(0 == _stricmp(pcName,"ORIGIN_TEXTURE"))	//항상 니가 먼저 쓰여져야 한다
					{
						m_kOriginTextureList.push_back(std::string(pcValue));
					}
					else if(0 == _stricmp(pcName,"CHANGED_TEXTURE"))
					{
						if(m_kOriginTextureList.size() <= m_kChangedTextureList.size())
						{
							PgXmlError1(pkElement, "XmlParse: CHANGED_TEXTURE count Not Same ORIGIN_TEXTURE count! Path : %s", pcValue);
							break;
						}

						m_kChangedTextureList.push_back(std::string(pcValue));
					}
					else if(0 == _stricmp(pcName,"NOT_DETACH_ON_DIE"))
					{
						m_bNotDetachOnDie = (0 == stricmp(pcValue,"TRUE"));
					}
					else if(0 == _stricmp(pcName,"FILTER_COLOR"))
					{
						::sscanf_s(pcValue, "%x", &m_kFocusFilter.dwColor);
					}
					else if(0 == _stricmp(pcName,"ALPHA_START"))
					{
						m_kFocusFilter.fAlphaStart = static_cast<float>(atof(pcValue));
					}
					else if(0 == _stricmp(pcName,"ALPHA_END"))
					{
						m_kFocusFilter.fAlphaEnd = static_cast<float>(atof(pcValue));
					}
					else if(0 == _stricmp(pcName,"START_TIME"))
					{
						m_kFocusFilter.fStartTime = static_cast<float>(atof(pcValue));
					}
					else if(0 == _stricmp(pcName,"CLEAR_TIME"))
					{
						m_kFocusFilter.fClearTime = static_cast<float>(atof(pcValue));
					}
					else if(0 == _stricmp(pcName,"EQUIP_POS"))
					{
						::sscanf_s(pcValue, "%d", &iEquipPos);
					}
					else if(0 == _stricmp(pcName,"EQUIP_ITEM"))
					{
						::sscanf_s(pcValue, "%d", &iEquipItemNo);
					}
					else if(0 == _stricmp(pcName,"OTHER_EQUIP_ITEM_RETURN_VALUE"))
					{
						::sscanf_s(pcValue, "%d", &m_iOtherEquipItemReturnValue);
					}
					else if(0 == _stricmp(pcName,"SCRIPT"))
					{
						m_kScriptName = std::string(pcValue);
					}
					else if(0 == _stricmp(pcName,"WHITE_OUT_TEXTURE"))
					{
						if(!(m_ValidItems&IT_WHITE_OUT))	//먼저 옵션부터
						{
							PgXmlError1(pkElement, "XmlParse: WHITE_OUT_TEXTURE Must Have IT_WHITE_OUT Option. Path : %s", pcValue);
							break;
						}
						else
						{
							m_kContWhiteOutTexture.push_back(std::string(pcValue));
						}
					}
					else if(0 == _stricmp(pcName,"GIF_TITLE_EMOTICON_ID"))
					{
						m_iGIFTitleEmoticonID = static_cast<int>(atoi(pcValue));
					}
					else if(0 == _stricmp(pcName,"ACTIVATE_EVENT_SCRIPT_NO"))
					{
						m_iActivateEventScript = static_cast<int>(atoi(pcValue));
					}
					else if(0 == _stricmp(pcName,"DEACTIVATE_EVENT_SCRIPT_NO"))
					{
						m_iDeactivateEventScript = static_cast<int>(atoi(pcValue));
					}
					else if(0 == _stricmp(pcName,"HIDE_NODE_NAME"))
					{
						m_kHideNodeName = pcValue;
					}
					else if(0 == _stricmp(pcName,"DO_DMG_ACTION_ON_FREEZED"))
					{
						m_bDoDmgActionOnFreezed = static_cast<bool>(atoi(pcValue));
					}
					else if(0 == _stricmp(pcName,"SCALE_FACTOR_BY_AT_ATTACK_RANGE"))
					{
						SetAttackScaleByAbil(static_cast<float>(atof(pcValue)));
					}					

					pkAttr = pkAttr->Next();
				}

				if(-1 != iInputSlotID)
				{
					m_vInputSlotInfo.push_back(stInputSlotInfo(iInputSlotID,bInputSlotEnable));
				}

				if(0 <= iEquipPos && 0 <= iEquipItemNo)
				{
					m_kEquipItemCont.insert(std::make_pair(iEquipPos,iEquipItemNo));	
				}

				if(!kChangeActionFrom.empty() && !kChangeActionTo.empty())
				{
					m_kContChangeAction.insert(std::make_pair(kChangeActionFrom,kChangeActionTo));
				}
			}
			else if( 0 == _stricmp(pcTagName, "ABIL") )
			{
				TiXmlAttribute* pkAttr = pkElement->FirstAttribute();
				WORD wType = 0;
				int iValue = 0;
				while(pkAttr)
				{
					char const* pcName = pkAttr->Name();
					char const* pcValue = pkAttr->Value();

					if( 0 == _stricmp(pcName,"TYPE") )
					{
						wType = static_cast<WORD>(::atoi(pcValue));
					}
					else if ( 0 == _stricmp(pcName,"VALUE") )
					{
						iValue = ::atoi(pcValue);
					}

					if ( wType && iValue )
					{
						break;
					}

					pkAttr = pkAttr->Next();
				}

				if ( wType && iValue )
				{
					m_kContAddAbil.insert( std::make_pair( wType, iValue ) );
				}
			}
			else if( 0 == _stricmp(pcTagName, "PARAM") )
			{
				TiXmlAttribute* pkAttr = pkElement->FirstAttribute();
				std::string kKey;
				std::string kValue;
				while(pkAttr)
				{
					char const* pcName = pkAttr->Name();
					char const* pcValue = pkAttr->Value();

					if( 0 == _stricmp(pcName,"KEY") )
					{
						kKey = pcValue;
					}
					else if ( 0 == _stricmp(pcName,"VALUE") )
					{
						kValue = pcValue;
					}

					pkAttr = pkAttr->Next();
				}

				if ( false==kKey.empty() && false==kValue.empty() )
				{
					m_kContParam.insert( std::make_pair( kKey, kValue ) );
				}
			}
			else
			{
				PgXmlError1(pkElement, "XmlParse: Incoreect Tag '%s'", pcTagName);
				break;
			}
		}
		break;

	default:
		break;
	}


	if(-1 == m_fAlphaStartTransitionTime)
	{
		m_fAlphaStartTransitionTime = 0.2f;
	}
	if(-1 == m_fAlphaEndTransitionTime)
	{
		m_fAlphaEndTransitionTime = 0.2f;
	}


	// 같은 층의 다음 노드를 재귀적으로 파싱한다.
	TiXmlNode const* pkNextNode = pkNode->NextSibling();
	if(pkNextNode)
	{
		if(!ParseXml(pkNextNode))
		{
			return false;
		}
	}

	return true;
}

char const*	PgStatusEffect::GetRedirectionXMLID(int const iEffectValue) const
{
	RedirectionMap::const_iterator itor = m_RedirectionMap.find(iEffectValue);
	if(m_RedirectionMap.end() == itor)
	{
		return NULL;
	}

	return	itor->second.c_str();
}


void PgStatusEffect::Create()
{
	m_ValidItems = IT_NONE;

	m_fColorTransitSpeed = m_fColorTransitTime = 1.0f;
	m_iHideEnable = E_NONE_HELMET_CHANGE_TYPE;
	m_fBodySize = 1.0f;
	m_fBodyChangeTime = 1000.0f;
	m_bNotDetachOnDie = false;
	m_iTransformNameNo = 0;
	m_iOtherEquipItemReturnValue = 0;
	m_eStartActionOption = SAOT_NONE;
	m_iLoopCount = 1;
	m_fAlphaStartTransitionTime = -1;
	m_fAlphaEndTransitionTime = -1;
	m_fAlphaEnd = -1;
	m_iGIFTitleEmoticonID = 0;
	m_iActivateEventScript = 0;
	m_iDeactivateEventScript = 0;
	m_kColorAlpha = NiColorA(1.0f,1.0f,1.0f,1.0f);
	m_bDoDmgActionOnFreezed = false;
	m_bOnlyCaster = false;
	m_AttackScaleByAbil = 0.0f;
}
void PgStatusEffect::Destroy()
{
	for(ParticleCont::iterator itor = m_kParticleContainer.begin(); itor != m_kParticleContainer.end(); ++itor)
	{
		SAFE_DELETE(*itor);
	}
	m_kParticleContainer.clear();

	for(ParticleCont::iterator itor = m_kDetachParticleContainer.begin(); itor != m_kDetachParticleContainer.end(); ++itor)
	{
		SAFE_DELETE(*itor);
	}
	m_kDetachParticleContainer.clear();

	for(SkillTextCont::iterator itor = m_kSkillTextContainer.begin(); itor != m_kSkillTextContainer.end(); ++itor)
	{
		SAFE_DELETE(*itor);
	}
	m_kSkillTextContainer.clear();
	m_iHideEnable = E_NONE_HELMET_CHANGE_TYPE;
}


void PgStatusEffect::StartWorldFocusFilter(bool const bMyPlayer) const
{
	if(0 < m_kFocusFilter.fStartTime)
	{// 시작값이 반드시 존재 하고
		if(g_pkWorld && bMyPlayer)
		{// 내 플레이어가 상태 이상에 걸릴때
			g_pkWorld->SetShowWorldFocusFilter(m_kFocusFilter.dwColor, m_kFocusFilter.fAlphaStart, 
				m_kFocusFilter.fAlphaEnd, m_kFocusFilter.fStartTime, false, false);
		}
	}
}

void PgStatusEffect::ClearWorldFocusFilter(bool const bMyPlayer) const
{
	if(0 < m_kFocusFilter.fStartTime)
	{// 시작값이 반드시 존재 하고
		if(g_pkWorld && bMyPlayer)
		{// 내 플레이어가 상태 이상에서 해제 될때
			float fStartAlpha = g_pkWorld->GetWorldFocusFilterAlpha();
			g_pkWorld->SetShowWorldFocusFilter(m_kFocusFilter.dwColor, fStartAlpha, 
				0, m_kFocusFilter.fClearTime, true, false);
		}
	}
}

PgStatusEffect::EQUIP_ITEM_CONT const& PgStatusEffect::GetEquipItemList() const
{
	return m_kEquipItemCont;
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	class	PgStatusEffectMan
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PgStatusEffectMan::Create()
{
}

void PgStatusEffectMan::Destroy()
{
	for(ContStatusEffect::iterator itor = m_ContStatusEffect.begin(); itor != m_ContStatusEffect.end(); ++itor)
	{
		SAFE_DELETE(itor->second);
	}

	m_ContStatusEffect.clear();
}

void PgStatusEffectMan::ReAddEveryEffect(PgPilot* pkTargetPilot)
{// 모든 이펙트를 다시 걸어줄때(맵이동 등)
	PgUnitEffectMgr& rkEffectMgr = pkTargetPilot->GetUnit()->GetEffectMgr();
	ContEffectItor kItor;

	rkEffectMgr.GetFirstEffect(kItor);
	CEffect* pkEffect = NULL;

	NILOG(PGLOG_LOG,"PgStatusEffectMan::ReAddEveryEffect() Pilot:%s EffectCount : %d\n", MB(pkTargetPilot->GetGuid().str()), rkEffectMgr.Size());

	DWORD const dwServerElapsedTime32 = g_kEventView.GetServerElapsedTime();
	typedef std::vector<CEffect*> CONT_PTR_EFFECT;
	CONT_PTR_EFFECT kContEff;

	while ((pkEffect = rkEffectMgr.GetNextEffect(kItor)) != NULL)
	{
		if(0 < pkEffect->GetAbil(4204))
		{// 무적인 이펙트를 따로 골라내서
			kContEff.push_back(pkEffect);
		}
		else
		{
			AddStatusEffect(pkTargetPilot, NULL, 0, pkEffect->GetEffectNo(), pkEffect->GetValue(), true, false, pkEffect->GetDurationTime() - (pkEffect->GetEndTime() - dwServerElapsedTime32), pkEffect->GetStartTime(), pkEffect->GetEndTime()/*pkEffect->GetTime()*/, pkEffect->GetKey());
//			AddStatusEffectAfterAction(pkTargetPilot, pkEffect->GetEffectNo());
		}
	}
	std::vector<CEffect*>::iterator itor = kContEff.begin();
	while(itor !=  kContEff.end())
	{// 제일 마지막에 무적 이펙트를 추가해준다
		// 쌍카 변신 무적 이펙트가 걸린 후,다른 변신 스킬이 적용되면,
		// 무적을 유지한 상태로 쌍카 변신등이 풀리게 되기 때문에 무적 이펙트는 가장 마지막에 추가해줌
		pkEffect = *itor;
		AddStatusEffect(pkTargetPilot, NULL, 0, pkEffect->GetEffectNo(), pkEffect->GetValue(), true, false, pkEffect->GetDurationTime() - (pkEffect->GetEndTime() - dwServerElapsedTime32), pkEffect->GetStartTime(), pkEffect->GetEndTime()/*pkEffect->GetTime()*/, pkEffect->GetKey());
//		AddStatusEffectAfterAction(pkTargetPilot, pkEffect->GetEffectNo());
		++itor;
	}
}

/*
// 이펙트가 걸린 후 특정 행동을 하는 경우 (예외 처리)
void PgStatusEffectMan::AddStatusEffectAfterAction(PgPilot* pkTargetPilot, int const iEffectID)
{
	if( !pkTargetPilot )
	{
		return;
	}

	switch( iEffectID )
	{
	case SAFE_FOAM_EFFECTNO:
		{
			if( g_kPilotMan.IsMyPlayer(pkTargetPilot->GetGuid()) )
			{
				DWORD const iItemNo = 98000010;

				GET_DEF(CItemDefMgr, kItemDefMgr);
				if( iEffectID == kItemDefMgr.GetAbil(iItemNo, AT_EFFECTNUM1) )
				{
					int const iTTW = 450052;
					std::wstring const kMessage = TTW(iTTW);
					BM::Stream kPacket( PT_C_M_REQ_MSGBOX_CALL, false );
					kPacket.Push(iItemNo);

					char const* pszText = "SFRM_MSG_COMMON_CANCEL";
					std::wstring const wstr = UNI(pszText);
					XUI::CXUI_Wnd* pWnd = NULL;
					if(!XUIMgr.IsActivate(wstr, pWnd))
					{
						lua_tinker::call<void, char const*, lwPacket, bool >("CommonMsgBoxCancel", MB(kMessage), lwPacket(&kPacket), false);
					}					
					g_kPilotMan.LockPlayerInput(SReqPlayerLock(EPLT_StatusEffect1, false));
				}
			}
		}break;
	default:
		{		
		}break;
	}
}
*/

void PgStatusEffectMan::RemoveStatusEffectAfterAction(PgPilot* pkTargetPilot, int const iEffectID)
{
	if( !pkTargetPilot )
	{
		return;
	}

	switch( iEffectID )
	{
// 	case SAFE_FOAM_EFFECTNO:
// 		{
// 			BM::GUID kPlayerGuid;
// 			if( g_kPilotMan.IsMyPlayer(pkTargetPilot->GetGuid()) )
// 			{
// 				int const iTTW = 450053;
// 				char* pszText = "SFRM_MSG_COMMON_CANCEL";
// 
// 				std::wstring const wstr = UNI(pszText);
// 				XUI::CXUI_Wnd* pWnd = NULL;
// 				if(XUIMgr.IsActivate(wstr, pWnd))
// 				{
// 					lwCloseUI(pszText);
// 				}
// 
// 				g_kChatMgrClient.LogMsgBox(iTTW);
// 
// 				PgPlayer* pkPlayer = dynamic_cast<PgPlayer*>(pkTargetPilot->GetUnit());
// 				if( pkPlayer )
// 				{
// 					lua_tinker::call<void,lwGUID,BYTE>("Net_PT_C_M_REQ_FOLLOWING",lwGUID(pkPlayer->FollowingHeadGuid()),((BYTE)EFollow_Cancel));
// 				}
// 				g_kSafeFoamMgr.Clear();
// 				g_kPilotMan.UnlockPlayerInput(SReqPlayerLock(EPLT_StatusEffect1, false));
// 			}
// 		}break;
	case 302401:
	case 302501:
	case 302601:
	case 302701:
		{
			PgActor* pkActor = dynamic_cast<PgActor*>(pkTargetPilot->GetWorldObject());
			if( pkActor )
			{
				lua_tinker::call<void,lwActor>("Act_Rest_OnClearUpRun",lwActor(pkActor));
				pkActor->ReserveTransitAction(ACTIONNAME_IDLE);
			}
		}break;

	default:
		break;
	}
	
	PgActor* pkTargetActor = dynamic_cast<PgActor*>(pkTargetPilot->GetWorldObject());
	if( NULL != pkTargetActor
		&& true == pkTargetActor->IsMyActor() )
	{
		GET_DEF(CEffectDefMgr, kEffectDefMgr);
		CEffectDef const* pkDef = kEffectDefMgr.GetDef(iEffectID);
		if(pkDef)
		{
			if( SAFE_FOAM_EFFECTNO == pkDef->GetAbil(AT_PARENT_ITEM_NO) )
			{
				lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 450053, true);
			}
		}
	}
}

void PgStatusEffectMan::AddStatusEffect(PgPilot* pkTargetPilot, PgPilot* pkCasterPilot, int const iActionInstanceID, int const iEffectID, int const iValue, bool const bAddEvenExist,
										bool const bNowAdded, DWORD const dwElpasedTime, unsigned long const ulStartTime, unsigned long const ulEndTime, int iEffectKey, BM::DBTIMESTAMP_EX const * const pkExpireTime )
{
	GET_DEF(CEffectDefMgr, kEffectDefMgr);
	CEffectDef const* pkDef = kEffectDefMgr.GetDef(iEffectID);
	if(!pkDef)
	{
		NILOG(PGLOG_LOG,"[Warnning] [PgStatusEffectMan::AddStatusEffect] EffectDef Not Found. EffectID : %d\n",iEffectID);
		return;
	}

	if(EFFECT_TYPE_PASSIVE == pkDef->GetType())	//	패시브형 이펙트 일 경우 무시한다.
	{
		return;
	}

	if(!pkTargetPilot)
	{
		return;
	}
	if(!pkTargetPilot->GetUnit())
	{
		return;
	}

	PgActor* pkTargetActor = dynamic_cast<PgActor*>(pkTargetPilot->GetWorldObject());
	if(!pkTargetActor)
	{
		return;
	}

	{
		CUnit *pkTargetUnit = pkTargetPilot->GetUnit();
		if(NULL != pkTargetUnit && true == pkTargetUnit->IsUnitType(UT_PLAYER) && true == pkTargetActor->IsRidingPet())
		{ //대상이 플레이어고 펫에 탑승 중인가?
			std::wstring wstrActionName = pkDef->GetActionName();
			if( pkDef->GetType() == EFFECT_TYPE_CURSED ||
				wstrActionName == _T("se_transform_to_metamorphosis") ||
				wstrActionName == _T("se_transform_to_mahalka_Fig") ||
				wstrActionName == _T("se_transform_to_mahalka_Mag") ||
				wstrActionName == _T("se_transform_to_mahalka_Arc") ||
				wstrActionName == _T("se_transform_to_mahalka_Thi")

				)
			{
				PgActor* pkPetActor = pkTargetActor->GetMountTargetPet();
				if(pkPetActor)
				{
					pkPetActor->TransitAction("a_mount");
					//pkPetActor->ReserveTransitAction("a_mount"); //강제하차
				}
			}
		}
	}
	/*
	if(pkTargetPilot->GetUnit()->IsUnitType(UT_PLAYER) && pkDef->GetType() & EFFECT_TYPE_CURSED && pkTargetActor->IsRidingPet())
	{ //액터가 펫에 탑승했고 상태이상(디버프)에 걸렸다면
		PgActor* pkPetActor = pkTargetActor->GetMountTargetPet();
		if(pkPetActor)
		{
			pkPetActor->ReserveTransitAction("a_mount"); //강제하차
		}
	}
	*/


	NILOG(PGLOG_LOG,"PgStatusEffectMan::AddStatusEffect() Pilot:%s %s ActionInstanceID : %d EffectID : %d EffectValue : %d bAddEvenExist:%d bNowAdded:%d\n", MB(pkTargetPilot->GetGuid().str()), MB(pkTargetPilot->GetName()),iActionInstanceID,iEffectID,iValue,bAddEvenExist,bNowAdded);

	PgUnitEffectMgr& rkEffectMgr = pkTargetPilot->GetUnit()->GetEffectMgr();

	CEffect* pkEffect = rkEffectMgr.FindInGroup(iEffectID, true);
	if(NULL == pkEffect)
	{// 아이템으로 인한 이펙트인지도 체크해야 한다. 
		pkEffect = rkEffectMgr.FindInGroupItemEffectByEffectNo(iEffectID, true);
	}

	if(NULL == pkEffect)
	{
		SEffectCreateInfo kCreate;
		kCreate.eType = EFFECT_TYPE_NORMAL;
		kCreate.iEffectNum = iEffectID;
		kCreate.iValue = iValue;
		kCreate.dwElapsedTime = dwElpasedTime;
		kCreate.eOption = SEffectCreateInfo::ECreateOption_CallbyClient;

		if ( pkExpireTime )
		{
			kCreate.kWorldExpireTime = *pkExpireTime;
		}

		pkEffect = pkTargetPilot->GetUnit()->AddEffect(kCreate);	
		if(NULL == pkEffect)
		{
			return;	//Add한 이후에 또 NULL일 수 있다
		}
	}
	else
	{
		pkEffect->SetValue(iValue);

		if ( pkExpireTime )
		{
			pkEffect->ExpireTime( *pkExpireTime );
		}

		if(false == bAddEvenExist)
		{
			return;
		}
	}
	pkEffect->SetStartTime(0==ulStartTime ? g_kEventView.GetServerElapsedTime() : ulStartTime);
	pkEffect->SetVisualState(EVS_VISIBLE);
	if(pkCasterPilot)
	{
		pkEffect->SetCaster(pkCasterPilot->GetGuid());
	}

	if ( pkExpireTime && !pkExpireTime->IsNull() )
	{
		BM::DBTIMESTAMP_EX kLocalTime;
		g_kEventView.GetLocalTime(kLocalTime);

		__int64 i64RemainTimeSec = CGameTime::GetElapsedTime( *pkExpireTime, kLocalTime, CGameTime::MILLISECOND );
		pkEffect->SetEndTime( g_kEventView.GetServerElapsedTime() + static_cast<unsigned long>(i64RemainTimeSec) );
	}
	else
	{
		pkEffect->SetEndTime(0==ulEndTime ? pkEffect->GetStartTime()+pkEffect->GetDurationTime()-pkEffect->GetTime() : ulEndTime);
	}

	bool bAddHeadBuffIcon = (0!=pkDef->GetAbil(AT_DISPLAY_HEADBUFFICON));
	if(g_kPilotMan.GetPlayerPilot() == pkTargetPilot)
	{
		lwAddBuff(iEffectKey, iEffectID, iValue, bNowAdded, dwElpasedTime, pkEffect->GetStartTime(), pkEffect->GetEndTime());
	}
	else
	{
		if(EFFECT_TYPE_CURSED == pkDef->GetType())	//	디버프일 경우 머리위에 띄운다.
		{
			bAddHeadBuffIcon = true;
		}
	}

	if(pkDef->GetAbil(AT_DISPLAY_EFFECT_COUNTDOWN))
	{
		pkTargetActor->SetEffectCountDown(iEffectID, pkDef->GetDurationTime());
	}

	if( bAddHeadBuffIcon )
	{
		pkTargetActor->AddHeadBuffIcon(iEffectID);
	}

	CEffectDef const* pkEffectDef = pkEffect->GetEffectDef();
	std::string	kActionName;
	if(pkEffectDef)
	{
		kActionName = std::string(MB(pkEffectDef->GetActionName()));
	}

	{//맵 이동시에는 토글 정보는 초기화 되므로, 맵 이동시에 서버에서 이펙트를 보내 줄때 토글인지 체크하여, 토글 상태로 만들어 주어야 한다.
		//이펙트 번호와 같은 스킬 번호를 얻어 온다.
		ToggleStateChange(iEffectID, pkTargetPilot, true);
	}
	if(g_pkWorld)
	{
		g_pkWorld->AddEffectRemainTime(pkTargetPilot->GetGuid(), iEffectID, pkEffect->GetEndTime(), pkEffectDef);
	}

	if(-1 != AddStatusEffectToActor(pkTargetPilot, pkCasterPilot, kActionName,iEffectID,iEffectKey,iValue,bNowAdded,false))
	{// 투구 변형 스킬이 전체 변신 스킬을 없애버리는 버그 수정 부분
		PgStatusEffectInstance* pkNowAddedEffectInst = NULL;
		PgStatusEffect const* pkNowAddedStatusEffect = NULL;

		PgStatusEffectInstance* pkTranformAllEffInst = NULL;
		PgStatusEffect const* pkTranformAllStatusEff = NULL;
		
		StatusEffectInstanceList& kEffectList = pkTargetActor->GetStatusEffectInstanceList();
		StatusEffectInstanceList::iterator itor = kEffectList.begin();

		while(itor != kEffectList.end())
		{// 액터에 적용된 이펙트 리스트 중
			PgStatusEffectInstance* pkInstance = (*itor);
			if(pkInstance->GetEffectID() == iEffectID)
			{// 방금전에 등록된 녀석을 얻어두고
				pkNowAddedEffectInst = pkInstance;
				pkNowAddedStatusEffect = pkInstance->GetStatusEffect();
			}
			PgStatusEffect const* pkStatusEffect = pkInstance->GetStatusEffect();
			if(pkStatusEffect->GetValidItems() & PgStatusEffect::IT_TRANSFORM)
			{// 전체 변신 스킬이 적용 되고 있으면 얻어 두어
				pkTranformAllEffInst = pkInstance;
				pkTranformAllStatusEff = pkInstance->GetStatusEffect();
			}
			++itor;
		}

		if(pkTranformAllEffInst
			&& pkTranformAllStatusEff
			&& pkNowAddedEffectInst
			&& pkNowAddedStatusEffect
			&& pkNowAddedStatusEffect->GetHideEnable()
			)
		{// 최근에 적용된 전체 변신 이펙트를 다시 보이게 하고
			pkTranformAllEffInst->AttachEffect(pkTargetPilot,false);
			pkTranformAllEffInst->SetVisualState(EVS_VISIBLE);
			pkTranformAllEffInst->SetTransformation(pkTargetActor);
			pkTranformAllEffInst->AttachAdjustedItem(pkTargetActor);
			
			// 현재 투구 변형 이펙트를 감춘다
			pkNowAddedEffectInst->DetachEffect(pkTargetPilot);
			pkNowAddedEffectInst->SetVisualState(EVS_HIDE);
			pkNowAddedEffectInst->RestoreTransformation(pkTargetActor);
			pkNowAddedEffectInst->DetachAdjustedItem(pkTargetActor);
			pkNowAddedEffectInst->StopSound();
		}
	}
	
	{// 안전거품이 걸리는것이라면, 
		if( SAFE_FOAM_EFFECTNO == pkDef->GetAbil(AT_PARENT_ITEM_NO) )
		{
			PgActor* pkSubActor = PgActorUtil::GetSubPlayerActor(pkTargetActor);
			if(pkSubActor)
			{// 보조캐릭터를 감춰 주고
				pkSubActor->SetHide(true);
				pkSubActor->SetHideShadow(true);
			}
		}
		if( 0 < pkDef->GetAbil(AT_MOUNTED_PET_SKILL) )
		{
			lwActor kTargetActor( pkTargetActor );
			lwActor kCallerActor = kTargetActor.GetCallerActor();
			if( false == kCallerActor.IsNil() )
			{
				lwActor kSubActor = kCallerActor.GetSubPlayer();
				if(false == kSubActor.IsNil())
				{// 보조캐릭터를 감춰 주고
					kSubActor.SetHide(true);
					kSubActor.SetHideShadow(true);
				}
			}
		}
	}
}

void PgStatusEffectMan::RemoveStatusEffect(PgPilot* pkTargetPilot, int const iEffectID)
{
	if(!pkTargetPilot)
	{
		return;
	}
	if(!pkTargetPilot->GetUnit())
	{
		return;
	}

	PgActor* pkTargetActor = dynamic_cast<PgActor*>(pkTargetPilot->GetWorldObject());
	if(!pkTargetActor)
	{
		return;
	}

	NILOG(PGLOG_LOG,"PgStatusEffectMan::RemoveStatusEffect() Pilot:%s %s EffectID : %d\n", MB(pkTargetPilot->GetGuid().str()), MB(pkTargetPilot->GetName()),iEffectID);

	if(g_kPilotMan.GetPlayerPilot() == pkTargetPilot)
	{
		lwDeleteBuff(iEffectID);
	}
	
	pkTargetActor->RemoveHeadBuffIcon(iEffectID);

	pkTargetPilot->GetUnit()->DeleteEffect(iEffectID);

	RemoveStatusEffectFromActor(pkTargetPilot,iEffectID);

	ToggleStateChange(iEffectID, pkTargetPilot, false);

	GET_DEF(CEffectDefMgr, kEffectDefMgr);
	CEffectDef const* pkDef = kEffectDefMgr.GetDef(iEffectID);
	if(pkDef)
	{
		if( SAFE_FOAM_EFFECTNO == pkDef->GetAbil(AT_PARENT_ITEM_NO) )
		{// 안전거품이 제거 되는것이라면
			PgActor* pkSubActor = PgActorUtil::GetSubPlayerActor(pkTargetActor);
			if(pkSubActor)
			{// 보조캐릭터를 보여주고 
				pkSubActor->SetHide(false);
				pkSubActor->SetHideShadow(false);
			}
		}
		if( 0 < pkDef->GetAbil(AT_MOUNTED_PET_SKILL) )
		{
			lwActor kTargetActor( pkTargetActor );
			lwActor kCallerActor = kTargetActor.GetCallerActor();
			if( false == kCallerActor.IsNil() )
			{
				lwActor kSubActor = kCallerActor.GetSubPlayer();
				if(false == kSubActor.IsNil())
				{// 보조캐릭터를 보여주고
					kSubActor.SetHide(false);
					kSubActor.SetHideShadow(false);
				}
			}
		}
	}
	pkTargetActor->DelEffectCountDown(iEffectID);
}

int	PgStatusEffectMan::AddStatusEffectToActor(PgPilot* pkTargetPilot, PgPilot* pkCaster, std::string kEffectXMLID, int const iEffectID, int const iEffectKey, int const iEffectValue, bool const bNowAdded, bool const bMadeByItem)
{
	if(!pkTargetPilot)
	{
		return -1;
	}
	PgActor* pkTargetActor = dynamic_cast<PgActor*>(pkTargetPilot->GetWorldObject());
	if(!pkTargetActor)
	{
		return -1; 
	}

	PgStatusEffect const* pkStatusEffect = GetStatusEffect(kEffectXMLID,iEffectValue);
	if(pkStatusEffect)
	{
		if( pkStatusEffect->IsOnlyAttachCaster() 
			&& pkCaster
			)
		{
			if( pkTargetPilot->GetGuid() != pkCaster->GetGuid() )
			{
				return -1;
			}
		}

		if(0 < iEffectID)
		{//	같은 ID 의 이펙트가 이미 있을 경우 그냥 리턴하자.			
			PgStatusEffectInstance* pkInstance = NULL;
			StatusEffectInstanceList& kEffectList = pkTargetActor->GetStatusEffectInstanceList();
			for(StatusEffectInstanceList::iterator itor = kEffectList.begin(); itor != kEffectList.end(); ++itor)
			{
				pkInstance = (*itor);
				if(pkInstance->GetEffectID() == iEffectID && bMadeByItem == pkInstance->GetMadeByItem())
				{
					return -1;
				}
			}
		}
		PgStatusEffectInstance*	pkNewInstance = new PgStatusEffectInstance(pkStatusEffect,bMadeByItem);
		pkNewInstance->SetStartTime(BM::GetTime32());
		pkNewInstance->SetEffectID(iEffectID);
		pkNewInstance->SetEffectValue(iEffectValue);
		pkNewInstance->SetEffectKey(iEffectKey);
		pkNewInstance->AttachEffectAbil(pkTargetPilot);

		if(TemporaryHideSameTypeEffect(pkTargetPilot, *pkNewInstance))
		{//	같은 타입의 이펙트가 기존에 있을 경우 그 이펙트를 임시로 HIDE 시킨다.
			pkTargetActor->AddStatusEffectInstance(pkNewInstance);
			pkNewInstance->StartEffect(pkTargetPilot);
			pkNewInstance->AttachEffect(pkTargetPilot,bNowAdded);	// 추가되는 파티클의 EVS_VISIBLE은 여기서 세팅
			UpdateEffect(pkTargetPilot);
			pkNewInstance->PlaySound(pkTargetActor);
			pkNewInstance->AttachAdjustedItem(pkTargetActor);

			if(bNowAdded)
			{
				pkNewInstance->ShowSkillTexts(pkTargetPilot);
			}
		}
		else
		{//	지금 추가하려는 Instance가 ItemEffect인데, 같은 타입의 NoneItemEffect 가 기존에 존재한다.
		 //	따라서, 현재 추가하려는 것을 Hide시킨다.
			pkNewInstance->SetVisualState(EVS_HIDE);
			pkTargetActor->AddStatusEffectInstance(pkNewInstance);
			pkNewInstance->StartEffect(pkTargetPilot);
		}
		bool const IsMyPlayer = g_kPilotMan.IsMyPlayer(pkTargetPilot->GetGuid());
		pkStatusEffect->StartWorldFocusFilter(IsMyPlayer);

		return	pkNewInstance->GetInstanceID();
	}
	return	-1;
}
void PgStatusEffectMan::RemoveAllStatusEffect(PgPilot* pkTargetPilot, bool const bIncludeItemStatusEffect)
{
	if(!pkTargetPilot)
	{
		return;
	}

	PgActor* pkTargetActor = dynamic_cast<PgActor*>(pkTargetPilot->GetWorldObject());
	if(!pkTargetActor)
	{
		return;
	}

	bool const IsMyPlayer = g_kPilotMan.IsMyPlayer(pkTargetPilot->GetGuid());

	PgStatusEffectInstance* pkInstance = NULL;
	StatusEffectInstanceList& kEffectList = pkTargetActor->GetStatusEffectInstanceList();
	for(StatusEffectInstanceList::iterator itor = kEffectList.begin(); itor != kEffectList.end();)
	{
		pkInstance = (*itor);

		if(false == bIncludeItemStatusEffect && pkInstance->GetMadeByItem())
		{
			++itor;
			continue;
		}

		PgStatusEffect const* pkStatusEffect = pkInstance->GetStatusEffect();
		PG_ASSERT_LOG(pkStatusEffect);

		if(pkStatusEffect->GetNotDetachOnDie())
		{
			++itor;
			continue;
		}

		EffectVisualState kVisualState = pkInstance->GetVisualState();
		
		if(pkStatusEffect && (EVS_VISIBLE == kVisualState))
		{
			pkInstance->DetachEffect(pkTargetPilot);
			pkInstance->DetachAdjustedItem(pkTargetActor);
			pkInstance->StopSound();
		}
		pkInstance->DetachEffectAbil(pkTargetPilot);

		itor = pkTargetActor->RemoveStatusEffectInstance(*pkInstance);

		if(pkStatusEffect)
		{
			pkStatusEffect->ClearWorldFocusFilter(IsMyPlayer);
			if(EVS_VISIBLE == kVisualState)
			{
				ShowTemporaryHidedEffect(pkTargetPilot,pkStatusEffect->GetType());
			}

			UpdateEffect(pkTargetPilot);
		}
	}

	if(g_kHeadBuffIconListMgr.IsAlive())
	{
		PgHeadBuffIconList* pkList = pkTargetActor->GetHeadBuffIconList();
		if(pkList)
		{
			pkList->RemoveAllIcon();
		}
	}

	
}
void PgStatusEffectMan::RemoveStatusEffectFromActor(PgPilot* pkTargetPilot, int const iEffectID)
{
	PgActor* pkTargetActor = dynamic_cast<PgActor*>(pkTargetPilot->GetWorldObject());
	if(!pkTargetActor)
	{
		return;
	}

	PgStatusEffectInstance* pkInstance = NULL;
	StatusEffectInstanceList& kEffectList = pkTargetActor->GetStatusEffectInstanceList();
	for(StatusEffectInstanceList::iterator itor = kEffectList.begin(); itor != kEffectList.end(); ++itor)
	{
		pkInstance = (*itor);
		if(pkInstance->GetMadeByItem())
		{
			continue;
		}

		if(pkInstance->GetEffectID() != iEffectID)
		{
			continue;
		}

		PgStatusEffect const* pkStatusEffect = pkInstance->GetStatusEffect();
		PG_ASSERT_LOG(pkStatusEffect);

		EffectVisualState kVisualState = pkInstance->GetVisualState();
		
		if(pkStatusEffect && (EVS_VISIBLE == kVisualState))
		{
			pkInstance->DetachEffect(pkTargetPilot);
			pkInstance->DetachAdjustedItem(pkTargetActor);
			pkInstance->StopSound();
		}
		pkInstance->DetachEffectAbil(pkTargetPilot);

		pkTargetActor->RemoveStatusEffectInstance(*pkInstance);
 
		if(pkStatusEffect)
		{
			bool const IsMyPlayer = g_kPilotMan.IsMyPlayer(pkTargetPilot->GetGuid());
			pkStatusEffect->ClearWorldFocusFilter(IsMyPlayer);
			UpdateEffect(pkTargetPilot);
			if(EVS_VISIBLE == kVisualState)
			{
				ShowTemporaryHidedEffect(pkTargetPilot,pkStatusEffect->GetType());
			}
		}
	
		break;
	}

	{// 투구 감추기 옵션이 있을 경우
		bool bDisplayHelmet = 0;
		int const iOption = pkTargetPilot->GetAbil(AT_CLIENT_OPTION_SAVE) ;
		READ_DWORD_TO_BOOL(iOption, OPTION_TYPE_SHOW_HELMET, bDisplayHelmet);
		if(true == bDisplayHelmet)
		{// Actor의 투구 감추기 어빌이 존재하고,이펙트의 속성중 HIDE_ENABLE값이 존재하고
			PgActor* pkActor = g_kPilotMan.FindActor(pkTargetPilot->GetGuid());
			if(pkActor)
			{
				g_kStatusEffectMan.DisplayHeadTransformEffect(pkActor, false);
			}
		}
	}
}
void PgStatusEffectMan::RemoveStatusEffectFromActor2(PgPilot* pkTargetPilot, int const iInstanceID)
{
	if(0 > iInstanceID)
	{
		return;
	}

	PgActor* pkTargetActor = dynamic_cast<PgActor*>(pkTargetPilot->GetWorldObject());
	if(!pkTargetActor)
	{
		return;
	}

	PgStatusEffectInstance* pkInstance = NULL;
	StatusEffectInstanceList& kEffectList = pkTargetActor->GetStatusEffectInstanceList();
	for(StatusEffectInstanceList::iterator itor = kEffectList.begin(); itor != kEffectList.end(); ++itor)
	{

		pkInstance = (*itor);
		if(pkInstance->GetInstanceID() != iInstanceID)
		{
			continue;
		}

		PgStatusEffect const* pkStatusEffect = pkInstance->GetStatusEffect();
		PG_ASSERT_LOG(pkStatusEffect);

		EffectVisualState kVisualState = pkInstance->GetVisualState();
		
		if(pkStatusEffect && (EVS_VISIBLE == kVisualState))
		{
			pkInstance->DetachEffect(pkTargetPilot);
			pkInstance->DetachAdjustedItem(pkTargetActor);
			pkInstance->StopSound();
		}
		pkInstance->DetachEffectAbil(pkTargetPilot);

		pkTargetActor->RemoveStatusEffectInstance(*pkInstance);
		
		if(pkStatusEffect)
		{
			bool const IsMyPlayer = g_kPilotMan.IsMyPlayer(pkTargetPilot->GetGuid());
			pkStatusEffect->ClearWorldFocusFilter(IsMyPlayer);
			if(EVS_VISIBLE == kVisualState)
			{
				ShowTemporaryHidedEffect(pkTargetPilot,pkStatusEffect->GetType());
			}

			UpdateEffect(pkTargetPilot);
		}
	
		break;
	}

	{// 투구 감추기 옵션이 있을 경우
		bool bDisplayHelmet = 0;
		int const iOption = pkTargetPilot->GetAbil(AT_CLIENT_OPTION_SAVE) ;
		READ_DWORD_TO_BOOL(iOption, OPTION_TYPE_SHOW_HELMET, bDisplayHelmet);
		if(true == bDisplayHelmet)
		{// Actor의 투구 감추기 어빌이 존재하고,이펙트의 속성중 HIDE_ENABLE값이 존재하고
			PgActor* pkActor = g_kPilotMan.FindActor(pkTargetPilot->GetGuid());
			if(pkActor)
			{
				g_kStatusEffectMan.DisplayHeadTransformEffect(pkActor, false);
			}
		}
	}
}

//	return true : kSrcStatusEffectInstance 를 보여주면 된다.
//	return false : kSrcStatusEffectInstance 가 Hide되도록 처리해야한다.
bool PgStatusEffectMan::TemporaryHideSameTypeEffect(PgPilot* pkTargetPilot, PgStatusEffectInstance const& kSrcStatusEffectInstance)
{
	PgStatusEffect const* pkSrcStatusEffect = kSrcStatusEffectInstance.GetStatusEffect();
	if(!pkSrcStatusEffect)
	{
		return true;
	}

	if("" == pkSrcStatusEffect->GetType())
	{
		return true;
	}

	PgActor* pkTargetActor = dynamic_cast<PgActor*>(pkTargetPilot->GetWorldObject());
	if(!pkTargetActor)
	{
		return true;
	}

	StatusEffectInstanceList& kEffectList = pkTargetActor->GetStatusEffectInstanceList();

	if(kSrcStatusEffectInstance.GetMadeByItem())
	{
		for(StatusEffectInstanceList::iterator itor = kEffectList.begin(); itor != kEffectList.end(); ++itor)
		{
			PgStatusEffectInstance* pkInstance = (*itor);

			PgStatusEffect const* pkStatusEffect = pkInstance->GetStatusEffect();
			PG_ASSERT_LOG(pkStatusEffect);

			if(!pkStatusEffect)
			{
				continue;
			}

			if(pkStatusEffect->GetType() != pkSrcStatusEffect->GetType())
			{
				continue;
			}

			if(EVS_HIDE == pkInstance->GetVisualState())
			{
				continue;
			}

			if(false == pkInstance->GetMadeByItem())
			{
				return	false;
			}
		}
	}

	for(StatusEffectInstanceList::iterator itor = kEffectList.begin(); itor != kEffectList.end(); ++itor)
	{
		PgStatusEffectInstance* pkInstance = (*itor);

		PgStatusEffect const* pkStatusEffect = pkInstance->GetStatusEffect();		
		PG_ASSERT_LOG(pkStatusEffect);

		if(!pkStatusEffect)
		{
			continue;
		}

		if(pkStatusEffect->GetType() != pkSrcStatusEffect->GetType())
		{
			continue;
		}
		if(EVS_HIDE == pkInstance->GetVisualState())
		{
			continue;
		}

		pkInstance->DetachEffect(pkTargetPilot);
		pkInstance->SetVisualState(EVS_HIDE);
		pkInstance->RestoreTransformation(pkTargetActor);
		pkInstance->DetachAdjustedItem(pkTargetActor);
		pkInstance->StopSound();
	}
 
	return	true;

}
void PgStatusEffectMan::ShowTemporaryHidedEffect(PgPilot* pkTargetPilot,std::string kEffectType)
{// 임시적으로 감추었던 이펙트를 보이게 하는 방법은
	if("" == kEffectType)
	{
		return;
	}

	PgActor* pkTargetActor = dynamic_cast<PgActor*>(pkTargetPilot->GetWorldObject());
	if(!pkTargetActor)
	{
		return;
	}

	StatusEffectInstanceList& kEffectList = pkTargetActor->GetStatusEffectInstanceList();
	PgStatusEffectInstance* pkInstance = NULL;	
	PgStatusEffectInstance* pkLatestEffectInstance = NULL;

	for(StatusEffectInstanceList::iterator itor = kEffectList.begin(); itor != kEffectList.end(); ++itor)
	{// Actor의 상태이펙트들 가운데
		pkInstance = (*itor);

		PgStatusEffect const* pkStatusEffect = pkInstance->GetStatusEffect();
		if(!pkStatusEffect)
		{
			continue;
		}

		if(pkStatusEffect->GetType() != kEffectType)
		{
			continue;
		}
		if(EVS_VISIBLE == pkInstance->GetVisualState())
		{// 감추어졌고
			continue;
		}
		
		if(NULL == pkLatestEffectInstance 
			|| pkInstance->GetStartTime()>pkLatestEffectInstance->GetStartTime()
			)
		{// 가장 최근의 상태이펙트를 얻어와
			if(NULL == pkLatestEffectInstance 
				|| (pkLatestEffectInstance->GetMadeByItem() == pkInstance->GetMadeByItem()) 
				|| (false == pkInstance->GetMadeByItem())
				)
			{
				pkLatestEffectInstance = pkInstance;
			}
		}

	}


	if(pkLatestEffectInstance)
	{
		PgStatusEffect const* pkLatestStatusEffect = pkLatestEffectInstance->GetStatusEffect();
		if(pkLatestStatusEffect)
		{// 이펙트를 다시 붙인다(외관적인)
			pkLatestEffectInstance->AttachEffect(pkTargetPilot,false);
			pkLatestEffectInstance->SetVisualState(EVS_VISIBLE);
			pkLatestEffectInstance->SetTransformation(pkTargetActor);
			pkLatestEffectInstance->AttachAdjustedItem(pkTargetActor);
		}
	}
}

void PgStatusEffectMan::UpdateEffect(PgPilot* pkTargetPilot)
{
	if(!pkTargetPilot)
	{
		return;
	}

	CUnit* pkUnit = pkTargetPilot->GetUnit();
	if(!pkUnit)
	{
		return;
	}

	PgActor* pkTargetActor = dynamic_cast<PgActor*>(pkTargetPilot->GetWorldObject());
	if(!pkTargetActor)
	{
		return;
	}

	//	Color,Alpha,머리크기 처리하기
	NiColorA	kColor(1.0f,1.0f,1.0f,1.0f);
	NiColor		kSpecular(0.0f,0.0f,0.0f);
	float		fHeadSize = 1.0f;
	float		fBodySize = 1.0f;
	float		fColorTransitSpeed = 1.0f;
	float		fColorTransitTime = 0.01f;
	bool		bHide = false;
	bool		bHideShadow = false;
	bool		bFreezed = false;
	bool		bDoDmgActionOnFreezed=false;
	bool		bNotActionShift = false;
	bool		bOnlyMoveAction = false;
	bool		bOnyDefaultAttack = false;
	bool		bStun = false;
	int			iInvincible = pkUnit->GetAbil(AT_INVINCIBLE2);
	bool		bDarkSight = false;
	bool		bReverseInputDir = false;
	
	std::string kTransformID(""), kTransformNextAction("");
	int iTransformNameNo = 0;
	int iTransformEffectID = 0;
	StatusEffectInstanceList& kEffectList = pkTargetActor->GetStatusEffectInstanceList();
	PgStatusEffectInstance* pkInstance = NULL;	
	for(StatusEffectInstanceList::iterator itor= kEffectList.begin(); itor != kEffectList.end(); ++itor)
	{
		pkInstance = (*itor);
		if(EVS_HIDE == pkInstance->GetVisualState())
		{
			continue;
		}

		PgStatusEffect const* pkStatusEffect = pkInstance->GetStatusEffect();

		if(!pkStatusEffect)
		{
			continue;
		}

		if(pkStatusEffect->GetValidItems() & PgStatusEffect::IT_HEAD_SIZE)
		{	
			fHeadSize = pkStatusEffect->GetHeadSize();
		}
		if(pkStatusEffect->GetValidItems() & PgStatusEffect::IT_STUN)
		{
			bStun = true;
		}
		if(pkStatusEffect->GetValidItems() & PgStatusEffect::IT_HIDE)
		{
			bHide = true;
		}
		if(pkStatusEffect->GetValidItems() & PgStatusEffect::IT_HIDE_SHADOW)
		{
			bHideShadow = true;
		}
		if(pkStatusEffect->GetValidItems() & PgStatusEffect::IT_ONLY_MOVE_ACTION)
		{
			bOnlyMoveAction = true;
		}
		if(pkStatusEffect->GetValidItems() & PgStatusEffect::IT_ONLY_DEFAULT_ATTACK)
		{
			bOnyDefaultAttack = true;
		}
		if(pkStatusEffect->GetValidItems() & PgStatusEffect::IT_COLOR)
		{
			kColor.r = pkStatusEffect->GetColorA().r;
			kColor.g = pkStatusEffect->GetColorA().g;
			kColor.b = pkStatusEffect->GetColorA().b;
			kSpecular = pkStatusEffect->GetSpecular();

			fColorTransitSpeed = pkStatusEffect->GetColorTransitSpeed();
			fColorTransitTime = pkStatusEffect->GetColorTransitTime();
		}
		if(pkStatusEffect->GetValidItems() & PgStatusEffect::IT_TRANSFORM)
		{
			//변신시 남녀에 따라 다르게 변신해야 하는 경우
			if(!pkStatusEffect->GetTransformIDForMale().empty() && !pkStatusEffect->GetTransformIDForFemale().empty())
			{
				if(UG_MALE == pkTargetPilot->GetAbil(AT_GENDER))
				{
					kTransformID = pkStatusEffect->GetTransformIDForMale(); 
				}
				else
				{
					kTransformID = pkStatusEffect->GetTransformIDForFemale(); 
				}
			}
			else 
			{
				kTransformID = pkStatusEffect->GetTransformID(); 
			}

			iTransformEffectID = pkInstance->GetEffectID();
			iTransformNameNo = pkStatusEffect->GetTransformNameNo();
			kTransformNextAction = pkStatusEffect->GetTransformNextAction();
		}
		if(pkStatusEffect->GetValidItems() & PgStatusEffect::IT_FREEZED)
		{
			bFreezed = true;
		}
		if(pkStatusEffect->GetValidItems() & PgStatusEffect::IT_NOT_ACTION_SHIFT)
		{
			bNotActionShift = true;
		}
		if(pkStatusEffect->GetValidItems() & PgStatusEffect::IT_INVINCIBLE)
		{
			++iInvincible;
		}
		if(pkStatusEffect->GetValidItems() & PgStatusEffect::IT_DARK_SIGHT)
		{
			bDarkSight = true;
		}
		if(pkStatusEffect->GetValidItems() & PgStatusEffect::IT_INPUTDIR_REVERSE)	//키보드 입력 반대
		{
			bReverseInputDir = true;
		}
		//if(pkStatusEffect->GetValidItems() & PgStatusEffect::IT_WHITE_OUT)
		//{
		//}
		bDoDmgActionOnFreezed = pkStatusEffect->IsDoDmgActionOnFreezed();
	}

	pkTargetActor->SetTargetHeadSize(fHeadSize);
	pkTargetActor->SetFreezed(bFreezed, true, bDoDmgActionOnFreezed);
	pkTargetActor->SetNotActionShift(bNotActionShift);
	pkTargetActor->SetOnlyMoveAction(bOnlyMoveAction);
	pkTargetActor->SetOnlyDefaultAttack(bOnyDefaultAttack);
	pkTargetActor->SetHide(bHide);
	pkTargetActor->SetHideShadow(bHideShadow);
	pkTargetActor->SetStun(bStun);

	pkTargetActor->SetTargetColor(NiColor(kColor.r,kColor.g,kColor.b),fColorTransitTime);
	pkTargetActor->SetTargetSpecular(kSpecular,fColorTransitTime);

	if ( iInvincible > 0 )
	{
		pkUnit->SetAbil( AT_INVINCIBLE2, iInvincible );
	}

	if("" == kTransformID)
	{
		bool bTransformToIdle = true;
		PgAction* pkAction =  pkTargetActor->GetAction();
		if(pkAction)
		{
			if(pkAction->GetActionType() == "IDLE")
			{
				std::string const& kCurActionName = pkAction->GetID();
				pkTargetActor->RestoreTransformation(kCurActionName.c_str());
				bTransformToIdle = false;
			}
		}
		if(bTransformToIdle)
		{
			pkTargetActor->RestoreTransformation(ACTIONNAME_IDLE);
		}
		
	}
	else
	{
		if(kTransformID != pkTargetActor->GetTransformedActorID())
		{
			//다른 Transform이 설정되어 있으면 기존 내역을 삭제하고 새로 세팅 한다.
			if(!pkTargetActor->GetTransformedActorID().empty())
			{
				pkTargetActor->RestoreTransformation(ACTIONNAME_IDLE);
			}
			pkTargetActor->Transformation(kTransformID.c_str(),kTransformNextAction.c_str(), iTransformEffectID);
			if(iTransformNameNo)
			{
				std::wstring const *pkName = NULL;
				if(GetDefString(iTransformNameNo, pkName))
				{
					pkTargetActor->UpdateName(*pkName);
				}
			}
		}
	}

	if(g_pkWorld)
	{
		if( g_pkWorld->GetAttr() != GATTR_CHAOS_F && g_pkWorld->GetAttr() != GATTR_CHAOS_MISSION && g_pkWorld->IsForceSpotlight() == false )
		{
			if( bDarkSight )
			{
				g_pkWorld->TurnOnSpotLight(true);
			}
			else
			{
				g_pkWorld->TurnOnSpotLight(false);
			}
		}
	}

	SetInputDirReverseCount(pkTargetActor->IsMyActor(), bReverseInputDir);
}
PgStatusEffect* PgStatusEffectMan::GetStatusEffect(std::string const& kID, int const iEffectValue)
{
	PgStatusEffect* pkEffect = GetStatusEffectEx(kID);

	if(!pkEffect)
	{
		return NULL;
	}

	char const* strRedirection = pkEffect->GetRedirectionXMLID(iEffectValue);
	if(!strRedirection)
	{
		return pkEffect;
	}

	return GetStatusEffectEx(std::string(strRedirection));
}

PgStatusEffect* PgStatusEffectMan::GetStatusEffectEx(std::string const& kID)
{
	if(kID == "" || kID == "0")
	{
		return NULL;
	}
	
	PgStatusEffect* pEffect = NULL;
	ContStatusEffect::iterator	itor = m_ContStatusEffect.find(kID);
	if(m_ContStatusEffect.end() != itor)
	{
		return	itor->second;
	}

	pEffect = dynamic_cast<PgStatusEffect *>(PgXmlLoader::CreateObject(kID.c_str()));
	PG_ASSERT_LOG(pEffect);

	if(pEffect)
	{
		m_ContStatusEffect.insert(std::make_pair(kID,pEffect));
	}

	return pEffect;
}


size_t PgStatusEffectMan::DisplayHeadTransformEffect(PgActor* const pkActor, bool const bShow)
{// 머리 변형 스킬(HIDE_ENABLE==1)인 이펙트를 보이거나 감춘다
	size_t FoundCnt=0;
	if(!pkActor)
	{
		return 0;
	}
	PgPilot* pkTargetPilot = pkActor->GetPilot();
	if(!pkTargetPilot)
	{
		return 0;
	}	
	PgActor* pkTargetActor = dynamic_cast<PgActor*>(pkTargetPilot->GetWorldObject());
	if(!pkTargetActor)
	{
		return 0;
	}

	bool bExistTransformAllEffect = false;
	{
		PgStatusEffectInstance* pkInstance = NULL;
		StatusEffectInstanceList& kEffectList = pkActor->GetStatusEffectInstanceList();
		StatusEffectInstanceList::iterator itor = kEffectList.begin();

		while(itor != kEffectList.end())
		{// 액터에 적용된 이펙트 리스트 중
			pkInstance = (*itor);
			PgStatusEffect const* pkStatusEffect = pkInstance->GetStatusEffect();
			if(pkStatusEffect->GetValidItems() & PgStatusEffect::IT_TRANSFORM)
			{// 전체 변신 스킬이 적용 되고 있는지 확인하고
				bExistTransformAllEffect = true;
				break;
			}
			++itor;
		}
		if(bExistTransformAllEffect)
		{// 전체 변신 스킬이 적용 되고 있으면
			return 0;
		}
	}

	PgStatusEffectInstance* pkInstance = NULL;
	StatusEffectInstanceList& kEffectList = pkActor->GetStatusEffectInstanceList();
	StatusEffectInstanceList::iterator itor = kEffectList.begin();	

	while(itor != kEffectList.end())
	{// 액터에 적용된 이펙트 리스트 중
		pkInstance = (*itor);
		PgStatusEffect const* pkStatusEffect  = pkInstance->GetStatusEffect();
		if(pkStatusEffect->GetID() == "se_hide_face")
		{// 얼굴 감추기가 존재 하면
			if(!bShow)
			{// 투구 보이는 동작을 하기 위해(머리 변형 스킬 이펙트를 감추기위해)
				if(EVS_HIDE == pkInstance->GetVisualState())
				{// 얼굴 감추기가 비활성화 되어있으면 다시 켜주고
					pkInstance->AttachEffect(pkTargetPilot,false);
					pkInstance->SetVisualState(EVS_VISIBLE);
					pkInstance->SetTransformation(pkTargetActor);
					pkInstance->AttachAdjustedItem(pkTargetActor);
				}
			}
		}
		if(pkStatusEffect 
			&& PgStatusEffect::E_HELMET_CHANGE_TYPE == pkStatusEffect->GetHideEnable())
		{
			if(bShow)
			{// 가장 최근에 감춰진 변형 타입 이펙트를 보이게 한다
				ShowTemporaryHidedEffect(pkTargetPilot,pkStatusEffect->GetType());
				break;
			}
			else
			{// 보이는 이펙트를 감추고
				if(EVS_HIDE != pkInstance->GetVisualState())
				{
					pkInstance->DetachEffect(pkTargetPilot);
					pkInstance->SetVisualState(EVS_HIDE);
					pkInstance->RestoreTransformation(pkTargetActor);
					pkInstance->DetachAdjustedItem(pkTargetActor);
					pkInstance->StopSound();
				}
			}
			UpdateEffect(pkTargetPilot);
			++FoundCnt;	// 몇개나 수정되었는지 카운트 하여
		}
		++itor;
	}
	return FoundCnt;	// 알려준다
}

bool DefaultEffectMsg(std::wstring const *pkName, EEffectType const eType, BM::vstring & kOutText, ENoticeLevel & eNoticeLevel)
{
	if(!pkName)
	{
		return false;
	}

	switch(eType)
	{
	case EFFECT_TYPE_BLESSED:
		{// 버프
			kOutText = TTW(799030);
			kOutText.Replace( L"#BUFF#", pkName->c_str());
			eNoticeLevel = EL_Notice4;
		}break;
	case EFFECT_TYPE_CURSED:
		{// 디버프
			kOutText = TTW(799031);
			kOutText.Replace( L"#DEBUFF#", pkName->c_str());
			eNoticeLevel = EL_Warning;
		}break;
	}
	return true;
}

bool PgStatusEffectMan::NoticeMsgWhenEffectApplyToPlayer(int iEffectID, PgPilot* pkTargetPilot)
{
	if(!pkTargetPilot)
	{
		return false;
	}
	if(g_kPilotMan.GetPlayerPilot() != pkTargetPilot)
	{
		return false;
	}
	
	GET_DEF(CEffectDefMgr, kEffectDefMgr);
	CEffectDef const* pkDefEff = kEffectDefMgr.GetDef(iEffectID);
	if(!pkDefEff)
	{
		return false;
	}
	
	int const iReplacementNameNo = pkDefEff->GetAbil(AT_USE_EFFECT_MSG_BY_STRINGNO);
	std::wstring const *pkName = NULL;
	if(0 < iReplacementNameNo)
	{// 이펙트 이름을 지정하는 어빌이 있으면 출력하고
		GetDefString(iReplacementNameNo, pkName);
	}
	else if(0 == pkDefEff->GetAbil(AT_APPLY_EFFECT_MSG))
	{// 이름 보이는 어빌이 없으면 이름을 보여줄수 없지만
		return false;
	}

	if(0 < pkDefEff->GetAbil(AT_NAMENO))
	{// 이름을 보이는 어빌이 있으면NameNo를 얻어다 보여주고
		GetDefString(pkDefEff->GetAbil(AT_NAMENO), pkName);
	}
	else
	{// 그것도 없으면 이름을 보여줄수 없다
		return false;
	}
	
	BM::vstring kText;
	ENoticeLevel eNoticeLevel = EL_Normal;

	switch(pkDefEff->GetAbil(AT_APPLY_EFFECT_MSG_TYPE))
	{
	case EFTMT_MODE7:
		{
			if(pkName)
			{
				kText = TTW(401152);
				kText.Replace(L"#NAME#", *pkName);
			}
		}break;
	default:
		{
			DefaultEffectMsg(pkName, static_cast<EEffectType>(pkDefEff->GetType()), kText, eNoticeLevel);
		}break;
	}

	if( 0==kText.size() )
	{
		return false;
	}

	Notice_Show(kText, eNoticeLevel, false);
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	class	PgStatusEffectInstance
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
PgStatusEffectInstance::~PgStatusEffectInstance()
{

}

void PgStatusEffectInstance::ChangeDefaultAttack(PgPilot* pkTargetPilot)
{
	if(m_pkStatusEffect->GetValidItems() & PgStatusEffect::IT_CHANGE_DEFAULT_ATTACK)
	{
		PgActor* pkActor = dynamic_cast<PgActor*>(pkTargetPilot->GetWorldObject());
		if(pkActor)
		{
			pkActor->SetNormalAttackActionID(m_pkStatusEffect->GetNewDefaultAttack().c_str());
		}
	}
}
void PgStatusEffectInstance::RestoreDefaultAttack(PgPilot* pkTargetPilot)
{
	if(m_pkStatusEffect->GetValidItems() & PgStatusEffect::IT_CHANGE_DEFAULT_ATTACK)
	{
		PgActor* pkActor = dynamic_cast<PgActor*>(pkTargetPilot->GetWorldObject());
		if(pkActor)
		{
			pkActor->SetNormalAttackActionID(m_pkStatusEffect->GetOriginalDefaultAttack().c_str());
		}
	}
}

void PgStatusEffectInstance::ShowSkillTexts(PgPilot* pkTargetPilot)
{
	PgActor* pkActor = dynamic_cast<PgActor*>(pkTargetPilot->GetWorldObject());
	if(pkActor)
	{
		NiPoint3 kTargetPos = pkActor->GetPos();
		kTargetPos.z += 20.0f;

		if(pkActor->GetActorManager())
		{
			//NiAVObject* pkDummy = pkActor->GetNIFRoot()->GetObjectByName(ATTACH_POINT_STAR);
			NiAVObject* pkDummy = pkActor->GetNodePointStar();
			if(pkDummy)
			{
				kTargetPos = pkDummy->GetWorldTranslate();
			}
		}

		stStatusEffectSkillText	*pkSkillText;

		PgStatusEffect::SkillTextCont const& rkSkillTextContainer = m_pkStatusEffect->GetSkillTextContainer();
		for(PgStatusEffect::SkillTextCont::const_iterator itor = rkSkillTextContainer.begin(); itor != rkSkillTextContainer.end(); ++itor)
		{
			pkSkillText = (*itor);
			if(-1 == pkSkillText->m_iSkillTextType)
			{
				continue;
			}

			if(stStatusEffectSkillText::SESTT_SKILL_TEXT == pkSkillText->m_bySkillTextType)
			{
				pkActor->ShowSkillText(kTargetPos, pkSkillText->m_iSkillTextType, pkSkillText->m_bIsUp);
				kTargetPos.z += 20.0f;
			}
			else if(stStatusEffectSkillText::SESTT_SIMPLE_TEXT == pkSkillText->m_bySkillTextType)
			{
				pkActor->ShowSimpleText(kTargetPos, pkSkillText->m_iSkillTextType);
				kTargetPos.z += 20.0f;
			}
		}
	}
}

void PgStatusEffectInstance::HideParts(PgPilot* pkTargetPilot)
{
	if(!pkTargetPilot || !pkTargetPilot->GetWorldObject())
	{
		return;
	}

	PgActor* pkActor = dynamic_cast<PgActor*>(pkTargetPilot->GetWorldObject());
	if(!pkActor)
	{
		return;
	}

	//_PgOutputDebugString("[PgStatusEffect::HideParts] Actor : %s\n",MB(pkTargetPilot->GetGuid().str()));

	PgStatusEffect::IntCont const& rkHidePartContainer = m_pkStatusEffect->GetHidePartContainer();
	for(PgStatusEffect::IntCont::const_iterator itor = rkHidePartContainer.begin(); itor != rkHidePartContainer.end(); ++itor)
	{
		pkActor->HideParts(static_cast<eEquipLimit>(*itor),true);
	}
}
void PgStatusEffectInstance::ShowParts(PgPilot* pkTargetPilot)
{
	if(!pkTargetPilot || !pkTargetPilot->GetWorldObject())
	{
		return;
	}

	PgActor* pkActor = dynamic_cast<PgActor*>(pkTargetPilot->GetWorldObject());
	if(!pkActor)
	{
		return;
	}

	//_PgOutputDebugString("[PgStatusEffect::ShowParts] Actor : %s\n",MB(pkTargetPilot->GetGuid().str()));

	PgStatusEffect::IntCont const& rkHidePartContainer = m_pkStatusEffect->GetHidePartContainer();
	for(PgStatusEffect::IntCont::const_iterator itor = rkHidePartContainer.begin(); itor != rkHidePartContainer.end(); ++itor)
	{
		pkActor->HideParts(static_cast<eEquipLimit>(*itor), false);
	}
}

void PgStatusEffectInstance::ShowPartsIfOnlyHided(PgPilot* pkTargetPilot)
{
	if(!pkTargetPilot || !pkTargetPilot->GetWorldObject())
	{
		return;
	}

	PgActor* pkActor = dynamic_cast<PgActor*>(pkTargetPilot->GetWorldObject());
	if(!pkActor)
	{
		return;
	}

	//_PgOutputDebugString("[PgStatusEffect::ShowPartsIfOnlyHided] Actor : %s\n",MB(pkTargetPilot->GetGuid().str()));
		
	PgStatusEffect::IntCont const& rkHidePartContainer = m_pkStatusEffect->GetHidePartContainer();
	for(PgStatusEffect::IntCont::const_iterator itor = rkHidePartContainer.begin(); itor != rkHidePartContainer.end(); ++itor)
	{
		int iHideCnt=0;
		eEquipLimit const eEquip = static_cast<eEquipLimit>(*itor);
		pkActor->GetPartsHideCnt(eEquip, iHideCnt);
		if(iHideCnt > 0)
		{// 오직 이미 Hide된 파츠에만 Hide를 푼다
			pkActor->HideParts(eEquip, false);
		}
	}
}

void PgStatusEffectInstance::HidePartsIfOnlyShowed(PgPilot* pkTargetPilot)
{
	if(!pkTargetPilot || !pkTargetPilot->GetWorldObject())
	{
		return;
	}

	PgActor* pkActor = dynamic_cast<PgActor*>(pkTargetPilot->GetWorldObject());
	if(!pkActor)
	{
		return;
	}

	//_PgOutputDebugString("[PgStatusEffect::HidePartsIfOnlyShowed] Actor : %s\n",MB(pkTargetPilot->GetGuid().str()));
		
	PgStatusEffect::IntCont const& rkHidePartContainer = m_pkStatusEffect->GetHidePartContainer();
	for(PgStatusEffect::IntCont::const_iterator itor = rkHidePartContainer.begin(); itor != rkHidePartContainer.end(); ++itor)
	{
		int iHideCnt=0;
		eEquipLimit const eEquip = static_cast<eEquipLimit>(*itor);
		pkActor->GetPartsHideCnt(eEquip, iHideCnt);
		if(iHideCnt == 0)
		{// 보여지고 있을때만 hide 시킨다
			pkActor->HideParts(eEquip, true);
		}
	}
}

void PgStatusEffectInstance::AttachParticles(PgPilot* pkTargetPilot, bool const bNowAdded)
{
	if(m_pkStatusEffect)
	{
		AttachParticles(pkTargetPilot, m_pkStatusEffect->GetParticleContainer(), bNowAdded);
	}
}

void PgStatusEffectInstance::AttachParticles(PgPilot* pkTargetPilot, PgStatusEffect::ParticleCont const& rkParticleContainer, bool const bNowAdded)
{
	if(!pkTargetPilot || !pkTargetPilot->GetWorldObject())
	{
		return;
	}

	PgActor* pkActor = dynamic_cast<PgActor*>(pkTargetPilot->GetWorldObject());
	if(!pkActor)
	{
		return;
	}
	int	iSlotNum = GetInstanceID() * 1000000;
	PgParticle* pkParticle = NULL;
	stStatusEffectParticle* pkParticleInfo = NULL;
	for(PgStatusEffect::ParticleCont::const_iterator itor = rkParticleContainer.begin(); itor != rkParticleContainer.end(); ++itor)
	{
		pkParticleInfo = (*itor);
		float fSize = pkActor->GetEffectScale();
		if(0.0f != pkParticleInfo->m_fScale)
		{
			fSize = pkParticleInfo->m_fScale;
		}
		else if(pkParticleInfo->m_bAutoScale)
		{
			fSize = lua_tinker::call<float,lwActor>("GetEffectAutoScale", lwActor(pkActor));
		}

		fSize = StatusEffectRangeScaleByAbil(fSize);//도발스킬 이펙트 범위 체크

		bool bIsAttachParticle = false;
		//모두에게 보여지는 파티클
		if(stStatusEffectParticle::SO_ALL == pkParticleInfo->m_byShowOption)
		{
			bIsAttachParticle = true;
		}
		//파티클이 붙을 때 옵션이 있을 경우
		else
		{
			BYTE byParticleType = 0;

			// 나 일 경우
			if(pkActor->IsMyActor())
			{
				byParticleType |= 1 << stStatusEffectParticle::SO_ME;
			}
			else if(pkActor->GetPilot())
			{
				// 필드에서는 팀이 0으로 세팅되어 있다. 즉 Player의 팀은 0
				if(0 == pkActor->GetPilot()->GetAbil(AT_TEAM)) // 0이면 팀이 없는 것
				{
					byParticleType |= 1 << stStatusEffectParticle::SO_MY_TEAM;
				}
				// PvP 상황
				else // 팀이 있을 경우
				{
					PgActor* pkMyActor = g_kPilotMan.GetPlayerActor();
					if(pkMyActor)
					{
						if(pkMyActor->GetPilot())
						{
							// 나와 같은 팀
							if(pkActor->GetPilot()->GetAbil(AT_TEAM) == pkMyActor->GetPilot()->GetAbil(AT_TEAM))
							{
								byParticleType |= 1 << stStatusEffectParticle::SO_MY_TEAM;
							}
							// 나의 적
							else
							{
								byParticleType |= 1 << stStatusEffectParticle::SO_ENEMY;
							}
						}
					}
				}
			}

			// 붙는 옵션이 일치 하면
			if(pkParticleInfo->m_byShowOption & byParticleType)
			{
				bIsAttachParticle = true;
			}
		}

		PgParticle* pkParticle = g_kParticleMan.GetParticle(pkParticleInfo->m_kParticleID.c_str(),PgParticle::O_SCALE, fSize);		
		if(pkParticle && bIsAttachParticle)
		{
			//	NowAdded 가 false 경우에는, Loop 되는 이펙트만 붙인다.
			if(false == pkParticle->IsLoop() && false == bNowAdded)
			{
				++iSlotNum;
				continue;
			}
			
			if(!pkParticleInfo->m_kFollowRotationTargetNodeID.empty())
			{// 특정 노드의 회전값을 세팅해주고
				std::string const& kNodeID = pkParticleInfo->m_kFollowRotationTargetNodeID;
				NiQuaternion kQuaternion(0,0,0,1);
				if(kNodeID == "PLAYER")
				{// 플레이어의 방향을 넣는다면
					pkActor->GetRotate(kQuaternion);
				}
				else
				{// 노드의 방향을 넣는다면
					lwActor kTemp(pkActor);
					lwQuaternion kQuat = kTemp.GetNodeWorldRotate(kNodeID.c_str());
					kQuaternion = kQuat();
				}
				pkParticle->SetRotate(kQuaternion);
			}

			pkParticle->SetNoFollowParentRotation(pkParticleInfo->m_bNoFollowParentRotation);

			NiPoint3 kRandomOffset(0.0f, 0.0f, 0.0f);
			// RandomOffset이 있을 경우
			if((0.0f != pkParticleInfo->m_kRandomOffsetMin.x || 0.0f != pkParticleInfo->m_kRandomOffsetMin.y || 0.0f != pkParticleInfo->m_kRandomOffsetMin.z)
				&&(0.0f != pkParticleInfo->m_kRandomOffsetMax.x || 0.0f != pkParticleInfo->m_kRandomOffsetMax.y || 0.0f != pkParticleInfo->m_kRandomOffsetMax.z)
				)
			{
				kRandomOffset.x = BM::Rand_Range(pkParticleInfo->m_kRandomOffsetMax.x, pkParticleInfo->m_kRandomOffsetMin.x);
				kRandomOffset.y = BM::Rand_Range(pkParticleInfo->m_kRandomOffsetMax.y, pkParticleInfo->m_kRandomOffsetMin.y);
				kRandomOffset.z = BM::Rand_Range(pkParticleInfo->m_kRandomOffsetMax.z, pkParticleInfo->m_kRandomOffsetMin.z);
			}
			
			if(pkParticleInfo->m_bSeeFront)
			{// 파티클을 위치점에 붙이는게 아니라면 적용되지 않음에 유의
				NiPoint3 kLookingDir = -pkActor->GetPathNormal();
				kLookingDir.z=0;
				kLookingDir.Unitize();
				NiPoint3 kCross = kLookingDir.UnitCross(NiPoint3::UNIT_Y);
				if(kCross.SqrLength() < 0.0001f)
				{// LookingDir이 Y축과 딱 맞으면, UnitCross는 (0,0,0)을 리턴한다.
					kCross = (kLookingDir.y > 0 ? NiPoint3::UNIT_Z : -NiPoint3::UNIT_Z);
				}
				NiQuaternion kRot(NiACos(kLookingDir.Dot(-NiPoint3::UNIT_Y)), kCross);
				pkParticle->SetRotate(kRot);
			}

			if(pkParticleInfo->m_bAttachToWorld)
			{
				if(g_pkWorld)
				{
					NiPoint3 kPoint(0.0f, 0.0f, 0.0f);
					if(!pkParticleInfo->m_kAttachTargetNodeID.empty())
					{
						kPoint = lwActor(pkActor).GetNodeWorldPos(pkParticleInfo->m_kAttachTargetNodeID.c_str())();
					}
					else
					{
						kPoint = pkActor->GetPos();
					}
					kPoint += kRandomOffset;

					int iWorldAttachSlotID = g_pkWorld->AttachParticle(NiDynamicCast(NiAVObject, pkParticle), kPoint);
					m_kAttachedStatusEffectParticleContainer.insert(std::make_pair(pkParticleInfo->m_iInstanceID, iWorldAttachSlotID));
					//상태이상에서 Attach 시키는 파티클이 World의 어느 슬롯에 저장되어있는지 기록해둔다.
				}
				
			}
			// Attach To Point로 붙일 것인가?
			else if(pkParticleInfo->m_bAttachToPoint && !pkParticleInfo->m_bAttachToWorld)
			{
				NiPoint3 kPos = pkActor->GetTranslate() + kRandomOffset;
				pkActor->AttachToPoint(iSlotNum, pkActor->GetTranslate(), (NiAVObject*)pkParticle);
			}			
			else
			{
				pkActor->AttachTo(iSlotNum, pkParticleInfo->m_kAttachTargetNodeID.c_str(), (NiAVObject*)pkParticle);
			}

			PgWorld::IncTimeToAniObj(pkParticle, static_cast<unsigned __int64>(pkParticleInfo->m_fPhase));
			if( !pkParticleInfo->m_kParticleSoundID.empty() )
			{
				pkActor->AttachToSound(NiAudioSource::TYPE_3D, pkParticleInfo->m_kParticleSoundID.c_str(), 0.0f, 0.0f, 0.0f);
			}
			pkActor->NeedToUpdateMaterialProp(true);
			pkParticleInfo->m_bIsAttached = true;
		}
		++iSlotNum;
	}

}

void PgStatusEffectInstance::DetachParticles(PgPilot* pkTargetPilot)
{
	int	iSlotNum = GetInstanceID() * 1000000;
	PgParticle* pkParticle = NULL;
	stStatusEffectParticle* pkParticleInfo = NULL;

	PgStatusEffect::ParticleCont const& rkParticleContainer = m_pkStatusEffect->GetParticleContainer();
	for(PgStatusEffect::ParticleCont::const_iterator itor = rkParticleContainer.begin(); itor != rkParticleContainer.end(); ++itor)
	{
		pkParticleInfo = (*itor);
		if(pkParticleInfo->m_bIsAttached)
		{
			if(!pkParticleInfo->m_bAttachToWorld)
			{
				if(!pkTargetPilot->GetWorldObject())
				{
					return;
				}

				PgActor* pkActor = dynamic_cast<PgActor*>(pkTargetPilot->GetWorldObject());
				if(!pkActor)
				{
					return;
				}

				if(false==pkParticleInfo->m_bNotDetachParticle)
				{
					pkActor->DetachFrom(iSlotNum, true);
					pkActor->RemoveTransformEffectAttachInfo(GetEffectID());
				}
			}
			else
			{
				if(g_pkWorld)
				{
					ContIDMaped::const_iterator itor = m_kAttachedStatusEffectParticleContainer.find(pkParticleInfo->m_iInstanceID); //해당 인스턴스 ID로 Attach된 Slot을 찾는다.
					if(m_kAttachedStatusEffectParticleContainer.end() != itor)
					{
						if(false==pkParticleInfo->m_bNotDetachParticle)
						{
							g_pkWorld->DetachParticle((*itor).second);
						}
					}
				}
			}
		}
		++iSlotNum;
	}
}
void PgStatusEffectInstance::AttachEffect(PgPilot* pkTargetPilot,bool const bNowAdded)
{
	if(!pkTargetPilot)
	{
		return;
	}

	PgActor* pkActor = dynamic_cast<PgActor*>(pkTargetPilot->GetWorldObject());
	if(!pkActor)
	{
		return;
	}

	PgStatusEffect const* const pkStEff = GetStatusEffect();
	if(!pkStEff)
	{
		return;
	}

	HideParts(pkTargetPilot);
	AttachParticles(pkTargetPilot,bNowAdded);
	ChangeDefaultAttack(pkTargetPilot);
	DoInputSlotIDEnable();
	ChangeTexture(pkTargetPilot);
	AttachWhiteOut(pkTargetPilot);

	{// 투구 감추기 옵션이 있을 경우
		bool bDisplayHelmet = 0;
		int const iOption = pkTargetPilot->GetAbil(AT_CLIENT_OPTION_SAVE) ;
		READ_DWORD_TO_BOOL(iOption, OPTION_TYPE_SHOW_HELMET, bDisplayHelmet);

		if(true == bDisplayHelmet
			&& PgStatusEffect::E_HELMET_CHANGE_TYPE == pkStEff->GetHideEnable())
		{// Actor의 투구 감추기 어빌이 존재하고,이펙트의 속성중 HIDE_ENABLE값이 존재하면
			PgActor* pkActor = g_kPilotMan.FindActor(pkTargetPilot->GetGuid());
			if(pkActor)
			{// 투구 이펙트들을 감춘다
				g_kStatusEffectMan.DisplayHeadTransformEffect(pkActor, false);
			}
		}
	}


	if(PgStatusEffect::IT_INVISIBLE & pkStEff->GetValidItems())
	{
		pkActor->SetInvisible(true);
	}
	if(PgStatusEffect::IT_ALPHA & pkStEff->GetValidItems())
	{
		m_fDetachAlpha = (-1==pkStEff->GetAlphaEnd()) ? 1 : pkStEff->GetAlphaEnd();
		pkActor->SetTargetAlpha(pkStEff->GetColorA().a, pkStEff->GetColorA().a, pkStEff->GetAlphaStartTransitionTime());
	}
	if(PgStatusEffect::IT_BODY_SIZE & pkStEff->GetValidItems())
	{
		pkActor->SetTargetScale(pkActor->GetScale() * pkStEff->GetBodySize(), pkStEff->GetBodyChangeTime());
	}
	if(pkStEff->GetGIFTitleEmoticonID())
	{
		pkTargetPilot->SetAbil(AT_DISPLAY_GIF_TITLE,pkStEff->GetGIFTitleEmoticonID());
		pkActor->UpdateName();
	}
	if(pkStEff->GetActivateEventScript())
	{
		g_kEventScriptSystem.ActivateEvent(pkStEff->GetActivateEventScript());
	}
	if(PgStatusEffect::IT_HIDE_NODE & pkStEff->GetValidItems())
	{
		pkActor->HideNode(pkStEff->GetHideNodeName().c_str(),true);
	}
	if(PgStatusEffect::IT_CHANGE_ACTION & pkStEff->GetValidItems())
	{
		PgAction * pkAction = NULL;
		PgStatusEffect::CONT_CHANGE_ACTION const& kCont = pkStEff->GetContChangeAction();
		PgStatusEffect::CONT_CHANGE_ACTION::const_iterator c_it = kCont.begin();
		while(c_it != kCont.end())
		{
			if(pkAction = pkActor->GetAction())
			{
				if(pkAction->GetID() == (*c_it).first)
				{
					pkActor->CancelAction(pkAction->GetActionNo(), pkAction->GetActionInstanceID(), (*c_it).second.c_str());
				}
			}
			pkActor->AddChangeAction((*c_it).first, (*c_it).second);
			++c_it;
		}
	}
}

void PgStatusEffectInstance::DetachEffect(PgPilot* pkTargetPilot)
{
	if(!pkTargetPilot)
	{
		return;
	}

	DetachParticles(pkTargetPilot);
	ShowParts(pkTargetPilot);
	
	RestoreDefaultAttack(pkTargetPilot);
	UndoInputSlotIDEnable();

	PgStatusEffect const* const pkStEff = GetStatusEffect();
	PgActor* pkActor = dynamic_cast<PgActor*>(pkTargetPilot->GetWorldObject());
	if( pkStEff && pkActor )
	{
		if(PgStatusEffect::IT_CHANGE_ACTION & pkStEff->GetValidItems())
		{
			PgAction * pkAction = NULL;
			PgStatusEffect::CONT_CHANGE_ACTION const& kCont = pkStEff->GetContChangeAction();
			PgStatusEffect::CONT_CHANGE_ACTION::const_iterator c_it = kCont.begin();
			while(c_it != kCont.end())
			{
				pkActor->DelChangeAction((*c_it).first);
				if(pkAction = pkActor->GetAction())
				{
					if(pkAction->GetID() == (*c_it).first)
					{
						pkActor->CancelAction(pkAction->GetActionNo(), pkAction->GetActionInstanceID(), (*c_it).second.c_str());
					}
				}
				++c_it;
			}
		}
		if(PgStatusEffect::IT_DETACH_ACTION & pkStEff->GetValidItems() )
		{
			pkActor->ReserveTransitAction(GetStatusEffect()->GetDetachActionID().c_str());
		}
		if(PgStatusEffect::IT_DETACH_PARTICLE & pkStEff->GetValidItems())
		{
			AttachParticles(pkTargetPilot, pkStEff->GetDetachParticleContainer(), true);
		}
		if(PgStatusEffect::IT_ALPHA & pkStEff->GetValidItems())
		{
			pkActor->SetTargetAlpha(pkActor->GetAlpha(), m_fDetachAlpha, pkStEff->GetAlphaEndTransitionTime());
		}
		if(PgStatusEffect::IT_INVISIBLE & pkStEff->GetValidItems())
		{
			pkActor->SetInvisible(false);
		}
		if(PgStatusEffect::IT_BODY_SIZE & pkStEff->GetValidItems())
		{
			pkActor->SetTargetScale(pkActor->GetScale(), pkStEff->GetBodyChangeTime());
		}
		if(pkStEff->GetGIFTitleEmoticonID())
		{
			pkTargetPilot->SetAbil(AT_DISPLAY_GIF_TITLE, 0);
			pkActor->UpdateName();
		}
		if(pkStEff->GetDeactivateEventScript())
		{
			g_kEventScriptSystem.ActivateEvent(pkStEff->GetDeactivateEventScript());
		}
		if(PgStatusEffect::IT_HIDE_NODE & pkStEff->GetValidItems())
		{
			pkActor->HideNode(pkStEff->GetHideNodeName().c_str(),false);
		}
	}

	ChangeTexture(pkTargetPilot, true);
	DetachWhiteOut(pkTargetPilot);
}

void PgStatusEffectInstance::AttachEffectAbil(PgPilot *pkTargetPilot)
{
	PgStatusEffect const* const pkStEff = GetStatusEffect();
	if( pkStEff )
	{
		CUnit *pkUnit = pkTargetPilot->GetUnit();
		if ( pkUnit )
		{
			PgStatusEffect::CONT_ADD_ABIL const &kContAbil = pkStEff->GetAddAbil();
			PgStatusEffect::CONT_ADD_ABIL::const_iterator abil_itr = kContAbil.begin();
			for ( ; abil_itr != kContAbil.end() ; ++abil_itr )
			{
				pkUnit->AddAbil( abil_itr->first, abil_itr->second, true );
			}
		}
	}
}

void PgStatusEffectInstance::DetachEffectAbil(PgPilot *pkTargetPilot)
{
	PgStatusEffect const* const pkStEff = GetStatusEffect();
	if( pkStEff )
	{
		CUnit *pkUnit = pkTargetPilot->GetUnit();
		if ( pkUnit )
		{
			PgStatusEffect::CONT_ADD_ABIL const &kContAbil = pkStEff->GetAddAbil();
			PgStatusEffect::CONT_ADD_ABIL::const_iterator abil_itr = kContAbil.begin();
			for ( ; abil_itr != kContAbil.end() ; ++abil_itr )
			{
				pkUnit->AddAbil( abil_itr->first, -(abil_itr->second), true );

				if ( 0 > pkUnit->CAbilObject::GetAbil( abil_itr->first ) )
				{// 방어 코드 추가
					pkUnit->CAbilObject::EraseAbil( abil_itr->first );
					NILOG(PGLOG_LOG, "DetachEffectAbil Type<%u> Error\n", abil_itr->first );	
				}
			}
		}
	}
}

void PgStatusEffectInstance::StartEffect(PgPilot* pkTargetPilot)
{
	PgActor* pkActor = g_kPilotMan.FindActor(pkTargetPilot->GetGuid());
	if(pkActor && PgStatusEffect::IT_START_ACTION&GetStatusEffect()->GetValidItems())
	{
		// 액션이 나일 경우에만 하도록 되어 있는데 나의 액터가 아닌 경우라면 하지말아야 한다.
		if((SAOT_ME&GetStatusEffect()->GetStartActionOption()) && !pkActor->IsUnderMyControl())
		{
			return;
		}

		if( (SAOT_NOT_OVERLAP & GetStatusEffect()->GetStartActionOption())
		&& pkActor->GetAction()
		&& pkActor->GetAction()->GetID() == GetStatusEffect()->GetStartActionID())
		{
			return;
		}

		//현재 행동 취소
		if(PgAction* pkNowAction = pkActor->GetAction())
		{
			pkActor->CancelAction(pkNowAction->GetActionNo(), pkNowAction->GetActionInstanceID(), GetStatusEffect()->GetStartActionID().c_str());
		}
		else
		{
			pkActor->ReserveTransitAction( GetStatusEffect()->GetStartActionID().c_str() );
		}

		PgAction* pkAction = pkActor->GetAction();//pkActor->ReserveTransitAction(GetStatusEffect()->GetStartActionID().c_str());
		if( pkAction )
		{
			if( m_pkStatusEffect && m_pkStatusEffect->GetValidItems() & PgStatusEffect::IT_NOT_ACTION_SHIFT)
			{
				pkAction->SetActionOption(PgAction::AO_IGNORE_NOTACTIONSHIFT, true);
			}
			pkAction->SetParam(GetStatusEffect()->GetParamIndex(), GetStatusEffect()->GetParamID().c_str());
			if( GetStatusEffect()->GetEffectSave() )
			{
				static char szBuff[32];
				NiSprintf(szBuff, 31, "%d", GetStatusEffect()->GetEffectSave());
				pkAction->SetParam(GetStatusEffect()->GetParamIndex()+1, szBuff);
			}
		}
	}
}

void PgStatusEffectInstance::PlaySound(PgActor* pkTarget)
{
	std::string const kSoundID = m_pkStatusEffect->GetSoundID();
	if(!kSoundID.empty())
	{
		g_kSoundMan.PlayAudioSourceByID(NiAudioSource::TYPE_3D, kSoundID.c_str(), 0.0f, 0.0f, 0.0f, pkTarget, NULL, m_pkStatusEffect->GetLoopCount());
	}
}

void PgStatusEffectInstance::StopSound()
{
	std::string const kSoundID = m_pkStatusEffect->GetSoundID();
	if(!kSoundID.empty())
	{
		if(0 == m_pkStatusEffect->GetLoopCount()) // 무한 루프 사운드 이면 중지 시켜주어야 한다.
		{
			g_kSoundMan.StopAudioSourceByID(kSoundID);
		}
	}
}
void PgStatusEffectInstance::DoInputSlotIDEnable()
{
	if(!g_pkLocalManager)
	{
		return;
	}

	if((m_pkStatusEffect->GetValidItems() & PgStatusEffect::IT_INPUT_SLOT))
	{
		PgStatusEffect::VInputSlotInfo const& rvInputSlotInfo = m_pkStatusEffect->GetInputSlotInfo();
		
		int iTotal = rvInputSlotInfo.size();
		for(int i=0;i<iTotal;++i)
		{
			PgStatusEffect::stInputSlotInfo const& rkInfo = rvInputSlotInfo[i];
			g_pkLocalManager->ActivateExtendedSlot(rkInfo.m_iSlotID, rkInfo.m_bEnable);
		}
	}
}
void PgStatusEffectInstance::UndoInputSlotIDEnable()
{
	if(!g_pkLocalManager)
	{
		return;
	}

	if((m_pkStatusEffect->GetValidItems() & PgStatusEffect::IT_INPUT_SLOT))
	{
		PgStatusEffect::VInputSlotInfo const& rvInputSlotInfo = m_pkStatusEffect->GetInputSlotInfo();
		
		int iTotal = rvInputSlotInfo.size();		
		for(int i=0;i<iTotal;++i)
		{
			PgStatusEffect::stInputSlotInfo const& rkInfo = rvInputSlotInfo[i];
			g_pkLocalManager->ActivateExtendedSlot(rkInfo.m_iSlotID, !rkInfo.m_bEnable);
		}
	}
}

void PgStatusEffectInstance::SetTransformation(PgActor* pkTarget)
{
	if(GetStatusEffect())
	{
		std::string kTransformID;
		PgPilot* pkTargetPilot = pkTarget->GetPilot();
		//변신시 남녀에 따라 다르게 변신해야 하는 경우
		if(!GetStatusEffect()->GetTransformIDForMale().empty() && !GetStatusEffect()->GetTransformIDForFemale().empty())
		{
			if(pkTargetPilot)
			{
				if(UG_MALE == pkTargetPilot->GetAbil(AT_GENDER))
				{
					kTransformID = GetStatusEffect()->GetTransformIDForMale(); 
				}
				else
				{
					kTransformID = GetStatusEffect()->GetTransformIDForFemale(); 
				}
			}
		}
		else 
		{
			kTransformID = GetStatusEffect()->GetTransformID(); 
		}

		std::string const kTransformNextAction(GetStatusEffect()->GetTransformNextAction());
		if(!kTransformID.empty())
		{
			if(kTransformID != pkTarget->GetTransformedActorID())
			{
				if(!pkTarget->GetTransformedActorID().empty())
				{
					pkTarget->RestoreTransformation(ACTIONNAME_IDLE);
				}
				pkTarget->Transformation(kTransformID.c_str(),kTransformNextAction.c_str(), GetEffectID());
			}
		}
	}
}

void PgStatusEffectInstance::RestoreTransformation(PgActor* pkTarget)
{
	if(GetStatusEffect())
	{
		if(!GetStatusEffect()->GetTransformID().empty() || !GetStatusEffect()->GetTransformIDForMale().empty() || !GetStatusEffect()->GetTransformIDForFemale().empty())
		{
			pkTarget->RestoreTransformation(ACTIONNAME_IDLE);
		}
	}
}


//
template< typename _T_KEY, typename _T_LIST_VALUE >
void PgStatusEffectMan::AddReserve(std::map< _T_KEY, std::list< _T_LIST_VALUE > >& rkCont, _T_KEY const& rkKey, _T_LIST_VALUE const& rkValue)
{
	typedef std::map< _T_KEY, std::list< _T_LIST_VALUE > > ContMap;
	ContMap::iterator find_iter = rkCont.find( rkKey );
	if( rkCont.end() == find_iter )
	{
		std::pair<ContMap::iterator, bool> kRet = rkCont.insert( std::make_pair(rkKey, ContMap::mapped_type()) );
		if( kRet.second )
		{
			find_iter = kRet.first;
		}
	}

	if( rkCont.end() != find_iter )
	{
		ContMap::mapped_type& rkList = (*find_iter).second;
		if( rkList.end() == std::find(rkList.begin(), rkList.end(), rkValue) )
		{
			std::back_inserter(rkList) = rkValue;
		}
	}
}


//
void PgStatusEffectMan::AddReserveEffect(PgStatusEffectManUtil::SReservedEffect const& rkReserveEffect)
{
	using namespace PgStatusEffectManUtil;

	{
		BM::CAutoMutex kLock(m_kMutex);
		AddReserve(m_kAddReserve, rkReserveEffect.kPilotGuid, rkReserveEffect);

		//ContDelReserveEffect m_kDelReserve; // 예약시 삭제에 대해서 컨트롤 하지 않는다.
	}

	TryReservedDoEffect(rkReserveEffect.kPilotGuid); // 다시 확인
}


//
void PgStatusEffectMan::DelReserveEffect(BM::GUID const& rkReservePilotGuid, int const iEffectKey)
{
	using namespace PgStatusEffectManUtil;

	{
		BM::CAutoMutex kLock(m_kMutex);
		SReservedEffectKey kNewDelKey(rkReservePilotGuid, iEffectKey);
	
		bool bFindFromAddReseve = false;
		{
			// 추가 예약 된곳에서 검사
			ContAddReserveEffect::iterator find_iter = m_kAddReserve.find( rkReservePilotGuid );
			if( m_kAddReserve.end() != find_iter )
			{
				ContAddReserveEffect::mapped_type& rkAddList = (*find_iter).second;
				ContAddReserveEffect::mapped_type kNewAddList;

				std::remove_copy(rkAddList.begin(), rkAddList.end(), std::back_insert_iterator< ContAddReserveEffect::mapped_type >(kNewAddList), kNewDelKey);
				if( rkAddList.size() != kNewAddList.size() )
				{
					// 쓰레드 로딩중에 AddEffect 된것 예약 취소
					rkAddList.swap(kNewAddList);
					bFindFromAddReseve = true;
				}
			}
		}

		if( !bFindFromAddReseve ) // 삭제 예약에 추가
		{
			AddReserve(m_kDelReserve, rkReservePilotGuid, kNewDelKey);
		}
	}

	TryReservedDoEffect(rkReservePilotGuid); // 다시 확인
}


//
void PgStatusEffectMan::TryReservedDoEffect(BM::GUID const& rkReservePilotGuid)
{
	// Lock 없음
	PgPilot* pkPilot = g_kPilotMan.FindPilot( rkReservePilotGuid );
	if( pkPilot )
	{
		DoReserved(pkPilot);
	}
}

//
void PgStatusEffectMan::ClearReserveEffectAll()
{
	BM::CAutoMutex kLock(m_kMutex);
	m_kAddReserve.clear();
	m_kDelReserve.clear();
}


//
void PgStatusEffectMan::ClearReserveEffect(BM::GUID const& rkReservePilotGuid)
{
	using namespace PgStatusEffectManUtil;

	BM::CAutoMutex kLock(m_kMutex);
	m_kAddReserve.erase( rkReservePilotGuid );
	m_kDelReserve.erase( rkReservePilotGuid );
}


//
void PgStatusEffectMan::DoReserved(PgPilot* pkPilot)
{
	using namespace PgStatusEffectManUtil;

	if( !this )
	{
		return;
	}

	if( !pkPilot )
	{
		return;
	}

	BM::GUID const& rkPilotGuid = pkPilot->GetGuid();
	ContReserveEffect kDoAddList;
	ContReserveEffectKey kDoDelList;

	// Pop Do List (**Only THREAD-LOCK this block)
	{
		BM::CAutoMutex kLock(m_kMutex);
		ContAddReserveEffect::iterator addfind_iter = m_kAddReserve.find( rkPilotGuid );
		if( m_kAddReserve.end() != addfind_iter )
		{
			(*addfind_iter).second.swap(kDoAddList);
			m_kAddReserve.erase(addfind_iter);
		}

		ContDelReserveEffect::iterator delfind_iter = m_kDelReserve.find( rkPilotGuid );
		if( m_kDelReserve.end() != delfind_iter )
		{
			(*delfind_iter).second.swap(kDoDelList);
			m_kDelReserve.erase(delfind_iter);
		}
	}


	// DO Process
	if( !kDoAddList.empty() )
	{
		ContReserveEffect::const_iterator add_iter = kDoAddList.begin();
		while( kDoAddList.end() != add_iter )
		{
			AddReservedEffect(*this, pkPilot, (*add_iter));
			++add_iter;
		}
	}

	if( !kDoDelList.empty() )
	{
		ContReserveEffectKey::const_iterator del_iter = kDoDelList.begin();
		while( kDoDelList.end() != del_iter )
		{
			DelReservedEffect(*this, pkPilot, (*del_iter).iEffectID);
			++del_iter;
		}
	}
}

bool PgStatusEffectMan::ToggleStateChange(int const iEffectID, PgPilot* pkTargetPilot, bool const bOn)
{
	if(!pkTargetPilot)
	{
		return false;
	}

	GET_DEF(CSkillDefMgr, kSkilldefMgr);
	CSkillDef const* pkSkillDef = kSkilldefMgr.GetDef(iEffectID);
	if(!pkSkillDef)
	{
		return false;
	}
	if(EST_TOGGLE != pkSkillDef->GetType())
	{
		return false;
	}
	PgActor* pkTargetActor = dynamic_cast<PgActor*>(pkTargetPilot->GetWorldObject());
	if(!pkTargetActor)
	{
		return false;
	}
	if(bOn == !pkTargetActor->GetActionToggleState(pkSkillDef->No()) )
	{// 토글 이펙트가 걸릴 때 토클이 활성화 되어있는지 확인.
		pkTargetActor->ActionToggleStateChange(pkSkillDef->No(), bOn);
		return true;
	}

	return false;
}

//
namespace PgStatusEffectManUtil
{
	void AddEffect(PgStatusEffectMan& rkEffectMan, PgPilot* pkPilot, BM::GUID const& rkCasterGuid, int const iEffectID, int const iActionInstanceID, int const iValue, unsigned long const ulStartTime, unsigned long const ulEndTime, int iEffectKey, BM::DBTIMESTAMP_EX const * const pkExpireTime )
	{
		if( pkPilot )
		{
			PgPilot	*pkCasterPilot = g_kPilotMan.FindPilot( rkCasterGuid );

			rkEffectMan.RemoveStatusEffect(pkPilot, iEffectID); // 기존 것은 지운다.
			rkEffectMan.AddStatusEffect(pkPilot, pkCasterPilot, iActionInstanceID, iEffectID, iValue, false, true, 0, ulStartTime, ulEndTime, iEffectKey, pkExpireTime);
			rkEffectMan.NoticeMsgWhenEffectApplyToPlayer(iEffectID, pkPilot);	// 내 플레이어에게 걸리는 이펙트라면 메세지를 표시한다
		}
	}
	
	void DelEffect(PgStatusEffectMan& rkEffectMan, PgPilot* pkPilot, int const iEffectID)
	{
		if( pkPilot )
		{
			g_kStatusEffectMan.RemoveStatusEffect(pkPilot, iEffectID);
		}
	}

	bool AddReservedEffect(PgStatusEffectMan& rkEffectMan, PgPilot* pkPilot, SReservedEffect const& rkEffect)
	{
		if( pkPilot )
		{
			AddEffect(rkEffectMan, pkPilot, rkEffect.kCasterGuid, rkEffect.iEffectID, rkEffect.iActionInstanceID, rkEffect.iValue, rkEffect.ulStartTime, rkEffect.ulEndTime, rkEffect.iEffectKey, &(rkEffect.kExpireTime) );
//			rkEffectMan.AddStatusEffectAfterAction(pkPilot, rkEffect.iEffectID);
			return true;
		}
		return false; 
	}
	bool DelReservedEffect(PgStatusEffectMan& rkEffectMan, PgPilot* pkPilot, int const iEffectID)
	{
		if( pkPilot )
		{
			DelEffect(rkEffectMan, pkPilot, iEffectID);
			rkEffectMan.RemoveStatusEffectAfterAction(pkPilot, iEffectID);
			return true;
		}
		return false;
	}

	bool AddReservedEffect(PgStatusEffectMan& rkEffectMan, SReservedEffect const& rkEffect)
	{
		PgPilot* pkPilot = g_kPilotMan.FindPilot( rkEffect.kPilotGuid );
		return AddReservedEffect(rkEffectMan, pkPilot, rkEffect); // 내부에서 if( pkPilot ) 검사
	}
	bool DelReservedEffect(PgStatusEffectMan& rkEffectMan, BM::GUID const& rkPilotGuid, int const iEffectID)
	{
		PgPilot* pkPilot = g_kPilotMan.FindPilot( rkPilotGuid );
		return DelReservedEffect(rkEffectMan, pkPilot, iEffectID); // 내부에서 if( pkPilot ) 검사
	}
};

void PgStatusEffectInstance::ChangeTexture(PgPilot* pkTargetPilot, bool const bRecover)
{
	if(!m_pkStatusEffect)
	{
		return;
	}

	if(!(m_pkStatusEffect->GetValidItems() & PgStatusEffect::IT_TEXTURE_CHANGE))
	{
		return;
	}

	if(!pkTargetPilot || !pkTargetPilot->GetWorldObject())
	{
		return;
	}

	PgActor* pkActor = dynamic_cast<PgActor*>(pkTargetPilot->GetWorldObject());
	if(!pkActor)
	{
		return;
	}

	if(pkActor->GetCharRoot())
	{
		ChangeTextureRecursiveProcess(pkActor->GetCharRoot(), bRecover);
	}
}

void PgStatusEffectInstance::ChangeTextureRecursiveProcess(NiAVObject* pkObject, bool const bRestore)
{
	if(!pkObject)
	{
		return;
	}

	NiGeometry	*pkGeom = NiDynamicCast(NiGeometry, pkObject);
	if(pkGeom && pkGeom->GetPropertyState() && pkGeom->GetPropertyState()->GetTexturing() )
	{
		//일단 기존의 텍스쳐가 있는지
		NiTexturingProperty	*pkTextureProp = pkGeom->GetPropertyState()->GetTexturing();//NiDynamicCast(NiTexturingProperty, pkGeom->GetPropertyState()->GetTexturing()->Clone());
		if(pkTextureProp)
		{
			NiTexturingProperty::NiMapArray const &kMaps = pkTextureProp->GetMaps();
			unsigned int const iTotal = kMaps.GetSize();
			size_t const iTextureCount = m_pkStatusEffect->GetChangeTextureCount();
			PgStatusEffect::TEXTURE_PATH_LIST const& rkOriginList = bRestore ? m_pkStatusEffect->GetChangedTextureList() : m_pkStatusEffect->GetOriginTextureList();
			PgStatusEffect::TEXTURE_PATH_LIST const& rkChangeList = bRestore ? m_pkStatusEffect->GetOriginTextureList() : m_pkStatusEffect->GetChangedTextureList();

			NiTexturingProperty	*pkNewTextureProp = NULL;
			for(unsigned int i = 0;i < iTotal; ++i)
			{
				NiTexturingProperty::Map *pkMap = kMaps.GetAt(i);
				if(!pkMap || !pkMap->GetTexture())
				{
					continue;
				}

				NiSourceTexture* pkSrc = NiDynamicCast(NiSourceTexture, pkMap->GetTexture());
				if(!pkSrc)
				{
					continue;
				}

				NiFixedString const& strTexture = pkSrc->GetFilename();

				for(size_t si = 0; si < iTextureCount; ++si)
				{
					NiFixedString strPath(rkOriginList.at(si).c_str());
					if(strPath.ContainsNoCase(strTexture))//텍스쳐 이름이 같으면
					{
						NiSourceTexture* pkTexture = g_kNifMan.GetTexture(rkChangeList.at(si));	//텍스쳐를 진짜 찾을때는 절대경로로. 1:1 매칭 필수
						if(pkTexture)
						{
							if(!pkNewTextureProp)
							{
								pkNewTextureProp = NiDynamicCast(NiTexturingProperty, pkGeom->GetPropertyState()->GetTexturing()->Clone());
							}

							if(pkNewTextureProp)
							{
								NiTexturingProperty::NiMapArray const &kNewMaps = pkNewTextureProp->GetMaps();
								NiTexturingProperty::Map *pkNewMap = kNewMaps.GetAt(i);
								if(pkNewMap && pkNewMap->GetTexture())
								{
									pkNewMap->SetTexture(pkTexture);
								}
							}
						}
					}
				}
			}

			if(pkNewTextureProp)
			{
				pkGeom->DetachProperty(pkGeom->GetPropertyState()->GetTexturing());
				pkGeom->AttachProperty(pkNewTextureProp);
				NiPropertyState* pkNewState = NiNew NiPropertyState(*pkGeom->GetPropertyState());
				if(pkNewState)
				{
					pkNewState->SetProperty(pkNewTextureProp);

					pkGeom->SetPropertyState(pkNewState);
					pkGeom->UpdateProperties();
				}
			}
		}

        NewWare::Renderer::SetRenderStateTagExtraDataNumber( *pkGeom, 
                            NewWare::Renderer::Kernel::RenderStateTagExtraData::RESET_RENDERSTATE );
	}

	NiNode* pkNode = NiDynamicCast(NiNode, pkObject);
	if(pkNode)
	{
		for(unsigned int i = 0; i < pkNode->GetArrayCount(); ++i)
		{
			NiAVObject* pkChild = pkNode->GetAt(i);
			if(pkChild)
			{
				ChangeTextureRecursiveProcess(pkChild, bRestore);
			}
		}
	}
}

void PgStatusEffectInstance::AttachAdjustedItem(PgActor* pkTarget)
{
	if(m_pkStatusEffect)
	{
		if(!m_pkStatusEffect->GetEquipItemList().empty())
		{
			pkTarget->SetAdjustedItem(m_pkStatusEffect->GetEquipItemList(), m_pkStatusEffect->GetOtherEquipItemReturnValue());
			pkTarget->EquipAllItem();
		}
	}
}

void PgStatusEffectInstance::DetachAdjustedItem(PgActor* pkTarget)
{// StatusEffect로 붙여진 아이템들은
	if(m_pkStatusEffect)
	{// 이미 장착된 아이템이 있으면 장착된 아이템으로, 없으면 Default아이템으로 장착을 시키는데
		PgStatusEffect::EQUIP_ITEM_CONT const& rkCont = m_pkStatusEffect->GetEquipItemList();
		if( !rkCont.empty() )
		{// 장착 부위가 없고, Default 아이템도 없는 부위 라면 (ex 백팩) 떼어지지가 않기 때문에
			PgStatusEffect::EQUIP_ITEM_CONT kCopyCont = rkCont;	// 현재 StatusEffect로 붙여진 장비 아이템을 임시로 저장해두고
			{// PgActor->UnequipItem()이 AdjustedItem 컨테이너를 사용하기 때문에 
				PgStatusEffect::EQUIP_ITEM_CONT kTemp;
				pkTarget->SetAdjustedItem(kTemp, 0); // StatusEffect로 붙은 아이템정보를 싹 비운 후
			}
			
			PgStatusEffect::EQUIP_ITEM_CONT::const_iterator kItor = kCopyCont.begin();
			while( kCopyCont.end() != kItor )
			{// 임시로 저장해둔 StatusEffect로 인해 붙은 아이템 부위들을 떼어주고
				EEquipPos const ePos = static_cast< EEquipPos >(kItor->first);
				pkTarget->UnequipItem(IT_NONE, ePos, kItor->second, PgItemEx::LOAD_TYPE_INSTANT);
				++kItor;
			}
			// 모든 아이템이 정리된 상태에서 
			pkTarget->EquipAllItem();	// 현재 진짜로 장착하고 있는 아이템들을 싹 다시 붙여준다
		}
	}
}

std::string PgStatusEffectInstance::GetParam(std::string const& kKey)const
{
	CONT_PARAM::const_iterator c_it = m_kContParam.find(kKey);
	if(c_it != m_kContParam.end())
	{
		return (*c_it).second;
	}
	return "";
}

void PgStatusEffectInstance::SetParam(std::string const& kKey, std::string const& kValue)
{
	m_kContParam[kKey] = kValue;
}

bool PgStatusEffectManUtil::SEffectUpdateInfo::Update(float const fAccumTime)
{
	if(NULL==m_pkInstance || 0.0f==m_kTickTime)	{return false;}

	if(	(fAccumTime - m_kLastUpdateTime) >= m_kTickTime )
	{
		m_kLastUpdateTime = fAccumTime;
		return true;
	}

	return false;
}

void PgStatusEffectInstance::AttachWhiteOut(PgPilot* pkTargetPilot)
{
	if(!(PgStatusEffect::IT_WHITE_OUT & m_pkStatusEffect->GetValidItems()))
	{
		return;
	}

	if(false == g_kPilotMan.IsMyPlayer(pkTargetPilot->GetGuid()))
	{
		return;
	}
	
	NiRenderer *pRenderer = NiRenderer::GetRenderer();
	int const kScreenWidth = pRenderer->GetDefaultRenderTargetGroup()->GetWidth(0);
	int const kScreenHeight = pRenderer->GetDefaultRenderTargetGroup()->GetHeight(0);
	int const kScreenWidthHalf = kScreenWidth/2;
	int const kScreenHeightHalf = kScreenHeight/2;

	PgStatusEffect::TEXTURE_PATH_LIST const& rkCont = m_pkStatusEffect->GetContWhiteOutTexture();
	size_t kTotal = rkCont.size();

	int kCount = 0;
	int kPosX = 0;
	int kPosY = 0;
	float kRelativeX = 0.0f;
	float kRelativeY = 0.0f;
	POINT3 kScreenCenter(kScreenWidthHalf, kScreenHeightHalf, 0.0f);
	POINT3 kSpot(0.0f, 0.0f, 0.0f);
	
	PgStatusEffect::CONT_ADD_ABIL const &kContAbil = m_pkStatusEffect->GetAddAbil();
	PgStatusEffect::CONT_ADD_ABIL::const_iterator it = kContAbil.find(AT_SCREEN_EFFECT_COUNT);
	int kBaseCount = 230;
	if(kContAbil.end() != it)
	{
		kBaseCount = (*it).second;
	}

	int kMax = kBaseCount+((kScreenWidth-1024) + (kScreenHeight - 768))/3;//해상도 클수록 많이 찍자
	while(kCount<kMax)
	{
		kSpot.x = BM::Rand_Index(kScreenWidth);
		kSpot.y = BM::Rand_Index(kScreenHeight);
		
		if(180.0f>POINT3::Distance(kScreenCenter, kSpot))
		{
			continue;
		}

		kRelativeX = kSpot.x/static_cast<float>(kScreenWidth);
		kRelativeY = kSpot.y/static_cast<float>(kScreenHeight);

		g_kScreenEffect.AddNewScreenEffect(rkCont[kCount%kTotal], kRelativeX, kRelativeY, 
			GetEffectKey(), 100000);

		++kCount;
	}
}

void PgStatusEffectInstance::DetachWhiteOut(PgPilot* pkTargetPilot)
{
	if(!(PgStatusEffect::IT_WHITE_OUT & m_pkStatusEffect->GetValidItems()))
	{
		return;
	}

	if(false == g_kPilotMan.IsMyPlayer(pkTargetPilot->GetGuid()))
	{
		return;
	}

	g_kScreenEffect.DeleteEffectByNo(GetEffectKey());
}

float PgStatusEffectInstance::StatusEffectRangeScaleByAbil(float fSize)const
{
	if(NULL == GetStatusEffect())
	{
		return fSize;
	}
	if(!(GetStatusEffect()->GetAttackScaleByAbil()))
	{
		return fSize;
	}

	GET_DEF(CEffectDefMgr, kEffectDefMgr);
	CEffectDef const* pkDef = kEffectDefMgr.GetDef(GetEffectID());
	if(NULL == pkDef)
	{
		return fSize;
	}

	float const OriginalAttackRange = pkDef->GetAbil(AT_ATTACK_RANGE);
	float AttackScaleAbil = GetStatusEffect()->GetAttackScaleByAbil();
	float fscale = OriginalAttackRange * AttackScaleAbil;

	if(fscale != 0)
	{	
		fSize = (fSize * fscale);
		return fSize;
	}

	return fSize;
}
