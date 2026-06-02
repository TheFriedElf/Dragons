#include "stdafx.h"
#include "Variant/PgJobSkillLocationItem.h"
#include "PgParticle.h"
#include "PgParticleMan.h"
#include "PgActor.h"
#include "PgActorUtil.h"
#include "PgPilot.h"
#include "PgHelpSystem.H"
#include "PgTrigger.h"
#include "PgJobSkillLocationInfo.h"
#include "PgPilotMan.h"

namespace PgActorUtil
{
	typedef struct tagFindAchivementsItem
	{
	private:
		typedef CONT_DEF_ACHIEVEMENTS::_Mybase::value_type pair_type;
	public:
		inline tagFindAchivementsItem(int const iItemNo)
			:m_iFindItemNo(iItemNo)
		{
		}

		inline tagFindAchivementsItem(tagFindAchivementsItem const& rhs)
			:m_iFindItemNo(rhs.m_iFindItemNo)
		{
		}

		bool operator () (pair_type const& rhs) const
		{
			return rhs.second.iItemNo == m_iFindItemNo;
		}

	private:
		int const m_iFindItemNo;
	} SFindAchivementsItem;

	float FindNamePosZ(PgActor const* pkActor)
	{
		if( !pkActor )
		{
			return fMinimumAddedZ;
		}

		PgActor::PartsAttachInfo const &rkPartsAttachInfo = pkActor->GetPartsAttachInfo();
		PgActor::PartsAttachInfo::const_iterator iter = rkPartsAttachInfo.find(EQUIP_LIMIT_HELMET);
		if( rkPartsAttachInfo.end() != iter )
		{
			NiNodePtr spRootNode = (*iter).second->GetMeshRoot();
			if( spRootNode )
			{
				float const fWorldScale = spRootNode->GetWorldScale();
				float const fHelmetRadius = spRootNode->GetWorldBound().GetRadius();
				float const fDivideValue = 0.63f;
				return ::NiMax(fMinimumAddedZ, fHelmetRadius*fWorldScale*fDivideValue);
			}
		}
		return fMinimumAddedZ * pkActor->GetWorldScale();
	}

	bool GetEquipAchievementItem(PgActor* pkActor, PgBase_Item& kAchievementsItem)
	{
		if( !pkActor )
		{
			return 0;
		}

		PgPilot* pkPilot = pkActor->GetPilot();
		if( !pkPilot )
		{
			return 0;
		}

		PgPlayer* pkPlayer = dynamic_cast< PgPlayer* >( pkPilot->GetUnit() );
		if( !pkPlayer )
		{
			return 0;
		}

		PgInventory* pkInven = pkPlayer->GetInven();
		if( !pkInven )
		{
			return 0;
		}

		return (S_OK == pkInven->GetItem(SItemPos(IT_FIT, EQUIP_POS_MEDAL), kAchievementsItem));
	}

	int GetAchievementsTitleNo(PgBase_Item const& kAchievementsItem)
	{
		CONT_DEF_ACHIEVEMENTS const* pkDef = NULL;
		g_kTblDataMgr.GetContDef(pkDef);
		if( !pkDef )
		{
			return 0;
		}

		CONT_DEF_ACHIEVEMENTS::const_iterator find_iter = std::find_if(pkDef->begin(), pkDef->end(), SFindAchivementsItem(kAchievementsItem.ItemNo()));
		if( pkDef->end() == find_iter )
		{
			return 0;
		}
		CONT_DEF_ACHIEVEMENTS::mapped_type const& rkAchievementsDef = (*find_iter).second;
		return rkAchievementsDef.iTitleNo;
	}

	bool UpdateColorShadow( PgIWorldObject &rkObject, PgWorld * const pkWorld, bool const bUpdate, bool bIsClassRank4 )
	{
		static std::string const strParticleID[4]	=	{	"e_ef_colorshadow_battler"
														,	"e_ef_colorshadow_magician"
														,	"e_ef_colorshadow_archer"
														,	"e_ef_colorshadow_thief"
														};

		static std::string const strPvPParticle[2]	=	{	"e_ef_colorshadow_pvp_red"
														,	"e_ef_colorshadow_pvp_blue"
														};

		static std::string const strEmBattleParticle[2]	=	{	"e_ef_colorshadow_embattle_red"
															,	"e_ef_colorshadow_embattle_blue"
															};

		static std::string const strClassRank4ParticleID = "e_ef_colorshadow_classRank4";
		
		static const int iSlotNo = 9812341;
		if(!g_pkWorld)
		{
			return false;
		}
		if ( !pkWorld )
		{
			PG_ASSERT_LOG(NULL);
			return false;
		}

		PgPilot *pkPilot = rkObject.GetPilot();
		if ( !pkPilot )
		{
			PG_ASSERT_LOG(NULL);
			return false;
		}

		if ( true == bUpdate )
		{
			rkObject.DetachFrom(iSlotNo);
			rkObject.DetachFrom(iSlotNo + 1);
		}

		if ( true == g_bUseColorShadow )
		{
			NiAVObject *pkParticle = NULL;
			NiAVObject *pkClassRankParticle = NULL;

			switch ( g_pkWorld->GetAttr() )
			{
			case GATTR_PVP:
				{
					int const iTeam = ( pkPilot->GetAbil( AT_TEAM ) - 1 );
					if ( (0 <= iTeam) && (2 > iTeam) )
					{
						pkParticle = dynamic_cast<NiAVObject*>( g_kParticleMan.GetParticle( strPvPParticle[iTeam].c_str(), PgParticle::O_SCALE,rkObject.GetEffectScale() ) );
					}
				}break;
			case GATTR_EMPORIABATTLE:
				{
					PgActor* pkActor = dynamic_cast<PgActor*>(pkPilot->GetWorldObject());
					if(pkActor && pkActor->IsUnderMyControl())
					{						
						int const iTeam = ( pkPilot->GetAbil( AT_TEAM ) - 1 );
						if ( (0 <= iTeam) && (2 > iTeam) )
						{
							pkParticle = dynamic_cast<NiAVObject*>( g_kParticleMan.GetParticle( strEmBattleParticle[iTeam].c_str(),PgParticle::O_SCALE, rkObject.GetEffectScale() ) );
						}
					}
				}break;
			default:
				{
					if ( g_kHelpSystem.HelperGuid() != rkObject.GetGuid() )
					{
						int const iBaseClassID = pkPilot->GetBaseClassID();
						if ( iBaseClassID < UCLASS_FIGHTER )
						{
							PG_ASSERT_LOG(NULL);
							return false;
						}
						
						std::string strApplyParticleID;
						switch(iBaseClassID)
						{
						case UCLASS_FIGHTER:
						case UCLASS_MAGICIAN:
						case UCLASS_ARCHER:
						case UCLASS_THIEF:
							{
								strApplyParticleID = strParticleID[iBaseClassID-1];
							}break;
						case UCLASS_SHAMAN:
							{
								strApplyParticleID = "eff_colorshadow_summons_01";
							}break;
						case UCLASS_DOUBLE_FIGHTER:
							{
								strApplyParticleID = "eff_colorshadow_twin_01";
							}break;
						default:
							{
								return false;
							}break;
						}
						
						PgActor* pkActor = dynamic_cast<PgActor*>(pkPilot->GetWorldObject());
						
						if(pkActor
							&& g_kPilotMan.IsMyPlayer(pkPilot->GetGuid())
							)
						{
							PgTrigger* pkCurTrigger =  pkActor->GetCurrentTrigger();
							EGatherType eTriggerGatherType = EGatherType::GT_None;
							if(pkCurTrigger
								&& pkCurTrigger->GetTriggerType() == PgTrigger::TRIGGER_TYPE_JOB_SKILL)
							{// 현재 채집 트리거 위에 있다면, 해당 트리거의 타입을 얻어온다
								g_kJobSkillLocationInfo.GetGatherType(pkCurTrigger->GetID(), eTriggerGatherType);
								bool const bEnable = g_kJobSkillLocationInfo.IsEnable( pkCurTrigger->GetID() );
								switch(eTriggerGatherType)
								{
								case GT_WoodMachine:
									{// 벌목류
										if(bEnable)
										{
											strApplyParticleID = "eff_life_axe01";
										}
										else
										{
											strApplyParticleID = "eff_life_axe00";
										}
									}break;
								case GT_Jewelry:
								case GT_Smelting:
									{// 채광류
										if(bEnable)
										{
											strApplyParticleID = "eff_life_pick01";
										}
										else
										{
											strApplyParticleID = "eff_life_pick00";
										}
									}break;
								case GT_Garden:
									{// 밭류
										if(bEnable)
										{
											strApplyParticleID = "eff_life_hoe01";
										}
										else
										{
											strApplyParticleID = "eff_life_hoe00";
										}
									}break;
								case GT_Fishbowl:
									{// 낚시류
										if(bEnable)
										{
											strApplyParticleID = "eff_life_fishing01";
										}
										else
										{
											strApplyParticleID = "eff_life_fishing00";
										}
									}break;
								default:
									{
									}break;
								}
							}
						}

						pkParticle = dynamic_cast<NiAVObject*>( g_kParticleMan.GetParticle( strApplyParticleID.c_str(), PgParticle::O_SCALE,rkObject.GetEffectScale() ) );
						if(true == bIsClassRank4)
						{
							pkClassRankParticle = dynamic_cast<NiAVObject*>( g_kParticleMan.GetParticle( strClassRank4ParticleID.c_str(), PgParticle::O_SCALE,rkObject.GetEffectScale() ) );
						}
					}
				}break;
			}

			if ( pkParticle )
			{
				if( false == rkObject.AttachTo( iSlotNo, "char_root", pkParticle ) )
				{
					THREAD_DELETE_PARTICLE(pkParticle);
				}
			}
			if( pkClassRankParticle )
			{
				if( false == rkObject.AttachTo( iSlotNo + 1, "char_root", pkClassRankParticle ) )
				{
					THREAD_DELETE_PARTICLE(pkClassRankParticle);
				}
			}

			return false;
		}
		return true;
	}
	
	bool UpdateColorShadow(PgActor* pkActor, bool const bUpdate)
	{
		if(!pkActor)
		{
			return false;
		}
		PgPilot* pkPilot = pkActor->GetPilot();
		if(!pkPilot)
		{
			return false;
		}
		
		bool bIsClassRank4 = false;
		switch(pkPilot->GetAbil(AT_CLASS) )
		{
		case UCLASS_DRAOON:
		case UCLASS_DESTROYER:
		case UCLASS_ARCHMAGE:
		case UCLASS_WARLORD:
		case UCLASS_SENTINEL:
		case UCLASS_LAUNCHER:
		case UCLASS_MANIAC:
		case UCLASS_SHADOW:
			{
				bIsClassRank4 = true;
			}break;
		}

		PgIWorldObject *pkWorldObject = dynamic_cast<PgIWorldObject*>(pkActor);
		if ( pkWorldObject )
		{
			UpdateColorShadow( *pkWorldObject, g_pkWorld, bUpdate, bIsClassRank4 );
			return true;
		}
		return false;
	}

	bool IsCanPlaySound(PgActor* pkActor)
	{
		PgPilot* pkPilot = pkActor->GetPilot();
		if( pkPilot )
		{
			CUnit* pkUnit = pkPilot->GetUnit();
			if( pkUnit->IsUnitType(UT_NPC)
			&&	pkActor->IsHide() )
			{
				// NPC이고, 숨겨진 상태면 사운드 출력 없다
				return false;
			}
		}
		return true;
	}
};
