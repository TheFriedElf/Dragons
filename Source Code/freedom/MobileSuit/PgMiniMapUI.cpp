#include "stdafx.h"
#include "PgMobileSuit.h"
#include "Variant/PgQuestInfo.h"
#include "PgRenderMan.h"
#include "PgRenderer.h"
#include "PgPilot.h"
#include "PgPilotMan.h"
#include "PgNifMan.h"
#include "PgTrigger.h"
#include "lwUI.h"
#include "NiDx9RenderedTextureData.h"
#include "lwWorld.h"
#include "PgClientParty.h"
#include "PgQuest.h"
#include "PgUIScene.H"
#include "PgDirectionArrow.h"
#include "PgShineStone.h"
#include "PgCoupleMgr.h"
#include "PgResourceIcon.h"
#include "PgHouse.h"
#include "variant/PgMyHome.h"
#include "PgFriendMgr.h"
#include "PgGuild.h"
#include "PgMiniMapUI.h"

#define PRERENDER_MINIMAP_WIDTH XUI::EXV_DEFAULT_SCREEN_WIDTH
#define PRERENDER_MINIMAP_HEIGHT XUI::EXV_DEFAULT_SCREEN_HEIGHT
#define MINIMAP_ICON_SIZE 12
#define MINIMAP_LARGEICON_SIZE 32
#define MINIMAP_STONEICON_SIZE 27
#define MINIMAP_MEICON_SIZE 27
#define MINIMAP_ICON_HALF_SIZE (MINIMAP_ICON_SIZE / 2)
#define MINIMAP_LARGEICON_HALF_SIZE (MINIMAP_LARGEICON_SIZE / 2)
#define MINIMAP_STONEICON_HALF_SIZE (MINIMAP_STONEICON_SIZE / 2)
#define MINIMAP_MEICON_HALF_SIZE (MINIMAP_MEICON_SIZE / 2)
#define MINIMAP_CENTER_THRESHOLD 0.05f
#define MINIMAP_MOVE_TO_CENTER_STEP 0.05f
#define MINIMAP_SCREEND_IMAGE_RATIO		0.75f
#define MINIMAP_STONE_MAX_FRAME 4

extern SGroundKey g_kNowGroundKey;
wchar_t const * const NPC_ICON_TYPE_NAME = _T("ICON_PATH_NPC");

bool g_bAbleSetTeleMove = false;
//PgMiniMapUI::kMapIconTexContainer	PgMiniMapUI::m_kMapIconTexCont;
kMapIconToIdxContainer	PgMiniMapUI::m_kMapIconTypeToIdxCont;
bool					PgMiniMapUI::m_bIsShowText = false;

PgMiniMapUI::PgMiniMapUI(const POINT2 &ptWndSize) :
	m_bInitialized(false),
	m_bShowNPC(true),
	m_bShowTrigger(true),
	m_bShowQuest(true),
	m_iShowActor(0),
	m_bShowPartyMember(true),
	m_bShowGuildMember(true),
	m_spRoot(0),
	m_spCamera(0),
	m_spMiniMapScreenTexture(0),
	m_spModelScreenTexture(0),
	m_spRenderTargetGroup(0),	
	m_spRenderTexture(0),
	m_spMiniMapImage(0),
	m_pObjectContainer(0),
	m_pTriggerContainer(0),
	m_kWorldCenter(0.0f, 0.0f, 0.0f),
	m_fWorldRadius(0.0f),
	m_fZoomFactor(0.5f),
	m_kWndSize(0, 0),
	m_fScreenImageRatio(MINIMAP_SCREEND_IMAGE_RATIO), // 4:3 
	m_kScreenCenter(0.5f, 0.5f),
	m_bUseMiniMapImage(false),
	m_bAlwaysMinimap(true),
	m_bIsShowToolTip(false),
	m_kTextObject(NULL)
{
	m_kWndSize = ptWndSize;
	m_kMiniMapImageName = "";
}

PgMiniMapUI::~PgMiniMapUI()
{
	Terminate();
}

void PgMiniMapUI::Terminate()
{
	m_spRoot = 0;
	m_spCamera = 0;
	m_spMiniMapScreenTexture = 0;
	m_spModelScreenTexture = 0;
	m_spRenderTargetGroup = 0;
	m_spRenderTexture = 0;
	m_pObjectContainer = 0;
	m_spMiniMapImage = 0;

	m_kMapIcons.clear();
	m_kMapLeftIcons.clear();
	m_kMapRightIcons.clear();
	m_kQuestInfoCont.clear();

	m_pObjectContainer = NULL;
	m_pTriggerContainer = NULL;

	m_bInitialized = false;
}

bool PgMiniMapUI::Initialize( NiAVObjectPtr spModelRoot, PgWorld::ObjectContainer* pObjectContainer, 
                              PgWorld::TriggerContainer* pTriggerContainer, NiCameraPtr pkWorldCamera, 
                              std::string& kMiniMapImage, POINT* pDrawHeight )
{
	m_kMiniMapImageName = kMiniMapImage;
	if (spModelRoot == NULL || pObjectContainer == NULL)
		return false;

	if (m_bInitialized)
		Terminate();

	m_spRoot = spModelRoot;
	m_pObjectContainer = pObjectContainer;
	m_pTriggerContainer = pTriggerContainer;
	m_pkDrawHeight = pDrawHeight;
	
	if(!UpdateModel(pkWorldCamera))
	{
		return 0;
	}

	MiniMapAniIconCont::_Pairib Rst = m_kAniIconCont.insert(std::make_pair(ICONTYPE_ME, MiniMapAniIconContainer()));
	if( Rst.second )
	{
		SMiniMapAniIcon	Icon(0.2f, 4, POINT2(2, 2));
		Rst.first->second.push_back(Icon);
	}
	MiniMapAniIconCont::_Pairib bRet = m_kAniIconCont.insert(std::make_pair(ICONTYPE_PARTY, MiniMapAniIconContainer()));
	if( bRet.second )
	{
		SMiniMapAniIcon	Icon(0.2f, 4, POINT2(2, 2));
		bRet.first->second.push_back(Icon);
	}

	NiRenderer *pkRenderer = NiRenderer::GetRenderer();

	ParseMiniMapXml();

	//  Alpha를 뺴준다.
	NiTexture::FormatPrefs kFormat;
	kFormat.m_ePixelLayout= NiTexture::FormatPrefs::TRUE_COLOR_32;
	kFormat.m_eAlphaFmt = NiTexture::FormatPrefs::SMOOTH;
	kFormat.m_eMipMapped = NiTexture::FormatPrefs::NO;

	if (kMiniMapImage.empty())
	{
		m_spRenderTexture = NiRenderedTexture::Create(PRERENDER_MINIMAP_WIDTH, PRERENDER_MINIMAP_HEIGHT, pkRenderer, kFormat);		
		m_bUseMiniMapImage = false;
		m_fScreenImageRatio = 1;
	}
	else
	{
		m_bUseMiniMapImage = true;
		m_spRenderTexture = NULL;
		m_spMiniMapImage = g_kNifMan.GetTexture(kMiniMapImage);
		if (m_spMiniMapImage == NULL)
			return false;

		float fWidth = m_spMiniMapImage->GetWidth();
		float fHeight = m_spMiniMapImage->GetHeight();
		m_kDrawHeight.x = pDrawHeight->x / fHeight;
		m_kDrawHeight.y = (pDrawHeight->y + pDrawHeight->x) / fHeight;
		m_fScreenImageRatio = (fWidth / fHeight) * MINIMAP_SCREEND_IMAGE_RATIO; // 이미지가 4:3비율이라면 1.0이 된다.
		m_iMiniMapDrawGap = (fHeight - fWidth * m_fScreenImageRatio) / 2;
	}

	if (!m_bUseMiniMapImage)
	{
		if (m_spRenderTexture == NULL)
			return false;

		m_spRenderTargetGroup = NiRenderTargetGroup::Create(m_spRenderTexture->GetBuffer(), pkRenderer, true, true);		
	}

	//! ScreenElement Init
	m_spMiniMapScreenTexture = NiNew NiScreenElements(
		NiNew NiScreenElementsData(false, false, 1));

	if (m_spMiniMapScreenTexture == NULL)
		return false;

	m_spMiniMapScreenTexture->Insert(4);

	NiTexturingPropertyPtr spTextureProp = NiNew NiTexturingProperty;
	if (m_bUseMiniMapImage)
	{
		spTextureProp->SetBaseTexture(m_spMiniMapImage);
	}
	else
	{
		spTextureProp->SetBaseTexture(m_spRenderTexture);
	}
	spTextureProp->SetBaseFilterMode(NiTexturingProperty::FILTER_NEAREST);
	spTextureProp->SetApplyMode(NiTexturingProperty::APPLY_REPLACE);
	spTextureProp->SetBaseClampMode(NiTexturingProperty::CLAMP_S_CLAMP_T);

	NiAlphaPropertyPtr spAlphaProp = NiNew NiAlphaProperty;
	spAlphaProp->SetAlphaBlending(true);

	NiZBufferPropertyPtr spZBufferProp = NiNew NiZBufferProperty;
	spZBufferProp->SetZBufferTest(false);
	spZBufferProp->SetZBufferWrite(true);

	float scaleFactor = m_fZoomFactor / 2;
	float screenHalfHeight = (1.0f - scaleFactor * 2) / 2;

	m_spMiniMapScreenTexture->SetRectangle(0, 0.1f, 0.1f, 0.1f, 0.1f);
	m_spMiniMapScreenTexture->UpdateBound();
	m_spMiniMapScreenTexture->SetTextures(0, 0,
		0.0f, 
		0.5 - screenHalfHeight, 
		1.0f, 
		0.5 + screenHalfHeight);

	m_spMiniMapScreenTexture->AttachProperty(spTextureProp);
	m_spMiniMapScreenTexture->AttachProperty(spAlphaProp);
	m_spMiniMapScreenTexture->AttachProperty(spZBufferProp);

	m_spMiniMapScreenTexture->UpdateProperties();
	m_spMiniMapScreenTexture->Update(0.0f);

	//! Rendering once
	bool bInsideFrame = pkRenderer->GetInsideFrameState();
	
	if (!bInsideFrame)
		pkRenderer->BeginFrame();
	pkRenderer->SetSorter(NiNew NiAlphaAccumulator());
	pkRenderer->GetSorter()->StartAccumulating(m_spCamera);

	NiRenderTargetGroup *pkOldTarget = 0;
	if(pkRenderer->IsRenderTargetGroupActive())
	{
		pkOldTarget = (NiRenderTargetGroup *) pkRenderer->GetCurrentRenderTargetGroup();
		pkRenderer->EndUsingRenderTargetGroup();
	}

	pkRenderer->SetBackgroundColor(DEF_BG_COLORA);
	pkRenderer->BeginUsingRenderTargetGroup(m_spRenderTargetGroup, NiRenderer::CLEAR_ALL);
	pkRenderer->SetCameraData(m_spCamera);

	NiFogProperty* pFogProperty = (NiFogProperty*)m_spRoot->GetProperty(NiProperty::FOG);
	bool bOldFog = false;
	if (pFogProperty)
	{
		bOldFog = pFogProperty->GetFog();
		pFogProperty->SetFog(false);
	}

	{
		NiVisibleArray kArray;
		NiCullingProcess kCuller(&kArray);

		NiCullScene(m_spCamera, m_spRoot, kCuller, kArray, true);
		NiDrawVisibleArrayAppend(kArray);
	}

	pkRenderer->GetSorter()->FinishAccumulating();
	pkRenderer->EndUsingRenderTargetGroup();
	pkRenderer->SetBackgroundColor(DEF_BG_COLOR);

	if (pFogProperty)
	{
		pFogProperty->SetFog(bOldFog);
	}

	if(pkOldTarget)
	{
		pkRenderer->BeginUsingRenderTargetGroup(pkOldTarget, NiRenderer::CLEAR_NONE);
	}

	if (!bInsideFrame)
	{
		pkRenderer->EndFrame();
		pkRenderer->DisplayFrame();
	}

	UpdateQuest();
	m_bInitialized = true;

	if( !m_kTextObject )
	{
		m_kTextObject = NiNew PgTextObject();
		if( m_kTextObject )
		{
			CXUI_Font *pFont = g_kFontMgr.GetFont(FONT_CHAT); // 이름
			m_kTextObject->SetText(_T("{C=0xFFFFFF00/}너는누구냐"), pFont);
			
		}
	}
	return true;
}

void PgMiniMapUI::RenderFrame(NiRenderer *pkRenderer, const POINT2 &ptWndPos)
{
	if(!pkRenderer || !m_bInitialized)
		return;	

	if (m_spMiniMapScreenTexture == false)
		return;

	float uiScreenWidth = (float)pkRenderer->GetDefaultRenderTargetGroup()->GetWidth(0);
	float uiScreenHeight = (float)pkRenderer->GetDefaultRenderTargetGroup()->GetHeight(0);
	PgActor* pMyActor = g_kPilotMan.GetPlayerActor();

	if( pMyActor == NULL || m_kMapIconTexCont.empty() )
		return;

	kMapIconTexContainer::iterator iter = m_kMapIconTexCont.begin();
	while( iter != m_kMapIconTexCont.end() )
	{
		if( NULL == iter->second.IconTexture )
			return;
		else
			iter->second.IconTexture->RemoveAll();

		++iter;
	}

	int iconCount = 0;

	if( !g_pkWorld->IsHaveAttr(GATTR_FLAG_HOMETOWN) )
	{
		if (m_pTriggerContainer != NULL)
		{
			//! 일단 PgWorld에서 가져온 object containter부터 체크, NPC가 여기에 들어있다.
			if (m_bShowTrigger)
			{
				for(PgWorld::TriggerContainer::iterator itr = m_pTriggerContainer->begin();
					itr != m_pTriggerContainer->end();
					++itr)
				{
					PgTrigger* pTrigger = itr->second;
					if (pTrigger)
					{
						addTriggerToMiniMap(pTrigger, ptWndPos);
					}
					else
						continue;
				}
			}
		}
	}

	//pkRenderer->BeginUsingDefaultRenderTargetGroup(NiRenderer::CLEAR_ALL);
	if (m_pObjectContainer != NULL)
	{
		PgWorld::ObjectContainer::iterator iter = m_pObjectContainer[WOCID_BEFORE].begin();
		for(; m_pObjectContainer[WOCID_BEFORE].end() != iter; ++iter)
		{
			MiniMapQuestSortCont	kSortQuestCont;
			PgWorld::ObjectContainer::mapped_type& pkElement = (*iter).second;
			if( !pkElement ) continue;
			PgPilot* pkPilot = pkElement->GetPilot();
			if( !pkPilot ) continue;

			if( pkPilot )
			{
				bool bDraw = false;
				int iconType = 0;
				EUnitGrade Grade = static_cast<EUnitGrade>(pkPilot->GetAbil(AT_GRADE));
				switch( Grade )
				{
				case EOGRADE_SUBCORE:
					{
						int const Team = pkPilot->GetAbil(AT_TEAM);
						switch( Team )
						{
						case TEAM_ATTACKER: { iconType = ICONTYPE_EMPORIA_SUB_RED;	 bDraw = true;	} break;
						case TEAM_DEFENCER: { iconType = ICONTYPE_EMPORIA_SUB_BLUE;	 bDraw = true;	} break;
						}
					}break;
				case EOGRADE_MAINCORE:
					{
						int const Team = pkPilot->GetAbil(AT_TEAM);
						switch( Team )
						{
						case TEAM_ATTACKER: { iconType = ICONTYPE_EMPORIA_MAIN_RED;	 bDraw = true;	} break;
						case TEAM_DEFENCER: { iconType = ICONTYPE_EMPORIA_MAIN_BLUE; bDraw = true;	} break;
						}
					}break;
				default:{}break;
				}
				if( bDraw )
				{
					PgActor* pkActor = dynamic_cast<PgActor*>(pkPilot->GetWorldObject());
					if( pkActor )
						addPositionToMiniMap(pkActor->GetPos(), ptWndPos, static_cast<IconType>(iconType));
				}
			}
		}

		//! 일단 PgWorld에서 가져온 object containter부터 체크, NPC가 여기에 들어있다.
		if( m_bShowNPC )
		{
			MiniMapQuestSortCont	kSortQuestCont;
			iter = m_pObjectContainer[WOCID_MAIN].begin();
			for(; m_pObjectContainer[WOCID_MAIN].end() != iter; ++iter)
			{
				PgWorld::ObjectContainer::mapped_type& pkElement = (*iter).second;
				if( !pkElement ) continue;
				PgPilot* pkPilot = pkElement->GetPilot();
				if( !pkPilot ) continue;
				PgActor* pkActor = dynamic_cast<PgActor*>(pkPilot->GetWorldObject());
				if( !pkActor )
				{//홈은 액터가 아니다
					CheckAddIconToMyHome(pkPilot, ptWndPos);
					continue;
				}

				//! 나는 나중에 따로 그린다.
				if( pkActor
				&&	!pkActor->IsMyActor() )
				{
					BM::GUID const &rkGuid = pkActor->GetGuid();
					if( !pkActor->IsHide() )
					{
						bool bDrawQuest = true;
						MiniMapQuestCont::const_iterator quest_iter = m_kQuestInfoCont.find(rkGuid);
						if( m_bShowQuest )
						{
							EQuestState eState = QS_None;
							SQuestMiniMapKey kKey(pkActor->GetGuid());
							if( m_kQuestInfoCont.end() != quest_iter )
							{// 퀘스트표시는 따로 그린다.
								eState = (*quest_iter).second;
								switch( eState )
								{
								case QS_Begin_NYet:		{ kKey.kOrder = PgQuestManUtil::EQMDO_HIGH;				}break;
								case QS_Ing:			{ kKey.kOrder = PgQuestManUtil::EQMDO_ING;				}break;
								case QS_Begin_Loop:		{ kKey.kOrder = PgQuestManUtil::EQMDO_REPEAT_START;		}break;
								case QS_End_Loop:		{ kKey.kOrder = PgQuestManUtil::EQMDO_REPEAT_END;		}break;
								case QS_Begin:			{ kKey.kOrder = PgQuestManUtil::EQMDO_ADVENTURE_START;	}break;
								case QS_End:			{ kKey.kOrder = PgQuestManUtil::EQMDO_ADVENTURE_END;	}break;
								case QS_Begin_Tactics:	{ kKey.kOrder = PgQuestManUtil::EQMDO_ONEDAY_START;		}break;
								case QS_End_Tactics:	{ kKey.kOrder = PgQuestManUtil::EQMDO_ONEDAY_END;		}break;
								case QS_Begin_Story:	{ kKey.kOrder = PgQuestManUtil::EQMDO_HERO_START;		}break;
								case QS_End_Story:		{ kKey.kOrder = PgQuestManUtil::EQMDO_HERO_END;			}break;
								case QS_Begin_Weekly:	{ kKey.kOrder = PgQuestManUtil::EQMDO_WEEKLY_START;		}break;
								case QS_End_Weekly:		{ kKey.kOrder = PgQuestManUtil::EQMDO_WEEKLY_END;		}break;
								case QS_Begin_Couple:	{ kKey.kOrder = PgQuestManUtil::EQMDO_COUPLE_START;		}break;
								case QS_End_Couple:		{ kKey.kOrder = PgQuestManUtil::EQMDO_COUPLE_END;		}break;
								default:
									{
										bDrawQuest = false;
									}break;
								}
							}
							else
							{
								bDrawQuest = false;
							}

							if( bDrawQuest )
							{	
								SQuestMiniMapInfo kInfo;
								MiniMapQuestSortCont::_Pairib Rst = kSortQuestCont.insert(std::make_pair(kKey, kInfo));
								if( Rst.second )
								{
									Rst.first->second.eState = eState;
									Rst.first->second.pkActor = pkActor;
								}
							}
						}

						if( !bDrawQuest )
						{// 퀘스트가 아닌놈들.
							addActorToMiniMap(pkActor, ptWndPos);
						}

						++iconCount;
					}
				}
			}

			//퀘스트 그리자.
			MiniMapQuestSortCont::const_iterator c_iter = kSortQuestCont.begin();
			while( kSortQuestCont.end() != c_iter )
			{
				addQuestToMiniMap(c_iter->second.pkActor, c_iter->second.eState, ptWndPos);
				++c_iter;
			}
		}

		//! 길드 멤버 그리기
		if (m_bShowGuildMember)
		{

		}			

		//! 파티 멤버 그리기
		if (m_bShowPartyMember && g_kParty.MemberCount() > 1)
		{
			int iGroundNo = 0;

			if( !g_kNowGroundKey.IsEmpty() )
			{
				iGroundNo = g_kNowGroundKey.GroundNo();
			}			

			ContPartyMember kPartyMemberList;
			g_kParty.GetPartyMemberList(kPartyMemberList);
			ContPartyMember::const_iterator iter = kPartyMemberList.begin();
			for(; iter != kPartyMemberList.end(); ++iter)
			{
				PgPilot* pilot = g_kPilotMan.FindPilot((*iter)->kCharGuid);
				if (pilot && pilot->GetWorldObject())
				{
					if(g_kPilotMan.IsMyPlayer(pilot->GetGuid()))
					{
						continue;
					}
					//! 파티원이 다른 맵에 있는 경우.
					if (pilot->GetWorldObject()->GetWorld() == NULL || pilot->GetWorldObject()->GetWorld() != pMyActor->GetWorld())
					{
						if (addOtherWorldActorToMiniMap(dynamic_cast<PgActor *>(pilot->GetWorldObject()), ptWndPos, ICONTYPE_PARTY))
						{
							iconCount++;
						}
					}
					else
					{
						MiniMapAniIconCont::iterator	iter = m_kAniIconCont.find(ICONTYPE_PARTY);
						if( iter != m_kAniIconCont.end() )
						{
							if( !iter->second.empty() )
							{
								MiniMapAniIconContainer::iterator e_iter = iter->second.begin();
								if( e_iter != iter->second.end() )
								{
									e_iter->NexFrame(g_pkWorld->GetAccumTime());
									addActorToMiniMap(dynamic_cast<PgActor *>(pilot->GetWorldObject()), ptWndPos, ICONTYPE_PARTY);								
									iconCount++;
								}
							}
						}
					}
				}
				else
				{
					if( (0 != iGroundNo) && (iGroundNo == (*iter)->GroundNo()) )
					{
						NiPoint3 WorldPt = NiPoint3((*iter)->ptPos.x, (*iter)->ptPos.y, (*iter)->ptPos.z);
						MiniMapAniIconCont::iterator	iter = m_kAniIconCont.find(ICONTYPE_PARTY);
						if( iter != m_kAniIconCont.end() )
						{
							if( !iter->second.empty() )
							{
								MiniMapAniIconContainer::iterator e_iter = iter->second.begin();
								if( e_iter != iter->second.end() )
								{
									e_iter->NexFrame(g_pkWorld->GetAccumTime());
									addPositionToMiniMap(WorldPt, ptWndPos, ICONTYPE_PARTY);
									iconCount++;
								}
							}
						}
					}
				}
			}
		}

		//	커플
		if( g_kCoupleMgr.Have() )
		{
			PgPilot* pilot = g_kPilotMan.FindPilot(g_kCoupleMgr.GetMyInfo().CoupleGuid());

			if( pilot && pilot->GetWorldObject() )
			{
				if( (pilot->GetWorldObject()->GetWorld() == NULL)
				 || (pilot->GetWorldObject()->GetWorld() != pMyActor->GetWorld()) )
				{
					if(addOtherWorldActorToMiniMap(dynamic_cast<PgActor *>(pilot->GetWorldObject()), ptWndPos, ICONTYPE_COUPLE))
					{
						iconCount++;
					}
				}
				else
				{
					if(addActorToMiniMap(dynamic_cast<PgActor *>(pilot->GetWorldObject()), ptWndPos, ICONTYPE_COUPLE))
					{
						iconCount++;
					}
				}
			}
		}

		//! 나 자신 위치 표시
		if (pMyActor)
		{
			PG_ASSERT_LOG(pMyActor->IsMyActor());
			MiniMapAniIconCont::iterator	iter = m_kAniIconCont.find(ICONTYPE_ME);
			if( iter != m_kAniIconCont.end() )
			{
				MiniMapAniIconContainer::iterator e_iter = iter->second.begin();
				e_iter->NexFrame(g_pkWorld->GetAccumTime());
				addActorToMiniMap(pMyActor, ptWndPos, ICONTYPE_ME);
				iconCount++;
			}
		}
	}

	float scaleFactor = m_fZoomFactor / 2;
	PG_ASSERT_LOG (scaleFactor >= 0.0f && scaleFactor <= 0.4f);
	float screenHalfWidth = (1.0f - scaleFactor * 2) / 2;
	float screenHalfHeight = (1.0f - scaleFactor * 2) / 2;
	float gap = (1.0f - m_fScreenImageRatio) / 2;

	if (m_spMiniMapScreenTexture)
	{
		if( m_bAlwaysMinimap )
		{
			m_spMiniMapScreenTexture->SetRectangle(0, 
				(ptWndPos.x / uiScreenWidth), (ptWndPos.y / uiScreenHeight),
				(m_kWndSize.x / uiScreenWidth), (m_kWndSize.y / uiScreenHeight));
			m_spMiniMapScreenTexture->UpdateBound();
			m_spMiniMapScreenTexture->SetTextures(0, 0,
				m_kScreenCenter.x - screenHalfWidth, (m_kScreenCenter.y - screenHalfHeight) * m_fScreenImageRatio + gap,
				m_kScreenCenter.x + screenHalfWidth, (m_kScreenCenter.y + screenHalfHeight) * m_fScreenImageRatio + gap);
		}
		else
		{
			m_spMiniMapScreenTexture->SetRectangle(0, 
				(ptWndPos.x / uiScreenWidth), (ptWndPos.y / uiScreenHeight),
				(m_kWndSize.x / uiScreenWidth), (m_pkDrawHeight->y / uiScreenHeight));
			m_spMiniMapScreenTexture->UpdateBound();
			m_spMiniMapScreenTexture->SetTextures(0, 0,
				m_kScreenCenter.x - screenHalfWidth, m_kDrawHeight.x,
				m_kScreenCenter.x + screenHalfWidth, m_kDrawHeight.y);
		}
	}
	//pkRenderer->EndUsingRenderTargetGroup();
	return;
}

POINT2 PgMiniMapUI::GetWndSize()
{
	return m_kWndSize;
}

NiCameraPtr PgMiniMapUI::GetCamera()
{
	return m_spCamera;
}

NiAVObjectPtr PgMiniMapUI::GetModelRoot()
{
	return m_spRoot;
}

NiScreenElementsPtr PgMiniMapUI::GetMiniMapScreenTexture()
{
	return m_spMiniMapScreenTexture;
}

NiScreenTexturePtr PgMiniMapUI::GetModelScreenTexture()
{
	return m_spModelScreenTexture;
}

NiRenderTargetGroupPtr PgMiniMapUI::GetRenderTargetGroup()
{
	return m_spRenderTargetGroup;
}

NiRenderedTexturePtr PgMiniMapUI::GetRenderTexture()
{
	return m_spRenderTexture;
}

NiTexturePtr PgMiniMapUI::GetMiniMapImage()
{
	return m_spMiniMapImage;
}

PgWorld::ObjectContainer* PgMiniMapUI::GetObjectContainer()
{
	return m_pObjectContainer;
}

PgWorld::TriggerContainer* PgMiniMapUI::GetTriggerContainer()
{
	return m_pTriggerContainer;
}

MiniMapIconContainer PgMiniMapUI::GetMapIcons()
{
	return m_kMapIcons;
}

MiniMapIconContainer PgMiniMapUI::GetMapLeftIcons()
{
	return m_kMapLeftIcons;
}

MiniMapIconContainer PgMiniMapUI::GetMapRightIcons()
{
	return m_kMapRightIcons;
}

kMapIconTexContainer& PgMiniMapUI::GetMapIconTexCont()
{
	return m_kMapIconTexCont;
}

bool PgMiniMapUI::UpdateModel(NiCameraPtr pkWorldCamera)
{
	if(!m_spRoot)
	{
		return false;
	}

	NiBound kBound;
	GetWorldBounds(m_spRoot, kBound);
	m_fWorldRadius = kBound.GetRadius();
	m_kWorldCenter = kBound.GetCenter();

	if (pkWorldCamera)
	{
		if (m_spCamera)
		{
			m_spCamera = 0;
		}
		m_spCamera = pkWorldCamera;
	}
	else if(!m_spCamera)
	{
		m_spCamera = NiNew NiCamera;
		NiFrustum kFrustum = m_spCamera->GetViewFrustum();
		//kFrustum.m_bOrtho = true;
		//kFrustum.m_fLeft = -m_fWorldRadius;
		//kFrustum.m_fRight = m_fWorldRadius;
		//kFrustum.m_fTop = -m_fWorldRadius;
		//kFrustum.m_fBottom = m_fWorldRadius;
		kFrustum.m_fNear = 1.0f;
		kFrustum.m_fFar = 100000.0f;
		m_spCamera->SetViewFrustum(kFrustum);
		updateCameraPosition();
	}
	

	return true;
}

void PgMiniMapUI::updateCameraPosition()
{
	if (m_spCamera == NULL)
		return;

	NiPoint3 kCameraPos;
	kCameraPos = NiPoint3(m_kWorldCenter.x, m_kWorldCenter.y - m_fWorldRadius * 0.7f, m_kWorldCenter.z + m_fWorldRadius * 0.65f);
	NiPoint3 a = kCameraPos - m_kWorldCenter;
	NiPoint3 b = NiPoint3::UNIT_X;
	NiPoint3 kUpVector = a.UnitCross(b);

	// 모델을 찍을 카메라를 설정한다.
	m_spCamera->SetTranslate(kCameraPos);	
	m_spCamera->Update(0.0f);
	m_spCamera->LookAtWorldPoint(m_kWorldCenter, kUpVector);
	//m_spCamera->LookAtWorldPoint(kCenter, NiPoint3::UNIT_Z);
	m_spCamera->Update(0.0f);
#ifdef _DEBUG
	//char temp[64];
	//sprintf(temp, "\n zoom factor : %f", m_fZoomFactor);
	//OutputDebugStringA(temp);
#endif
}

void PgMiniMapUI::RefreshZoomMiniMap()
{
	NiPoint3 worldPosition;
	NiPoint3 viewPosition;
//	NiPoint3 screenPosition;

	PgActor *pkActor = g_kPilotMan.GetPlayerActor();
	if(pkActor)
	{
		worldPosition = pkActor->GetPos();
		if (m_spCamera->WorldPtToScreenPt3(worldPosition, viewPosition.x, viewPosition.y, viewPosition.z))
		{
//			viewPtToScreenPt(viewPosition, screenPosition);
			//캐릭터 위치로 이동
			m_kScreenCenter.x = viewPosition.x;
			m_kScreenCenter.y = 1.0f - viewPosition.y;
		}
	}
	arrangeScreenBoundary();
}

void PgMiniMapUI::ZoomInMiniMap(int zoomLevel)
{
	if (!m_spCamera || !m_bInitialized)
		return;

	m_fZoomFactor += 0.02f;	
	if (m_fZoomFactor >= MINIMAP_ZOOMFACTOR_MAX)
		m_fZoomFactor = 0.8f;

	RefreshZoomMiniMap();
}

void PgMiniMapUI::ZoomOutMiniMap(int zoomLevel)
{
	if (!m_spCamera || !m_bInitialized)
		return;

	m_fZoomFactor -= 0.02f;
	if (m_fZoomFactor <= MINIMAP_ZOOMFACTOR_MIN)
		m_fZoomFactor = 0.0f;

	RefreshZoomMiniMap();
}

PgTrigger* PgMiniMapUI::GetTriggerInMiniMap(const POINT2 &ptWndPos, const POINT2 &pt, EMapIconTexType TexType)
{
	PgTrigger* pkRetTrigger = 0;

	NiPoint3 worldPosition;
	NiPoint3 viewPosition;
	NiPoint3 screenPosition;
	POINT2 screenPixel;
	RECT rtIconRect;

	if (m_pTriggerContainer == NULL)
		return NULL;

	PgWorld::TriggerContainer::iterator itr = m_pTriggerContainer->begin();
	while(itr != m_pTriggerContainer->end())
	{
		PgTrigger* pkTrigger = itr->second;
#ifndef EXTERNAL_RELEASE
		if (pkTrigger && pkTrigger->GetTriggerObject())
#else
		if (pkTrigger && pkTrigger->GetTriggerObject() && getIconType(pkTrigger) != ICONTYPE_NONE)
#endif
		{
			worldPosition = pkTrigger->GetTriggerObject()->GetWorldTranslate();
			if (m_spCamera->WorldPtToScreenPt3(worldPosition, viewPosition.x, viewPosition.y, viewPosition.z) == false)
			{
				++itr;
				continue;
			}
			if (viewPtToScreenPt(viewPosition, screenPosition))
			{
				screenPixel.x = (LONG)(ptWndPos.x + m_kWndSize.x * screenPosition.x);
				if( m_bAlwaysMinimap )	{ screenPixel.y = (LONG)(ptWndPos.y + m_kWndSize.y * screenPosition.y); }
				else					{ screenPixel.y = (LONG)(ptWndPos.y + (m_kWndSize.x * m_fScreenImageRatio * screenPosition.y) - m_pkDrawHeight->x + m_iMiniMapDrawGap); }
				if (!GetIconRect(rtIconRect, ptWndPos, screenPixel, TexType))
				{
					++itr;
					continue;
				}
				rtIconRect.right += rtIconRect.left;
				rtIconRect.bottom += rtIconRect.top;
				if (PtInRect(&rtIconRect, pt))
				{
					pkRetTrigger = pkTrigger;
				}
			}
		}
		++itr;
	}

	return pkRetTrigger;
}

std::wstring PgMiniMapUI::GetObjectInMiniMapParty(const POINT2 &ptWndPos, const POINT2 &pt, EMapIconTexType TexType)
{
	std::wstring kName = std::wstring();
	NiPoint3 worldPosition;
	NiPoint3 viewPosition;
	NiPoint3 screenPosition;
	POINT2 screenPixel;
	RECT rtIconRect;

//Party
	ContPartyMember kPartyMemberList;
	g_kParty.GetPartyMemberList(kPartyMemberList);
	ContPartyMember::const_iterator iter = kPartyMemberList.begin();
	for(; iter != kPartyMemberList.end(); ++iter)
	{
		worldPosition = NiPoint3((*iter)->ptPos.x, (*iter)->ptPos.y, (*iter)->ptPos.z);
	
		if( true == m_spCamera->WorldPtToScreenPt3(worldPosition, viewPosition.x, viewPosition.y, viewPosition.z) )
		{
			if (viewPtToScreenPt(viewPosition, screenPosition))
			{
				screenPixel.x = (LONG)(ptWndPos.x + m_kWndSize.x * screenPosition.x);
				if( m_bAlwaysMinimap )	{ screenPixel.y = (LONG)(ptWndPos.y + m_kWndSize.y * screenPosition.y); }
				else					{ screenPixel.y = (LONG)(ptWndPos.y + (m_kWndSize.x * m_fScreenImageRatio * screenPosition.y) - m_pkDrawHeight->x + m_iMiniMapDrawGap); }
				if(GetIconRect(rtIconRect, ptWndPos, screenPixel, TexType))
				{
					rtIconRect.right += rtIconRect.left;
					rtIconRect.bottom += rtIconRect.top;
					if (PtInRect(&rtIconRect, pt))
					{
						kName = (*iter)->kName;
					}
				}
			}
		}
	}
	return kName;
}

PgActor* PgMiniMapUI::GetObjectInMiniMap(const POINT2 &ptWndPos, const POINT2 &pt, EMapIconTexType TexType)
{
	PgActor* pkRetObject = 0;

	NiPoint3 worldPosition;
	NiPoint3 viewPosition;
	NiPoint3 screenPosition;
	POINT2 screenPixel;
	RECT rtIconRect;

	if (m_pObjectContainer == NULL)
		return NULL;

	PgWorld::ObjectContainer::iterator itr = m_pObjectContainer[WOCID_MAIN].begin();
	while(itr != m_pObjectContainer[WOCID_MAIN].end())
	{
		if (itr->second && itr->second->GetPilot() && itr->second->GetPilot()->GetWorldObject())
		{
			PgActor* pkObject = dynamic_cast<PgActor *>(itr->second->GetPilot()->GetWorldObject());
			if (!pkObject || !pkObject->GetPilot() || !pkObject->GetPilot()->GetWorldObject())
			{
				++itr;
				continue;
			}
			worldPosition = pkObject->GetPilot()->GetWorldObject()->GetWorldTranslate();
			if (m_spCamera->WorldPtToScreenPt3(worldPosition, viewPosition.x, viewPosition.y, viewPosition.z) == false)
			{
				++itr;
				continue;
			}
			if (viewPtToScreenPt(viewPosition, screenPosition))
			{
				screenPixel.x = (LONG)(ptWndPos.x + m_kWndSize.x * screenPosition.x);
				if( m_bAlwaysMinimap )	{ screenPixel.y = (LONG)(ptWndPos.y + m_kWndSize.y * screenPosition.y); }
				else					{ screenPixel.y = (LONG)(ptWndPos.y + (m_kWndSize.x * m_fScreenImageRatio * screenPosition.y) - m_pkDrawHeight->x + m_iMiniMapDrawGap); }
				if (!GetIconRect(rtIconRect, ptWndPos, screenPixel, TexType))
				{
					++itr;
					continue;
				}
				rtIconRect.right += rtIconRect.left;
				rtIconRect.bottom += rtIconRect.top;
				if (PtInRect(&rtIconRect, pt))
				{
					pkRetObject = pkObject;
				}
			}
		}
		++itr;
	}

	return pkRetObject;
}

void PgMiniMapUI::MouseOverMiniMap(const POINT2 &ptWndPos, const POINT2 &pt, char const* wndName)
{
	std::wstring wstrText;
	int iIconNo = 0;
	bool bIsToolTipChange = false;
	bool bIsToolTipClose = false;

	PgActor* pkActor = GetObjectInMiniMap(ptWndPos, pt, EMITT_SMALL);
	PgTrigger* pkTrigger = GetTriggerInMiniMap(ptWndPos, pt, EMITT_LARGE);
	if( pkActor && pkActor->GetPilot() && !pkActor->IsHide() )
	{
		std::wstring	wstrIconID;
		SResourceIcon	rkRscIcon;
		if(g_kResourceIcon.GetIconIDFromActorName(UNI(pkActor->GetID()), wstrIconID))
		{
			if(g_kResourceIcon.GetIcon(rkRscIcon, wstrIconID))
			{
				if(rkRscIcon.wstrImageID.compare(NPC_ICON_TYPE_NAME) == 0)
				{
					iIconNo = rkRscIcon.iIdx;
				}
			}
		}
		wstrText = pkActor->GetPilot()->GetName();
		bIsToolTipChange = true;
	}
	else if( pkTrigger )
	{
		int iTextID = pkTrigger->GetTriggerTitleTextID();
		if( iTextID )
		{
			wstrText = TTW(iTextID);
			bIsToolTipChange = true;
		}
		else
		{
			bIsToolTipClose = true;
		}
	}
	else
	{
		wstrText = GetObjectInMiniMapParty(ptWndPos, pt, EMITT_MYPARTY);
		if( wstrText.empty() )
		{
			bIsToolTipClose = true;
		}
		else
		{
			bIsToolTipChange = true;
		}
	}

	if( bIsToolTipChange )
	{
		m_bIsShowToolTip = true;
		lwPoint2 kPoint(pt);
		lwCallMutableToolTipByText(wstrText, kPoint, iIconNo, wndName);
	}
	
	if( bIsToolTipClose )
	{
		m_bIsShowToolTip = false;
		m_strToolTipID.clear();
		lwCloseToolTip();
	}
}

void PgMiniMapUI::MouseClickMiniMap(const POINT2 &ptWndPos, const POINT2 &pt)
{
	if (!g_pkWorld)
	{
		return;
	}

	PgTrigger* pkTrigger = GetTriggerInMiniMap(ptWndPos, pt, EMITT_LARGE);
	PgActor* pkActor = GetObjectInMiniMap(ptWndPos, pt, EMITT_SMALL);
	// TYPE 2
	// 캐릭터 위에 화살표 띄움
	if (pkTrigger)
	{
		g_pkWorld->SetDirectionArrow(pkTrigger->GetTriggerObject());
	}
	else if(pkActor && pkActor->GetPilot())
	{
		g_pkWorld->SetDirectionArrow(pkActor->GetPilot()->GetWorldObject());
	}
#ifndef USE_INB
	if (!g_bAbleSetTeleMove)
	{
		return;
	}
	// TYPE 1
	// 클릭한 위치로 캐릭터 이동.
	NiPoint3 kPoint = NiPoint3::ZERO;
	if (pkTrigger)
	{
		kPoint = pkTrigger->GetTriggerObjectPos();
	}
	else if(pkActor && pkActor->GetPilot())
	{
		kPoint = pkActor->GetPos();
	}

	if (kPoint != NiPoint3::ZERO)
	{
		kPoint.z += 80;
		PgActor* pkActor = g_kPilotMan.GetPlayerActor();
		if (pkActor)
		{
			lwActor kActor = lwActor(pkActor);
			kActor.SetTranslate(lwPoint3(kPoint),false);
		}
	}
#endif
}

bool PgMiniMapUI::UpdateQuest()
{
	if( !m_pObjectContainer )
	{
		return false;
	}

	m_kQuestInfoCont.clear();
	if( !m_bShowQuest )
	{
		return false;
	}

	PgPlayer *pkPC = g_kPilotMan.GetPlayerUnit();
	if( !pkPC )
	{
		return false;
	}

	int const iPlayerLevel = pkPC->GetAbil(AT_LEVEL);

	PgWorld::ObjectContainer::iterator iter = m_pObjectContainer[WOCID_MAIN].begin();
	for(; m_pObjectContainer[WOCID_MAIN].end() != iter; ++iter)
	{
		PgWorld::ObjectContainer::mapped_type& pkElement = (*iter).second;
		
		if( !pkElement ) continue;
		PgPilot* pkPilot = pkElement->GetPilot();
		if( !pkPilot ) continue;
		if( !pkPilot->GetWorldObject() ) continue;
		PgActor* pkActor = dynamic_cast<PgActor*>(pkPilot->GetWorldObject());
		if (!pkActor) continue;

		BM::GUID const &rkGuid = pkActor->GetGuid();

		ContNpcQuestInfo kQuestInfoCont, kResultVec;
		size_t const iCountRet = g_kQuestMan.PopNPCQuestInfo(rkGuid, kQuestInfoCont);
		if( !iCountRet ) continue;

		//10레벨 이상 차이나는 시작가능한 퀘스트 표시 생략
		std::remove_copy_if(kQuestInfoCont.begin(), kQuestInfoCont.end(), std::back_inserter(kResultVec), SNPCQuestInfo::SPlayerLevelDiff(iPlayerLevel));

		const EQuestState eState = PgQuestManUtil::QuestOrderByState(rkGuid, kResultVec);
		m_kQuestInfoCont.insert( std::make_pair(rkGuid, eState) );
	}

	return true;
}

bool PgMiniMapUI::Update(float fAccumTime, float fFrameTime)
{
	if (m_bInitialized == false)
		return false;

	//! 스크린 위치를 캐릭터가 가운데로 오게 만든다
	NiPoint3 worldPosition;
	NiPoint3 viewPosition;
//	NiPoint3 screenPosition;

	PgActor *pkActor = g_kPilotMan.GetPlayerActor();
	if(pkActor)
	{
		worldPosition = pkActor->GetPos();
		if (m_spCamera->WorldPtToScreenPt3(worldPosition, viewPosition.x, viewPosition.y, viewPosition.z))
		{
//			viewPtToScreenPt(viewPosition, screenPosition);

			m_kScreenCenter.x = viewPosition.x;
			m_kScreenCenter.y = 1.0f - viewPosition.y;
			
		}
	}

	arrangeScreenBoundary();

 	return true;
}

void PgMiniMapUI::Draw(PgRenderer *pkRenderer)
{
	if(pkRenderer && m_bInitialized)
	{
		if (m_spMiniMapScreenTexture)
		{
			PgUIScene::Render_UIObject(pkRenderer,m_spMiniMapScreenTexture);
		}

		kMapIconTexContainer::iterator iter = m_kMapIconTexCont.begin();
		while( iter != m_kMapIconTexCont.end() )
		{
			if( iter->second.IconTexture )
			{
				PgUIScene::Render_UIObject(pkRenderer, iter->second.IconTexture);
			}
			++iter;
		}

		if( m_bIsShowText )
		{
			kMapTextContainer::iterator txt_iter = m_kMapRenderTextCont.begin();
			while( txt_iter != m_kMapRenderTextCont.end() )
			{
				txt_iter->second.Render(pkRenderer);
				++txt_iter;
			}
		}
	}
}

PgUITexture* PgMiniMapUI::GetTex()
{
	if (m_bInitialized)
	{
		// niscreenelement를 쓰는 경우는 일단 null
		return NULL;
	}
	return NULL;
}

void PgMiniMapUI::ScrollMiniMap(NiPoint2 direction)
{
	float scaleFactor = m_fZoomFactor / 2;
	PG_ASSERT_LOG (scaleFactor >= 0.0f && scaleFactor <= 0.4f);
	float screenHalfWidth = (1.0f - scaleFactor * 2) / 2;
	float screenHalfHeight = (1.0f - scaleFactor * 2) / 2;
	float xDelta = direction.x * screenHalfWidth / 15;
	float yDelta = direction.y * screenHalfHeight / 15;

	m_kScreenCenter.x += xDelta;
	m_kScreenCenter.y += yDelta;

	arrangeScreenBoundary();
}

void PgMiniMapUI::SetWndSize(POINT2 kWndSize)
{
	m_kWndSize = kWndSize;
}

std::string& PgMiniMapUI::GetMiniMapImageName()
{
	return m_kMiniMapImageName;
}

void PgMiniMapUI::SetZoomFactor(float fZoomFactor)
{
	if (fZoomFactor > 0.8f)
		fZoomFactor = 0.8f;
	if (fZoomFactor < 0.0f)
		fZoomFactor = 0.0f;
	m_fZoomFactor = fZoomFactor;
	// RefreshZoomMiniMap 를 해줘야 바뀐다.
}

void PgMiniMapUI::CloneFromSrc(PgMiniMapUI* pkSrcMiniMap)
{
	PG_ASSERT_LOG(pkSrcMiniMap);
	if (pkSrcMiniMap == NULL)
		return;

	m_spRoot = pkSrcMiniMap->GetModelRoot();
	m_spCamera = pkSrcMiniMap->GetCamera();
	m_spMiniMapScreenTexture = pkSrcMiniMap->GetMiniMapScreenTexture();
	m_spModelScreenTexture = pkSrcMiniMap->GetModelScreenTexture();
	m_spRenderTargetGroup = pkSrcMiniMap->GetRenderTargetGroup();
	m_spRenderTexture = pkSrcMiniMap->GetRenderTexture();
	m_pObjectContainer = pkSrcMiniMap->GetObjectContainer();
	m_pTriggerContainer = pkSrcMiniMap->GetTriggerContainer();
	m_spMiniMapImage = pkSrcMiniMap->GetMiniMapImage();
	m_kMapIcons = pkSrcMiniMap->GetMapIcons();
	m_kMapLeftIcons = pkSrcMiniMap->GetMapLeftIcons();
	m_kMapRightIcons = pkSrcMiniMap->GetMapRightIcons();
	m_kMapIconTexCont.insert(pkSrcMiniMap->GetMapIconTexCont().begin(), pkSrcMiniMap->GetMapIconTexCont().end());
	
	Initialize(m_spRoot, m_pObjectContainer, m_pTriggerContainer, m_spCamera, 
        pkSrcMiniMap->GetMiniMapImageName(), m_pkDrawHeight);
	SetAlwayType(pkSrcMiniMap->GetAlwayType());
	m_bInitialized = true;
}

void PgMiniMapUI::GetWorldBounds(NiAVObject *pkObject, NiBound &kBound)
{
	if (pkObject == NULL)
		return;

	if (NiIsKindOf(NiNode, pkObject))
	{
		//GetWorldBoundsNode((NiNode *)pkObject, kBound);
		kBound = pkObject->GetWorldBound();
	}
	else if (NiIsKindOf(NiTriBasedGeom, pkObject))
	{
		NiGeometry* pkGeom = (NiGeometry *) pkObject;
		NiPoint3* pkVerts = pkGeom->GetVertices();

		NiBound kChildBound;
		kChildBound.ComputeFromData(pkGeom->GetVertexCount(), pkVerts);

		NiSkinInstance *pkSkin = pkGeom->GetSkinInstance();
		if (pkSkin != NULL)
		{
			pkSkin->UpdateModelBound(kChildBound);
		}

		kBound.Update(kChildBound, pkGeom->GetWorldTransform());
	}
}

void PgMiniMapUI::GetWorldBoundsNode(NiNode *pkObject, NiBound &kBound)
{
	NiBound kWorldBound;
	kWorldBound.SetRadius(0.0f);

	for (unsigned int i = 0; i < pkObject->GetArrayCount(); i++)
	{
		NiAVObject* pkChild = pkObject->GetAt(i);
		if (pkChild)
		{
			NiBound kChildBound;
			GetWorldBounds(pkChild, kChildBound);

			if (kChildBound.GetRadius() > 0.0f)
			{
				if (kWorldBound.GetRadius() == 0.0f)
				{
					kWorldBound = kChildBound;
				}
				else
				{
					kWorldBound.Merge(&kChildBound);
				}
			}
		}
	}

	kBound = kWorldBound;
}

void PgMiniMapUI::RenderImmediate(NiRenderer* pkRenderer, NiAVObject *pkObject)
{
	if(NiIsKindOf(NiGeometry, pkObject))
	{
		((NiGeometry *)pkObject)->RenderImmediate(pkRenderer);
	} 
	else if(NiIsKindOf(NiNode, pkObject))
	{
		NiNode *pkNode = (NiNode *)pkObject;

		for(unsigned int i = 0;
			i < ((NiNode *)pkObject)->GetArrayCount();
			i++)
		{
			RenderImmediate(pkRenderer, pkNode->GetAt(i));
		}
	}
}

bool PgMiniMapUI::viewPtToScreenPt(NiPoint3 viewPt, NiPoint3& screenPt)
{
	bool result = true;
	float scaleFactor = m_fZoomFactor / 2;
	PG_ASSERT_LOG (scaleFactor >= 0.0f && scaleFactor <= 0.4f);
	float screenHalfWidth = (1.0f - scaleFactor * 2) / 2;
	float screenHalfHeight = (1.0f - scaleFactor * 2) / 2;

	float screenLeft = m_kScreenCenter.x - screenHalfWidth;
	float screenTop = m_kScreenCenter.y - screenHalfHeight;

	//! screen 밖에 있더라도, 상대적인 위치를 return.
	if (viewPt.x < screenLeft || viewPt.x > screenLeft + 2 * screenHalfWidth)
		result = false;

	if ((1 - viewPt.y) < screenTop || (1 - viewPt.y) > screenTop + 2 * screenHalfHeight)
		result = false;

	screenPt.x = (viewPt.x - screenLeft) / (2 * screenHalfWidth);
	screenPt.y = ((1 - viewPt.y) - screenTop) / (2 * screenHalfHeight);

	return result;
}

bool PgMiniMapUI::addOtherWorldActorToMiniMap(PgActor* actor, const POINT2 &ptWndPos, IconType eIconType)
{
	IconType iconType = ICONTYPE_NONE;
	POINT2 iconTexturePos(0,0);

	if (actor == NULL)
		return false;
	if (eIconType == ICONTYPE_NONE)
		iconType = getIconType(actor);
	else
		iconType = eIconType;

	if (iconType == ICONTYPE_NONE)
		return false;

	bool const bGetIconRet = getIconTexturePosByType(iconType, iconTexturePos);
	if( !bGetIconRet )
	{
		return false;
	}

	MiniMapIconData iconData;
	if (iconType == ICONTYPE_PARTY_LEFT_ARROW)
		iconData.screenPixel.x = ptWndPos.x + MINIMAP_ICON_HALF_SIZE;
	else if (iconType == ICONTYPE_PARTY_RIGHT_ARROW)
		iconData.screenPixel.x = ptWndPos.x + m_kWndSize.x - MINIMAP_ICON_SIZE;
	if( m_bAlwaysMinimap )	{ iconData.screenPixel.y = (LONG)(ptWndPos.y + m_kWndSize.y * 0.5f - MINIMAP_ICON_HALF_SIZE); }
	else					{ iconData.screenPixel.y = (LONG)(ptWndPos.y + (m_spMiniMapImage->GetHeight() * 0.5f) - MINIMAP_ICON_HALF_SIZE - m_pkDrawHeight->x); }
	iconData.viewPosition = NiPoint3(0,0,0);
	iconData.pData = actor;
	iconData.iconType = iconType;

	if (iconType == ICONTYPE_PARTY_LEFT_ARROW)
		m_kMapLeftIcons.push_back(iconData);
	else if (iconType == ICONTYPE_PARTY_RIGHT_ARROW)
		m_kMapRightIcons.push_back(iconData);

	addMiniMapIcon(ptWndPos, iconData.screenPixel, iconTexturePos);
	return true;
}

bool PgMiniMapUI::addPositionToMiniMap(NiPoint3 const& WorldPt, POINT2 const& ptWndPos, IconType eIconType)
{
	NiPoint3 worldPosition;
	NiPoint3 viewPosition;
	NiPoint3 screenPosition;
	IconType iconType = ICONTYPE_NONE;
	POINT2 iconTexturePos(0,0);

	if( WorldPt == NiPoint3::ZERO )
		return false;
	else if (eIconType == ICONTYPE_NONE)
		return false;
	else
		iconType = eIconType;

	worldPosition = NiPoint3(WorldPt.x, WorldPt.y, WorldPt.z);
	bool const bGetIconRet = getIconTexturePosByType(iconType, iconTexturePos);
	if( !bGetIconRet )
	{
		return false;
	}

	if (m_spCamera->WorldPtToScreenPt3(worldPosition, viewPosition.x, viewPosition.y, viewPosition.z) == false)
		return false;

	if (viewPtToScreenPt(viewPosition, screenPosition))
	{
		MiniMapIconData iconData;
		iconData.screenPixel.x = (LONG)(ptWndPos.x + m_kWndSize.x * screenPosition.x);
		if( m_bAlwaysMinimap )	{ iconData.screenPixel.y = (LONG)(ptWndPos.y + m_kWndSize.y * screenPosition.y); }
		else					{ iconData.screenPixel.y = (LONG)(ptWndPos.y + (m_kWndSize.x * m_fScreenImageRatio * screenPosition.y) - m_pkDrawHeight->x + m_iMiniMapDrawGap); }
		iconData.viewPosition = viewPosition;

		// TODO: 매 프레임 지우고 넣는게 아니라 좌표만 바꾸는 방식으로 바꾸자
		//m_kMapIcons.push_back(iconData); 
		float fAlpha = 1.0f;
		switch( iconType )
		{
		case ICONTYPE_EMPORIA_MAIN_RED:
		case ICONTYPE_EMPORIA_MAIN_BLUE:
			{
				addMiniMapIcon(ptWndPos, iconData.screenPixel, iconTexturePos, fAlpha , EMITT_CORE);
			}break;
		case ICONTYPE_EMPORIA_SUB_RED:
		case ICONTYPE_EMPORIA_SUB_BLUE:
			{
				addMiniMapIcon(ptWndPos, iconData.screenPixel, iconTexturePos, fAlpha , EMITT_SUB);
			}break;
		case ICONTYPE_PARTY:
			{
				MiniMapAniIconCont::const_iterator	c_iter = m_kAniIconCont.find(eIconType);
				if( c_iter != m_kAniIconCont.end() )
				{
					
					MiniMapAniIconContainer::const_iterator	e_c_iter = c_iter->second.begin();
					if( e_c_iter != c_iter->second.end() )
					{
						iconTexturePos.x = (e_c_iter->NowFrame() % e_c_iter->UVInfo().x) * MINIMAP_MEICON_SIZE;
						iconTexturePos.y = (e_c_iter->NowFrame() / e_c_iter->UVInfo().x) * MINIMAP_MEICON_SIZE;
						addMiniMapIcon(ptWndPos, iconData.screenPixel, iconTexturePos, fAlpha , EMITT_MYPARTY);
					}
				}
			}break;
		default:
			{
				addMiniMapIcon(ptWndPos, iconData.screenPixel, iconTexturePos, fAlpha , EMITT_LARGE);
			}break;
		}
		return true;
	}

	return false;
}

bool PgMiniMapUI::addActorToMiniMap(PgActor* actor, const POINT2 &ptWndPos, IconType eIconType)
{
	NiPoint3 worldPosition;
	NiPoint3 viewPosition;
	NiPoint3 screenPosition;
	IconType iconType = ICONTYPE_NONE;
	POINT2 iconTexturePos(0,0);

	if (actor == NULL)
		return false;
	if (eIconType == ICONTYPE_NONE)
		iconType = getIconType(actor);
	else
		iconType = eIconType;

	if (iconType == ICONTYPE_NONE)
		return false;

#if !defined(EXTERNAL_RELEASE)
	switch(iconType)
	{
	case ICONTYPE_ME:
		PG_ASSERT_LOG(actor->IsMyActor());
		break;
	//case ICONTYPE_MYPET:
	//	PG_ASSERT_LOG(actor->IsMyPet());
	//	break;
	case ICONTYPE_PARTY:
		PG_ASSERT_LOG(actor->GetObjectID() == PgIXmlObject::ID_PC);
		break;
	case ICONTYPE_GUILD:
		PG_ASSERT_LOG(actor->GetObjectID() == PgIXmlObject::ID_PC);
		break;
	case ICONTYPE_COUPLE:
		PG_ASSERT_LOG(actor->GetObjectID() == PgIXmlObject::ID_PC);
		break;
	case ICONTYPE_NPC:
		PG_ASSERT_LOG(actor->GetObjectID() == PgIXmlObject::ID_NPC);
		break;
	}
#endif

	worldPosition = actor->GetPos();
	bool const bGetIconRet = getIconTexturePosByType(iconType, iconTexturePos);
	if( !bGetIconRet )
	{
		return false;
	}

	if (m_spCamera->WorldPtToScreenPt3(worldPosition, viewPosition.x, viewPosition.y, viewPosition.z) == false)
		return false;

	if (viewPtToScreenPt(viewPosition, screenPosition))
	{
		MiniMapIconData iconData;
		iconData.screenPixel.x = (LONG)(ptWndPos.x + m_kWndSize.x * screenPosition.x);
		if( m_bAlwaysMinimap )	{ iconData.screenPixel.y = (LONG)(ptWndPos.y + m_kWndSize.y * screenPosition.y); }
		else					{ iconData.screenPixel.y = (LONG)(ptWndPos.y + (m_kWndSize.x * m_fScreenImageRatio * screenPosition.y) - m_pkDrawHeight->x + m_iMiniMapDrawGap); }
		iconData.viewPosition = viewPosition;
		iconData.pData = actor;
		iconData.iconType = iconType;

		// TODO: 매 프레임 지우고 넣는게 아니라 좌표만 바꾸는 방식으로 바꾸자
		//m_kMapIcons.push_back(iconData); 
		float fAlpha = 1.0f;
#ifndef EXTERNAL_RELEASE
		switch(actor->GetInvisibleGrade())
		{
		case PgActor::INVISIBLE_NEAR:
			fAlpha = 0.8f;
			break;
		case PgActor::INVISIBLE_MIDDLE:
			fAlpha = 0.6f;
			break;
		case PgActor::INVISIBLE_FAR:
			fAlpha = 0.4f;
			break;
		case PgActor::INVISIBLE_FARFAR:
			fAlpha = 0.2f;
			break;
		}
#endif
		if( eIconType == ICONTYPE_ME || eIconType == ICONTYPE_PARTY )
		{
			MiniMapAniIconCont::const_iterator	c_iter = m_kAniIconCont.find(eIconType);
			if( c_iter != m_kAniIconCont.end() )
			{
				MiniMapAniIconContainer::const_iterator	e_c_iter = c_iter->second.begin();
				if( e_c_iter != c_iter->second.end() )
				{
					iconTexturePos.x = (e_c_iter->NowFrame() % e_c_iter->UVInfo().x) * MINIMAP_MEICON_SIZE;
					iconTexturePos.y = (e_c_iter->NowFrame() / e_c_iter->UVInfo().x) * MINIMAP_MEICON_SIZE;
					addMiniMapIcon(ptWndPos, iconData.screenPixel, iconTexturePos, fAlpha , (ICONTYPE_ME == eIconType)?(EMITT_MYACTOR):(EMITT_MYPARTY));
				}
			}
		}
		else
		{
			addMiniMapIcon(ptWndPos, iconData.screenPixel, iconTexturePos, fAlpha);
		}
		return true;
	}

	return false;
}

bool PgMiniMapUI::addTriggerToMiniMap(PgTrigger* trigger, const POINT2 &ptWndPos, IconType eIconType)
{
	NiPoint3 worldPosition;
	NiPoint3 viewPosition;
	NiPoint3 screenPosition;
	IconType iconType = ICONTYPE_NONE;
	POINT2 iconTexturePos(0,0);

	if (trigger == NULL || trigger->GetTriggerObject() == NULL)
		return false;
	if (eIconType == ICONTYPE_NONE)
		iconType = getIconType(trigger);
	else
		iconType = eIconType;

	// TODO : NONE은 나중에 false 해줘야 함...
	if (iconType == ICONTYPE_NONE)
	{
		return false;
	}

	if( trigger->GetTriggerType() == PgTrigger::TRIGGER_TYPE_LOCATION )
	{
		int iTriggerNo = trigger->Param();
		int iQuestID = trigger->Param2();

		PgPlayer *pkPC = g_kPilotMan.GetPlayerUnit();
		if( pkPC )
		{
			PgMyQuest const *pkMyQuest = pkPC->GetMyQuest();
			if(!pkMyQuest)
			{
				return false;
			}
			
			bool const bIngRet = pkMyQuest->IsIngQuest(iQuestID);
			if(!bIngRet)
			{
				return false;
			}

			const PgQuestInfo* pkQuestInfo = NULL;
			pkQuestInfo = g_kQuestMan.GetQuest(iQuestID);
			if( !pkQuestInfo )
			{
				return false;
			}

			const ContQuestLocation &rkDependLocation = pkQuestInfo->m_kDepend_Location;
			ContQuestLocation::const_iterator location_iter = rkDependLocation.begin();
			while( rkDependLocation.end() != location_iter )
			{
				const ContQuestLocation::value_type rkElement = (*location_iter);
				
				const SUserQuestState *pkState = pkMyQuest->Get(iQuestID);
				if( pkState )
				{
					if( ((QS_Ing == pkState->byQuestState) || (QS_End == pkState->byQuestState)) && (rkElement.iType == QET_LOCATION_LocationEnter)
						&& pkState->byParam[rkElement.iObjectNo] > 0
						&& rkElement.iGroundNo == g_kNowGroundKey.GroundNo()
						&& rkElement.iLocationNo == iTriggerNo
						)
					{
						return false;
					}
				}
				++location_iter;
			}
		}
	}

	worldPosition = trigger->GetTriggerObject()->GetWorldTranslate();
	bool const bGetIconRet = getIconTexturePosByType(iconType, iconTexturePos);
	if( !bGetIconRet )
	{
		return false;
	}

	if (m_spCamera->WorldPtToScreenPt3(worldPosition, viewPosition.x, viewPosition.y, viewPosition.z) == false)
		return false;

	int const iTextID = trigger->GetTriggerTitleTextID();
	if (viewPtToScreenPt(viewPosition, screenPosition))
	{
		MiniMapIconData iconData;
		iconData.screenPixel.x = (LONG)(ptWndPos.x + m_kWndSize.x * screenPosition.x);
		if( m_bAlwaysMinimap )	{ iconData.screenPixel.y = (LONG)(ptWndPos.y + m_kWndSize.y * screenPosition.y); }
		else					{ iconData.screenPixel.y = (LONG)(ptWndPos.y + (m_kWndSize.x * m_fScreenImageRatio * screenPosition.y) - m_pkDrawHeight->x + m_iMiniMapDrawGap); }
		iconData.viewPosition = viewPosition;
		iconData.pData = trigger;
		iconData.iconType = iconType;

		// TODO: 매 프레임 지우고 넣는게 아니라 좌표만 바꾸는 방식으로 바꾸자
		//m_kMapIcons.push_back(iconData); 
		double radian = 0;
		if (iconType == ICONTYPE_TELEJUMP && g_pkWorld && m_spCamera)
		{
			radian = trigger->GetRotation();
			if(radian == -9999)
			{
				lwWorld kWorld = lwWorld(g_pkWorld);
				lwPoint3 kPoint3 = kWorld.GetObjectPosByName(MB(trigger->ParamString()));
				NiPoint3 worldPos = kPoint3();
				// 두 점의 각도를 구하자.
				NiPoint3 viewPos = NiPoint3::ZERO;
				m_spCamera->WorldPtToScreenPt3(worldPos, viewPos.x, viewPos.y, viewPos.z);
				NiPoint2 screenPix;
				screenPix.x = viewPos.x - viewPosition.x;
				screenPix.y = viewPos.y - viewPosition.y;
				screenPix.Unitize();
				radian = atan2( (double)screenPix.y, (double)screenPix.x );

				trigger->SetRotation(radian);
			}

		}

		if( ICONTYPE_PORTAL == iconType && 0 != iTextID )
		{
			kMapTextContainer::_Pairib Result = m_kMapRenderTextCont.insert(std::make_pair(iTextID, SMiniMapRenderText()));
			if( Result.second )
			{
				Result.first->second.SetStr(TTW(iTextID), _T("Font_Text_Small"), COLOR_WHITE);
			}
			POINT TextSize = Result.first->second.GetSize();
			POINT2 PtLoc(iconData.screenPixel.x, iconData.screenPixel.y);

			if( (PtLoc.x - ptWndPos.x) < (TextSize.x / 2) )
			{ 
				PtLoc.x = ptWndPos.x + 1; 
			}
			else
			{
				PtLoc.x -= (TextSize.x / 2);
			}

			if( PtLoc.y < ptWndPos.y || ( PtLoc.y - TextSize.y ) < ptWndPos.y )
			{
				PtLoc.y += MINIMAP_LARGEICON_HALF_SIZE;
			}
			else
			{
				PtLoc.y -= TextSize.y;
			}
			int iGab = (PtLoc.x + TextSize.x) - (m_kWndSize.x + ptWndPos.x);
			if( 0 < iGab ) { PtLoc.x -= iGab; }
			iGab = (PtLoc.y + TextSize.y) - (m_kWndSize.y + ptWndPos.y);
			if( 0 < iGab ) { PtLoc.y -= iGab; }

			Result.first->second.SetAttr(PtLoc, NiColorA(0.95f, 0.92f, 0.78f, 1.f), NiColorA(0.f, 0.f, 0.f, 1.f));
		}

		float fAlpha = 1.0f;
		addMiniMapIcon(ptWndPos, iconData.screenPixel, iconTexturePos, fAlpha, EMITT_LARGE, radian);
		return true;
	}
	else
	{
		kMapTextContainer::iterator txt_iter = m_kMapRenderTextCont.find(iTextID);
		if( m_kMapRenderTextCont.end() != txt_iter )
		{
			m_kMapRenderTextCont.erase(txt_iter);
		}		
	}

	return false;
}

bool PgMiniMapUI::addQuestToMiniMap(PgActor* pkActor, const EQuestState eState, const POINT2 &ptWndPos, IconType eIconType)
{
	if( pkActor == NULL )
	{
		return false;
	}

	NiPoint3 worldPosition;
	NiPoint3 viewPosition;
	NiPoint3 screenPosition;
	//IconType iconType = ICONTYPE_NONE;
	POINT2 iconTexturePos(0,0);

	if( ICONTYPE_NONE == eIconType )
	{
		switch( eState )
		{
		case QS_Begin_Story:		{ eIconType = ICONTYPE_QUEST_BEGIN_STORY; }break;
		case QS_End_Story:			{ eIconType = ICONTYPE_QUEST_END_STORY; }break;
		case QS_Begin:				{ eIconType = ICONTYPE_QUEST_BEGIN; }break;
		case QS_Ing:				{ eIconType = ICONTYPE_QUEST_ING; }break;
		case QS_Begin_NYet:			{ eIconType = ICONTYPE_QUEST_NOTYET; }break;
		case QS_End:				{ eIconType = ICONTYPE_QUEST_END; }break;
		case QS_Begin_Loop:			{ eIconType = ICONTYPE_QUEST_BEGIN_LOOP; }break;
		case QS_End_Loop:			{ eIconType = ICONTYPE_QUEST_END_LOOP; }break;
		case QS_Begin_Tactics:		{ eIconType = ICONTYPE_QUEST_BEGIN_GUILD; }break;
		case QS_End_Tactics:		{ eIconType = ICONTYPE_QUEST_END_GUILD; }break;
		case QS_Begin_Weekly:		{ eIconType = ICONTYPE_QUEST_BEGIN_WEEKLY; }break;
		case QS_End_Weekly:			{ eIconType = ICONTYPE_QUEST_END_WEEKLY; }break;
		case QS_Begin_Couple:		{ eIconType = ICONTYPE_QUEST_BEGIN_COUPLE; }break;
		case QS_End_Couple:			{ eIconType = ICONTYPE_QUEST_END_COUPLE; }break;
		default:
			return false;
		}
	}

	worldPosition = pkActor->GetPos();
	bool const bGetIconRet = getIconTexturePosByType(eIconType, iconTexturePos);
	if( !bGetIconRet )
	{
		return false;
	}

	if (m_spCamera->WorldPtToScreenPt3(worldPosition, viewPosition.x, viewPosition.y, viewPosition.z) == false)
	{
		return false;
	}
	if (viewPtToScreenPt(viewPosition, screenPosition))
	{
		MiniMapIconData iconData;
		iconData.screenPixel.x = (LONG)(ptWndPos.x + m_kWndSize.x * screenPosition.x);
		if( m_bAlwaysMinimap )	{ iconData.screenPixel.y = (LONG)(ptWndPos.y + m_kWndSize.y * screenPosition.y); }
		else					{ iconData.screenPixel.y = (LONG)(ptWndPos.y + (m_kWndSize.x * m_fScreenImageRatio * screenPosition.y) - m_pkDrawHeight->x + m_iMiniMapDrawGap); }
		iconData.viewPosition = viewPosition;
		iconData.pData = pkActor;
		iconData.iconType = eIconType;

		float fAlpha = 1.0f;
		addMiniMapIcon(ptWndPos, iconData.screenPixel, iconTexturePos, fAlpha, EMITT_LARGE);
		return true;
	}

	return false;
}

bool PgMiniMapUI::GetIconRect(RECT& rtIconRect, const POINT2& ptWndPos, const POINT2& screenPixel, EMapIconTexType TexType)
{
	//! 현재 보이는 화면 안에 있다.				
	kMapIconTexContainer::iterator iter = m_kMapIconTexCont.find(TexType);
	if( iter == m_kMapIconTexCont.end() )
	{
		return false;
	}

	NiScreenElementsPtr spIconTexture = iter->second.IconTexture;
	int iIconSize = iter->second.W;
	int iIconHalfSize = iIconSize * 0.5f;

	if (screenPixel.x - iIconHalfSize < ptWndPos.x || screenPixel.x + iIconHalfSize > ptWndPos.x + m_kWndSize.x ||
		screenPixel.y - iIconHalfSize < ptWndPos.y || screenPixel.y + iIconHalfSize > ptWndPos.y + m_kWndSize.y)
	{
		//! 아이콘이 완전히 안보이는 경우라면
		if (screenPixel.x + iIconHalfSize < ptWndPos.x || screenPixel.x - iIconHalfSize > ptWndPos.x + m_kWndSize.x ||
			screenPixel.y + iIconHalfSize < ptWndPos.y || screenPixel.y - iIconHalfSize > ptWndPos.y + m_kWndSize.y)
			return false;

		int left = 0;
		int right = 0;
		int top = 0;
		int bottom = 0;

		if (screenPixel.x - iIconHalfSize < ptWndPos.x)
			left = ptWndPos.x - (screenPixel.x - iIconHalfSize);
		if (screenPixel.x + iIconHalfSize > ptWndPos.x + m_kWndSize.x)
			right = screenPixel.x + iIconHalfSize - (ptWndPos.x + m_kWndSize.x);
		if (screenPixel.y - iIconHalfSize < ptWndPos.y)
			top = ptWndPos.y - (screenPixel.y - iIconHalfSize);
		if (screenPixel.y + iIconHalfSize > ptWndPos.y + m_kWndSize.y)
			bottom = screenPixel.y + iIconHalfSize - (ptWndPos.y + m_kWndSize.y);

		PG_ASSERT_LOG (!(left && right));
		PG_ASSERT_LOG (!(top && bottom));

		rtIconRect.top = (short)(screenPixel.y - iIconHalfSize + top);
		rtIconRect.left = (short)(screenPixel.x - iIconHalfSize + left);
		rtIconRect.right = (unsigned short)(iIconSize - left - right);
		rtIconRect.bottom = (unsigned short)(iIconSize - top - bottom);
	}
	else
	{
		rtIconRect.top = (short)(screenPixel.y - iIconHalfSize);
		rtIconRect.left = (short)(screenPixel.x - iIconHalfSize);
		rtIconRect.right = (unsigned short)(iIconSize);
		rtIconRect.bottom = (unsigned short)iIconSize;
	}

	return true;
}

void PgMiniMapUI::addMiniMapIcon(const POINT2& ptWndPos, const POINT2& screenPixel, const POINT2& iconTexturePos, float fAlpha, EMapIconTexType TexType, float fDir)
{
	//! 현재 보이는 화면 안에 있다.				
	RECT rtIconRect;
	if (!GetIconRect(rtIconRect, ptWndPos, screenPixel, TexType))
	{
		return;
	}

	kMapIconTexContainer::iterator iter = m_kMapIconTexCont.find(TexType);
	if( iter == m_kMapIconTexCont.end() )
	{
		return;
	}

	NiScreenElementsPtr spIconTexture = iter->second.IconTexture;
	int iIconSize = iter->second.W;
	int iIconHalfSize = iIconSize * 0.5f;


	float uiScreenWidth = PRERENDER_MINIMAP_WIDTH;
	float uiScreenHeight = PRERENDER_MINIMAP_HEIGHT;

	unsigned int uiWidth = 0;
	unsigned int uiHeight = 0;
	NiTexture* pTexture = 0;
	NiTListIterator kPos = spIconTexture->GetPropertyList().GetHeadPos();
	while (kPos)
	{
		NiProperty* pProperty = spIconTexture->GetPropertyList().GetNext(kPos);
		if (pProperty && pProperty->Type() == NiProperty::TEXTURING)
		{
			NiTexturingProperty* spTex = NiDynamicCast(NiTexturingProperty, pProperty);
			if (spTex)
			{
				pTexture = spTex->GetBaseTexture();
				uiWidth = pTexture->GetWidth();
				uiHeight = pTexture->GetHeight();
			}
		}
	}

	if (screenPixel.x - iIconHalfSize < ptWndPos.x || screenPixel.x + iIconHalfSize > ptWndPos.x + m_kWndSize.x ||
		screenPixel.y - iIconHalfSize < ptWndPos.y || screenPixel.y + iIconHalfSize > ptWndPos.y + m_kWndSize.y)
	{
		//! 아이콘이 완전히 안보이는 경우라면
		if (screenPixel.x + iIconHalfSize < ptWndPos.x || screenPixel.x - iIconHalfSize > ptWndPos.x + m_kWndSize.x ||
			screenPixel.y + iIconHalfSize < ptWndPos.y || screenPixel.y - iIconHalfSize > ptWndPos.y + m_kWndSize.y)
			return;

		int left = 0;
		int right = 0;
		int top = 0;
		int bottom = 0;

		if (screenPixel.x - iIconHalfSize < ptWndPos.x)
			left = ptWndPos.x - (screenPixel.x - iIconHalfSize);
		if (screenPixel.x + iIconHalfSize > ptWndPos.x + m_kWndSize.x)
			right = screenPixel.x + iIconHalfSize - (ptWndPos.x + m_kWndSize.x);
		if (screenPixel.y - iIconHalfSize < ptWndPos.y)
			top = ptWndPos.y - (screenPixel.y - iIconHalfSize);
		if (screenPixel.y + iIconHalfSize > ptWndPos.y + m_kWndSize.y)
			bottom = screenPixel.y + iIconHalfSize - (ptWndPos.y + m_kWndSize.y);

		PG_ASSERT_LOG (!(left && right));
		PG_ASSERT_LOG (!(top && bottom));

		{
			if (pTexture)
			{
				int iPoly = spIconTexture->Insert(4);
				spIconTexture->SetRectangle(iPoly, 
					(rtIconRect.left/uiScreenWidth), (rtIconRect.top/uiScreenHeight),
					(rtIconRect.right/uiScreenWidth), (rtIconRect.bottom/uiScreenHeight));
				spIconTexture->UpdateBound();

				float fLeft = (float)(iconTexturePos.x+left) / (float)uiWidth;
				float fTop = (float)(iconTexturePos.y+top) / (float)uiHeight;
				float fRight = (float)(iconTexturePos.x+iIconSize-right) / (float)uiWidth;
				float fBottom = (float)(iconTexturePos.y+iIconSize-bottom) / (float)uiHeight;

				if(fDir != 0)
				{// 텍스쳐를 회전시켜서 가져온다.
					fLeft = (float)(iconTexturePos.x+left);
					fTop = (float)(iconTexturePos.y+top);
					fRight = (float)(iconTexturePos.x+iIconSize-right);
					fBottom = (float)(iconTexturePos.y+iIconSize-bottom);

					float fLeftOri = (float)(iconTexturePos.x);
					float fTopOri = (float)(iconTexturePos.y);
					float fRightOri = (float)(iconTexturePos.x+iIconSize);
					float fBottomOri = (float)(iconTexturePos.y+iIconSize);

					float fXLeft = fLeftOri;
					float fYTop = fTopOri;
					float fWidthHalf = (fRightOri - fLeftOri) / 2;
					float fHeightHalf = (fBottomOri - fTopOri) / 2;

					float fHol = fXLeft + fWidthHalf;
					float fVer = fYTop + fHeightHalf;

					NiPoint2 Pos[4];
					Pos[0].x = fLeft - fHol;
					Pos[0].y = fTop - fVer;
					Pos[1].x = fLeft - fHol;
					Pos[1].y = fBottom - fVer;
					Pos[2].x = fRight - fHol;
					Pos[2].y = fBottom - fVer;
					Pos[3].x = fRight - fHol;
					Pos[3].y = fTop - fVer;

					float const fOffset = -1.4f;
					float fCos = NiCos(fDir + fOffset);
					float fSin = NiSin(fDir + fOffset);

					for(int nn = 0; nn<4; ++nn)
					{	
						NiPoint2 kVec(-Pos[nn].x*fCos + Pos[nn].y*fSin + fHol, 
							Pos[nn].x*fSin + Pos[nn].y*fCos + fVer);
						kVec.x = kVec.x / (float)uiWidth;
						kVec.y = kVec.y / (float)uiHeight;
						spIconTexture->SetTexture(iPoly, nn, 0, kVec);
					}
				}
				else
				{
					spIconTexture->SetTextures(iPoly, 0, fLeft, fTop, fRight, fBottom);
				}
			}
		}
	}
	else
	{
		if (pTexture)
		{
			int iPoly = spIconTexture->Insert(4);
			spIconTexture->SetRectangle(iPoly, 
				(rtIconRect.left/uiScreenWidth), (rtIconRect.top/uiScreenHeight),
				(rtIconRect.right/uiScreenWidth), (rtIconRect.bottom/uiScreenHeight));
			spIconTexture->UpdateBound();

			float fLeft = (float)(iconTexturePos.x) / (float)uiWidth;
			float fTop = (float)(iconTexturePos.y) / (float)uiHeight;
			float fRight = (float)(iconTexturePos.x+iIconSize) / (float)uiWidth;
			float fBottom = (float)(iconTexturePos.y+iIconSize) / (float)uiHeight;

			if(fDir != 0)
			{// 텍스쳐를 회전시켜서 가져온다.
				fLeft = (float)(iconTexturePos.x);
				fTop = (float)(iconTexturePos.y);
				fRight = (float)(iconTexturePos.x+iIconSize);
				fBottom = (float)(iconTexturePos.y+iIconSize);

				float fXLeft = fLeft;
				float fYTop = fTop;
				float fWidthHalf = (fRight - fLeft) / 2;
				float fHeightHalf = (fBottom - fTop) / 2;

				float fHol = fXLeft + fWidthHalf;
				float fVer = fYTop + fHeightHalf;

				NiPoint2 Pos[4];
				Pos[0].x = fLeft - fHol;
				Pos[0].y = fTop - fVer;
				Pos[1].x = fLeft - fHol;
				Pos[1].y = fBottom - fVer;
				Pos[2].x = fRight - fHol;
				Pos[2].y = fBottom - fVer;
				Pos[3].x = fRight - fHol;
				Pos[3].y = fTop - fVer;

				float const fOffset = -1.4f;
				float fCos = NiCos(fDir + fOffset);
				float fSin = NiSin(fDir + fOffset);

				for(int nn = 0; nn<4; ++nn)
				{	
					NiPoint2 kVec(-Pos[nn].x*fCos + Pos[nn].y*fSin + fHol, 
						Pos[nn].x*fSin + Pos[nn].y*fCos + fVer);
					kVec.x = kVec.x / (float)uiWidth;
					kVec.y = kVec.y / (float)uiHeight;
					spIconTexture->SetTexture(iPoly, nn, 0, kVec);
				}
			}
			else
			{
				spIconTexture->SetTextures(iPoly, 0, fLeft, fTop, fRight, fBottom);
			}
		}
	}
}

IconType PgMiniMapUI::getIconType(PgActor* actor)
{
	if (actor == NULL)
		return ICONTYPE_NONE;

	if (actor->IsMyActor())
		return ICONTYPE_ME;

	//if (actor->IsMyPet())
	//	return ICONTYPE_MYPET;

	if (actor->GetObjectID() == PgIXmlObject::ID_NPC)
		return ICONTYPE_NPC;

	if (actor->GetObjectID() == PgIXmlObject::ID_MONSTER)
	{	
		PgPilot* pkPilot = actor->GetPilot();		
		if(pkPilot)
		{
			if(pkPilot->GetAbil(AT_HP) <= 0)
			{
				return ICONTYPE_NONE;
			}
			else
			{
				return ICONTYPE_MONSTER;
			}
		}
		
		return ICONTYPE_NONE;
	}

#ifndef EXTERNAL_RELEASE
	if ((m_iShowActor == 1 || m_iShowActor == 2) && actor->GetObjectID() == PgIXmlObject::ID_MONSTER)
	{
		if (actor->IsVisible())
		{
			if (actor->ContainsDirection(DIR_LEFT))
				return ICONTYPE_PARTY_LEFT_ARROW;
			else
				return ICONTYPE_PARTY_RIGHT_ARROW;
		}
		else
		{
			if (actor->ContainsDirection(DIR_LEFT))
				return ICONTYPE_GUILD_LEFT_ARROW;
			else
				return ICONTYPE_GUILD_RIGHT_ARROW;
		}
	}

	if ((m_iShowActor == 1 || m_iShowActor == 3) && actor->GetObjectID() == PgIXmlObject::ID_PC)
	{
		if (actor->IsVisible())
		{
			if (actor->ContainsDirection(DIR_LEFT))
				return ICONTYPE_PARTY_LEFT_ARROW;
			else
				return ICONTYPE_PARTY_RIGHT_ARROW;
		}
		else
		{
			if (actor->ContainsDirection(DIR_LEFT))
				return ICONTYPE_GUILD_LEFT_ARROW;
			else
				return ICONTYPE_GUILD_RIGHT_ARROW;
		}
	}
#endif

	return ICONTYPE_NONE;
}

IconType PgMiniMapUI::getIconType(PgTrigger* trigger)
{
	if (trigger == NULL || trigger->GetTriggerObject() == NULL)
	{
		return ICONTYPE_NONE;
	}
	switch(trigger->GetTriggerType())
	{
	case PgTrigger::TRIGGER_TYPE_PORTAL:
		return ICONTYPE_PORTAL;
	case PgTrigger::TRIGGER_TYPE_MISSION:
		return ICONTYPE_MISSION;
	case PgTrigger::TRIGGER_TYPE_JUMP:
		return ICONTYPE_JUMP;
	case PgTrigger::TRIGGER_TYPE_TELEJUMP:
		return ICONTYPE_TELEJUMP;
	case PgTrigger::TRIGGER_TYPE_LOCATION:
		return ICONTYPE_UNKNOWN_POINT;
	case PgTrigger::TRIGGER_TYPE_MISSION_EASY:
		return ICONTYPE_MISSION_EASY;
	}
	return ICONTYPE_NONE;
}


bool PgMiniMapUI::getIconTexturePosByType(IconType iconType, POINT2& rkOutPoint)
{
	//#define NULL_ICON 256
	if( iconType <= ICONTYPE_NONE )
	{
		return false;
	}

	kMapIconToIdxContainer::const_iterator c_iter = m_kMapIconTypeToIdxCont.find(SIconTypeToIndex(iconType));
	if( c_iter != m_kMapIconTypeToIdxCont.end() )
	{
		kMapIconTexContainer::const_iterator tex_iter = m_kMapIconTexCont.find(c_iter->TexType);
		if( tex_iter != m_kMapIconTexCont.end() )
		{
			rkOutPoint.x = ((c_iter->Index - 1) % tex_iter->second.U) * tex_iter->second.W;
			rkOutPoint.y = ((c_iter->Index - 1) / tex_iter->second.U) * tex_iter->second.H;
			return true;
		}
	}

	return false;
}

void PgMiniMapUI::arrangeScreenBoundary()
{
	float scaleFactor = m_fZoomFactor / 2;
	PG_ASSERT_LOG (scaleFactor >= 0.0f && scaleFactor <= 0.4f);
	float screenHalfWidth = (1.0f - scaleFactor * 2) / 2;
	float screenHalfHeight = (1.0f - scaleFactor * 2) / 2;

	//float screenLeft = m_kScreenCenter.x - screenHalfWidth;
	//float screenTop = m_kScreenCenter.y - screenHalfHeight;

	if (m_kScreenCenter.x - screenHalfWidth < 0.0f)
	{
		m_kScreenCenter.x = screenHalfWidth;
	}

	if (m_kScreenCenter.x + screenHalfWidth > 1.0f)
	{
		m_kScreenCenter.x = 1.0f - screenHalfWidth;
	}

	if (m_kScreenCenter.y - screenHalfHeight< 0.0f)
	{
		m_kScreenCenter.y = screenHalfHeight;
	}

	if (m_kScreenCenter.y + screenHalfHeight > 1.0f)
	{
		m_kScreenCenter.y = 1.0f - screenHalfHeight;
	}
}

void PgMiniMapUI::CheckAddIconToMyHome(PgPilot* pkPilot, POINT2 const& ptWndPos)
{
	if( !pkPilot )
	{
		return;
	}

	PgHouse* pkHouse = dynamic_cast<PgHouse*>(pkPilot->GetWorldObject());
	if( !pkHouse )
	{
		return;
	}

	PgMyHome* pkHome = dynamic_cast<PgMyHome*>(pkPilot->GetUnit());
	if( !pkHome )
	{
		return;
	}

	NiPoint3 worldPosition = pkHouse->GetTranslate();

	if( worldPosition == NiPoint3::ZERO )
		return;

	//아이콘 타입을 찾아
	IconType iconType = ICONTYPE_HOME_CLOSE;
	if( MAS_IS_BIDDING == pkHome->GetAbil(AT_MYHOME_STATE) )
	{
		iconType = ICONTYPE_HOME_AUCTION;
	}
	else
	{
		if( g_kPilotMan.IsMyPlayer(pkHome->OwnerGuid()) )
		{
			iconType = ICONTYPE_HOME_MYHOUSE;
		}
		else
		{
			int const kHomeVisitFlag = pkHome->GetAbil(AT_MYHOME_VISITFLAG);

			if( kHomeVisitFlag != MEV_ONLY_OWNER )
			{
				if( (kHomeVisitFlag & MEV_ALL) == MEV_ALL )
				{
					iconType = ICONTYPE_HOME_OPEN;
				}

				if( (kHomeVisitFlag & MEV_COUPLE) == MEV_COUPLE )
				{
					SCouple kMyCouple = g_kCoupleMgr.GetMyInfo();

					if( kMyCouple.CoupleGuid() != BM::GUID::NullData()
					&& kMyCouple.CoupleGuid() == pkHome->OwnerGuid() )
					{
						iconType = ICONTYPE_HOME_OPEN;
					}
				}

				if( (kHomeVisitFlag & MEV_GUILD) == MEV_GUILD )
				{
					SGuildMemberInfo kTemp;
					if( g_kGuildMgr.IamHaveGuild()
					&& g_kGuildMgr.GetMemberByGuid( pkHome->OwnerGuid(), kTemp ) )
					{
						iconType = ICONTYPE_HOME_OPEN;
					}
				}

				if( (kHomeVisitFlag & MEV_FRIEND) == MEV_FRIEND )
				{
					SFriendItem kTemp;
					if( g_kFriendMgr.Friend_Find_ByGuid( pkHome->OwnerGuid(), kTemp ) )
					{
						iconType = ICONTYPE_HOME_OPEN;
					}
				}
			}
		}
	}

	POINT2 iconTexturePos(0,0);
	bool const bGetIconRet = getIconTexturePosByType(iconType, iconTexturePos);
	if( !bGetIconRet )
	{
		return;
	}

	NiPoint3 viewPosition;
	if (m_spCamera->WorldPtToScreenPt3(worldPosition, viewPosition.x, viewPosition.y, viewPosition.z) == false)
		return;

	NiPoint3 screenPosition;
	if (viewPtToScreenPt(viewPosition, screenPosition))
	{
		MiniMapIconData iconData;
		iconData.screenPixel.x = (LONG)(ptWndPos.x + m_kWndSize.x * screenPosition.x);
		if( m_bAlwaysMinimap )	{ iconData.screenPixel.y = (LONG)(ptWndPos.y + m_kWndSize.y * screenPosition.y); }
		else					{ iconData.screenPixel.y = (LONG)(ptWndPos.y + (m_kWndSize.x * m_fScreenImageRatio * screenPosition.y) - m_pkDrawHeight->x + m_iMiniMapDrawGap); }
		iconData.viewPosition = viewPosition;

		float fAlpha = 1.0f;
		// TODO: 매 프레임 지우고 넣는게 아니라 좌표만 바꾸는 방식으로 바꾸자
		//m_kMapIcons.push_back(iconData); 
		addMiniMapIcon(ptWndPos, iconData.screenPixel, iconTexturePos, fAlpha, EMITT_LARGE);
	}
}

void PgMiniMapUI::copyTextureToBuffer(NiTexture* pTexture)
{
	NiDX9Renderer *pkRenderer = (NiDX9Renderer*)NiRenderer::GetRenderer();
	//NiDX9Renderer *pkRenderer2 = NiDX9Renderer::GetRenderer();

	if (pkRenderer == NULL || pTexture == NULL)
		return;

	NiDX9RenderedTextureData* pData = (NiDX9RenderedTextureData*)pTexture->GetRendererData();
	if (pData == NULL)
		return;

	LPDIRECT3DTEXTURE9 pD3DTexture = (LPDIRECT3DTEXTURE9)pData->GetD3DTexture();
	LPDIRECT3DSURFACE9 pD3DSurface = NULL;

	HRESULT hr;

	hr = pD3DTexture->GetSurfaceLevel(0, &pD3DSurface);
	if (FAILED(hr))
		return;

	LPDIRECT3DTEXTURE9 pTargetTexture, pTargetTexture2;
	LPDIRECT3DSURFACE9 pTargetSurface, pTargetSurface2;

	D3DSURFACE_DESC surfDesc;
	hr = pD3DSurface->GetDesc(&surfDesc);

	pD3DSurface->Release();
	
	hr = D3DXCreateTexture(pkRenderer->GetD3DDevice(), PRERENDER_MINIMAP_WIDTH, PRERENDER_MINIMAP_HEIGHT,
		1, 0, surfDesc.Format, D3DPOOL_DEFAULT, &pTargetTexture);

	// Retrieve the surface image of the texture.	
	hr = pTargetTexture->GetLevelDesc(0, &surfDesc);
	hr = pTargetTexture->GetSurfaceLevel(0, &pTargetSurface);

//	hr = D3DXLoadSurfaceFromSurface(pTargetSurface, 0, 0, pD3DSurface, 0, 0, D3DX_FILTER_NONE, 0);
//	hr = D3DXSaveTextureToFile(TEXT("file1.bmp"), D3DXIFF_BMP, pTargetTexture, NULL);
	pTargetSurface->Release();

	LPDIRECT3DSURFACE9 pD3DDepth;
	hr = pkRenderer->GetD3DDevice()->GetDepthStencilSurface(&pD3DDepth);
	hr = pD3DDepth->GetDesc(&surfDesc);

	hr = D3DXCreateTexture(pkRenderer->GetD3DDevice(), PRERENDER_MINIMAP_WIDTH, PRERENDER_MINIMAP_HEIGHT,
		1, 0, surfDesc.Format, D3DPOOL_DEFAULT, &pTargetTexture2);

	hr = pTargetTexture2->GetLevelDesc(0, &surfDesc);
	hr = pTargetTexture2->GetSurfaceLevel(0, &pTargetSurface2);

//	hr = D3DXLoadSurfaceFromSurface(pTargetSurface2, 0, 0, pD3DDepth, 0, 0, D3DX_FILTER_NONE, 0);
//	hr = D3DXSaveTextureToFile(TEXT("file2.bmp"), D3DXIFF_BMP, pTargetTexture2, NULL);
	pTargetSurface2->Release();
}

void PgMiniMapUI::AddMapIconTex(char const* pPath, SMapIconTexInfo& rkInfo)
{
	rkInfo.IconTexture = NiNew NiScreenElements(NiNew NiScreenElementsData(false, false, 1));
	NiTexturingPropertyPtr spTex = NiNew NiTexturingProperty; 
	PG_ASSERT_LOG(spTex);
	spTex->SetApplyMode(NiTexturingProperty::APPLY_MODULATE);
	spTex->SetBaseFilterMode(NiTexturingProperty::FILTER_NEAREST);
	spTex->SetBaseClampMode(NiTexturingProperty::CLAMP_S_CLAMP_T);
	NiSourceTexture* pTexture = g_kNifMan.GetTexture(pPath);
	spTex->SetBaseTexture(pTexture);
	rkInfo.IconTexture->AttachProperty(spTex);

	NiAlphaPropertyPtr spAlphaProp = NiNew NiAlphaProperty;
	spAlphaProp->SetAlphaBlending(true);
	rkInfo.IconTexture->AttachProperty(spAlphaProp);

	NiVertexColorProperty* pkVCProp = NiNew NiVertexColorProperty;
	pkVCProp->SetSourceMode(NiVertexColorProperty::SOURCE_IGNORE);
	pkVCProp->SetLightingMode(NiVertexColorProperty::LIGHTING_E);
	rkInfo.IconTexture->AttachProperty(pkVCProp);

	NiAlphaProperty* pkAlphaProp = NiNew NiAlphaProperty;
	pkAlphaProp->SetAlphaBlending(true);
	pkAlphaProp->SetAlphaTesting(false);
	pkAlphaProp->SetSrcBlendMode(NiAlphaProperty::ALPHA_SRCALPHA);
	pkAlphaProp->SetDestBlendMode(NiAlphaProperty::ALPHA_INVSRCALPHA);
	rkInfo.IconTexture->AttachProperty(pkAlphaProp);

	rkInfo.IconTexture->Update(0.0f);
	rkInfo.IconTexture->UpdateEffects();
	rkInfo.IconTexture->UpdateProperties();
}

bool PgMiniMapUI::ParseMiniMapXml()
{
	if( !m_kMapIconTexCont.empty() && !m_kMapIconTypeToIdxCont.empty() )
	{
		return true;
	}

	TiXmlDocument kXmlDoc("MiniMapIconInfo.xml");
	if( !PgXmlLoader::LoadFile(kXmlDoc, UNI("MiniMapIconInfo.xml")) )
	{
		return false;
	}

	TiXmlElement const* pkElement = kXmlDoc.FirstChildElement();
	char const* pcTagName = pkElement->Value();
	if( strcmp(pcTagName, "MAPICON_PATH") == 0 )
	{
		TiXmlElement const* pkSubElem = pkElement->FirstChildElement();
		
		while( pkSubElem )
		{
			char const* pcTagName = pkSubElem->Value();
			if( strcmp(pcTagName, "MAPICON") == 0 )
			{
				EMapIconTexType	TexType = EMITT_NONE;
				std::string kPath;
				SMapIconTexInfo	kInfo;

				TiXmlAttribute const* pkAttr = pkSubElem->FirstAttribute();
				while( pkAttr )
				{
					char const* pcAttrName = pkAttr->Name();
					char const* pcAttrValue = pkAttr->Value();

					if( strcmp(pcAttrName, "ID") == 0 )
					{
						TexType = (EMapIconTexType)(atoi(pcAttrValue));
					}
					else if( strcmp(pcAttrName, "PATH") == 0 )
					{
						kPath = pcAttrValue;
					}
					else if( strcmp(pcAttrName, "ICON_W") == 0 )
					{
						kInfo.W = atoi(pcAttrValue);
					}
					else if( strcmp(pcAttrName, "ICON_H") == 0 )
					{
						kInfo.H = atoi(pcAttrValue);
					}
					else if( strcmp(pcAttrName, "U") == 0 )
					{
						kInfo.U = atoi(pcAttrValue);
					}
					else if( strcmp(pcAttrName, "V") == 0 )
					{
						kInfo.V = atoi(pcAttrValue);
					}
					else
					{
						//??
					}
					pkAttr = pkAttr->Next();
				}

				if( EMITT_NONE != TexType )
				{
					kMapIconTexContainer::_Pairib result = m_kMapIconTexCont.insert(std::make_pair(TexType, kInfo));
					if( result.second )
					{
						AddMapIconTex(kPath.c_str(), result.first->second);
					}
				}
			}
			else if( strcmp(pcTagName, "TYPE_TO_INDEX") == 0 )
			{
				SIconTypeToIndex	Info;
				TiXmlAttribute const* pkAttr = pkSubElem->FirstAttribute();
				while( pkAttr )
				{
					char const* pcAttrName = pkAttr->Name();
					char const* pcAttrValue = pkAttr->Value();

					if( strcmp(pcAttrName, "TYPE") == 0 )
					{
						Info.Type = (IconType)(atoi(pcAttrValue));
					}
					else if( strcmp(pcAttrName, "INDEX") == 0 )
					{
						Info.Index = atoi(pcAttrValue);
					}
					else if( strcmp(pcAttrName, "ICON") == 0 )
					{
						Info.TexType = (EMapIconTexType)(atoi(pcAttrValue));
					}
					else
					{
					}
					pkAttr = pkAttr->Next();
				}

				kMapIconToIdxContainer::_Pairib result = m_kMapIconTypeToIdxCont.insert(Info);
				if( !result.second )
				{
					//??
				}
			}
			pkSubElem = pkSubElem->NextSiblingElement();
		}
	}
	return true;
}