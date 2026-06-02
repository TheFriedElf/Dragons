#include "StdAfx.h"
#include "Lohengrin/packetstruct.h"
#include "Variant/Global.h"
#include "ServerLib.h"
#include "PgUIScene.h"
#include "PgRenderer.h"
#include "PgRenderMan.h"
#include "PgInput.h"
#include "lwUI.h"
#include "lwPacket.h" 
#include "mmsystem.h"
#include "NiUIManager.h"
#include "PgLocalManager.h"
#include "PgMobileSuit.h"
#include "PgPostProcessMan.h"
#include "PgIPostProcessEffect.h"
#include "PgActor.h"
#include "PgWorld.h"
#include "PgPilotMan.h"
#include "PgPilot.h"
#include "PgItemEx.h"
#include "PgNifMan.h"
#include "PgXmlLoader.h"
#include "PgQuestUI.h"
#include "PgQuestIngList.h"
#include "PgQuestDialog.h"
#include "PgQuestSelectiveList.h"
#include "PgPartyBalloon.h"
#include "PgExpeditionBalloon.h"
#include "PgTextBalloon.h"
#include "PgMarkBalloon.h"
#include "PgVendorBalloon.h"
#include "PgActor.h"
#include "PgActorNpc.h"
#include "PgUISound.h"
#include "Pg2DString.h"
#include "PgError.h"
#include "PgCameraModeFree.H"
#include "PgOption.h"
#include "PgChainAttack.h"
#include "PgScreenBreak.H"
#include <D3dx9core.h>
#include "PgEnergyGuage.H"
#include "lwUI.h"
#include "PgComboAdvisor.H"
#include "PgMissionComplete.h"
#include "lwTrade.h"
#include "PgMovieMgr.h"
#include "PgEmoticonFont.H"
#include "PgHelpSystem.H"
#include "PgFontSwitchTable.h"
#include "PgAlphaAccumulator.h"
#include "PgQuestUI.h"
#include "PgTextTypistWnd.h"
#include "PgMoveAnimateWnd.h"
#include "lwMonsterKillUImgr.h"
#include "lwMissionScoreUImgr.h"
#include "PgStitchImageMng.h"
#include "PgTextDialogMng.h"
#include "PgBalloonEmoticonMgr.h"
#include "lwUIBook.h"
#include "PgMiniMap.h"
#include "PgChatMgrClient.h"
#include "PgQuestMarkHelp.h"
#include "PgSuperGroundUI.h"
#include "PgRagnarokGroundUI.h"
#include "PgLegendItemMakeHelpMng.h"
#include "PgScreenEffect.H"
#include "PgTrigger.h"
#include "lwTrigger.h"

typedef enum EFontFound
{
	eDefaultFontFound = 1,
	eFontFound = 2,
	eNoneFound = 3,
}EFontFound;

namespace PgUISceneUtil
{
	TiXmlNode const* FindInLocal(std::string const& kConfLocalName, TiXmlNode const* pkTopNode, char const* szContents)
	{
		if( !pkTopNode )
		{
			return NULL;
		}

		TiXmlNode const* pkFindLocalNode = NULL;
		TiXmlNode const* pkDefaultLocalNode = NULL;
		TiXmlNode const* pkSubChild = pkTopNode->FirstChild();
		while( pkSubChild )
		{
			TiXmlElement const* pkSubElement = dynamic_cast<TiXmlElement const*>(pkSubChild);
			if(pkSubElement)
			{
				TiXmlAttribute const* pkAttr = pkSubElement->FirstAttribute();
				if( pkAttr )
				{
					typedef std::list< std::string > Contstr;

					char const *pcAttrValue = pkAttr->Value();
					if( pcAttrValue )
					{
						std::string const kCurLocal(pcAttrValue);

						Contstr kList;
						BM::vstring::CutTextByKey(kCurLocal, std::string("/"), kList);
#ifndef USE_INB
						if( 1 < kList.size()
							&&	kList.end() != std::find( kList.begin(), kList.end(), std::string("DEFAULT")) )
						{
							std::string const &rkErrorMsg = std::string(szContents) + std::string(": 'DEFAULT' is can't same use, other local tag in ") + kCurLocal;
							//::MessageBox(NULL, UNI(rkErrorMsg), L"Error", MB_OK);
							_PgMessageBox("Error", rkErrorMsg.c_str());
						}
#endif

						if(kConfLocalName == kCurLocal)
						{
#ifndef USE_INB
							if( pkFindLocalNode )
							{
								std::string const& kTempStr = std::string(szContents) + ": " + kConfLocalName + " local is duplicated";
								//::MessageBox(NULL, UNI(kTempStr), L"Error", MB_OK);
								_PgMessageBox("Error", kTempStr.c_str());
							}
#endif
							pkFindLocalNode = pkSubChild;
						}
						else if("DEFAULT" == kCurLocal)
						{
							pkDefaultLocalNode = pkSubChild;
						}
					}
				}
			}
			pkSubChild = pkSubChild->NextSibling();
		}

		if( pkFindLocalNode )
		{
			return pkFindLocalNode;
		}
		else
		{
			if( NULL == pkFindLocalNode
			&&	NULL != pkDefaultLocalNode )
			{
				return pkDefaultLocalNode;
			}
		}
		return NULL;
	}
};

using namespace CS;

//XUI 에서 Call UI 할때 Render 에 Insert 해주고. 닫을때 Remove 해 줄 수 있도록 바꾸는것이 중요.
PgUIScene	g_kUIScene;

extern NiPoint3 g_kSpanTrn;
extern NiPoint3 g_kSpanRot;
extern NiPoint3 g_kAdjustTrn;
extern NiPoint3 g_kAdjustTargetTrn;
extern NiQuaternion g_kSpeed;

extern NiPoint3 g_kCameraTrn;
extern NiQuaternion g_kCameraRot;

extern SGroundKey g_kNowGroundKey;
extern void TabGame( bool bDisplay );

int const NULL_ITEM_NUM = 20400217;

DWORD const MOUSE_HIDE_TIME = 15000;	//마우스 무반응 15초

#define DEFAULT_CAND_NUM_PER_PAGE 5
#define IME_BALLOON_GAP	65

PgScreenTextureRendererDataGeomGroup	g_kTempScreenTextureRendererDataGeomGroup;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//		class	PgScreenTextureRendererData
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
PgScreenTextureRendererData::PgScreenTextureRendererData()
{
	m_iTotalVerts = 0;
	m_pkVertexBuffer = NULL;
	m_lpVertexBuffer = 0;
	
	SetGeometryGroup(&g_kTempScreenTextureRendererDataGeomGroup);
}
PgScreenTextureRendererData::~PgScreenTextureRendererData()
{
	if(m_lpVertexBuffer)
	{
		m_lpVertexBuffer->Release();
	}
	m_lpVertexBuffer = 0;

	if(m_pkVertexBuffer)
		delete []m_pkVertexBuffer;
	m_pkVertexBuffer= NULL;
	m_iTotalVerts = 0;
}
bool PgScreenTextureRendererData::ContainsVertexData(NiShaderDeclaration::ShaderParameter 
	eParameter) const
{
	return	true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//		class	PgCursour
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool PgCursour::Draw(float fFrameTime)
{
#ifndef EXTERNAL_RELEASE
	PgPostProcessMan *pPostProcessMan = g_kRenderMan.GetRenderer()->m_pkPostProcessMan;
	PgIPostProcessEffect *pBloom = 0;
	PgIPostProcessEffect *pFSAA = 0;
	if(pPostProcessMan)
	{
		pBloom = pPostProcessMan->GetEffect(PgIPostProcessEffect::BRIGHT_BLOOM);
		pFSAA = pPostProcessMan->GetEffect(PgIPostProcessEffect::FSAA);
	}

	std::wstring wstr;
//	kCSIME.GetWindowText(wstr);
	BM::vstring str(wstr);

	/*
	str += _T(" Cam Interpol: ( ");
	str += (double)g_kSpeed.GetX();
	str += _T(", ");
	str += (double)g_kSpeed.GetY();
	str += _T(", ");
	str += (double)g_kSpeed.GetZ();
	str += _T(", ");
	str += (double)g_kSpeed.GetW();
	str += _T(")");
	*/

	/*
	str +=_T("\nTotal Time : ");
	str +=g_pkApp->m_DebugInfo.m_iTotalUpdateTime;
	str +=_T("\nTotal Update Time : ");
	str +=g_pkApp->m_DebugInfo.m_iUpdateTime;
	str +=_T(" Rate : ");
	str +=(g_pkApp->m_DebugInfo.m_iUpdateTime/((float)g_pkApp->m_DebugInfo.m_iTotalUpdateTime))*100.0f;
	str +=_T("%");
	str +=_T("\nTotal Rendering Time : ");
	str +=g_pkApp->m_DebugInfo.m_iRenderingTime;
	str +=_T(" Rate : ");
	str +=(g_pkApp->m_DebugInfo.m_iRenderingTime/((float)g_pkApp->m_DebugInfo.m_iTotalUpdateTime))*100.0f;
	str +=_T("%");
	*/	

	if(pBloom && pBloom->GetActive())
	{
		/*
		str += _T(" \nBlurWidth:");
		str += (double)pBloom->GetParam("BlurWidth");
		str += _T(" \nBrightness:");
		str += (double)pBloom->GetParam("Brightness");
		*/
		/*str += _T(" \nSrc:");
		str += (double)pBloom->GetParam("SceneIntensity");
		str += _T(" \nGIow:");
		str += (double)pBloom->GetParam("GlowIntensity");*/
		

			/*
		str += _T(" \nHighlight:");
		str += (double)pBloom->GetParam("HighlightIntensity");
		str += _T(" \nBlurCount:");
		str += (double)pBloom->GetParam("BlurCount");
		str += _T("\n");
		*/
	}
	if (pFSAA && pFSAA->GetActive())
	{
		//str += _T(" \nFSAA on ");
	}

	//str += _T("Pilot Pos");
	//str += _T(" X:");
	//str += (int)g_kPlayerLoc.x;
	//str += _T(" Y:");
	//str += (int)g_kPlayerLoc.y;
	//str += _T(" Z:");
	//str += (int)g_kPlayerLoc.z;
	//str += _T("\n");

	PgWorld* world = NULL;
	PgActor* pkActor = g_kPilotMan.GetPlayerActor();
	if (pkActor)
		world = pkActor->GetWorld();
	if (world && world->GetUseCameraAdjust())
	{
		if (world->m_kCameraMan.GetCameraModeE() == PgCameraMan::CMODE_FOLLOW || world->m_kCameraMan.GetCameraModeE() == PgCameraMan::CMODE_FOLLOW_DEPTH_PATH)
		{
			PgCameraModeFollow* pkCameraMode = dynamic_cast<PgCameraModeFollow*>(world->m_kCameraMan.GetCameraMode());
			if (pkCameraMode != NULL)
			{					
				str += _T(" WIDTH:");
				str += (int)pkCameraMode->GetCameraAdjustInfo().kCameraAdjust.fCameraWidth;				
				str += _T(" ZOOM:");
				str += (int)pkCameraMode->GetCameraAdjustInfo().kCameraAdjust.fDistanceFromTarget;
				str += _T(" UP:");
				str += (int)pkCameraMode->GetCameraAdjustInfo().kCameraAdjust.fCameraHeight;
				str += _T("\n");
				
				str += _T(" TARGET_X:");
				str += (int)pkCameraMode->GetCameraAdjustInfo().kCameraAdjust.fLookAtWidth;				
				str += _T(" TARGET_Y:");
				str += (int)pkCameraMode->GetCameraAdjustInfo().kCameraAdjust.fLookAtDepth;
				str += _T(" TARGET_Z:");
				str += (int)pkCameraMode->GetCameraAdjustInfo().kCameraAdjust.fLookAtHeight;
				str += _T("\n");
			}
		}
		str += _T("Cam");
		str += _T(" X:");	str += (float)g_kCameraTrn.x;
		str += _T(" Y:");	str += (float)g_kCameraTrn.y;
		str += _T(" Z:");	str += (float)g_kCameraTrn.z;
		str += _T(" -- ");
		str += _T(" X:");	str += (float)g_kCameraRot.GetX();
		str += _T(" Y:");	str += (float)g_kCameraRot.GetY();
		str += _T(" Z:");	str += (float)g_kCameraRot.GetZ();
		str += _T(" W:");	str += (float)g_kCameraRot.GetW();
		str += _T("\n");
	}
/*
	str += _T("Cam");
	str += _T(" X:");	str += (float)g_kCameraTrn.x;
	str += _T(" Y:");	str += (float)g_kCameraTrn.y;
	str += _T(" Z:");	str += (float)g_kCameraTrn.z;
	str += _T(" -- ");
	str += _T(" X:");	str += (float)g_kCameraRot.GetX();
	str += _T(" Y:");	str += (float)g_kCameraRot.GetY();
	str += _T(" Z:");	str += (float)g_kCameraRot.GetZ();
	str += _T(" W:");	str += (float)g_kCameraRot.GetW();
	str += _T("\n");

	str += _T(":");
	*/

	if ( g_bShowChannelInfo && g_kPilotMan.GetPlayerActor() )
	{
#ifdef USE_DYNAMIC_LCHANNEL
		int iPrimaryNo = (g_kNowGroundKey.LogicalChannelNo()>>8) + 1;
		int iSecondaryNo = SECONDARY_CHANNEL(g_kNowGroundKey.LogicalChannelNo());
		str += _T("Channel : ");
		str += g_kNowGroundKey.GroundNo();
		str += _T("[");
		str += iPrimaryNo;
		str += _T("-");
		str += iSecondaryNo;
		str += _T("]");
		str += _T("\n");
#else
		str += _T("Channel : ");
		str += g_kNowGroundKey.GroundNo();
//		str += _T("[");
//		str += (g_kNowGroundKey.LogicalChannelNo()+1);
//		str += _T("]");
		str += _T("\n");
#endif
	}

	if(g_bShowCharacterPosition)
	{
		PgActor *pkActor = g_kPilotMan.GetPlayerActor();
		if(pkActor)
		{
			NiPoint3 kCharPos = pkActor->GetPosition();
			str += _T("Char Pos : ");
			str += (float)kCharPos.x;
			str += _T(", \t");
			str += (float)kCharPos.y;
			str += _T(", \t");
			str += (float)kCharPos.z;
			str += _T("\n");
		}
	}

	if(g_bShowTriangleCount)
	{
		str += _T("Triangle : ");
		str += (int)g_iTriangleCount;
		str += _T("\n");
	}

	if (g_pkApp->GetFrameRateEnabled())
	{
		str += _T("RefCount : ");
		str += (int)NiRefObject::GetTotalObjectCount();
		if (PgAlphaAccumulator::GetUseAlphaGroup())
			str += _T(" Using AlphaGroup");
		str += _T("\n");
	}

	if( m_pRenderer )
	{
		static XUI::CXUI_Font *pFont = g_kFontMgr.GetFont(FONT_TEXT);

		//g_pkApp->GetFrameRate();
		//float rate = g_pkApp->GetFrameTime()*60.0;
		//str += _T("FPS: ");
		//str += floor(1.0f / g_pkApp->GetFrameTime());

		//str += ((int)rate) *100;
		SRenderTextInfo kRenderTextInfo;

		kRenderTextInfo.dwDiffuseColor = COLOR_GREEN;
		kRenderTextInfo.fAlpha = 1.0f;
		kRenderTextInfo.kLoc = POINT2( 10, 50 );
		SetRect(&kRenderTextInfo.rcDrawable, 0, 0, 0, 0);
		kRenderTextInfo.wstrText = (const wchar_t*)str;
		kRenderTextInfo.wstrFontKey = FONT_TEXT;
			
		static CXUI_2DString* p2DString = (CXUI_2DString*)g_kFontMgr.CreateNew2DString(PgFontDef(pFont, kRenderTextInfo.dwDiffuseColor), kRenderTextInfo.wstrText);

		if(p2DString)
		{
			p2DString->SetText(PgFontDef(pFont, kRenderTextInfo.dwDiffuseColor),kRenderTextInfo.wstrText);
		}

		kRenderTextInfo.m_p2DString = p2DString;

		g_kUIScene.RenderText(kRenderTextInfo);
	}
#endif
	return true;
}

bool FInputHook( int const iKey, XUI::E_INPUT_EVENT_INDEX type ){return false;}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



PgUIScene::PgUIScene(void)
	: PgIScene(100, 5)
	, m_kCursor(this)
	, m_spRenderTargetGroup(0)	
	, m_spRenderTexture(0)
	, m_spScreenTexture(0)
	, m_bInsideDraw(false)
	, m_bUseOffscreenDraw(false)
//	, m_kTextObjPool(20)
//	, m_kSpriteObjPool(20)
	, m_pInputHookFunc(FInputHook)
	, m_dwLastMouseMoveTime(0)
	, m_bMouseMove(false)
	, m_bAlwaysMinimapDraw(true)//PgAlwaysMiniMap::DrawText와 같아야함
{

}

PgUIScene::~PgUIScene(void)
{
	//Terminate();
}

bool PgUIScene::Initialize()
{
	m_pImeNameTextBalloon = new PgToolTipBalloon();
	m_pImeNameTextBalloon->SetTextFont(std::wstring(_T("ImeToolTip")));
	m_pImeCandidateTextBalloon = new PgToolTipBalloon();
	m_pImeCandidateTextBalloon->SetTextFont(std::wstring(_T("ImeToolTip")));
	m_pImeCompStrTextBalloon = new PgToolTipBalloon();
	m_pImeCompStrTextBalloon->SetTextFont(std::wstring(_T("ImeToolTip")));
	m_pImeCompReadTextBalloon = new PgToolTipBalloon();
	m_pImeCompReadTextBalloon->SetTextFont(std::wstring(_T("ImeToolTip")));

	m_bImeToolTipCursor = true;

	// QUESTION: 없는 파일인데 작동은 잘 되나?
	NiSourceTexture	*pTexture = g_kNifMan.GetTexture("../Data/6_UI/White.dds");
	m_spFillRect = NiNew NiScreenTexture(pTexture);
	m_spFillRect->SetApplyMode(NiTexturingProperty::APPLY_MODULATE);

	memset( m_bOldKeyLog, 0, sizeof(m_bOldKeyLog) );

	//	이모티콘 폰트 추가.
	g_kEmoFontMgr.ParseXML("FontSwitchTable.xml");
	g_kEmoFontMgr.ParseXML("SysFontSwitchTable.xml");

	PgEmoticonFont	*pkEmoticonFont = new PgEmoticonFont(UNI("EmoticonFont"),"EmoticonFont.xml");
	g_kFontMgr.AddNewFont(UNI("EmoticonFont"),pkEmoticonFont);

	// 툴팁 소켓 론트 추가
	pkEmoticonFont = new PgEmoticonFont(UNI("EmoticonFont13x13"),"EmoticonFont13x13.xml");
	g_kFontMgr.AddNewFont(UNI("EmoticonFont13x13"),pkEmoticonFont);

	pkEmoticonFont = new PgEmoticonFont(UNI("EmoticonFont24x24"),"EmoticonFont24x24.xml");
	g_kFontMgr.AddNewFont(UNI("EmoticonFont24x24"),pkEmoticonFont);

	
	pkEmoticonFont = new PgEmoticonFont(UNI("EmoticonFont32x32"),"EmoticonFont32x32.xml");
	g_kFontMgr.AddNewFont(UNI("EmoticonFont32x32"),pkEmoticonFont);

	// 업적 타이틀
	pkEmoticonFont = new PgEmoticonFont(UNI("AchievementsTitleFont"),"AchievementsTitleFont.xml");
	g_kFontMgr.AddNewFont(UNI("AchievementsTitleFont"),pkEmoticonFont);

	// GIF 타이틀
	pkEmoticonFont = new PgEmoticonFont(UNI("GIFTitleFont"),"GIFTitleFont.xml");
	g_kFontMgr.AddNewFont(UNI("GIFTitleFont"),pkEmoticonFont);

	// 시스템용 이모티콘 폰트
	pkEmoticonFont = new PgEmoticonFont(UNI("SysEmoFont"),"SysEmoFont.xml");
	g_kFontMgr.AddNewFont(UNI("SysEmoFont"),pkEmoticonFont);
#if 0	
	unsigned int uiScreenWidth = XUIMgr.GetResolutionSize().x;
	unsigned int uiScreenHeight = XUIMgr.GetResolutionSize().y;
	NiRenderer* pkRenderer = NiRenderer::GetRenderer();
	PG_ASSERT_LOG(pkRenderer);
	if (pkRenderer)
	{
		uiScreenWidth = pkRenderer->GetDefaultRenderTargetGroup()->GetWidth(0);
		uiScreenHeight = pkRenderer->GetDefaultRenderTargetGroup()->GetHeight(0);
	}

	if (uiScreenHeight != 0.0f && uiScreenWidth != 0.0f)
	{
		NiTexture::FormatPrefs kFormat;
		kFormat.m_ePixelLayout= NiTexture::FormatPrefs::TRUE_COLOR_32;
		kFormat.m_eAlphaFmt = NiTexture::FormatPrefs::SMOOTH;
		kFormat.m_eMipMapped = NiTexture::FormatPrefs::NO;
		m_spRenderTexture = NiRenderedTexture::Create(uiScreenWidth, uiScreenHeight, pkRenderer, kFormat);
		if (m_spRenderTexture)
		{
			m_spRenderTargetGroup = NiRenderTargetGroup::Create(m_spRenderTexture->GetBuffer(), pkRenderer, false, false);
			if (m_spRenderTargetGroup)
			{
				m_spScreenTexture = NiNew NiScreenTexture(m_spRenderTexture);
				if (m_spScreenTexture)
				{
					m_spScreenTexture->SetApplyMode(NiTexturingProperty::APPLY_MODULATE);
					m_spScreenTexture->AddNewScreenRect(0, 0, (unsigned short)uiScreenWidth, (unsigned short)uiScreenHeight, 0, 0, NiColorA::WHITE);
					m_spScreenTexture->MarkAsChanged(NiScreenTexture::EVERYTHING_MASK);
				}
			}
		}		
	}
#endif
	
	m_iImeToolTipCursorToggleTime = 0;
	m_fLastUpdateTime = NiGetCurrentTimeInSec();

	m_kMissionImgCont.clear();

	m_pkWorldMapUI = 0;
	m_pkWorldMapPopUpUI = 0;

	return	true;
}

bool PgUIScene::Build()
{
/*
	NiPoint2 m_kUINavHelpStart;
	NiUILabelPtr m_spOutputLog;
	NiUILabelPtr m_spFrameRateLabel;
	NiPoint2 m_kUIElementGroupOffset(100.0f, 100.0f);
	float m_uiMaxOutputLogLines = 3;
	float m_fUIGroupHeaderHeight = 0;

	unsigned int uiWidth, uiHeight;
    NiRenderer::GetRenderer()->ConvertFromNDCToPixels(1.0f, 1.0f, 
        uiWidth, uiHeight);

    NiPoint2 kDimensions(0.0f, 0.0f);

    NiUIGroup* pkLogGroup = NiNew NiUIGroup("Output Log", m_fUIGroupHeaderHeight, true);
    
    float fLogWidth = NiMin(0.45f, NiUIManager::GetUIManager()->GetMaxCharWidthInNSC() * 35.0f);
    float fCharHeight = NiUIManager::GetUIManager()->GetMaxCharHeightInNSC();
    float fFRHeight = fCharHeight;
    float fLogHeight = fFRHeight * (float)m_uiMaxOutputLogLines;
    
	kDimensions.x += fLogWidth + 2.0f * m_kUIElementGroupOffset.x;
    kDimensions.y += m_kUIElementGroupOffset.y;

    m_spFrameRateLabel = NiNew NiUILabel("", NiColorA::BLACK, NiColor::WHITE);
    m_spFrameRateLabel->SetOffset(m_kUIElementGroupOffset.x, kDimensions.y);
    m_spFrameRateLabel->SetDimensions(fLogWidth, fFRHeight);
    m_spFrameRateLabel->SetAlignment(NiUILabel::TOP, NiUILabel::LEFT);
    pkLogGroup->AddChild(m_spFrameRateLabel);
    kDimensions.y += fFRHeight + 0.5f * fCharHeight;

    m_spOutputLog = NiNew NiUILabel( "", NiColorA::BLACK, NiColor::WHITE);
    m_spOutputLog->SetOffset(m_kUIElementGroupOffset.x, kDimensions.y);
    m_spOutputLog->SetDimensions(fLogWidth, fLogHeight);
    m_spOutputLog->SetAlignment(NiUILabel::TOP, NiUILabel::LEFT);
    pkLogGroup->AddChild(m_spOutputLog);
    kDimensions.y += fLogHeight + fCharHeight;
    
//    AddLogEntry("To hide the log and all UI elements, press \'Z\' on a "
//        "keyboard or left analog stick press on a Gamepad.");


//매니저에 등록.
    pkLogGroup->SetOffset(0.0f, 0.0f);
    pkLogGroup->SetDimensions(kDimensions.x, kDimensions.y);
    pkLogGroup->UpdateRect();
    NiUIManager::GetUIManager()->AddUIGroup(pkLogGroup);

    m_kUINavHelpStart.x = 0.0f;
    m_kUINavHelpStart.y = kDimensions.y + fCharHeight;

	pkLogGroup->SetVisible(true);
//	if (m_bUseNavSystem)
//	{
//      m_uiNavControllerLine = AddLogEntry("");
//
//	NiNavBaseController* pkController = 
//		NiNavManager::GetNavManager()->GetCurrentNavController();
//	UpdateControllerLog(pkController);
//	}
*/

//종료시 처리.
//	NiUIManager::GetUIManager()->PrepareRenderer();
	return true;
}

//#define MAX_KEY 255
bool PgUIScene::ProcessInput(PgInput *pkInput)
{	
	// xxxxxxxxxxxxxxx TODO PgIScene 에서 ProcessInput을 1차적으로 처리하고
	// xxxxxxxxxxxxxxx TODO PgIScenen에서 <INPUT>을 파싱해서, PgWorld, PgUIScene에서 공유하게 한다.
	if(pkInput->IsRemote())
	{
		InputContainer::iterator itr = m_kInputContainer.find(pkInput->GetUKey());

		if(itr != m_kInputContainer.end())
		{
			lua_tinker::call<bool, lwPacket>(itr->second.c_str(), lwPacket(pkInput->GetPacket()));
		}
		return false;
	}
	else if(pkInput->IsExtendedSlot() && XUIMgr.GetFocusedEdit())
	{
		// 어딘가 EditBox Focus가 가 있다면, ExtendedSlot은 무조건 무시.
		unsigned int uiAppData = pkInput->Data()->GetAppData();//Unique Key
		int const value = pkInput->Data()->GetDataValue();
		//int const iVal = g_kGlobalOption.GetUKeyToKey(uiAppData);//Key
		int const iVal = uiAppData - PgInput::UR_LOCAL_BEGIN;
		if (iVal < NiInputKeyboard::KEY_TOTAL_COUNT)
		{
			m_bOldKeyLog[iVal] = (!value) ? false : true;
		}
		return true;
	}
	

	if(pkInput->IsLocal() && (!pkInput->GetPressed()))
	{
		g_kPilotMan.UpdateDirectionSlot(pkInput->GetUKey(),pkInput->GetPressed());

#ifndef EXTERNAL_RELEASE
		switch(pkInput->GetUKey() - PgInput::UR_LOCAL_BEGIN)
		{
        case NiInputKeyboard::KEY_F3:
            {
                NewWare::OptionFlag::EnableDumpScene( !NewWare::OptionFlag::EnableDumpScene() );
            }
            break;
		case NiInputKeyboard::KEY_F6:
			{
				SHORT const sKeyRet = ::GetAsyncKeyState(VK_LCONTROL);
				if( sKeyRet ) 
				{
					g_kLegendItemMakeHelpMng.Clear();
					g_kSuperGroundEnterUIMgr.Clear();
					g_kRagnarokGroundEnterUIMgr.Clear();
					g_kBalloonImageMng.Clear();
					PgDefStringDebugUtil::ClearDebugString();
					if( g_pkTT ) {g_pkTT->Clear();}//기존 초기화
					PgXmlLoader::ReleaseXmlDocumentInCacheByID(XML_ELEMENT_TEXT_TABLE);//캐쉬 날리고
					PgXmlLoader::ReleaseXmlDocumentInCacheByType(XML_ELEMENT_QUEST_TEXT_TABLE);
					PgXmlLoader::ReleaseXmlDocumentInCacheByID(XML_ELEMENT_MAP_TEXT_TABLE);
					PgXmlLoader::ReleaseXmlDocumentInCacheByID(XML_ELEMENT_LEGEND_ITEM_MAKE_HELP);
					PgXmlLoader::ReleaseXmlDocumentInCacheByID(XML_ELEMENT_SUPER_GROUND_ENTER_SETTING);
					PgXmlLoader::ReleaseXmlDocumentInCacheByID(XML_ELEMENT_RAGNAROK_GROUND_ENTER_SETTING);
					PgXmlLoader::ReleaseXmlDocumentInCacheByID(XML_ELEMENT_BALLOON_FORM);
					PgXmlLoader::CreateObject(XML_ELEMENT_TEXT_TABLE);//새로 로드
					PgXmlLoader::CreateObjectByType(XML_ELEMENT_QUEST_TEXT_TABLE);
					PgXmlLoader::CreateObject(XML_ELEMENT_MAP_TEXT_TABLE);
					PgXmlLoader::CreateObject(XML_ELEMENT_LEGEND_ITEM_MAKE_HELP);
					PgXmlLoader::CreateObject(XML_ELEMENT_SUPER_GROUND_ENTER_SETTING);
					PgXmlLoader::CreateObject(XML_ELEMENT_RAGNAROK_GROUND_ENTER_SETTING);
					PgXmlLoader::CreateObject(XML_ELEMENT_BALLOON_FORM);
					g_kStitchImageMng.Reload();
					g_kTextDialogMng.Reload();
					g_kBookActorCameraMgr.Init();
					
					return true;
				}				
			}break;
		case NiInputKeyboard::KEY_F8:
			{				
				SHORT const sKeyRet = ::GetAsyncKeyState(VK_LCONTROL);
				if( sKeyRet ) 
				{
					ResetUI();
					
					return true;
				}
			}break;
		case NiInputKeyboard::KEY_F11:
			{
				SHORT const sKeyRet = ::GetAsyncKeyState(VK_LCONTROL);
				if( ::OpenClipboard(NULL)
				&&	sKeyRet )
				{
					if( ::IsClipboardFormatAvailable(CF_TEXT) )//문자형이클립보드에있으면
					{
						HANDLE hData = ::GetClipboardData(CF_TEXT);//데이터를가져와서
						LPVOID pVoid = ::GlobalLock(hData);
						//LPCTSTR pData = (LPCTSTR)
						if( pVoid )
						{
							std::string kTemp = (char const*)pVoid;
							if( !kTemp.empty() )
							{
								lua_tinker::dobuffer(*lua_wrapper_user(g_kLuaTinker), kTemp.c_str(), kTemp.size());
							}
						}
						::GlobalUnlock(hData);
					}
					::CloseClipboard();

					return true;
				}
			}break;
		}
#endif
	}
	
//	static bool bOld_KeyLog[MAX_KEY] = {false, };//이전 상태값
//	static bool bCur_KeyLog[MAX_KEY] = {false, };//이후 상태값

	PG_STAT(PgStatTimerF timer(g_kUIStatGroup.GetStatInfo("ProcessInput"), g_pkApp->GetFrameCount()));

	m_bMouseMove = false;
//클릭 미클릭.
//	pkInput->Data()->
	if( pkInput->Data() )
	{
		int const value = pkInput->Data()->GetDataValue();

		int xui_value = value;
		XUI::E_INPUT_EVENT_INDEX xui_event_type = XUI::IEI_NONE;

		POINT3I pt3Pos;

		unsigned int uiAppData = pkInput->Data()->GetAppData();//Unique Key
		//int const iVal = g_kGlobalOption.GetUKeyToKey(uiAppData);//Key
		int const iVal = uiAppData - PgInput::UR_LOCAL_BEGIN;
		int const iOrgKey = g_kGlobalOption.GetUKeyToKey(uiAppData);

		float fMouseSpeedSensitivity = 1;
		if(g_bUseDirectInput)
		{
			fMouseSpeedSensitivity = XUIMgr.MouseSensitivity()/10 + 0.4f;;//Max Windows Sensitivity
			fMouseSpeedSensitivity = NiMax(fMouseSpeedSensitivity, 0.8f);
			fMouseSpeedSensitivity = NiMin(fMouseSpeedSensitivity, 2.0f);			
		}		
		
		switch(uiAppData)
		{
		case NiAction::MOUSE_AXIS_X:
			{
				xui_event_type = XUI::IEI_MS_MOVE;
				if(g_bUseDirectInput)
				{
					pt3Pos = POINT3I( (int)(value*fMouseSpeedSensitivity), 0, 0 );
				}
				else
				{
					pt3Pos = POINT3I(value - XUIMgr.MousePos().x,0,0);
				}
				m_bMouseMove = true;
			}break;
		case NiAction::MOUSE_AXIS_Y:
			{
				xui_event_type = XUI::IEI_MS_MOVE;
				if(g_bUseDirectInput)
				{
					
					pt3Pos = POINT3I( 0, (int)(value*fMouseSpeedSensitivity), 0);
				}
				else
				{
					pt3Pos = POINT3I(0,value - XUIMgr.MousePos().y,0);
				}
				m_bMouseMove = true;
			}break;
		case NiAction::MOUSE_AXIS_Z:
			{
				xui_event_type = XUI::IEI_MS_MOVE;
				pt3Pos = POINT3I( 0, 0, value/20);
				m_bMouseMove = true;
			}break;
		case NiAction::MOUSE_BUTTON_LEFT:
			{
				xui_value = XUI::MEI_BTN_0;
				
				if( !value )
				{
					xui_event_type = XUI::IEI_MS_UP;
				}
				else
				{
					xui_event_type = XUI::IEI_MS_DOWN;
				}
				m_bMouseMove = true;
			}break;
		case NiAction::MOUSE_BUTTON_RIGHT:	
			{
				xui_value = XUI::MEI_BTN_1;

				if( !value )
				{
					xui_event_type = XUI::IEI_MS_UP;
				}
				else
				{
					xui_event_type = XUI::IEI_MS_DOWN;
				}
				m_bMouseMove = true;
			}break;
		case NiAction::MOUSE_BUTTON_MIDDLE:	
			{
				xui_value = XUI::MEI_BTN_2;

				if( !value )
				{
					xui_event_type = XUI::IEI_MS_UP;
				}
				else
				{
					xui_event_type = XUI::IEI_MS_DOWN;
				}
				m_bMouseMove = true;
			}break;
		default:
			{
				//if(uiAppData >= PgInput::UR_LOCAL_BEGIN || uiAppData < PgInput::UR_REMOTE_BEGIN)
				if( 0 != iOrgKey )
				{//키보드 인풋.
					//xui_value = uiAppData-PgInput::UR_LOCAL_BEGIN;
					xui_value = uiAppData;

					bool bDown = (!value)? false: true;
//					bOld_KeyLog[iVal] = bCur_KeyLog[iVal];//이전 상태 기록
//					bCur_KeyLog[iVal] = bDown;

					// 주의 : KEY_UP과 KEY_DOWN이 바껴있다!!!!!!!(바껴있는거 수정할려고 하면 고생좀 할꺼다)
					if( !m_bOldKeyLog[iVal] && bDown)
					{
						xui_event_type = XUI::IEI_KEY_UP;				
					}
					else if( m_bOldKeyLog[iVal] && !bDown)
					{
						xui_event_type = XUI::IEI_KEY_DOWN;
					}
					else
					{
						xui_event_type = XUI::IEI_KEY_DOWN_PRESS;
					}
					m_bOldKeyLog[iVal] = bDown;

					if ( (*m_pInputHookFunc)( iVal, xui_event_type ) )
					{
						return true;
					}

					switch( (NiInputKeyboard::KeyCode)iVal )
					{
					case NiInputKeyboard::KEY_TAB:
						{
							if( XUI::IEI_KEY_UP == xui_event_type )
							{
								TabGame(true);

								XUI::CXUI_Edit* pkEdit = XUI::CXUI_Edit::GetFocusedEdit();
								if( pkEdit )
								{//이놈은 에디트간 포커스 이동을 시켜준다.
									CXUI_Edit::FocusingPassGroupEdit(pkEdit);
								}
								
								m_bAlwaysMinimapDraw = !m_bAlwaysMinimapDraw;

								PgAlwaysMiniMap::DrawText(m_bAlwaysMinimapDraw);
								if(m_bAlwaysMinimapDraw)
								{
									PgMiniMapUtil::CallMiniMapIconInfoUI();
								}
								else
								{
									PgMiniMapUtil::CloseMiniMapIconInfoUI();
								}
							}
							else if ( XUI::IEI_KEY_DOWN == xui_event_type )
							{
								TabGame(false);
							}
						}break;
					case NiInputKeyboard::KEY_ESCAPE:
						{
							if(g_kMovieMgr.IsPlay() && XUI::IEI_KEY_DOWN == xui_event_type)
							{
								g_kMovieMgr.Stop();
								return true;
							}

							if ( XUI::IEI_KEY_DOWN == xui_event_type && CallESCScript() )
							{
								return true;
							}
						}break;
					case NiInputKeyboard::KEY_GRAVE:	// HideUI로 사라진 UI를 다시 보이게만 한다.
						{
							if ( XUI::IEI_KEY_DOWN == xui_event_type )
							{
								if (g_bToggleUIDraw == 0)
									g_bToggleUIDraw = 3;
							}
						}
						break;
					case NiInputKeyboard::KEY_F11:
						{
							if ( XUI::IEI_KEY_DOWN == xui_event_type )
							{
								g_pkApp->ToggleFrameRate();
							}
						}
						break;
					case NiInputKeyboard::KEY_LMENU:
						{
							if( XUI::IEI_KEY_DOWN == xui_event_type )
							{
								XUI::CXUI_Wnd *pkTooltip_Item = XUIMgr.Get(_T("ToolTip2"));
								if(pkTooltip_Item)
								{
									XUI::CXUI_Wnd *pkTooltip = XUIMgr.Get(_T("ToolTipItemExtend"));
									if(!pkTooltip)
									{
										lwHidePartUI("TOOLTIPITEMEXTEND", false);
									}
									else
									{
										lwHidePartUI("TOOLTIPITEMEXTEND", true);
									}
								}
							}
						}break;
					}
				}
			}break;
		}		
		

		if( GetPushModifierState() )
		{
			xui_value = iOrgKey + PgInput::UR_LOCAL_BEGIN;
		}

		bool	const	bIsReleased = (pkInput->GetPressed() == false && XUIMgr.GetFocusedEdit());

		if (g_kMissionComplete.CompleteState() == PgMissionComplete::E_MSN_CPL_NONE || 
			g_kMissionComplete.CompleteState() == PgMissionComplete::E_MSN_CPL_WAIT)
		{
			if( XUIMgr.VPeekEvent( xui_event_type, pt3Pos, xui_value, &iVal) )//! XUI에서 처리를 했다면, 그냥 return을 해버린다.
			{
				return !bIsReleased;
			}
			else
			{
				if( 1028 == xui_value )
				{
					if( g_kChatMgrClient.CheckChatOut() )
					{
						g_kChatMgrClient.SetChatOutFocus();
						return !bIsReleased;
					}
				}
			}
		}
		else if (g_kBossComplete.CompleteState() == PgBossComplete::E_MSN_CPL_NONE || 
			g_kBossComplete.CompleteState() == PgBossComplete::E_MSN_CPL_WAIT)
		{
			if( XUIMgr.VPeekEvent( xui_event_type, pt3Pos, xui_value, &iVal) )//! XUI에서 처리를 했다면, 그냥 return을 해버린다.
			{
				return !bIsReleased;
			}
		}
		else if (g_kMissionComplete.CompleteState() == PgMissionComplete::E_MSN_CPL_REWARD_WAIT && xui_value == 1028 )
		{
			if( XUIMgr.VPeekEvent( xui_event_type, pt3Pos, xui_value, &iVal) )//! XUI에서 처리를 했다면, 그냥 return을 해버린다.
			{
				return !bIsReleased;
			}
		}
		else if (g_kBossComplete.CompleteState() == PgBossComplete::E_MSN_CPL_REWARD_WAIT && xui_value == 1028 )
		{
			if( XUIMgr.VPeekEvent( xui_event_type, pt3Pos, xui_value, &iVal) )//! XUI에서 처리를 했다면, 그냥 return을 해버린다.
			{
				return !bIsReleased;
			}
		}
		else
		{
			if (xui_event_type == XUI::IEI_MS_DOWN || xui_event_type == XUI::IEI_MS_UP || xui_event_type == XUI::IEI_MS_MOVE )
			{
				if( XUIMgr.VPeekEvent( xui_event_type, pt3Pos, xui_value, &iVal) )//! XUI에서 처리를 했다면, 그냥 return을 해버린다.
				{
					return !bIsReleased;
				}
			}			
		}
	}

	PgWorld* pkWorld = g_kPilotMan.GetPlayerWorld();
	if (pkWorld)
	{ 
		if (pkWorld->GetCameraMan()->Input(pkInput))
		{
			return false;
		}
	}


	return false;
}

PgUIUVSpriteObject* PgUIScene::GetIconTexture(int const iItemNo)
{
	PG_STAT(PgStatTimerF timer1(g_kUIStatGroup.GetStatInfo("GetIconTexture"), g_pkApp->GetFrameCount()));
	
	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const* pkItemDef = kItemDefMgr.GetDef(iItemNo);
	if( !pkItemDef )
	{
		return NULL;
	}

	CONT_IMG::const_iterator kIter = m_mapImg.find(pkItemDef->ResNo());
	if(m_mapImg.end() == kIter)
	{
		if(true==AddIconRsc(pkItemDef->ResNo()))
		{
			kIter = m_mapImg.find(pkItemDef->ResNo());
		}
	}
	
	if(m_mapImg.end() != kIter)
	{
		NiScreenTexture* pkOriginal = kIter->second->GetTex();
		NiTexture* pkTexture = pkOriginal->GetTexture();
		if (pkTexture == NULL)
		{
			NILOG(PGLOG_ERROR, "[PgUISpriteObject] %d Item icon - can't find source texture\n", iItemNo);
			return NULL;
		}

		NiScreenTexture *pkSprite = NiNew NiScreenTexture(pkTexture);
		PG_ASSERT_LOG(pkSprite);
		if (pkSprite == NULL)
			return NULL;

		AddScreenTextureToSpriteCont(pkSprite);	// 나중에 리소스 해지를 위해서 넣어둔다.
		
		PgUIUVSpriteObject* pkSpriteObj =  new PgUIUVSpriteObject(pkSprite, ((PgUIUVSpriteObject*)kIter->second)->GetUVInfo());
		PG_ASSERT_LOG(pkSpriteObj);
		if (pkSpriteObj == NULL)
			return NULL;

		m_kSpriteObjContainer.push_back(pkSpriteObj); // 나중에 리소스 해지를 위해서 넣어둔다.
		return pkSpriteObj;
	}

	NILOG(PGLOG_ERROR, "Can't find %d item Resource", iItemNo);
	PG_ASSERT_LOG(0);
	return NULL;//실패
}
void	PgUIScene::AddScreenTextureToSpriteCont(NiScreenTexture *pkScreenTexture)
{
	m_kSpriteContainer.push_back(pkScreenTexture);
}
void	PgUIScene::RemoveScreenTextureFromSpriteCont(NiScreenTexture *pkScreenTexture)
{
	for(SpriteContainer::iterator itor = m_kSpriteContainer.begin(); itor != m_kSpriteContainer.end(); ++itor)
	{
		NiScreenTexture	*pkScreenTexture2 = *itor;
	
		if(pkScreenTexture2 == pkScreenTexture)
		{
			SAFE_DELETE_NI(pkScreenTexture2);
			m_kSpriteContainer.erase(itor);
			return;
		}
	}
}

void	PgUIScene::DrawFillRect(PgRenderer *pkRenderer,int iDX,int iDY,int iWidth,int iHeight,const NiColorA &kColor)
{
	PG_STAT(PgStatTimerF timer(g_kUIStatGroup.GetStatInfo("DrawFillRect"), g_pkApp->GetFrameCount()));
	int	iTexWidth = 64,iTexHeight = 64;

	int	iHorizCount = iWidth/iTexWidth;
	int	iHorizRemain = iWidth%iTexWidth;
	int	iVertCount = iHeight/iTexHeight;
	int	iVertRemain = iHeight%iTexHeight;

	m_spFillRect->RemoveAllScreenRects();

	for(int i=0;i<iVertCount;i++)
	{
		for(int j=0;j<iHorizCount;j++)
		{
			m_spFillRect->AddNewScreenRect(iDY+i*iTexHeight,iDX+j*iTexWidth,iTexWidth,iTexHeight,0,0,kColor);
		}
		if(iHorizRemain>0)
			m_spFillRect->AddNewScreenRect(iDY+i*iTexHeight,iDX+iHorizCount*iTexWidth,iHorizRemain,iTexHeight,0,0,kColor);
	}
	if(iVertRemain>0)
	{
		for(int j=0;j<iHorizCount;j++)
		{
			m_spFillRect->AddNewScreenRect(iDY+iVertCount*iTexHeight,iDX+j*iTexWidth,iTexWidth,iVertRemain,0,0,kColor);
		}
		if(iHorizRemain>0)
			m_spFillRect->AddNewScreenRect(iDY+iVertCount*iTexHeight,iDX+iHorizCount*iTexWidth,iHorizRemain,iVertRemain,0,0,kColor);
	}
	m_spFillRect->MarkAsChanged(NiScreenTexture::EVERYTHING_MASK);
	m_spFillRect->Draw(pkRenderer->GetRenderer());
}

void	PgUIScene::RenderImeInfo(PgRenderer *pkRenderer, float fFrameTime)	//	IME 정보를 렌더링 한다.
{
	PG_STAT(PgStatTimerF timer(g_kUIStatGroup.GetStatInfo("RenderImeInfo"), g_pkApp->GetFrameCount()));
	CXUI_Edit	*pkEdit = (CXUI_Edit*)XUIMgr.GetFocusedEdit();

	if(pkEdit == NULL) 
	{
		return;
	}

	if(m_iImeToolTipCursorToggleTime == 0 || BM::GetTime32()-m_iImeToolTipCursorToggleTime>500)
	{
		m_iImeToolTipCursorToggleTime = BM::GetTime32();
		m_bImeToolTipCursor = !m_bImeToolTipCursor;
	}

	int	iSelectStart,iSelectEnd,iCaretPos;

	CS::CCSIME& kCSIME = XUIMgr.GetIME();
	const CARETDATA caret_data = kCSIME.GetCaretPos();
	
	iSelectStart=caret_data.iSelectStart;
	iSelectEnd=caret_data.iSelectEnd;
	iCaretPos=caret_data.iCaretPos;

	CXUI_Font	*pEditFont = g_kFontMgr.GetFont(pkEdit->EditFont());
	int	iTextWidth = pEditFont->CalcWidth(kCSIME.GetResultStr());
	CXUI_Font	*pImeToolTipFont = g_kFontMgr.GetFont(_T("ImeToolTip"));
	if(!pEditFont || !pImeToolTipFont) return;

	POINT2	ptPosition = pkEdit->TotalLocation()+pkEdit->EditTextPos();
	//	IME 이름 출력
	POINT ptCaret;
	ptCaret.x=ptPosition.x+iTextWidth;
	ptCaret.y=ptPosition.y;

	switch(kCSIME.GetLanguageType())
	{
		case CCSIME::LT_DEFAULT:
		case CCSIME::LT_KOREAN:
			{
				//	Candidate 출력

				if(kCSIME.GetCadidateData().size()>0)
				{
					int	iPosY = ptCaret.y - IME_BALLOON_GAP;
					if(ptPosition.y < IME_BALLOON_GAP)
						iPosY=ptCaret.y+10;

					RenderCandList(pkRenderer,fFrameTime,ptCaret.x,iPosY);
				}
			}
			break;
		case CCSIME::LT_JAPANESE:
			{
				int	iPosY=30;

				//	CompString 출력
				const	CCSIME::CANDI_BUF	kCandiBuf = kCSIME.GetCadidateData();
				std::wstring kCompString = kCSIME.GetCompStr();

				if(kCompString.length()>0)
				{
					int iThisY=ptCaret.y-iPosY;

					if(ptPosition.y<iPosY)
						iPosY=ptCaret.y+iPosY-IME_BALLOON_GAP;

					iPosY+=20;

					POINT2	ptTargetPos;
					ptTargetPos.x= ptCaret.x;
					ptTargetPos.y= iThisY;
					m_pImeCompStrTextBalloon->SetNewBalloon(CT_NORMAL, kCompString,0);
					m_pImeCompStrTextBalloon->SetTargetPos(ptTargetPos);
					m_pImeCompStrTextBalloon->DrawImmediate( pkRenderer, NULL, 0.0f );

					//	Caret 그리기
					if(kCandiBuf.size()==0 && m_bImeToolTipCursor)
					{
						int	iStrLen=kCompString.length();
						int	iCaretPos=caret_data.iImeCursor;

						POINT	ptTextOutLoc=m_pImeCompStrTextBalloon->GetMsgRenderTargetLoc();

						if(iStrLen<iCaretPos)	//	커서만 깜빡인다.
						{
							int	iX=pImeToolTipFont->CalcWidth(kCompString,iCaretPos-1);
							if(iCaretPos-1 == 0) iX = 0;
							int	iHeight = pImeToolTipFont->MaxHeight(kCompString);

							DrawFillRect(pkRenderer,ptTextOutLoc.x+iX,ptTextOutLoc.y+iHeight-4,6,4,NiColorA(0,0,0,1));
							DrawFillRect(pkRenderer,ptTextOutLoc.x+iX+1,ptTextOutLoc.y+iHeight-3,4,2,NiColorA(1,1,1,1));
						}
						else
						{
							//	해당 글자 앞에 세로 막대 커서가 깜빡인다.

							int	iPos=0;

							int iStrLen = kCompString.length();
							for(iPos=0;iPos<iStrLen;iPos++)
							{
								if(iPos<=iCaretPos && iPos+1>iCaretPos) break;
							}
							int iX=pImeToolTipFont->CalcWidth(kCompString,iPos);
							int	iHeight = pImeToolTipFont->MaxHeight(kCompString);
							if(iPos == 0) iX = 0;

							DrawFillRect(pkRenderer,ptTextOutLoc.x+iX+1,ptTextOutLoc.y,2,iHeight,NiColorA(0,0,0,0.5));
						}
					}

				}

				//	Candidate 출력

				if(kCandiBuf.size()>0)
				{
					int iThisY = ptCaret.y-iPosY-IME_BALLOON_GAP;

					if(ptPosition.y<iPosY)
						iPosY=ptCaret.y+iPosY-IME_BALLOON_GAP;

					iPosY+=20;

					RenderCandListVertical(pkRenderer,fFrameTime,ptCaret.x,iThisY,kCSIME.GetCompStr());
				}
			}

			break;
		case CCSIME::LT_TRADITIONAL_CHINESE:
		case CCSIME::LT_SIMPLIFIED_CHINESE:
			{
				int	iPosY=30;

				std::wstring kImeName = kCSIME.GetIMEName();
				if(kImeName.length()>0)	
				{
					int iThisY=ptCaret.y-iPosY;

					if(ptPosition.y<iPosY)
						iPosY=ptCaret.y+iPosY-IME_BALLOON_GAP;

					iPosY+=20;

					POINT2	ptPos;
					ptPos.x=ptCaret.x;
					ptPos.y= iThisY;
					m_pImeNameTextBalloon->SetNewBalloon(CT_NORMAL, kImeName,0);
					m_pImeNameTextBalloon->SetTargetPos(ptPos);
					m_pImeNameTextBalloon->DrawImmediate( pkRenderer, NULL, 0.0f );
				}
						
				const	CCSIME::CANDI_BUF	kCandiBuf = kCSIME.GetCadidateData();
				//	CompString 출력

				std::wstring kCompString = kCSIME.GetCompStr();
				if(kCompString.length()>0)
				{
					int iThisY=ptCaret.y-iPosY;

					if(ptPosition.y<iPosY)
						iPosY=ptCaret.y+iPosY-IME_BALLOON_GAP;

					iPosY+=20;

					POINT2	ptPos;
					ptPos.x=ptCaret.x;
					ptPos.y= iThisY;

					m_pImeCompStrTextBalloon->SetNewBalloon(CT_NORMAL, kCompString,0);
					m_pImeCompStrTextBalloon->SetTargetPos(ptPos);
					m_pImeCompStrTextBalloon->DrawImmediate( pkRenderer, NULL, 0.0f );

					//	Caret 그리기

					if(kCandiBuf.size()==0)
					{

						int	iStrLen=kCompString.length();
						int	iCaretPos=caret_data.iImeCursor;

						POINT	ptTextOutLoc=m_pImeCompStrTextBalloon->GetMsgRenderTargetLoc();

						if(iStrLen<iCaretPos)	//	커서만 깜빡인다.
						{
							if(m_bImeToolTipCursor)
							{
								int	iX=pImeToolTipFont->CalcWidth(kCompString,iCaretPos-1);
								if(iCaretPos-1 == 0) iX = 0;
								int	iHeight = pImeToolTipFont->MaxHeight(kCompString);

								DrawFillRect(pkRenderer,ptTextOutLoc.x+iX,ptTextOutLoc.y+iHeight-4,6,4,NiColorA(0,0,0,1));
								DrawFillRect(pkRenderer,ptTextOutLoc.x+iX+1,ptTextOutLoc.y+iHeight-3,4,2,NiColorA(1,1,1,1));
							}
						}
						else
						{
							//	해당 글자가 블럭 처리 된다.
							int	iPos=0;

							int iStrLen = kCompString.length();
							for(iPos=0;iPos<iStrLen;iPos++)
							{
								if(iPos<=iCaretPos && iPos+1>iCaretPos) break;
							}
							int iX=pImeToolTipFont->CalcWidth(kCompString,iPos);
							if(iPos == 0) iX = 0;

							int iX2=pImeToolTipFont->CalcWidth(kCompString, iPos+1);
							int	iHeight = pImeToolTipFont->MaxHeight(kCompString);
	
							DrawFillRect(pkRenderer,ptTextOutLoc.x+iX-2,ptTextOutLoc.y-2,iX2-iX+4,iHeight+2,NiColorA(0,0,0,0.5));
						}
					}

				}
				//	CompRead 출력

				std::wstring kCompReadString = kCSIME.GetCompReadStr();
				if(kCompReadString.length()>0)
				{
					int iThisY=ptCaret.y-iPosY;

					if(ptPosition.y<iPosY)
						iPosY=ptCaret.y+iPosY-IME_BALLOON_GAP;

					iPosY+=20;

					POINT2 ptPos;
					ptPos.x=ptCaret.x;
					ptPos.y=iThisY;
					m_pImeCompReadTextBalloon->SetNewBalloon(CT_NORMAL, kCompReadString,0);
					m_pImeCompReadTextBalloon->SetTargetPos(ptPos);
					m_pImeCompReadTextBalloon->DrawImmediate( pkRenderer, NULL, 0.0f );

				}

				//	Candidate 출력

				if(kCandiBuf.size()>0)
				{
					int iThisY=ptCaret.y-iPosY;

					if(ptPosition.y<iPosY)
						iPosY=ptCaret.y+iPosY-IME_BALLOON_GAP;

					iPosY+=20;

					RenderCandList(pkRenderer,fFrameTime,ptCaret.x,iThisY);
				}
			}
			break;
	}
}
void	PgUIScene::RenderCandListVertical(PgRenderer *pkRenderer, float fFrameTime,int iDX,int iDY,std::wstring const &kCompString)
{
	PG_STAT(PgStatTimerF timer1(g_kUIStatGroup.GetStatInfo("RenderCandListVertical1"), g_pkApp->GetFrameCount()));
	PG_STAT(PgStatTimerF timer2(g_kUIStatGroup.GetStatInfo("RenderCandListVertical2"), g_pkApp->GetFrameCount()));
	PG_STAT(PgStatTimerF timer3(g_kUIStatGroup.GetStatInfo("RenderCandListVertical3"), g_pkApp->GetFrameCount()));
	PG_STAT(PgStatTimerF timer4(g_kUIStatGroup.GetStatInfo("RenderCandListVertical3"), g_pkApp->GetFrameCount()));
	
	PG_STAT(timer1.Start());
	const CS::CCSIME& kCSIME = XUIMgr.GetIME();
	const	CCSIME::CANDI_BUF	kCandiBuf = kCSIME.GetCadidateData();
	PG_STAT(timer1.Stop());

	if(kCandiBuf.size()==0) return;

	PG_STAT(timer2.Start());

	CXUI_Font	*pFont = g_kFontMgr.GetFont(_T("ImeToolTip"));
	if(!pFont)
	{
		return;
	}

	std::wstring kCandiString,kCandiStringTemp;
	char	buf[255];
	int	iSelected = -1;
	int	iSelectedTextWidth = 0;
	int	iSelectedTextHeight = 0;
	for(unsigned int i=0;i<kCandiBuf.size();i++)
	{
		CCandidateData data = kCandiBuf[i];
		sprintf_s(buf,255,"[%d] ",data.GetIndex());
		kCandiStringTemp=_T("");
		kCandiStringTemp+=UNI(buf);
		kCandiStringTemp+=data.GetString();
		kCandiStringTemp+=_T("\n");

		kCandiString+=kCandiStringTemp;

		if(data.GetString() == kCompString) 
		{
			iSelected = i;
			iSelectedTextWidth = pFont->CalcWidth(kCandiStringTemp);
			iSelectedTextHeight =  pFont->MaxHeight(kCandiStringTemp);
		}
	}
	PG_STAT(timer2.Stop());

	PG_STAT(timer3.Start());

	POINT2 ptPos;
	m_pImeCandidateTextBalloon->SetNewBalloon(CT_NORMAL, kCandiString,0);
	
	PG_STAT(timer3.Stop());

	PG_STAT(timer4.Start());

	int iTotalCand = kCandiBuf.size();

	int	iTargetY=iDY;
	
	if( g_kLocal.ServiceRegion() != LOCAL_MGR::NC_JAPAN )
	{
		iTargetY += -m_pImeCandidateTextBalloon->GetSize().y+12*2;
	}

	if(iTargetY<0)
	{
		iTargetY=iDY+20;
	}

	ptPos.x = iDX;
	ptPos.y = iTargetY;
	m_pImeCandidateTextBalloon->SetTargetPos(ptPos);
	m_pImeCandidateTextBalloon->DrawImmediate( pkRenderer, NULL, 0.0f );

	//	선택 영역 박스 렌더링
	if( 0 <= iSelected )
	{
		Pg2DString	*pText2DString = m_pImeCandidateTextBalloon->GetText(1);
		if ( pText2DString )
		{
			POINT ptTextPos = m_pImeCandidateTextBalloon->GetMsgRenderTargetLoc();
			POINT ptLinePos = pText2DString->GetLinePosition(iSelected);
			ptTextPos.x+=ptLinePos.x;
			ptTextPos.y+=ptLinePos.y;
			DrawFillRect(pkRenderer,ptTextPos.x,ptTextPos.y,iSelectedTextWidth,iSelectedTextHeight,NiColorA(0,0,0,0.5));
		}
	}
	PG_STAT(timer4.Stop());
}

void	PgUIScene::RenderCandList(PgRenderer *pkRenderer, float fFrameTime,int iDX,int iDY)
{
	PG_STAT(PgStatTimerF timer1(g_kUIStatGroup.GetStatInfo("RenderCandList1"), g_pkApp->GetFrameCount()));
	PG_STAT(PgStatTimerF timer2(g_kUIStatGroup.GetStatInfo("RenderCandList2"), g_pkApp->GetFrameCount()));
	PG_STAT(PgStatTimerF timer3(g_kUIStatGroup.GetStatInfo("RenderCandList3"), g_pkApp->GetFrameCount()));
	PG_STAT(PgStatTimerF timer4(g_kUIStatGroup.GetStatInfo("RenderCandList4"), g_pkApp->GetFrameCount()));

	PG_STAT(timer1.Start());
	const CS::CCSIME& kCSIME = XUIMgr.GetIME();
	const	CCSIME::CANDI_BUF	kCandiBuf = kCSIME.GetCadidateData();
	PG_STAT(timer1.Stop());
	if(kCandiBuf.size()==0) return;

	int	iScreenWidth = pkRenderer->GetDefaultRenderTargetGroup()->GetWidth(0);
	int	iScreenHeight = pkRenderer->GetDefaultRenderTargetGroup()->GetHeight(0);

	PG_STAT(timer2.Start());
	std::wstring kCandString;
	char	buf[255];
	for(unsigned int i=0;i<kCandiBuf.size();i++)
	{
		if (i == kCSIME.GetCandidateIndex())
		{
			kCandString+=TEXT("{C=0xFF50FF2E/}");
		}
		CCandidateData data = kCandiBuf[i];
		sprintf_s(buf," [%u]",data.GetIndex());
		kCandString+=UNI(buf);
		kCandString+=data.GetString();
		
		if (i == kCSIME.GetCandidateIndex())
		{
			kCandString+=TEXT("{C=0xFFFFFFFF/}");
		}
	}
	PG_STAT(timer2.Stop());

	PG_STAT(timer3.Start());

	POINT2	ptTarget;
	ptTarget.x=iDX;
	ptTarget.y = iDY;
	m_pImeCandidateTextBalloon->SetTargetPos(ptTarget);
	m_pImeCandidateTextBalloon->SetNewBalloon(CT_NORMAL, kCandString,0);

	PG_STAT(timer3.Stop());
	PG_STAT(timer4.Start());

	POINT	ptSize = m_pImeCandidateTextBalloon->GetSize();
	if(iDX+ptSize.x>iScreenWidth)
		iDX = iScreenWidth - ptSize.x - 10;
	if(iDY+ptSize.y>iScreenHeight)
		iDY = iScreenHeight - ptSize.y - 10;

	if(ptTarget.x != iDX || ptTarget.y != iDY)
	{
		ptTarget.x=iDX;
		ptTarget.y = iDY;
		m_pImeCandidateTextBalloon->SetTargetPos(ptTarget);
	}
	m_pImeCandidateTextBalloon->DrawImmediate( pkRenderer, NULL, 0.0f );
	PG_STAT(timer4.Stop());
}

bool PgUIScene::Update(float fAccumTime, float fFrameTime)
{
	if (g_bToggleUIDraw == 1 || g_bToggleUIDraw == 3)
	{
		g_kUISound.Update(fAccumTime);
		g_kChainAttack.Update(fAccumTime,fFrameTime);
		g_kScreenBreak.Update(fAccumTime,fFrameTime);
		g_kScreenEffect.Update(fAccumTime,fFrameTime);
		g_kEnergyGaugeMan.Update(fAccumTime,fFrameTime);
		g_kComboAdvisor.Update(fAccumTime,fFrameTime);
		g_kBalloonEmoticonMgr.Update(fAccumTime,fFrameTime);
		UIModelContainer::iterator model_itor = m_kUIModelContainer.begin();
		while(model_itor != m_kUIModelContainer.end())
		{
			(*model_itor).second->Update(fAccumTime, fFrameTime);
			++model_itor;
		}

		if (!m_kUIDistActionList.empty())
		{
			UIActinByDistMap::iterator ui_itor = m_kUIDistActionList.begin();

			PgActor *pkActor = g_kPilotMan.GetPlayerActor();
			if (pkActor)
			{
				NiPoint3 const pt3 = pkActor->GetTranslate();
				POINT3 pt(pt3.x, pt3.y, pt3.z);

				while (m_kUIDistActionList.end() != ui_itor)
				{
					if ((*ui_itor).second.Update(pt))
					{
						ui_itor = m_kUIDistActionList.erase(ui_itor);
					}
					else
					{
						++ui_itor;
					}
				} 
			}
		}

		PG_STAT(PgStatTimerF timer(g_kUIStatGroup.GetStatInfo("Update/VOnTick"), g_pkApp->GetFrameCount()));
		XUIMgr.VOnTick();

		MiniMapContainer::iterator iter = m_kMiniMapContainer.begin();
		while (iter != m_kMiniMapContainer.end())
		{
			MiniMapContainer::mapped_type& pkMiniMap = iter->second;
			if (pkMiniMap && pkMiniMap->ShowMiniMap())
			{
				pkMiniMap->Update(0.0f, 0.0f);
			}
			++iter;
		}

		//XUIMgr.FlushUnusedControls();
		m_fLastUpdateTime = NiGetCurrentTimeInSec();
	}

	if(true == m_bMouseMove)
	{// 마우스 움직임이 있었다면, 마우스 커서 보이게하고
		m_bMouseMove = false;
		// 마지막 마우스 움직인 시간을 갱신함
		m_dwLastMouseMoveTime = BM::GetTime32();		
		m_kCursor.ShowCursor(CS_NONE);
	}
	else
	{// 마우스 움직임이 없었고
		DWORD const& dwCurrentTime = BM::GetTime32();
		DWORD const dwElapsTime = dwCurrentTime - m_dwLastMouseMoveTime;
		if(dwElapsTime > MOUSE_HIDE_TIME)
		{// 일정시간동안 없다면 커서를 감춤
			if(false == m_kCursor.HideCursor(CS_NONE) )
			{// 감출수 없다면, 시간 카운트를 리셋한다
				m_dwLastMouseMoveTime = dwCurrentTime;
			}
		}
	}

	return true;
}

void PgUIScene::ResetUI()
{
	XUIMgr.ClearAllControl();

	for(SpriteContainer::iterator itr = m_kSpriteContainer.begin();
		itr != m_kSpriteContainer.end();
		++itr)
	{
		SAFE_DELETE_NI(*itr);
	}
	m_kSpriteContainer.clear();

	for(SpriteObjContainer::iterator itr = m_kSpriteObjContainer.begin();
		itr != m_kSpriteObjContainer.end();
		++itr)
	{
		SAFE_DELETE_NI(*itr);
	}
	m_kSpriteObjContainer.clear();

	for(RenderedSpriteObjContainer::iterator itr = m_kRenderedSpriteObjContainer.begin();
		itr != m_kRenderedSpriteObjContainer.end();
		++itr)
	{
		SAFE_DELETE_NI(*itr);
	}
	m_kRenderedSpriteObjContainer.clear();
	ClearDrawLIst();

	XUIMgr.ClearXMLData();
	
	if(!LoadUIPath("UIPath.xml"))
	{
		PG_ASSERT_LOG(NULL);
	}

	if(!XUIMgr.BuildXUI())
	{
		PG_ASSERT_LOG(NULL);
	}

	PgWorld* world = g_kPilotMan.GetPlayerWorld();
	if (world)
	{
		world->SetWorldDataForAllMiniMap();
	}
	XUIMgr.ReCallControl();
}

//#define USE_BACK_BUFFER
void	PgUIScene::AddToDrawList(PgUIDrawObject *pkDrawObject,bool bPushFront)
{
	//if (m_bInsideDraw == false)
	//{
	//	NILOG(PGLOG_WARNING, "[PgUIScene] AddToDrawList not inside draw, Type:%d (%d,%d)\n", pkDrawObject->Type(), pkDrawObject->GetRect().top, pkDrawObject->GetRect().left);
	//}
	if(g_kMovieMgr.IsPlay())
		return;

	pkDrawObject->SetDeleteLock(true);

	if(bPushFront)
		m_kDrawList.push_front(pkDrawObject);
	else
		m_kDrawList.push_back(pkDrawObject);
}
void	PgUIScene::RemoveFromDrawList(PgUIDrawObject *pkDrawObject)
{
	for(DrawList::iterator itr = m_kDrawList.begin();	itr != m_kDrawList.end(); ++itr)
	{
		PgUIDrawObject *pData = (*itr);
		if(pData == pkDrawObject)
		{
			m_kDrawList.erase(itr);
			return;
		}
	}
}
void	PgUIScene::ClearDrawLIst()
{
	for(DrawList::iterator itr = m_kDrawList.begin();	itr != m_kDrawList.end(); ++itr)
	{
		PgUIDrawObject *pData = (*itr);
		pData->SetDeleteLock(false);
	}

	m_kDrawList.clear();
}
void	PgUIScene::SetUpdateEveryOffscreen()
{
	ClearAllOffscreen();
}
void	PgUIScene::EnableOffScreenRendering(bool bEnable)
{

	NiDX9Renderer *pDX9Renderer = (NiDX9Renderer*)NiRenderer::GetRenderer();
	if(!pDX9Renderer)
	{
		return;
	}

	D3DCAPS9	const	*pkCaps = pDX9Renderer->GetDeviceCaps();
	if(!pkCaps) 
	{
		return;
	}

	//DWORD dwCaps2 = pkCaps->PrimitiveMiscCaps;
	//if( (dwCaps2&D3DPMISCCAPS_SEPARATEALPHABLEND) == 0)
	//{
	//	return;
	//}

	m_bUseOffscreenDraw = bEnable;

	CXUI_Wnd::WND_LIST & kActiveList = XUIMgr.GetActiveList();

	CXUI_Wnd::WND_LIST::iterator kItor = kActiveList.begin();//visible 아닌거 없에준다
	while(kActiveList.end() != kItor)
	{
		CXUI_Wnd::WND_LIST::value_type &ekElement = (*kItor);
		if(ekElement->Visible() && ekElement->UseNewOffscreen())
		{
			ekElement->NewOffscreen(NULL);
		}
		++kItor;
	}

}
void	PgUIScene::UpdateOffscreen(PgRenderer *pkRenderer,float fFrameTime)
{
	if(!m_bUseOffscreenDraw)
	{
		return;
	}

	NiDX9Renderer *pDX9Renderer = (NiDX9Renderer*)pkRenderer->GetRenderer();
	NiDX9RenderState	*pkRenderState = pDX9Renderer->GetRenderState();

	pkRenderState->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE,true);
	pkRenderState->SetRenderState(D3DRS_SRCBLENDALPHA,D3DBLEND_ONE);
	pkRenderState->SetRenderState(D3DRS_DESTBLENDALPHA,D3DBLEND_ONE);

	m_kOffscreenDrawList.clear();

	CXUI_Wnd::WND_LIST & kActiveList = XUIMgr.GetActiveList();

	CXUI_Wnd::WND_LIST::iterator kItor = kActiveList.begin();//visible 아닌거 없에준다
	while(kActiveList.end() != kItor)
	{
		CXUI_Wnd::WND_LIST::value_type &ekElement = (*kItor);
		if(ekElement->Visible() && ekElement->UseNewOffscreen())
		{
			ekElement->NewOffscreen(NULL);

			stUIOffscreen *pkOffscreen = DrawUIToOffscreen(pkRenderer,ekElement);
			if(pkOffscreen)
			{
				ekElement->NewOffscreen((void*)pkOffscreen);
			}
			else
			{
				ekElement->UseNewOffscreen(false);
				ekElement->NewOffscreen(NULL);
			}
		}
		++kItor;
	}

	pkRenderState->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE,false);
}
stUIOffscreen*	PgUIScene::DrawUIToOffscreen(PgRenderer *pkRenderer,CXUI_Wnd *pkWnd)
{
	if(!pkWnd)
	{
		return NULL;
	}

	POINT	ptSize;
	ptSize.x = pkWnd->Size().x;
	ptSize.y = pkWnd->Size().y;

	if(ptSize.x == 0 || ptSize.y == 0)
	{
		return	NULL;
	}

	stUIOffscreen	*pkOffscreen = NULL;

	bool	bInvalidate = false;

	UIOffscreenMap::iterator itor = m_kOffscreenMap.find(pkWnd->ID());
	if(itor == m_kOffscreenMap.end())
	{
		pkOffscreen = new stUIOffscreen();
		bInvalidate = true;
		m_kOffscreenMap.insert(std::make_pair(pkWnd->ID(),pkOffscreen));
	}
	else
	{
		pkOffscreen = itor->second;
	}

	if(
#ifndef EXTERNAL_RELEASE
		lua_tinker::call<bool>("InvalidateUI") == true && 
#endif
		pkWnd->Invalidate())
	{
		bInvalidate = true;

		pkWnd->Invalidate(false);
	}

	POINT	ptTotalPos;
	ptTotalPos.x = pkWnd->TotalLocation().x;
	ptTotalPos.y = pkWnd->TotalLocation().y;

	if(!bInvalidate)
	{
		pkOffscreen->m_kPos = ptTotalPos;

		return	pkOffscreen;
	}

	pkWnd->Location(POINT2(0,0));

	NiRenderedTexturePtr	spRederedTexture = pkOffscreen->m_spRenderedTexture;
	if(!spRederedTexture || pkOffscreen->m_kSize.x != ptSize.x || pkOffscreen->m_kSize.y != ptSize.y)
	{
		NiTexture::FormatPrefs kPrefs;
		kPrefs.m_eAlphaFmt = NiTexture::FormatPrefs::SMOOTH;
		kPrefs.m_ePixelLayout = NiTexture::FormatPrefs::TRUE_COLOR_32;
		kPrefs.m_eMipMapped = NiTexture::FormatPrefs::NO;

		int	iTexSizeX=1,iTexSizeY=1;

		while(iTexSizeX<ptSize.x)
		{
			iTexSizeX = iTexSizeX<<1;
		}
		while(iTexSizeY<ptSize.y)
		{
			iTexSizeY = iTexSizeY<<1;
		}

		spRederedTexture = NiRenderedTexture::Create(iTexSizeX,iTexSizeY,pkRenderer->GetRenderer(),kPrefs);
		if(!spRederedTexture)
		{
			return	NULL;
		}

		pkOffscreen->m_spRenderedTexture = spRederedTexture;
	}

	pkOffscreen->m_kSize = ptSize;
	
	if(m_spRenderTargetGroup == 0)
	{
		m_spRenderTargetGroup = NiRenderTargetGroup::Create(1,pkRenderer->GetRenderer());
	}

	m_spRenderTargetGroup->AttachBuffer(spRederedTexture->GetBuffer(),0);

	pkWnd->VDisplay();
	pkWnd->VDisplayEnd();

	pkRenderer->GetRenderer()->SetBackgroundColor(NiColorA(0,0,0,0));

	pkRenderer->BeginUsingRenderTargetGroup(m_spRenderTargetGroup,NiRenderer::CLEAR_ALL);

	NiDX9Renderer *pDX9Renderer = (NiDX9Renderer*)pkRenderer->GetRenderer();
	NiDX9RenderState	*pkRenderState = pDX9Renderer->GetRenderState();

	//D3DBaseTexturePtr	pkD3DTexture = NULL;

	//NiDX9SourceTextureData* pkSourceTexData = static_cast<NiDX9SourceTextureData *>(spRederedTexture->GetRendererData());
	//if(pkSourceTexData)
	//{
	//	pkD3DTexture = pkSourceTexData->GetD3DTexture();
	//}
	//bool	bSave = false;

	int	iCount = 0;
	for(DrawList::iterator itr = m_kDrawList.begin();	itr != m_kDrawList.end(); ++itr)
	{
		PgUIDrawObject *pData = (*itr);

		if (pData == NULL)
			continue;

		if(!IsNullRect(pData->GetRect()) )
		{
			pkRenderState->SetRenderState(D3DRS_SCISSORTESTENABLE,true);
			pDX9Renderer->GetD3DDevice()->SetScissorRect(&pData->GetRect());//설정된 영역에서
			pData->Draw(pkRenderer);
			pkRenderState->SetRenderState(D3DRS_SCISSORTESTENABLE,false);
		}
		else
		{
			pData->Draw(pkRenderer);
		}
		pData->Release();//Pool 의 경우 리소스 해지

		//if(bSave && pkD3DTexture)
		//{
		//	char	filename[200];
		//	sprintf_s(filename,200,"SaveFile[%d].tga",iCount++);
		//	D3DXSaveTextureToFile(UNI(filename),D3DXIFF_TGA,pkD3DTexture,NULL);
		//}
		
	}
	
	pkRenderer->EndUsingRenderTargetGroup();


	ClearDrawLIst();

	pkWnd->Location(POINT2(ptTotalPos.x,ptTotalPos.y));

	pkOffscreen->m_kPos = ptTotalPos;
	return	pkOffscreen;

}
void	PgUIScene::DrawOffscreen(PgRenderer *pkRenderer,float fFrameTime)
{
	pkRenderer->BeginUsingDefaultRenderTargetGroup(NiRenderer::CLEAR_NONE);

	NiDX9Renderer *pDX9Renderer = (NiDX9Renderer*)pkRenderer->GetRenderer();
	NiDX9RenderState	*pkRenderState = pDX9Renderer->GetRenderState();

	for(UIOffscreenList::iterator itor = m_kOffscreenDrawList.begin(); itor != m_kOffscreenDrawList.end(); itor++)
	{
		stUIOffscreen *pkOffscreen = *itor;

		NiRenderedTexturePtr	spRenderedTexture = pkOffscreen->m_spRenderedTexture;
		if(!spRenderedTexture)
		{
			continue;
		}

		NiScreenTexturePtr spScreenTexture = pkOffscreen->m_spScreenTexture;
		if(!spScreenTexture)
		{
			spScreenTexture = NiNew NiScreenTexture(pkOffscreen->m_spRenderedTexture);
			spScreenTexture->SetApplyMode(NiTexturingProperty::APPLY_MODULATE);

			spScreenTexture->AddNewScreenRect(pkOffscreen->m_kPos.y,pkOffscreen->m_kPos.x,
				spRenderedTexture->GetWidth(),spRenderedTexture->GetHeight(),
				0,0,NiColorA::WHITE);

			pkOffscreen->m_spScreenTexture = spScreenTexture;
		}
		else
		{
			spScreenTexture->SetTexture(spRenderedTexture);
		}

		NiScreenTexture::ScreenRect const & kRect = spScreenTexture->GetScreenRect(0);

		if(kRect.m_sPixLeft != pkOffscreen->m_kPos.x ||
			kRect.m_sPixTop != pkOffscreen->m_kPos.y ||
			kRect.m_usPixWidth != pkOffscreen->m_kSize.x ||
			kRect.m_usPixHeight != pkOffscreen->m_kSize.y)
		{
			spScreenTexture->RemoveAllScreenRects();

			spScreenTexture->AddNewScreenRect(pkOffscreen->m_kPos.y,pkOffscreen->m_kPos.x,
				pkOffscreen->m_kSize.x,pkOffscreen->m_kSize.y,
				0,0,NiColorA::WHITE);

			spScreenTexture->MarkAsChanged(NiScreenTexture::EVERYTHING_MASK);
		}

		Render_UIObject(pkRenderer,spScreenTexture);
	}

	RenderImeInfo(pkRenderer,fFrameTime);

	pkRenderer->EndUsingRenderTargetGroup();
	
}
bool PgUIScene::RenderOffscreen(void* &pOffscreen)
{
	stUIOffscreen *pkOffscreen = (stUIOffscreen*)pOffscreen;

	AddToDrawList(pkOffscreen,true);

	return	true;
}
void	PgUIScene::ClearAllOffscreen()
{
	m_kOffscreenDrawList.clear();

	for(UIOffscreenMap::iterator itor = m_kOffscreenMap.begin(); itor != m_kOffscreenMap.end(); itor++)
	{
		stUIOffscreen	*pkOffScreen = itor->second;
		if(pkOffScreen)
		{
			pkOffScreen->m_spRenderedTexture = 0;
			pkOffScreen->m_spScreenTexture = 0;
		}
	}
	m_kOffscreenMap.clear();
}

void PgUIScene::Draw(PgRenderer *pkRenderer, float fFrameTime)
{
    PROFILE_FUNC();

	if (pkRenderer == NULL)
		return;

	m_bInsideDraw = true;

	if (g_bToggleUIDraw > 1)
	{

		Begin_RenderUIObject(pkRenderer);
		UpdateOffscreen(pkRenderer,fFrameTime);


#ifdef USE_BACK_BUFFER
		pkRenderer->BeginUsingRenderTargetGroup(m_spRenderTargetGroup, NiRenderer::CLEAR_ALL);
#else
		pkRenderer->BeginUsingDefaultRenderTargetGroup(NiRenderer::CLEAR_NONE);
#endif
		//NiRenderTargetGroup *pkOldTarget = 0;
		NiRenderer* pkNiRenderer = pkRenderer->GetRenderer();
		//if(pkRenderer->IsRenderTargetGroupActive())
		//{
		//	pkOldTarget = (NiRenderTargetGroup *)pkNiRenderer->GetCurrentRenderTargetGroup();
		//	pkRenderer->EndUsingRenderTargetGroup();
		//}

		pkRenderer->SetBackgroundColor(DEF_BG_COLORA);		
		pkRenderer->GetRenderer()->SetScreenSpaceCameraData();


		PG_STAT(PgStatTimerF timer(g_kUIStatGroup.GetStatInfo("Draw"), g_pkApp->GetFrameCount()));
		PG_STAT(PgStatTimerF timer1(g_kUIStatGroup.GetStatInfo("Draw.balloon"), g_pkApp->GetFrameCount()));
		PG_STAT(PgStatTimerF timer2(g_kUIStatGroup.GetStatInfo("Draw.VDisplay"), g_pkApp->GetFrameCount()));
		PG_STAT(PgStatTimerF timer6(g_kUIStatGroup.GetStatInfo("Draw.IME"), g_pkApp->GetFrameCount()));
		PG_STAT(PgStatTimerF timer7(g_kUIStatGroup.GetStatInfo("Draw.End"), g_pkApp->GetFrameCount()));

		NiCameraPtr spCamera = ( g_pkWorld ? g_pkWorld->GetCameraMan()->GetCamera() : NULL );

		PG_STAT(timer1.Start());

		//도움말 렌더링
		g_kQuestMarkHelpMng.DrawImmediate(pkRenderer, fFrameTime);
		g_kEnergyGaugeMan.DrawImmediate(pkRenderer);
		//파티 타이틀 렌더링
		g_kPartyBalloonMgr.DrawImmediate(pkRenderer, fFrameTime);
		//원정대 타이틀 렌더링
		g_kExpeditionBalloonMgr.DrawImmediate(pkRenderer, fFrameTime);
		//노점 타이틀 랜더링
		g_kVendorBalloonMgr.DrawImmediate(pkRenderer, fFrameTime);
		//	채팅 풍선 렌더링
		g_kBalloonMan2D.DrawImmediate( pkRenderer, spCamera, fFrameTime);
		// 마크 풍선 렌더링
		g_kMarkBalloonMan.DrawImmediate(pkRenderer, fFrameTime);
		g_kChainAttack.DrawImmediate(pkRenderer);
		g_kComboAdvisor.DrawImmediate(pkRenderer);
		g_kBalloonEmoticonMgr.DrawImmediate(pkRenderer);
		g_kScreenBreak.Draw(pkRenderer);
		g_kScreenEffect.Draw(pkRenderer);
		PG_STAT(timer1.Stop());

		m_kCursor.Draw(fFrameTime);

		PG_STAT(timer2.Start());
		XUIMgr.VDisplay();//여기서 좌표 변경들이 일어남.
		PG_STAT(timer2.Stop());

		NiDX9Renderer *pDX9Renderer = (NiDX9Renderer*)pkRenderer->GetRenderer();
		NiDX9RenderState	*pkRenderState = pDX9Renderer->GetRenderState();

		if (g_bToggleUIDraw > 2)
		{
#ifndef EXTERNAL_RELEASE
			if(lua_tinker::call<bool>("DrawUI"))
#endif
			{
				for(DrawList::iterator itr = m_kDrawList.begin();	itr != m_kDrawList.end(); ++itr)
				{
					PgUIDrawObject *pData = (*itr);

					if (pData == NULL)
						continue;

					if(!IsNullRect(pData->GetRect()) )
					{
						PG_STAT(PgStatTimerF timer3(g_kUIStatGroup.GetStatInfo("Draw.Scissor"), g_pkApp->GetFrameCount()));
						PG_STAT(timer3.Start());
						pkRenderState->SetRenderState(D3DRS_SCISSORTESTENABLE,true);
						pDX9Renderer->GetD3DDevice()->SetScissorRect(&pData->GetRect());//설정된 영역에서
						pData->Draw(pkRenderer);
						pkRenderState->SetRenderState(D3DRS_SCISSORTESTENABLE,false);
						PG_STAT(timer3.Stop());
					}
					else
					{
						PG_STAT(PgStatTimerF timer4(g_kUIStatGroup.GetStatInfo("Draw.Total"), g_pkApp->GetFrameCount()));
						PG_STAT(timer4.Start());
						pData->Draw(pkRenderer);
						PG_STAT(timer4.Stop());
					}
					PG_STAT(PgStatTimerF timer5(g_kUIStatGroup.GetStatInfo("Draw.Release"), g_pkApp->GetFrameCount()));

					PG_STAT(timer5.Start());
					pData->Release();//Pool 의 경우 리소스 해지
					PG_STAT(timer5.Stop());
				}
			}
		}
		
		End_RenderUIObject();

		//	IME 정보 렌더링
		PG_STAT(timer6.Start());
		RenderImeInfo(pkRenderer,fFrameTime);
		PG_STAT(timer6.Stop());
		PG_STAT(timer7.Start());
		pkRenderer->EndUsingRenderTargetGroup();
		PG_STAT(timer7.Stop());
		//pkRenderer->SetBackgroundColor(DEF_BG_COLOR);
		
		//if(pkOldTarget)
		//{
		//	pkRenderer->BeginUsingRenderTargetGroup(pkOldTarget, NiRenderer::CLEAR_NONE);
		//}
		//m_fLastUpdateTime = NiGetCurrentTimeInSec();
	}

#ifdef USE_BACK_BUFFER
	pkRenderer->BeginUsingDefaultRenderTargetGroup(NiRenderer::CLEAR_NONE);
	pkRenderer->GetRenderer()->SetScreenSpaceCameraData();
	m_spScreenTexture->Draw(pkRenderer->GetRenderer());
	pkRenderer->EndUsingRenderTargetGroup();
#endif

// EXTERNAL_RELEASE일때도 나오도록 수정.
//#if !defined(EXTERNAL_RELEASE)
	pkRenderer->BeginUsingDefaultRenderTargetGroup(NiRenderer::CLEAR_NONE);
	pkRenderer->GetRenderer()->SetScreenSpaceCameraData();
	if (g_pkApp->GetFrameRateEnabled() && g_pkApp->GetFrameRate())
	{
		g_pkApp->GetFrameRate()->GetElements()->Draw(pkRenderer->GetRenderer());
	}
	pkRenderer->EndUsingRenderTargetGroup();
//#endif
	m_bInsideDraw = false;

	ClearDrawLIst();
}
void	PgUIScene::Begin_RenderUIObject(PgRenderer *pkRenderer)
{
	//if(lua_tinker::call<bool>("UI_DRAW_DX") == false) return;

	NiDX9Renderer* pkDXRenderer = NiDynamicCast(NiDX9Renderer,pkRenderer->GetRenderer()); 

	LPDIRECT3DDEVICE9 pkDevice = pkDXRenderer->GetD3DDevice();
	if(!pkDevice) return;

	NiDX9RenderState	*pkRenderState = pkDXRenderer->GetRenderState();

	pkRenderState->SetPixelShader(NULL);
	pkRenderState->SetVertexShader(NULL);
	pkRenderState->SetFVF(D3DFVF_UIRECT);

	float	fConstant[] = {0.3f,0.59f,0.11f,1.0f};
	pkDevice->SetPixelShaderConstantF(0,fConstant,1);

	pkRenderState->SetRenderState(D3DRS_ALPHABLENDENABLE,true);
	pkRenderState->SetRenderState(D3DRS_ZENABLE,false);
	pkRenderState->SetRenderState(D3DRS_ZWRITEENABLE,false);
	pkRenderState->SetRenderState(D3DRS_FOGENABLE,false);
	pkRenderState->SetRenderState(D3DRS_COLORVERTEX,true);
	pkRenderState->SetRenderState(D3DRS_LIGHTING,false);
	pkRenderState->SetRenderState(D3DRS_ALPHATESTENABLE,false);
	pkRenderState->SetRenderState(D3DRS_CULLMODE,D3DCULL_CCW);

	pkRenderState->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_SRCALPHA);
	pkRenderState->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA);
	pkRenderState->SetRenderState(D3DRS_BLENDOP,D3DBLENDOP_ADD);

	pkRenderState->SetSamplerState(0,D3DSAMP_MINFILTER,D3DTEXF_LINEAR);
	pkRenderState->SetSamplerState(0,D3DSAMP_MAGFILTER,D3DTEXF_LINEAR);

	pkRenderState->SetTextureStageState(0,D3DTSS_COLOROP,D3DTOP_MODULATE);
	pkRenderState->SetTextureStageState(0,D3DTSS_COLORARG1,D3DTA_TEXTURE);
	pkRenderState->SetTextureStageState(0,D3DTSS_COLORARG2,D3DTA_DIFFUSE);

	pkRenderState->SetTextureStageState(0,D3DTSS_ALPHAOP,D3DTOP_MODULATE);
	pkRenderState->SetTextureStageState(0,D3DTSS_ALPHAARG1,D3DTA_TEXTURE);
	pkRenderState->SetTextureStageState(0,D3DTSS_ALPHAARG2,D3DTA_DIFFUSE);

	pkRenderState->SetRenderState(D3DRS_CLIPPING,false);

	int const iMaxTextureStage = 16;
	for(int i=0;i<iMaxTextureStage;i++)
	{
		pkRenderState->SetTexture(i,0);
	}
}
void	PgUIScene::End_RenderUIObject()
{
	//if(lua_tinker::call<bool>("UI_DRAW_DX") == false) return;
	NiRenderer *pkRenderer = NiRenderer::GetRenderer();
	if(!pkRenderer)
	{
		return;
	}

	//if(lua_tinker::call<bool>("UI_DRAW_DX") == false) return;
	NiDX9Renderer* pkDXRenderer = NiDynamicCast(NiDX9Renderer,pkRenderer->GetRenderer()); 

	LPDIRECT3DDEVICE9 pkDevice = pkDXRenderer->GetD3DDevice();
	if(!pkDevice) return;

	NiDX9RenderState	*pkRenderState = pkDXRenderer->GetRenderState();
	pkRenderState->SetRenderState(D3DRS_CLIPPING,true);
}
void	PgUIScene::Render_UIObject(PgRenderer *pkRenderer,NiObject *pkAVObject,DWORD const dwType,const stRenderOption &kRenderOption)
{
	if(!pkAVObject)
		return;

	NiDX9Renderer* pkDXRenderer = NiDynamicCast(NiDX9Renderer, pkRenderer->GetRenderer());

	NiDX9RenderState	*pkRenderState = pkDXRenderer->GetRenderState();
	
	float	fConstant[] = {0.0f,0.0f,0.0f,0.0f, 0.0f,0.0f,0.0f,0.0f};

	if (dwType & PgRenderer::E_PS_GRAY)
	{
		for (int i = 0; i<4; ++i)
		{
			fConstant[i] = 1.0f;
		}
	}

	if (dwType & PgRenderer::E_PS_HIGHLIGHT)
	{
		for (int i = 4; i<7; ++i)
		{
			fConstant[i] = 0.2f;
		}
	}
	
	LPDIRECT3DDEVICE9 pkDevice = pkDXRenderer->GetD3DDevice();
	if(pkDevice)
	{
		pkDevice->SetPixelShaderConstantF(1,fConstant,2);
	}

	pkRenderState->SetPixelShader( ( (dwType != PgRenderer::E_PS_NONE) ? pkRenderer->GetGrayScalePS() : NULL));

	if(NiIsKindOf(NiScreenTexture,pkAVObject))
	{
		Render_ScreenTexture(pkRenderer,(NiScreenTexture*)pkAVObject,kRenderOption);
	}
	else if(NiIsKindOf(NiScreenElements,pkAVObject))
	{
		Render_ScreenElements(pkRenderer,(NiScreenElements*)pkAVObject);
	}
}
void	PgUIScene::Pack_ScreenTexture(PgRenderer *pkRenderer,NiScreenTexture* pkScreenTexture,const stRenderOption &kRenderOption)
{
	PG_STAT(PgStatTimerF timer(g_kUIStatGroup.GetStatInfo("Pack_ScreenTexture"), g_pkApp->GetFrameCount()));
	if(!pkScreenTexture)
	{
		return;
	}
	NiDX9Renderer *pDX9Renderer = (NiDX9Renderer*)pkRenderer->GetRenderer();
	if(!pDX9Renderer)
	{
		return;
	}

	LPDIRECT3DDEVICE9 lpDevice = pDX9Renderer->GetD3DDevice();
	if(!lpDevice)
	{
		return;
	}

	PgScreenTextureRendererData	*pkRendererData = dynamic_cast<PgScreenTextureRendererData*>(pkScreenTexture->GetRendererData());
	if(!pkRendererData)
	{
		pkRendererData = NiNew PgScreenTextureRendererData();
		pkScreenTexture->SetRendererData(pkRendererData);
	}

	pkRendererData->SetRenderOption(kRenderOption);

	stFVF_UIRECT	*pkVertexBuffer = NULL;
	LPDIRECT3DVERTEXBUFFER9	lpD3DVertexBuffer = 0;

	float	fTexWidth=0.0,fTexHeight=0.0;
	float	fTx=0.0,fTy=0.0,fTw=0.0,fTh=0.0;
	NiScreenTexture::ScreenRect	kTargetRect;

	int	iVertCount = 0;
	

	D3DCOLOR	kDiffuseColor=0;
	D3DCOLOR	kOutlineDiffuseColor=D3DCOLOR_ARGB((int)(kRenderOption.m_kOutlineColor.a*255),(int)(kRenderOption.m_kOutlineColor.r*255),(int)(kRenderOption.m_kOutlineColor.g*255),(int)(kRenderOption.m_kOutlineColor.b*255));
	D3DCOLOR	kSpecularColor = D3DCOLOR_ARGB((int)(kRenderOption.m_kSpecular.a*255),(int)(kRenderOption.m_kSpecular.r*255),(int)(kRenderOption.m_kSpecular.g*255),(int)(kRenderOption.m_kSpecular.b*255));
	
	int	iNumRects = pkScreenTexture->GetNumScreenRects();
	if(iNumRects == 0)
	{
		pkRendererData->SetVertexBuffer(NULL);
		pkRendererData->SetTotalVerts(0);
		return;
	}

	int	iTotalVerts = (iNumRects-1)*6+4;
	if(kRenderOption.m_bExtendOutline)
	{
		iTotalVerts = (iNumRects-1)*30+28;
	}
	if(iTotalVerts != pkRendererData->GetTotalVerts())
	{
		int	iSize = sizeof(stFVF_UIRECT)*iTotalVerts;

		HRESULT hr = lpDevice->CreateVertexBuffer(iSize,0,D3DFVF_UIRECT,D3DPOOL_MANAGED,&lpD3DVertexBuffer,NULL);
		if(hr != D3D_OK)
		{
			return;
		}

		void	*pkLockBuf;
		hr = lpD3DVertexBuffer->Lock(0,0,&pkLockBuf,0);
		if(hr != D3D_OK)
		{
			return;
		}

		pkVertexBuffer = (stFVF_UIRECT*)pkLockBuf;

		pkRendererData->SetTotalVerts(iTotalVerts);
		pkRendererData->SetD3DVertexBuffer(lpD3DVertexBuffer);
	}
	else
	{
		lpD3DVertexBuffer = pkRendererData->GetD3DVertexBuffer();
		void	*pkLockBuf;
		HRESULT hr = lpD3DVertexBuffer->Lock(0,0,&pkLockBuf,0);
		if(hr != D3D_OK)
		{
			return;
		}
		pkVertexBuffer = (stFVF_UIRECT*)pkLockBuf;

	}

	const	int	iExtendLength = 1;

	for(int i=0;i<iNumRects ;i++)
	{
		NiScreenTexture::ScreenRect	&kScreenRect = pkScreenTexture->GetScreenRect(i);

		kDiffuseColor = D3DCOLOR_ARGB((int)(kScreenRect.m_kColor.a*255),(int)(kScreenRect.m_kColor.r*255),(int)(kScreenRect.m_kColor.g*255),(int)(kScreenRect.m_kColor.b*255));

		fTexWidth = (float)(pkScreenTexture->GetTexture()->GetWidth());
		fTexHeight = (float)(pkScreenTexture->GetTexture()->GetHeight());


		fTx = kScreenRect.m_usTexLeft/fTexWidth;
		fTy = kScreenRect.m_usTexTop/fTexHeight;
		fTw = kScreenRect.m_usPixWidth/fTexWidth;
		fTh = kScreenRect.m_usPixHeight/fTexHeight;

		int	iRectVertStart = iVertCount;
		if(i>0)
		{
			(pkVertexBuffer+iVertCount++)->p = (pkVertexBuffer+iVertCount-1)->p;
			iRectVertStart = iVertCount;

			if(kRenderOption.m_bExtendOutline)
			{
				(pkVertexBuffer+iVertCount++)->p = D3DXVECTOR4( iExtendLength-0.5f,-0.5f,0.0f, 1.0f );
			}
			else
			{
				(pkVertexBuffer+iVertCount++)->p = D3DXVECTOR4( -0.5f,-0.5f,0.0f, 1.0f );
			}

		}

		if(kRenderOption.m_bExtendOutline)
		{
			const	int	iStartVertex = iVertCount;

			//	왼쪽
			(pkVertexBuffer+iVertCount++)->p = D3DXVECTOR4( -iExtendLength-0.5f,-0.5f,0.0f, 1.0f );
			(pkVertexBuffer+iVertCount++)->p = D3DXVECTOR4( -iExtendLength-0.5f+kScreenRect.m_usPixWidth,-0.5f,0.0f, 1.0f );
			(pkVertexBuffer+iVertCount++)->p = D3DXVECTOR4( -iExtendLength-0.5f,-0.5f+kScreenRect.m_usPixHeight,0.0f, 1.0f );
			(pkVertexBuffer+iVertCount++)->p = D3DXVECTOR4( -iExtendLength-0.5f+kScreenRect.m_usPixWidth,-0.5f+kScreenRect.m_usPixHeight,0.0f, 1.0f );

			(pkVertexBuffer+iVertCount++)->p = (pkVertexBuffer+iVertCount-1)->p;

			//	오른쪽
			(pkVertexBuffer+iVertCount++)->p = D3DXVECTOR4( iExtendLength-0.5f,-0.5f,0.0f, 1.0f );
			(pkVertexBuffer+iVertCount++)->p = (pkVertexBuffer+iVertCount-1)->p;
			(pkVertexBuffer+iVertCount++)->p = D3DXVECTOR4( iExtendLength-0.5f+kScreenRect.m_usPixWidth,-0.5f,0.0f, 1.0f );
			(pkVertexBuffer+iVertCount++)->p = D3DXVECTOR4( iExtendLength-0.5f,-0.5f+kScreenRect.m_usPixHeight,0.0f, 1.0f );
			(pkVertexBuffer+iVertCount++)->p = D3DXVECTOR4( iExtendLength-0.5f+kScreenRect.m_usPixWidth,-0.5f+kScreenRect.m_usPixHeight,0.0f, 1.0f );

			(pkVertexBuffer+iVertCount++)->p = (pkVertexBuffer+iVertCount-1)->p;

			//	위쪽
			(pkVertexBuffer+iVertCount++)->p = D3DXVECTOR4( -0.5f,-iExtendLength-0.5f,0.0f, 1.0f );
			(pkVertexBuffer+iVertCount++)->p = (pkVertexBuffer+iVertCount-1)->p;
			(pkVertexBuffer+iVertCount++)->p = D3DXVECTOR4( -0.5f+kScreenRect.m_usPixWidth,-iExtendLength-0.5,0.0f, 1.0f );
			(pkVertexBuffer+iVertCount++)->p = D3DXVECTOR4( -0.5f,-iExtendLength-0.5f+kScreenRect.m_usPixHeight,0.0f, 1.0f );
			(pkVertexBuffer+iVertCount++)->p = D3DXVECTOR4( -0.5f+kScreenRect.m_usPixWidth,-iExtendLength-0.5f+kScreenRect.m_usPixHeight,0.0f, 1.0f );

			(pkVertexBuffer+iVertCount++)->p = (pkVertexBuffer+iVertCount-1)->p;

			//	아래쪽
			(pkVertexBuffer+iVertCount++)->p = D3DXVECTOR4( -0.5f,iExtendLength-0.5f,0.0f, 1.0f );
			(pkVertexBuffer+iVertCount++)->p = (pkVertexBuffer+iVertCount-1)->p;
			(pkVertexBuffer+iVertCount++)->p = D3DXVECTOR4( -0.5f+kScreenRect.m_usPixWidth,iExtendLength-0.5f,0.0f, 1.0f );
			(pkVertexBuffer+iVertCount++)->p = D3DXVECTOR4( -0.5f,iExtendLength-0.5f+kScreenRect.m_usPixHeight,0.0f, 1.0f );
			(pkVertexBuffer+iVertCount++)->p = D3DXVECTOR4( -0.5f+kScreenRect.m_usPixWidth,iExtendLength-0.5f+kScreenRect.m_usPixHeight,0.0f, 1.0f );

			(pkVertexBuffer+iVertCount++)->p = (pkVertexBuffer+iVertCount-1)->p;
			(pkVertexBuffer+iVertCount++)->p = D3DXVECTOR4( -0.5f,-0.5f,0.0f, 1.0f );

			for(int k=0;k<iVertCount/6;k++)
			{
				(pkVertexBuffer+iStartVertex+k*6+0)->kDiffuse = kOutlineDiffuseColor;
				(pkVertexBuffer+iStartVertex+k*6+1)->kDiffuse = kOutlineDiffuseColor;
				(pkVertexBuffer+iStartVertex+k*6+2)->kDiffuse = kOutlineDiffuseColor;
				(pkVertexBuffer+iStartVertex+k*6+3)->kDiffuse = kOutlineDiffuseColor;

				(pkVertexBuffer+iStartVertex+k*6+0)->t = D3DXVECTOR2( fTx, fTy );
				(pkVertexBuffer+iStartVertex+k*6+1)->t = D3DXVECTOR2( fTx+fTw, fTy );
				(pkVertexBuffer+iStartVertex+k*6+2)->t = D3DXVECTOR2( fTx, fTy+fTh );
				(pkVertexBuffer+iStartVertex+k*6+3)->t = D3DXVECTOR2( fTx+fTw, fTy+fTh );
			}
		}

		(pkVertexBuffer+iVertCount+0)->p = D3DXVECTOR4( -0.5f,-0.5f,0.0f, 1.0f );
		(pkVertexBuffer+iVertCount+1)->p = D3DXVECTOR4( -0.5f+kScreenRect.m_usPixWidth,-0.5f,0.0f, 1.0f );
		(pkVertexBuffer+iVertCount+2)->p = D3DXVECTOR4( -0.5f,-0.5f+kScreenRect.m_usPixHeight,0.0f, 1.0f );
		(pkVertexBuffer+iVertCount+3)->p = D3DXVECTOR4( -0.5f+kScreenRect.m_usPixWidth,-0.5f+kScreenRect.m_usPixHeight,0.0f, 1.0f );

		(pkVertexBuffer+iVertCount+0)->t = D3DXVECTOR2( fTx, fTy );
		(pkVertexBuffer+iVertCount+1)->t = D3DXVECTOR2( fTx+fTw, fTy );
		(pkVertexBuffer+iVertCount+2)->t = D3DXVECTOR2( fTx, fTy+fTh );
		(pkVertexBuffer+iVertCount+3)->t = D3DXVECTOR2( fTx+fTw, fTy+fTh );

		(pkVertexBuffer+iVertCount+0)->kDiffuse = kDiffuseColor;
		(pkVertexBuffer+iVertCount+1)->kDiffuse = kDiffuseColor;
		(pkVertexBuffer+iVertCount+2)->kDiffuse = kDiffuseColor;
		(pkVertexBuffer+iVertCount+3)->kDiffuse = kDiffuseColor;

		(pkVertexBuffer+iVertCount+0)->kSpecular = kSpecularColor;
		(pkVertexBuffer+iVertCount+1)->kSpecular = kSpecularColor;
		(pkVertexBuffer+iVertCount+2)->kSpecular = kSpecularColor;
		(pkVertexBuffer+iVertCount+3)->kSpecular = kSpecularColor;

		D3DXMATRIX	kTransformMat;
		D3DXQUATERNION	kQuatRot;

		D3DXVECTOR3 const kTranslation(static_cast<float>(kScreenRect.m_sPixLeft), static_cast<float>(kScreenRect.m_sPixTop), 0.0f );
		D3DXVECTOR2 const kScaleCenter(kScreenRect.m_usPixWidth*kRenderOption.m_kScaleCenter.x
									,kScreenRect.m_usPixHeight*kRenderOption.m_kScaleCenter.y);

		{// 회전 할만큼 만들어줄 행렬을 만들고
			if(0.0f == kRenderOption.m_fRotateAngle)
			{
				D3DXQuaternionIdentity(&kQuatRot);
			}
			else
			{
				static D3DXVECTOR3 const UNIT_Z(0.0f,0.0f,1.0f);
				D3DXQuaternionRotationAxis(&kQuatRot, &UNIT_Z, D3DXToRadian(kRenderOption.m_fRotateAngle));
			}
		}
		float const& fScale = kRenderOption.m_fScale;

		// Rotation의 중점을 설정하고
		D3DXVECTOR2 const kRotationCenter(kScreenRect.m_usPixWidth*kRenderOption.m_kRotationCenter.x
										 ,kScreenRect.m_usPixHeight*kRenderOption.m_kRotationCenter.y);
		
		// 위 값을 포함하는 행렬을 만들어
		D3DXMatrixTransformation(
			&kTransformMat,
			&D3DXVECTOR3(kScaleCenter.x, kScaleCenter.y,0.0f),			// 스케일 중심점
			NULL,
			&D3DXVECTOR3( fScale, fScale, 0.0f ),						// 스케일 값
			&D3DXVECTOR3(kRotationCenter.x, kRotationCenter.y, 0.0f),	// 회전 중심점
			&kQuatRot,
			&kTranslation
		);

		iVertCount+=4;

		int	iRectVertCount = iVertCount-iRectVertStart;
		for(int i=0;i<iRectVertCount;i++)
		{// 각 정점에 직접적으로 적용한다 (FVF가 RHW이므로)
			D3DXVec4Transform(&(pkVertexBuffer+iRectVertStart+i)->p,&(pkVertexBuffer+iRectVertStart+i)->p,&kTransformMat);
		}
	}

	if(lpD3DVertexBuffer)
	{
		lpD3DVertexBuffer->Unlock();
	}

}
void	PgUIScene::Render_ScreenTexture(PgRenderer *pkRenderer,NiScreenTexture* pkScreenTexture,const stRenderOption &kRenderOption)
{

	if(!pkScreenTexture)
	{
		return;
	}
	PgScreenTextureRendererData	*pkRendererData = dynamic_cast<PgScreenTextureRendererData*>(pkScreenTexture->GetRendererData());

	if(pkRendererData == NULL || pkScreenTexture->GetRevisionID()!=0 || pkRendererData->GetRenderOption().IsEqual(kRenderOption) == false)
	{
		Pack_ScreenTexture(pkRenderer,pkScreenTexture,kRenderOption);
		pkRendererData = dynamic_cast<PgScreenTextureRendererData*>(pkScreenTexture->GetRendererData());
	}

	pkScreenTexture->ClearRevisionID();

	if(!pkRendererData || pkScreenTexture->GetNumScreenRects() == 0)
	{
		return;
	}

	D3DBaseTexturePtr	pkD3DTexture = NULL;

	NiDX9Renderer* pkDXRenderer = NiDynamicCast(NiDX9Renderer, pkRenderer->GetRenderer());

	LPDIRECT3DDEVICE9 pkDevice = pkDXRenderer->GetD3DDevice();
	if(!pkDevice) 
	{
		return;
	}

	NiDX9RenderState	*pkRenderState = pkDXRenderer->GetRenderState();

	pkRenderState->SetRenderState(D3DRS_SPECULARENABLE,(kRenderOption.m_kSpecular != NiColorA::BLACK));

	if(pkScreenTexture->GetApplyMode() == NiTexturingProperty::APPLY_MODULATE)
	{
		pkRenderState->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
		pkRenderState->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	}
	else
	{
		pkRenderState->SetTextureStageState(0,D3DTSS_COLOROP,D3DTOP_SELECTARG1);
		pkRenderState->SetTextureStageState(0,D3DTSS_ALPHAOP,D3DTOP_SELECTARG1);
	}

	NiDX9SourceTextureData* pkSourceTexData = static_cast<NiDX9SourceTextureData *>(pkScreenTexture->GetTexture()->GetRendererData());
	if(!pkSourceTexData)
	{
		return;
	}
	
	pkD3DTexture = pkSourceTexData->GetD3DTexture();
	if(!pkD3DTexture)
	{
		return;
	}

	pkDevice->SetTexture(0,pkD3DTexture);
	pkDevice->SetStreamSource(0,pkRendererData->GetD3DVertexBuffer(),0,sizeof(stFVF_UIRECT));
	pkDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0,pkRendererData->GetTotalVerts()-2);
}

void	PgUIScene::Render_DynamicTexture(HDC kDC,int iScreenWidth,int iScreenHeight,PgDynamicTexture *pkDTexture,NiScreenTexture* pkScreenTexture,const stRenderOption &kRenderOption)
{
	if(!pkScreenTexture || !pkDTexture) return;

	int	iDX = pkScreenTexture->GetScreenRect(0).m_sPixLeft;
	int	iDY = pkScreenTexture->GetScreenRect(0).m_sPixTop;

	int	iTexTop = pkScreenTexture->GetScreenRect(0).m_usTexTop;
	int	iTexLeft = pkScreenTexture->GetScreenRect(0).m_usTexLeft;

	int	iWidth = pkScreenTexture->GetScreenRect(0).m_usPixWidth;
	int	iHeight = pkScreenTexture->GetScreenRect(0).m_usPixHeight;

	int	iTexWidth = pkDTexture->GetWidth();

	WORD	*pkBuf = (WORD*)pkDTexture->GetCopyData();
	if(!pkBuf)
	{
		return;
	}

	NiColorA	kColor = pkScreenTexture->GetScreenRect(0).m_kColor;

	for(int i=0;i<iHeight;i++)
	{
		for(int j=0;j<iWidth;j++)
		{
			WORD	wColor = *(pkBuf+(i+iTexTop)*iTexWidth+j+iTexLeft);
			
			float	fTexR = ((wColor&0x0f00)>>4)/256.0f;
			float	fTexG = ((wColor&0x00f0))/256.0f;
			float	fTexB = ((wColor&0x000f)<<4)/256.0f;

			int	iAlpha = (wColor&0xf000)>>8;
			int	iR = iAlpha*kColor.r*fTexR;
			int	iG = iAlpha*kColor.g*fTexG;
			int	iB = iAlpha*kColor.b*fTexB;
			SetPixel(kDC,iDX+j,iDY+i,RGB(iR,iG,iB));
		}
	}
}
#define PG_USE_ELE_ROTATE

void	PgUIScene::Render_ScreenElements(PgRenderer *pkRenderer,NiScreenElements* spScreenElements)
{
	/*if(lua_tinker::call<bool>("UI_DRAW_DX") == false) 
	{
		spScreenElements->RenderImmediate(NiRenderer::GetRenderer());
		return;
	}*/


	stFVF_UIRECT	kVertex[4];
	D3DBaseTexturePtr	pkD3DTexture;

	NiDX9Renderer* pkDXRenderer = NiDynamicCast(NiDX9Renderer, pkRenderer->GetRenderer());

	LPDIRECT3DDEVICE9 pkDevice = pkDXRenderer->GetD3DDevice();
	if(!pkDevice) return;

	float	fScreenWidth = (float)pkDXRenderer->GetDefaultRenderTargetGroup()->GetWidth(0);
	float	fScreenHeight = (float)pkDXRenderer->GetDefaultRenderTargetGroup()->GetHeight(0);

	NiDX9RenderState	*pkRenderState = pkDXRenderer->GetRenderState();


	D3DCOLOR	kDiffuseColor = 0;

	if(spScreenElements)
	{
		NiTexturingProperty	*pkTexturing = spScreenElements->GetPropertyState()->GetTexturing();


		if(pkTexturing && pkTexturing->GetBaseTexture())
		{
			if(pkTexturing->GetApplyMode() == NiTexturingProperty::APPLY_MODULATE)
			{
				pkRenderState->SetTextureStageState(0,D3DTSS_COLOROP,D3DTOP_MODULATE);
				pkRenderState->SetTextureStageState(0,D3DTSS_ALPHAOP,D3DTOP_MODULATE);
			}
			else
			{
				pkRenderState->SetTextureStageState(0,D3DTSS_COLOROP,D3DTOP_SELECTARG1);
				pkRenderState->SetTextureStageState(0,D3DTSS_ALPHAOP,D3DTOP_SELECTARG1);
			}


			if(pkTexturing->GetBaseClampMode() == NiTexturingProperty::CLAMP_S_CLAMP_T)
			{
				pkRenderState->SetSamplerState(0,D3DSAMP_ADDRESSU,D3DTADDRESS_CLAMP);
				pkRenderState->SetSamplerState(0,D3DSAMP_ADDRESSV,D3DTADDRESS_CLAMP);
			}
			else
			{
				pkRenderState->SetSamplerState(0,D3DSAMP_ADDRESSU,D3DTADDRESS_WRAP);
				pkRenderState->SetSamplerState(0,D3DSAMP_ADDRESSV,D3DTADDRESS_WRAP);
			}
		

			NiDX9SourceTextureData* pkSourceTexData = (NiDX9SourceTextureData *)pkTexturing->GetBaseTexture()->GetRendererData();
			
			if(pkSourceTexData)
			{
				//if(pkSourceTexData->IsRenderedTexture() || pkSourceTexData->IsDynamicTexture()) return;
				pkD3DTexture = pkSourceTexData->GetD3DTexture();

				if(pkD3DTexture)
				{
					const	NiTransform	&kTransform = spScreenElements->GetWorldTransform();

					float	fLeft,fTop,fWidth,fHeight;

					for (int iPoly=0 ; iPoly<spScreenElements->GetNumPolygons() ; iPoly++)
					{
						if (!spScreenElements->IsValid(iPoly))
						{
							continue;
						}

						NiPoint2	kTextures[4];
						spScreenElements->GetRectangle(iPoly,fLeft,fTop,fWidth,fHeight);
						spScreenElements->GetTextures(iPoly,0,kTextures[0],kTextures[1],kTextures[2],kTextures[3]);

						if (fWidth <= 0.0f || fHeight <= 0.0f)
						{
							continue;
						}

#ifdef PG_USE_ELE_ROTATE
						if (kTransform.m_Rotate != NiMatrix3::IDENTITY)
						{
							float fMaxWidth = fScreenWidth-0.5f;
							float fMaxHeight = fScreenHeight-0.5f;
							// 회전
							NiPoint3 kVerts[4];
							kVerts[0] = NiPoint3((kTransform.m_Translate.x+fLeft*kTransform.m_fScale)*fMaxWidth,(kTransform.m_Translate.y+fTop*kTransform.m_fScale)*fMaxHeight,0.0f);
							kVerts[1] = NiPoint3((kTransform.m_Translate.x+(fLeft+fWidth)*kTransform.m_fScale)*fMaxWidth,(kTransform.m_Translate.y+fTop*kTransform.m_fScale)*fMaxHeight,0.0f);
							kVerts[2] = NiPoint3((kTransform.m_Translate.x+fLeft*kTransform.m_fScale)*fMaxWidth,(kTransform.m_Translate.y+(fTop+fHeight)*kTransform.m_fScale)*fMaxHeight,0.0f);
							kVerts[3] = NiPoint3((kTransform.m_Translate.x+(fLeft+fWidth)*kTransform.m_fScale)*fMaxWidth,(kTransform.m_Translate.y+(fTop+fHeight)*kTransform.m_fScale)*fMaxHeight,0.0f);
							NiPoint3 kCenter;
							kCenter.x = ((fWidth*fMaxWidth) * 0.5f) + (fLeft*fMaxWidth);
							kCenter.y = ((fHeight*fMaxHeight) * 0.5f) + (fTop*fMaxHeight);
							kCenter.z = 0;
							//kCenter = (kVerts[0] + kVerts[1] + kVerts[2] + kVerts[3]) / 4;
							kVerts[0] = (kTransform.m_Rotate * (kVerts[0] - kCenter)) + kCenter;
							kVerts[1] = (kTransform.m_Rotate * (kVerts[1] - kCenter)) + kCenter;
							kVerts[2] = (kTransform.m_Rotate * (kVerts[2] - kCenter)) + kCenter;
							kVerts[3] = (kTransform.m_Rotate * (kVerts[3] - kCenter)) + kCenter;
		
							kVertex[0].p = D3DXVECTOR4( kVerts[0].x, kVerts[0].y, kVerts[0].z, 1.0f );
							kVertex[1].p = D3DXVECTOR4( kVerts[1].x, kVerts[1].y, kVerts[1].z, 1.0f );
							kVertex[2].p = D3DXVECTOR4( kVerts[2].x, kVerts[2].y, kVerts[2].z, 1.0f );
							kVertex[3].p = D3DXVECTOR4( kVerts[3].x, kVerts[3].y, kVerts[3].z, 1.0f );
						}
						else
						{
							kVertex[0].p = D3DXVECTOR4( (kTransform.m_Translate.x+fLeft*kTransform.m_fScale)*fScreenWidth-0.5f,(kTransform.m_Translate.y+fTop*kTransform.m_fScale)*fScreenHeight-0.5f,0.0f, 1.0f );
							kVertex[1].p = D3DXVECTOR4( (kTransform.m_Translate.x+(fLeft+fWidth)*kTransform.m_fScale)*fScreenWidth-0.5f,(kTransform.m_Translate.y+fTop*kTransform.m_fScale)*fScreenHeight-0.5f,0.0f, 1.0f );
							kVertex[2].p = D3DXVECTOR4( (kTransform.m_Translate.x+fLeft*kTransform.m_fScale)*fScreenWidth-0.5f,(kTransform.m_Translate.y+(fTop+fHeight)*kTransform.m_fScale)*fScreenHeight-0.5f,0.0f, 1.0f );
							kVertex[3].p = D3DXVECTOR4( (kTransform.m_Translate.x+(fLeft+fWidth)*kTransform.m_fScale)*fScreenWidth-0.5f,(kTransform.m_Translate.y+(fTop+fHeight)*kTransform.m_fScale)*fScreenHeight-0.5f,0.0f, 1.0f );
						}
#else
						kVertex[0].p = D3DXVECTOR4( (kTransform.m_Translate.x+fLeft*kTransform.m_fScale)*fScreenWidth-0.5f,(kTransform.m_Translate.y+fTop*kTransform.m_fScale)*fScreenHeight-0.5f,0.0f, 1.0f );
						kVertex[1].p = D3DXVECTOR4( (kTransform.m_Translate.x+(fLeft+fWidth)*kTransform.m_fScale)*fScreenWidth-0.5f,(kTransform.m_Translate.y+fTop*kTransform.m_fScale)*fScreenHeight-0.5f,0.0f, 1.0f );
						kVertex[2].p = D3DXVECTOR4( (kTransform.m_Translate.x+fLeft*kTransform.m_fScale)*fScreenWidth-0.5f,(kTransform.m_Translate.y+(fTop+fHeight)*kTransform.m_fScale)*fScreenHeight-0.5f,0.0f, 1.0f );
						kVertex[3].p = D3DXVECTOR4( (kTransform.m_Translate.x+(fLeft+fWidth)*kTransform.m_fScale)*fScreenWidth-0.5f,(kTransform.m_Translate.y+(fTop+fHeight)*kTransform.m_fScale)*fScreenHeight-0.5f,0.0f, 1.0f );
#endif

						kVertex[0].t = D3DXVECTOR2( kTextures[0].x, kTextures[0].y );
						kVertex[1].t = D3DXVECTOR2( kTextures[3].x, kTextures[3].y );
						kVertex[2].t = D3DXVECTOR2( kTextures[1].x, kTextures[1].y );
						kVertex[3].t = D3DXVECTOR2( kTextures[2].x, kTextures[2].y );

						NiColorA	kColor;
						if(spScreenElements->GetColor(iPoly,0, kColor) == false)
						{
							pkRenderState->SetTextureStageState(0,D3DTSS_COLOROP,D3DTOP_SELECTARG1);
							pkRenderState->SetTextureStageState(0,D3DTSS_ALPHAOP,D3DTOP_SELECTARG1);
						}
						kDiffuseColor = D3DCOLOR_ARGB((int)(kColor.a*255),(int)(kColor.r*255),(int)(kColor.g*255),(int)(kColor.b*255));
						kVertex[0].kDiffuse = kDiffuseColor;
						kVertex[1].kDiffuse = kDiffuseColor;
						kVertex[2].kDiffuse = kDiffuseColor;
						kVertex[3].kDiffuse = kDiffuseColor;

						pkDevice->SetTexture(0,pkD3DTexture);

						pkDevice->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 2, kVertex, sizeof( stFVF_UIRECT ) );
					}
				}
			}
		}
	}
}

bool PgUIScene::RenderText(const SRenderTextInfo &rRenderTextInfo)//찍기.
{
	PG_STAT(PgStatTimerF timer(g_kUIStatGroup.GetStatInfo("RenderText"), g_pkApp->GetFrameCount()));
//	if(!rRenderTextInfo.m_p2DString) return true;

	PgUITextPoolObject *pObject = 0;
	if(!m_kTextPool.size())
	{
		pObject = new PgUITextPoolObject(&g_kFontMgr, &m_kTextPool);
	}
	else
	{
		pObject = m_kTextPool.front();
		m_kTextPool.pop_front();
	}

	//pObject->SetData(rRenderTextInfo.kLoc, rRenderTextInfo.wstrText, rRenderTextInfo.dwColor,(CXUI_2DString*)rRenderTextInfo.m_p2DString);
	pObject->SetData(rRenderTextInfo.kLoc, rRenderTextInfo.wstrText, rRenderTextInfo.dwDiffuseColor, rRenderTextInfo.dwOutLineColor,(CXUI_2DString*)rRenderTextInfo.m_p2DString);
	pObject->SetRect(rRenderTextInfo.rcDrawable);
	pObject->SetFont(rRenderTextInfo.wstrFontKey);
	pObject->TextFlag(rRenderTextInfo.dwTextFlag);
	pObject->Alpha(rRenderTextInfo.fAlpha);


	AddToDrawList(pObject,true);
	return true;
}

bool PgUIScene::RenderSprite(void* &pSprite, int &iScrIndex, SRenderInfo const &rRenderInfo, bool bInsertList)
{
//	g_TempLog.Init(BM::OUTPUT_FILE_AND_TRACE, BM::LFC_WHITE);
//	g_TempLog.Log(BM::LOG_LV0, _T("RenderSprite %p"), (PgUIDrawObject*)pSprite);
//리무브 하지말고. 스케일을 0 으로 해주자.
//	스프라이트 객체는 각 UI 개체가 이용을 하네. 리소스 만들때 스스로 관리를 해버림. -_;; 썅..
	PG_STAT(PgStatTimerF timer(g_kUIStatGroup.GetStatInfo("RenderSprite"), g_pkApp->GetFrameCount()));
	SSizedScale const &rSS = rRenderInfo.kSizedScale;//찍힐 영역. --> 이게 찍힐 영역이면 안됨.
	SUVInfo const &rUVInfo = rRenderInfo.kUVInfo;//UV 짤린거.

	short nLocX = static_cast<short>(rRenderInfo.kLoc.x);
	short nLocY = static_cast<short>(rRenderInfo.kLoc.y);
	
	RECT const &rDrawable = rRenderInfo.rcDrawable;
	float rAlpha = rRenderInfo.fAlpha;

	long const lDrawX = abs(rSS.ptDrawSize.x);
	long const lDrawY = abs(rSS.ptDrawSize.y);

	WORD width = static_cast<WORD>(lDrawX);
	WORD height = static_cast<WORD>(lDrawY);

	WORD posU = (WORD)(((rUVInfo.Index-1) % rUVInfo.U) * rSS.ptSrcSize.x/rUVInfo.U);//u 가 가로
	WORD posV = (WORD)(((rUVInfo.Index-1) / rUVInfo.U) * rSS.ptSrcSize.y/rUVInfo.V);

	RECT const &rClip = rRenderInfo.rcClip;

	if( rClip.left > 0 )
	{
		width -= static_cast<WORD>(rClip.left);
		if( !width )
		{
			return false;
		}
		posU += static_cast<WORD>(rClip.left);
	}

	if( rClip.right > 0 )
	{
		width -= static_cast<WORD>(rClip.right);
		if( !width )
		{
			return false;
		}
	}

	if( rClip.top > 0 )
	{
		height -= static_cast<WORD>(rClip.top);
		if( !height )
		{
			return false;
		}

		posV += static_cast<WORD>(rClip.top);
	}

	if( rClip.bottom > 0 )
	{
		height -= static_cast<WORD>(rClip.bottom);
		if( !height )
		{
			return false;
		}
	}

	if ( 0 > rSS.ptDrawSize.x )
	{
		WORD const nValue = static_cast<WORD>(rSS.ptSrcSize.x - lDrawX);
		nLocX += static_cast<short>(nValue);
		posU += nValue;
	}

	if ( 0 > rSS.ptDrawSize.y )
	{
		WORD const nValue = static_cast<WORD>(rSS.ptSrcSize.y - lDrawY);
		nLocY += static_cast<short>(nValue);
		posV += nValue;
	}

	PgUISpriteObject *pkSpriteObj = (PgUISpriteObject *)pSprite;

	pkSpriteObj->SetGrayScale(rRenderInfo.bGrayScale);
	pkSpriteObj->SetHighLightScale(rRenderInfo.bTwinkle);

	NiScreenTexture *pkSprite = pkSpriteObj->GetTex();

	short nFlag = 0;

	if(iScrIndex == -2)
	{
		pkSprite->RemoveAllScreenRects();
		iScrIndex = -1;
	}

	if(iScrIndex == -1)
	{
		iScrIndex = pkSprite->AddNewScreenRect( nLocY, nLocX, width, height, posU, posV ); //
		nFlag = NiScreenTexture::VERTEX_MASK;
	}

	{
		NiScreenTexture::ScreenRect& kPanicRect = pkSprite->GetScreenRect(iScrIndex);

		if( kPanicRect.m_sPixTop	!= nLocY
		||	kPanicRect.m_sPixLeft	!= nLocX
		||	kPanicRect.m_usPixHeight != height
		||	kPanicRect.m_usPixWidth  != width)
		{
			kPanicRect.m_sPixTop	= nLocY;
			kPanicRect.m_sPixLeft	= nLocX;

			kPanicRect.m_usPixHeight = height;
			kPanicRect.m_usPixWidth  = width;
			nFlag |= NiScreenTexture::VERTEX_MASK;
			nFlag |= NiScreenTexture::TEXTURE_MASK;
		}

		if( kPanicRect.m_usTexTop != posV
		||	kPanicRect.m_usTexLeft!= posU)
		{
			kPanicRect.m_usTexTop	= posV;
			kPanicRect.m_usTexLeft	= posU;
			nFlag |= NiScreenTexture::VERTEX_MASK;
			nFlag |= NiScreenTexture::TEXTURE_MASK;
		}

		if (rAlpha < 0.0f)	{ rAlpha = 0.0f; }

		if (rAlpha > 1.0f)	{ rAlpha = 1.0f; }

		if(rAlpha <= 1.0f && kPanicRect.m_kColor.a != rAlpha)//1.0 보다 작으면 
		{
			kPanicRect.m_kColor.a = rAlpha;
			nFlag |= NiScreenTexture::COLOR_MASK;
		}

		if (rRenderInfo.bColorChange)
		{
			DWORD dw0x = 0x000000ff;
			kPanicRect.m_kColor.b = (rRenderInfo.dwColor & dw0x)/255.0f;
			kPanicRect.m_kColor.g = (rRenderInfo.dwColor>>8 & dw0x)/255.0f;
			kPanicRect.m_kColor.r = (rRenderInfo.dwColor>>16 & dw0x)/255.0f;
			nFlag |= NiScreenTexture::COLOR_MASK;
		}

		if(nFlag)
		{
			pkSprite->MarkAsChanged(nFlag);
		}
	}

	pkSpriteObj->SetRect(rDrawable);
	pkSpriteObj->Scale(rRenderInfo.fScale);
	pkSpriteObj->ScaleCenter(rRenderInfo.kScaleCenter);

	pkSpriteObj->RotationDeg(rRenderInfo.fRotationDeg);
	pkSpriteObj->RotationCenter(rRenderInfo.kRotationCenter);

	if( 0 == iScrIndex && bInsertList)
	{//0번은 이미 그리고 있을것이야.
		if(0 < width 
		&& 0 < height)
		{
			AddToDrawList(pkSpriteObj,true);
		}
	}
	return true;
}

//bool PgUIScene::ChangeOffscreenBgColor(void* &pOffscreen, DWORD bgColor, float fAlpha)
//{
//	if (pOffscreen == NULL)
//		return false;
//
//	PgUIRenderedSpriteObject* pScreen = (PgUIRenderedSpriteObject*)pOffscreen;	
//
//	// 위에서 생성한 텍스쳐를 이용해서 NiScreenTextrue를 만든다.
//	NiScreenTexture *pkSprite = pScreen->GetTex();
//	if (pkSprite == NULL)
//		return false;
//
//	NiScreenTexture::ScreenRect kRect = pkSprite->GetScreenRect(0);
//
//	DWORD dw0x = 0x000000ff;
//	NiColorA color = NiColorA((rRenderInfo.dwColor & dw0x)/255.0f, (rRenderInfo.dwColor>>8 & dw0x)/255.0f, (rRenderInfo.dwColor>>16 & dw0x)/255.0f, fAlpha);
//	kRect.m_kColor = color;
//
//	pkSprite->MarkAsChanged(NiScreenTexture::EVERYTHING_MASK);
//}

bool PgUIScene::InvalidateOffscreen(void* &pOffscreen)
{
	PG_STAT(PgStatTimerF timer1(g_kUIStatGroup.GetStatInfo("InvalidateOffscreen"), g_pkApp->GetFrameCount()));
	if (pOffscreen == NULL)
		return false;

	PgUIRenderedSpriteObject* pScreen = (PgUIRenderedSpriteObject*)pOffscreen;

	pScreen->SetDrawn(false);
	// TODO
	return true;
}

bool PgUIScene::InvalidateOffscreen(void* &pOffscreen, RECT& rectInvalidate)
{
	PG_STAT(PgStatTimerF timer1(g_kUIStatGroup.GetStatInfo("InvalidateOffscreen"), g_pkApp->GetFrameCount()));
	if (pOffscreen == NULL)
		return false;

	PgUIRenderedSpriteObject* pScreen = (PgUIRenderedSpriteObject*)pOffscreen;

	pScreen->SetDrawn(false);
	// TODO
	return true;
}

bool PgUIScene::EndRenderOffscreen(void* &pOffscreen, void** ppImages, int iNumImages, bool bClearOffscreen)
{
	PG_STAT(PgStatTimerF timer1(g_kUIStatGroup.GetStatInfo("EndRenderOffscreen"), g_pkApp->GetFrameCount()));
	if (pOffscreen == NULL)
		return false;

	if (ppImages == NULL)
		return false;

	if (iNumImages <= 0 && bClearOffscreen == false)	// Item이 없더라도 ClearOffscreen은 하자.
		return false;

	PgUIRenderedSpriteObject* pScreen = (PgUIRenderedSpriteObject*)pOffscreen;
	//if (pScreen->GetDrawn())
	//	return true;

	NiScreenTexture* pScreenTexture = (NiScreenTexture*)pScreen->GetTex();
	if (pScreenTexture == NULL)
		return false;

	NiRenderedTexture* pRenderedTexture = (NiRenderedTexture*)pScreenTexture->GetTexture();
	if (pRenderedTexture == NULL)
		return false;

	NiScreenTexture::ScreenRect rect = pScreenTexture->GetScreenRect(0);
	//PgOutputPrint3("[PgUIScene] EndRenderOffscreen %d image %dx%d offscreen\n", iNumImages, rect.m_usPixWidth, rect.m_usPixHeight);

	NiRenderer* pkRenderer = NiRenderer::GetRenderer();
	NiRenderTargetGroupPtr pTargetGroup = NiRenderTargetGroup::Create(pRenderedTexture->GetBuffer(), pkRenderer, false, false);

	NiRenderTargetGroup *pkOldTarget = 0;
	NiColorA kOldColor;
	if(pkRenderer->IsRenderTargetGroupActive())
	{
		pkOldTarget = (NiRenderTargetGroup *) pkRenderer->GetCurrentRenderTargetGroup();
		pkRenderer->GetBackgroundColor(kOldColor);
		pkRenderer->EndUsingRenderTargetGroup();
	}

	pkRenderer->SetBackgroundColor(NiColorA(0,0,0,0));
	pkRenderer->BeginUsingRenderTargetGroup(pTargetGroup, bClearOffscreen ? NiRenderer::CLEAR_ALL : NiRenderer::CLEAR_NONE);
	pkRenderer->SetScreenSpaceCameraData();

	for (int i = 0; i < iNumImages; i++)
	{
		PgUIDrawObject* pObj = (PgUIDrawObject*)ppImages[i];
		if (pObj)
		{
			NiTexturingProperty::ApplyMode oldMode = NiTexturingProperty::APPLY_MODULATE;
			NiScreenTexture* pTexture = pObj->GetTex();
			if (pTexture && pTexture->GetTexturingProperty())
			{
				oldMode = pTexture->GetTexturingProperty()->GetApplyMode();
				pTexture->GetTexturingProperty()->SetApplyMode(NiTexturingProperty::APPLY_REPLACE);
			}
			pObj->Draw(g_kRenderMan.GetRenderer());
			if (pTexture && pTexture->GetTexturingProperty())
			{
				pTexture->GetTexturingProperty()->SetApplyMode(oldMode);
			}
			pObj->Release();
		}
	}
	pkRenderer->EndUsingRenderTargetGroup();

	if(pkOldTarget)
	{
		pkRenderer->BeginUsingRenderTargetGroup(pkOldTarget, NiRenderer::CLEAR_NONE);
		pkRenderer->SetBackgroundColor(kOldColor);
	}

	pScreen->SetDrawn(true);
	return true;
}

bool PgUIScene::EndRenderOffscreen(void* &pOffscreen, std::list<std::pair<void*,bool>>& rkDrawList, bool bClearOffscreen)
{
	PG_STAT(PgStatTimerF timer1(g_kUIStatGroup.GetStatInfo("EndRenderOffscreen"), g_pkApp->GetFrameCount()));
	if (pOffscreen == NULL)
		return false;

	if (rkDrawList.size() <= 0 && bClearOffscreen == false)	// Item이 없더라도 ClearOffscreen은 하자.
		return false;

	PgUIRenderedSpriteObject* pScreen = (PgUIRenderedSpriteObject*)pOffscreen;
	//if (pScreen->GetDrawn() && bClearOffscreen)
	//	return true;

	NiScreenTexture* pScreenTexture = (NiScreenTexture*)pScreen->GetTex();
	if (pScreenTexture == NULL)
		return false;

	NiRenderedTexture* pRenderedTexture = (NiRenderedTexture*)pScreenTexture->GetTexture();
	if (pRenderedTexture == NULL)
		return false;

	NiScreenTexture::ScreenRect rect = pScreenTexture->GetScreenRect(0);
	//PgOutputPrint4("[PgUIScene] EndRenderOffscreen(list) %d image %dx%d offscreen(%d)\n", rkDrawList.size(), rect.m_usPixWidth, rect.m_usPixHeight, bClearOffscreen);
	NiRenderer* pkRenderer = NiRenderer::GetRenderer();
	NiRenderTargetGroupPtr pTargetGroup = NiRenderTargetGroup::Create(pRenderedTexture->GetBuffer(), pkRenderer, false, false);

	NiRenderTargetGroup *pkOldTarget = 0;
	NiColorA kOldColor;
	if(pkRenderer->IsRenderTargetGroupActive())
	{
		pkOldTarget = (NiRenderTargetGroup *) pkRenderer->GetCurrentRenderTargetGroup();
		pkRenderer->GetBackgroundColor(kOldColor);
		pkRenderer->EndUsingRenderTargetGroup();
	}

	DWORD dwColor = XUIMgr.GetColor(1);
	DWORD dw0x = 0x000000ff;
	NiColorA kColor(0,0,0,0);
	//kColor.b = (dwColor & dw0x)/255.0f;
	//kColor.g = (dwColor>>8 & dw0x)/255.0f;
	//kColor.r = (dwColor>>16 & dw0x)/255.0f;
	//kColor.a = 0.0f;

	pkRenderer->SetBackgroundColor(kColor);
	pkRenderer->BeginUsingRenderTargetGroup(pTargetGroup, bClearOffscreen ? NiRenderer::CLEAR_ALL : NiRenderer::CLEAR_NONE);
	pkRenderer->SetScreenSpaceCameraData();

	for (std::list<std::pair<void*,bool>>::iterator iter = rkDrawList.begin(); iter != rkDrawList.end(); ++iter)
	{
		PgUIDrawObject* pObj = (PgUIDrawObject*)(iter->first);
		if (pObj)
		{
			NiTexturingProperty::ApplyMode oldMode = NiTexturingProperty::APPLY_MODULATE;			
			NiScreenTexture* pTexture = pObj->GetTex();
			int iRectCount = pTexture->GetNumScreenRects();
			bool bReplace = false;

			if (pTexture && pTexture->GetTexturingProperty())
			{
				oldMode = pTexture->GetTexturingProperty()->GetApplyMode();
				if (iter->second)
					pTexture->GetTexturingProperty()->SetApplyMode(NiTexturingProperty::APPLY_REPLACE);
				else
					pTexture->GetTexturingProperty()->SetApplyMode(NiTexturingProperty::APPLY_MODULATE);
			}
			pObj->Draw(g_kRenderMan.GetRenderer());
			if (pTexture && pTexture->GetTexturingProperty())
			{
				pTexture->GetTexturingProperty()->SetApplyMode(oldMode);
			}
			pObj->Release();
		}
	}
	pkRenderer->EndUsingRenderTargetGroup();

	if(pkOldTarget)
	{
		pkRenderer->BeginUsingRenderTargetGroup(pkOldTarget, NiRenderer::CLEAR_NONE);
		pkRenderer->SetBackgroundColor(kOldColor);
	}

	pScreen->SetDrawn(true);
	return true;
}

bool PgUIScene::RenderSpriteToOffscreen(void* &pOffscreen, void* &pSprite, int &iScrIndex, const SRenderInfo &rRenderInfo)
{
	if (pOffscreen == NULL || pSprite == NULL)
		return false;
	
	RenderSprite(pSprite, iScrIndex, rRenderInfo, false);
	return true;
}

void PgUIScene::RecreateOffscreen()
{
	XUIMgr.VRefresh();	// QUESTION: 이 함수를 쓰는게 좋을까?

	SetUpdateEveryOffscreen();
	
	// 미니맵도 재생성 해준다.
	RecreateAllMiniMap(true);
}

void PgUIScene::UpdateQuest()
{
	PG_ASSERT_LOG(m_kMiniMapContainer.size() <= 2);
	MiniMapContainer::iterator itr = m_kMiniMapContainer.begin();
	while(itr != m_kMiniMapContainer.end())
	{
		MiniMapContainer::key_type const& kMiniMapName = itr->first;
		MiniMapContainer::mapped_type& pkMiniMap = itr->second;

		if( pkMiniMap )
		{
			switch( pkMiniMap->Type() )
			{
			case EMUT_ALWAYS_VIEW:
				{
					PgAlwaysMiniMap* pkAlwaysMiniMap = dynamic_cast<PgAlwaysMiniMap*>(pkMiniMap);
					if( pkAlwaysMiniMap )
					{
						pkAlwaysMiniMap->UpdateQuest();
					}
				}break;
			}
		}
		++itr;
	}
}

void PgUIScene::RecreateAllMiniMap(bool bResetAll)
{ 
	if (bResetAll)
	{
	 	MiniMapContainer::iterator iter = m_kMiniMapContainer.begin();
		while(iter != m_kMiniMapContainer.end())
		{
			MiniMapContainer::key_type const& kMiniMapName = iter->first;
			MiniMapContainer::mapped_type& pkMiniMap = iter->second;

			if (pkMiniMap != NULL)
			{
				pkMiniMap->Terminate();
			}

			++iter;
		}

		PgMiniMapBase::ReleaseAll();	//  bResetAll is set when device changed. so release all saved images
	}
 	MiniMapContainer::iterator itr = m_kMiniMapContainer.begin();
	while(itr != m_kMiniMapContainer.end())
	{
		MiniMapContainer::key_type const& kMiniMapName = itr->first;
		MiniMapContainer::mapped_type& pkMiniMap = itr->second;
		if (pkMiniMap != NULL)
		{
			PgWorld* world = g_kPilotMan.GetPlayerWorld();
			if (world)
			{
				world->SetWorldDataForMiniMap(kMiniMapName, pkMiniMap->WndSize());
			}
		}

		++itr;
	}
}

void PgUIScene::RecreateMiniMap(std::string strMiniMapName, POINT2 const& WndSize)
{
	MiniMapContainer::iterator itr = m_kMiniMapContainer.find(strMiniMapName);
	if(itr == m_kMiniMapContainer.end())
	{
		return;
	}

	MiniMapContainer::key_type const& kMiniMapName = itr->first;
	MiniMapContainer::mapped_type& pkMiniMap = itr->second;

	if (pkMiniMap == NULL)
		return;

	PgWorld* world = g_kPilotMan.GetPlayerWorld();
	if (world)
		world->SetWorldDataForMiniMap(kMiniMapName, WndSize);
}

PgMiniMapBase* PgUIScene::GetMiniMapUI(std::string strMiniMapName)
{
	MiniMapContainer::iterator itr = m_kMiniMapContainer.find(strMiniMapName);
	if(itr != m_kMiniMapContainer.end())
	{
		MiniMapContainer::mapped_type& pkMiniMap = itr->second;
		return pkMiniMap;
	}
	return NULL;
}

void PgUIScene::UpdateMinimapQuest()
{
	MiniMapContainer::iterator itr = m_kMiniMapContainer.begin();
	while( m_kMiniMapContainer.end() != itr )
	{
		MiniMapContainer::mapped_type& pkMiniMap = itr->second;
		if( pkMiniMap )
		{
			pkMiniMap->UpdateQuest();
		}
		++itr;
	}
}

void PgUIScene::TerminateAllMiniMap()
{
	MiniMapContainer::iterator itr = m_kMiniMapContainer.begin();
	while(itr != m_kMiniMapContainer.end())
	{
		MiniMapContainer::key_type const& kMiniMapName = itr->first;
		MiniMapContainer::mapped_type& pkMiniMap = itr->second;
		if (pkMiniMap)
		{
			pkMiniMap->Terminate();
		}
		++itr;
	}
	//m_kMiniMapContainer.clear(); //PgMiniMap instace를 재활용 하기 때문에 지우지 않는다. delete도 안함.
}

void PgUIScene::TerminateMiniMap(std::string strMiniMapName)
{
	MiniMapContainer::iterator itr = m_kMiniMapContainer.find(strMiniMapName);
	if(itr == m_kMiniMapContainer.end())
	{
		return;
	}

	MiniMapContainer::key_type const& kMiniMapName = itr->first;
	MiniMapContainer::mapped_type& pkMiniMap = itr->second;

	if (pkMiniMap == NULL)
	{
		return;
	}

	pkMiniMap->Terminate();

	//m_kMiniMapContainer.erase(itr); //PgMiniMap instace를 재활용 하기 때문에 지우지 않는다. delete도 안함.
}

bool PgUIScene::InitMiniMap(std::string const& strMiniMapName, const POINT2 &ptWndSize, const POINT2 &ptWndPos, int const InitType)
{
	PgMiniMapBase* pkMiniMap = NULL;
	switch( InitType )
	{
	case EMUT_ALWAYS_VIEW:
		{
			pkMiniMap = new PgAlwaysMiniMap;
		}break;
	}

	if (pkMiniMap == NULL)
	{
		return false;
	}

	pkMiniMap->WindowPos(ptWndPos);
	MiniMapContainer::iterator itr = m_kMiniMapContainer.find(strMiniMapName);
	if(itr != m_kMiniMapContainer.end())
	{
		MiniMapContainer::mapped_type& pkOldMiniMap = itr->second;
		if (pkOldMiniMap)
		{
			pkOldMiniMap->Terminate();
			SAFE_DELETE(pkOldMiniMap);
		}
		(*itr).second = pkMiniMap;
	}
	else
	{
		m_kMiniMapContainer.insert(std::make_pair(strMiniMapName, pkMiniMap));
	}


	switch( InitType )
	{
	case EMUT_ALWAYS_VIEW:
		{
			PgWorld* world = g_kPilotMan.GetPlayerWorld();
			if (world)
			{
				world->SetWorldDataForMiniMap(strMiniMapName, ptWndSize);
			}
		}break;
	}

	return true;
}

bool PgUIScene::SetWorldDataForMiniMap(std::string strMiniMapName, PgWorld::ObjectContainer* pObjectContainter, PgWorld::TriggerContainer* pTriggerContainer, NiCameraPtr worldCamera, std::string& kMiniMapImage, POINT2 const& WndSize, POINT* pDrawHeight)
{
	MiniMapContainer::iterator itr = m_kMiniMapContainer.find(strMiniMapName);
	if(itr == m_kMiniMapContainer.end())
	{
		return false;
	}

	MiniMapContainer::mapped_type& pkMiniMap = itr->second;
	if (pkMiniMap == NULL)
		return false;

	PgAlwaysMiniMap* pkAlwaysMiniMap = dynamic_cast<PgAlwaysMiniMap*>(pkMiniMap);
	if( pkAlwaysMiniMap == NULL )
		return false;

	if (pkAlwaysMiniMap == NULL || pObjectContainter == NULL || pTriggerContainer == NULL || pDrawHeight == NULL)
		return false;

	PgAlwaysMiniMap::S_INIT_INFO	kInitInfo;
	kInitInfo.kUISize = WndSize;
	kInitInfo.pkCamera = worldCamera;
	kInitInfo.kImgPath = kMiniMapImage;
	kInitInfo.pkHeightInfo = pDrawHeight;
	kInitInfo.pkObjectCont = pObjectContainter;
	kInitInfo.pkTriggerCont = pTriggerContainer;

	return pkAlwaysMiniMap->Initialize(kInitInfo);
}

bool PgUIScene::SetWorldDataForAllMiniMap(PgWorld::ObjectContainer* pObjectContainter, PgWorld::TriggerContainer* pTriggerContainer, NiCameraPtr worldCamera, std::string& kMiniMapImage, POINT* pDrawHeight)
{
	MiniMapContainer::iterator itr = m_kMiniMapContainer.begin();
	while(itr != m_kMiniMapContainer.end())
	{
		MiniMapContainer::mapped_type& pkMiniMap = itr->second;
		++itr;
		if( pkMiniMap )
		{
			PgAlwaysMiniMap* pkAlwaysMiniMap = dynamic_cast<PgAlwaysMiniMap*>(pkMiniMap);
			if( pkAlwaysMiniMap == NULL )
			{
				continue;
			}

			PgAlwaysMiniMap::S_INIT_INFO	kInitInfo;
			kInitInfo.kUISize = pkMiniMap->WndSize();
			kInitInfo.pkCamera = worldCamera;
			kInitInfo.kImgPath = kMiniMapImage;
			kInitInfo.pkHeightInfo = pDrawHeight;
			kInitInfo.pkObjectCont = pObjectContainter;
			kInitInfo.pkTriggerCont = pTriggerContainer;

			pkAlwaysMiniMap->Initialize(kInitInfo);
		}
	}

	return true;
}
void PgUIScene::SetRefreshAllMiniMap()
{
	MiniMapContainer::iterator itr = m_kMiniMapContainer.begin();
	while(itr != m_kMiniMapContainer.end())
	{
		MiniMapContainer::mapped_type& pkMiniMap = itr->second;
		++itr;
		if( pkMiniMap )
		{
			pkMiniMap->SetRefreshAllMiniMapIconInfo(true);
		}
	}
}
bool PgUIScene::RenderMiniMap(std::string const& strMiniMapName, const POINT2 &ptWndPos)
{
	MiniMapContainer::iterator itr = m_kMiniMapContainer.find(strMiniMapName);
	if(itr == m_kMiniMapContainer.end())
	{
		return false;
	}

	MiniMapContainer::mapped_type& pkMiniMap = itr->second;
	if (pkMiniMap == NULL)
	{
		return false;
	}

	PG_STAT(PgStatTimerF timer(g_kUIStatGroup.GetStatInfo("RenderMiniMap"), g_pkApp->GetFrameCount()));
 
	//pkMiniMap->Update(0.0f, 0.0f);
	pkMiniMap->RenderFrame(NiRenderer::GetRenderer(), ptWndPos);
	AddToDrawList(pkMiniMap,true);

	return true;
}

bool PgUIScene::RenderMiniMapUI(std::string const& strMiniMapName, XUI::CXUI_Wnd *pWnd)
{
	MiniMapContainer::iterator itr = m_kMiniMapContainer.find(strMiniMapName);
	if(itr == m_kMiniMapContainer.end())
	{
		return false;
	}

	MiniMapContainer::mapped_type& pkMiniMap = itr->second;
	if (pkMiniMap == NULL)
	{
		return false;
	}

	pkMiniMap->RenderFrameUI(pWnd);
	return true;
}

bool PgUIScene::SetMiniMapTriggerIconUI(std::string strMiniMapName, XUI::CXUI_Wnd *pWnd, const POINT2 &ptWndPos)
{
	if( !pWnd )
	{
		return false;
	}
	
	MiniMapContainer::iterator itr = m_kMiniMapContainer.find(strMiniMapName);
	if(itr == m_kMiniMapContainer.end())
	{
		return false;
	}

	MiniMapContainer::mapped_type& pkMiniMap = itr->second;
	if (pkMiniMap == NULL)
	{
		return false;
	}
 
	pkMiniMap->SetTriggerIconUI(pWnd, ptWndPos);
	return true;
}

bool PgUIScene::ShowMiniMap(std::string strMiniMapName, bool bShow)
{
	MiniMapContainer::iterator itr = m_kMiniMapContainer.find(strMiniMapName);
	if(itr == m_kMiniMapContainer.end())
	{
		return false;
	}

	MiniMapContainer::mapped_type& pkMiniMap = itr->second;
	if (pkMiniMap == NULL)
	{
		return false;
	}

	pkMiniMap->ShowMiniMap(bShow);
	return true;
}

bool PgUIScene::CloseMiniMap(std::string strMiniMapName)
{
	MiniMapContainer::iterator itr = m_kMiniMapContainer.find(strMiniMapName);
	if(itr == m_kMiniMapContainer.end())
	{
		return false;
	}

	MiniMapContainer::mapped_type& pkMiniMap = itr->second;
	if (pkMiniMap == NULL)
	{
		return false;
	}

	pkMiniMap->Close();	
	return true;
}

bool PgUIScene::SetWndSize(std::string strMiniMapName, POINT2 kWndSize)
{
	MiniMapContainer::iterator itr = m_kMiniMapContainer.find(strMiniMapName);
	if(itr == m_kMiniMapContainer.end())
	{
		return false;
	}

	MiniMapContainer::mapped_type& pkMiniMap = itr->second;
	if (pkMiniMap == NULL)
	{
		return false;
	}

	//pkMiniMap->SetWndSize(kWndSize);
	return true;
}

PgMiniMapBase* PgUIScene::CloneMiniMap(std::string strSrcMiniMap, std::string strDestMiniMap, POINT2 kWndSize)
{
	//MiniMapContainer::iterator itr_src = m_kMiniMapContainer.find(strSrcMiniMap);
	//MiniMapContainer::iterator itr_dest = m_kMiniMapContainer.find(strDestMiniMap);
	//if(itr_src == m_kMiniMapContainer.end() || itr_dest !=  m_kMiniMapContainer.end())
	//{
	//	return NULL;
	//}

	//MiniMapContainer::mapped_type& pkSrcMiniMap = itr->second;
	//if (!pkSrcMiniMap)
	//{
	//	return NULL;
	//}

	//PgMiniMapUI *pkDestMiniMap = new PgMiniMapUI();
	//if (!pkDestMiniMap)
	//{
	//	return NULL;
	//}

	//pkDestMiniMap->CloneFromSrc(pkSrcMiniMap);
	//m_kMiniMapContainer.insert(std::make_pair(strDestMiniMap, pkDestMiniMap));
	//// TODO : Clone 해서 Dest 를 Insert 해야함.

	return NULL;//pkUIDestMiniMap;
}

bool PgUIScene::RemoveMiniMapIcon(BM::GUID const & rkGuid, std::string const & strMiniMapName)
{
	if(strMiniMapName.empty())
	{
		for(MiniMapContainer::iterator itr = m_kMiniMapContainer.begin(); itr!=m_kMiniMapContainer.end(); ++itr)
		{
			if(itr->second)
			{
				itr->second->RemoveMiniMapIcon(rkGuid);
			}
		}
	}
	else
	{
		MiniMapContainer::iterator itr = m_kMiniMapContainer.find(strMiniMapName);
		if(itr == m_kMiniMapContainer.end() || !itr->second)
		{
			return false;
		}

		itr->second->RemoveMiniMapIcon(rkGuid);
	}
	return true;
}

bool PgUIScene::InitRenderModel(std::string const &strRenderModel, const POINT2 &ptWndSize, const POINT2 &ptWndPos, bool bUpdate, bool bOrtho)
{
	UIModelContainer::iterator itr = m_kUIModelContainer.find(strRenderModel);
	
	if(itr == m_kUIModelContainer.end())
	{
		PgUIModel *pNewModel = PgUIModel::Create(ptWndSize, bUpdate, bOrtho);
		if(!pNewModel)
		{
			_PgOutputDebugString("PgUIScene::InitRenderModel - Create failed.\n");
			return false;
		}

		// 텍스쳐가 갱신되어야 하기 때문에 PgUIModel은 업데이트 리스트에 넣어 주어야 한다.
		std::pair<UIModelContainer::iterator, bool> kRet = m_kUIModelContainer.insert(std::make_pair(strRenderModel, pNewModel));
		if(!kRet.second)
		{
			_PgOutputDebugString("PgUIScene::InitRenderModel - UIModelContainer insert failed.\n");
			return false;
		}
		itr = kRet.first;
	}
	else
	{
		PgUIModel *pkModel = itr->second;
		if( !pkModel )
		{
			return false;
		}
		pkModel->SetEnableUpdate(bUpdate);
	}
	return true;
}

void PgUIScene::AddToDrawListRenderModel(std::string const &strRenderModel)
{
	PgUIModel *pkModel = FindUIModel(strRenderModel);
	if(pkModel)
	{
		AddToDrawList(pkModel,true);
	}
}

/*
bool PgUIScene::AddRenderModelActor(std::string const &strRenderModel, std::string const &strActorName, NiActorManager *pNiActor, bool bClone, bool bCameraReset)
{
	PgUIModel *pkModel = FindUIModel(strRenderModel);
	if(pkModel)
	{
		pkModel->AddActor(bAutoRotate, fXAxis, fYAxis, fZAxis);
	}
	return false;
}
bool PgUIScene::AddRenderModelNIF(std::string const &strRenderModel, std::string const &strNIFName, NiNode *pNiNode, bool bClone, bool bCameraReset)
{
	return false;
}
*/
PgUIModel* PgUIScene::FindUIModel(std::string const &strRenderModel)
{
	UIModelContainer::iterator itr = m_kUIModelContainer.find(strRenderModel);

	if(itr != m_kUIModelContainer.end())
	{
		PgUIModel *pkModel = itr->second;
		if(!pkModel)
		{
			return NULL;
		}

		return pkModel;
	}	

	return NULL;
}

//void PgUIScene::ZoomInMiniMap(int zoomLevel)
//{
//	if (!m_pUIMiniMap)
//		return;
//
//	m_pUIMiniMap->ZoomInMiniMap(zoomLevel);
//}
//
//void PgUIScene::ZoomOutMiniMap(int zoomLevel)
//{
//	if (!m_pUIMiniMap)
//		return;
//
//	m_pUIMiniMap->ZoomOutMiniMap(zoomLevel);
//}
//
//void PgUIScene::ScrollMiniMap(NiPoint2 direction)
//{
//	if (!m_pUIMiniMap)
//		return;
//
//	m_pUIMiniMap->ScrollMiniMap(direction);
//}
bool PgUIScene::RenderStaticForm(void *&pkStaticForm, const SRenderInfo &rRenderInfo, bool bInsertList)
{

	PgUIStaticFormDrawObject	*pkNewStaticFormObject = (PgUIStaticFormDrawObject*)pkStaticForm;
	pkNewStaticFormObject->SetRenderInfo(rRenderInfo);
	AddToDrawList(pkNewStaticFormObject,true);

	return	true;
}

bool PgUIScene::RenderModel(char const *pcModelName, const POINT2 &ptWndSize, const POINT2 &ptWndPos, NiActorManager *pkActorManager, NiAVObject *pkModelRoot)
{
	std::string kModelName(pcModelName);
	UIModelContainer::iterator itr = m_kUIModelContainer.find(kModelName);
	
	if(itr == m_kUIModelContainer.end())
	{
//		PgUIModel *pNewModel = PgUIModel::Create(pkActorManager, pkModelRoot, ptWndSize);
//		if(!pNewModel)
		{
			return false;
		}

//		std::pair<UIModelContainer::iterator, bool> kRet = m_kUIModelContainer.insert(std::make_pair(kModelName, pNewModel));
//		if(!kRet.second)
		{
			return false;
		}
//		itr = kRet.first;
	}

	PgUIModel *pkModel = itr->second;
	if(!pkModel)
	{
		return 0;
	}

//	if(pkModelRoot)
	{
//		pkModel->UpdateModel(pkModelRoot);
	}
	
	pkModel->RenderFrame(NiRenderer::GetRenderer(), ptWndPos);
	AddToDrawList(pkModel,true);
	_PgOutputDebugString("PgUIScene::RenderModel\n");

	return true;
}

PgUIModel *PgUIScene::GetRenderModel(char const *pcModelName)
{
	std::string kModelName(pcModelName);
	UIModelContainer::iterator itr = m_kUIModelContainer.find(kModelName);
	
	if(itr == m_kUIModelContainer.end())
	{
		return 0;
	}

	return itr->second;
}

bool PgUIScene::VPrepareResource(std::wstring const &rKey)
{
	if (rKey.empty())
		return NULL;

	PG_STAT(PgStatTimerF timer(g_kUIStatGroup.GetStatInfo("VPrepareResource"), g_pkApp->GetFrameCount()));
	// 새 텍스쳐 리소스를 만든다.
	//unsigned int iLevel = NiRenderer::GetRenderer()->GetMipmapSkipLevel();
	//if (iLevel > 0)
	//{
	//	NiRenderer::GetRenderer()->SetMipmapSkipLevel(0);
	//}
	NiSourceTexture* pkTexture = g_kNifMan.GetTexture(MB(rKey));
	//if (iLevel > 0)
	//{
	//	NiRenderer::GetRenderer()->SetMipmapSkipLevel(iLevel);
	//}
	if (!pkTexture)
	{
		PgError1("%s texture can't load\n", MB(rKey));
		return false;
	}
	return true;
}

void* PgUIScene::VCreateIconResource(int const IconNo, RECT rectIcon,bool bGrayScale)
{
	PG_ASSERT_LOG(IconNo > 0);
	if (IconNo <= 0)
		return NULL;

	PG_STAT(PgStatTimerF timer(g_kUIStatGroup.GetStatInfo("VCreateIconResource"), g_pkApp->GetFrameCount()));
	CONT_IMG::const_iterator img_itor = m_mapImg.find(IconNo);
	if(m_mapImg.end() == img_itor)
	{
		if(true==AddIconRsc(IconNo))
		{
			img_itor = m_mapImg.find(IconNo);
		}
	}

	if(img_itor != m_mapImg.end())
	{
		//////////////////////////////////////////////
		PgUISpritePoolObject::SprObjlist::value_type pObject = 0;
		SpritePool::iterator pool_itor =  m_kSpritePool.find(IconNo);

		if(pool_itor == m_kSpritePool.end())
		{
			std::pair<SpritePool::iterator, bool> ret = m_kSpritePool.insert(std::make_pair(IconNo, new PgUISpritePoolObject::SprObjlist ));

			if(ret.second)
			{
				pool_itor = ret.first;
			}
			else
			{
				PG_ASSERT_LOG(0);
			}
		}

		if(pool_itor != m_kSpritePool.end())
		{//찾았을 경우다.
			SpritePool::mapped_type pList =  (*pool_itor).second;

			if(pList->size())
			{
				pObject = pList->front();
				pList->pop_front();
			}
			else
			{//새로 만들었는데 풀에 넣으면 안됨. 이건 풀이니까.
				NiObject *pClonedSprite = (*img_itor).second->GetTex()->Clone();
				pObject = new PgUISpritePoolObject((NiScreenTexture*)pClonedSprite, pList);
			}

			if(!pObject)
			{
				PG_ASSERT_LOG(0);
			}
		}

		if (pObject)
		{
			NiScreenTexture *pTex = pObject->GetTex();
			pTex->RemoveAllScreenRects();
			pObject->SetGrayScale(bGrayScale);

			PgUIUVSpriteObject* pkSpriteObj = static_cast<PgUIUVSpriteObject*>( (*img_itor).second );
			//const SUVInfo &rUVInfo = pkSpriteObj->m_kUVInfo;
			const SUVInfo &rUVInfo = pkSpriteObj->GetUVInfo();
			WORD posU = (WORD)(((rUVInfo.Index-1) % rUVInfo.U) * (rectIcon.right - rectIcon.left + 1));//u 가 가로
			WORD posV = (WORD)(((rUVInfo.Index-1) / rUVInfo.U) * (rectIcon.bottom - rectIcon.top + 1));
			int const index = pTex->AddNewScreenRect(
				(unsigned short)rectIcon.top, 
				(unsigned short)rectIcon.left, 
				(unsigned short)(rectIcon.right - rectIcon.left + 1), 
				(unsigned short)(rectIcon.bottom - rectIcon.top + 1), 
				posV, posU);//

			pTex->MarkAsChanged(NiScreenTexture::EVERYTHING_MASK);
			return pObject;
		}
	}
	return NULL;
}
void* PgUIScene::VCreateStaticFormUIDrawObject(void *pkStaticForm)
{
	return	new PgUIStaticFormDrawObject((CXUI_StaticForm*)pkStaticForm);
}
bool PgUIScene::VReleaseStaticFormUIDrawObject(SPRITE_HASH::mapped_type &pRsc)
{

	PgUIStaticFormDrawObject	*pkObject = (PgUIStaticFormDrawObject*)pRsc;
	SAFE_DELETE(pkObject);

	pRsc = 0;
	return	true;
}

void* PgUIScene::VCreateResource(std::wstring const &rKey)
{
	if (rKey.empty())
		return NULL;

	PG_STAT(PgStatTimerF timer(g_kUIStatGroup.GetStatInfo("VCreateResource"), g_pkApp->GetFrameCount()));

	// 새 텍스쳐 리소스를 만든다.
	NiSourceTexture* pkTexture = g_kNifMan.GetTexture(MB(rKey));
	if(!pkTexture)
	{
		PgError1("%s texture can't load\n", MB(rKey));
		return NULL;
	}

	// 위에서 생성한 텍스쳐를 이용해서 NiScreenTextrue를 만든다.
	NiScreenTexture *pkSprite = NiNew NiScreenTexture(pkTexture);
	// 리소스 컨테이너에 등록한다.
	AddScreenTextureToSpriteCont(pkSprite);
	
	pkSprite->SetApplyMode(NiTexturingProperty::APPLY_MODULATE);
	pkSprite->SetClampMode(NiTexturingProperty::WRAP_S_WRAP_T);//CLAMP_S_CLAMP_T	

	PgUISpriteObject* pkSpriteObj = new PgUISpriteObject(pkSprite);
	pkSpriteObj->m_TexName = rKey;
	
	m_kSpriteObjContainer.push_back(pkSpriteObj);
	// 새로 만든 NiScreenTexutre를 반환한다.
	return pkSpriteObj;
}

void* PgUIScene::VCreateOffscreen(const POINT2& ptSize, int staticNum, DWORD bgColor, float fAlpha)
{
	if (ptSize.x == 0 || ptSize.y == 0 || staticNum == -1)
		return NULL;

	PG_STAT(PgStatTimerF timer(g_kUIStatGroup.GetStatInfo("VCreateOffscreen"), g_pkApp->GetFrameCount()));

	DWORD dw0x = 0x000000ff;
	NiColorA color = NiColorA((bgColor>>16 & dw0x)/255.0f, (bgColor>>8 & dw0x)/255.0f, (bgColor & dw0x)/255.0f, fAlpha);

	NiRenderedTexture* pkTexture = NULL;
	// 새 텍스쳐 리소스를 만든다.
	NiTexture::FormatPrefs kFormat;
	kFormat.m_ePixelLayout= NiTexture::FormatPrefs::TRUE_COLOR_32;
	kFormat.m_eAlphaFmt = NiTexture::FormatPrefs::SMOOTH;
	kFormat.m_eMipMapped = NiTexture::FormatPrefs::NO;
	pkTexture = NiRenderedTexture::Create(ptSize.x, ptSize.y, NiRenderer::GetRenderer(), kFormat);

/*
	if (pkTexture)
	{
		NiColorA bgColor = color;
		bgColor.a = 0.0f;
		NiRenderer* pkRenderer = NiRenderer::GetRenderer();
		NiRenderTargetGroupPtr pTargetGroup = NiRenderTargetGroup::Create(pkTexture->GetBuffer(), pkRenderer, false, false);

		NiRenderTargetGroup *pkOldTarget = 0;
		NiColorA kOldColor;
		if(pkRenderer->IsRenderTargetGroupActive())
		{
			pkOldTarget = (NiRenderTargetGroup *) pkRenderer->GetCurrentRenderTargetGroup();
			pkRenderer->GetBackgroundColor(kOldColor);
			pkRenderer->EndUsingRenderTargetGroup();
		}

		pkRenderer->SetBackgroundColor(bgColor);
		pkRenderer->BeginUsingRenderTargetGroup(pTargetGroup, NiRenderer::CLEAR_ALL);
		pkRenderer->EndUsingRenderTargetGroup();

		if(pkOldTarget)
		{
			pkRenderer->BeginUsingRenderTargetGroup(pkOldTarget, NiRenderer::CLEAR_NONE);
			pkRenderer->SetBackgroundColor(kOldColor);
		}
	}
*/
	if(!pkTexture)
	{
		PG_ASSERT_LOG(!"Can't create texture");
		NILOG(PGLOG_ERROR, "Can't create texture %dx%d(%d) - %s\n", ptSize.x, ptSize.y, staticNum, NiRenderer::GetLastErrorString());		
		return NULL;
	}

	// 위에서 생성한 텍스쳐를 이용해서 NiScreenTextrue를 만든다.
	NiScreenTexture *pkSprite = NiNew NiScreenTexture(pkTexture);
	AddScreenTextureToSpriteCont(pkSprite);
	
	// 리소스 컨테이너에 등록한다.
	pkSprite->SetApplyMode(NiTexturingProperty::APPLY_MODULATE);
	pkSprite->SetClampMode(NiTexturingProperty::WRAP_S_WRAP_T);
	pkSprite->AddNewScreenRect(0, 0, (unsigned short)ptSize.x, (unsigned short)ptSize.y, 0, 0, color);
	pkSprite->MarkAsChanged(NiScreenTexture::EVERYTHING_MASK);

	PgUIRenderedSpriteObject* pkSpriteObj = new PgUIRenderedSpriteObject(pkSprite, ptSize, staticNum);
	m_kRenderedSpriteObjContainer.push_back(pkSpriteObj);	
	return pkSpriteObj;
}

bool PgUIScene::VReleaseOffscreen(SPRITE_HASH::mapped_type &pRsc)
{
	pRsc = 0; // 일단 아무것도 하지 말자.
	return true;
}

bool PgUIScene::VReleaseRsc(SPRITE_HASH::mapped_type &pRsc)
{
	if (pRsc)
	{
		PgUIDrawObject* pObject = (PgUIDrawObject*)pRsc;

		bool bFound = false;
		SpriteObjContainer::iterator itr = m_kSpriteObjContainer.begin();
		for(; itr != m_kSpriteObjContainer.end(); ++itr)
		{
			PgUIDrawObject* pkDrawObject = (*itr);
			if (!pkDrawObject)
			{
				continue;
			}
			if (pkDrawObject == pObject)
			{
				RemoveScreenTextureFromSpriteCont(pObject->GetTex());
				SAFE_DELETE_NI(pObject);

				m_kSpriteObjContainer.erase(itr);
				bFound = true;
				break;
			}
		}
		
		if (bFound == false)
		{
			RenderedSpriteObjContainer::iterator itr = m_kRenderedSpriteObjContainer.begin();
			for(; itr != m_kRenderedSpriteObjContainer.end(); ++itr)
			{
				PgUIRenderedSpriteObject* pkSpriteObject = (*itr);
				if (!pkSpriteObject || pObject)
				{
					continue;
				}
				if (pkSpriteObject == pObject)
				{
					RemoveScreenTextureFromSpriteCont(pObject->GetTex());
					SAFE_DELETE_NI(pObject);

					m_kRenderedSpriteObjContainer.erase(itr);
					break;
				}
			}
		}
	}

	pRsc = 0;//delete 는 소유자인 pool이 한다
//	SAFE_DELETE(pRsc);
	return true;
}

bool CALLBACK PgUIScene::ScriptCallback( std::string const &rScript, XUI::CXUI_Wnd *pWnd)
{
	PG_ASSERT_LOG(pWnd);
	if (pWnd == NULL)
	{
		NILOG(PGLOG_LOG, "[PgUIScene] pWnd is null\n");
	}

	PG_ASSERT_LOG(rScript.size() > 0);
	if (rScript.size() == 0)
	{
		NILOG(PGLOG_LOG, "[PgUIScene] illegal call from 0x%0X name(%s), loc(%d,%d), size(%d,%d), parent(%s)\n", pWnd, MB(pWnd->ID()), pWnd->Location().x, pWnd->Location().y, pWnd->Size().x, pWnd->Size().y, pWnd->Parent() ? MB(pWnd->Parent()->ID()) : "no");
		return false;
	}

	PG_STAT(std::string groupName = "\"" + rScript + "\"");
	PG_STAT(PgStatTimerF timer1(g_kUIStatGroup.GetStatInfo("ScriptCallBack.SetEnv"), g_pkApp->GetFrameCount()));
	PG_STAT(PgStatTimerF timer2(g_kUIStatGroup.GetStatInfo("ScriptCallBack.DoBuffer"), g_pkApp->GetFrameCount()));
	PG_STAT(PgStatTimerF timer3(g_kScriptStatGroup.GetStatInfo(groupName), g_pkApp->GetFrameCount()));

	PG_STAT(timer3.Start());
	PG_STAT(timer1.Start());
	lua_tinker::call<int, lwUIWnd>("UI_SetEnv", lwUIWnd(pWnd));
	PG_STAT(timer1.Stop());

	PG_STAT(timer2.Start());
	bool bRet = PgScripting::DoBuffer( rScript.c_str(), rScript.size() );
	PG_STAT(timer2.Stop());
	PG_STAT(timer3.Stop());
	return bRet;
}
void	PgUIScene::XUIManagerCallBackFunc(CXUI_Wnd *pkWnd,CXUI_Manager::CallBackEventType kEventType)
{
	if(!pkWnd)
	{
		return;
	}

	switch(kEventType)
	{
	case CXUI_Manager::CBET_ON_CALL:
		{
			g_kHelpSystem.ActivateByCondition(HELP_CONDITION_OPEN_UI,std::string(MB(pkWnd->ID())));
		}
		break;
	}

}
bool CALLBACK PgUIScene::TextClickCallBackFunc(XUI::CXUI_Wnd* pWnd,int iButton,std::wstring const &kOriginalString, PgExtraDataPack const &kExtraDataPack, bool bIsDoubleClick)
{
	int	iCount = kExtraDataPack.m_kExtraDataCont.size();
	for(int i=0;i<iCount;++i)
	{
		XUI::PgExtraData const	&kData = kExtraDataPack.m_kExtraDataCont[i];
		if(kData.m_kType == LINK_EDT_ITEM )
//		&& kData.m_kBinaryValue.size() == sizeof(PgBase_Item) )	//	아이템 링크 클릭
		{//아이템 사이즈로 비교 하면 안됨.

			BM::Stream kItemPacket;
			kItemPacket.Data() = kData.m_kBinaryValue;
			kItemPacket.PosAdjust();
			
			PgBase_Item	kItemInfo;

			kItemInfo.ReadFromPacket(kItemPacket);

			if(!PgBase_Item::IsEmpty(&kItemInfo))
			{
				std::wstring wstrText;
				std::wstring wstrLank;
				if(MakeToolTipText(kItemInfo, wstrText, TBL_SHOP_IN_GAME::NullData(), wstrLank))
				{
					lwPoint2	pt(XUIMgr.MousePos().x+100,XUIMgr.MousePos().y);
					lwCallToolTipByText(kItemInfo.ItemNo(), wstrText, pt, "ToolTip_CloseBtn", 0, MB(wstrLank));
				}
			}

		}
		else if(kData.m_kType == LINK_EDT_CHAR)
		{
			BM::GUID	kCharGuid(kData.m_kValue);
			if(bIsDoubleClick)
			{
				//	캐릭터명 더블클릭
			}
			else
			{
				//	캐릭터명 클릭
			}
		}

	}
	return	false;
}

DWORD CALLBACK PgUIScene::HotKeyToRealKey(DWORD const& dwHotKey)
{
	DWORD const iOrgKey = g_kGlobalOption.GetUKeyToKey(dwHotKey+PgInput::UR_LOCAL_BEGIN);

	//키셋팅이 없으면
	if( 0 == iOrgKey)
	{
		return dwHotKey;
	}

	return iOrgKey;
}

void CALLBACK PgUIScene::UniqueExistUICallFailNotice(std::wstring const& kType)
{
	SUniqueUITypeErrInfo const& ErrInfo = XUIMgr.ConvertUniqueTypeUIToErrNo(kType);
	if( 0 != ErrInfo.iTT_No )
	{
		lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", ErrInfo.iTT_No, true);
	}

	if( !ErrInfo.kErrSoundPath.empty() )
	{
		g_kUISound.PlaySoundW(ErrInfo.kErrSoundPath);
	}
}

bool CALLBACK PgUIScene::CursorCallBackFunc(CXUI_Wnd *pkCursorWnd, const POINT2& kPtPoint, const SIconInfo &kIconInfo, const eInputEventIndex kBtn1State, const eInputEventIndex kBtn2State)
{
	CXUI_Cursor *pkCursor = dynamic_cast<CXUI_Cursor *>(pkCursorWnd);
	if(!pkCursor)
	{
		return false;
	}

	PG_STAT(PgStatTimerF timer1(g_kUIStatGroup.GetStatInfo("CursorCallBackFunc.SetEnv"), g_pkApp->GetFrameCount()));
	//좌표아래 무슨 UI 인지 받아서 맞는거 아니면 지우는게야.
	CXUI_Wnd *pkTargetParent = XUIMgr.PickWnd(kPtPoint, false);
	CXUI_Wnd *pkTargetChild = NULL;

	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if (!pkPlayer)
	{
		return false;
	}

	PgInventory *pkInv = pkPlayer->GetInven();
	if(!pkInv)
	{
		return false;
	}

	if( ( pkCursor->CursorState() != CS_NONE ) && kBtn2State == IEI_MS_DOWN ) 
	{// 커서상태가 일반상태 아닐 때 우클릭시 커서상태 리셋
		CXUI_Cursor::m_IconInfo.Clear();
		pkCursor->ResetCursorState();

		return true;
	}

	if(	pkTargetParent 
	&& !pkTargetParent->PassEvent())
	{//타겟이 있다
		pkTargetParent->PickWnd(kPtPoint, pkTargetChild);
	}
	else
	{//타겟이 없다..
		if( kBtn2State == IEI_MS_DOWN )
		{
			return false;
		}
		switch(kIconInfo.iIconGroup)
		{
		case KUIG_EQUIP:
		case KUIG_CONSUME:
		case KUIG_ETC:
		case KUIG_CASH:
		case KUIG_SAFE:
		case KUIG_SAFE_ADDITION:
		case KUIG_CASH_SAFE:
		case KUIG_FIT:
		case KUIG_INV_VIEW:
		case KUIG_RENTAL_SAFE1:
		case KUIG_RENTAL_SAFE2:
		case KUIG_RENTAL_SAFE3:
		case KUIG_PET:
			{// -> Call 해줄때.
				SItemPos kItemPos(kIconInfo.iIconGroup, kIconInfo.iIconKey);
				PgBase_Item kItem;
				if( S_OK != pkInv->GetItem(kItemPos, kItem)
				||	!kItem.ItemNo())
				{
					break;
				}

				if (BM::GUID::NullData() != g_kTradeMgr.GetTradeGuid())
				{
					lwAddWarnDataTT(400732);
					break;
				} 

				if (XUIMgr.Get(_T("SFRM_ITEM_PLUS_UPGRADE")))
				{
					lwAddWarnDataTT(1208);
					break;
				}

				if (XUIMgr.Get(_T("SFRM_ITEM_MAKING_COOK")))
				{
					lwAddWarnDataTT(1612);
					break;
				}

				if (XUIMgr.Get(_T("SFRM_ITEM_RARITY_UPGRADE")))
				{
					lwAddWarnDataTT(1482);
					break;
				}

				if (XUIMgr.Get(_T("SFRM_REPAIR")))
				{
					lwAddWarnDataTT(1223);
					break;
				}

				if (XUIMgr.Get(_T("SFRM_ITEM_RARITY_BUILDUP")))
				{
					lwAddWarnDataTT(1550);
					break;
				}

				if (XUIMgr.Get(_T("SFRM_ITEM_RARITY_AMPLIFY")))
				{
					lwAddWarnDataTT(1551);
					break;
				}

				if (XUIMgr.Get(_T("SFRM_ENCHANT_SHIFT")))
				{
					lwAddWarnDataTT(1278);
					break;
				}

				if (XUIMgr.Get(_T("SFRM_JL3_ITEM_CREATE")))
				{
					lwAddWarnDataTT(799818);
					break;
				}

				SEnchantInfo const& kEhtInfo = kItem.EnchantInfo();
				if(!kEhtInfo.IsBinding() )
				{// 자물쇠가 사용되지 않았다면
					CXUI_Wnd *pkCalledWnd = XUIMgr.Call(UNI("SFRM_DELETE_ITEM"), true);

					assert(pkCalledWnd);
					if(pkCalledWnd)
					{
						SItemPos kItemPos;
						kItemPos.x = kIconInfo.iIconGroup;
						kItemPos.y = kIconInfo.iIconKey;
						pkCalledWnd->SetCustomData(&kItemPos, sizeof(kItemPos));		//모달로 띄우자
					}
				}
				else
				{// 자물쇠가 사용된 아이템이라면 메세지로 알리고 삭제 UI를 호출 하지 않음
					lwAddWarnDataTT(790314);
				}
			}break;
		case KUIG_QUICK_INVEN:
			{
				PgQuickInventory *pkQInv = pkPlayer->GetQInven();
				if(pkQInv)
				{
					SQuickInvInfo kQuickInvInfo;
					Send_PT_C_M_REQ_REGQUICKSLOT(pkQInv->AdjustSlotIDX(kIconInfo.iIconKey), kQuickInvInfo);
				}
			}break;
		case KUIG_NONE:
			{//디폴트는 return false; (다음이 실행 되도록)
				if(kBtn1State != IEI_MS_UP)
				{
					CXUI_Cursor::m_IconInfo.Clear();
					pkCursor->ResetCursorState();

					if(XUIMgr.Get(L"FRM_COMMON_POPUP"))
					{
						XUIMgr.Close(L"FRM_COMMON_POPUP");
					}
				}
				return false;
			}break;
		}
	
		CXUI_Cursor::m_IconInfo.Clear();
		pkCursor->ResetCursorState();
		return true;
	}	
	
//포인트 넣으면 Activate 최상위에 있는 윈도우가 뭔지 알려줘야함.
//그 윈도우를 가지고. 장난질 해야됨.

	return false;//트루 리턴이면 다음 메세지를 전혀 안먹음.
}

bool PgUIScene::LoadXUIXML(std::wstring const &wstrFileName)
{
	PG_STAT(PgStatTimerF timer(g_kUIStatGroup.GetStatInfo("LoadXUIXML"), g_pkApp->GetFrameCount()));
	TiXmlDocument kXmlDoc;
	if(!PgXmlLoader::LoadFile(kXmlDoc, wstrFileName)){return false;}
	if(!XUIMgr.ParseXML(kXmlDoc)){return false;}
	return true;
}

bool PgUIScene::LoadUIPath(char const *pcPathXml)
{
	float fTime = NiGetCurrentTimeInSec();

	TiXmlDocument kXmlDoc;
	if(!PgXmlLoader::LoadFile(kXmlDoc, UNI(pcPathXml)))
	{
		PG_ASSERT_LOG(!"failed to loading path.xml");
		return false;
	}

	// 최상위 노드를 찾는다.
	TiXmlNode *pkRootNode = kXmlDoc.FirstChild();
	while(pkRootNode && pkRootNode->Type() != TiXmlNode::ELEMENT)
	{
		pkRootNode = pkRootNode->NextSibling();
	}

	// 최상위 노드가 없다면,
	if(!pkRootNode)
	{
		PgError1("No [%s] Top Class Node", pcPathXml);
		return false;
	}

	TiXmlElement *pkElement = pkRootNode->FirstChildElement();
	while(pkElement)
	{
		char const *pcID = 0;

		// GUID를 알아낸다.
		TiXmlAttribute* pkAttr = pkElement->FirstAttribute();
		while(pkAttr)
		{
			char const *pcAttrName = pkAttr->Name();
			char const *pcAttrValue = pkAttr->Value();
			
			if(strcmp(pcAttrName, "ID") == 0)
			{
				pcID = pcAttrValue;
			}
			else
			{
				PgXmlError1(pkElement, "XmlParse: Incoreect Attr '%s'", pcAttrName);
			}
			

			pkAttr = pkAttr->Next();
		}

		if(!LoadXUIXML(UNI(pkElement->GetText())))
		{
			PG_ASSERT_LOG(NULL);
			PgXmlError1(pkElement, "Can't Load XML:[%s]", pkElement->GetText());
		}
//		// 경로 컨테이너에 추가한다.
//		if(pcID && !m_kXmlPathContainer.insert(std::make_pair(pcID, pkElement->GetText())).second)
//		{
//			PG_ASSERT_LOG(!"xml path adding failed");
//		}
		// 같은 층의 다음 노드를 파싱한다.
		pkElement = pkElement->NextSiblingElement();
	}
	NILOG(PGLOG_LOG, "[PgUIScene] ParseXml %f time elasped\n", NiGetCurrentTimeInSec() - fTime);
	return true;
}
/*
class CXUI_Quest_Form
	: public XUI::CXUI_Form
{
public:
	CXUI_Quest_Form()
	{
	}
	
	virtual CXUI_Wnd* VCreate()const{return new CXUI_Quest_Form;}
};
*/
int QueryKey(HKEY hKey, TCHAR *szSubKey, std::wstring &rkRet);

bool PgUIScene::ParseXml(const TiXmlNode * pkNode, void *pArg, bool bUTF8)
{
	PG_STAT(PgStatTimerF timer1(g_kUIStatGroup.GetStatInfo("ParseXml"), g_pkApp->GetFrameCount()));
	int const iType = pkNode->Type();

	if(strcmp(pkNode->Value(), "UI") == 0)
	{
		static bool bXUIInited = false;

		if(!bXUIInited)
		{
			bXUIInited = true;

			XUI::XUI_INIT_INFO info;
			info.pRscMgr		= (CXUI_Resource_Manager_Base *)this;
			info.pRenderer		= (XUI::CXUI_Renderer_Base *)this;
			info.pMediaPlayer	= &g_kUISound;
			info.pTextTable		= (XUI::CXUI_TextTable_Base *)this;
			info.pScriptFunc	= &PgUIScene::ScriptCallback;
			info.pHotKeyModifier = (XUI::CXUI_HotKeyModifier *)this;
			info.pCursorCallBackFunc	= &PgUIScene::CursorCallBackFunc;
			info.pTextClickCallBackFunc = &PgUIScene::TextClickCallBackFunc;
			info.pHotKeyToRealKey = &PgUIScene::HotKeyToRealKey;
			info.pUniqueExistCheckNotice = &PgUIScene::UniqueExistUICallFailNotice;
			info.pLogFunc = &PgOutputDebugStringFromXUI;

			info.hWnd = NiApplication::ms_pkApplication->GetWindowReference();
			info.hInstance = NiApplication::ms_pkApplication->GetInstanceReference();

			if(!XUIMgr.InitXUI(info,
#if defined(PG_USE_THREAD_LOADING)
				false // BACKGROUND_THREAD를 쓸 때는 PgMobileSuit::OnDefault에서 InitIME가 불려진다.
#else
				true
#endif
				))
			{
				return false;
			}

			XUIMgr.SetCallBackFunc(XUIManagerCallBackFunc);

			{//윈도우 마우스 셋팅
				HKEY hTestKey = 0;
				if( RegOpenKeyEx( HKEY_CURRENT_USER, _T("Control Panel\\Mouse"), 0, KEY_READ, &hTestKey) == ERROR_SUCCESS )
				{
					std::wstring kDblClickSpeed, kDblClickWidth, kDblClickHeight, kMouseSensitivity;
					int iRet1 = QueryKey(hTestKey, _T("DoubleClickSpeed"), kDblClickSpeed);
					int iRet2 = QueryKey(hTestKey, _T("DoubleClickWidth"), kDblClickWidth);
					int iRet3 = QueryKey(hTestKey, _T("DoubleClickHeight"), kDblClickHeight);
					int iRet4 = QueryKey(hTestKey, _T("MouseSensitivity"), kMouseSensitivity);
					int iDblClickSpeed = _ttoi(kDblClickSpeed.c_str());
					int iDblClickWidth = _ttoi(kDblClickWidth.c_str());
					int iDblClickHeight = _ttoi(kDblClickHeight.c_str());
					int iMouseSensitivity = (kMouseSensitivity.size())? _ttoi(kMouseSensitivity.c_str()): 10;
					if(!FAILED(iRet1)) {XUIMgr.DblClickTick(iDblClickSpeed);};
					if(!FAILED(iRet2)&&!FAILED(iRet3)) {XUIMgr.DblClickBound(POINT3I(iDblClickWidth/2, iDblClickHeight/2, 0));};
					XUIMgr.MouseSensitivity(iMouseSensitivity);
				}
			}

			XUIMgr.RegistCreator(_T("QUEST_LIST_PANNEL"), PgQUI_IngQuestList());
			XUIMgr.RegistCreator(_T("FORM_QUEST_INFO"), PgQUI_QuestDialog());
			XUIMgr.RegistCreator(_T("FORM_SELECTIVE_QUEST_LIST"), PgQUI_SelectiveQuestList());
			XUIMgr.RegistCreator(_T("FORM_SNAPED_HEAD_WND"), PgFormSnapedHeadWnd());
			XUIMgr.RegistCreator(_T("FORM_SNAPED_SUB_WND"), PgFormSnapedSubWnd());
			XUIMgr.RegistCreator(_T("FORM_ANI_MOVE_WND"), PgFormAnimatedMoveWnd());
			XUIMgr.RegistCreator(_T("FORM_ICON_ROULETTE_WND"), PgFormRouletteWnd());
			XUIMgr.RegistCreator(_T("FORM_MISSION_ANI_MOVE_WND"), PgMissionFormAnimatedMoveWnd());
			XUIMgr.RegistCreator(_T("FORM_TEXT_TYPIST_WND"), PgTextTypistWnd());
			XUIMgr.RegistCreator(_T("FORM_MOVE_ANIMATE_WND"), PgMoveAnimateWnd());

			if(!LoadUIPath("UIPath.xml")){return false;}

			NILOG(PGLOG_LOG, "[PgUIScene] Start XUIMgr.BuildXUI\n");
			if(!XUIMgr.BuildXUI())
			{
				return false;
			}
			NILOG(PGLOG_LOG, "[PgUIScene] end XUIMgr.BuildXUI\n");
			Build();
			NILOG(PGLOG_LOG, "[PgUIScene] end BuildXUI\n");
		}
		XUIMgr.CloseAll();
		NILOG(PGLOG_LOG, "[PgUIScene] end CloseAll\n");
	}

	switch(iType)
	{
	case TiXmlNode::ELEMENT:
		{
			TiXmlElement *pkElement = (TiXmlElement *)pkNode;
			PG_ASSERT_LOG(pkElement);
			
			char const *pcTagName = pkElement->Value();

			if(strcmp(pcTagName, "UI") == 0)
			{
				PgRenderMan *pkRenderMan = (PgRenderMan*)pArg;
				m_pkRenderer = pkRenderMan->GetRenderer();

				const TiXmlAttribute *pkAttr = pkElement->FirstAttribute();
				while(pkAttr)
				{
					char const *pcTagName = pkAttr->Name();
					char const *pcTagValue = pkAttr->Value();

					if(strcmp(pcTagName, "ID") == 0)
					{
						SetID(pcTagValue);
					}
					else
					{
						PgXmlError1(pkElement, "XmlParse: Incoreect Attr '%s'", pcTagName);
					}

					pkAttr = pkAttr->Next();
				}

				// 자식 노드들을 파싱한다.
				// 첫 자식만 여기서 걸어주면, 나머지는 NextSibling에 의해서 자동으로 파싱된다.
				const TiXmlNode * pkChildNode = pkNode->FirstChild();
				if(pkChildNode != 0)
				{
					return ParseXml(pkChildNode, pArg, bUTF8);
				}
			}
			else if(strcmp(pcTagName, "SCRIPT") == 0)
			{
				PgScripting::DoFile(pkElement->GetText());
			}

			else if(strcmp(pcTagName, "FONT") == 0)
			{
				TiXmlElement* pkElement = (TiXmlElement*)pkNode;
				TiXmlAttribute const *pkAttr = pkElement->FirstAttribute();
				if(pkAttr)
				{	
					std::wstring kFontName;
					char const *pcAttrName = pkAttr->Name();
					char const *pcAttrValue = pkAttr->Value();
					if(_stricmp(pcAttrName, "NAME") == 0)
					{
						kFontName = UNI(pcAttrValue);
					}
					
					BM::vstring kFontSize;
					std::string kFontPath;
					CXUI_Font::FONT_STYLE kFontStyle = CXUI_Font::FS_NONE;

					std::string kDeafaultFontPath;
					BM::vstring kDefaultFontSize;
					CXUI_Font::FONT_STYLE kDefaultFontStyle = CXUI_Font::FS_NONE;

					EFontFound eFound = eNoneFound;
					TiXmlNode const* pkNextNode = pkNode->FirstChild();
					if(pkNextNode)
					{						
						while(pkNextNode)
						{
							while (pkNextNode && pkNextNode->Type() == TiXmlNode::COMMENT)
							{
								pkNextNode = pkNextNode->NextSibling();
							}
							if(!pkNextNode)
							{
								break;
							}
							pkElement = (TiXmlElement*)pkNextNode;
							TiXmlAttribute *pkAttr = pkElement->FirstAttribute();
							while(pkAttr)
							{
								char const *pcAttrName1 = pkAttr->Name();
								char const *pcAttrValue1 = pkAttr->Value();
								if(strcmp(pcAttrName1, "SERVICE_REGION") == 0)
								{
									std::string const kServiceRegion(MB(XUIMgr.LocalName()));
									std::string const kServiceRegionGroup(pcAttrValue1);
									std::string const kDefaultStr("DEFAULT");
									typedef std::list< std::string > Contstr;
									Contstr kList;
									BM::vstring::CutTextByKey(kServiceRegionGroup, std::string("/"), kList);
									if(kList.empty())
									{
										_PgMessageBox("PgUIScene::ParseXml()", "Cant find any font:\"%s\"",kFontName.c_str());
										return false;
									}
									Contstr::const_iterator itor = kList.begin();
									while(kList.end() != itor)
									{
										if(kServiceRegion == (*itor))
										{// 적합한 폰트를 찾았다면 바로 저장. 현재 상태는 적합한 로컬 파싱중
											eFound = eFontFound;
											kFontPath = pkElement->GetText();
											break;
										}
										else if(kDefaultStr == (*itor))
										{// DEFAULT를 찾았다면 DEFAULT 폰트를 임시 저장.
											eFound = eDefaultFontFound;
											kDeafaultFontPath = pkElement->GetText();
										}
										++itor;
									}
									if( eNoneFound == eFound)
									{
										break;
									}
								}
								else if(strcmp(pcAttrName1, "SIZE") == 0)
								{
									if(eFontFound == eFound)
									{
										kFontSize = UNI(pcAttrValue1);
									}
									else if( eDefaultFontFound == eFound)
									{// 현재 상태가 찾지 못함 상태면, 디폴트 저장 외에는 들어와서는 안된다.
										kDefaultFontSize = UNI(pcAttrValue1);
									}
								}
								else if(strcmp(pcAttrName1, "STYLE") == 0)
								{
									if(strcmp(pcAttrValue1, "NONE") == 0)
									{
										if(eFontFound == eFound)
										{
											kFontStyle = CXUI_Font::FS_NONE;
										}
										else if( eDefaultFontFound == eFound)
										{
											kDefaultFontStyle  = CXUI_Font::FS_NONE;
										}
									}
									else if(strcmp(pcAttrValue1, "BOLD") == 0)
									{
										if(eFontFound == eFound)
										{
											kFontStyle = CXUI_Font::FS_BOLD;
										}
										else if( eDefaultFontFound == eFound)
										{
											kDefaultFontStyle = CXUI_Font::FS_BOLD;
										}
									}
									else if(strcmp(pcAttrValue1, "ITALIC") == 0)
									{
										if(eFontFound == eFound)
										{
											kFontStyle = CXUI_Font::FS_ITALIC;
										}
										else if( eDefaultFontFound == eFound)
										{
											kDefaultFontStyle = CXUI_Font::FS_ITALIC;
										}
									}
									else if(strcmp(pcAttrValue1, "FS_UNDERLINE") == 0)
									{
										if(eFontFound == eFound)
										{
											kFontStyle = CXUI_Font::FS_NONE;
										}
										else if( eDefaultFontFound == eFound)
										{
											kDefaultFontStyle = CXUI_Font::FS_NONE;
										}
									}
								}
								else
								{
									PgXmlError1(pkElement, "XmlParse: Incoreect Attr '%s'", pcAttrName);
								}
								pkAttr = pkAttr->Next();
							}
							if(eFontFound == eFound)
							{//이미 찾았으면, 다음은 안찾아도 되지 않나?
								break;
							}
							else if( eDefaultFontFound == eFound )
							{//한번 디폴트값 찾았으면 인자값만 다시 파싱하지 않도록 상태 초기화 한다.
								eFound = eNoneFound;
							}
							pkNextNode = pkNextNode->NextSibling();
						}
						//폰트 추가 부분
						if(eFontFound != eFound)
						{// 못찾았으면 무조건 Default 폰트로 간주
							kFontPath = kDeafaultFontPath;
							kFontSize = kDefaultFontSize;
							kFontStyle = kDefaultFontStyle;
						}
						int const kFontPathSize = kFontPath.size();
						int const kFontSizeSize = kFontSize.size();
						if( 0 != kFontPathSize
						 && 0 != kFontSizeSize)
						{
							CXUI_Font* pFont = g_kFontMgr.AddNewFont(kFontName, UNI(kFontPath),(int)kFontSize);
							if(pFont)
							{
								pFont->SetStyle2(kFontStyle);
							}
						}
						else
						{// Default도 없고, 맞는 영역의 폰트도 없을경우
							_PgMessageBox("PgUIScene::ParseXml()","Can\'t find \'%s\' Default Font :", kFontName.c_str());
						}
					}
				}
			}
			else if(strcmp(pcTagName, "LOCAL") == 0)
			{
				TiXmlNode const* pkFindLocalNode = PgXmlLocalUtil::FindInLocal(g_kLocal, pkElement, "PgUIScene");
				if( pkFindLocalNode )
				{
					TiXmlNode const* pkResultNode = pkFindLocalNode->FirstChild();
					if( pkResultNode )
					{
						ParseXml( pkResultNode, pArg, bUTF8 );
					}
				}
			}
			else if(strcmp(pcTagName, "INTERFACE") == 0)
			{
				const TiXmlAttribute *pkAttr = pkElement->FirstAttribute();
				while(pkAttr)
				{
					char const *pcAttrName = pkAttr->Name();
					char const *pcAttrValue = pkAttr->Value();
					
					if(strcmp(pcAttrName, "NAME") == 0)
					{
						XUIMgr.Call(UNI(pcAttrValue));
					}
					else if(strcmp(pcAttrName, "NAME_MODAL") == 0)
					{
						XUIMgr.Call(UNI(pcAttrValue), true);
					}

					pkAttr = pkAttr->Next();
				}
			}
			else if(strcmp(pcTagName, "INPUT") == 0)
			{
				TiXmlElement *pkChild = pkElement->FirstChildElement();

				while(pkChild)
				{
					char const *pcTagName = pkChild->Value();

					if(strcmp(pcTagName, "SLOT") == 0)
					{
						TiXmlAttribute *pkAttr = pkChild->FirstAttribute();
						unsigned int uiKey = 0;
						char const *pcScript = 0;

						while(pkAttr)
						{
							if(strcmp(pkAttr->Name(), "NAME") == 0)
							{
							}
							else if(strcmp(pkAttr->Name(), "KEY") == 0)
							{
								uiKey = atoi(pkAttr->Value());
							}
							else if(strcmp(pkAttr->Name(), "SCRIPT") == 0)
							{
								pcScript = pkAttr->Value();
							}
							else
							{
								PgXmlError1(pkChild, "XmlParse: Incoreect Attr '%s'", pcTagName);
							}

							pkAttr = pkAttr->Next();
						}

						if(uiKey && pcScript)
						{
							m_kInputContainer.insert(std::make_pair(uiKey, std::string(pcScript)));
						}
					}
					else
					{
						PgXmlError1(pkChild, "XmlParse: Incoreect Tag '%s'", pcTagName);
					}

					pkChild = pkChild->NextSiblingElement();
				}
			}
			else if(strcmp(pcTagName, "MISSION_INFO") == 0)
			{
				ParseXmlForMission(pkElement->GetText());
			}
			else if(strcmp(pcTagName, "NPC_2D_FACE") == 0)
			{
				g_kQuestMan.ParseXmlForNpcFace(pkElement->GetText());
			}
			else if(strcmp(pcTagName, "LOADING_IMAGE_LINK") == 0)
			{
				ParseXmlForLoadingImg(pkElement->GetText());
			}
			else
			{
				PgXmlError1(pkElement, "XmlParse: Incoreect Tag '%s'", pcTagName);
			}
		}break;
	default:
		break;
	}
	
	// 같은 층의 다음 노드를 재귀적으로 파싱한다.
	const TiXmlNode* pkNextNode = pkNode->NextSibling();

	while (pkNextNode && pkNextNode->Type() == TiXmlNode::COMMENT)
	{
		pkNextNode = pkNextNode->NextSibling();
	}

	if(pkNextNode)
	{
		return ParseXml(pkNextNode, pArg, bUTF8);
	}

	// 모든 파싱이 끝났다면 XUI를 초기화한다.
//	/*
	PgRenderMan *pkRenderMan = (PgRenderMan *)pArg;

	m_pkRenderer = pkRenderMan->GetRenderer();
//	*/
///*

//	float const fScale = 800.f;// g_RenderMan.GetScreenSize().x / 800.f;

//	XUIMgr.VScale(fScale);
	return true;
}

void PgUIScene::Terminate()
{
	SAFE_DELETE_NI(m_pImeNameTextBalloon);
	SAFE_DELETE_NI(m_pImeCandidateTextBalloon);
	SAFE_DELETE_NI(m_pImeCompStrTextBalloon);
	SAFE_DELETE_NI(m_pImeCompReadTextBalloon);

	ClearAllOffscreen();

	m_spFillRect = 0;

	{
		SpriteContainer::iterator itr = m_kSpriteContainer.begin();
		for(; itr != m_kSpriteContainer.end(); ++itr)
		{
			SAFE_DELETE_NI(*itr);
		}
		m_kSpriteContainer.clear();
	}

	{
		SpriteObjContainer::iterator itr = m_kSpriteObjContainer.begin();
		for(; itr != m_kSpriteObjContainer.end(); ++itr)
		{
			SAFE_DELETE_NI(*itr);
		}
		m_kSpriteObjContainer.clear();
	}

	{
		RenderedSpriteObjContainer::iterator itr = m_kRenderedSpriteObjContainer.begin();
		for(; itr != m_kRenderedSpriteObjContainer.end(); ++itr)
		{
			SAFE_DELETE_NI(*itr);
		}
		m_kRenderedSpriteObjContainer.clear();
	}

	{
		CONT_IMG::iterator itr = m_mapImg.begin();
		for(; itr != m_mapImg.end(); itr++)
		{
			NiScreenTexture* pkTex = itr->second->GetTex();
			SAFE_DELETE_NI(pkTex);
			SAFE_DELETE(itr->second);
		}	
		m_mapImg.clear();
	}

	SpritePool::iterator pool_itor = m_kSpritePool.begin();
	for(;pool_itor != m_kSpritePool.end(); ++pool_itor)
	{
		PgUISpritePoolObject::SprObjlist* pList = (*pool_itor).second;
		
		PgUISpritePoolObject::SprObjlist::iterator list_itor = pList->begin();
		for(; list_itor!=pList->end(); ++list_itor)
		{
			SAFE_DELETE((*list_itor));
		}
		
		SAFE_DELETE((*pool_itor).second);
	}
	m_kSpritePool.clear();


	PgUITextPoolObject::CONT_TEXT_POOL::iterator text_pool_itor = m_kTextPool.begin();
	for(; text_pool_itor != m_kTextPool.end(); ++text_pool_itor)
	{
		SAFE_DELETE((*text_pool_itor));
	}
	m_kTextPool.clear();

	MiniMapContainer::iterator itr = m_kMiniMapContainer.begin();
	while(itr != m_kMiniMapContainer.end())
	{
		MiniMapContainer::mapped_type& pkMiniMap = itr->second;
		SAFE_DELETE(pkMiniMap);
		++itr;
	}
	m_kMiniMapContainer.clear();
	
	//m_kTextureContainer.clear();
	m_spRenderTargetGroup = 0;
	m_spRenderTexture = 0;
	m_spScreenTexture = 0;

	m_pkWorldMapUI = 0;

	for(UIModelContainer::iterator itor = m_kUIModelContainer.begin(); itor != m_kUIModelContainer.end();itor++)
	{
		NiDelete (itor->second);
	}
	m_kUIModelContainer.clear();

	XUIMgr.Terminate();
}

bool PgUIScene::AddIconRsc(int const IconNo, std::wstring const &wstrRscName, const SUVInfo &rkUvInfo)//리소스 추가.
{
	//아이콘 이미지를. pool에 담고. clone 으로 배껴.
	//IconNo 등록 할때 UV도 등록을 해.
	if (wstrRscName == _T("."))
		return false;

	NiSourceTexture* pkTexture = g_kNifMan.GetTexture(MB(wstrRscName));
	if (pkTexture == NULL)
	{
		PgError1("%s texture can't load\n", MB(wstrRscName));
		return false;
	}

	NiScreenTexture *pkSprite = NiNew NiScreenTexture(pkTexture);
	PG_ASSERT_LOG(pkSprite);
	if (pkSprite == NULL)
		return false;

	PgUIUVSpriteObject* pkSpriteObj =  new PgUIUVSpriteObject(pkSprite, rkUvInfo);
	PG_ASSERT_LOG(pkSpriteObj);
	if (pkSpriteObj == NULL)
		return false;

	//pkSpriteObj->m_TexName = wstrRscName;

	m_mapImg.insert(std::make_pair(IconNo, pkSpriteObj));
	return true;
}
bool	PgUIScene::GetIconInfo(int const iIconNo,
	bool const bIs48siz,
	unsigned short sW, 
	unsigned short sH,
	NiTexture *&pkTexture,
	float &fU,float &fV,
	float &fW,float &fH)
{
	PG_STAT(PgStatTimerF timer(g_kUIStatGroup.GetStatInfo("RenderIcon"), g_pkApp->GetFrameCount()));
	CONT_IMG::const_iterator img_itor = m_mapImg.find(iIconNo);
	if(m_mapImg.end() == img_itor)
	{
		if(true==AddIconRsc(iIconNo))
		{
			img_itor = m_mapImg.find(iIconNo);
		}
	}


	unsigned short w = sW;
	unsigned short h = sH;
	if(bIs48siz)
	{
		w = 48;
		h = 48;
	}

	if(img_itor != m_mapImg.end())
	{
		PgUISpriteObject	*pkSpriteObj = (*img_itor).second;

		NiScreenTexture	*pkScreenTexture = pkSpriteObj->GetTex();
		if(pkScreenTexture)
		{
			pkTexture = pkScreenTexture->GetTexture();

			if (pkTexture)
			{
				PgUIUVSpriteObject* pkUVSpriteObj = static_cast<PgUIUVSpriteObject*>(pkSpriteObj);

				if(pkUVSpriteObj)
				{
					const SUVInfo &rUVInfo = pkUVSpriteObj->GetUVInfo();
					WORD posU = ((rUVInfo.Index-1) % rUVInfo.U) * w;//u 가 가로
					WORD posV = ((rUVInfo.Index-1) / rUVInfo.U) * h;

					fU = posU/((float)pkTexture->GetWidth());
					fV = posV/((float)pkTexture->GetHeight());
					fW = w/((float)pkTexture->GetWidth());
					fH = h/((float)pkTexture->GetHeight());

					return	true;
				}
			}
		}
	}

	return	false;
}

void PgUIScene::RenderIcon(int const IconNo, const POINT2 &ptPos, bool bIs48siz, unsigned short sW, unsigned short sH,bool bGrayScale)
{//렌더를 다 걸어놓고 해쉬 정보를 마지막에 전부 에드 해주자.
//그 이후에 텍스트 정보도.
	PG_STAT(PgStatTimerF timer(g_kUIStatGroup.GetStatInfo("RenderIcon"), g_pkApp->GetFrameCount()));
	CONT_IMG::const_iterator img_itor = m_mapImg.find(IconNo);
	if(m_mapImg.end() == img_itor)
	{
		if(true==AddIconRsc(IconNo))
		{
			img_itor = m_mapImg.find(IconNo);
		}
	}

	unsigned short w = sW;
	unsigned short h = sH;
	if(bIs48siz)
	{
		w = 48;
		h = 48;
	}

	if(img_itor != m_mapImg.end())
	{
		//////////////////////////////////////////////
		PgUISpritePoolObject::SprObjlist::value_type pObject = 0;
		SpritePool::iterator pool_itor =  m_kSpritePool.find(IconNo);

		if(pool_itor == m_kSpritePool.end())
		{
			std::pair<SpritePool::iterator, bool> ret = m_kSpritePool.insert(std::make_pair(IconNo, new PgUISpritePoolObject::SprObjlist ));

			if(ret.second)
			{
				pool_itor = ret.first;
			}
			else
			{
				PG_ASSERT_LOG(0);
			}
		}

		if(pool_itor != m_kSpritePool.end())
		{//찾았을 경우다.
			SpritePool::mapped_type pList =  (*pool_itor).second;

			if(pList->size())
			{
				 pObject = pList->front();
				 pList->pop_front();
			}
			else
			{//새로 만들었는데 풀에 넣으면 안됨. 이건 풀이니까.
				NiObject *pClonedSprite = (*img_itor).second->GetTex()->Clone();
				pObject = new PgUISpritePoolObject((NiScreenTexture*)pClonedSprite, pList);
			}

			if(!pObject)
			{
				PG_ASSERT_LOG(0);
			}
		}
		
		if (pObject)
		{
			NiScreenTexture *pTex = pObject->GetTex();
			pTex->RemoveAllScreenRects();

			PgUIUVSpriteObject* pkSpriteObj = static_cast<PgUIUVSpriteObject*>( (*img_itor).second );
			//const SUVInfo &kUVInfo = pkSpriteObj->m_kUVInfo;
			SUVInfo kUVInfo = pkSpriteObj->GetUVInfo();


			WORD posU = ((kUVInfo.Index-1) % kUVInfo.U) * w;//u 가 가로
			WORD posV = ((kUVInfo.Index-1) / kUVInfo.U) * h;
			int const index = pTex->AddNewScreenRect( (WORD)ptPos.y, (WORD)ptPos.x, w, h, posV, posU);//
			pkSpriteObj->SetGrayScale(bGrayScale);
			pObject->SetGrayScale(bGrayScale);

			pTex->MarkAsChanged(NiScreenTexture::EVERYTHING_MASK);
			AddToDrawList(pObject,true);
		}
	}
}

void PgUIScene::RenderIcon(int const IconNo, const SRenderInfo &rRenderInfo)
{
	PG_STAT(PgStatTimerF timer(g_kUIStatGroup.GetStatInfo("RenderIcon"), g_pkApp->GetFrameCount()));

	if (rRenderInfo.rcClip.left >= rRenderInfo.kSizedScale.ptSrcSize.x ||
		rRenderInfo.rcClip.right >= rRenderInfo.kSizedScale.ptSrcSize.x)
	{
		return;
	}

	if (rRenderInfo.rcClip.top >= rRenderInfo.kSizedScale.ptSrcSize.y ||
		rRenderInfo.rcClip.bottom >= rRenderInfo.kSizedScale.ptSrcSize.y)
	{
		return;
	}
	CONT_IMG::const_iterator img_itor = m_mapImg.find(IconNo);
	if(m_mapImg.end() == img_itor)
	{
		if(true==AddIconRsc(IconNo))
		{
			img_itor = m_mapImg.find(IconNo);
		}
	}

	if(img_itor != m_mapImg.end())
	{
		//////////////////////////////////////////////
		PgUISpritePoolObject::SprObjlist::value_type pObject = 0;
		SpritePool::iterator pool_itor =  m_kSpritePool.find(IconNo);

		if(pool_itor == m_kSpritePool.end())
		{
			std::pair<SpritePool::iterator, bool> ret = m_kSpritePool.insert(std::make_pair(IconNo, new PgUISpritePoolObject::SprObjlist ));

			if(ret.second)
			{
				pool_itor = ret.first;
			}
			else
			{
				PG_ASSERT_LOG(0);
			}
		}

		if(pool_itor != m_kSpritePool.end())
		{//찾았을 경우다.
			SpritePool::mapped_type pList =  (*pool_itor).second;

			if(pList->size())
			{
				pObject = pList->front();
				pList->pop_front();
			}
			else
			{//새로 만들었는데 풀에 넣으면 안됨. 이건 풀이니까.
				NiObject *pClonedSprite = (*img_itor).second->GetTex()->Clone();
				pObject = new PgUISpritePoolObject((NiScreenTexture*)pClonedSprite, pList);
			}

			if(!pObject)
			{
				PG_ASSERT_LOG(0);
			}
		}

		if (pObject)
		{
			NiScreenTexture *pTex = pObject->GetTex();
			pTex->RemoveAllScreenRects();

			//////////////////////////////////////////////////////////////////////////
			//
			const SSizedScale &rSS = rRenderInfo.kSizedScale;//찍힐 영역. --> 이게 찍힐 영역이면 안됨.

			//const SUVInfo &rUVInfo = rRenderInfo.kUVInfo;//UV 짤린거.

			const POINT2 &rLoc = rRenderInfo.kLoc;//찍힐 위치.

			const RECT &rDrawable = rRenderInfo.rcDrawable;

			const RECT &rClip = rRenderInfo.rcClip;
			//
			//////////////////////////////////////////////////////////////////////////

			PgUIUVSpriteObject* pkSpriteObj = static_cast<PgUIUVSpriteObject*>( (*img_itor).second );
			//const SUVInfo &rUVInfo = pkSpriteObj->m_kUVInfo;
			const SUVInfo &rUVInfo = pkSpriteObj->GetUVInfo();
			WORD posU = (WORD)(((rUVInfo.Index-1) % rUVInfo.U) * rSS.ptSrcSize.x + rClip.left);//u 가 가로
			WORD posV = (WORD)(((rUVInfo.Index-1) / rUVInfo.U) * rSS.ptSrcSize.y + rClip.top);
			NiColorA const kColor(
				((rRenderInfo.dwColor & 0x00ff0000)>>16)/255.0f,
				((rRenderInfo.dwColor & 0x0000ff00)>>8)/255.0f,
				(rRenderInfo.dwColor & 0x000000ff)/255.0f,
				((rRenderInfo.dwColor & 0xff000000)>>24)/255.0f);
			int const index = pTex->AddNewScreenRect( 
				(WORD)(rLoc.y + rClip.top), 
				(WORD)(rLoc.x + rClip.left), 
				(unsigned short)(rSS.ptSrcSize.x - (rClip.left + rClip.right)),
				(unsigned short)(rSS.ptSrcSize.y - (rClip.top + rClip.bottom)), 
				posV, posU, kColor);//
		
			pkSpriteObj->SetGrayScale(rRenderInfo.bGrayScale);
			pObject->SetGrayScale(rRenderInfo.bGrayScale);
			pObject->SetColor(rRenderInfo.dwColor);
			pObject->Scale(rRenderInfo.fScale);

			pTex->MarkAsChanged(NiScreenTexture::EVERYTHING_MASK);
			pTex->SetApplyMode(NiTexturingProperty::APPLY_MODULATE);
			AddToDrawList(pObject,true);
		}
	}


}

bool PgUIScene::AddIconRsc(int const resNo)
{//멀티쓰레드라면 X됨!!!! 큰일남
	CONT_DEFRES const* pkContDefRes = NULL;
	g_kTblDataMgr.GetContDef(pkContDefRes);
	CONT_DEFRES::const_iterator res_itr = pkContDefRes->find(resNo);
	if (pkContDefRes->end()!=res_itr)
	{
		SUVInfo kUVInfo( (*res_itr).second.U, (*res_itr).second.V, (*res_itr).second.UVIndex);
		bool ret = AddIconRsc( (*res_itr).second.ResNo, (*res_itr).second.strIconPath, kUVInfo );
		return ret;
	}

	return false;
}

const TCHAR *PgUIScene::GetTextW(unsigned long ulTextNo)const
{
	return TTW(ulTextNo).c_str();
}



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//		class	PgCursour
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//if( RegOpenKeyEx( HKEY_CURRENT_USER, _T("Control Panel\\Mouse"), 0, KEY_READ, &hTestKey) == ERROR_SUCCESS )
	//{
	//	std::wstring kDblClickSpeed, kDblClickWidth, kDblClickHeight;
	//	QueryKey(hTestKey, _T("DoubleClickSpeed"), kDblClickSpeed);
	//	QueryKey(hTestKey, _T("DoubleClickWidth"), kDblClickWidth);
	//	QueryKey(hTestKey, _T("DoubleClickHeight"), kDblClickHeight);
	//	std::cout << _ttoi(kDblClickSpeed.c_str()) << std::endl;
	//	std::cout << _ttoi(kDblClickWidth.c_str()) << std::endl;
	//	std::cout << _ttoi(kDblClickHeight.c_str()) << std::endl;
	//}
#define MAX_KEY_LENGTH 255
#define MAX_VALUE_NAME 16383
int QueryKey(HKEY hKey, TCHAR *szSubKey, std::wstring &rkRet)
{ 
	//TCHAR    achKey[MAX_KEY_LENGTH];   // buffer for subkey name
	//DWORD    cbName;                   // size of name string 
	TCHAR    achClass[MAX_PATH] = TEXT("");  // buffer for class name 
	DWORD    cchClassName = MAX_PATH;  // size of class string 
	DWORD    cSubKeys = 0;             // number of subkeys 
	DWORD    cbMaxSubKey;              // longest subkey size 
	DWORD    cchMaxClass;              // longest class string 
	DWORD    cValues;              // number of values for key 
	DWORD    cchMaxValue;          // longest value name 
	DWORD    cbMaxValueData;       // longest value data 
	DWORD    cbSecurityDescriptor; // size of security descriptor 
	FILETIME ftLastWriteTime;      // last write time 

	DWORD i = 0, retCode = 0; 
	TCHAR  achValue[MAX_VALUE_NAME]; 
	DWORD cchValue = MAX_VALUE_NAME; 

	// Get the class name and the value count.
	retCode = RegQueryInfoKey(
		hKey,                    // key handle 
		achClass,                // buffer for class name 
		&cchClassName,           // size of class string 
		NULL,                    // reserved 
		&cSubKeys,               // number of subkeys 
		&cbMaxSubKey,            // longest subkey size 
		&cchMaxClass,            // longest class string 
		&cValues,                // number of values for this key 
		&cchMaxValue,            // longest value name 
		&cbMaxValueData,         // longest value data 
		&cbSecurityDescriptor,   // security descriptor 
		&ftLastWriteTime);       // last write time 

	// Enumerate the subkeys, until RegEnumKeyEx fails.

	/*
	if (cSubKeys)
	{
		printf( "\nNumber of subkeys: %d\n", cSubKeys);

		for (i=0; i<cSubKeys; i++) 
		{ 
			cbName = MAX_KEY_LENGTH;
			retCode = RegEnumKeyEx(hKey, i, achKey, &cbName,  NULL, NULL, NULL, &ftLastWriteTime); 
			if (retCode == ERROR_SUCCESS) 
			{
				_tprintf(TEXT("(%d) %s\n"), i+1, achKey);
			}
		}
	}
	*/
 
	// Enumerate the key values.
	if (cValues) 
	{
		//printf( "\nNumber of values: %d\n", cValues);

		for (i=0, retCode=ERROR_SUCCESS; i<cValues; i++) 
		{
			cchValue = MAX_VALUE_NAME;
			achValue[0] = '\0';

			TCHAR szVal[MAX_PATH] = {0, };
			DWORD dwValSize = MAX_PATH;
			DWORD dwType = REG_SZ;
			retCode = RegEnumValue(hKey, i, achValue, &cchValue,  NULL, &dwType, (BYTE*)szVal, &dwValSize);

			if(ERROR_SUCCESS==retCode && 0==_tcscmp(szSubKey, achValue))
			{ 
				switch(dwType)
				{
				case REG_SZ:
					{
						//_tcscpy_s(szString, MAX_PATH, szVal);
						rkRet = szVal;
						return REG_SZ;
					}break;
				case REG_DWORD:
					{
						TCHAR szTemp[255] = {0, };
						DWORD dwVal = *((DWORD*)szVal);
						errno_t eRet = _itot_s(dwVal, szTemp, 255, 10);
						if(eRet==0)
						{
							rkRet = szTemp;
							return REG_DWORD;
						}
						break;
					}
				default:
					{
					}break;
				}
			}
		}
	}
	return -1;
}

bool PgUIScene::CallESCScript()
{
	if ( m_kEscKeyScript.empty() )
		return false;

	lua_tinker::call< void >( m_kEscKeyScript.c_str() );
	ClearESCScript();
	return true;
}

void PgUIScene::RemoveModel(char const *pcModelName)
{
	std::string kModelName(pcModelName);
	UIModelContainer::iterator itr = m_kUIModelContainer.find(kModelName);

	if(itr != m_kUIModelContainer.end())
	{
		NiDelete(itr->second);
		m_kUIModelContainer.erase(itr);
	}
}

bool PgUIScene::ParseXmlForMission(char const* pcFileName)
{
	if (!pcFileName)
	{
		return false;
	}

	TiXmlDocument kXmlDoc(pcFileName);

	if(!PgXmlLoader::LoadFile(kXmlDoc, UNI(pcFileName)))
	{
		PgError1("Parse Failed [%s]", pcFileName);
		return false;
	}

	const TiXmlElement *pkElement = kXmlDoc.FirstChildElement();
	PG_ASSERT_LOG(strcmp(pkElement->Value(), "MISSION_IMG") == 0);

	pkElement = pkElement->FirstChildElement();

	while (pkElement)
	{
		char const *pcTagName = pkElement->Value();

		if(strcmp(pcTagName, "MISSION") == 0)
		{
			char const *pcID = 0;
			char const *pcPath = 0;
			char const *pcExt = 0;
			const TiXmlAttribute *pkAttr = pkElement->FirstAttribute();
			while (pkAttr)
			{
				char const *pcAttrName = pkAttr->Name();
				char const *pcAttrValue = pkAttr->Value();

				if(strcmp(pcAttrName, "ID") == 0)
				{
					pcID = pcAttrValue;
				}
				else if(strcmp(pcAttrName, "PATH") == 0)
				{
					pcPath = pcAttrValue;
				}
				else if(strcmp(pcAttrName, "EXTENSION") == 0)
				{
					pcExt = pcAttrValue;
				}
				pkAttr = pkAttr->Next();
			}

			if(!pcID || !pcPath)
			{
				PgXmlError(pkElement, "Not Enough Effect Data");
			}
			else
			{
				SMissionImgInfo kTempStruct;
				kTempStruct.iID = atoi(pcID);
				kTempStruct.strPath = pcPath;
				if (!pcExt)
				{
					kTempStruct.strExtension = "tga";
				}
				else
				{
					kTempStruct.strExtension = pcExt;
				}
				
				m_kMissionImgCont.insert(std::make_pair(kTempStruct.iID, kTempStruct));
			}
		}
		else
		{
			PgXmlError1(pkElement, "XmlParse: Incoreect Tag '%s'", pcFileName);
		}

		pkElement = pkElement->NextSiblingElement();
	}
	return true;
}

std::string PgUIScene::GetMissionMapPath(int iID)
{
	MissionImgContIt it = m_kMissionImgCont.find(iID);
	if (it != m_kMissionImgCont.end())
	{
		const SMissionImgInfo kInfo = (*it).second;
		return kInfo.strPath;
	}

	return std::string("");
}

bool PgUIScene::ParseXmlForLoadingImg(char const* pcszFileName)
{
	if (!pcszFileName)
	{
		return false;
	}

	TiXmlDocument kXmlDoc(pcszFileName);

	if(!PgXmlLoader::LoadFile(kXmlDoc, UNI(pcszFileName)))
	{
		PgError1("Parse Failed [%s]", pcszFileName);
		return false;
	}

	const TiXmlElement *pkElement = kXmlDoc.FirstChildElement();
	PG_ASSERT_LOG(strcmp(pkElement->Value(), "IMAGE_LINK") == 0);

	pkElement = pkElement->FirstChildElement();
	while(pkElement)
	{
		char const *pcTagName = pkElement->Value();

		if(strcmp(pcTagName, "ITEM") == 0)
		{
			int iID	= 0;
			int LoadingImageType = 1;
			char const *pcPath = pkElement->GetText();
			const TiXmlAttribute *pkAttr = pkElement->FirstAttribute();
			while (pkAttr)
			{
				char const *pcAttrName = pkAttr->Name();
				char const *pcAttrValue = pkAttr->Value();

				if(strcmp(pcAttrName, "ID") == 0)
				{
					iID = atoi(pcAttrValue);
				}
				else if( strcmp( pcAttrName, "TYPE") == 0)
				{
					LoadingImageType = atoi(pcAttrValue);
				}
				pkAttr = pkAttr->Next();
			}

			if(((LoadingImageType > 0) && (iID == 0)) || !pcPath)
			{
				PgXmlError(pkElement, "Not Enough Effect Data");
			}
			else
			{
				std::string strPath(pcPath);
				switch( LoadingImageType )
				{
				case 0:
					{
						m_kBasicLoadingImgContainer.insert(std::make_pair((SLoadingImgType)iID, strPath));
					}break;
				case 1:
					{
						m_kLoadingImgContainerTargetMap.insert(std::make_pair(iID, strPath));
					}break;
				case 2:
					{
						m_TriggerLoadingImgContainer.insert(std::make_pair(iID, strPath));
					}break;
				case 3:
					{
						m_kLoadingImgContainerSourceMap.insert(std::make_pair(iID, strPath));
					}break;
				default:
					{
						PgXmlError(pkElement, "Not Find LoadingImageType");
					}break;
				}
			}
		}
		else
		{
			PgXmlError1(pkElement, "XmlParse: Incoreect Tag '%s'", pcszFileName);
		}
		pkElement = pkElement->NextSiblingElement();
	}
	return true;
}

char const* PgUIScene::FindLoadingImg(int iMapIndex, EGndAttr const eMapAttr, int LoadingImageID, bool bIsRidingPet, int iOldMapNo)
{
	if( LoadingImageID )
	{
		UITriggerLoadingImageMap::iterator	iter = m_TriggerLoadingImgContainer.find(LoadingImageID);
		if( iter != m_TriggerLoadingImgContainer.end() )
		{
			return iter->second.c_str();
		}
	}

	if(bIsRidingPet) //PC액터가 유효하고(캐선다음과정이고) 라이딩펫에 탑승 중이면, 우선적으로 탑승용 이미지를 사용
	{
		UIBasicLoadingImageMap::iterator	basic_iter = m_kBasicLoadingImgContainer.find(LOADING_IMG_PetLD);
		if( basic_iter != m_kBasicLoadingImgContainer.end() )
		{
			return basic_iter->second.c_str();
		}
	}

	bool const bShowNewRace = lua_tinker::call<bool>("GetNewRaceLoadingImg");
	if( bShowNewRace )
	{
		UIBasicLoadingImageMap::iterator	basic_iter = m_kBasicLoadingImgContainer.find(LOADING_IMG_NEW_RACE);
		if( basic_iter != m_kBasicLoadingImgContainer.end() )
		{
			lua_tinker::call<void, bool>("SetNewRaceLoadingImg", false);
			return basic_iter->second.c_str();
		}
	}

	if(iOldMapNo > 0)
	{
		UILoadingImageMap::const_iterator src_itr = m_kLoadingImgContainerSourceMap.find(iOldMapNo);
		if(0 != iMapIndex && src_itr != m_kLoadingImgContainerSourceMap.end())
		{
			return src_itr->second.c_str();
		}
	}

	UILoadingImageMap::iterator itr = m_kLoadingImgContainerTargetMap.find(iMapIndex);
	if(0 != iMapIndex && itr != m_kLoadingImgContainerTargetMap.end())
	{
		return itr->second.c_str();
	}

	SLoadingImgType	Type = LOADING_IMG_DEFAULT;
	switch(eMapAttr)
	{
	case GATTR_DEFAULT:
	case GATTR_INSTANCE:
	case GATTR_STATIC_DUNGEON:		{ Type = LOADING_IMG_FILED;		}break;
	case GATTR_FLAG_PVP:
	case GATTR_PVP:					{ Type = LOADING_IMG_PVP;		}break;
	case GATTR_FLAG_MISSION:
	case GATTR_MISSION:				{ Type = LOADING_IMG_MISSION;	}break;
	case GATTR_FLAG_BOSS:
	case GATTR_BOSS:				{ Type = LOADING_IMG_DRAGON;	}break;
	case GATTR_FLAG_RESULT:			{ Type = LOADING_IMG_RESULT;	}break;
	case GATTR_FLAG_EMPORIABATTLE:
	case GATTR_EMPORIABATTLE:		{ Type = LOADING_IMG_EMBATTLE;	}break;
	case GATTR_HIDDEN_F:			{ Type = LOADING_IMG_HIDDEN;	}break;
	case GATTR_CHAOS_F:				{ Type = LOADING_IMG_CHAOS;		}break;
	case GATTR_FLAG_MYHOME:
	//case GATTR_MYHOME:				{ Type = LOADING_IMG_MYHOME;	}break;
	default:
		{
		}break;
	}

	UIBasicLoadingImageMap::iterator	basic_iter = m_kBasicLoadingImgContainer.find(Type);
	if( basic_iter != m_kBasicLoadingImgContainer.end() )
	{
		return basic_iter->second.c_str();
	}

	//없으면 기본으로
	if( m_kBasicLoadingImgContainer.empty() )
	{
		return NULL;
	}
	return m_kBasicLoadingImgContainer.begin()->second.c_str();
}

void PgUIScene::GetLoadingImgeIDList(LOADINGIMG_ID_LIST& List)
{
	UILoadingImageMap::iterator src_itr = m_kLoadingImgContainerSourceMap.begin();
	while( src_itr != m_kLoadingImgContainerSourceMap.end() )
	{
		List.push_back(src_itr->second);
		++src_itr;
	}

	UILoadingImageMap::iterator itr = m_kLoadingImgContainerTargetMap.begin();
	while( itr != m_kLoadingImgContainerTargetMap.end() )
	{
		List.push_back(itr->second);
		++itr;
	}

	UIBasicLoadingImageMap::iterator	basic_iter = m_kBasicLoadingImgContainer.begin();
	while( basic_iter != m_kBasicLoadingImgContainer.end() )
	{
		List.push_back(basic_iter->second);
		++basic_iter;
	}

	UITriggerLoadingImageMap::iterator iter = m_TriggerLoadingImgContainer.begin();
	while( iter != m_TriggerLoadingImgContainer.end() )
	{
		List.push_back(iter->second);
		++iter;
	}
}

void PgUIScene::InputHook( bool (*pInputHookFunc)(int const,XUI::E_INPUT_EVENT_INDEX) )
{
	if ( pInputHookFunc )
	{
		m_pInputHookFunc = pInputHookFunc;
	}
	else
	{
		m_pInputHookFunc = FInputHook;
	}
}

bool PgUIScene::RegistUIAction(std::wstring const &rkName, SUIActinByDist &rkItem)
{
	std::pair<UIActinByDistMap::iterator, bool> ib = m_kUIDistActionList.insert(std::make_pair(rkName, rkItem));
	return ib.second;
}

bool PgUIScene::RegistUIAction(std::wstring const &rkName, std::wstring const &rkScript, NiPoint3 const &pt3)
{
	float const fClientDist = PgNpcTalkUtil::fNPC_TALK_ENABLE_RANGE - 40.f; // 서버 거리보다 적게
	return RegistUIAction(rkName, rkScript, pt3, fClientDist);
}

bool PgUIScene::RegistUIAction(std::wstring const &rkName, std::wstring const &rkScript, NiPoint3 const &pt3, float fDist)
{
	SUIActinByDist kNew;
	kNew.m_wstrUIName = rkName;
	kNew.m_wstrScript = rkScript;
	kNew.m_kOriginPoint.x = pt3.x;
	kNew.m_kOriginPoint.y = pt3.y;
	kNew.m_kOriginPoint.z = pt3.z;
	kNew.m_fDist = fDist;	

	std::pair<UIActinByDistMap::iterator, bool> ib = m_kUIDistActionList.insert(std::make_pair(rkName, kNew));
	return ib.second;
}

bool PgUIScene::DelUIAction(std::wstring const &rkName)
{
	UIActinByDistMap::iterator ui_iter = m_kUIDistActionList.find(rkName);
	if (m_kUIDistActionList.end() != ui_iter)
	{
		(*ui_iter).second.m_bRegistDelete = true;
		return true;
	}

	return false;
}

void PgUIScene::DelAllUIAction()
{
	m_kUIDistActionList.clear();
}

bool PgCursour::HideCursor(XUI::E_CURSOR_STATE const eAtThisState)
{
	{// XUIMgr에서 얻어오는것은 VCreate()함수를 사용하는데 커서의 것은 싱글톤을 반환하게 되어있다.
	 // 그러니 걱정말고 이것으로 부터 상태를 확인하면 된다
		XUI::CXUI_Cursor* pCursor = dynamic_cast<XUI::CXUI_Cursor*>(CXUI_Cursor::GetInstance());
		if(pCursor)
		{// 
			if(eAtThisState != pCursor->CursorState())
			{// 커서의 상태가 인자값의 상태와 같을때만 이 함수를 수행한다
				return false;
			}
		}
		else
		{// singletone이 없다?
			_PgMessageBox("PgCursour::HideCursor()", "CXUI_Cursor::GetInstance() is NULL");
			return false;
		}
	}
	XUI::CXUI_Wnd* pCursor = XUIMgr.Get(WSTR_XUI_CURSOR);
	if(pCursor)
	{
		pCursor->Visible(false);
		m_bHide = true;
		return true;
	}
	return false;	
}

bool PgCursour::ShowCursor(XUI::E_CURSOR_STATE const eAtThisState)
{
	{// XUIMgr에서 얻어오는것은 VCreate()함수를 사용하는데 커서의 것은 싱글톤을 반환하게 되어있다.
	 // 그러니 걱정말고 이것으로 부터 상태를 확인하면 된다
		XUI::CXUI_Cursor* pCursor = dynamic_cast<XUI::CXUI_Cursor*>(CXUI_Cursor::GetInstance());
		if(pCursor)
		{
			if(eAtThisState != pCursor->CursorState())
			{// 커서의 상태가 인자값의 상태와 같을때만 이 함수를 수행한다.
				return false;
			}
		}
		else
		{// singletone이 없다?
			_PgMessageBox("PgCursour::ShowCursor()", "CXUI_Cursor::GetInstance() is NULL");
			return false;
		}
	}
	// 커서를 검색해서
	XUI::CXUI_Wnd* pCursor = XUIMgr.Get(WSTR_XUI_CURSOR);
	if(NULL == pCursor)
	{// 커서가 없다면 제거된 상태이므로 호출해야 한다.		
		XUIMgr.Call(WSTR_XUI_CURSOR);
		m_bHide = false;
		return true;		
	}
//	else
//	{// 찾았으면 커서는 화면에 보여지고 있으므로 손댈 필요 없다.//}
	return false;
}