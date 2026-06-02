#include "StdAfx.h"
#include "lwUIItemRarityAmplify.h"
#include "lwUIToolTip.h"
#include "PgUIScene.h"
#include "PgPilotMan.h"
#include "PgNetwork.h"
#include "PgMobileSuit.h"
#include "PgNifMan.h"
#include "Variant/PgItemRarityUpgradeFormula.h"
#include "PgSoundMan.h"

extern void lwUIItemRarityAmplify::RegisterWrapper( lua_State *pkState )
{
	using namespace lua_tinker;
	def(pkState, "CallItemAmplifySelectUI", lwUIItemRarityAmplify::CallItemAmplifySelectUI);
	def(pkState, "CallItemAmplifyUI", lwUIItemRarityAmplify::CallItemAmplifyUI);
	def(pkState, "Clear_RarityAmplify", lwUIItemRarityAmplify::Clear);
	def(pkState, "GetAmplifyNeedMoney", lwUIItemRarityAmplify::GetAmplifyNeedMoney);
	def(pkState, "OnDisplay_RarityAmplify", lwUIItemRarityAmplify::OnDisplay);
	def(pkState, "OnTick_RarityAmplify", lwUIItemRarityAmplify::OnTick);
	def(pkState, "DisplaySrcIcon_RarityAmplify", lwUIItemRarityAmplify::DisplaySrcIcon);
	def(pkState, "DisplayNeedItemIcon_RarityAmplify", lwUIItemRarityAmplify::DisplayNeedItemIcon);
	def(pkState, "UseInsureItem_RarityAmplify", lwUIItemRarityAmplify::UseInsureItem);
	def(pkState, "Start_RarityAmplify", lwUIItemRarityAmplify::Start);
	def(pkState, "Cancle_RarityAmplify", lwUIItemRarityAmplify::Cancle);
	def(pkState, "Check_RarityAmplify", lwUIItemRarityAmplify::Check);
	def(pkState, "IsUseInsureItem_RarityAmplify", lwUIItemRarityAmplify::IsUseInsureItem);
}

void lwUIItemRarityAmplify::CallItemAmplifySelectUI()
{
	CXUI_Wnd* pkTopWnd = XUIMgr.Call(L"ITEM_RARITY_AMPLIFY_SELECT");
	if( pkTopWnd )
	{
		RegistUIAction(pkTopWnd);
	}
}

void lwUIItemRarityAmplify::CallItemAmplifyUI()
{
	CXUI_Wnd* pkTopWnd = XUIMgr.Call(L"SFRM_ITEM_RARITY_AMPLIFY");
	if( pkTopWnd )
	{
		RegistUIAction(pkTopWnd);
	}
}

void lwUIItemRarityAmplify::Clear()
{
	g_kItemRarityAmplify.Clear();
}

__int64 lwUIItemRarityAmplify::GetAmplifyNeedMoney()
{
	return g_kItemRarityAmplify.GetAmplifyNeedMoney();
}

void lwUIItemRarityAmplify::OnDisplay()
{
	g_kItemRarityAmplify.OnDisplay();
}

void lwUIItemRarityAmplify::OnTick( POINT2 kPoint )
{
	g_kItemRarityAmplify.OnTick(kPoint);
}

void lwUIItemRarityAmplify::DisplaySrcIcon()
{
	g_kItemRarityAmplify.DisplaySrcIcon();
}

void lwUIItemRarityAmplify::DisplayNeedItemIcon( int const iNeedIndex, POINT2 kPoint )
{
	g_kItemRarityAmplify.DisplayNeedItemIcon(iNeedIndex, kPoint);
}

void lwUIItemRarityAmplify::UseInsureItem(bool bUse)
{
	g_kItemRarityAmplify.CheckInsureItem(bUse);
}

bool lwUIItemRarityAmplify::IsUseInsureItem()
{
	return g_kItemRarityAmplify.UseInsureItem();
}

void lwUIItemRarityAmplify::Start()
{
	g_kItemRarityAmplify.Start();
}

extern bool lwUIItemRarityAmplify::Cancle()
{
	return g_kItemRarityAmplify.Cancle();
}

extern bool lwUIItemRarityAmplify::Check()
{
	return g_kItemRarityAmplify.Check();
}
//////////////////////////////////////////////////////////////////////////
// PgUIItemRarityAmplify
//////////////////////////////////////////////////////////////////////////
char const UIMODEL_RARITY_EFFECT_NAME[] = "ef_UImix_01";
char const UIMODEL_RARITY_EFFECT_NIF_NAME[] = "ef_UImix_01_NIF";
char const UIMODEL_RARITY_EFFECT_PATH[] = "../Data/5_Effect/4_UI/ef_UImix_01_spin.nif";
char const UIMODEL_RARITY_EFFECT_NIF_SUCC_NAME[] = "ef_UImix_01_SUCC_NIF";
char const UIMODEL_RARITY_EFFECT_SUCC_PATH[] = "../Data/5_Effect/4_UI/ef_UImix_02_succ.nif";
char const UIMODEL_RARITY_EFFECT_NIF_FAIL_NAME[] = "ef_UImix_01_FAIL_NIF";
char const UIMODEL_RARITY_EFFECT_FAIL_PATH[] = "../Data/5_Effect/4_UI/ef_UImix_03_fail.nif";
float const RARITY_PROGRESS_TIME = 0.667f; //진행 시간은 1초
float const UIMODEL_RARITY_EFFECT_RESULT_TIME = 0.53f;	//결과 이펙트 지속 시간
const POINT2 UIMODEL_RARITY_POS(0, 0);
int const SOUL_ITEM_NO = 79000030;
int const ISURANCE_ITEM_NO_BASE = 98000040;
int const AMPLIFY_ITEM_NO = 99600110;

PgUIItemRarityAmplify::PgUIItemRarityAmplify()
{
	Clear();
	InitUIModel();
}

void PgUIItemRarityAmplify::Clear()
{
	m_bProcessStart = false;
	m_bUseInsureItem = false;
	m_fStartTime = NULL;
	m_iNeedItemNo = NULL;
	m_hResult = E_FAIL;

	m_kTargetItem.Clear();
	m_kTargetItemPos.Clear();
	m_kInsureItemPos.Clear();
	m_kSoulItem.Clear();
	m_kSoulItemPos.Clear();

	XUI::CXUI_Wnd* pkWnd = XUIMgr.Get( L"SFRM_ITEM_RARITY_AMPLIFY" );
	if (!pkWnd)
	{
		return;
	}

	for (int i = 0; i < 4; ++i)
	{
		BM::vstring kString(L"ICN_MATERIAL");
		kString+=i;
		XUI::CXUI_Wnd* pkMat = pkWnd->GetControl(kString);
		if (pkMat)
		{
			lwUIWnd(pkMat).SetCustomData<int>(0);
		}

		kString = L"BTN_REG";
		kString += i;
		XUI::CXUI_Wnd* pReg = pkWnd->GetControl(kString);
		if( pReg )
		{
			pReg->Visible(false);
		}

		kString = L"BTN_DEREG";
		kString += i;
		XUI::CXUI_Wnd* pDeReg = pkWnd->GetControl(kString);
		if( pReg )
		{
			pDeReg->Visible(false);
		}

		kString = L"ICN_MATERIAL_";
		kString+=i;
		XUI::CXUI_Wnd* pkIcon = pkWnd->GetControl(kString);
		if (pkIcon)
		{
			lwUIWnd(pkIcon).SetCustomData<int>(0);
		}
	}

	XUI::CXUI_Wnd* pkSrc = pkWnd->GetControl(L"ICN_SRC");
	if (pkSrc)
	{
		lwUIWnd(pkSrc).SetCustomData<int>(0);
	}

	XUI::CXUI_Wnd* pkSdw = pkWnd->GetControl(L"SFRM_SHADOW");
	if (pkSdw)
	{
		pkSdw->Text(L"");
	}

	pkWnd->SetInvalidate();
}


__int64 PgUIItemRarityAmplify::GetAmplifyNeedMoney()
{
	SDefBasicOptionAmp const * pkOptionAmpInfo = GetBasicOptionAmp();
	if(pkOptionAmpInfo)
	{
		return pkOptionAmpInfo->iNeedCost;
	}

	return 0;
}

SDefBasicOptionAmp const * PgUIItemRarityAmplify::GetBasicOptionAmp()const
{
	if(m_kTargetItem.IsEmpty())
	{
		return NULL;	
	}

	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const *pkItemDef = kItemDefMgr.GetDef(m_kTargetItem.ItemNo());
	if( !pkItemDef )
	{
		return NULL;
	}

	SEnchantInfo const& kEnchantInfo = m_kTargetItem.EnchantInfo();
	return PgItemRarityUpgradeFormula::GetBasicOptionAmp(GetEquipType(pkItemDef), pkItemDef->GetAbil(AT_LEVELLIMIT), kEnchantInfo.BasicAmpLv()+1);
}

void PgUIItemRarityAmplify::OnDisplay()
{
	if (m_bProcessStart || E_FAIL != m_hResult)
	{
		if(m_pkWndUIModel)
		{
			g_kUIScene.AddToDrawListRenderModel(UIMODEL_RARITY_EFFECT_NAME);
		}
	}
}

void PgUIItemRarityAmplify::OnTick(POINT2 kPoint)
{
	if (m_bProcessStart && RARITY_PROGRESS_TIME < g_pkApp->GetAccumTime() - m_fStartTime )
	{
		m_fStartTime = NULL;
		m_bProcessStart = false;
		Send_ReqAmplify();
		ProcessEffect(false);
	}
	else
	{
		if ( m_bProcessStart )
		{
			if(m_pkWndUIModel)
			{
				m_pkWndUIModel->RenderFrame(NiRenderer::GetRenderer(), kPoint);
			}
		}
		else
		{
			if (UIMODEL_RARITY_EFFECT_RESULT_TIME < g_pkApp->GetAccumTime() - m_fStartTime)
			{
				m_hResult = E_FAIL;
			}
			else if ( E_FAIL != m_hResult )
			{
				if(m_pkWndUIModel)
				{
					m_pkWndUIModel->RenderFrame(NiRenderer::GetRenderer(), kPoint);
				}
			}
		}
	}

	// 아이템 보유 수량 등의 설명 텍스트를 업데이트 해줌..
	UpdateExplainText();
}

void PgUIItemRarityAmplify::UpdateExplainText()
{
	XUI::CXUI_Wnd* pkWndTop = XUIMgr.Get(L"SFRM_ITEM_RARITY_AMPLIFY");
	if( !pkWndTop )
	{
		return;	
	}

	XUI::CXUI_Wnd* pWndText = pkWndTop->GetControl(L"SFRM_SHADOW");
	if( !pWndText )
	{
		return;
	}

	//하단에 나오는 현재 가지고 있는 소울 수량 등의 설명을 써줘야함....
	int iItemNo = NULL;
	BM::vstring vStrText(::TTW(1540)), vStrChangeText;

	int iNeedItemCount = GetNeedSoulCount();
	int iHaveItemCount = GetHaveSoulCount();
	if( iNeedItemCount < iHaveItemCount )
	{
		iHaveItemCount = iNeedItemCount;
	}
	vStrText << L"\n" << ::TTW(1541);
	vStrChangeText = iHaveItemCount;
	vStrText.Replace(L"#Have#", vStrChangeText);
	vStrChangeText = iNeedItemCount;
	vStrText.Replace(L"#Need#", vStrChangeText);


	iNeedItemCount = GetNeedAmplifyCount();
	iHaveItemCount = GetHaveAmplifyCount(iItemNo);
	if( iNeedItemCount < iHaveItemCount )
	{
		iHaveItemCount = iNeedItemCount;
	}
	vStrText << L"\n" << ::TTW(1542);
	vStrChangeText = iHaveItemCount;
	vStrText.Replace(L"#Have#", vStrChangeText);
	vStrChangeText = iNeedItemCount;
	vStrText.Replace(L"#Need#", vStrChangeText);
	pWndText->Text(vStrText);
}

void PgUIItemRarityAmplify::DisplaySrcIcon()
{
	XUI::CXUI_Wnd* pkWndTop = XUIMgr.Get(_T("SFRM_ITEM_RARITY_AMPLIFY"));
	if( !pkWndTop )
	{
		return;
	}

	XUI::CXUI_Wnd* pkWndSrc = pkWndTop->GetControl(_T("ICN_SRC"));
	if( !pkWndSrc )
	{
		return;
	}

	if( m_kTargetItem.IsEmpty() )
	{
		int iNull = NULL;
		pkWndSrc->SetCustomData(&iNull, sizeof(iNull));
		m_kTargetItem.Clear();
		m_kTargetItemPos.Clear();
		return;
	}

	POINT2 kPoint = pkWndSrc->GetTotalLocation();

	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const *pItemDef = kItemDefMgr.GetDef(m_kTargetItem.ItemNo());
	if(pItemDef)
	{
		g_kUIScene.RenderIcon( pItemDef->ResNo(), kPoint, false );
	}

	pkWndSrc->SetCustomData(&m_kTargetItem.ItemNo(), sizeof(m_kTargetItem.ItemNo()));
}



void PgUIItemRarityAmplify::DisplayNeedItemIcon(int const iNeedIndex, POINT2 kPointLocation)
{
	XUI::CXUI_Wnd* pkWndTop = XUIMgr.Get(_T("SFRM_ITEM_RARITY_AMPLIFY"));
	if( !pkWndTop )
	{
		return;
	}

	int iNull = NULL;
	if( m_kTargetItem.IsEmpty() )
	{
		pkWndTop->SetCustomData(&iNull, sizeof(iNull));
		pkWndTop->Text(std::wstring(L""));
		return;
	}

	int iItemNo = 0;
	bool bGray = false;	//흑백으로 그릴지
	switch(iNeedIndex)
	{
	case AIT_SOUL:
		{//소울
			if( GetHaveSoulCount() < GetNeedSoulCount() )
			{//갯수 모자라면 흑백임
				bGray = true;
			}
			iItemNo = SOUL_ITEM_NO;
		}break;
	case AIT_NEED_ITEM:	
		{//옵션 증폭기
			int iNeedItemNo = 0;
			if( GetHaveAmplifyCount(iNeedItemNo) < GetNeedAmplifyCount() )
			{
				bGray = true;
			}
			iItemNo = iNeedItemNo;//AMPLIFY_ITEM_NO

		}break;
	case AIT_INSUR_ITEM:
		{ //인챈트 보험 스크롤
			int iInsureItemNo;
			SItemPos kInsureItemPos;
			if( !GetHaveInsureItem(iInsureItemNo, kInsureItemPos) )
			{
				bGray = true;
				UseInsureItem(false);
			}

			XUI::CXUI_Wnd* pkWndDeReg = pkWndTop->GetControl(L"BTN_DEREG3");
			XUI::CXUI_Wnd* pkWndReg = pkWndTop->GetControl(L"BTN_REG3");
			if( pkWndDeReg && pkWndReg)
			{
				pkWndDeReg->Visible(m_bUseInsureItem);
				pkWndReg->Visible(!m_bUseInsureItem);
				if( !m_bUseInsureItem )
				{
					bGray = true;
				}
			}

			iItemNo = iInsureItemNo;//ISURANCE_ITEM_NO_BASE
		}break;
	default:
		{
			pkWndTop->SetCustomData(&iNull, sizeof(iNull));
			pkWndTop->Text(std::wstring(L""));
			return;
		}
	}

	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkPlayer )
	{ 
		return; 
	}

	PgInventory* pkInv = pkPlayer->GetInven();
	if( !pkInv )
	{  
		return; 
	}

	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const *pItemDef = kItemDefMgr.GetDef(iItemNo);
	if(pItemDef)
	{
		g_kUIScene.RenderIcon(pItemDef->ResNo(), kPointLocation, false, 40, 40, bGray);
	}

	BM::vstring kString(L"ICN_MATERIAL_");
	kString+=iNeedIndex;
	XUI::CXUI_Wnd* pkIcon = pkWndTop->GetControl(kString);
	if (pkIcon)
	{
		pkIcon->SetCustomData(&(iItemNo), sizeof(iItemNo));
	}
	
	return;
}

void PgUIItemRarityAmplify::UseInsureItem(bool bUse)
{
	m_bUseInsureItem = bUse;
}

bool PgUIItemRarityAmplify::UseInsureItem()const
{
	return m_bUseInsureItem;
}

void PgUIItemRarityAmplify::CheckInsureItem(bool const bUse)
{
	bool bRealUse = bUse;
	if(bUse)
	{
		if(0 == GetHaveInsureItem(m_iInsureItemNo, m_kInsureItemPos))
		{
			bRealUse = false;
			lwAddWarnDataTT(1555);
		}
	}
	UseInsureItem(bRealUse);
}

size_t PgUIItemRarityAmplify::GetHaveSoulCount()
{
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkPlayer )
	{
		return 0;
	}
	PgInventory *pkInv = pkPlayer->GetInven();
	if( !pkInv )
	{
		return 0;
	}

	return pkInv->GetTotalCount(SOUL_ITEM_NO);
}

int PgUIItemRarityAmplify::GetNeedSoulCount()
{
	SDefBasicOptionAmp const * pkOptionAmpInfo = GetBasicOptionAmp();
	if(pkOptionAmpInfo)
	{
		return pkOptionAmpInfo->iNeedSoulCount;
	}
	return 0;
}

int PgUIItemRarityAmplify::GetHaveAmplifyCount(int& iOutAmplifyItemNo)
{
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkPlayer )
	{
		return 0;
	}

	PgInventory *pkInv = pkPlayer->GetInven();
	if( !pkInv )
	{ 
		return 0;
	}

	SDefBasicOptionAmp const * pkOptionAmpInfo = GetBasicOptionAmp();
	if( !pkOptionAmpInfo )
	{
		return 0;
	}

	iOutAmplifyItemNo = pkOptionAmpInfo->iAmpItemNo;
	return pkInv->GetTotalCount(iOutAmplifyItemNo);
}
int PgUIItemRarityAmplify::GetNeedAmplifyCount()
{
	SDefBasicOptionAmp const * pkOptionAmpInfo = GetBasicOptionAmp();
	if(pkOptionAmpInfo)
	{
		return pkOptionAmpInfo->iAmpItemCount;
	}
	return 0;
}

void PgUIItemRarityAmplify::InitUIModel()
{
	m_pkWndUIModel = NULL;
	m_pkWndUIModel_Result = NULL;

	g_kUIScene.InitRenderModel(UIMODEL_RARITY_EFFECT_NAME, POINT2(230,230), UIMODEL_RARITY_POS, false);
	m_pkWndUIModel = g_kUIScene.FindUIModel(UIMODEL_RARITY_EFFECT_NAME);
	if (m_pkWndUIModel)
	{
		m_pkWndUIModel->AddNIF(UIMODEL_RARITY_EFFECT_NIF_NAME, g_kNifMan.GetNif(UIMODEL_RARITY_EFFECT_PATH), false, true);
		m_pkWndUIModel->AddNIF(UIMODEL_RARITY_EFFECT_NIF_SUCC_NAME, g_kNifMan.GetNif(UIMODEL_RARITY_EFFECT_SUCC_PATH), false, true);
		m_pkWndUIModel->AddNIF(UIMODEL_RARITY_EFFECT_NIF_FAIL_NAME, g_kNifMan.GetNif(UIMODEL_RARITY_EFFECT_FAIL_PATH), false, true);
		m_pkWndUIModel->SetCameraZoomMinMax(-300, 300);
		m_pkWndUIModel->CameraZoom(210.0f);
	}
}

void PgUIItemRarityAmplify::ProcessEffect(bool bRun)
{
	if (m_pkWndUIModel)
	{
		m_pkWndUIModel->SetNIFEnableUpdate(UIMODEL_RARITY_EFFECT_NIF_NAME,bRun);
		if (bRun)
		{
			m_pkWndUIModel->SetEnableUpdate(bRun);
			m_pkWndUIModel->ResetNIFAnimation(UIMODEL_RARITY_EFFECT_NIF_NAME);
			m_pkWndUIModel->RenderFrame(NiRenderer::GetRenderer(), UIMODEL_RARITY_POS);
			m_pkWndUIModel->SetNIFEnableUpdate(UIMODEL_RARITY_EFFECT_NIF_SUCC_NAME,!bRun);
			m_pkWndUIModel->SetNIFEnableUpdate(UIMODEL_RARITY_EFFECT_NIF_FAIL_NAME,!bRun);
		}
	}
}

void PgUIItemRarityAmplify::SetItem( EKindUIIconGroup const kType, SIconInfo const & rkInfo )
{
	if( m_bProcessStart )
	{
		lwAddWarnDataTT(1551);
	}
	if( 0 == rkInfo.iIconGroup || 0 > rkInfo.iIconKey)
	{
		return;
	}

	if( KUIG_ITEM_RARITY_AMPLIFY_SRC != kType )
	{
		return;
	}

	if( SetSrcItem(SItemPos(rkInfo.iIconGroup, rkInfo.iIconKey)) )
	{
		XUI::CXUI_Wnd* pkWnd = XUIMgr.Get( _T("SFRM_ITEM_RARITY_AMPLIFY") );
		if (!pkWnd)
		{
			return;
		}

		XUI::CXUI_Wnd* pkWndItem = pkWnd->GetControl(L"IMG_ITEM");
		if( pkWndItem )
		{//각종 무기모양 회색 아이콘. 아이템 아이콘으로 그려줄거니 false로 셋팅함
			pkWndItem->Visible(false);
		}

		XUI::CXUI_Wnd* pkBtnRefine = pkWnd->GetControl(L"BTN_TRY_REFINE");
		if( pkBtnRefine )
		{
			pkBtnRefine->Enable(true);
		}
	}
}

bool PgUIItemRarityAmplify::SetSrcItem(const SItemPos &rkItemPos)
{
	if( m_bProcessStart )
	{//이미 진행중임.
		lwAddWarnDataTT(1408);
		return false;
	}
	

	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkPlayer )
	{
		return false;
	}

	PgInventory *pkInv = pkPlayer->GetInven();
	if( !pkInv )
	{ 
		return false;
	}

	PgBase_Item kItem;
	if( S_OK != pkInv->GetItem(rkItemPos, kItem) )
	{// 템이 없엉..ㅜㅜ
		::Notice_Show(::TTW(1512), EL_Warning);
		return false;
	}

	E_ITEM_GRADE const eItemGrade = ::GetItemGrade(kItem);
	switch( eItemGrade )
	{
	case IG_SEAL:
	case IG_CURSE:
		{//봉인/저주는 사용 불가
			::Notice_Show(::TTW(1532), EL_Warning);
			return false;;
		}
	default: {}break;
	}

	if( kItem.EnchantInfo().IsBinding() )
	{//잠김 아이템은 사용 불가
		::Notice_Show(::TTW(1532), EL_Warning);
		return false;
	}

	switch(rkItemPos.x)
	{
	case KUIG_EQUIP:
	case KUIG_CASH:
		{
			if( LOCAL_MGR::NC_JAPAN == g_kLocal.ServiceRegion() 
				&& KUIG_CASH == rkItemPos.x )
			{//일본의 경우 캐시 아이템은 소울 크래프트 관련 작업 불가
				lwAddWarnDataTT(59002);
				return false;
			}

			GET_DEF(CItemDefMgr, kItemDefMgr);
			CItemDef const *pkItemDef = kItemDefMgr.GetDef(kItem.ItemNo());
			if( !pkItemDef )
			{
				return false;
			}

			if( false == FindCraftOption(kItem, pkItemDef) )
			{//증폭할 옵션이 존재하지 않음. 소울크래프트부터 하셈.
				::Notice_Show(::TTW(1515), EL_Warning);
				return false;
			}

			SEnchantInfo const& kEnchantInfo = kItem.EnchantInfo();
			if( !PgItemRarityUpgradeFormula::GetBasicOptionAmp(GetEquipType(pkItemDef), pkItemDef->GetAbil(AT_LEVELLIMIT), kEnchantInfo.BasicAmpLv()+1) )
			{//최대 레벨에 도달했음. 
				::Notice_Show(::TTW(1549), EL_Warning);
				return false;
			}

			//아이템을 등록
			m_kTargetItemPos = rkItemPos;
			m_kTargetItem = kItem;
		}break;
	case KUIG_FIT:
		{//장비중인건 못해여
			lwAddWarnDataTT(1406);
			return false;
		}break;
	case KUIG_CONSUME:
	case KUIG_ETC:
		{//소비나 기타템은 할수 없음.
			lwAddWarnDataTT(59007);
		}break;
	default: { }break;
	}

	// 강화기 아이템 번호 가져옴
	GetHaveAmplifyCount(m_iNeedItemNo);
	
	// 보험 스크롤 아이템 번호 가져옴
	GetHaveInsureItem(m_iInsureItemNo, m_kInsureItemPos);

	return true;
}

void PgUIItemRarityAmplify::Send_ReqAmplify()
{
//	PT_C_M_REQ_BASIC_OPTION_AMP
//	SItemPos : 대상 아이템 위치
//	SItemPos : 인첸트 보험 카드 위치
	SItemPos kInsureItemPos;
	if( m_bUseInsureItem )
	{//보험 쓸때만 제대로된 위치 정보를 보냄.
		kInsureItemPos = m_kInsureItemPos;
	}
	BM::Stream kPacket(PT_C_M_REQ_BASIC_OPTION_AMP);
	kPacket.Push(m_kTargetItemPos);
	kPacket.Push(kInsureItemPos);
	NETWORK_SEND(kPacket);
}

void PgUIItemRarityAmplify::Recv( BM::Stream &rkPacket )
{
	HRESULT kResult = EC_OK;
	if( !rkPacket.Pop(kResult) )
	{
		return;
	}
	
	bool bUseInsureItem = false;
	if( !rkPacket.Pop(bUseInsureItem) )
	{
		return;
	}

	XUI::CXUI_Wnd* pkWnd = XUIMgr.Get( _T("SFRM_ITEM_RARITY_AMPLIFY") );
	if (!pkWnd)
	{
		return;
	}

	XUI::CXUI_Wnd* pkBtnRefine = pkWnd->GetControl(L"BTN_TRY_REFINE");
	if( pkBtnRefine )
	{
		pkBtnRefine->Enable(true);
	}

	m_fStartTime = g_pkApp->GetAccumTime();
	RecentResult(kResult);

	int iNoticeTextTableNo = 0;
	int iNoticeLevel = EL_Warning;
	PgBase_Item kResultItem;
	char szName[100] = "EnchantFail";
	switch(kResult)
	{
	case E_BASICOPTIONAMP_SUCCESS:
		{//아이템옵션증폭성공			
			::Notice_Show(::TTW(1530), EL_Normal);

			kResultItem.ReadFromPacket(rkPacket);
			std::wstring wstrEnchantText;
			CONT_ABILS kContAbils;

			int iLevelLimit = 0;
			GET_DEF(CItemDefMgr, kItemDefMgr);
			CItemDef const *pkItemDef = kItemDefMgr.GetDef(kResultItem.ItemNo());	
			if( pkItemDef )
			{
				iLevelLimit = pkItemDef->GetAbil(AT_LEVELLIMIT);
			}

			if( GetChangeEnchantInfo(kContAbils, m_kTargetItem, kResultItem, iLevelLimit) )
			{//뭔가 옵션이 변했음. 공지로 보여줘야댐.
				if( pkItemDef ) 
				{//성공 값을 공지로 알려준다.
					NoticeShow_SuccessInfo(kResultItem, pkItemDef, kContAbils);
				}
			}

			SEnchantInfo const& kEnchantInfo = kResultItem.EnchantInfo();
			if( !PgItemRarityUpgradeFormula::GetBasicOptionAmp(GetEquipType(pkItemDef), iLevelLimit, kEnchantInfo.BasicAmpLv()+1) )
			{//아이템 업그레이드 최대 레벨에 도달하였습니다.
				::Notice_Show(::TTW(1552), EL_Normal);
				Clear();
				if( pkBtnRefine )
				{
					pkBtnRefine->Enable(false);
				}
				lwSetReqInvWearableUpdate(true);
			}
			else
			{//아이템 갱신
				m_kTargetItem = kResultItem;
			}

			sprintf(szName, "EnchantSuccess");
			g_kSoundMan.PlayAudioSourceByID(NiAudioSource::TYPE_3D, szName, 0.0f, 80, 100, g_kPilotMan.GetPlayerActor());
			return;
		}break;
	case E_BASICOPTIONAMP_FAIL_BROKEN:
		{//실패 아이템 증폭 정보 초기화
			iNoticeTextTableNo = 1536;
			kResultItem.ReadFromPacket(rkPacket);
			m_kTargetItem = kResultItem;
		}break;
	case E_BASICOPTIONAMP_FAIL:
		{//실패
			iNoticeTextTableNo = 1537;
			kResultItem.ReadFromPacket(rkPacket);
			m_kTargetItem = kResultItem;
		}break;
	case E_BASICOPTIONAMP_NOT_FOUND_ITEM:			{ iNoticeTextTableNo = 1531;	}break;//대상아이템이없음
	case E_BASICOPTIONAMP_CANT_AMP_ITEM:			{ iNoticeTextTableNo = 1532;	}break;//증폭불가능한아이템
	case E_BASICOPTIONAMP_NOT_FOUND_INSURANCEITEM:	{ iNoticeTextTableNo = 1533;	}break;//보험아이템을찾을수없음
	case E_BASICOPTIONAMP_NOT_ENOUGH_MONEY:			{ iNoticeTextTableNo = 1534;	}break;//돈이부족함
	case E_BASICOPTIONAMP_NOT_ENOUGH_SOUL:			{ iNoticeTextTableNo = 1535;	}break;//소울이부족함
	case E_BASICOPTIONAMP_NOT_ENOUGH_AMPITEM:		{ iNoticeTextTableNo = 1538;	}break;//증폭아이템부족함
	case E_BASICOPTIONAMP_DBERROR:					{ iNoticeTextTableNo = 1539;	}break;//디비처리에러
	}

	g_kSoundMan.PlayAudioSourceByID(NiAudioSource::TYPE_3D, szName, 0.0f, 80, 100, g_kPilotMan.GetPlayerActor());
	::Notice_Show(::TTW(iNoticeTextTableNo), iNoticeLevel);
}

int PgUIItemRarityAmplify::GetEquipPos(CItemDefMgr const &rkItemDefMgr, PgBase_Item &rkItem)
{
	CItemDef const *pkItemDef = rkItemDefMgr.GetDef(rkItem.ItemNo());	
	if( !pkItemDef ) 
	{
		return 0;
	}
	return pkItemDef->EquipPos();
}

bool PgUIItemRarityAmplify::GetChangeEnchantInfo(CONT_ABILS& rkOutContAbils, PgBase_Item& rkPrevItem, PgBase_Item& rkResultItem, int const iLevelLimit)
{
	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const *pkPrevItemDef = kItemDefMgr.GetDef(rkPrevItem.ItemNo());
	CItemDef const *pkResultItemDef = kItemDefMgr.GetDef(rkResultItem.ItemNo());
	if( !pkPrevItemDef || !pkResultItemDef )
	{
		return false;
	}

	int const iEquipPos = pkResultItemDef->EquipPos();
	if( !iEquipPos )
	{
		return false;
	}

	SEnchantInfo kPrevEnchantInfo = rkPrevItem.EnchantInfo();
	if( kPrevEnchantInfo.IsEmpty() )
	{
		return false;
	}

	SEnchantInfo kResultEnchantInfo = rkResultItem.EnchantInfo();
	if( kResultEnchantInfo.IsEmpty() )
	{
		return false;
	}
	
	SBasicOptionAmpKey const kResultAmpKey(GetEquipType(pkResultItemDef), pkResultItemDef->GetAbil(AT_LEVELLIMIT), kResultEnchantInfo.BasicAmpLv());
	SBasicOptionAmpKey const kPrevAmpKey(GetEquipType(pkPrevItemDef), pkPrevItemDef->GetAbil(AT_LEVELLIMIT), kPrevEnchantInfo.BasicAmpLv());
	for( int iIndex=0; iIndex<4; ++iIndex)
	{
		GetEnchantAbils(rkOutContAbils, iIndex, iEquipPos, kPrevEnchantInfo, kResultEnchantInfo, kPrevAmpKey, kResultAmpKey);
	}
	if( rkOutContAbils.empty() )
	{
		return false;
	}

	return true;
};

bool PgUIItemRarityAmplify::GetEnchantAbils(CONT_ABILS &rkOutContAbil, int const iIndex, int const iEquipPos, SEnchantInfo& rkPrevEnchantInfo, SEnchantInfo& rkResultEnchantInfo, SBasicOptionAmpKey const& kPrevAmpKey, SBasicOptionAmpKey const& kResultAmpKey)
{
	__int64 i64PrevBasicLv = 0;
	__int64 i64ResultBasicLv = 0;
	__int64 i64BasicType = 0;
	switch(iIndex)
	{
	case 0: 
		{ 
			i64BasicType = rkResultEnchantInfo.BasicType1();
			i64PrevBasicLv = rkPrevEnchantInfo.BasicLv1();
			i64ResultBasicLv = rkResultEnchantInfo.BasicLv1();
		}break;
	case 1: 
		{ 
			i64BasicType = rkResultEnchantInfo.BasicType2();
			i64PrevBasicLv = rkPrevEnchantInfo.BasicLv2();
			i64ResultBasicLv = rkResultEnchantInfo.BasicLv2();
		}break;
	case 2: 
		{ 
			i64BasicType = rkResultEnchantInfo.BasicType3();
			i64PrevBasicLv = rkPrevEnchantInfo.BasicLv3();
			i64ResultBasicLv = rkResultEnchantInfo.BasicLv3();
		}break;
	case 3: 
		{ 
			i64BasicType = rkResultEnchantInfo.BasicType4();
			i64PrevBasicLv = rkPrevEnchantInfo.BasicLv4();
			i64ResultBasicLv = rkResultEnchantInfo.BasicLv4();
		}break;
	default: 
		{
			return false;
		}break;
	}

	CONT_ENCHANT_ABIL kContEnchantAbil;
	::GetAbilObject(i64BasicType, iEquipPos, i64ResultBasicLv, 0, iIndex, kContEnchantAbil, kResultAmpKey);
	::GetAbilObject(i64BasicType, iEquipPos, i64PrevBasicLv, 0, iIndex, kContEnchantAbil, kPrevAmpKey);

	if( 2 > kContEnchantAbil.size() ) 
	{
		return false;
	}

	if( kContEnchantAbil[0].iValue <= kContEnchantAbil[1].iValue )
	{
		return true;
	}

	kContEnchantAbil[0].iValue -= kContEnchantAbil[1].iValue;
	rkOutContAbil.insert(std::make_pair(kContEnchantAbil[0].wType, kContEnchantAbil[0].iValue));
	return true;
};

void PgUIItemRarityAmplify::RecentResult( HRESULT hResult )
{
	bool bSucc = false;
	bool bFail = false;
	m_hResult = hResult;
	switch( m_hResult )
	{
	case E_BASICOPTIONAMP_SUCCESS:
		{
			bSucc = true;
			m_pkWndUIModel->SetNIFEnableUpdate(UIMODEL_RARITY_EFFECT_NIF_SUCC_NAME, bSucc);
			m_pkWndUIModel->ResetNIFAnimation(UIMODEL_RARITY_EFFECT_NIF_SUCC_NAME);
		}break;
	case E_BASICOPTIONAMP_FAIL:
	case E_BASICOPTIONAMP_FAIL_BROKEN:
		{
			bFail = true;
			m_pkWndUIModel->SetNIFEnableUpdate(UIMODEL_RARITY_EFFECT_NIF_FAIL_NAME, bFail);
			m_pkWndUIModel->ResetNIFAnimation(UIMODEL_RARITY_EFFECT_NIF_FAIL_NAME);
		}break;
	}

	m_pkWndUIModel->RenderFrame(NiRenderer::GetRenderer(), UIMODEL_RARITY_POS);
}

int PgUIItemRarityAmplify::GetHaveInsureItem( int& iOutItemNo, SItemPos &rkOutItemPos )
{
	iOutItemNo = 0;
	rkOutItemPos.Clear();

	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkPlayer )
	{
		return 0;
	}

	PgInventory *pkInv = pkPlayer->GetInven();
	if( !pkInv )
	{ 
		return 0;
	}

	SDefBasicOptionAmp const * pkOptionAmpInfo = GetBasicOptionAmp();
	if( !pkOptionAmpInfo )
	{
		return 0;
	}

	iOutItemNo = pkOptionAmpInfo->iInsuranceItemNo;
	if(S_OK != pkInv->GetFirstItem(iOutItemNo, rkOutItemPos))
	{
		return 0;
	}

	return pkInv->GetTotalCount(iOutItemNo);
}

__int64 PgUIItemRarityAmplify::GetHaveMoney()
{
	CUnit *pkUnit = g_kPilotMan.GetPlayerUnit();
	if(!pkUnit)
	{
		return 0;
	}
	return pkUnit->GetAbil64(AT_MONEY);
}

void PgUIItemRarityAmplify::Start()
{
	if( false == Check() )
	{
		return;
	}

	XUI::CXUI_Wnd* pkWnd = XUIMgr.Get( _T("SFRM_ITEM_RARITY_AMPLIFY") );
	if (!pkWnd)
	{
		return;
	}

	XUI::CXUI_Wnd* pkBtnRefine = pkWnd->GetControl(L"BTN_TRY_REFINE");
	if( pkBtnRefine )
	{
		pkBtnRefine->Enable(false);
	}

	ProcessEffect(true);
	m_fStartTime = g_pkApp->GetAccumTime();
	m_bProcessStart = true;
	char szName[100] = "Enchant";
	g_kSoundMan.PlayAudioSourceByID(NiAudioSource::TYPE_3D, szName, 0.0f, 80, 100, g_kPilotMan.GetPlayerActor());
}

void PgUIItemRarityAmplify::NoticeShow_SuccessInfo(PgBase_Item const& kItem, CItemDef const* pDef, CONT_ABILS const& kContAbils)
{
	if( kContAbils.empty() )
	{
		return;
	}

	int iNoticeCount = 0;
	std::wstring wstrNoticeText;
	CONT_ABILS::const_iterator kItor = kContAbils.begin();
	while( kItor != kContAbils.end() )
	{
		std::wstring wstrAbilString;
		MakeAbilString(kItem, pDef, kItor->first, kItor->second, wstrAbilString, NULL, NULL, true, true, 0, true);//레벨
		if( !wstrAbilString.empty() )
		{
			++iNoticeCount;
			if( 1 == iNoticeCount % 2 )
			{
				wstrNoticeText = wstrAbilString;
			}
			else
			{//옵션 2개 모아서 출력
				wstrNoticeText += L" / ";
				wstrNoticeText += wstrAbilString;
				wstrNoticeText += ::TTW(1234);// 상승하였습니다.	
				::Notice_Show(wstrNoticeText, EL_Normal);
				wstrNoticeText.clear();
			}
		}

		++kItor;
	}

	if( !wstrNoticeText.empty() )
	{//옵션이 1개나 3개라면 일로 올수 있다.
		wstrNoticeText += ::TTW(1234);// 상승하였습니다.	
		::Notice_Show(wstrNoticeText, EL_Normal);
	}
}

bool PgUIItemRarityAmplify::FindCraftOption(PgBase_Item const& rkItem, CItemDef const* pkItemDef)const
{
	SEnchantInfo const kEnchantInfo = rkItem.EnchantInfo();
	int const iEquipPos = pkItemDef->EquipPos();
	int const iLevelLimit = pkItemDef->GetAbil(AT_LEVELLIMIT);
	SBasicOptionAmpKey const kAmpKey(GetEquipType(pkItemDef), iLevelLimit, kEnchantInfo.BasicAmpLv()+1);

	CONT_ENCHANT_ABIL kEnchantAbil;
	::GetAbilObject(kEnchantInfo.BasicType1(), iEquipPos, kEnchantInfo.BasicLv1(), 0, 0, kEnchantAbil, kAmpKey);
	::GetAbilObject(kEnchantInfo.BasicType2(), iEquipPos, kEnchantInfo.BasicLv2(), 0, 1, kEnchantAbil, kAmpKey);
	::GetAbilObject(kEnchantInfo.BasicType3(), iEquipPos, kEnchantInfo.BasicLv3(), 0, 2, kEnchantAbil, kAmpKey);
	::GetAbilObject(kEnchantInfo.BasicType4(), iEquipPos, kEnchantInfo.BasicLv4(), 0, 3, kEnchantAbil, kAmpKey);

	if( kEnchantAbil.empty() )
	{
		return false;
	}
	return true;
}

bool PgUIItemRarityAmplify::Cancle()
{
	bool bRet = !m_bProcessStart;
	if( m_bProcessStart )
	{//연출중에만 취소가 가능함.
		::Notice_Show(::TTW(1221), EL_Warning);
		g_kSoundMan.StopAudioSourceByID("Enchant");
		Clear();
	}

	return bRet;
}

bool PgUIItemRarityAmplify::Check()
{
	if( m_bProcessStart )
	{//이미 진행중이다
		return false;
	}

	int iNeedItemNo = 0;
	if( GetHaveAmplifyCount(iNeedItemNo) < GetNeedAmplifyCount() )
	{//강화기가 모자르다
		::Notice_Show(::TTW(1538), EL_Warning);
		return false;
	}

	if( GetHaveSoulCount() < GetNeedSoulCount() )
	{//소울이 모자르다
		::Notice_Show(::TTW(1535), EL_Warning);
		return false;
	}

	if( GetHaveMoney() < GetAmplifyNeedMoney() )
	{//돈이 모자르다
		::Notice_Show(::TTW(1534), EL_Warning);
		return false;
	}

	return true;
}

bool PgUIItemRarityAmplify::IsUseableItem(PgBase_Item const& rkItem, bool & rbGrayVisible)const
{
	XUI::CXUI_Wnd* pkWnd = XUIMgr.Get( L"SFRM_ITEM_RARITY_AMPLIFY" );
	if( !pkWnd )
	{
		return false;
	}

	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const *pkItemDef = kItemDefMgr.GetDef(rkItem.ItemNo());
	if( !pkItemDef )
	{
		return false;
	}

	rbGrayVisible = true;

	SEnchantInfo const& kEnchantInfo = rkItem.EnchantInfo();
	if( FindCraftOption(rkItem, pkItemDef) 
	&& PgItemRarityUpgradeFormula::GetBasicOptionAmp(GetEquipType(pkItemDef), pkItemDef->GetAbil(AT_LEVELLIMIT), kEnchantInfo.BasicAmpLv()+1) )
	{
		rbGrayVisible = false;
	}

	return true;
}