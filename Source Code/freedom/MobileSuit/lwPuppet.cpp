#include "StdAfx.h"
#include "lwPuppet.h"
#include "PgPuppet.h"
#include "PgWEObjectState.h"
#include "PgParticle.h"
#include "PgParticleMan.h"

lwPuppet::lwPuppet(PgPuppet *pkPuppet)
{
	m_pkPuppet = pkPuppet;
}

lwPuppet::~lwPuppet(void)
{
}

lwPuppet lwToPuppet(void *pkObject)
{
	return lwPuppet((PgPuppet *)pkObject);
}


lwPuppet	lwGetPuppet(char const *pkPuppetID)
{
	if( g_pkWorld
	&&	pkPuppetID )
	{
		std::string const kName(pkPuppetID);
		NiAVObject* pkFindedNode = NULL;
		if( g_pkWorld->RunObjectGroupFunc(OGT_PUPPET, NiAVObjectUtil::FindPuppetNode(kName, pkFindedNode)) )
		{
			PgPuppet* pkPupper = NiDynamicCast(PgPuppet, pkFindedNode);
			if( pkPupper )
			{
				return lwPuppet(pkPupper);
			}
		}
	}

	return	lwPuppet(NULL);
}
bool lwPuppet::RegisterWrapper(lua_State *pkState)
{
	using namespace lua_tinker;

	def(pkState, "ToPuppet", &lwToPuppet);
	def(pkState, "GetPuppet", &lwGetPuppet);

	class_<lwPuppet>(pkState, "Puppet")
		.def(pkState, constructor<PgPuppet *>())
		.def(pkState, "IsNil", &lwPuppet::IsNil)
		.def(pkState, "TransitAction", &lwPuppet::TransitAction)
		.def(pkState, "AttachParticle", &lwPuppet::AttachParticle)
		.def(pkState, "PlayAnimation", &lwPuppet::PlayAnimation)
		.def(pkState, "GetCurAnimation", &lwPuppet::GetCurAnimation)
		.def(pkState, "ReloadNif", &lwPuppet::ReloadNif)
		.def(pkState, "GetGuid", &lwPuppet::GetGuid)
		.def(pkState, "GetWorldEventStateID", &lwPuppet::GetWorldEventStateID)
		.def(pkState, "SetWorldEventStateID", &lwPuppet::SetWorldEventStateID)
		;

	return true;
}

bool lwPuppet::PlayAnimation(int iAnimationID)
{
	return	m_pkPuppet->PlayAnimation(iAnimationID);
}
int	lwPuppet::GetWorldEventStateID()
{
	return	m_pkPuppet->GetWorldEventStateID();
}
void	lwPuppet::SetWorldEventStateID(int iNewID,bool bSetImmediate)
{
	m_pkPuppet->SetWorldEventStateID(iNewID,g_kEventView.GetGameSecTime(),bSetImmediate);
}

PgPuppet *lwPuppet::operator()()
{
	return m_pkPuppet;
}

bool lwPuppet::IsNil()
{
	return (m_pkPuppet == 0);
}

bool lwPuppet::TransitAction(char const *pcActionName)
{
	return m_pkPuppet->TransitAction(pcActionName);
}

void lwPuppet::SetGuid(lwGUID kGuid)
{
	m_pkPuppet->SetGuid(kGuid());
}

lwGUID lwPuppet::GetGuid()
{
	return lwGUID(m_pkPuppet->GetGuid());
}

bool lwPuppet::AttachParticle(int iSlot, char const *pcTarget, char const *pcParticle)
{
	PgParticle *pkParticle = g_kParticleMan.GetParticle(pcParticle);
	if(!pkParticle)
	{
		return false;
	}

	if(!m_pkPuppet->AttachTo(iSlot, pcTarget, pkParticle))
	{
		THREAD_DELETE_PARTICLE(pkParticle);
	}

	return true;
}

int lwPuppet::GetCurAnimation()
{
	return m_pkPuppet->GetCurAnimation();
}

void lwPuppet::ReloadNif()
{
	m_pkPuppet->ReloadNif();
}
