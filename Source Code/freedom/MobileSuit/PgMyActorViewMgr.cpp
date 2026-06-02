#include "StdAfx.h"
#include "PgUIScene.h"
#include "PgPilotMan.h"
#include "PgPilot.h"
#include "PgMyActorViewMgr.h"

char const* const STR_DEF_MODEL_NAME = "PgMyActorView";

void lwMyActorView::RegisterWrapper(lua_State *pkState)
{
	using namespace lua_tinker;
	def(pkState, "InitMyActorView", &lwMyActorView::lwInitActor);
	def(pkState, "UpdateMyActorView", &lwMyActorView::lwUpdateActor);
	def(pkState, "DeleteMyActor", &lwMyActorView::lwDeleteMyActor);
	def(pkState, "UpdateOtherActorView", &lwMyActorView::lwUpdateOtherActor);
	def(pkState, "RotateMyActorView", &lwMyActorView::lwRotateActor);
	def(pkState, "UpdateOrthoZoom", &lwMyActorView::lwUpdateOrthoZoom);
	def(pkState, "UpdateOrthoZoomMinMax", &lwMyActorView::lwUpdateOrthoZoomMinMax);
	def(pkState, "SetMyActorEquip", &lwMyActorView::lwSetMyActorEquip);
	def(pkState, "DrawMyActorView", &lwMyActorView::lwDrawMyActorView);
	def(pkState, "AddToDrawListMyActorView",  &lwMyActorView::lwAddToDrawListMyActorView);

	def(pkState, "UpdateMyActorPetView", &lwMyActorView::lwUpdateMyActorPetView);
	def(pkState, "UpdateOtherActorPetView", &lwMyActorView::lwUpdateOtherActorPetView);
}

void lwMyActorView::lwInitActor(lwUIWnd UISelf, char const* szAddName, bool bOrtho)
{
	g_kMyActorViewMgr.Init(UISelf.GetSelf(), szAddName, bOrtho);
}

void lwMyActorView::lwUpdateActor(char const* szAddName)
{
	g_kMyActorViewMgr.Update(szAddName);
}

void lwMyActorView::lwUpdateOtherActor(char const* szAddName, lwGUID Guid)
{
	g_kMyActorViewMgr.Update(szAddName, Guid());
}

void lwMyActorView::lwRotateActor(char const* szAddName, float const fRad)
{
	g_kMyActorViewMgr.Rotate(szAddName, fRad);
}

void lwMyActorView::lwSetMyActorEquip(char const* szAddName)
{
	BM::GUID kGuid;
	g_kPilotMan.GetPlayerPilotGuid(kGuid);
	g_kMyActorViewMgr.ChangeEquip(szAddName, kGuid);
}

void lwMyActorView::lwDrawMyActorView(lwUIWnd UISelf, char const* szAddName)
{
	g_kMyActorViewMgr.Draw(UISelf.GetSelf(), szAddName);
}

void lwMyActorView::lwAddToDrawListMyActorView(lwUIWnd UISelf, char const* szAddName)
{
	g_kMyActorViewMgr.AddToDrawList(UISelf.GetSelf(), szAddName);
}

void lwMyActorView::lwDeleteMyActor(lwUIWnd UISelf, char const* szAddName)
{
	g_kMyActorViewMgr.DeleteActor(szAddName);
}

void lwMyActorView::lwUpdateOrthoZoom(char const* szAddName, float const fScale, int const iX, int const iY)
{
	g_kMyActorViewMgr.UpdateOrthoZoom(szAddName, fScale, iX, iY);
}

void lwMyActorView::lwUpdateOrthoZoomMinMax(char const* szAddName, float const fMin, float const fMax)
{
	g_kMyActorViewMgr.UpdateOrthoZoomMinMax(szAddName, fMin, fMax);
}

void lwMyActorView::lwUpdateMyActorPetView(char const* szAddName)
{
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkPlayer )
	{
		return;
	}

	BM::GUID const& kPetGuid = pkPlayer->SelectedPetID();
	if( kPetGuid.IsNotNull() )
	{
		g_kMyActorViewMgr.UpdatePet(szAddName, kPetGuid);
	}
}

void lwMyActorView::lwUpdateOtherActorPetView(char const* szAddName, lwGUID Guid)
{
	PgPilot* pkPilot = g_kPilotMan.FindPilot(Guid.GetGUID());
	if( !pkPilot )
	{
		return;
	}

	PgPlayer* pkPlayer = dynamic_cast<PgPlayer*>(pkPilot->GetUnit());
	if( !pkPlayer )
	{
		return;
	}

	BM::GUID const& kPetGuid = pkPlayer->SelectedPetID();
	if( kPetGuid.IsNotNull() )
	{
		g_kMyActorViewMgr.UpdatePet(szAddName, kPetGuid);
	}
}

PgMyActorViewMgr::PgMyActorViewMgr(void)
	: m_pkTempActor(NULL)
{
}

PgMyActorViewMgr::~PgMyActorViewMgr(void)
{
	CONT_VIEW_ACTOR::iterator	iter = m_kActorContainer.begin();
	if( iter != m_kActorContainer.end() )
	{
		iter = m_kActorContainer.erase(iter);
	}
}

bool PgMyActorViewMgr::Init(XUI::CXUI_Wnd* pTarget, char const* szAddName, bool bOrtho)
{
	CONT_VIEW_ACTOR::iterator	iter = m_kActorContainer.find(std::string(szAddName));
	if( iter != m_kActorContainer.end() )
	{
		return false;
	}

	std::string kModelName = STR_DEF_MODEL_NAME + std::string(szAddName);
	return g_kUIScene.InitRenderModel(kModelName, pTarget->Size(), pTarget->TotalLocation(), true, bOrtho);
}

bool PgMyActorViewMgr::Update(char const* szAddName)
{
	PgActor* pActor = g_kPilotMan.GetPlayerActor();
	if( !pActor )
	{
		return false;
	}
	return Update(szAddName, pActor->GetGuid());
}

bool PgMyActorViewMgr::Update(char const* szAddName, BM::GUID const& Guid)
{
	CONT_VIEW_ACTOR::iterator	iter = m_kActorContainer.find(std::string(szAddName));
	if( iter != m_kActorContainer.end() )
	{
		return false;
	}

	PgActor* pActor = g_kPilotMan.FindActor(Guid);
	if( !pActor )
	{
		return false;
	}

	std::string kModelName = STR_DEF_MODEL_NAME + std::string(szAddName);
	PgUIModel* pModel = g_kUIScene.FindUIModel(kModelName);
	if( !pModel )
	{
		return false;
	}

	pModel->AddPgActor("ActorName", pActor, true, false);
	PgActor* pkCloneActor = pModel->GetPgActor("ActorName");
	pModel->SetCameraZoomMinMax(0.2f, 1.0f);
	pModel->SetOrthoZoom(1.0f);
	SCameraResetInfo kInfo;
	kInfo.fYCenter = 0.9f;
	kInfo.eHeightSet = SCameraResetInfo::EHS_FIX_MODEL_VALUE;
	
	PgPilot* pkPilot= g_kPilotMan.FindPilot(Guid);
	if(pkPilot)
	{// 캐릭터 정보창의 캐릭터 크기는 기본적으로 자종조절되나 잘못 리소스가 만들어진경우
		int const iBaseClass = pkPilot->GetAbil(AT_BASE_CLASS);
		switch( iBaseClass )
		{// 어쩔수 없이 수동으로 수정을 해주어야 하기때문에 이와 같이 하드코딩 하여 수정함
		case UCLASS_SHAMAN:
			{// 소환사
				kInfo.fRadius = -6.0f;
			}break;
		case UCLASS_DOUBLE_FIGHTER:
			{// 격투가
				kInfo.fRadius = -1.0f;
			}break;
		}
	}
	pModel->CameraReset(pkCloneActor->GetNIFRoot(), kInfo);
	// 직교 투영이므로 카메라가 뒤로 빠져도 바라보는 것에는 영향을 미치지 않는다.
	// 카메라가 너무 가까우면 모델링이 짤려 보이므로, 살짝 뒤로 빼줌..
	// 투영행렬을 바꿀 경우 거리 조절은 다시 해주어야한다.

	std::pair<CONT_VIEW_ACTOR::iterator, bool> Rst = m_kActorContainer.insert(szAddName);
	if( Rst.second )
	{
		return true;
	}
	
	return false;
}

bool PgMyActorViewMgr::UpdatePet(char const* szAddName, BM::GUID const& Guid)
{
	CONT_VIEW_ACTOR::iterator	iter = m_kActorContainer.find(std::string(szAddName));
	if( iter != m_kActorContainer.end() )
	{
		return false;
	}

	PgActorPet* pPet = dynamic_cast<PgActorPet*>(g_kPilotMan.FindActor(Guid));
	if( !pPet )
	{
		return false;
	}

	std::string kModelName = STR_DEF_MODEL_NAME + std::string(szAddName);
	PgUIModel* pModel = g_kUIScene.FindUIModel(kModelName);
	if( !pModel )
	{
		return false;
	}

	pModel->AddPgPet("ActorName", pPet, true, false);
	PgActorPet* pkClonePet = pModel->GetPgPet("ActorName");
	pModel->SetCameraZoomMinMax(0.2f, 1.0f);
	pModel->SetOrthoZoom(1.0f);
	SCameraResetInfo kInfo;
	kInfo.fYCenter = pkClonePet->UIModelOpt().fDrawHeight;
	if( 0.0f == kInfo.fYCenter )
	{//디폴트 높이
		kInfo.fYCenter = 0.8f;
	}
	kInfo.eHeightSet = SCameraResetInfo::EHS_FIX_MODEL_VALUE;
	kInfo.fRadius = pkClonePet->UIModelOpt().fIncreaseCamRad;

	NiAVObject* pkObject = pkClonePet->GetObjectByName("Bip01");
	if( pkObject )
	{
		pModel->CameraReset(pkObject, kInfo);
	}
	else
	{
		pModel->CameraReset(pkClonePet->GetNIFRoot(), kInfo);
	}
	// 직교 투영이므로 카메라가 뒤로 빠져도 바라보는 것에는 영향을 미치지 않는다.
	// 카메라가 너무 가까우면 모델링이 짤려 보이므로, 살짝 뒤로 빼줌..
	// 투영행렬을 바꿀 경우 거리 조절은 다시 해주어야한다.

	std::pair<CONT_VIEW_ACTOR::iterator, bool> Rst = m_kActorContainer.insert(szAddName);
	if( Rst.second )
	{
		return true;
	}
	
	return false;
}

bool PgMyActorViewMgr::DeleteActor(char const* szAddName)
{
	CONT_VIEW_ACTOR::iterator	iter = m_kActorContainer.find(std::string(szAddName));
	if( iter == m_kActorContainer.end() )
	{
		return false;
	}

	bool bIsActor = true;
	PgActor* pkActor = GetActor(szAddName);
	if( !pkActor )
	{
		bIsActor = false;
		pkActor = GetPet(szAddName);
	}

	m_kActorContainer.erase(iter);

	std::string kModelName = STR_DEF_MODEL_NAME + std::string(szAddName);
	PgUIModel* pModel = g_kUIScene.FindUIModel(kModelName);
	if( !pModel )
	{
		return false;
	}

	if(bIsActor)
	{
		pModel->DeletePgActor("ActorName");
	}
	else
	{
		pModel->DeletePgPet("ActorName");
	}
	return true;
}

bool PgMyActorViewMgr::BeginChangeParts(char const* szAddName)
{
	PgActor* pkActor = GetActor(szAddName);
	if( pkActor )
	{
		m_pkTempActor = pkActor;
		return true;
	}
	return false;
}

bool PgMyActorViewMgr::ChangeParts(int const iItemNo, EChangeEquipType const Type)
{
	if(!m_pkTempActor)
	{
		return false;
	}

	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const *pkItemDef = kItemDefMgr.GetDef(iItemNo);
	if( !pkItemDef )
	{
		return false;
	}
	eEquipLimit equipLimit = static_cast<eEquipLimit>(pkItemDef->GetAbil(AT_EQUIP_LIMIT));

	switch(Type)
	{
	case CET_ADD:{ m_pkTempActor->AddEquipItem(iItemNo, true, PgItemEx::LOAD_TYPE_USEQUEUE); }break;
	case CET_DEL:
		{
			if( m_pkTempActor->DelDefaultItem(equipLimit) )
			{
				m_pkTempActor->UnequipItem(static_cast<EInvType>(pkItemDef->PrimaryInvType()), static_cast<EEquipPos>(pkItemDef->EquipPos()), pkItemDef->No(), PgItemEx::LOAD_TYPE_INSTANT);
			}		
		}break;
	}

	return true;
}

bool PgMyActorViewMgr::ChangePartsColor(eEquipLimit const Limit, DWORD const iItemNo)
{
	if(!m_pkTempActor)
	{
		return false;
	}

	m_pkTempActor->SetItemColor(Limit, iItemNo);
	return true;
}

bool PgMyActorViewMgr::EndChangeParts()
{
	if( !m_pkTempActor )
	{
		return false;
	}
	m_pkTempActor->EquipAllItem();
	m_pkTempActor = NULL;
	return true;
}

bool PgMyActorViewMgr::ChangeEquip(char const* szAddName, BM::GUID const& Guid)
{
	PgActor* pkActor = GetActor(szAddName);
	if( pkActor )
	{
		PgActor* pActor = g_kPilotMan.FindActor(Guid);
		if( !pActor )
		{
			return false;
		}

		pkActor->CopyEquipItem(pActor);
		return true;
	}
	return false;
}

bool PgMyActorViewMgr::ChangePetEquip(char const* szAddName, BM::GUID const& Guid)
{
	PgActorPet* pkModelPet = GetPet(szAddName);
	if( pkModelPet )
	{
		PgActorPet* pkPet = dynamic_cast<PgActorPet*>(g_kPilotMan.FindActor(Guid));
		if( !pkPet )
		{
			return false;
		}

		pkModelPet->CopyEquipItem(pkPet);
		return true;
	}
	return false;
}

bool PgMyActorViewMgr::ChangePetColor(char const* szAddName, BM::GUID const& Guid)
{
	PgActorPet* pkPet = GetPet(szAddName);
	if( !pkPet )
	{
		return false;
	}

	PgPilot* pkPilot = pkPet->GetPilot();
	if( pkPilot )
	{
		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( !pkPlayer )
		{
			return false;
		}

		BM::GUID const& kPetGuid = pkPlayer->SelectedPetID();
		if( kPetGuid.IsNull() )
		{
			return false;
		}

		PgPilot* pkNewPilot = g_kPilotMan.FindPilot(kPetGuid);
		if( !pkNewPilot )
		{
			return false;
		}

		int const iColor = pkNewPilot->GetAbil(AT_COLOR_INDEX);
		pkPilot->SetAbil(AT_COLOR_INDEX, iColor);
		pkPet->DoChangeColor();
		return true;
	}
	return false;
}

void PgMyActorViewMgr::Del(char const* szAddName)
{
	CONT_VIEW_ACTOR::iterator	iter = m_kActorContainer.find(std::string(szAddName));
	if( iter != m_kActorContainer.end() )
	{
		std::string	kModelName = STR_DEF_MODEL_NAME + std::string(szAddName);
		g_kUIScene.RemoveModel(kModelName.c_str());
		m_kActorContainer.erase(iter);
	}
}

void PgMyActorViewMgr::Rotate(char const* szAddName, float const fRad)
{
	CONT_VIEW_ACTOR::iterator	iter = m_kActorContainer.find(std::string(szAddName));
	if( iter != m_kActorContainer.end() )
	{
		PgActor* pkActor = GetActor(szAddName);
		if( pkActor )
		{
			pkActor->IncRotate(fRad);
		}
		else
		{
			PgActorPet* pkPet = GetPet(szAddName);
			if( pkPet )
			{
				pkPet->IncRotate(fRad);
			}
		}
	}
}

void PgMyActorViewMgr::Draw(XUI::CXUI_Wnd* pTarget, char const* szAddName)
{
	CONT_VIEW_ACTOR::iterator	iter = m_kActorContainer.find(std::string(szAddName));
	if( iter == m_kActorContainer.end() )
	{
		return;
	}


	std::string kModelName = STR_DEF_MODEL_NAME + std::string(szAddName);
	PgUIModel* pModel = g_kUIScene.FindUIModel(kModelName);
	if( !pModel )
	{
		return;
	}

	pModel->RenderFrame(NiRenderer::GetRenderer(), pTarget->TotalLocation());
}

void PgMyActorViewMgr::AddToDrawList(XUI::CXUI_Wnd* pTarget, char const* szAddName)
{
	CONT_VIEW_ACTOR::iterator	iter = m_kActorContainer.find(std::string(szAddName));
	if( iter == m_kActorContainer.end() )
	{
		return;
	}

	std::string kModelName = STR_DEF_MODEL_NAME + std::string(szAddName);
	g_kUIScene.AddToDrawListRenderModel(kModelName);
}

void PgMyActorViewMgr::UpdateOrthoZoom(char const* szAddName, float const fScale, int const iX, int const iY)
{
	CONT_VIEW_ACTOR::iterator	iter = m_kActorContainer.find(std::string(szAddName));
	if( iter != m_kActorContainer.end() )
	{
		std::string kModelName = STR_DEF_MODEL_NAME + std::string(szAddName);
		PgUIModel* pModel = g_kUIScene.FindUIModel(kModelName);
		if( pModel )
		{
			pModel->SetOrthoZoom( fScale, iX, iY );
		}
	}
}

void PgMyActorViewMgr::UpdateOrthoZoomMinMax(char const* szAddName, float const fMin, float const fMax)
{
	CONT_VIEW_ACTOR::iterator	iter = m_kActorContainer.find(std::string(szAddName));
	if( iter != m_kActorContainer.end() )
	{
		std::string kModelName = STR_DEF_MODEL_NAME + std::string(szAddName);
		PgUIModel* pModel = g_kUIScene.FindUIModel(kModelName);
		if( pModel )
		{
			pModel->SetCameraZoomMinMax(fMin, fMax);
		}
	}
}

void PgMyActorViewMgr::ChangePetAction(char const* szAddName, std::string const& strActionName)
{
	CONT_VIEW_ACTOR::iterator	iter = m_kActorContainer.find(std::string(szAddName));
	if( iter != m_kActorContainer.end() )
	{
		std::string kModelName = STR_DEF_MODEL_NAME + std::string(szAddName);
		PgUIModel* pModel = g_kUIScene.FindUIModel(kModelName);
		if( pModel )
		{
			pModel->ChangePetAction("ActorName", strActionName);
		}
	}
}


PgActor* PgMyActorViewMgr::GetActor(char const* szAddName)
{
	CONT_VIEW_ACTOR::iterator	iter = m_kActorContainer.find(std::string(szAddName));
	if( iter != m_kActorContainer.end() )
	{
		std::string kModelName = STR_DEF_MODEL_NAME + std::string(szAddName);
		PgUIModel* pModel = g_kUIScene.FindUIModel(kModelName);
		if( pModel )
		{
			return pModel->GetPgActor("ActorName");
		}
	}
	return NULL;
}

PgActorPet* PgMyActorViewMgr::GetPet(char const* szAddName)
{
	CONT_VIEW_ACTOR::iterator	iter = m_kActorContainer.find(std::string(szAddName));
	if( iter != m_kActorContainer.end() )
	{
		std::string kModelName = STR_DEF_MODEL_NAME + std::string(szAddName);
		PgUIModel* pModel = g_kUIScene.FindUIModel(kModelName);
		if( pModel )
		{
			return pModel->GetPgPet("ActorName");
		}
	}
	return NULL;
}