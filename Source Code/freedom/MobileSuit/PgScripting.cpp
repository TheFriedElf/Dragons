#include "StdAfx.h"
#include "Variant/PgStringUtil.h"
#include "Variant/PgBattleSquare.h"
#include "PgScripting.h"

//// Wrappers
//
#include "lwBase.h"
#include "lwPoint2.h"
#include "lwPoint2F.h"
#include "lwPoint3.h"
#include "lwQuaternion.h"
#include "lwGUID.h"
#include "lwWorld.h"
#include "lwUI.h"
#include "lwUIQuest.h"
#include "lwActor.h"
#include "lwPilot.h"
#include "lwAction.h"
#include "lwTrigger.h"
#include "lwRenderMan.h"
#include "lwPilotMan.h"
#include "lwNet.h"
#include "lwPacket.h"
#include "lwParticle.h"
#include "lwParticleMan.h"
#include "lwDropBox.h"
#include "PgClientParty.h"
#include "PgClientExpedition.h"
#include "PgMissionComplete.h"
#include "lwWorldActionMan.H"
#include "lwWorldActionObject.H"
#include "lwPuppet.h"
#include "lwInt64.h"
#include "PgUISound.h"
#include "PgQuestMan.h"
#include "lwQuestMan.H"
#include "lwHome.h"
#include "lwFurniture.h"
#include "lwProjectileMan.H"
#include "lwProjectile.H"
#include "lwBaseItemSet.h"
#include "PgMobileSuit.h"
#include "lwSkillTree.H"	
#include "lwSelectStage.h"
#include "lwUIItemPlusUpgrade.h"
#include "lwUIItemRarityUpgrade.h"
#include "lwUIItemSocketSystem.h"
#include "lwUIItemJewelBox.h"
#include "lwUIItemGamble.h"
#include "lwUIItemConvertSystem.h"
#include "lwUIItemRepair.h"
#include "lwUIShineStone.h"
#include "lwUIItemMaking.h"
#include "lwUIItemCooking.h"
#include "lwUIItemLottery.h"
#include "lwUILogin.h"
#include "lwChatMgrClient.h"
#include "lwActionResult.H"
#include "lwEffectDef.H"
#include "lwSkillDef.H"
#include "lwSkillSet.H"
#include "lwEffect.H"
#include "lwFriendItem.h"
#include "lwFriendMgr.h"
#include "lwActionTargetInfo.H"
#include "lwActionTargetList.H"
#include "lwFindTargetParam.H"
#include "lwUNIT_PTR_ARRAY.h"
#include "lwUnit.h"
#include "lwStatusEffect.H"
#include "lwActionTargetTransferInfoMan.H"
#include "lwComboAdvisor.H"
#include "lwUIPetition.h"
#include "lwLinkedPlane.H"
#include "lwEventTimer.h"
#include "lwInputSlotInfo.h"
#include "PgMovieMgr.h"
#include "lwPlayTime.h"
#include "lwObject.H"
#include "lwHelpSystem.H"
#include "lwHelpObject.H"
#include "lwGuildUI.h"
#include "PgContentsBase.h"
#include "lwCouple.h"
#include "lwMarket.h"
#include "lwDateTime.h"
#include "PgNiFile.h"
#include "lwRope.H"
#include "lwEventScriptCmd.H"
#include "lwEventScriptSystem.H"
#include "lwCameraModeEvent.H"
#include "./Ranking/PgRankingMgr.h"
#include "PgSystemInventory.h"
#include "PgSafeFoamMgr.h"
#include "lwCashShop.H"
#include "PgTextDialogMng.h"
#include "lwUIBook.H"
#include "lwAttackEffect.h"
#include "lwTeleCardUI.h"
#include "lwEventView.h"
#include "PgBriefingLoadUIMgr.h"
#include "PgMyActorViewMgr.h"
#include "PgDailyQuestUI.h"
#include "PgAchieveNfyMgr.h"
#include "lwUICharInfo.h"
#include "lwOXQuizUI.h"
#include "lwStyleString.h"
#include "lwSoundMan.h"
#include "PgBalloonEmoticonMgr.h"
#include "lwCharacterCard.h"
#include "lwUIItemBind.h"
#include "lwUIMemTransCard.h"
#include "lwCashItem.h"
#include "PgOption.h"
#include "lwQuestNfy.h"
#include "lwUIPet.h"
#include "PgEmporiaAdministrator.h"
#include "PgBattleSquare.h"
#include "PgItemMix_Script.h"
#include "lwUIGamePad.h"
#include "lwHomeTown.h"
#include "lwUIFireLove.h"
#include "lwUIIGGacha.h"
#include "lwUIActiveStatus.h"
#include "lwTransTower.h"
#include "lwUICSGacha.h"
#include "lwUIItemEventBox.h"
#include "PgHomeRenew.h"
#include "lwUIItemRarityBuildUp.h"
#include "lwUIItemRarityAmplify.h"
#include "PgLegendItemMakeHelpMng.h"
#include "lwUICostumeMix.h"
#include "lwVendor.h"
#include "lwInventoryUI.h"
#include "lwJobSkillLearn.h"
#include "lwJobSkillView.h"
#include "lwJobSkillItem.h"
#include "lwDeathSnatchSkillScriptHelpFunc.h"
#include "lwCommonSkillUtilFunc.h"

#include "PgSuperGroundUI.h"
#include "PgRagnarokGroundUI.h"
//#ifndef USE_INB
#include "lwSkillTargetMan.H"
#include "lwZoneDrawing.h"
#include "lwHomeUI_Script.h"
#include "lwMinimap_Script.h"
#include "PgEventQuestUI.h"
#include "PgBattleSquare.h"
#include "lwLuckyStarUI.h"
#include "HardCoreDungeon.h"
//#endif//USE_INB
#include "PgTextBalloon.h"
#include "lwItemSkillView.h"
#include "lwUITreasureChest.h"
#include "lwSoulTransfer.h"
#include "lwSummon_Info.h"
#include "lwDefenceMode.h"
#include "lwElementDungeonUI.h"
#include "lwUIManufacture.h"
#include "PgCommunityEvent.h"
#include "PgConstellation.h"

using namespace lua_tinker;

#ifdef EXTERNAL_RELEASE
bool PgScripting::m_bDoFileOptimize = true;
#else
bool PgScripting::m_bDoFileOptimize = false;
#endif

PgStatGroup PgScripting::m_kDoBufferStatGroup("PgScripting");
CPgLuaDebugger *g_pkScriptDebug = NULL;
DWORD PgScripting::m_hMainThread = 0;
FrameStat PgScripting::m_kDoBufferStat;
StatInfoF PgScripting::m_kDoStringStat;
PgScripting::ScriptStatContainer PgScripting::m_kScriptStatContainer;
PgScripting::ScriptDoFileQueue PgScripting::m_kScriptDoFileQueue;
Loki::Mutex PgScripting::m_kDoFileQueueLock;

PgScripting::PgScripting()
{
	g_pkScriptDebug = 0;
}

PgScripting::~PgScripting()
{
}

bool PgScripting::RegisterModules()
{
	lua_wrapper_user kLua(g_kLuaTinker);
	lwBase::RegisterWrapper(*kLua);
	lwPoint2::RegisterWrapper(*kLua);
	lwPoint2F::RegisterWrapper(*kLua);
	lwPoint3::RegisterWrapper(*kLua);
	lwQuaternion::RegisterWrapper(*kLua);
	lwGUID::RegisterWrapper(*kLua);
	lwWorld::RegisterWrapper(*kLua);
	lwActor::RegisterWrapper(*kLua);
	lwPilot::RegisterWrapper(*kLua);
	lwAction::RegisterWrapper(*kLua);
	lwTrigger::RegisterWrapper(*kLua);
	lwRenderMan::RegisterWrapper(*kLua);
	lwPilotMan::RegisterWrapper(*kLua);
	lwUIWnd::RegisterWrapper(*kLua);
	lwNet::RegisterWrapper(*kLua);
	lwPacket::RegisterWrapper(*kLua);
	lwParticleMan::RegisterWrapper(*kLua);
	lwParticle::RegisterWrapper(*kLua);
	lwDropBox::RegisterWrapper(*kLua);
	PgClientParty::RegisterWrapper(*kLua);
	PgClientExpedition::RegisterWrapper(*kLua);
	PgExpeditionComplete::RegisterWrapper(*kLua);
	lwWString::RegisterWrapper(*kLua);
	lwWorldActionMan::RegisterWrapper(*kLua);
	lwWorldActionObject::RegisterWrapper(*kLua);
	lwInt64::RegisterWrapper(*kLua);
	lwPuppet::RegisterWrapper(*kLua);
	PgUISound::RegisterWrapper(*kLua);
	lwQuestMan::RegisterWrapper(*kLua);
	lwHome::RegisterWrapper(*kLua);
	lwFurniture::RegisterWrapper(*kLua);

	//lwQuestInfoList::RegisterWrapper(*kLua);
	//lwQuestInfo::RegisterWrapper(*kLua);
	//lwQuestXMLInfo::RegisterWrapper(*kLua);
	lwUIListItem::RegisterWrapper(*kLua);

	lwProjectileMan::RegisterWrapper(*kLua);
	lwProjectile::RegisterWrapper(*kLua);
	lwProjectileOptionInfo::RegisterWrapper(*kLua);

	lwSkillTree::RegisterWrapper(*kLua);
	lwSkillSet::RegisterWrapper(*kLua);
	lwBaseItemSet::RegisterWrapper(*kLua);
	lwSelectStage::RegisterWrapper(*kLua);
	lwFindTargetParam::RegisterWrapper(*kLua);
	
	lwUIItemPlusUpgrade::RegisterWrapper(*kLua);
	lwUIItemRarityUpgrade::RegisterWrapper(*kLua);
	lwUILogin::RegisterWrapper(*kLua);

	lwUIItemSocketSystem::RegisterWrapper(*kLua);
	lwUIItemJewelBox::RegisterWrapper(*kLua);
	lwUIItemGamble::RegisterWrapper(*kLua);
	lwUIItemConvertSystem::RegisterWrapper(*kLua);

	lwChatMgrClient::RegisterWrapper(*kLua);
	lwActionResult::RegisterWrapper(*kLua);
	lwActionTargetInfo::RegisterWrapper(*kLua);
	lwActionTargetList::RegisterWrapper(*kLua);

	lwEffectDef::RegisterWrapper(*kLua);
	lwSkillDef::RegisterWrapper(*kLua);
	lwEffect::RegisterWrapper(*kLua);

	lwFriendMgr::RegisterWrapper(*kLua);
	lwFriendItem::RegisterWrapper(*kLua);
	lwSkillTargetMan::RegisterWrapper(*kLua);
	lwUNIT_PTR_ARRAY::RegisterWrapper(*kLua);
	lwActionResultVector::RegisterWrapper(*kLua);
	lwUnit::RegisterWrapper(*kLua);
	lwStatusEffectMan::RegisterWrapper(*kLua);
	lwActionTargetTransferInfoMan::RegisterWrapper(*kLua);
	lwGuild::RegisterWrapper(*kLua);

	lwUIItemRepair::RegisterWrapper(*kLua);
	lwUIShineStoneUpgrade::RegisterWrapper(*kLua);
	lwUIItemMaking::RegisterWrapper(*kLua);
	lwUIItemCooking::RegisterWrapper(*kLua);
	lwUIItemLottery::RegisterWrapper(*kLua);
	Quest::RegisterWrapper(*kLua);

	lwComboAdvisor::RegisterWrapper(*kLua);
	lwUIPetition::RegisterWrapper(*kLua);

	lwLinkedPlane::RegisterWrapper(*kLua);
	lwLinkedPlaneGroup::RegisterWrapper(*kLua);
	lwLinkedPlaneGroupMan::RegisterWrapper(*kLua);

	PgContentsBase::RegisterWrapper(*kLua);
	lwPgEmporiaAdministrator::RegisterWrapper(*kLua);

	lwEventTimer::RegisterWrapper(*kLua);
	lwInputSlotInfo::RegisterWrapper(*kLua);

	lwMovieMgr::RegisterWrapper(*kLua);
	lwHelpSystem::RegisterWrapper(*kLua);
	lwHelpObject::RegisterWrapper(*kLua);

	lwCouple::RegisterWrapper(*kLua);
	lwSystemInventory::RegisterWrapper(*kLua);
	lwSafeFoam::RegisterWrapper(*kLua);

	lwMarket::RegisterWrapper(*kLua);
	lwTeleCardUI::RegisterWrapper(*kLua);
	lwEventView::RegisterWrapper(*kLua);
	lwUIEmBattleLoad::RegisterWrapper(*kLua);
	lwMyActorView::RegisterWrapper(*kLua);
	lwCharInfo::RegisterWrapper(*kLua);
	
	lwDateTime::RegisterWrapper(*kLua);
	lwObject::RegisterWrapper(*kLua);
	lwRope::RegisterWrapper(*kLua);
	lwEventScriptCmd::RegisterWrapper(*kLua);
	lwEventScriptSystem::RegisterWrapper(*kLua);
	lwCameraModeEvent::RegisterWrapper(*kLua);
	lwCashShop::RegisterWrapper(*kLua);
	lwUIBook::RegisterWrapper(*kLua);
	lwAchieveNfyMgr::RegisterWrapper(*kLua);

	lwTextDialog::RegisterWrapper(*kLua);

	PgRankingMgr::RegisterWrapper(*kLua);

	lwAttackEffect::RegisterWrapper(*kLua);
	DailyQuestUI::RegisterWrapper(*kLua);
	RandomQuestUI::RegisterWrapper(*kLua);
	RandomTacticsQuestUI::RegisterWrapper(*kLua);
	lwOXQuizUI::RegisterWrapper(*kLua);
	lwLuckyStarUI::RegisterWrapper(*kLua);
	lwStyleString::RegisterWrapper(*kLua);

	lwBalloonEmoticonUtil::RegisterWrapper(*kLua);
	
	lwSoundMan::RegisterWrapper(*kLua);
	lwCharacterCard::RegisterWrapper(*kLua);
	lwUIItemBind::RegisterWrapper(*kLua);
	lwUIMemTransCard::RegisterWrapper(*kLua);
	PgCashItemUIRegWrapper(*kLua);
	lwOption::RegisterWrapper(*kLua);
	lwItemMix::RegisterWrapper(*kLua);
#ifndef USE_INB
	lwZoneDrawing::RegisterWrapper(*kLua);
#endif//USE_INB
	lwQuestNfy::RegisterWrapper(*kLua);

	BattleSquareUI::RegisterWrapper(*kLua);

	PgPetUIManager::RegisterWrapper(*kLua);
	// Event
	EventTaiwanEffectQuest::RegisterWrapper(*kLua);
	RealmQuest::RegisterWrapper(*kLua);

	lwGamePadUI::RegisterWrapper(*kLua);
	lwHomeUI::RegisterWrapper(*kLua);
	lwMinimap::RegisterWrapper(*kLua);

	lwHomeTown::RegisterWrapper(*kLua);
	lwHomeRenew::RegisterWrapper(*kLua);

	lwUIFireLove::RegisterWrapper(*kLua);
	PgEventQuestUI::RegisterWrapper(*kLua);
	lwUIIGGacha::RegisterWrapper(*kLua);
	lwUIActiveStatus::RegisterWrapper(*kLua);

	lwTransTower::RegisterWrapper(*kLua);
	PgHardCoreDungeon::RegisterWrapper(*kLua);
	lwUICSGacha::RegisterWrapper(*kLua);
	lwUIItemEventBox::RegisterWrapper(*kLua);
	lwUIItemRarityBuildUp::RegisterWrapper(*kLua);
	lwUIItemRarityAmplify::RegisterWrapper(*kLua);

	WantedQuestUI::RegisterWrapper(*kLua);
	PgLegendItemMakeHelpMng::RegisterWrapper(*kLua);
	PgSuperGroundUI::RegisterWrapper(*kLua);
	PgRagnarokGroundUI::RegisterWrapper(*kLua);

	lwUICostumeMix::RegisterWrapper(*kLua);

	PgPetExpItemUIManager::RegisterWrapper(*kLua);
	BalloonUtil::RegisterWrapper(*kLua);
	lwVendor::RegisterWrapper(*kLua);
	lwInventory::RegisterWrapper(*kLua);
	lwJobSkillLearn::RegisterWrapper(*kLua);
	lwJobSkillView::RegisterWrapper(*kLua);
	lwJobSkillItem::RegisterWrapper(*kLua);
	lwItemSkillUI::RegisterWrapper(*kLua);

	lwDeathSnatchSkillScriptHelpFunc::RegisterWrapper(*kLua);
	lwTreasureChest::RegisterWrapper(*kLua);
	lwSoulTransfer::RegisterWrapper(*kLua);
	lwCommonSkillUtilFunc::RegisterWrapper(*kLua);
	lwSummon_Info::RegisterWrapper(*kLua);

	lwDefenceMode::RegisterWrapper(*kLua);

	lwElementDungeonUI::RegisterWrapper(*kLua);

	lwManufacture::RegisterWrapper(*kLua);

	lwCommunityEvent::RegisterWrapper(*kLua);
	lwConstellationEnterUIUtil::RegisterWrapper(*kLua);

	lwPetUpgrade::RegisterWrapper(*kLua);

	lwJobSkill_NfySaveIdx::RegisterWrapper(*kLua);
	return false;
}

bool PgScripting::Initialize()
{
	if(S_OK != g_kLuaTinker.open())
	{
		return false;
	}

	RegisterModules();

#ifndef EXTERNAL_RELEASE
	char pcData[256] = "";
	lua_wrapper_user kLua(g_kLuaTinker);
	if ( g_pkApp->GetCommand()->String("luadebug", pcData, sizeof(pcData)) != 0 )
	{
		VEC_STRING kVec;
		std::string strIP = pcData;
		PgStringUtil::BreakSep(strIP, kVec, ",");//IP와 포트 번호 얻기.

		g_pkScriptDebug = new CPgLuaDebugger;
		g_pkScriptDebug->Initialize(
			*kLua,
			atoi(kVec[1].c_str()),
			kVec[0].c_str(),
			NiApplication::ms_pkApplication->GetWindowReference(),
			PgScripting::DoFile
			);
	}
#endif
	m_kScriptStatContainer.clear();
	m_kScriptDoFileQueue.clear();
	m_hMainThread = ::GetCurrentThreadId();
	//m_kDoBufferStatGroup.StartRecording();
	return true;
}

bool PgScripting::Terminate()
{
	if (g_pkScriptDebug)
	{
		g_pkScriptDebug->Terminate();
		SAFE_DELETE(g_pkScriptDebug);
	}

	g_kLuaTinker.close();
	
	NILOG(PGLOG_STAT, "[PgScripting] Statistics for script call\n");

	for (ScriptStatContainer::iterator iter = m_kScriptStatContainer.begin(); iter != m_kScriptStatContainer.end(); ++iter)
	{
		NILOG(PGLOG_STAT, "\t[PgScripting] %s script %d times called, total time %f, min %f, max %f, average %f\n",iter->first.c_str(), iter->second.iCount, iter->second.fTotalTime, iter->second.iCount > 0 ? iter->second.fMinTime : 0.0f, iter->second.fMaxTime, iter->second.iCount > 0 ? iter->second.fTotalTime / iter->second.iCount : 0.0f);
	}
	PrintFrameCount(m_kDoBufferStat, "\t[PgScripting] DoBuffer ");

	NILOG(PGLOG_STAT, "\t[PgScripting] DoString %d times called, total time %f min %f, max %f, average %f\n", m_kDoStringStat.iCount, m_kDoStringStat.fTotalTime, m_kDoStringStat.iCount > 0 ? m_kDoStringStat.fMinTime : 0.0f, m_kDoStringStat.fMaxTime, m_kDoStringStat.iCount > 0 ? m_kDoStringStat.fTotalTime / m_kDoStringStat.iCount : 0.0f);

	PG_STAT(m_kDoBufferStatGroup.PrintStatGroupForCSV());
	m_kScriptStatContainer.clear();
	m_kScriptDoFileQueue.clear();
	return true;
}

void PgScripting::Reset()
{
	if (m_bDoFileOptimize)
	{
		m_kScriptStatContainer.clear();
	}
}

void PgScripting::Update()
{
	if (m_kScriptDoFileQueue.size() == 0)
	{
		return;
	}

	//! Background Thread를 쓸 때에는 Queuing된 녀석들을 Update에서 Call한다.
	BM::CAutoMutex kLock(m_kDoFileQueueLock);

	for (ScriptDoFileQueue::iterator iter = m_kScriptDoFileQueue.begin(); iter != m_kScriptDoFileQueue.end(); ++iter)
	{
		doFile(iter->strFileName.c_str());
		NILOG(PGLOG_LOG, "[PgScripting] Update %s file, diff(%d)\n", iter->strFileName.c_str(), NiGetCurrentTimeInSec() - iter->fAddTime);
	}
	m_kScriptDoFileQueue.clear();
	return;
}

void PgScripting::DoFile(char const *pcFileName)
{
	if(pcFileName == NULL)
	{
		return;
	}

	if (GetCurrentThreadId() == m_hMainThread)
	{
		doFile(pcFileName);
	}
	else
	{
		BM::CAutoMutex kLock(m_kDoFileQueueLock);
		NILOG(PGLOG_THREAD, "[PgScripting] DoFile(%s) queued\n", pcFileName);

		stDoFileInfo info;
		info.strFileName = pcFileName;
		info.fAddTime = NiGetCurrentTimeInSec();
		m_kScriptDoFileQueue.push_back(info);
	}
}

//! 내부적으로 update시에 불리게 된다.
void PgScripting::doFile(char const *pcFileName)
{
	if (pcFileName == NULL)
	{
		return;
	}

	ScriptStatContainer::iterator iter = m_kScriptStatContainer.find(pcFileName);

	if (iter != m_kScriptStatContainer.end())
	{
		iter->second.iCount++;
		NILOG(PGLOG_LOG, "[PgScripting] DoFile(%s) called %d times, doFileOptimize:%d\n", pcFileName, iter->second.iCount, m_bDoFileOptimize);
		if (m_bDoFileOptimize == true)
		{
			return;
		}
	}

	float fCallTime = 0;
	NILOG(PGLOG_LOG, "[PgScripting] DoFile(%s) called\n", pcFileName);
	//! lwBase에 있던 것을 이리로 가져왔다. PgScripting에서 모두 관리하기 위해서.
	if(g_bUsePackData)
	{
		std::vector< char > data(0);
		PG_STAT(PgStatTimerF timerA(PgNiFile::ms_kFileIOStatGroup.GetStatInfo("LoadFromPack"), g_pkApp->GetFrameCount()));
		PG_STAT(timerA.Start());
		if(!BM::PgDataPackManager::LoadFromPack(_T("../script.dat"), (std::wstring)(_T("./"))+UNI(pcFileName), data))
		{
			NILOG(PGLOG_ERROR, "[PgScripting] DoFile(%s) LoadFromPack Failed\n", pcFileName);
			return;
		}
		PG_STAT(timerA.Stop());

		if(!data.size())
		{
			NILOG(PGLOG_ERROR, "[PgScripting] DoFile(%s) LoadFromPack size 0\n", pcFileName);
			return;
		}
		fCallTime = NiGetCurrentTimeInSec();
		lua_tinker::dobuffer(*lua_wrapper_user(g_kLuaTinker), &data.at(0), data.size());
		fCallTime = NiGetCurrentTimeInSec() - fCallTime;
	}
	else
	{
		std::string strPath = "../script/";
		strPath += pcFileName;
		fCallTime = NiGetCurrentTimeInSec();
		lua_tinker::dofile(*lua_wrapper_user(g_kLuaTinker), strPath.c_str());
		fCallTime = NiGetCurrentTimeInSec() - fCallTime;
	}

	if (iter != m_kScriptStatContainer.end())
	{
		UpdateStatInfo(iter->second, fCallTime);
	}
	else
	{
		StatInfoF stat;
		UpdateStatInfo(stat, fCallTime);
		m_kScriptStatContainer.insert(std::make_pair(pcFileName, stat));
	}
}

void PgScripting::DoString(char const *pcCmd)
{
	if (pcCmd == NULL)
		return;

	if (GetCurrentThreadId() != m_hMainThread)
	{
		NILOG(PGLOG_LOG, "[PgScripting] DoString(%s) call from %d thread\n", pcCmd, GetCurrentThreadId());
	}
	float fCallTime;
	fCallTime = NiGetCurrentTimeInSec();
	dostring(*lua_wrapper_user(g_kLuaTinker), pcCmd);
	fCallTime = NiGetCurrentTimeInSec() - fCallTime;

	UpdateStatInfo(m_kDoStringStat, fCallTime);
	if (fCallTime > 0.05f)
	{
		NILOG(PGLOG_STAT, "[PgScripting] DoString(%s) takes more than 0.05 sec\n", pcCmd);
	}
}

void PrintStackStatus(lua_State *L) 
{
	NILOG("----------------------------------------------------------------\n");
	NILOG("(top:%d)", lua_gettop(L)); 
	for (int i = 1; i <= lua_gettop(L); i++)  
	{ 
		switch(lua_type(L, i)) 
		{ 
		case LUA_TTABLE : 
		case LUA_TNIL :  
		case LUA_TFUNCTION : 
		case LUA_TUSERDATA : 
		case LUA_TTHREAD : 
		case LUA_TBOOLEAN :
			{ 
				NILOG("(%d:%s)%d ", i, lua_typename(L, lua_type(L, i)), lua_toboolean(L, i)); 
			}break; 
		case LUA_TLIGHTUSERDATA : 
			{ 
				NILOG("(%d:%s) ", i, lua_typename(L, lua_type(L, i))); 
			}break; 
		case LUA_TNUMBER : 
			{ 
				NILOG("(%d:%s)%d ", i, lua_typename(L, lua_type(L, i)), (int)lua_tonumber(L, i)); 
			}break;
		case LUA_TSTRING : 
			{ 
				NILOG("(%d:%s)%s ", i, lua_typename(L, lua_type(L, i)), lua_tostring(L, i)); 
			}break;
		} 
	}
	NILOG("\n");
	NILOG("----------------------------------------------------------------\n");
}

bool PgScripting::DoBuffer(char const *pcCmd, size_t const size)
{
	if (pcCmd == NULL || size == 0)
	{
		return false;
	}

	if (GetCurrentThreadId() != m_hMainThread)
	{
		NILOG(PGLOG_LOG, "[PgScripting] DoBuffer(%s) call from %d thread\n", pcCmd, GetCurrentThreadId());
	}

	//PG_STAT(PgStatTimerF timerD(m_kDoBufferStatGroup.GetStatInfo(std::string(pcCmd)), g_pkApp->GetFrameCount()));

	float fCallTime;
	fCallTime = NiGetCurrentTimeInSec();
	//lua_tinker의 dobuffer(L, Cmd, sz); 배껴서 lua에서 return value있도록 조정
//	size_t iCnt = lua_tinker::lua_state::get_count();
//	for(size_t i=0; i<iCnt; ++i)
//	{
		lua_wrapper_user kLua(g_kLuaTinker);
		lua_State *L = *kLua;
		lua_settop(L, 0);
		lua_pushcclosure(L, lua_tinker::on_error, 0);

		if( 0 == luaL_loadbuffer(L, pcCmd, size, __FUNCTION__) )
		{
			//Success (대부분 여기로 진입
			bool bRet = false; //Return val

			if( 0 == lua_pcall(L, 0, 1, 1) ) //Call Function Argument is 0, Return Value is 1, Error func 1
			{
				switch( lua_type(L, -1) ) //Return value type
				{
				case LUA_TNUMBER:
					{
						int iRet = (int)lua_tonumber(L, -1);
						bRet = (0==iRet)? false: true;
					}
				case LUA_TBOOLEAN:
					{
						int iRet = lua_toboolean(L, -1);
						bRet = (0==iRet)? false: true;
					}break;
				}
				lua_pop(L, 1);//Remove Return Value
			}
			lua_pop(L, 1);//Remove Function

			fCallTime = NiGetCurrentTimeInSec() - fCallTime;
			UpdateFrameCount(m_kDoBufferStat, g_pkApp->GetFrameCount(), fCallTime);
			return bRet;
		}
		else
		{
			lua_pop(L, 1);
		}
//	}
	/*
	//NILOG(PGLOG_LOG, "DoBuffer(%s) called\n", pcCmd);
	dobuffer(pcCmd, size);
	*/
	fCallTime = NiGetCurrentTimeInSec() - fCallTime;
	UpdateFrameCount(m_kDoBufferStat, g_pkApp->GetFrameCount(), fCallTime);
	if (fCallTime > 0.05f)
	{
		NILOG(PGLOG_WARNING, "[PgScripting] DoBuffer(%s) takes more than 0.05 sec\n", pcCmd);
	}
	return false;
}