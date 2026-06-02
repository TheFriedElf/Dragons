#include "stdafx.h"
#include "PgActorAppearanceMan.H"
#include "PgActor.H"
#include "PgPilot.H"
#include "PgAction.h"
#include "lwActor.h"

PgActorAppearanceMan::PgActorAppearanceMan(PgActor *pkActor):
m_pkActor(pkActor),
m_iLastUpdatedHP(-1),
m_iLastUpdatedCondition(-1)
{
}
PgActorAppearanceMan::~PgActorAppearanceMan()
{
}

bool PgActorAppearanceMan::ParseXml(const TiXmlNode *pkNode)
{
	int const iType = pkNode->Type();
	
	switch(iType)
	{
	case TiXmlNode::ELEMENT:
		{
			TiXmlElement *pkElement = (TiXmlElement *)pkNode;
			PG_ASSERT_LOG(pkElement);
			
			char const *pcTagName = pkElement->Value();

			if(stricmp(pcTagName, "APPEARANCE_CHANGE") == 0)
			{
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
			}
			else if(stricmp(pcTagName, "CONDITION") == 0)
			{
				int	iHPMin=0,iHPMax=0;
				TiXmlAttribute *pkAttr = pkElement->FirstAttribute();
				while(pkAttr)
				{
					char const *pcName = pkAttr->Name();
					char const *pcValue = pkAttr->Value();
					if (strcmp(pcName, "HP_MIN") == 0)
					{
						iHPMin = atoi(pcValue);
					}
					else if(strcmp(pcName, "HP_MAX") == 0)
					{
						iHPMax = atoi(pcValue);
					}
					else
					{
						PG_ASSERT_LOG(!"Unknown Attributes!");
					}
					pkAttr = pkAttr->Next();
				}

				stConditionInfo	*pkNewCondInfo = new stConditionInfo();
				pkNewCondInfo->m_iHPMax = iHPMax;
				pkNewCondInfo->m_iHPMin = iHPMin;

				m_vConditionInfos.push_back(pkNewCondInfo);
				const TiXmlNode * pkChildNode = pkNode->FirstChild();
				if(pkChildNode != 0)
				{
					if(!ParseChangeInfo(pkChildNode,pkNewCondInfo))
					{
						return false;
					}
				}
			}
		};
	};
	// 같은 층의 다음 노드를 재귀적으로 파싱한다.
	const TiXmlNode* pkNextNode = pkNode->NextSibling();
	if(pkNextNode)
	{
		if(!ParseXml(pkNextNode))
		{
			return false;
		}
	}

	return	true;
}
bool PgActorAppearanceMan::ParseChangeInfo(const TiXmlNode *pkNode,stConditionInfo *pkConditionInfo)
{
	int const iType = pkNode->Type();
	
	switch(iType)
	{
	case TiXmlNode::ELEMENT:
		{
			TiXmlElement *pkElement = (TiXmlElement *)pkNode;
			PG_ASSERT_LOG(pkElement);
			
			char const *pcTagName = pkElement->Value();

			if(stricmp(pcTagName, "CHANGE_ITEM") == 0)
			{
				stChangeInfo	kNewInfo;
				TiXmlAttribute *pkAttr = pkElement->FirstAttribute();
				while(pkAttr)
				{
					char const *pcName = pkAttr->Name();
					char const *pcValue = pkAttr->Value();
					if (strcmp(pcName, "TYPE") == 0)
					{
						if(stricmp(pcValue,"KFM")==0)
						{
							kNewInfo.m_kChangeType = CT_KFM;
						}
					}
					else if(strcmp(pcName, "KFMPATH") == 0)
					{
						kNewInfo.m_kKFMPath = pcValue;
					}
					else if(strcmp(pcName, "PARTICLE_ID") == 0)
					{
						kNewInfo.m_kEffectID = pcValue;
					}
					else if(strcmp(pcName, "PARTICLE_ATTACH_TARGET_NODE") == 0)
					{
						kNewInfo.m_eAttachType = stChangeInfo::EAT_NODE;
						kNewInfo.m_kEffectAttachNodeName = pcValue;
					}
					else if(strcmp(pcName, "PARTICLE_ATTACH_POS") == 0)
					{
						kNewInfo.m_eAttachType = stChangeInfo::EAT_POS;
						float fX = 0, fY = 0, fZ = 0;
						sscanf_s(pcValue,"%f,%f,%f",&fX,&fY,&fZ);
						kNewInfo.m_kEffectPos.x = fX;
						kNewInfo.m_kEffectPos.y = fY;
						kNewInfo.m_kEffectPos.z = fZ;
					}
					else
					{
						PG_ASSERT_LOG(!"Unknown Attributes!");
					}
					pkAttr = pkAttr->Next();
				}

				if(kNewInfo.m_kChangeType != CT_NONE)
				{
					pkConditionInfo->m_vChangeInfos.push_back(kNewInfo);
				}
			}
		};
	};
	// 같은 층의 다음 노드를 재귀적으로 파싱한다.
	const TiXmlNode* pkNextNode = pkNode->NextSibling();
	if(pkNextNode)
	{
		if(!ParseChangeInfo(pkNextNode, pkConditionInfo))
		{
			return false;
		}
	}

	return	true;
}

void	PgActorAppearanceMan::UpdateAppearance(bool const bOnlyChangeApperance)
{
	if(!m_pkActor)
	{
		return;
	}

	PgPilot	*pkPilot = m_pkActor->GetPilot();
	if(!pkPilot)
	{
		return;
	}

	int	iHP = pkPilot->GetAbil(AT_HP);
	if(iHP == m_iLastUpdatedHP)
	{
		return;
	}

	int	iMaxHP = pkPilot->GetAbil(AT_C_MAX_HP);
	if(iMaxHP == 0)
	{
		return;
	}

	int	iRate = (int)( (iHP / ((float)iMaxHP)) * 100);

	int	iTotal = m_vConditionInfos.size();
	for(int i=0;i<iTotal;i++)
	{
		stConditionInfo *pkInfo = m_vConditionInfos[i];

		if(iRate>pkInfo->m_iHPMin && iRate<=pkInfo->m_iHPMax)
		{
			if(i == m_iLastUpdatedCondition)
			{
				break;
			}

			UpdateAppearanceTo(pkInfo,bOnlyChangeApperance);

			m_iLastUpdatedCondition = i;
		}
	}

	m_iLastUpdatedHP = iHP;
}
void	PgActorAppearanceMan::UpdateAppearanceTo(stConditionInfo *pkCondition,bool const bOnlyChangeApperance)
{
	if(!pkCondition)
	{
		return;
	}

	VChangeInfo const &kChangeInfos = pkCondition->m_vChangeInfos;

	int	iTotal = kChangeInfos.size();
	for(int i=0;i<iTotal;i++)
	{
		switch(kChangeInfos[i].m_kChangeType)
		{
		case CT_KFM:
			{
				if(bOnlyChangeApperance)
				{
					m_pkActor->ReserveKFMTransit(kChangeInfos[i].m_kKFMPath.c_str(),true);

					if(m_pkActor->GetAction())
					{
						int	iActionNo = m_pkActor->GetAction()->GetActionNo();
						m_pkActor->CancelAction(m_pkActor->GetAction()->GetActionNo(),m_pkActor->GetAction()->GetActionInstanceID(),ACTIONNAME_IDLE);
						m_pkActor->ReserveTransitAction(iActionNo);
					}
					else
					{
						m_pkActor->ReserveTransitAction(ACTIONNAME_IDLE);
					}
				}
				else
				{
					PgAction	*pkNewAction = m_pkActor->ReserveTransitAction("a_AppearanceChange");
					if(pkNewAction)
					{
						pkNewAction->SetParam(0,kChangeInfos[i].m_kKFMPath.c_str());
						//KFM을 변경할때 붙여줘야할 이펙트가 있다면
						{
							lwAction kTemp(pkNewAction);
							kTemp.SetParamInt(1,static_cast<int>(kChangeInfos[i].m_eAttachType));
						}
						if(!kChangeInfos[i].m_kEffectID.empty())
						{// 붙일 파티클이 있다면 붙일 이펙트를 알려주고
							pkNewAction->SetParam(2,kChangeInfos[i].m_kEffectID.c_str());
							switch(kChangeInfos[i].m_eAttachType)
							{
							case stChangeInfo::EAT_NODE:
								{// 노드에 붙일것인지
									pkNewAction->SetParam(3,kChangeInfos[i].m_kEffectAttachNodeName.c_str());
								}break;
							case stChangeInfo::EAT_POS:
								{// 위치에 붙일것인지 알려준다
									NiPoint3 kPos = kChangeInfos[i].m_kEffectPos;
									pkNewAction->SetParamAsPoint(4, kPos);
								}break;
							}
						}
					}
				}
			}
			break;
		}
	}
}
PgActorAppearanceMan*	PgActorAppearanceMan::CreateCopy(PgActor *pkActor)
{
	PgActorAppearanceMan	*pkNew = new PgActorAppearanceMan(pkActor);

	int	iTotal = m_vConditionInfos.size();
	for(int i=0;i<iTotal;i++)
	{
		stConditionInfo *pkInfo = m_vConditionInfos[i];
		
		stConditionInfo *pkNewInfo = new stConditionInfo();

		*pkNewInfo = *pkInfo;
		pkNew->m_vConditionInfos.push_back(pkNewInfo);
	}

	return	pkNew;
}