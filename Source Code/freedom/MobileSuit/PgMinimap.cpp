#include "StdAfx.h"
#include "PgMinimap.h"
#include "PgMobileSuit.h"
#include "Variant/PgQuestInfo.h"
#include "Variant/PgBattleSquare.h"
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
#include "PgBattleSquare.h"
#include "PgOption.h"
#include "PgActorNpc.h"
#include "lwUIMission.h"
#include "PgNetwork.h"
#include "PgClientParty.h"
#include "PgUISound.h"
#include "PgContentsBase.h"
#include "PgPvPGame.h"
#include "PgDropBox.h"

bool g_bAbleSetTeleMove = false;
bool g_bDrawMinimapLink = false;

float g_MinimapAlramTime = 0.0f;

extern SGroundKey g_kNowGroundKey;

namespace PgMiniMapUtil
{
	void ProcessMsg(unsigned short const usType, BM::Stream& rkPacket)
	{
		switch( usType )
		{
		case PT_M_C_NFY_ALARM_MINIMAP:			{ Recv_PT_M_C_NFY_ALARM_MINIMAP(rkPacket); }break;
		case PT_M_C_NFY_TRIGGER_ONENTER:		{ Recv_PT_M_C_NFY_TRIGGER_ONENTER(rkPacket); }break;
		case PT_M_C_NFY_TRIGGER_ONLEAVE:		{ Recv_PT_M_C_NFY_TRIGGER_ONLEAVE(rkPacket); }break;
		default:
			{
			}break;
		}
	}

	void CallMiniMapIconInfoUI()
	{
		lua_tinker::call<void, lwUIWnd>("CallMinimapObjectList", lwUIWnd(XUIMgr.Get(L"SFRM_BIG_MAP")));
	}

	void CloseMiniMapIconInfoUI()
	{
		lua_tinker::call<void>("CloseMinimapObjectList");
	}

	void Send_PT_C_M_REQ_ALARM_MINIMAP(POINT2 const& WndPos, POINT2 const& MousePos)
	{
		float const fAccumTime = g_pkApp->GetAccumTime();
		if( fAccumTime - g_MinimapAlramTime < 0.2f )
		{
			return;
		}
		g_MinimapAlramTime = fAccumTime;

		XUI::CXUI_Wnd * MinimapWnd = XUIMgr.Get(L"SFRM_BIG_MAP");
		if( NULL == MinimapWnd )
		{
			return;
		}

		PgPlayer * pPlayer = g_kPilotMan.GetPlayerUnit();
		if( NULL == pPlayer )
		{
			return;
		}

		if( 0 == pPlayer->GetAbil(AT_TEAM)
			&& false == PgClientPartyUtil::IsInParty() )
		{
			return;
		}

		XUI::CXUI_Builder * AlramBuild =  dynamic_cast<XUI::CXUI_Builder*>(MinimapWnd->GetControl(L"BLD_MINIMAP_ALARM"));
		if( NULL == AlramBuild )
		{
			return;
		}

		int const BuildCount = AlramBuild->CountX() * AlramBuild->CountY();
		for( int WndCount = 0; WndCount < BuildCount; ++WndCount )
		{
			BM::vstring FormName("FRM_ALARM_");
			FormName += WndCount;
			XUI::CXUI_Wnd * AlarmWnd = MinimapWnd->GetControl((static_cast<std::wstring>(FormName).c_str()));
			if( NULL == AlarmWnd )
			{
				continue;
			}
			if( AlarmWnd->Visible() )
			{
				continue;
			}
			POINT2 MousePosInWnd = MousePos - WndPos;
			BM::Stream Packet(PT_C_M_REQ_ALARM_MINIMAP);
			Packet.Push(MousePosInWnd);
			NETWORK_SEND(Packet);
			break;
		}
	}

	void Recv_PT_M_C_NFY_ALARM_MINIMAP(BM::Stream & Packet)
	{
		POINT2 MousePosInWnd;
		Packet.Pop(MousePosInWnd);

		XUI::CXUI_Wnd * MinimapWnd = XUIMgr.Activate(L"SFRM_BIG_MAP");
		if( NULL == MinimapWnd )
		{
			MinimapWnd = XUIMgr.Call(L"SFRM_BIG_MAP");
		}

		PgPlayer * pPlayer = g_kPilotMan.GetPlayerUnit();
		if( NULL == pPlayer )
		{
			return;
		}

		if( 0 == pPlayer->GetAbil(AT_TEAM)
			&& false == PgClientPartyUtil::IsInParty() )
		{
			return;
		}

		XUI::CXUI_Builder * AlramBuild =  dynamic_cast<XUI::CXUI_Builder*>(MinimapWnd->GetControl(L"BLD_MINIMAP_ALARM"));
		if( NULL == AlramBuild )
		{
			return;
		}

		int const BuildCount = AlramBuild->CountX() * AlramBuild->CountY();
		for( int WndCount = 0; WndCount < BuildCount; ++WndCount )
		{
			BM::vstring FormName("FRM_ALARM_");
			FormName += WndCount;
			XUI::CXUI_Wnd * AlarmWnd = MinimapWnd->GetControl((static_cast<std::wstring>(FormName).c_str()));
			if( NULL == AlarmWnd )
			{
				continue;
			}
			if( AlarmWnd->Visible() )
			{
				continue;
			}
			AlarmWnd->Visible(true);
			POINT2 DrawPos;
			DrawPos.x = MousePosInWnd.x - (AlarmWnd->Size().x / 2) + 6;
			DrawPos.y = MousePosInWnd.y - (AlarmWnd->Size().y / 2) + 4;
			AlarmWnd->Location( DrawPos );

			SUVInfo uvInfo = AlarmWnd->UVInfo();
			uvInfo.Index = 1;
			AlarmWnd->UVInfo(uvInfo);

			lwPlaySoundByID("click_minimap");
			break;
		}
	}

	void Recv_PT_M_C_NFY_TRIGGER_ONENTER(BM::Stream & Packet)
	{
		std::string TriggerName;
		BM::GUID PlayerGuid;
		Packet.Pop(TriggerName);
		Packet.Pop(PlayerGuid);
		
		PgAlwaysMiniMap* pMinimap = dynamic_cast<PgAlwaysMiniMap*>(g_kUIScene.GetMiniMapUI("MiniMap_Whole"));
		if( !pMinimap )
		{
			return;
		}

		PgIWorldObject * pObject = pMinimap->GetObjectContainerByGUID(PlayerGuid);
		if( NULL == pObject )
		{
			return;
		}
		
		PgPilot * pPilot = pObject->GetPilot();
		if( NULL == pPilot )
		{
			return;
		}
		
		if( pPilot->GetWorldObject() )
		{
			switch( pObject->GetObjectID() )
			{
			case PgIXmlObject::ID_PC:
				{
					PgActor * pActor = dynamic_cast<PgActor*>(pPilot->GetWorldObject());
					if( pActor && g_pkWorld )
					{
						PgTrigger * pTrigger = g_pkWorld->GetTriggerByID(TriggerName);
						if( pTrigger )
						{
							pActor->SetCurrentTrigger( pTrigger );
						}
					}
				}break;
			default:
				{
				}break;
			}
		}	
	}

	void Recv_PT_M_C_NFY_TRIGGER_ONLEAVE(BM::Stream & Packet)
	{
		BM::GUID PlayerGuid;
		Packet.Pop(PlayerGuid);

		PgAlwaysMiniMap* pMinimap = dynamic_cast<PgAlwaysMiniMap*>(g_kUIScene.GetMiniMapUI("MiniMap_Whole"));
		if( !pMinimap )
		{
			return;
		}

		PgIWorldObject * pObject = pMinimap->GetObjectContainerByGUID(PlayerGuid);
		if( NULL == pObject )
		{
			return;
		}
		
		PgPilot * pPilot = pObject->GetPilot();
		if( NULL == pPilot )
		{
			return;
		}
		
		if( pPilot->GetWorldObject() )
		{
			switch( pObject->GetObjectID() )
			{
			case PgIXmlObject::ID_PC:
				{
					PgActor * pActor = dynamic_cast<PgActor*>(pPilot->GetWorldObject());
					if( pActor )
					{
						pActor->SetCurrentTrigger( NULL );
					}
				}break;
			default:
				{
				}break;
			}
		}	
	}
};

float const DEFAULT_LIMIT_ZOOM_FACTOR = 0.8f;
float const DEFAULT_HALF_ZOOM_FACTOR = DEFAULT_LIMIT_ZOOM_FACTOR * 0.5f;
float const MINIMAP_SCREEND_IMAGE_RATIO	= 0.75f;
int const PAIR_TELEPORT_ZONE_ADDPARAM = 0xFF00;

kMapIconTexContainer	PgMiniMapBase::ms_kMapIconTexCont;
kMapIconToIdxContainer	PgMiniMapBase::ms_kMapIconTypeToIdxCont;


PgMiniMapBase::PgMiniMapBase(void)
	: m_bInitialized(false)
	, m_spCamera(NULL)
	, m_fZoomFactor(0.5f)
	, m_fScreenImageRatio(MINIMAP_SCREEND_IMAGE_RATIO)
	, m_kWndSize()
	, m_kMiniMapSize()
	, m_kScreenCenter(0.5f, 0.5f)
	, m_iDrawGap(0)
	, m_spMiniMapScreenTexture(NULL)
	, m_spMiniMapImage(NULL)
	, m_bRefreshAllMiniMapIconInfo(true)
	, m_kShowMiniMap(true)
{
	//m_kMapIconTexCont.clear();
	m_kAniIconCont.clear();
}

PgMiniMapBase::~PgMiniMapBase(void)
{
}

void PgMiniMapBase::ReleaseAll()
{
	ms_kMapIconTexCont.clear();
	ms_kMapIconTypeToIdxCont.clear();
}

void PgMiniMapBase::Close()
{
}

bool PgMiniMapBase::BaseInitialize(S_INIT_INFO_BASE& kInitInfo)
{
	MiniMapIconCont::iterator iter = m_kMiniMapIconCont.begin();
	while (iter != m_kMiniMapIconCont.end())
	{
		if (iter->second.iIconPolygonIndex >= 0)
		{
			iter->second.pIconTexture->Remove(iter->second.iIconPolygonIndex);
		}
		++iter;
	}
	
	m_kMiniMapIconCont.clear();
	ClearRemoveMiniMapIcon();

	if( CreateScreenTexture(kInitInfo.kImgPath) )
	{
		if( ParseMiniMapIconXml() )
		{
			m_kWndSize = kInitInfo.kUISize;
			m_spCamera = kInitInfo.pkCamera;
			return true;
		}
	}
	return false;
}

void PgMiniMapBase::ShowMiniMap(bool bShow)
{
	if (m_kShowMiniMap == bShow)
		return;

	m_kShowMiniMap = bShow;
	if (bShow)
	{
		m_bRefreshAllMiniMapIconInfo = true;
	}
	else
	{
		MiniMapIconCont::iterator iter = m_kMiniMapIconCont.begin();

		while (iter != m_kMiniMapIconCont.end())
		{
			iter->second.pIconTexture->SetRectangle(iter->second.iIconPolygonIndex, 0, 0, 0, 0);
			++iter;
		}
	}
}

void PgMiniMapBase::RenderFrame(NiRenderer* pkRenderer, POINT2 const& ptWndSize)
{
	PROFILE_FUNC();
	if(!pkRenderer || !Initialized() )
	{
		return;	
	}

	if(NULL == m_spMiniMapScreenTexture)
	{
		return;
	}

	if( ms_kMapIconTexCont.empty() )
	{
		return;
	}

	if (m_bRefreshAllMiniMapIconInfo)
	{
		MiniMapIconCont::iterator iter = m_kMiniMapIconCont.begin();

		while (iter != m_kMiniMapIconCont.end())
		{
			SMiniMapIconInfo &kIconInfo = iter->second;
			kIconInfo.bDirty = true;
			if(kIconInfo.iIconPolygonIndex>=0)
			{
				kIconInfo.pIconTexture->SetRectangle(kIconInfo.iIconPolygonIndex, 0, 0, 0, 0);
			}
			++iter;
		}
	}

	NiPoint2 kScreenSize;
	kScreenSize.x = (float)pkRenderer->GetDefaultRenderTargetGroup()->GetWidth(0);
	kScreenSize.y = (float)pkRenderer->GetDefaultRenderTargetGroup()->GetHeight(0);

	DoRemoveMiniMapIcon();
	CustomizeRenderFrame(pkRenderer, kScreenSize, ptWndSize);
	m_bRefreshAllMiniMapIconInfo = false;
}

void PgMiniMapBase::RenderFrameUI(XUI::CXUI_Wnd * pkWnd)
{
}

void PgMiniMapBase::SetTriggerIconUI(XUI::CXUI_Wnd * pkWnd, const POINT2 &ptWndPos)
{
}

void PgMiniMapBase::Draw(PgRenderer* pkRenderer)
{
	PROFILE_FUNC();
	if(pkRenderer && Initialized())
	{
		if (m_spMiniMapScreenTexture)
		{
			PgUIScene::Render_UIObject(pkRenderer,m_spMiniMapScreenTexture);
		}

		kMapIconTexContainer::iterator icon_iter = ms_kMapIconTexCont.begin();
		while( icon_iter != ms_kMapIconTexCont.end() )
		{
			kMapIconTexContainer::mapped_type& kIconInfo = icon_iter->second;
			if( kIconInfo.IconTexture )
			{
				if (kIconInfo.IconTexture->BoundNeedsUpdate())
					kIconInfo.IconTexture->UpdateBound();

				PgUIScene::Render_UIObject(pkRenderer, kIconInfo.IconTexture);
			}
			++icon_iter;
		}
	}
}

void PgMiniMapBase::Zoom(float const fZoom)
{
	PROFILE_FUNC();
	if( !m_spCamera || !Initialized() )
	{
		return;
	}

	m_fZoomFactor += fZoom;
	if (m_fZoomFactor < 0.0f)
	{
		m_fZoomFactor = 0.0f;
	}
	if (m_fZoomFactor > DEFAULT_LIMIT_ZOOM_FACTOR)
	{
		m_fZoomFactor = DEFAULT_LIMIT_ZOOM_FACTOR;
	}

	m_bRefreshAllMiniMapIconInfo = true;
	RefreshZoomMiniMap();
}

PgUITexture* PgMiniMapBase::GetTex()
{
	return NULL;
}

void PgMiniMapBase::RefreshZoomMiniMap()
{
	PROFILE_FUNC();
	ArrangeScreenBoundary();
}

void PgMiniMapBase::Update(float const fAccumTime, float const fFrameTime)
{
	PROFILE_FUNC();
	RefreshZoomMiniMap();
}

void PgMiniMapBase::Terminate()
{
	PROFILE_FUNC();
	m_bInitialized = false;
	m_spMiniMapScreenTexture = NULL;
	m_spMiniMapImage = NULL;
	m_spCamera = NULL;

	MiniMapIconCont::iterator iter = m_kMiniMapIconCont.begin();
	while (iter != m_kMiniMapIconCont.end())
	{
		if (iter->second.iIconPolygonIndex >= 0)
		{
			iter->second.pIconTexture->Remove(iter->second.iIconPolygonIndex);
		}
		++iter;
	}
	
	m_kMiniMapIconCont.clear();

	//ms_kMapIconTexCont.clear();
	m_kAniIconCont.clear();
}

bool PgMiniMapBase::ParseMiniMapIconXml()
{
	if(!ms_kMapIconTypeToIdxCont.empty() || !ms_kMapIconTexCont.empty())
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
					std::pair<kMapIconTexContainer::iterator, bool> result = ms_kMapIconTexCont.insert(std::make_pair(TexType, kInfo));
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

				kMapIconTexContainer::iterator iter = ms_kMapIconTexCont.find(Info.TexType);
				if (iter != ms_kMapIconTexCont.end())
				{
					Info.TexInfo = iter->second;
					Info.IconPos.x = ((Info.Index - 1) % Info.TexInfo.U) * Info.TexInfo.W;
					Info.IconPos.y = ((Info.Index - 1) / Info.TexInfo.U) * Info.TexInfo.H;
				}
				else
				{
					PgXmlError1(pkElement, "XmlParse: Can't find TextType %d", Info.TexType);
				}

				if (Info.Type > ICONTYPE_NONE )
				{
					ms_kMapIconTypeToIdxCont.insert(std::make_pair(Info.Type, Info));
				}
			}
			pkSubElem = pkSubElem->NextSiblingElement();
		}
	}
	return true;
}

POINT2 const PgMiniMapBase::IconDrawPosition(POINT2 const& ptWndPos, NiPoint3 const& ScreenPos) const
{
	PROFILE_FUNC();
	return POINT2((LONG)(ptWndPos.x + m_kWndSize.x * ScreenPos.x), (LONG)(ptWndPos.y + m_kWndSize.y * ScreenPos.y));
}


PgMiniMapBase::E_RECT_CHECK_RESULT const PgMiniMapBase::GetIconRect(RECT& rtIconRect, const POINT2& ptWndPos, const POINT2& ScreenPixel, int const iIconSize) const
{
	PROFILE_FUNC();
	return GetIconRect(rtIconRect, ptWndPos, ScreenPixel, POINT2(iIconSize,iIconSize));
}

PgMiniMapBase::E_RECT_CHECK_RESULT const PgMiniMapBase::GetIconRect(RECT& rtIconRect, POINT2 const& ptWndPos, POINT2 const& ScreenPixel, POINT2 const iIconSize) const
{
	PROFILE_FUNC();
	int const iIconHalfSizeX = static_cast<int>(iIconSize.x * 0.5f);
	int const iIconHalfSizeY = static_cast<int>(iIconSize.y * 0.5f);

	//! 아이콘이 완전히 안보이는 경우
	if(ScreenPixel.x + iIconHalfSizeX < ptWndPos.x || ScreenPixel.x - iIconHalfSizeX > ptWndPos.x + m_kWndSize.x
	|| ScreenPixel.y + iIconHalfSizeY < ptWndPos.y || ScreenPixel.y - iIconHalfSizeY > ptWndPos.y + m_kWndSize.y )
	{
		return ERCR_OUT;
	}

	//! 현재 보이는 화면 안에 있다.				
	if (ScreenPixel.x - iIconHalfSizeX < ptWndPos.x || ScreenPixel.x + iIconHalfSizeX > ptWndPos.x + m_kWndSize.x ||
		ScreenPixel.y - iIconHalfSizeY < ptWndPos.y || ScreenPixel.y + iIconHalfSizeY > ptWndPos.y + m_kWndSize.y)
	{//일부만 보이는 경우
		int left = 0;
		int right = 0;
		int top = 0;
		int bottom = 0;

		if (ScreenPixel.x - iIconHalfSizeX < ptWndPos.x)
			left = ptWndPos.x - (ScreenPixel.x - iIconHalfSizeX);
		if (ScreenPixel.x + iIconHalfSizeX > ptWndPos.x + m_kWndSize.x)
			right = ScreenPixel.x + iIconHalfSizeX - (ptWndPos.x + m_kWndSize.x);
		if (ScreenPixel.y - iIconHalfSizeY < ptWndPos.y)
			top = ptWndPos.y - (ScreenPixel.y - iIconHalfSizeY);
		if (ScreenPixel.y + iIconHalfSizeY > ptWndPos.y + m_kWndSize.y)
			bottom = ScreenPixel.y + iIconHalfSizeY - (ptWndPos.y + m_kWndSize.y);

		PG_ASSERT_LOG (!(left && right));
		PG_ASSERT_LOG (!(top && bottom));

		rtIconRect.top = (short)(ScreenPixel.y - iIconHalfSizeY + top);
		rtIconRect.left = (short)(ScreenPixel.x - iIconHalfSizeX + left);
		rtIconRect.right = (unsigned short)(iIconSize.x - left - right);
		rtIconRect.bottom = (unsigned short)(iIconSize.y - top - bottom);
		return ERCR_PART;
	}
	else
	{//다보이는 경우
		rtIconRect.top = (short)(ScreenPixel.y - iIconHalfSizeY);
		rtIconRect.left = (short)(ScreenPixel.x - iIconHalfSizeX);
		rtIconRect.right = (unsigned short)(iIconSize.x);
		rtIconRect.bottom = (unsigned short)iIconSize.y;
	}
	return ERCR_ALL;
}


POINT2 const PgMiniMapBase::GetIconSize(EMapIconTexType const Type) const
{
	PROFILE_FUNC();
	kMapIconTexContainer::const_iterator icon_iter = ms_kMapIconTexCont.find(Type);
	if( icon_iter != ms_kMapIconTexCont.end() )
	{
		kMapIconTexContainer::mapped_type const& kIconInfo = icon_iter->second;
		NiScreenElementsPtr spIconTexture = kIconInfo.IconTexture;
		return POINT2(kIconInfo.W, kIconInfo.H);
	}
	return POINT2::NullData();
}

bool PgMiniMapBase::AddMiniMapEffectIcon(PgActor* pkActor, POINT2 const& ptWndPos)
{
	IconType eIconType = ICONTYPE_NONE;

	if( pkActor && false==pkActor->IsMyActor() )
	{
		switch( pkActor->GetObjectID() )
		{
		case PgIXmlObject::ID_NPC:
		case PgIXmlObject::ID_MONSTER:
		case PgIXmlObject::ID_PC:
			{
				PgPilot * pPilot = pkActor->GetPilot();
				if( pPilot )
				{
					IconType const effectIcon = static_cast<IconType>(pPilot->GetAbil(AT_DISPLAY_MINIMAP_EFFECT));
					if( effectIcon )
					{//엑터가 미니맵에 표시되는 이펙트에 걸려있는지 검사한다.
						switch( effectIcon )
						{
						case ICONTYPE_BOMBMAN_RED:
						case ICONTYPE_BOMBMAN_BLUE:
							{
								ETeam const OtherTeam = static_cast<ETeam>(pPilot->GetAbil(AT_TEAM));
								if( TEAM_RED == OtherTeam )
								{
									eIconType = ICONTYPE_BOMBMAN_RED;
								}
								else
								{
									eIconType = ICONTYPE_BOMBMAN_BLUE;
								}
							}break;
						default:
							{
								eIconType = effectIcon;
							}break;
						}
					}
				}
			}break;
		}
	}

	if (eIconType == ICONTYPE_NONE)
	{
		return AddMiniMapActorIcon(pkActor, ptWndPos);
	}

	return AddMiniMapIcon(pkActor->GetGuid(), pkActor->GetPos(), eIconType, ptWndPos);
}

bool PgMiniMapBase::AddMiniMapActorIcon(PgActor* pkActor, POINT2 const& ptWndPos)
{
	PROFILE_FUNC();
	if( !pkActor ){ return false; }

	IconType eIconType = ICONTYPE_NONE;

	if( pkActor->IsMyActor() )
	{
		return false;
	}
	else
	{
		switch( pkActor->GetObjectID() )
		{
		case PgIXmlObject::ID_NPC:		{ eIconType = ICONTYPE_NPC;	}break;
		case PgIXmlObject::ID_MONSTER:
			{

				PgPilot * pkPilot = pkActor->GetPilot();
				bool const bIsDependMonster = g_kQuestMan.IsDependIngQuestMonster(pkPilot->GetAbil(AT_CLASS), g_kNowGroundKey.GroundNo());
				if( pkPilot && pkPilot->GetAbil(AT_HP) > 0 )
				{
					eIconType = ICONTYPE_MONSTER;

					if (bIsDependMonster)
					{
						eIconType = ICONTYPE_QUEST_MON;
					}

				}
			}break;
		case PgIXmlObject::ID_PC:
			{
				PgPlayer * pPlayer = g_kPilotMan.GetPlayerUnit();
				PgPlayer * pOtherPlayer = dynamic_cast<PgPlayer*>(pkActor->GetUnit());
				if( pPlayer && pOtherPlayer )
				{
					if( 0 == pOtherPlayer->GetAbil(AT_TEAM) )
					{// 팀이 없으면
						PgTrigger * pTrigger = pkActor->GetCurrentTrigger();
						if( pTrigger
							&& PgTrigger::ADD_RADAR == pTrigger->EAddedType() )
						{// 트리거 안이라면 그 트리거의 추가 속성이 레이더 타입인지 확인한다.
							eIconType = ICONTYPE_TEAM_RED;
							pOtherPlayer->VisibleMyMinimapIcon(true);
						}
						else
						{// 트리거 밖으로 벗어나면 미니맵 아이콘을 지워준다.
							RemoveMiniMapIcon( pkActor->GetGuid() );
							pOtherPlayer->VisibleMyMinimapIcon(false);
						}
						RefreshMiniMapIcon(pkActor->GetGuid());
					}
					else
					{
						int const MyTeam = pPlayer->GetAbil(AT_TEAM);
						if( MyTeam )
						{//레드, 블루 팀 아이콘을 출력한다.
							int const OtherTeam = pOtherPlayer->GetAbil(AT_TEAM);
							if( MyTeam == OtherTeam )
							{
								if( TEAM_RED == OtherTeam )
								{
									eIconType = ICONTYPE_TEAM_RED;
								}
								else
								{
									eIconType = ICONTYPE_TEAM_BLUE;
								}
								pOtherPlayer->VisibleMyMinimapIcon(true);
							}
							else
							{//같은 팀이 아닐 경우는 진지 주변에 있는지 체크한다.
								PgTrigger * pTrigger = pkActor->GetCurrentTrigger();
								if( pTrigger
									&& PgTrigger::ADD_RADAR == pTrigger->EAddedType() )
								{//트리거 안이라면 그 트리거의 추가 속성이 레이더 타입인지 확인한다.
									if( TEAM_RED == OtherTeam )
									{
										eIconType = ICONTYPE_TEAM_RED;
									}
									else
									{
										eIconType = ICONTYPE_TEAM_BLUE;
									}
									pOtherPlayer->VisibleMyMinimapIcon(true);
								}
								else
								{//트리거 밖으로 벗어나면 미니맵 아이콘을 지워준다.
									RemoveMiniMapIcon( pkActor->GetGuid() );
									pOtherPlayer->VisibleMyMinimapIcon(false);
								}
							}
						}
					}
				}
			}break;
		default:
			return false;
		}
	}

	if (eIconType == ICONTYPE_NONE)
	{
		return false;
	}
	
	return AddMiniMapIcon(pkActor->GetGuid(), pkActor->GetPos(), eIconType, ptWndPos);
}

bool PgMiniMapBase::AddMiniMapTriggerIcon(PgTrigger* pkTrigger, POINT2 const& ptWndPos)
{
	PROFILE_FUNC();
	if( !pkTrigger ){ return false; }

	NiAVObject* pkTriggerObject = pkTrigger->GetTriggerObject();
	if( !pkTriggerObject ){ return false; }

	IconType eIconType = GetTriggerType(pkTrigger);

	if( eIconType == ICONTYPE_NONE )
	{
		return false;
	}

	float fRadian = 0.0f;
	if( ICONTYPE_TELEJUMP == eIconType && g_pkWorld && m_spCamera )
	{
		NiPoint3 ViewPosition;
		NiPoint3 ScreenPosition;

		if( m_spCamera->WorldPtToScreenPt3(pkTriggerObject->GetWorldTranslate(), ViewPosition.x, ViewPosition.y, ViewPosition.z) == false )
		{
			return false;
		}

		if( ViewPtToScreenPt(ViewPosition, ScreenPosition) )
		{
			double dbRadian = pkTrigger->GetRotation();
			if( dbRadian == -9999 )
			{
				lwWorld kWorld = lwWorld(g_pkWorld);
				lwPoint3 kPoint3 = kWorld.GetObjectPosByName(MB(pkTrigger->ParamString()));
				NiPoint3 worldPos = kPoint3();
				// 두 점의 각도를 구하자.
				NiPoint3 viewPos = NiPoint3::ZERO;
				m_spCamera->WorldPtToScreenPt3(worldPos, viewPos.x, viewPos.y, viewPos.z);
				NiPoint2 screenPix;
				screenPix.x = viewPos.x - ViewPosition.x;
				screenPix.y = viewPos.y - ViewPosition.y;
				screenPix.Unitize();
				dbRadian = atan2( (double)screenPix.y, (double)screenPix.x );

				pkTrigger->SetRotation(dbRadian);
			}
			fRadian = static_cast<float>(dbRadian);
		}
	}

	return AddMiniMapIcon(pkTrigger->GetGuid(), pkTriggerObject->GetWorldTranslate(), eIconType, ptWndPos, fRadian);
}

bool PgMiniMapBase::AddMiniMapBrokenObjectIcon(PgPilot* pkPilot, POINT2 const& ptWndPos)
{
	PROFILE_FUNC();
	if( !pkPilot ){ return false; }

	IconType eIconType = ICONTYPE_NONE;

	EUnitGrade Grade = static_cast<EUnitGrade>(pkPilot->GetAbil(AT_GRADE));
	switch( Grade )
	{
	case EOGRADE_SUBCORE:
		{
			int const Team = pkPilot->GetAbil(AT_TEAM);
			switch( Team )
			{
			case TEAM_ATTACKER: { eIconType = ICONTYPE_EMPORIA_SUB_RED;		} break;
			case TEAM_DEFENCER: { eIconType = ICONTYPE_EMPORIA_SUB_BLUE;	} break;
			}
		}break;
	case EOGRADE_MAINCORE:
		{
			int const Team = pkPilot->GetAbil(AT_TEAM);
			switch( Team )
			{
			case TEAM_ATTACKER: { eIconType = ICONTYPE_EMPORIA_MAIN_RED;	} break;
			case TEAM_DEFENCER: { eIconType = ICONTYPE_EMPORIA_MAIN_BLUE;	} break;
			}
		}break;
	default:
		return false;
	}

	PgActor* pkActor = dynamic_cast<PgActor*>(pkPilot->GetWorldObject());
	if( pkActor )
	{
		return AddMiniMapIcon(pkActor->GetGuid(), pkActor->GetPos(), eIconType, ptWndPos);
	}	
	return false;
}

bool PgMiniMapBase::AddMiniMapMyHomeIcon(PgHouse* pkHouse, PgMyHome* pkHome, POINT2 const& ptWndPos)
{
	PROFILE_FUNC();
	if( !pkHouse || !pkHome )
	{
		return false;
	}

	IconType eIconType = ICONTYPE_HOME_CLOSE;

	//아이콘 타입을 찾아
	if( MAS_IS_BIDDING == pkHome->GetAbil(AT_MYHOME_STATE) )
	{
		eIconType = ICONTYPE_HOME_AUCTION;
	}
	else
	{
		if( g_kPilotMan.IsMyPlayer(pkHome->OwnerGuid()) )
		{
			eIconType = ICONTYPE_HOME_MYHOUSE;
		}
		else
		{
			int const kHomeVisitFlag = pkHome->GetAbil(AT_MYHOME_VISITFLAG);

			if( kHomeVisitFlag != MEV_ONLY_OWNER )
			{
				if( (kHomeVisitFlag & MEV_ALL) == MEV_ALL )
				{
					eIconType = ICONTYPE_HOME_OPEN;
				}

				if( (kHomeVisitFlag & MEV_COUPLE) == MEV_COUPLE )
				{
					SCouple kMyCouple = g_kCoupleMgr.GetMyInfo();

					if( kMyCouple.CoupleGuid() != BM::GUID::NullData()
					&& kMyCouple.CoupleGuid() == pkHome->OwnerGuid() )
					{
						eIconType = ICONTYPE_HOME_OPEN;
					}
				}

				if( (kHomeVisitFlag & MEV_GUILD) == MEV_GUILD )
				{
					SGuildMemberInfo kTemp;
					if( g_kGuildMgr.IamHaveGuild()
					&& g_kGuildMgr.GetMemberByGuid( pkHome->OwnerGuid(), kTemp ) )
					{
						eIconType = ICONTYPE_HOME_OPEN;
					}
				}

				if( (kHomeVisitFlag & MEV_FRIEND) == MEV_FRIEND )
				{
					SFriendItem kTemp;
					if( g_kFriendMgr.Friend_Find_ByGuid( pkHome->OwnerGuid(), kTemp ) )
					{
						eIconType = ICONTYPE_HOME_OPEN;
					}
				}
			}
		}
	}

	return AddMiniMapIcon(pkHouse->GetGuid(), pkHouse->GetTranslate(), eIconType, ptWndPos);
}

bool PgMiniMapBase::AddMiniMapDropBoxItemIcon(PgDropBox* pDropBox, POINT2 const& ptWndPos)
{
	PROFILE_FUNC();
	if( !pDropBox )
	{
		return false;
	}
	
	IconType eIconType = ICONTYPE_NONE;

	PgBase_Item * pItem = pDropBox->GetItem(0);
	if( !pItem )
	{
		return false;
	}
	
	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const *pDef = kItemDefMgr.GetDef(pItem->ItemNo());
	if( !pDef )
	{
		return false;
	}

	ETeam const Type = static_cast<ETeam>(pDef->GetAbil(AT_DISPLAY_MINIMAP_INSITEM));
	switch( Type )
	{
	case ICONTYPE_BEAR_RED:
		{
			eIconType = ICONTYPE_BEAR_RED;
		}break;
	case ICONTYPE_BEAR_BLUE:
		{
			eIconType = ICONTYPE_BEAR_BLUE;
		}break;
	default:
		{
			return false;
		}
	}
	
	if( ICONTYPE_NONE == eIconType )
	{
		return false;
	}

	return AddMiniMapIcon(pDropBox->GetGuid(), pDropBox->GetTranslate(), eIconType, ptWndPos);
}

bool PgMiniMapBase::AddMiniMapQuestIcon(BM::GUID const& kGuid, NiPoint3 const& WorldPt, EQuestState const eState, POINT2 const& ptWndPos)
{
	PROFILE_FUNC();
	IconType eIconType = ICONTYPE_NONE;
	
	switch( eState )
	{
	case QS_Begin_Story:	{ eIconType = ICONTYPE_QUEST_BEGIN_STORY;	}break;
	case QS_End_Story:		{ eIconType = ICONTYPE_QUEST_END_STORY;		}break;
	case QS_Begin:			{ eIconType = ICONTYPE_QUEST_BEGIN;			}break;
	case QS_Ing:			{ eIconType = ICONTYPE_QUEST_ING;			}break;
	case QS_Begin_NYet:		{ eIconType = ICONTYPE_QUEST_NOTYET;		}break;
	case QS_End:			{ eIconType = ICONTYPE_QUEST_END;			}break;
	case QS_Begin_Loop:		{ eIconType = ICONTYPE_QUEST_BEGIN_LOOP;	}break;
	case QS_End_Loop:		{ eIconType = ICONTYPE_QUEST_END_LOOP;		}break;
	case QS_Begin_Tactics:	{ eIconType = ICONTYPE_QUEST_BEGIN_GUILD;	}break;
	case QS_End_Tactics:	{ eIconType = ICONTYPE_QUEST_END_GUILD;		}break;
	case QS_Begin_Weekly:	{ eIconType = ICONTYPE_QUEST_BEGIN_WEEKLY;	}break;
	case QS_End_Weekly:		{ eIconType = ICONTYPE_QUEST_END_WEEKLY;	}break;
	case QS_Begin_Couple:	{ eIconType = ICONTYPE_QUEST_BEGIN_COUPLE;	}break;
	case QS_End_Couple:		{ eIconType = ICONTYPE_QUEST_END_COUPLE;	}break;
	case QS_Begin_Event:	{ eIconType = ICONTYPE_QUEST_CARD_BEGIN;	}break;
	case QS_End_Event:		{ eIconType = ICONTYPE_QUEST_CARD_END;		}break;
	default:
		return false;
	}

	return AddMiniMapIcon(kGuid, WorldPt, eIconType, ptWndPos);
}

bool PgMiniMapBase::AddMiniMapIcon(BM::GUID const& kGuid, NiPoint3 const& kWorldPt, IconType const eIconType, POINT2 const& ptWndPos, float fRadian)
{
	if(!g_pkWorld)
	{
		return false;
	}
	PROFILE_FUNC();
	NiPoint3 ViewPosition;
	NiPoint3 ScreenPosition;
	//POINT2 kIconTexturePos(0,0);
	//EMapIconTexType eIconTexType = EMITT_NONE;
	bool bCheckCachedInfo = false;
	bool bUpdateCachedInfo = false;

	kMapIconToIdxContainer::const_iterator c_it = ms_kMapIconTypeToIdxCont.find(eIconType);
	if(c_it == ms_kMapIconTypeToIdxCont.end())
	{
		return false;
	}

	PROFILE_BEGIN(AddMiniMapIcon1);
	MiniMapIconCont::iterator iter = m_kMiniMapIconCont.find(kGuid);
	if (m_kMiniMapIconCont.end() == iter)
	{
		PgPilot * pPilot = g_kPilotMan.FindPilot(kGuid);
		if( pPilot )
		{ // HIDE_MINIMAP 속성을 가진 NPC는 미니맵에 녹색점으로 표시하지 않음.
			PgNpc * pNpc = dynamic_cast<PgNpc*>(pPilot->GetUnit());
			if( pNpc && pNpc->HideMiniMap() )
			{
				return false;
			}
		}
		// new icon, insert to icon cont.
		std::pair<MiniMapIconCont::iterator, bool> ret = m_kMiniMapIconCont.insert(std::make_pair(kGuid, SMiniMapIconInfo()));
		assert(ret.second);
		if (ret.second)
		{
			ret.first->second.kObjectGuid = kGuid;
			ret.first->second.bDirty = true;
			bCheckCachedInfo = true;
			iter = ret.first;
		}
	}
	else
	{
		bCheckCachedInfo = true;
	}
	PROFILE_END();

	PROFILE_BEGIN(AddMiniMapIcon2);
	if (bCheckCachedInfo)
	{
		SMiniMapIconInfo& rkIconInfo = iter->second;
		if (rkIconInfo.bDirty)
		{
			bUpdateCachedInfo = true;
		}
		// check cached info vs current request.
		if (rkIconInfo.kIconType != eIconType)
		{
			bUpdateCachedInfo = true;
			NiScreenElements* pOldIconTexture = rkIconInfo.pIconTexture;
			rkIconInfo.kIconType = eIconType;
			if (false == GetMiniMapIcon(eIconType, rkIconInfo.kLastIconTexturePos, rkIconInfo.kIconTexType, rkIconInfo.kIconSize, rkIconInfo.pIconTexture))
			{
				return false;
			}

			if( 0 <= rkIconInfo.iIconPolygonIndex
			&&	pOldIconTexture
			&&	pOldIconTexture != rkIconInfo.pIconTexture ) // 텍스쳐 변경시, 기존 텍스쳐에서 폴리곤을 빼준다
			{
				MiniMapIconCont::const_iterator mod_iter = m_kMiniMapIconCont.begin();
				while( m_kMiniMapIconCont.end() != mod_iter )
				{
					MiniMapIconCont::mapped_type const& rkVal = (*mod_iter).second;
					if( rkIconInfo.kObjectGuid != rkVal.kObjectGuid
					&&	pOldIconTexture == rkVal.pIconTexture
					&&	rkIconInfo.iIconPolygonIndex == rkVal.iIconPolygonIndex )
					{
						break;
					}
					++mod_iter;
				}
				if( m_kMiniMapIconCont.end() == mod_iter )
				{
					pOldIconTexture->Remove(rkIconInfo.iIconPolygonIndex);
				}
				rkIconInfo.iIconPolygonIndex = -1; // 무조건 Init
			}
		}
		if (rkIconInfo.fLastRotationRadian != fRadian)
		{
			bUpdateCachedInfo = true;
			rkIconInfo.fLastRotationRadian = fRadian;
		}
		if (rkIconInfo.ptLastWorldPos != kWorldPt)
		{
			bUpdateCachedInfo = true;
			rkIconInfo.ptLastWorldPos = kWorldPt;
		}
		if (rkIconInfo.ptLastPos != ptWndPos)
		{
			bUpdateCachedInfo = true;
			rkIconInfo.ptLastPos = ptWndPos;
		}
		if (rkIconInfo.bIsAniIcon || 
			eIconType == ICONTYPE_ME ||
			eIconType == ICONTYPE_PARTY ||
			eIconType == ICONTYPE_SELECT_OBJECT||
			eIconType == ICONTYPE_QUEST_BEGIN_CHANGECLASS
			)
		{
			PROFILE_BEGIN(AddMiniMapIcon21);
			// calc new icon texture pos
			MiniMapAniIconCont::iterator ani_iter = m_kAniIconCont.find(eIconType);
			if( ani_iter != m_kAniIconCont.end() )
			{
				MiniMapAniIconCont::mapped_type& kAniInfo = ani_iter->second;

				MiniMapAniIconContainer::iterator aniicon_iter = kAniInfo.begin();
				if( aniicon_iter != kAniInfo.end() )
				{
					MiniMapAniIconContainer::value_type& kAniImgInfo = (*aniicon_iter);
					kAniImgInfo.NexFrame(g_pkWorld->GetAccumTime());
					POINT2 kNowPos;
					kNowPos.x = (kAniImgInfo.NowFrame() % kAniImgInfo.UVInfo().x) * rkIconInfo.kIconSize.x;
					kNowPos.y = (kAniImgInfo.NowFrame() / kAniImgInfo.UVInfo().x) * rkIconInfo.kIconSize.y;

					if (rkIconInfo.bIsAniIcon == false || kNowPos != rkIconInfo.kLastIconTexturePos)
					{
						rkIconInfo.bIsAniIcon = true;
						rkIconInfo.kLastIconTexturePos = kNowPos;
						bUpdateCachedInfo = true;
					}
				}
			}
			PROFILE_END();
		}
	}
	PROFILE_END();

	if (bUpdateCachedInfo)
	{
		iter->second.bDirty = false;
	}
	else
	{
		// current request is same as cached info. so do not need to update icon.
		return true;
	}

	if( m_spCamera->WorldPtToScreenPt3(kWorldPt, ViewPosition.x, ViewPosition.y, ViewPosition.z) == false )
	{
		return false;
	}

	if( ViewPtToScreenPt(ViewPosition, ScreenPosition) )
	{
		POINT2 kDrawPosPt = IconDrawPosition(ptWndPos, ScreenPosition);
		if (eIconType == ICONTYPE_SELECT_OBJECT)
		{
			kDrawPosPt.y -= iter->second.kIconSize.y / 2;
		}
		addMiniMapIcon(ptWndPos, kDrawPosPt, iter->second.kLastIconTexturePos, iter->second, 1.0f, iter->second.kIconTexType, fRadian);
		return true;
	}
	else
	{
		if (iter->second.iIconPolygonIndex >= 0 && iter->second.pIconTexture != NULL)
		{
			// disable polygon
			iter->second.pIconTexture->SetRectangle(iter->second.iIconPolygonIndex, 0, 0, 0, 0);
		}
	}
	return false;
}

bool PgMiniMapBase::CreateScreenTexture(std::string const& kMiniMapImage)
{
	if( kMiniMapImage.empty() )
	{
		return false;
	}
	else
	{
		m_spMiniMapImage = g_kNifMan.GetTexture(kMiniMapImage);
		if( !m_spMiniMapImage )
		{
			return false;
		}

		m_kMiniMapSize.x = static_cast<float>(m_spMiniMapImage->GetWidth());
		m_kMiniMapSize.y = static_cast<float>(m_spMiniMapImage->GetHeight());
		m_fScreenImageRatio = (m_kMiniMapSize.x / m_kMiniMapSize.y) * MINIMAP_SCREEND_IMAGE_RATIO; // 이미지가 4:3비율이라면 1.0이 된다.
		m_iDrawGap = static_cast<int>((m_kMiniMapSize.y - m_kMiniMapSize.x * m_fScreenImageRatio) * 0.5f);
	}

	m_spMiniMapScreenTexture = NiNew NiScreenElements(NiNew NiScreenElementsData(false, false, 1));
	if( m_spMiniMapScreenTexture )
	{
		m_spMiniMapScreenTexture->Insert(4);
		NiTexturingPropertyPtr spTextureProp = NiNew NiTexturingProperty;

		spTextureProp->SetBaseTexture(m_spMiniMapImage);

		spTextureProp->SetBaseFilterMode(NiTexturingProperty::FILTER_NEAREST);
		spTextureProp->SetApplyMode(NiTexturingProperty::APPLY_REPLACE);
		spTextureProp->SetBaseClampMode(NiTexturingProperty::CLAMP_S_CLAMP_T);

		NiAlphaPropertyPtr spAlphaProp = NiNew NiAlphaProperty;
		spAlphaProp->SetAlphaBlending(true);

		NiZBufferPropertyPtr spZBufferProp = NiNew NiZBufferProperty;
		spZBufferProp->SetZBufferTest(false);
		spZBufferProp->SetZBufferWrite(true);

		float screenHalfHeight = (1.0f - m_fZoomFactor) / 2;

		m_spMiniMapScreenTexture->SetRectangle(0, 0.1f, 0.1f, 0.1f, 0.1f);
		m_spMiniMapScreenTexture->UpdateBound();
		m_spMiniMapScreenTexture->SetTextures(0, 0,
			0.0f, 0.5f - screenHalfHeight, 
			1.0f, 0.5f + screenHalfHeight);

		m_spMiniMapScreenTexture->AttachProperty(spTextureProp);
		m_spMiniMapScreenTexture->AttachProperty(spAlphaProp);
		m_spMiniMapScreenTexture->AttachProperty(spZBufferProp);

		m_spMiniMapScreenTexture->UpdateProperties();
		m_spMiniMapScreenTexture->Update(0.0f);
		return true;
	}
	return false;
}

bool PgMiniMapBase::GetMiniMapIcon(IconType const Type, POINT2& IconPos, EMapIconTexType& kType, POINT2& IconSize, NiScreenElements*& pIconTexture)
{
	PROFILE_FUNC();
	if( Type <= ICONTYPE_NONE )
	{
		return false;
	}

	kMapIconToIdxContainer::const_iterator c_it = ms_kMapIconTypeToIdxCont.find(Type);
	if(c_it == ms_kMapIconTypeToIdxCont.end())
	{
		return false;
	}

	SIconTypeToIndex const& kInfo = (*c_it).second;
	if (kInfo.Index < 0)
	{
		return false;
	}

	kType = kInfo.TexType;
	IconPos = kInfo.IconPos;
	IconSize = POINT2(kInfo.TexInfo.W, kInfo.TexInfo.H);
	pIconTexture = kInfo.TexInfo.IconTexture;

	return true;
}

void PgMiniMapBase::AddMapIconTex(char const* pPath, SMapIconTexInfo& rkInfo)
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

bool PgMiniMapBase::ViewPtToScreenPt(NiPoint3 const& ViewPt, NiPoint3& ScreenPt) const
{
	PROFILE_FUNC();
	bool result = true;
	PG_ASSERT_LOG (m_fZoomFactor >= 0.0f && m_fZoomFactor <= DEFAULT_LIMIT_ZOOM_FACTOR);
	float ScreenHalfWidth = (1.0f - m_fZoomFactor) * 0.5f;
	float ScreenHalfHeight = (1.0f - m_fZoomFactor) * 0.5f;

	float ScreenLeft = m_kScreenCenter.x - ScreenHalfWidth;
	float ScreenTop = m_kScreenCenter.y - ScreenHalfHeight;

	//! screen 밖에 있더라도, 상대적인 위치를 return.
	if (ViewPt.x < ScreenLeft || ViewPt.x > ScreenLeft + 2 * ScreenHalfWidth)
		result = false;

	if ((1 - ViewPt.y) < ScreenTop || (1 - ViewPt.y) > ScreenTop + 2 * ScreenHalfHeight)
		result = false;

	ScreenPt.x = (ViewPt.x - ScreenLeft) / (2 * ScreenHalfWidth);
	ScreenPt.y = ((1 - ViewPt.y) - ScreenTop) / (2 * ScreenHalfHeight);

	return result;
}

IconType const PgMiniMapBase::GetTriggerType(PgTrigger* pkTrigger) const
{
	PROFILE_FUNC();
	if( pkTrigger )
	{
		switch( pkTrigger->GetTriggerType() )
		{
		case PgTrigger::TRIGGER_TYPE_PORTAL:	{ return ICONTYPE_PORTAL;			}break;
		case PgTrigger::TRIGGER_TYPE_MISSION:	{ return ICONTYPE_MISSION;			}break;
		case PgTrigger::TRIGGER_TYPE_JUMP:		{ return ICONTYPE_JUMP;				}break;
		case PgTrigger::TRIGGER_TYPE_TELEJUMP:	{ return ICONTYPE_TELEJUMP;			}break;
		case PgTrigger::TRIGGER_TYPE_TELEPORT:	{ return ICONTYPE_TELEPORT;			}break;
		case PgTrigger::TRIGGER_TYPE_LOCATION:	{ return ICONTYPE_UNKNOWN_POINT;	}break;
		case PgTrigger::TRIGGER_TYPE_MISSION_EASY:	{ return ICONTYPE_MISSION_EASY;		}break;
		case PgTrigger::TRIGGER_TYPE_PARTYPORTAL:	{ return ICONTYPE_PORTAL;			}break;
		case PgTrigger::TRIGGER_TYPE_PARTY_MEMBER_PORTAL:	{ return ICONTYPE_PORTAL;			}break;	
		case PgTrigger::TRIGGER_TYPE_CHANNELPORTAL:			{ return ICONTYPE_PORTAL;			}break;
		case PgTrigger::TRIGGER_TYPE_MISSION_EVENT_NOT_HAVE_ARCADE:	{ return ICONTYPE_PORTAL;;	}break;
		case PgTrigger::TRIGGER_TYPE_MISSION_EVENT_NPC:         { return ICONTYPE_PORTAL;       }break;
		case PgTrigger::TRIGGER_TYPE_RAG_PARTY_MEMBER_PORTAL:	{ return ICONTYPE_PORTAL;		}break;
		case PgTrigger::TRIGGER_TYPE_MISSION_UNUSED_GADACOIN:	{ return ICONTYPE_MISSION;		}break;
		case PgTrigger::TRIGGER_TYPE_KING_OF_HILL:
			{
				ETeam const EntityTeam = static_cast<ETeam>(pkTrigger->EntityTeam());
				if( EntityTeam == TEAM_NONE )
				{// TEAM_NONE
					return ICONTYPE_NONE;
				}
				else
				{
					int const EntityLevel = GetEntityLevelByTriggerID( pkTrigger->GetID().c_str() );
					int const EntityLink = GetEntityLinkByTriggerID( pkTrigger->GetID().c_str() );
					int TotalLevel = EntityLevel + EntityLink;
					if( EntityTeam == TEAM_RED )
					{// TEAM_RED
						switch( TotalLevel )
						{
						case 1: { return ICONTYPE_STRONGHOLD_RED_LV1; }break;
						case 2: { return ICONTYPE_STRONGHOLD_RED_LV2; }break;
						case 3: { return ICONTYPE_STRONGHOLD_RED_LV3; }break;
						case 4: { return ICONTYPE_STRONGHOLD_RED_LV4; }break;
						case 5:
						case 6:
						case 7:
						case 8:
						case 9: { return ICONTYPE_STRONGHOLD_RED_LV5; }break;
						}
					}
					else if( EntityTeam == TEAM_BLUE )
					{// TEAM_BLUE
						switch( TotalLevel )
						{
						case 1: { return ICONTYPE_STRONGHOLD_BLUE_LV1; }break;
						case 2: { return ICONTYPE_STRONGHOLD_BLUE_LV2; }break;
						case 3: { return ICONTYPE_STRONGHOLD_BLUE_LV3; }break;
						case 4: { return ICONTYPE_STRONGHOLD_BLUE_LV4; }break;
						case 5:
						case 6:
						case 7:
						case 8:
						case 9:	{ return ICONTYPE_STRONGHOLD_BLUE_LV5; }break;
						}
					}
				}
			}break;
		case PgTrigger::TRIGGER_TYPE_LOVE_FENCE:
			{
				if( pkTrigger->LoveFenceLife() )
				{//울타리가 살아있는 상태
					return ICONTYPE_LOVE_FENCE;
				}
				else
				{//울타리가 파괴된 상태
					return ICONTYPE_DESTROYED_LOVE_FENCE;
				}
			}break;
		}
	}
	return ICONTYPE_NONE;
}

void PgMiniMapBase::ArrangeScreenBoundary()
{
	PROFILE_FUNC();
	float ScreenHalfWidth = (1.0f - m_fZoomFactor) * 0.5f;
	float ScreenHalfHeight = (1.0f - m_fZoomFactor) * 0.5f;

	if (m_kScreenCenter.x - ScreenHalfWidth < 0.0f)
	{
		m_kScreenCenter.x = ScreenHalfWidth;
	}

	if (m_kScreenCenter.x + ScreenHalfWidth > 1.0f)
	{
		m_kScreenCenter.x = 1.0f - ScreenHalfWidth;
	}

	if (m_kScreenCenter.y - ScreenHalfHeight< 0.0f)
	{
		m_kScreenCenter.y = ScreenHalfHeight;
	}

	if (m_kScreenCenter.y + ScreenHalfHeight > 1.0f)
	{
		m_kScreenCenter.y = 1.0f - ScreenHalfHeight;
	}
}

bool PgMiniMapBase::addMiniMapIcon(const POINT2& ptWndPos, const POINT2& screenPixel, const POINT2& iconTexturePos, SMiniMapIconInfo& kMiniMapIconInfo, float fAlpha, EMapIconTexType TexType, float fDir)
{
	PROFILE_FUNC();
	POINT2 IconSize = kMiniMapIconInfo.kIconSize;
	POINT2 IconHalfSize = POINT2(static_cast<int>(IconSize.x * 0.5f), static_cast<int>(IconSize.y * 0.5f));

	RECT rtIconRect;
	E_RECT_CHECK_RESULT const kResult = GetIconRect(rtIconRect, ptWndPos, screenPixel, IconSize);
	if( ERCR_OUT == kResult )
	{//화면에서 안보임
		if (kMiniMapIconInfo.iIconPolygonIndex >= 0 && kMiniMapIconInfo.pIconTexture != NULL)
		{
			// disable polygon
			kMiniMapIconInfo.pIconTexture->SetRectangle(kMiniMapIconInfo.iIconPolygonIndex, 0, 0, 0, 0);
		}
		return true;
	}

	float uiScreenWidth = XUIMgr.GetResolutionSize().x;
	float uiScreenHeight = XUIMgr.GetResolutionSize().y;

	unsigned int uiWidth = 0;
	unsigned int uiHeight = 0;
	NiTexture* pTexture = 0;

	NiTListIterator kPos = kMiniMapIconInfo.pIconTexture->GetPropertyList().GetHeadPos();
	while (kPos)
	{
		NiProperty* pProperty = kMiniMapIconInfo.pIconTexture->GetPropertyList().GetNext(kPos);
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

	int iPoly = -1;

	if( ERCR_PART == kResult )
	{//부분합
		int left = 0;
		int right = 0;
		int top = 0;
		int bottom = 0;

		if (screenPixel.x - IconHalfSize.x < ptWndPos.x)
			left = ptWndPos.x - (screenPixel.x - IconHalfSize.x);
		if (screenPixel.x + IconHalfSize.x > ptWndPos.x + m_kWndSize.x)
			right = screenPixel.x + IconHalfSize.x - (ptWndPos.x + m_kWndSize.x);
		if (screenPixel.y - IconHalfSize.y < ptWndPos.y)
			top = ptWndPos.y - (screenPixel.y - IconHalfSize.y);
		if (screenPixel.y + IconHalfSize.y > ptWndPos.y + m_kWndSize.y)
			bottom = screenPixel.y + IconHalfSize.y - (ptWndPos.y + m_kWndSize.y);

		PG_ASSERT_LOG (!(left && right));
		PG_ASSERT_LOG (!(top && bottom));

		{
			if (pTexture)
			{
				if (kMiniMapIconInfo.iIconPolygonIndex < 0)
				{
					iPoly = kMiniMapIconInfo.pIconTexture->Insert(4);
				}
				else
				{
					iPoly = kMiniMapIconInfo.iIconPolygonIndex;
				}

				kMiniMapIconInfo.pIconTexture->SetRectangle(iPoly, 
					(rtIconRect.left/uiScreenWidth), (rtIconRect.top/uiScreenHeight),
					(rtIconRect.right/uiScreenWidth), (rtIconRect.bottom/uiScreenHeight));
				//spIconTexture->UpdateBound();

				float fLeft = (float)(iconTexturePos.x+left) / (float)uiWidth;
				float fTop = (float)(iconTexturePos.y+top) / (float)uiHeight;
				float fRight = (float)(iconTexturePos.x+IconSize.x-right) / (float)uiWidth;
				float fBottom = (float)(iconTexturePos.y+IconSize.y-bottom) / (float)uiHeight;

				if(fDir != 0)
				{// 텍스쳐를 회전시켜서 가져온다.
					fLeft = (float)(iconTexturePos.x+left);
					fTop = (float)(iconTexturePos.y+top);
					fRight = (float)(iconTexturePos.x+IconSize.x-right);
					fBottom = (float)(iconTexturePos.y+IconSize.y-bottom);

					float fLeftOri = (float)(iconTexturePos.x);
					float fTopOri = (float)(iconTexturePos.y);
					float fRightOri = (float)(iconTexturePos.x+IconSize.x);
					float fBottomOri = (float)(iconTexturePos.y+IconSize.y);

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
						kMiniMapIconInfo.pIconTexture->SetTexture(iPoly, nn, 0, kVec);
					}
				}
				else
				{
					kMiniMapIconInfo.pIconTexture->SetTextures(iPoly, 0, fLeft, fTop, fRight, fBottom);
				}
			}
		}
	}
	else
	{//전체합
		if (pTexture)
		{
			if (kMiniMapIconInfo.iIconPolygonIndex < 0)
			{
				iPoly = kMiniMapIconInfo.pIconTexture->Insert(4);
			}
			else
			{
				iPoly = kMiniMapIconInfo.iIconPolygonIndex;
			}
			kMiniMapIconInfo.pIconTexture->SetRectangle(iPoly, 
				(rtIconRect.left/uiScreenWidth), (rtIconRect.top/uiScreenHeight),
				(rtIconRect.right/uiScreenWidth), (rtIconRect.bottom/uiScreenHeight));
			//spIconTexture->UpdateBound();

			float fLeft = (float)(iconTexturePos.x) / (float)uiWidth;
			float fTop = (float)(iconTexturePos.y) / (float)uiHeight;
			float fRight = (float)(iconTexturePos.x+IconSize.x) / (float)uiWidth;
			float fBottom = (float)(iconTexturePos.y+IconSize.y) / (float)uiHeight;

			if(fDir != 0)
			{// 텍스쳐를 회전시켜서 가져온다.
				fLeft = (float)(iconTexturePos.x);
				fTop = (float)(iconTexturePos.y);
				fRight = (float)(iconTexturePos.x+IconSize.x);
				fBottom = (float)(iconTexturePos.y+IconSize.y);

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
					kMiniMapIconInfo.pIconTexture->SetTexture(iPoly, nn, 0, kVec);
				}
			}
			else
			{
				kMiniMapIconInfo.pIconTexture->SetTextures(iPoly, 0, fLeft, fTop, fRight, fBottom);
			}
		}
	}

	if (iPoly >= 0)
	{
		kMiniMapIconInfo.iIconPolygonIndex = iPoly;
	}
	return true;
}

void PgMiniMapBase::ClearRemoveMiniMapIcon()
{
	BM::CAutoMutex kLock(m_kMutex);

	m_kRemoveMiniMapIconCont.clear();
}

void PgMiniMapBase::RemoveMiniMapIcon(BM::GUID const & rkGuid)
{
	BM::CAutoMutex kLock(m_kMutex);

	m_kRemoveMiniMapIconCont.push_back(rkGuid);
}

void PgMiniMapBase::RefreshMiniMapIcon(BM::GUID const& rkGuid)
{
	BM::CAutoMutex kLock(m_kMutex);
	MiniMapIconCont::iterator iter = m_kMiniMapIconCont.find(rkGuid);
	if(iter != m_kMiniMapIconCont.end())
	{
		(*iter).second.bDirty = true;
	}
}

void PgMiniMapBase::DoRemoveMiniMapIcon()
{
	BM::CAutoMutex kLock(m_kMutex);
	if( !m_kRemoveMiniMapIconCont.empty() )
	{
		VEC_GUID::const_iterator c_iter=m_kRemoveMiniMapIconCont.begin();
		while( c_iter!=m_kRemoveMiniMapIconCont.end() )
		{
			MiniMapIconCont::iterator iter = m_kMiniMapIconCont.find(*c_iter);
			if (m_kMiniMapIconCont.end() != iter)
			{
				iter->second.pIconTexture->SetRectangle(iter->second.iIconPolygonIndex, 0, 0, 0, 0);
			}

			++c_iter;
		}
		m_kRemoveMiniMapIconCont.clear();
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool PgAlwaysMiniMap::m_bDrawText = true;
bool PgAlwaysMiniMap::m_bMapTeleMoveUseItem = false;
BM::GUID const PgAlwaysMiniMap::m_kSelectArrowGUID = BM::GUID::Create();

wchar_t const * const NPC_ICON_TYPE_NAME = _T("ICON_PATH_NPC");
wchar_t const * const NPC_ICON_TYPE_NAME2 = _T("ICON_PATH_NPC2");

PgAlwaysMiniMap::PgAlwaysMiniMap(void)
	: m_kFixedSize(false)
	, m_pObjectContainer(NULL)
	, m_pTriggerContainer(NULL)
	, m_kDrawHeight()
{
	for(int i = 0; i < EIT_END; ++i)
	{
		m_bShowFlag[i] = true;
	}
	m_kMapRenderTextCont.clear();
	m_kQuestInfoCont.clear();
}

PgAlwaysMiniMap::~PgAlwaysMiniMap(void)
{
	Terminate();
}

bool PgAlwaysMiniMap::Initialize(S_INIT_INFO& kInitInfo)
{
	bool bRst = PgMiniMapBase::BaseInitialize(kInitInfo);
	if( bRst )
	{
		m_pObjectContainer = kInitInfo.pkObjectCont;
		m_pTriggerContainer = kInitInfo.pkTriggerCont;
		if( kInitInfo.pkHeightInfo )
		{
			m_kDrawHeight = (*kInitInfo.pkHeightInfo);
		}

		std::pair<MiniMapAniIconCont::iterator, bool> Rst = m_kAniIconCont.insert(std::make_pair(ICONTYPE_ME, MiniMapAniIconContainer()));
		if( Rst.second )
		{
			SMiniMapAniIcon	Icon(0.2f, 4, POINT2(2, 2));
			Rst.first->second.push_back(Icon);
		}
		Rst = m_kAniIconCont.insert(std::make_pair(ICONTYPE_PARTY, MiniMapAniIconContainer()));
		if( Rst.second )
		{
			SMiniMapAniIcon	Icon(0.2f, 4, POINT2(2, 2));
			Rst.first->second.push_back(Icon);
		}
		Rst = m_kAniIconCont.insert(std::make_pair(ICONTYPE_SELECT_OBJECT, MiniMapAniIconContainer()));
		if( Rst.second )
		{
			SMiniMapAniIcon	Icon(0.2f, 4, POINT2(2, 2));
			Rst.first->second.push_back(Icon);
		}
		Rst = m_kAniIconCont.insert(std::make_pair(ICONTYPE_QUEST_BEGIN_CHANGECLASS, MiniMapAniIconContainer()));
		if( Rst.second )
		{
			SMiniMapAniIcon	Icon(0.8f, 2, POINT2(2, 1));
			Rst.first->second.push_back(Icon);
		}

		UpdateQuest();
		CreateTriggerIconUI();

		m_bInitialized = true;
	}
	return m_bInitialized;
}

bool PgAlwaysMiniMap::UpdateQuest()
{
	PROFILE_FUNC();

	SetRefreshAllMiniMapIconInfo(true);

	if( !m_pObjectContainer )
	{
		return false;
	}

	m_kQuestInfoCont.clear();
	if( !GetViewFlag(EIT_QUEST) )
	{
		return false;
	}

	PgPlayer *pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkPlayer )
	{
		return false;
	}

	int const iPlayerLevel = pkPlayer->GetAbil(AT_LEVEL);

	PgWorld::ObjectContainer::iterator quest_iter = m_pObjectContainer[WOCID_MAIN].begin();
	while( quest_iter != m_pObjectContainer[WOCID_MAIN].end() )
	{
		PgWorld::ObjectContainer::mapped_type& pkQuestInfo = quest_iter->second;
		++quest_iter;
		
		if( !pkQuestInfo )
		{
			continue;
		}

		PgPilot* pkPilot = pkQuestInfo->GetPilot();
		if( !pkPilot )
		{
			continue;
		}

		BM::GUID const& kGuid = pkPilot->GetGuid();

		ContNpcQuestInfo kQuestInfoCont, kResultVec;
		size_t const iCountRet = g_kQuestMan.PopNPCQuestInfo(kGuid, kQuestInfoCont);
		if( !iCountRet ) 
		{
			continue;
		}

		int const iShowAllQuest = g_kGlobalOption.GetValue(XML_ELEMENT_ETC, STR_OPTION_SHOW_ALL_QUEST);
		if( 0 != iShowAllQuest )
		{
			kResultVec.insert(kResultVec.end(), kQuestInfoCont.begin(), kQuestInfoCont.end());
		}
		else
		{
			//10레벨 이상 차이나는 시작가능한 퀘스트 표시 생략
			std::remove_copy_if(kQuestInfoCont.begin(), kQuestInfoCont.end(), std::back_inserter(kResultVec), SNPCQuestInfo::SPlayerLevelDiff(iPlayerLevel));
		}

		// 스크롤 퀘스트 마크 제거
		ContNpcQuestInfo::iterator new_end = std::remove_if( kResultVec.begin(), kResultVec.end(), SHideScrollQuestMark() );
		kResultVec.erase( new_end, kResultVec.end() );

		SQuestOrderByResult const eState = PgQuestManUtil::QuestOrderByState(kGuid, kResultVec);
		m_kQuestInfoCont.insert( std::make_pair(kGuid, eState) );
	}
	return true;	
}

void PgAlwaysMiniMap::ClearTriggerIconUI()
{
	m_kContTriggerIconUI.clear();
}

bool PgAlwaysMiniMap::CreateTriggerIconUI()
{
	if(!m_pTriggerContainer)
	{
		return false;
	}

	ClearTriggerIconUI();

	NiPoint3 ViewPosition;
	int iRoadSignIdx = 0;

	PgWorld::TriggerContainer::const_iterator trigger_iter = m_pTriggerContainer->begin();
	while( trigger_iter != m_pTriggerContainer->end() )
	{
		PgWorld::TriggerContainer::mapped_type const& pkTrigger = trigger_iter->second;
		if( pkTrigger && !pkTrigger->MinimapHide() )
		{
			if( PgTrigger::TRIGGER_TYPE_ROADSIGN==pkTrigger->GetTriggerType() )
			{
				NiAVObject* pkTriggerObject = pkTrigger->GetTriggerObject();
				if( pkTriggerObject )
				if( m_spCamera->WorldPtToScreenPt3(pkTriggerObject->GetWorldTranslate(), ViewPosition.x, ViewPosition.y, ViewPosition.z) )
				{
					MiniMapIconUICont::mapped_type kInfo(static_cast<int>(pkTrigger->GetTriggerType()));
					kInfo.iIdx = iRoadSignIdx;
					kInfo.iTTNo = PgStringUtil::SafeAtoi(pkTrigger->GetParamFromParamMap("TEXT_NO"));
					kInfo.ViewPosition = ViewPosition;
					kInfo.W = PgStringUtil::SafeAtoi(pkTrigger->GetParamFromParamMap("WIDTH"));
					kInfo.H = PgStringUtil::SafeAtoi(pkTrigger->GetParamFromParamMap("HEIGHT"));
					kInfo.iTargetIdx = PgStringUtil::SafeAtoi(pkTrigger->GetParamFromParamMap("TargetIdx"));
					kInfo.iUVIndex = PgStringUtil::SafeAtoi(pkTrigger->GetParamFromParamMap("UVIndex"));
					kInfo.iTransX = PgStringUtil::SafeAtoi(pkTrigger->GetParamFromParamMap("TransX"));
					kInfo.iTransY = PgStringUtil::SafeAtoi(pkTrigger->GetParamFromParamMap("TransY"));
					m_kContTriggerIconUI.insert(std::make_pair(pkTrigger->GetGuid(), kInfo));
					
					++iRoadSignIdx;
				}
			}
		}
		++trigger_iter;
	}
	return true;
}

void PgAlwaysMiniMap::RefreshZoomMiniMap()
{
	PROFILE_FUNC();
	if( !Initialized() )
	{
		return;
	}

	PgActor *pkActor = g_kPilotMan.GetPlayerActor();

	if(pkActor)
	{
		if (pkActor->GetPos() != m_kActorLastPos)
		{
			m_bRefreshAllMiniMapIconInfo = true;
			m_kActorLastPos = pkActor->GetPos();
		}

		NiPoint3 ViewPosition;
		if (m_spCamera->WorldPtToScreenPt3(pkActor->GetPos(), ViewPosition.x, ViewPosition.y, ViewPosition.z))
		{//캐릭터 위치로 이동
			m_kScreenCenter.x = ViewPosition.x;
			m_kScreenCenter.y = 1.0f - ViewPosition.y;
		}
	}

	PgMiniMapBase::RefreshZoomMiniMap();
}

void PgAlwaysMiniMap::Draw(PgRenderer* pkRenderer)
{
	PROFILE_FUNC();
	if(pkRenderer && Initialized())
	{
		PgMiniMapBase::Draw(pkRenderer);
		kMapTextContainer::iterator txt_iter = m_kMapRenderTextCont.begin();
		while( txt_iter != m_kMapRenderTextCont.end() )
		{
			kMapTextContainer::mapped_type const& kTextInfo = txt_iter->second;
			if( kTextInfo.IsShow() && (kTextInfo.IsAlways() || m_bDrawText) )
			{
				kMapTextContainer::mapped_type& kTextInfo = txt_iter->second;
				kTextInfo.Render(pkRenderer);
			}
			++txt_iter;
		}
	}
}

void PgAlwaysMiniMap::Update(float const fAccumTime, float const fFrameTime)
{
	PROFILE_FUNC();
	if( !Initialized() )
	{
		return;
	}

	RefreshZoomMiniMap();
}

void PgAlwaysMiniMap::OnMouseOver(POINT2 const& ptWndPos, POINT2 const& ptMouse, char const* wndName) const
{
	PROFILE_FUNC();
	if( !Initialized() )
	{
		return;
	}

	std::wstring wstrText;
	int iIconNo = 0;
	bool use_M_ToolTip2 = false;
	bool bIsToolTipChange = false;
	bool bIsToolTipClose = false;

	PgActor* pkActor = OnOverPickActor(ptWndPos, ptMouse, EMITT_SMALL);
	PgTrigger* pkTrigger = OnOverPickTrigger(ptWndPos, ptMouse, EMITT_CORE);

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
				else
				{
					if(rkRscIcon.wstrImageID.compare(NPC_ICON_TYPE_NAME2) == 0)
					{
						iIconNo = rkRscIcon.iIdx;
						use_M_ToolTip2 = true;
					}
				}
			}
		}
		wstrText = pkActor->GetPilot()->GetName();
		if( wstrText.empty() )
		{
			bIsToolTipClose = true;
		}
		else
		{
			bIsToolTipChange = true;
		}

		if( g_pkWorld
			&& false == g_pkWorld->IsHaveAttr(GATTR_FLAG_EMPORIABATTLE)
			&& g_pkWorld->IsHaveAttr(GATTR_FLAG_PVP) )
		{// 엠포리아가 아닌 PVP 맵에서
			PgPilot * pPilot = g_kPilotMan.GetPlayerPilot();
			if( pPilot )
			{
				int const MyTeam = pPilot->GetAbil(AT_TEAM);
				int const OtherTeam = pkActor->GetPilot()->GetAbil(AT_TEAM);

				if( MyTeam != OtherTeam )
				{// 같은 팀이 아니고
					PgPlayer * pOtherPlayer = dynamic_cast<PgPlayer*>(pkActor->GetUnit());
					if( pOtherPlayer
						&& false == pOtherPlayer->VisibleMyMinimapIcon() )
					{// 미니맵에 아이콘이 표시되지 않는 상대면 툴팁 표시해줄 필요 없다.
						bIsToolTipChange = false;
					}
				}
			}
		}
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
		wstrText = OnOverPickParty(ptWndPos, ptMouse, EMITT_MYPARTY);
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
		lwPoint2 kPoint(ptMouse);
		if(false==use_M_ToolTip2)
		{
			lwCallMutableToolTipByText(wstrText, kPoint, iIconNo, wndName);
		}
		else
		{
			lwCallMutableToolTipByText(wstrText, kPoint, iIconNo, "M_ToolTip2");
		}
	}
	
	if( bIsToolTipClose )
	{
		m_bIsShowToolTip = false;
		lwCloseToolTip();
	}
}

void PgAlwaysMiniMap::OnMouseClick(POINT2 const& ptWndPos, POINT2 const& ptMouse) const
{
	PROFILE_FUNC();
	if( !Initialized() )
	{
		return;
	}

	if (!g_pkWorld)
	{
		return;
	}

	PgActor* pkActor = OnOverPickActor(ptWndPos, ptMouse, EMITT_LARGE);
	PgTrigger* pkTrigger = OnOverPickTrigger(ptWndPos, ptMouse, EMITT_SMALL);
	// TYPE 2
	// 캐릭터 위에 화살표 띄움
	//if (pkTrigger)
	//{
	//	g_pkWorld->SetDirectionArrow(pkTrigger->GetTriggerObject());
	//}
	//else if(pkActor && pkActor->GetPilot())
	//{
	//	g_pkWorld->SetDirectionArrow(pkActor->GetPilot()->GetWorldObject());
	//}
	
	PgMiniMapUtil::Send_PT_C_M_REQ_ALARM_MINIMAP(ptWndPos, ptMouse); // 미니맵 알람 표시 기능

	if( m_bMapTeleMoveUseItem )
	{
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
			OnClickMiniMapTeleMoveToTarget(POINT3(kPoint.x, kPoint.y, kPoint.z));
		}
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

void PgAlwaysMiniMap::Terminate()
{
	m_pObjectContainer = NULL;
	m_pTriggerContainer = NULL;
	m_kMapRenderTextCont.clear();
	m_kQuestInfoCont.clear();
	ClearTriggerIconUI();
	PgMiniMapBase::Terminate();
}

void PgAlwaysMiniMap::OnClickIconInfoListItem()
{
	PROFILE_FUNC();
	if( m_kSelectObject.Type == 0 )
	{
		return;
	}

	if (m_pObjectContainer == NULL || m_pTriggerContainer == NULL)
	{
		return;
	}

	NiAVObject* pkPickObject = GetPickObject(m_kSelectObject);
	if (pkPickObject)
	{
		//g_pkWorld->SetDirectionArrow(pkPickObject);

		if( m_bMapTeleMoveUseItem )
		{
			NiPoint3 kPoint = NiPoint3::ZERO;

			PgActor* pkActor = NiDynamicCast(PgActor, pkPickObject);
			if( pkActor )
			{
				kPoint = pkActor->GetPos();
			}
			else
			{
				kPoint = pkPickObject->GetWorldTranslate();
			}

			if (kPoint != NiPoint3::ZERO)
			{
				kPoint.z += 80;
				OnClickMiniMapTeleMoveToTarget(POINT3(kPoint.x, kPoint.y, kPoint.z));
			}
		}
	}
}

POINT2 PgAlwaysMiniMap::ActorToMinimapScreenPos( PgActor* pkActor, POINT2 const ptWndPos )
{
	PROFILE_FUNC();
	if( pkActor )
	{
		if( m_spCamera )
		{
			NiPoint3 ViewPos;
			if( m_spCamera->WorldPtToScreenPt3(pkActor->GetPos(), ViewPos.x, ViewPos.y, ViewPos.z) )
			{
				NiPoint3 ScreenPos;
				if( ViewPtToScreenPt(ViewPos, ScreenPos) )
				{
					return IconDrawPosition(ptWndPos, ScreenPos);
				}
			}
		}
	}
	return POINT2();
}

bool PgAlwaysMiniMap::GetMinimapIconInfo(CONT_MINIMAP_ICON_INFO& Cont)
{
	PROFILE_FUNC();
	if (m_pObjectContainer == NULL || m_pTriggerContainer == NULL || m_spCamera == NULL)
	{
		return false;
	}

	PgWorld::ObjectContainer::iterator obj_itor = m_pObjectContainer[WOCID_MAIN].begin();
	while(obj_itor != m_pObjectContainer[WOCID_MAIN].end())
	{
		PgWorld::ObjectContainer::mapped_type pkObject = obj_itor->second;
		if( pkObject )
		{ // HIDE_MINIMAP 속성을 가진 NPC는 미니맵 하단에 표시되는 NPC 리스트에 넣지 않음.
			PgPilot * pPilot = pkObject->GetPilot();
			if( pPilot )
			{
				PgNpc * pNpc = dynamic_cast<PgNpc*>(pPilot->GetUnit());
				if( pNpc && pNpc->HideMiniMap() )
				{
					++obj_itor;
					continue;
				}
			}
		}
		if( pkObject && pkObject->GetPilot() && pkObject->GetPilot()->GetWorldObject() )
		{
			switch( pkObject->GetObjectID() )
			{
			case PgIXmlObject::ID_NPC:
				{
					PgActorNpc* pkNpc = dynamic_cast<PgActorNpc*>(pkObject);
					if( pkNpc )
					{
						NiPoint3 ViewPos;
						if( true == m_spCamera->WorldPtToScreenPt3( pkNpc->GetPos(), ViewPos.x, ViewPos.y, ViewPos.z ) )
						{
							SMINIMAP_ICON_INFO kInfo;
							kInfo.Type = PgAlwaysMiniMap::EIT_NPC;
							kInfo.kIconID = UNI(pkObject->GetID());
							kInfo.kIconName = pkObject->GetPilot()->GetName();
							Cont.insert(Cont.end(), kInfo);
						}
					}
				}break;
			}
		}
		++obj_itor;
	}

	//트리거는 일단 보류
	//PgWorld::TriggerContainer::iterator trigger_iter = m_pTriggerContainer->begin();
	//while(trigger_iter != m_pTriggerContainer->end())
	//{
	//	PgWorld::TriggerContainer::mapped_type pkTrigger = trigger_iter->second;
	//	if( pkTrigger && !pkTrigger->MinimapHide() )
	//	{
	//		switch( pkTrigger->GetTriggerType() )
	//		{
	//		case PgTrigger::TRIGGER_TYPE_PORTAL:
	//			{
	//				SMINIMAP_ICON_INFO kInfo;
	//				kInfo.Type = PgAlwaysMiniMap::EIT_TRIGGER;
	//				kInfo.kIconID = UNI(pkTrigger->GetID());
	//				kInfo.kIconName = TTW(pkTrigger->GetTriggerTitleTextID());
	//				Cont.insert(Cont.end(), kInfo);
	//			}break;
	//		}
	//	}
	//	++trigger_iter;
	//}

	return (!Cont.empty());
}

void PgAlwaysMiniMap::SelectObject(std::wstring const& kObjectID)
{
	PROFILE_FUNC();
	if( kObjectID.empty() )
	{
		m_kSelectObject.Type = 0;
		m_kSelectObject.kIconID = L"";
		m_kSelectObject.kIconName = L"";
		return;
	}

	NiAVObject* pkObject = OnOverPickActor(kObjectID);
	if( pkObject )
	{
		PgActor* pkActor = NiDynamicCast(PgActor, pkObject);
		if( pkActor && pkActor->GetPilot() )
		{
			m_kSelectObject.Type = PgAlwaysMiniMap::EIT_NPC;
			m_kSelectObject.kIconID = UNI(pkActor->GetID());
			m_kSelectObject.kIconName = pkActor->GetPilot()->GetName();
		}
		return;
	}

	pkObject = OnOverPickTrigger(kObjectID);
	if( pkObject )
	{
		PgTrigger* pkTrigger = dynamic_cast<PgTrigger*>( pkObject );
		if( pkTrigger )
		{
			m_kSelectObject.Type = PgAlwaysMiniMap::EIT_TRIGGER;
			m_kSelectObject.kIconID = UNI(pkTrigger->GetID());
			m_kSelectObject.kIconName = TTW(pkTrigger->GetTriggerTitleTextID());
		}
	}
}

PgIWorldObject * PgAlwaysMiniMap::GetObjectContainerByGUID(BM::GUID const & PlayerGuid) const
{
	PgWorld::ObjectContainer::iterator obj_iter = m_pObjectContainer[WOCID_MAIN].find(PlayerGuid);
	if( obj_iter != m_pObjectContainer[WOCID_MAIN].end() )
	{
		PgWorld::ObjectContainer::mapped_type& pObject = obj_iter->second;
		return pObject;
	}
	return NULL;
}

void PgAlwaysMiniMap::CustomizeRenderFrame(NiRenderer* pkRenderer, NiPoint2 const& kScreenSize, POINT2 const& ptWndPos)
{
	if(!g_pkWorld)
	{
		return;
	}
	PROFILE_BEGIN(CustomizeRenderFrame1);
	if( !g_pkWorld->IsHaveAttr(GATTR_FLAG_HOMETOWN) )
	{
		if( g_pkWorld->IsHaveAttr(GKIND_RACE) )
		{// 달리기 이벤트 맵에서는 미니맵 드래그가 가능하기때문에 항상 갱신한다.
			m_bRefreshAllMiniMapIconInfo = true;
		}
		if (m_pTriggerContainer != NULL && GetViewFlag(EIT_TRIGGER) && m_bRefreshAllMiniMapIconInfo)
		{//Trigger를 출력한다.
			int iEnterDunNum = 0;
			PgWorld::TriggerContainer::const_iterator trigger_iter = m_pTriggerContainer->begin();
			while( trigger_iter != m_pTriggerContainer->end() )
			{
				PgWorld::TriggerContainer::mapped_type const& pkTrigger = trigger_iter->second;
				if( pkTrigger && !pkTrigger->MinimapHide() )
				{
					switch( pkTrigger->GetTriggerType() )
					{
					case PgTrigger::TRIGGER_TYPE_TELEPORT:
						{
							NiAVObject* pkTriggerObject = pkTrigger->GetTriggerObject();
							if( !pkTriggerObject )
							{//뭔가 오륜데.
								break;
							}
							PROFILE_BEGIN(CustomizeRenderFrameTeleport);

							if( pkTrigger->GetScript().compare("Map_EnterDungeon") == 0 )
							{//엔터 던전만 찍는다.
								++iEnterDunNum;
								bool bInsertPairTriggerIcon = true;
								MiniMapIconCont::const_iterator iter = m_kMiniMapIconCont.find(pkTrigger->GetGuid());
								if (iter != m_kMiniMapIconCont.end())
								{
									bInsertPairTriggerIcon = false;
								}

								PgMiniMapBase::AddMiniMapTriggerIcon(pkTrigger, ptWndPos);
								AddDrawText(pkTriggerObject->GetWorldTranslate(), iEnterDunNum, ptWndPos, ICONTYPE_TELEPORT);

								if (bInsertPairTriggerIcon)
								{
									NiAVObject *pkNode = g_pkWorld->GetSceneRoot()->GetObjectByName(MB(pkTrigger->ParamString()));
									if( pkNode )
									{
										BM::GUID kNewGuid = BM::GUID::Create();
										PgMiniMapBase::AddMiniMapIcon(kNewGuid, pkNode->GetTranslate(), ICONTYPE_TELEPORT, ptWndPos);
										AddDrawText(pkNode->GetTranslate(), iEnterDunNum + PAIR_TELEPORT_ZONE_ADDPARAM, ptWndPos, ICONTYPE_TELEPORT);

										MiniMapIconCont::iterator link1 = m_kMiniMapIconCont.find(pkTrigger->GetGuid());
										MiniMapIconCont::iterator link2 = m_kMiniMapIconCont.find(kNewGuid);

										if (link1 != m_kMiniMapIconCont.end() && link2 != m_kMiniMapIconCont.end())
										{
											link1->second.kLinkedObjectGuid = kNewGuid;
											link2->second.kLinkedObjectGuid = pkTrigger->GetGuid();
										}
									}
								}
								else
								{
									if (iter->second.kLinkedObjectGuid != BM::GUID::NullData())
									{
										MiniMapIconCont::iterator linked = m_kMiniMapIconCont.find(iter->second.kLinkedObjectGuid);
										if (linked != m_kMiniMapIconCont.end())
										{				
											PgMiniMapBase::AddMiniMapIcon(linked->second.kObjectGuid, linked->second.ptLastWorldPos, ICONTYPE_TELEPORT, ptWndPos);
											AddDrawText(linked->second.ptLastWorldPos, iEnterDunNum + PAIR_TELEPORT_ZONE_ADDPARAM, ptWndPos, ICONTYPE_TELEPORT);
										}
									}
								}
							}
							PROFILE_END();
						}break;
					case PgTrigger::TRIGGER_TYPE_LOCATION:
						{
							PROFILE_BEGIN(CustomizeRenderFrameLocation);
							if (CheckQuestNotifyMarkingTrigger(pkTrigger))
							{
								PgMiniMapBase::AddMiniMapTriggerIcon(pkTrigger, ptWndPos);
							}
							PROFILE_END();
						}
						break;
					case PgTrigger::TRIGGER_TYPE_KING_OF_HILL:
						{
							PROFILE_BEGIN(CustomizeRenderFrameKingOfHill);

							PgMiniMapBase::AddMiniMapTriggerIcon(pkTrigger, ptWndPos);

							PROFILE_END();
						}break;
					case PgTrigger::TRIGGER_TYPE_LOVE_FENCE:
						{
							PROFILE_BEGIN(CustomizeRenderFrameLoveFence);

							PgMiniMapBase::AddMiniMapTriggerIcon(pkTrigger, ptWndPos);

							PROFILE_END();
						}break;
					case PgTrigger::TRIGGER_TYPE_PORTAL:
					case PgTrigger::TRIGGER_TYPE_PARTYPORTAL:
					case PgTrigger::TRIGGER_TYPE_PARTY_MEMBER_PORTAL:
					case PgTrigger::TRIGGER_TYPE_RAG_PARTY_MEMBER_PORTAL:
					case PgTrigger::TRIGGER_TYPE_CHANNELPORTAL:
						{
							PROFILE_BEGIN(CustomizeRenderFramePortal);
							int const iTextNo = pkTrigger->GetTriggerTitleTextID();
							NiAVObject* pkTriggerObject = pkTrigger->GetTriggerObject();
							if( iTextNo && pkTriggerObject )
							{
								AddDrawText(pkTriggerObject->GetWorldTranslate(), iTextNo, ptWndPos, ICONTYPE_PORTAL);
							}
							PROFILE_END();
						}//break 원래 없음.
					default:
						{
							PROFILE_BEGIN(CustomizeRenderFrameDefault);
							PgMiniMapBase::AddMiniMapTriggerIcon(pkTrigger, ptWndPos);
							PROFILE_END();
						}break;
					}
				}
				++trigger_iter;
			}
		}
	}
	PROFILE_END();

	PROFILE_BEGIN(CustomizeRenderFrame2);
	if (m_pObjectContainer != NULL)
	{	//이쪽은 일반 오브젝트만 들어 온다.(지금은 파괴 오브젝트만 들어있음)
		PROFILE_BEGIN(CustomizeRenderFrameBreakObj);
		if (GetViewFlag(EIT_BREAKOBJ) && g_pkWorld->IsHaveAttr(GATTR_EMPORIABATTLE))
		{
			PgWorld::ObjectContainer::const_iterator obj_iter = m_pObjectContainer[WOCID_BEFORE].begin();
			while( obj_iter != m_pObjectContainer[WOCID_BEFORE].end() )
			{
				PgWorld::ObjectContainer::mapped_type const& pkObject = obj_iter->second;
				if( pkObject )
				{
					PgPilot* pkPilot = pkObject->GetPilot();
					PgMiniMapBase::AddMiniMapBrokenObjectIcon(pkPilot, ptWndPos);
				}
				++obj_iter;
			}
		}
		PROFILE_END();


		PROFILE_BEGIN(CustomizeRenderFrameNPC);
		//이쪽에 플레이어, NPC, 몬스터 정보가 다있다.
		MiniMapQuestSortCont kSortQuestCont;
		PgWorld::ObjectContainer::const_iterator obj_iter = m_pObjectContainer[WOCID_MAIN].begin();
		while( obj_iter != m_pObjectContainer[WOCID_MAIN].end() )
		{
			PgWorld::ObjectContainer::mapped_type const& pkObject = obj_iter->second;
			++obj_iter;
			if( pkObject )
			{
				PgPilot* pkPilot = pkObject->GetPilot();
				if( pkPilot && pkPilot->GetWorldObject() )
				{
					bool bIsSelectedNpc = false;
					switch( pkObject->GetObjectID() )
					{
					case PgIXmlObject::ID_MONSTER:
						{
							if( !GetViewFlag(EIT_MONSTER) )
							{
								continue;
							}
						}break;
					case PgIXmlObject::ID_NPC:
						{ 
							if( !GetViewFlag(EIT_NPC) )
							{ 
								continue; 
							} 

							PgActorNpc* pkNpc = dynamic_cast<PgActorNpc*>(pkPilot->GetWorldObject());
							if( pkNpc && !m_kSelectObject.kIconID.empty() )
							{
								bIsSelectedNpc = (m_kSelectObject.kIconID == UNI(pkNpc->GetID()));
							}
						}break;
					case PgIXmlObject::ID_PC:
						{//몬스터와 NPC가 아니면
							PgHouse* pkHouse = dynamic_cast<PgHouse*>(pkPilot->GetWorldObject());
							if( pkHouse )
							{//홈을 출력한다.
								PgMyHome* pkHome = dynamic_cast<PgMyHome*>(pkPilot->GetUnit());
								if( pkHome )
								{
									PgMiniMapBase::AddMiniMapMyHomeIcon(pkHouse, pkHome, ptWndPos);
								}
							}
							PgDropBox* pBox = dynamic_cast<PgDropBox*>(pkPilot->GetWorldObject());
							{//드롭박스를 출력한다.
								if( pBox )
								{
									PgMiniMapBase::AddMiniMapDropBoxItemIcon(pBox, ptWndPos);
								}
							}
						}//일부러 Break 하지 않는다.
					default://다른 유저들은 그리지 않는다.
						continue;
					}

					PgActor* pkActor = dynamic_cast<PgActor*>(pkPilot->GetWorldObject());
					if( pkActor )
					{//액터다.
						if( !pkActor->IsHide() )
						{//퀘스트가 있나 검사
							if( !GetViewFlag(EIT_QUEST) || !CheckQuestNotifyMarkingUnit(pkActor, kSortQuestCont) )
							{//없다. 그냥 그려라
								PgMiniMapBase::AddMiniMapEffectIcon(pkActor, ptWndPos);
							}
						}
					}

					if( bIsSelectedNpc )
					{
						AddMiniMapSelectObjectIconArrow(pkActor, ptWndPos);
					}
				}
			}
		}

		if (m_kSelectObject.Type == 0)
		{
			MiniMapIconCont::iterator iter = m_kMiniMapIconCont.find(m_kSelectArrowGUID);
			if (m_kMiniMapIconCont.end() != iter)
			{
				iter->second.pIconTexture->SetRectangle(iter->second.iIconPolygonIndex, 0, 0, 0, 0);
			}		
		}
		PROFILE_END();

		PROFILE_BEGIN(CustomizeRenderFrameQuest);
		//퀘스트 그리자.
		MiniMapQuestSortCont::const_iterator qsticon_iter = kSortQuestCont.begin();
		while( kSortQuestCont.end() != qsticon_iter )
		{
			MiniMapQuestSortCont::mapped_type const& kQuestInfo = qsticon_iter->second;

			if( kQuestInfo.pkActor )
			{
				SCustomQuestMarker kMarkerInfo;
				bool bUseQuestMarker = false;
				if( g_kQuestMan.CheckCustomQuestMarker(kQuestInfo.iQuestID, kMarkerInfo) )
				{
					IconType kType = ICONTYPE_NONE;
					switch( kQuestInfo.eState )
					{
					case QS_Begin:		
					case QS_Begin_Loop:	
					case QS_Begin_Tactics:
					case QS_Begin_Weekly:
					case QS_Begin_Story:
					case QS_Begin_Couple:
					case QS_Begin_Event:
						{
							bUseQuestMarker = true;
							kType = static_cast<IconType>(kMarkerInfo.iBeginIconIDX); 
						}break;
					case QS_End_Story:	
					case QS_End_Loop:	
					case QS_End_Tactics:
					case QS_End_Weekly:	
					case QS_End_Couple:	
					case QS_End_Event:	
						{ 
							bUseQuestMarker = true;
							kType = static_cast<IconType>(kMarkerInfo.iEndIconIDX);	
						}break;
					case QS_Ing:
						{
							bUseQuestMarker = true;
							kType = static_cast<IconType>(kMarkerInfo.iIngIconIDX);	
						}break;
					case QS_Begin_NYet:
						{
							bUseQuestMarker = true;
							kType = static_cast<IconType>(kMarkerInfo.iNotYetIconIDX);	
						}break;
					}

					if( bUseQuestMarker )
					{
						PgMiniMapBase::AddMiniMapIcon(kQuestInfo.pkActor->GetGuid(), kQuestInfo.pkActor->GetPos(), kType, ptWndPos);
					}
				}

				if( false == bUseQuestMarker )
				{
					PgMiniMapBase::AddMiniMapQuestIcon(kQuestInfo.pkActor->GetGuid(), kQuestInfo.pkActor->GetPos(), kQuestInfo.eState, ptWndPos);
				}
			}
			++qsticon_iter;
		}
		PROFILE_END();

		PROFILE_BEGIN(CustomizeRenderFrameGandP);
		if( GetViewFlag(EIT_GUILD) )
		{//길드 멤버는 여기서 그려라

		}

		if( GetViewFlag(EIT_PARTY) && g_kParty.PartyGuid().IsNotNull() && !g_kNowGroundKey.IsEmpty() )
		{//파티는 여기서 그린다.
			int const iGroundNo = g_kNowGroundKey.GroundNo();

			ContPartyMember kPartyMemberList;
			g_kParty.GetPartyMemberList(kPartyMemberList);
			for(ContPartyMember::const_iterator party_iter=kPartyMemberList.begin(); party_iter!=kPartyMemberList.end(); ++party_iter)
			{
				ContPartyMember::value_type const& kPartyInfo = (*party_iter);

				if( g_kPilotMan.IsMyPlayer( kPartyInfo->kCharGuid ) )
				{//나는 재낀다
					continue;
				}

				PgPilot* pkPilot = g_kPilotMan.FindPilot(kPartyInfo->kCharGuid);
				if( pkPilot )
				{//내 주변에 있는 파티
					PgActor* pkActor = dynamic_cast<PgActor*>(pkPilot->GetWorldObject());
					if( pkActor )
					{
						PgMiniMapBase::AddMiniMapIcon(pkActor->GetGuid(), pkActor->GetPos(), ICONTYPE_PARTY, ptWndPos);
					}
				}
				else
				{//저 멀리 있는 파티
					if( iGroundNo == kPartyInfo->GroundNo() )
					{
						NiPoint3 WorldPt = NiPoint3(kPartyInfo->ptPos.x, kPartyInfo->ptPos.y, kPartyInfo->ptPos.z);
						PgMiniMapBase::AddMiniMapIcon(kPartyInfo->kCharGuid, WorldPt, ICONTYPE_PARTY, ptWndPos);
					}
				}
			}
		}
		PROFILE_END();

		PROFILE_BEGIN(CustomizeRenderFrameCouple);
		if( g_pkWorld )
		{
			if( false == g_pkWorld->IsHaveAttr(GATTR_PVP) )
			{
				if( GetViewFlag(EIT_COUPLE) && g_kCoupleMgr.Have() )
				{//커플은 여기서
					PgPilot* pkPilot = g_kPilotMan.FindPilot(g_kCoupleMgr.GetMyInfo().CoupleGuid());
					if( pkPilot )
					{
						PgActor* pkActor = dynamic_cast<PgActor*>(pkPilot->GetWorldObject());
						if( pkActor )
						{
							PgMiniMapBase::AddMiniMapIcon(pkActor->GetGuid(), pkActor->GetPos(), ICONTYPE_COUPLE, ptWndPos);
						}
					}
				}
			}
		}
		PROFILE_END();

		PROFILE_BEGIN(CustomizeRenderFrameBS);
		if( g_pkWorld
		&&	GetViewFlag(EIT_BATTLESQUARE)
		&&	g_pkWorld->IsHaveAttr(GATTR_FLAG_BATTLESQUARE) )
		{
			CONT_BS_ITEM_UNIT_POS const& rkContItemPos = g_kBattleSquareMng.ContItemUnitPos();
			CONT_BS_ITEM_UNIT_POS::const_iterator bs_iter = rkContItemPos.begin();
			while( rkContItemPos.end() != bs_iter )
			{
				CONT_BS_ITEM_UNIT_POS::value_type const& rkBSItemUnitPos = (*bs_iter);
				IconType const eIconType = static_cast< IconType >(rkBSItemUnitPos.iIconType);
				NiPoint3 kPos = rkBSItemUnitPos.kPos;
				PgActor* pkActor = g_kPilotMan.FindActor( rkBSItemUnitPos.kGuid );
				if( pkActor )
				{
					kPos = pkActor->GetPos();
				}
				PgMiniMapBase::AddMiniMapIcon(rkBSItemUnitPos.kGuid, kPos, eIconType, ptWndPos);
				++bs_iter;
			}

			CONT_BS_UNIT_POS const& rkContUnitPos = g_kBattleSquareMng.ContUnitPos();
			CONT_BS_UNIT_POS::const_iterator pos_iter = rkContUnitPos.begin();
			m_kMapRenderTextCont.clear();
			while( rkContUnitPos.end() != pos_iter )
			{
				CONT_BS_UNIT_POS::key_type const& rkGuid = (*pos_iter).first;
				CONT_BS_UNIT_POS::mapped_type const& rkUnitPos = (*pos_iter).second;
				if( false == g_kPilotMan.IsMyPlayer(rkGuid) )
				{
					IconType eIconType = ICONTYPE_NONE;
					NiPoint3 kPos = rkUnitPos.kPos;
					PgActor* pkActor = g_kPilotMan.FindActor(rkGuid);
					if( pkActor )
					{
						kPos = pkActor->GetPos();
					}
					switch( rkUnitPos.iTeamNo )
					{
					case TEAM_RED:		
						{ 
							if( 0 != rkUnitPos.sIconCount)
							{
								eIconType = ICONTYPE_TEAM_RED_MEDAL;
							}
							else
							{
								eIconType = ICONTYPE_TEAM_RED; 
							}
						}break;
					case TEAM_BLUE:		
						{
							if( 0 != rkUnitPos.sIconCount)
							{
								eIconType = ICONTYPE_TEAM_BLUE_MEDAL;
							}
							else
							{
								eIconType = ICONTYPE_TEAM_BLUE; 
							}
						}break;
					default:
						{
							//
						}break;
					}
					if( eIconType != ICONTYPE_NONE )
					{
						PgMiniMapBase::AddMiniMapIcon(rkGuid, kPos, eIconType, ptWndPos);
						if( 1 < rkUnitPos.sIconCount )
						{
							POINT2 kWndPos = ptWndPos;
							kWndPos.y -= 10;
							AddDrawText(kPos, rkUnitPos.sIconCount, kWndPos, ICONTYPE_BATTLESQUARE_ITEM);
						}
					}
				}
				++pos_iter;
			}

			CONT_BS_MEDAL_POS const& rkContMedalPos = g_kBattleSquareMng.ContMedalPos();
			CONT_BS_MEDAL_POS::const_iterator Medal_iter = rkContMedalPos.begin();
			while( rkContMedalPos.end() != Medal_iter )
			{
				CONT_BS_MEDAL_POS::key_type const& rkGuid = (*Medal_iter).first;
				CONT_BS_MEDAL_POS::mapped_type const& rkUnitPos = (*Medal_iter).second;
				IconType const eIconType = static_cast< IconType >( IconType::ICONTYPE_BATTLESQUARE_ITEM );

				NiPoint3 kPos( rkUnitPos.kPos.x, rkUnitPos.kPos.y, rkUnitPos.kPos.z);

				PgActor* pkActor = g_kPilotMan.FindActor( rkGuid );
				if( pkActor )
				{
					kPos = pkActor->GetPos();
				}
				PgMiniMapBase::AddMiniMapIcon(rkGuid, kPos, eIconType, ptWndPos);

				++Medal_iter;
			}
		}
		PROFILE_END();

		PROFILE_BEGIN(CustomizeRenderFrameDefenceMode7);
		if( g_pkWorld
		&&	g_pkWorld->IsHaveAttr(GATTR_FLAG_MISSION)
        &&  (g_pkWorld->GetDifficultyLevel()>7) )
		{//전략디펜스모드인경우
            //가디언 표시
            if( GetViewFlag(EIT_GUARDIAN) )
            {
                CONT_MISSION_UNIT_POS const rkContGuardianIcon = g_kMissionMgr.GetMinimapGuardianIcon();
			    CONT_MISSION_UNIT_POS::const_iterator guardian_it = rkContGuardianIcon.begin();
			    while( rkContGuardianIcon.end() != guardian_it )
			    {
				    CONT_MISSION_UNIT_POS::value_type const& rkUnit = (*guardian_it);
					IconType eIconType = ICONTYPE_NONE;
					switch( rkUnit.iTeam )
					{
					case TEAM_RED:	{eIconType = ICONTYPE_TEAM_GUARDIAN_RED;	}break;
					case TEAM_BLUE:	{eIconType = ICONTYPE_TEAM_GUARDIAN_BLUE;	}break;
					}
					PgMiniMapBase::AddMiniMapIcon(rkUnit.kGuid, rkUnit.kPos, eIconType, ptWndPos);
				    ++guardian_it;
			    }
            }
		}
		PROFILE_END();

		PROFILE_BEGIN(CustomizeRenderFramePvP);
		if( g_pkWorld
			&& false == g_pkWorld->IsHaveAttr(GATTR_FLAG_EMPORIABATTLE)
			&& g_pkWorld->IsHaveAttr(GATTR_FLAG_PVP) )
		{//EMPORIABATTLE 이 아닌 PVP 지역일 때만
			UNIT_PTR_ARRAY kUnitArray;
			if( g_kPilotMan.FindUnit(UT_PLAYER, kUnitArray) )
			{
				UNIT_PTR_ARRAY::const_iterator iter = kUnitArray.begin();
				while(iter != kUnitArray.end())
				{
					PgActor* pkActor = g_kPilotMan.FindActor(iter->pkUnit->GetID());
					if( pkActor )
					{//액터를 출력한다.
						if( !pkActor->IsHide() )
						{
							PgMiniMapBase::AddMiniMapEffectIcon(pkActor, ptWndPos);
						}
					}
					++iter;
				}
			}
		}
		PROFILE_END();

		PROFILE_BEGIN(CustomizeRenderFrameRaceMode);
		if( g_pkWorld
			&& g_pkWorld->IsHaveAttr(GKIND_RACE) )
		{//달리기 모드일 때
			UNIT_PTR_ARRAY kUnitArray;
			if( g_kPilotMan.FindUnit(UT_PLAYER, kUnitArray) )
			{
				UNIT_PTR_ARRAY::const_iterator iter = kUnitArray.begin();
				while(iter != kUnitArray.end())
				{
					PgActor* pkActor = g_kPilotMan.FindActor(iter->pkUnit->GetID());
					if( pkActor )
					{//액터를 출력한다.
						if( !pkActor->IsHide() && !pkActor->IsMyActor() )
						{
							PgMiniMapBase::AddMiniMapIcon(pkActor->GetGuid(), pkActor->GetPos(), ICONTYPE_TEAM_BLUE, ptWndPos);
						}
					}
					++iter;
				}
			}
		}
		PROFILE_END();

		PROFILE_BEGIN(CustomizeRenderFrameMe);
		//! 나 자신 위치 표시
		PgActor* pkActor = g_kPilotMan.GetPlayerActor();
		if( pkActor && GetViewFlag(EIT_PLAYER) )
		{
			PgMiniMapBase::AddMiniMapIcon(pkActor->GetGuid(), pkActor->GetPos(), ICONTYPE_ME, ptWndPos);
		}
		PROFILE_END();
	}
	PROFILE_END();

	PROFILE_BEGIN(CustomizeRenderFrame3);
	float ScreenHalfWidth = (1.0f - m_fZoomFactor) * 0.5f;
	float ScreenHalfHeight = (1.0f - m_fZoomFactor) * 0.5f;
	float Gab = (1.0f - m_fScreenImageRatio) * 0.5f;

	NiPoint2 kRect( ptWndPos.x / kScreenSize.x, ptWndPos.y / kScreenSize.y );
	NiPoint2 kSize( m_kWndSize.x / kScreenSize.x, m_kWndSize.y / kScreenSize.y );
	NiPoint2 kTexUpPt( m_kScreenCenter.x - ScreenHalfWidth, (m_kScreenCenter.y - ScreenHalfHeight) * m_fScreenImageRatio + Gab );
	NiPoint2 kTexDnPt( m_kScreenCenter.x + ScreenHalfWidth, (m_kScreenCenter.y + ScreenHalfHeight) * m_fScreenImageRatio + Gab );
	if( FixedSize() )
	{

		kSize.y = m_kDrawHeight.y / kScreenSize.y;
		kTexUpPt.y = m_kDrawHeight.x / MiniMapSize().y;
		kTexDnPt.y = (m_kDrawHeight.y + m_kDrawHeight.x) / MiniMapSize().y;
	}

	m_spMiniMapScreenTexture->SetRectangle(0, kRect.x, kRect.y, kSize.x, kSize.y);
	m_spMiniMapScreenTexture->UpdateBound();
	m_spMiniMapScreenTexture->SetTextures(0, 0, kTexUpPt.x, kTexUpPt.y, kTexDnPt.x, kTexDnPt.y);
	PROFILE_END();
}

void PgAlwaysMiniMap::RenderFrameUI(XUI::CXUI_Wnd * pkWnd)
{	
	if(NULL==pkWnd || false==m_bRefreshAllMiniMapIconInfo)
	{
		return;
	}

	MiniMapIconUICont::const_iterator iter = m_kContTriggerIconUI.begin();
	while(iter!=m_kContTriggerIconUI.end())
	{
		MiniMapIconUICont::mapped_type const& rkTrigger = (*iter).second;
		
		BM::vstring vStr(L"FRN_NEW_TRIGGER_ROADSIGN_");
		vStr += rkTrigger.iIdx;

		XUI::CXUI_Wnd * pkRoadSingWnd = pkWnd->GetControl(vStr);
		if( pkRoadSingWnd )
		{
			NiPoint3 ScreenPosition;
			if( ViewPtToScreenPt(rkTrigger.ViewPosition, ScreenPosition) )
			{
				POINT2 const kOldDrawPosPt = pkRoadSingWnd->Location();
				POINT2 kDrawPosPt = IconDrawPosition(POINT2::NullData(), ScreenPosition);//스크린 절대좌표를 구함
				kDrawPosPt.x += rkTrigger.iTransX;
				kDrawPosPt.y += rkTrigger.iTransY;

				pkRoadSingWnd->Visible(true);
				pkRoadSingWnd->Location(kDrawPosPt);
			}
			else
			{
				pkRoadSingWnd->Visible(false);
			}
		}
		++iter;
	}
}

void PgAlwaysMiniMap::SetTriggerIconUI(XUI::CXUI_Wnd * pkWnd, POINT2 const& ptWndPos)
{
	if( !pkWnd )
	{
		return;
	}

	MiniMapIconUICont::const_iterator iter = m_kContTriggerIconUI.begin();
	while(iter!=m_kContTriggerIconUI.end())
	{
		MiniMapIconUICont::mapped_type const& rkTrigger = (*iter).second;

		if(PgTrigger::TRIGGER_TYPE_ROADSIGN==rkTrigger.iType)
		{
			NiPoint3 ScreenPosition;
			if( ViewPtToScreenPt(rkTrigger.ViewPosition, ScreenPosition) )
			{
				BM::vstring vStr(L"FRN_NEW_TRIGGER_ROADSIGN_");
				vStr += rkTrigger.iIdx;

				POINT2 kDrawPosPt = IconDrawPosition(POINT2::NullData(), ScreenPosition);//스크린 절대좌표를 구함
				kDrawPosPt.x += rkTrigger.iTransX;
				kDrawPosPt.y += rkTrigger.iTransY;
				lua_tinker::call<void,lwUIWnd,int,int,int,int,int,int,int,char const*,int>("AddChild_HelpBalloon",
					lwUIWnd(pkWnd), kDrawPosPt.x, kDrawPosPt.y, rkTrigger.W, rkTrigger.H,
					rkTrigger.iTargetIdx, rkTrigger.iUVIndex, rkTrigger.iTTNo, MB(vStr), 0);
			}
		}
		++iter;
	}
}

POINT2 const PgAlwaysMiniMap::IconDrawPosition(POINT2 const& ptWndPos, NiPoint3 const& ScreenPos) const
{
	PROFILE_FUNC();
	POINT2	kDrawPos;

	kDrawPos.x = (LONG)(ptWndPos.x + m_kWndSize.x * ScreenPos.x);
	kDrawPos.y = (LONG)(ptWndPos.y + m_kWndSize.y * ScreenPos.y);
	if( FixedSize() )
	{
		kDrawPos.y = (LONG)(ptWndPos.y + (m_kWndSize.x * m_fScreenImageRatio * ScreenPos.y) - m_kDrawHeight.x + m_iDrawGap);
	}
	return kDrawPos;
}

bool PgAlwaysMiniMap::AddMiniMapSelectObjectIconArrow(NiAVObject* pkObject, POINT2 const& ptWndPos)
{
	PROFILE_FUNC();
	if( !pkObject ){ return false; }

	PgIWorldObject* pkWorldObject = dynamic_cast<PgIWorldObject*>(pkObject);
	if( !pkWorldObject ){ return false; }

	IconType iconType = ICONTYPE_NONE;
	NiPoint3 kObjectPos(0,0,0);
	switch( pkWorldObject->GetObjectID() )
	{
	case PgIXmlObject::ID_NPC:
		{
			PgActorNpc* pkNpc = dynamic_cast<PgActorNpc*>(pkWorldObject);
			if( pkNpc )
			{
				kObjectPos = pkNpc->GetPos();
				iconType = ICONTYPE_NPC;
			}
		}break;
	default:
		return false;
	}

	return PgMiniMapBase::AddMiniMapIcon(m_kSelectArrowGUID, kObjectPos, ICONTYPE_SELECT_OBJECT, ptWndPos);
}

bool PgAlwaysMiniMap::CheckQuestNotifyMarkingUnit(PgActor* pkActor, MiniMapQuestSortCont& kSortQuestCont)
{
	PROFILE_FUNC();
	if( !pkActor ){ return false; }

	MiniMapQuestCont::const_iterator quest_iter = m_kQuestInfoCont.find(pkActor->GetGuid());
	if( quest_iter == m_kQuestInfoCont.end() )
	{
		return false;
	}

	MiniMapQuestCont::mapped_type const& kState = quest_iter->second;

	if( QS_None == kState.eState )
	{
		return false;
	}

	SQuestMiniMapKey kKey(pkActor->GetGuid(), GetQuestStateToDrawOrder(kState.eState));
	SQuestMiniMapInfo kInfo;
	kInfo.iQuestID = kState.iQuestID;
	kInfo.eState = kState.eState;
	kInfo.pkActor = pkActor;
	std::pair<MiniMapQuestSortCont::iterator, bool> Result = kSortQuestCont.insert(std::make_pair( kKey, kInfo ));
	return Result.second;
}

bool PgAlwaysMiniMap::CheckQuestNotifyMarkingTrigger(PgTrigger* pkTrigger)
{
	PROFILE_FUNC();
	if( !pkTrigger )
	{
		return false;
	}

	switch( pkTrigger->GetTriggerType() )
	{
	case PgTrigger::TRIGGER_TYPE_LOCATION:
		{
			int iTriggerNo = pkTrigger->Param();
			int iQuestID = pkTrigger->Param2();

			PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
			if( !pkPlayer ){ return false; }

			PgMyQuest const* pkMyQuest = pkPlayer->GetMyQuest();
			if( !pkMyQuest ){ return false; }

			bool const bIngRet = pkMyQuest->IsIngQuest(iQuestID);
			if( !bIngRet ){ return false; }

			PgQuestInfo const* pkQuestInfo = g_kQuestMan.GetQuest(iQuestID);
			if( !pkQuestInfo ){ return false; }

			ContQuestLocation const& rkDependLocation = pkQuestInfo->m_kDepend_Location;
			ContQuestLocation::const_iterator location_iter = rkDependLocation.begin();
			while( rkDependLocation.end() != location_iter )
			{
				ContQuestLocation::value_type const rkElement = (*location_iter);

				SUserQuestState const* pkState = pkMyQuest->Get(iQuestID);
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
		}break;
	}
	return true;
}

NiAVObject* PgAlwaysMiniMap::GetPickObject(SMINIMAP_ICON_INFO const& kObject)
{
	PROFILE_FUNC();
	switch( kObject.Type )
	{
	case EIT_NPC:
		{
			return OnOverPickActor(kObject.kIconID);
		}break;
	case EIT_TRIGGER:
		{
			return OnOverPickTrigger(kObject.kIconID);
		}break;
	}
	return NULL;
}

NiAVObject* PgAlwaysMiniMap::OnOverPickActor(std::wstring const& kActorID) const
{
	PROFILE_FUNC();
	if (m_pObjectContainer == NULL)
	{
		return NULL;
	}

	PgWorld::ObjectContainer::iterator obj_itor = m_pObjectContainer[WOCID_MAIN].begin();
	while(obj_itor != m_pObjectContainer[WOCID_MAIN].end())
	{
		PgWorld::ObjectContainer::mapped_type pkObject = obj_itor->second;
		if( pkObject )
		{ // HIDE_MINIMAP 속성을 가진 NPC는 미니맵 하단에 표시되는 NPC 리스트에서 마우스오버로 인한 툴팁이 보이지 않음.
			PgPilot * pPilot = pkObject->GetPilot();
			if( pPilot )
			{
				PgNpc * pNpc = dynamic_cast<PgNpc*>(pPilot->GetUnit());
				if( pNpc && pNpc->HideMiniMap() )
				{
					++obj_itor;
					continue;
				}
			}
		}
		if( pkObject && pkObject->GetPilot() && pkObject->GetPilot()->GetWorldObject() )
		{
			if( UNI(pkObject->GetID()) == kActorID )
			{
				return pkObject->GetPilot()->GetWorldObject();
			}
		}
		++obj_itor;
	}
	return NULL;
}

NiAVObject* PgAlwaysMiniMap::OnOverPickTrigger(std::wstring const& kTriggerID) const
{
	PROFILE_FUNC();
	if( m_pTriggerContainer == NULL )
	{
		return NULL;
	}

	PgWorld::TriggerContainer::iterator trigger_iter = m_pTriggerContainer->begin();
	while(trigger_iter != m_pTriggerContainer->end())
	{
		PgWorld::TriggerContainer::mapped_type pkTrigger = trigger_iter->second;
#ifndef EXTERNAL_RELEASE
		if (pkTrigger && pkTrigger->GetTriggerObject())
#else
		if (pkTrigger && pkTrigger->GetTriggerObject() && GetTriggerType(pkTrigger) != ICONTYPE_NONE)
#endif
		{
			if( UNI(pkTrigger->GetID()) == kTriggerID )
			{
				return pkTrigger->GetTriggerObject();
			}
		}
		++trigger_iter;
	}
	return NULL;
}

PgActor* PgAlwaysMiniMap::OnOverPickActor(const POINT2 &ptWndPos, const POINT2 &pt, EMapIconTexType TexType) const
{
	PROFILE_FUNC();
	PgActor* pkRetObject = 0;

	if (m_pObjectContainer == NULL)
	{
		return NULL;
	}

	PgWorld::ObjectContainer::iterator obj_itor = m_pObjectContainer[WOCID_MAIN].begin();
	while(obj_itor != m_pObjectContainer[WOCID_MAIN].end())
	{
		PgWorld::ObjectContainer::mapped_type pkObject = obj_itor->second;
		if( pkObject )
		{ // HIDE_MINIMAP 속성을 가진 NPC는 미니맵에서 마우스오버로 인한 툴팁이 보이지 않음.
			PgPilot * pPilot = pkObject->GetPilot();
			if( pPilot )
			{
				PgNpc * pNpc = dynamic_cast<PgNpc*>(pPilot->GetUnit());
				if( pNpc && pNpc->HideMiniMap() )
				{
					++obj_itor;
					continue;
				}

				if ( pPilot->GetUnit() && UT_SUB_PLAYER==pPilot->GetUnit()->UnitType())
				{
					++obj_itor;
					continue;
				}
			}
		}
		if( pkObject && pkObject->GetPilot() && pkObject->GetPilot()->GetWorldObject() )
		{
			PgIWorldObject* pkRealObject = pkObject->GetPilot()->GetWorldObject();
			if( CheckMiniMapIconPicking( pkRealObject->GetWorldTranslate(), ptWndPos, pt, GetIconSize(TexType) ) )
			{
				pkRetObject = dynamic_cast<PgActor*>(pkObject);
			}
		}
		++obj_itor;
	}

	return pkRetObject;
}

std::wstring const PgAlwaysMiniMap::OnOverPickParty(const POINT2 &ptWndPos, const POINT2 &pt, EMapIconTexType TexType) const
{
	PROFILE_FUNC();
	std::wstring kName;

	ContPartyMember kPartyMemberList;
	g_kParty.GetPartyMemberList(kPartyMemberList);
	ContPartyMember::const_iterator party_iter = kPartyMemberList.begin();
	while( party_iter != kPartyMemberList.end() )
	{
		ContPartyMember::value_type const& pkPartyInfo = (*party_iter);
		++party_iter;

		if( !pkPartyInfo )
		{
			continue;
		}

		NiPoint3 WorldPos = NiPoint3(pkPartyInfo->ptPos.x, pkPartyInfo->ptPos.y, pkPartyInfo->ptPos.z);

		if( CheckMiniMapIconPicking( WorldPos, ptWndPos, pt, GetIconSize(TexType) ) )
		{
			kName = pkPartyInfo->kName;
		}
	}
	return kName;
}

PgTrigger* PgAlwaysMiniMap::OnOverPickTrigger(const POINT2 &ptWndPos, const POINT2 &pt, EMapIconTexType TexType) const
{
	PROFILE_FUNC();
	PgTrigger* pkRetTrigger = NULL;

	if( m_pTriggerContainer == NULL )
	{
		return NULL;
	}

	PgWorld::TriggerContainer::iterator trigger_iter = m_pTriggerContainer->begin();
	while(trigger_iter != m_pTriggerContainer->end())
	{
		PgWorld::TriggerContainer::mapped_type pkTrigger = trigger_iter->second;
#ifndef EXTERNAL_RELEASE
		if (pkTrigger && pkTrigger->GetTriggerObject())
#else
		if (pkTrigger && pkTrigger->GetTriggerObject() && GetTriggerType(pkTrigger) != ICONTYPE_NONE)
#endif
		{
			NiAVObject* pkObject = pkTrigger->GetTriggerObject();
			if( CheckMiniMapIconPicking( pkObject->GetWorldTranslate(), ptWndPos, pt, GetIconSize(TexType) ) )
			{
				pkRetTrigger = pkTrigger;
			}
		}
		++trigger_iter;
	}

	return pkRetTrigger;
}

bool PgAlwaysMiniMap::CheckMiniMapIconPicking(NiPoint3 const& kWorldPos, POINT2 const& ptWndPos, POINT const& pt, POINT2 const TexSize) const
{
	PROFILE_FUNC();
	if( !m_spCamera )
	{
		return false;
	}

	NiPoint3 ViewPos;
	if( m_spCamera->WorldPtToScreenPt3(kWorldPos, ViewPos.x, ViewPos.y, ViewPos.z) == false )
	{
		return false;
	}

	NiPoint3 ScreenPos;
	if( !ViewPtToScreenPt(ViewPos, ScreenPos) )
	{
		return false;
	}

	POINT2 PixelPos = IconDrawPosition(ptWndPos, ScreenPos);
	RECT rcIconRect;
	if( ERCR_OUT == GetIconRect(rcIconRect, ptWndPos, PixelPos, TexSize) || !FixedSize() )
	{
		return false;
	}

	rcIconRect.right += rcIconRect.left;
	rcIconRect.bottom += rcIconRect.top;
	if( !(::PtInRect(&rcIconRect, pt)) )
	{
		return false;
	}

	return true;
}

bool PgAlwaysMiniMap::AddDrawText(NiPoint3 const& kWorldPt, int const iTextNo, POINT2 const& ptWndPos, IconType const eIconType)
{
	PROFILE_FUNC();
	NiPoint3 ViewPosition;
	NiPoint3 ScreenPosition;
	POINT2 kTemp;
	EMapIconTexType eIconTexType = EMITT_NONE;
	POINT2 iIconSize;
	NiScreenElements* pIconTexture = NULL;
	if( false == GetMiniMapIcon(eIconType, kTemp, eIconTexType, iIconSize, pIconTexture) )
	{
		return false;
	}

	if( m_spCamera->WorldPtToScreenPt3(kWorldPt, ViewPosition.x, ViewPosition.y, ViewPosition.z) == false )
	{
		return false;
	}

	if( ViewPtToScreenPt(ViewPosition, ScreenPosition) )
	{
		POINT2 kDrawPosPt = IconDrawPosition(ptWndPos, ScreenPosition);
		POINT2 PtLoc(kDrawPosPt.x, kDrawPosPt.y);

		std::pair<kMapTextContainer::iterator, bool> Result = m_kMapRenderTextCont.insert(std::make_pair(iTextNo, SMiniMapRenderText()));

		kMapTextContainer::iterator iter = m_kMapRenderTextCont.find(iTextNo);
		if (iter == m_kMapRenderTextCont.end())
		{
			std::pair<kMapTextContainer::iterator, bool> Result = m_kMapRenderTextCont.insert(std::make_pair(iTextNo, SMiniMapRenderText()));
			if (Result.second == false)
				return false;

			iter = Result.first;
		}

		SMiniMapRenderText& kTextInfo = iter->second;

		switch( eIconType )
		{
		case ICONTYPE_BATTLESQUARE_ITEM:
			{
				if( false == kTextInfo.IsTextExist() )
				{
					BM::vstring kStr;
					kStr = iTextNo;
					kTextInfo.SetStr(kStr, _T("Font_Text"), COLOR_YELLOW);
				}

				POINT TextSize;
				TextSize = kTextInfo.GetSize();
				PtLoc.x -= (TextSize.x * 0.5f);
				PtLoc.y -= (TextSize.y * 0.5f);

				kTextInfo.SetAttr(PtLoc, NiColorA(0.95f, 0.92f, 0.78f, 1.f), NiColorA(0.f, 0.f, 0.f, 1.f), true);
				kTextInfo.SetShow(true);
			}break;
		case ICONTYPE_PORTAL:
			{
				if( kTextInfo.IsTextExist() == false )
				{
					kTextInfo.SetStr(TTW(iTextNo), _T("Font_Text_Small"), COLOR_WHITE);
				}
				POINT TextSize;
				TextSize = kTextInfo.GetSize();

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
					PtLoc.y += iIconSize.y * 0.5f;
				}
				else
				{
					PtLoc.y -= TextSize.y;
				}
				int iGab = (PtLoc.x + TextSize.x) - (m_kWndSize.x + ptWndPos.x);
				if( 0 < iGab ) { PtLoc.x -= iGab; }
				iGab = (PtLoc.y + TextSize.y) - (m_kWndSize.y + ptWndPos.y);
				if( 0 < iGab ) { PtLoc.y -= iGab; }

				kTextInfo.SetAttr(PtLoc, NiColorA(0.95f, 0.92f, 0.78f, 1.f), NiColorA(0.f, 0.f, 0.f, 1.f), false);
				kTextInfo.SetShow(true);
			}break;
		case ICONTYPE_TELEPORT:
			{
				if( false == kTextInfo.IsTextExist() )
				{
					int const iValue = (iTextNo < PAIR_TELEPORT_ZONE_ADDPARAM)?(iTextNo):(iTextNo - PAIR_TELEPORT_ZONE_ADDPARAM);
					BM::vstring kStr;
					if( 10 > iValue )
					{
						kStr = iValue;
					}
					else
					{
						kStr = static_cast<wchar_t>(65 + iValue - 10);
					}

					kTextInfo.SetStr(kStr, _T("Font_Text_Small"), COLOR_WHITE);
				}

				POINT TextSize;
				TextSize = kTextInfo.GetSize();
				PtLoc.x -= (TextSize.x * 0.5f);
				PtLoc.y -= (TextSize.y * 0.5f);

				kTextInfo.SetAttr(PtLoc, NiColorA(0.95f, 0.92f, 0.78f, 1.f), NiColorA(0.f, 0.f, 0.f, 1.f), true);
				kTextInfo.SetShow(true);
			}break;
		}

		return true;
	}
	else
	{
		kMapTextContainer::iterator txt_itor = m_kMapRenderTextCont.find(iTextNo);
		if( txt_itor != m_kMapRenderTextCont.end() )
		{
			txt_itor->second.SetShow(false);
		}
	}
	return false;
}

void PgAlwaysMiniMap::UpdateStrongholdLink()
{
	if( NULL == g_pkWorld )
	{
		return;
	}
	XUI::CXUI_Wnd * pMinimapWnd = XUIMgr.Get(L"SFRM_BIG_MAP");
	if( !pMinimapWnd )
	{
		return;
	}

	XUI::CXUI_Wnd * pLinkWnd = dynamic_cast<XUI::CXUI_Wnd*>(pMinimapWnd->GetControl(L"FRM_MINIMAP_LINK"));
	if( !pLinkWnd )
	{
		return;
	}

	PgPvPGame *pGame = dynamic_cast<PgPvPGame*>(PgContentsBase::ms_pkContents);
	if( !pGame )
	{
		return;
	}
	
	CONT_STRONGHOLD_LINK const & ContLink = pGame->GetContLinkData();

	int ContLinkCount = -1;
	CONT_STRONGHOLD_LINK::const_iterator iter;
	for( iter = ContLink.begin(); iter != ContLink.end(); ++iter )
	{// 링크 정보를 순회하면서
		++ContLinkCount;
		PgTrigger * pLTrigger = g_pkWorld->GetTriggerByID(iter->LTrigger);
		PgTrigger * pRTrigger = g_pkWorld->GetTriggerByID(iter->RTrigger);
		if( pLTrigger && pRTrigger )
		{// 해당 트리거의 오브젝트 정보를 가져온다.
			NiAVObject* pLTriggerObject = pLTrigger->GetTriggerObject();
			NiAVObject* pRTriggerObject = pRTrigger->GetTriggerObject();
			if( pLTriggerObject && pRTriggerObject )
			{
				NiPoint3 ViewPosition;
				NiPoint3 ScreenPosition;
				POINT2 LDrawPosPt, RDrawPosPt;

				if( false == m_spCamera->WorldPtToScreenPt3(pLTriggerObject->GetWorldTranslate(), ViewPosition.x, ViewPosition.y, ViewPosition.z) )
				{
					continue;
				}
				if( ViewPtToScreenPt(ViewPosition, ScreenPosition) )
				{
					LDrawPosPt = IconDrawPosition(POINT2::NullData(), ScreenPosition);//스크린 절대좌표를 구함
					LDrawPosPt.x += 2;
				}

				if( false == m_spCamera->WorldPtToScreenPt3(pRTriggerObject->GetWorldTranslate(), ViewPosition.x, ViewPosition.y, ViewPosition.z) )
				{
					continue;
				}
				if( ViewPtToScreenPt(ViewPosition, ScreenPosition) )
				{
					RDrawPosPt = IconDrawPosition(POINT2::NullData(), ScreenPosition);//스크린 절대좌표를 구함
					RDrawPosPt.x += 2;
				}

				BM::vstring FormName("FRM_MINIMAP_LINE_");
				FormName += ContLinkCount;
				XUI::CXUI_Wnd * pItemWnd = dynamic_cast<XUI::CXUI_Wnd*>(pLinkWnd->GetControl((static_cast<std::wstring>(FormName).c_str())));
				if( pItemWnd )
				{
					if( g_bDrawMinimapLink )
					{
					}else
					{
						if( TEAM_NONE == iter->LinkTeam )
						{// 링크가 끊어지면 그리지 않는다.
							pItemWnd->Visible(false);
							continue;
						}
						else
						{// 팀에 따라 다른 색의 라인을 보이게 한다.
							std::wstring ImagePath;
							if( TEAM_RED == iter->LinkTeam )
							{
								ImagePath = L"../Data/6_ui/pvp/pvpLnR.tga";
							}
							else if( TEAM_BLUE == iter->LinkTeam )
							{
								ImagePath = L"../Data/6_ui/pvp/pvpLnB.tga";
							}
							if( pItemWnd->DefaultImgName() != ImagePath )
							{
								pItemWnd->DefaultImgName( ImagePath );
							}
							pItemWnd->Visible(true);
						}
					}

					//두 점의 거리를 구하고
					double temp = pow(double(LDrawPosPt.x - RDrawPosPt.x), 2) + pow(double(LDrawPosPt.y - RDrawPosPt.y), 2);
					int distance = static_cast<int>(sqrt(temp));
					pItemWnd->Size( POINT2( distance, pItemWnd->Size().y ) );
					pItemWnd->ImgSize( POINT2( distance, pItemWnd->Size().y ) );
					{//각도를 구하고
						pItemWnd->RotationCenter(POINT2F(0.0f, 1.0f));
						if( LDrawPosPt.x == RDrawPosPt.x )
						{//두 점이 같은 수직선상에 있으면 90도 회전
							pItemWnd->RotationDeg(90);
							if( LDrawPosPt.y < RDrawPosPt.y )
							{
								pItemWnd->Location( LDrawPosPt );
							}
							else
							{
								pItemWnd->Location( RDrawPosPt );
							}
						}
						else if( LDrawPosPt.y == RDrawPosPt.y )
						{//두 점이 같은 수직선상에 있으면 회전할 필요 없다.
							if( LDrawPosPt.x < RDrawPosPt.x )
							{
								pItemWnd->Location( LDrawPosPt );
							}
							else
							{
								pItemWnd->Location( RDrawPosPt );
							}
						}
						else if( LDrawPosPt.x < RDrawPosPt.x )
						{
							float radian = NiACos( (float)(RDrawPosPt.x - LDrawPosPt.x) / (float)distance );
							float degree = radian*(180/PI);
							if( LDrawPosPt.y > RDrawPosPt.y )
							{//L을 중심으로 R의 위치가 1사분면에 있을때
								pItemWnd->RotationDeg(-degree);
							}
							else
							{//4사분면에 잇을 때
								pItemWnd->RotationDeg(degree);
							}
							pItemWnd->Location( LDrawPosPt );
						}
						else if( RDrawPosPt.x < LDrawPosPt.x )
						{
							float radian = NiACos( (float)(LDrawPosPt.x - RDrawPosPt.x) / (float)distance );
							float degree = radian*(180/PI);
							if( RDrawPosPt.y > LDrawPosPt.y )
							{//R을 중심으로 L의 위치가 1사분면에 있을때
								pItemWnd->RotationDeg(-degree);
							}
							else
							{//4사분면에 잇을 때
								pItemWnd->RotationDeg(degree);
							}
							pItemWnd->Location( RDrawPosPt );
						}
					}
				}
			}
		}
	}
}

PgQuestManUtil::EQuestMarkDrawOrder const PgAlwaysMiniMap::GetQuestStateToDrawOrder(EQuestState const eState)
{
	switch( eState )
	{
	case QS_Begin_NYet:		{ return PgQuestManUtil::EQMDO_HIGH;			}break;
	case QS_Ing:			{ return PgQuestManUtil::EQMDO_ING;				}break;
	case QS_Begin_Loop:		{ return PgQuestManUtil::EQMDO_REPEAT_START;	}break;
	case QS_End_Loop:		{ return PgQuestManUtil::EQMDO_REPEAT_END;		}break;
	case QS_Begin:			{ return PgQuestManUtil::EQMDO_ADVENTURE_START;	}break;
	case QS_End:			{ return PgQuestManUtil::EQMDO_ADVENTURE_END;	}break;
	case QS_Begin_Tactics:	{ return PgQuestManUtil::EQMDO_ONEDAY_START;	}break;
	case QS_End_Tactics:	{ return PgQuestManUtil::EQMDO_ONEDAY_END;		}break;
	case QS_Begin_Story:	{ return PgQuestManUtil::EQMDO_HERO_START;		}break;
	case QS_End_Story:		{ return PgQuestManUtil::EQMDO_HERO_END;		}break;
	case QS_Begin_Weekly:	{ return PgQuestManUtil::EQMDO_WEEKLY_START;	}break;
	case QS_End_Weekly:		{ return PgQuestManUtil::EQMDO_WEEKLY_END;		}break;
	case QS_Begin_Couple:	{ return PgQuestManUtil::EQMDO_COUPLE_START;	}break;
	case QS_End_Couple:		{ return PgQuestManUtil::EQMDO_COUPLE_END;		}break;
	case QS_Begin_Event:	{ return PgQuestManUtil::EQMDO_REPEAT_START;	}break;
	case QS_End_Event:		{ return PgQuestManUtil::EQMDO_REPEAT_END;		}break;
	}
	return PgQuestManUtil::EQMDO_HIGH;
}