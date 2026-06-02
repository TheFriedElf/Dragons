#ifndef FREEDOM_DRAGONICA_RENDER_WORDOBJECT_ACTOR_PGACTORUTIL_H
#define FREEDOM_DRAGONICA_RENDER_WORDOBJECT_ACTOR_PGACTORUTIL_H

namespace PgActorUtil
{
	float const fMinimumAddedZ = 7.f;
	float const fAddedGuildNameZ = 6.f;
	float const fAddedGuildMarkZ = 2.f;
	float const fNameWidthGap = 2.f;

	float FindNamePosZ(PgActor const* pkActor);

	inline void SetNameCulled(PgTextObjectPtr spkName, PgTextObjectPtr spkGuildName, PgGuildMarkPtr spGuildMark, PgGuardianMarkPtr spGuardianMark, PgTextObjectPtr spkTitleName, 
		PgAchievementTitlePtr spkAchievementTitle, PgAchievementTitlePtr spkGIFTitle, PgMyhomeMarkPtr spkMyhomeMark, PgTextObjectPtr spCustomCount, PgDuelTitlePtr spDuelTitle, PgEffectCountDownPtr spEffectCountDown, bool const bSetCulled)
	{
		if( spkName )
		{
			spkName->SetAppCulled(bSetCulled);
		}
		if( spkGuildName )
		{
			spkGuildName->SetAppCulled(bSetCulled);
		}
		if( spGuildMark )
		{
			spGuildMark->SetAppCulled(bSetCulled);
		}
		if( spGuardianMark )
		{
			spGuardianMark->SetAppCulled(bSetCulled);
		}
		if( spkTitleName )
		{
			spkTitleName->SetAppCulled(bSetCulled);
		}
		if( spkAchievementTitle )
		{
			spkAchievementTitle->SetAppCulled(bSetCulled);
		}
		if( spkGIFTitle )
		{
			spkGIFTitle->SetAppCulled(bSetCulled);
		}
		if( spkMyhomeMark )
		{
			spkMyhomeMark->SetAppCulled(bSetCulled);
		}
		if( spCustomCount )
		{
			spCustomCount->SetAppCulled(bSetCulled);
		}
		if( spDuelTitle )
		{
			spDuelTitle->SetAppCulled(bSetCulled);
		}
		if( spEffectCountDown )
		{
			spEffectCountDown->SetAppCulled(bSetCulled);
		}
	}

	template< typename _T >
	void DetachFromNode(PgActor* pkActor, char const* const szFromObjectName, NiPointer< _T > &rkNiPointer)
	{
		if( !pkActor )
		{
			return;
		}

		NiNode* pkTargetNode = NiDynamicCast(NiNode, pkActor->GetObjectByName(szFromObjectName));
		if( !pkTargetNode )
		{
			return;
		}
		DetachFromNode(pkTargetNode, rkNiPointer);
	}

	template< typename _T >
	void DetachFromNode(NiNode* pkFromNode, NiPointer< _T > &rkNiPointer)
	{
		if( !pkFromNode )
		{
			return;
		}

		if( 0 != rkNiPointer )
		{
			pkFromNode->DetachChild(rkNiPointer);
		}
	}
	template< typename _T >
	void AttachToNode(NiNode* pkToNode, NiPointer< _T > &rkNiPointer)
	{
		if( !pkToNode )
		{
			return;
		}

		if( 0 != rkNiPointer )
		{
			pkToNode->AttachChild(rkNiPointer, true);
		}
	}

	bool GetEquipAchievementItem(PgActor* pkActor, PgBase_Item& kAchievementsItem);
	int GetAchievementsTitleNo(PgBase_Item const& kAchievementsItem);

	bool UpdateColorShadow( PgIWorldObject &rkObject, PgWorld * const pkWorld, bool const bUpdate, bool bIsClassRank4 );
	bool UpdateColorShadow(PgActor* pkActor, bool const bUpdate);
	bool IsCanPlaySound(PgActor* pkActor);
};

#endif // FREEDOM_DRAGONICA_RENDER_WORDOBJECT_ACTOR_PGACTORUTIL_H