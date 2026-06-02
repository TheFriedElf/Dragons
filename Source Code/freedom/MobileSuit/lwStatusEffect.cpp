#include "stdafx.h"
#include "lwStatusEffect.H"
#include "PgPilot.H"
#include "PgPilotMan.H"
#include "lwUI.h"

using namespace lua_tinker;

lwStatusEffectMan	lwGetStatusEffectMan()
{
	return	lwStatusEffectMan(&g_kStatusEffectMan);
}

void lwStatusEffectMan::RegisterWrapper(lua_State *pkState)
{

	def(pkState, "GetStatusEffectMan", &lwGetStatusEffectMan);

	LW_REG_CLASS(StatusEffectMan)
		LW_REG_METHOD(StatusEffectMan, AddStatusEffect)
		LW_REG_METHOD(StatusEffectMan, RemoveStatusEffect)
		LW_REG_METHOD(StatusEffectMan, AddStatusEffectToActor)
		LW_REG_METHOD(StatusEffectMan, RemoveStatusEffectFromActor)
		LW_REG_METHOD(StatusEffectMan, RemoveStatusEffectFromActor2)
		LW_REG_METHOD(StatusEffectMan, RemoveAllStatusEffect)
		;
}

void lwStatusEffectMan::AddStatusEffect(lwGUID	kPilotGUID,lwGUID	kCasterPilotGUID,int iActionInstanceID, int iEffectID,int iValue)
{
	_PgOutputDebugString("lwStatusEffectMan::AddStatusEffect PilotGUID : %s EffectID : %d Value : %d\n",kPilotGUID.GetString(),iEffectID,iValue);
	PgPilot	*pkPilot = g_kPilotMan.FindPilot(kPilotGUID());
	if( pkPilot )
	{
		PgStatusEffectManUtil::AddEffect(*m_pkStatusEffectMan, pkPilot, kCasterPilotGUID(), iEffectID, iActionInstanceID, iValue);
	}
}

void lwStatusEffectMan::RemoveStatusEffect(lwGUID	kPilotGUID,int iEffectID)
{
	PgPilot	*pkPilot = g_kPilotMan.FindPilot(kPilotGUID());
	if( pkPilot )
	{
		PgStatusEffectManUtil::DelEffect(*m_pkStatusEffectMan, pkPilot, iEffectID);
	}
}

void lwStatusEffectMan::RemoveAllStatusEffect(lwGUID kPilotGUID)
{
	PgPilot	*pkPilot = g_kPilotMan.FindPilot(kPilotGUID());
	if( pkPilot )
	{
		m_pkStatusEffectMan->RemoveAllStatusEffect(pkPilot);
	}
}


int	lwStatusEffectMan::AddStatusEffectToActor(lwGUID	kPilotGUID,char *strEffectXMLID,int iEffectID,int iEFfectKey,int iValue)
{
	PgPilot	*pkPilot = g_kPilotMan.FindPilot(kPilotGUID());
	if(!pkPilot) return -1;

	m_pkStatusEffectMan->RemoveStatusEffectFromActor(pkPilot,iEffectID);	//	기존 것은 지운다.

	return	m_pkStatusEffectMan->AddStatusEffectToActor(pkPilot, pkPilot, std::string(strEffectXMLID),
		iEffectID,iEFfectKey,iValue,true,false);

}
void	lwStatusEffectMan::RemoveStatusEffectFromActor(lwGUID	kPilotGUID,int iEffectID)
{
	PgPilot	*pkPilot = g_kPilotMan.FindPilot(kPilotGUID());
	if(!pkPilot) return;

	m_pkStatusEffectMan->RemoveStatusEffectFromActor(pkPilot,iEffectID);

}
void	lwStatusEffectMan::RemoveStatusEffectFromActor2(lwGUID	kPilotGUID,int iInstanceID)
{
	PgPilot	*pkPilot = g_kPilotMan.FindPilot(kPilotGUID());
	if(!pkPilot) return;

	m_pkStatusEffectMan->RemoveStatusEffectFromActor2(pkPilot,iInstanceID);

}