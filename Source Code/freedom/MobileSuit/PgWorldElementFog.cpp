#include "stdafx.H"
#include "PgWorldElementFog.H"
#include "PgMobileSuit.h"

PgWorldElementFog::PgWorldElementFog()
:m_fDepth(0),m_fFarDistance(0),m_fDensity(0),m_fStartDistance(0),m_fEndDistance(0),
m_kFogFunction(NiFogProperty::FOG_Z_LINEAR),m_fAlpha(1),m_bApplyToSkyBox(false),
m_bUseFogEndFar(false),m_dwValidAttribute(0)
{
	m_kTimeCondition.SetConditionAttributeName("CONDITION_TIME_FROM","CONDITION_TIME_TO");
}
bool	PgWorldElementFog::ReadFromXmlElement(TiXmlNode const *pkFogNode)
{
	assert(pkFogNode);
	if(!pkFogNode)
	{
		return false;
	}

	TiXmlElement *pkElement = (TiXmlElement *)pkFogNode;
	assert(pkElement);
	if(!pkElement)
	{
		return	false;
	}

	if(true == m_kTimeCondition.CheckTheElementHasTimeConditionAttribute(pkElement))
	{
		if(true == m_kTimeCondition.ReadFromXmlElement(pkElement))
		{
			SetAttributeValid(AT_CONDITION);
		}
	}

	TiXmlAttribute *pkAttr = pkElement->FirstAttribute();
	while(pkAttr)
	{
		std::string pcAttrName(pkAttr->Name());
		std::string pcAttrValue(pkAttr->Value());

		if(pcAttrName == "DEPTH")
		{
			SetAttributeValid(AT_DEPTH);
			m_fDepth = static_cast<float>(atof(pcAttrValue.c_str()));
		}
		else if(pcAttrName=="FAR")
		{
			SetAttributeValid(AT_FAR_DISTANCE);
			m_fFarDistance = static_cast<float>(atof(pcAttrValue.c_str()));
		}
		if(pcAttrName=="DENSITY")
		{
			SetAttributeValid(AT_DENSITY);
			m_fDensity = static_cast<float>(atof(pcAttrValue.c_str()));
		}
		else if(pcAttrName=="START")
		{
			SetAttributeValid(AT_START_DISTANCE);
			m_fStartDistance = static_cast<float>(atof(pcAttrValue.c_str()));
		}
		else if(pcAttrName=="END")
		{
			SetAttributeValid(AT_END_DISTANCE);
			m_fEndDistance = static_cast<float>(atof(pcAttrValue.c_str()));
		}
		else if (pcAttrName == "FUNC")
		{
			SetAttributeValid(AT_FUNC);
			if (pcAttrValue == "RANGE_SQ")
			{
				m_kFogFunction = NiFogProperty::FOG_RANGE_SQ;
			}
			else
			{
				m_kFogFunction = NiFogProperty::FOG_Z_LINEAR;
			}
		}
		else if (pcAttrName == "COLOR")
		{
			SetAttributeValid(AT_COLOR);
			float fR, fG, fB;
			sscanf_s(pcAttrValue.c_str(), "%f, %f, %f", &fR, &fG, &fB);
			m_kColor = NiColor(fR, fG, fB);
		}
		else if (pcAttrName == "ALPHA")
		{
			SetAttributeValid(AT_ALPHA);
			m_fAlpha = (float)atof(pcAttrValue.c_str());
		}
		else if (pcAttrName == "SKYBOX")
		{
			SetAttributeValid(AT_APPLY_TO_SKYBOX);
			m_bApplyToSkyBox = (0 != atoi(pcAttrValue.c_str()));
		}
		else if ( pcAttrName == "FOGEND")
		{
			SetAttributeValid(AT_USE_FOG_ENDFAR);
			m_bUseFogEndFar = (0 != atoi(pcAttrValue.c_str()));
		}
		else if( pcAttrName == "ID")
		{
			m_ID = pcAttrValue;
		}

		pkAttr = pkAttr->Next();
	}		
	return	true;
}
bool	PgWorldElementFog::IsConditionCorrect()	const
{
	if(!IsTheAttributeValid(AT_CONDITION))
	{
		return	true;
	}

	SYSTEMTIME	kNow;
	g_kEventView.GetLocalTime(&kNow);
#ifndef EXTERNAL_RELEASE
	if(g_pkApp->IsSingleMode())
	{
		::GetSystemTime(&kNow);
	}
#endif

	return	m_kTimeCondition.CheckTimeIsInsideDuration(kNow);
}