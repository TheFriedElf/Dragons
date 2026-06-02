#include "stdAfx.h"
#include "PgHouse.h"
#include "PgFurniture.h"
#include "PgPilot.h"
#include "PgPilotMan.h"
#include "PgNetwork.h"
#include "PgRenderMan.h"
#include "PgRenderer.h"
#include "PgPuppet.h"
#include "PgPilotMan.h"
#include "PgInput.h"
#include "lwWorld.h"

#include "NewWare/Renderer/DrawWorkflow.h"
#include "NewWare/Renderer/Kernel/RenderStateTagExtraData.h"


NiImplementRTTI(PgHouse, PgIWorldObject);

float const GARDEN_W = 400.0f;
float const GARDEN_H = 400.0f;

EEquipPos const PgHouse::EquipPosArray[DefaultExteriaCount] = {EQUIP_POS_HOME_COLOR, EQUIP_POS_HOME_STYLE, EQUIP_POS_HOME_FENCE, EQUIP_POS_HOME_GARDEN};
int const PgHouse::DefaultExteriaArray[DefaultExteriaCount] = {0, 70000680, 70000770, 70000650};

PgHouse::~PgHouse()
{
	Terminate();
}

void PgHouse::Init()
{
	m_kExteriaMap.clear();
	NiPoint3 const& rkPos = GetWorldTranslate();
	m_kWall.SetWallInfo(PgWall::WALL_TYPE_FLOOR, NiPoint3(rkPos.x - GARDEN_W*0.5f, rkPos.y - GARDEN_H*0.5f, rkPos.z), NiPoint3(rkPos.x + GARDEN_W*0.5f, 0, rkPos.z));
	m_spWallRoot = NULL;
	m_spWallRoot = NiNew NiNode();
	if( m_spWallRoot && g_pkWorld )
	{
		if (g_pkWorld->GetDynamicNodeRoot())
		{
			g_pkWorld->RunObjectGroupFunc(OGT_FURNITURE, WorldObjectGroupsUtil::AttachChild(m_spWallRoot, true));
		}
	}
	m_bArrangeMode = false;
	m_pkArrangingFurniture = NULL;
	m_pkPickedFurniture = NULL;

	m_kNamePlate = NULL;
}

void PgHouse::Terminate()
{
	m_kWall.Terminate();
	ExteriaMap::iterator itr = m_kExteriaMap.begin();
	while( itr != m_kExteriaMap.end() )
	{
		PgFurniture* pkFurniture = (*itr).second;
		if( pkFurniture )
		{
			NiNode* pkParent = pkFurniture->GetParent();
			if( pkParent )
			{
				pkParent->DetachChild(pkFurniture->GetNIFRoot());
			}
			//NiNode::DetachChild(pkFurniture->GetNIFRoot());
			PgFurniture::DeleteFurniture(pkFurniture);
			(*itr).second = NULL;
		}
		++itr;
	}
	m_kExteriaMap.clear();

	if( m_spWallRoot )
	{
		NiNode* pkParent = m_spWallRoot->GetParent();
		if (pkParent)
		{
			pkParent->DetachChild(m_spWallRoot);			
		}
		m_spWallRoot = NULL;
	}
}

void PgHouse::AddAllFurniture()
{
	if(!GetPilot()->GetUnit())
	{
		return;
	}
	PgInventory* pkInv = GetPilot()->GetUnit()->GetInven();
	if(!pkInv)
	{
		return;
	}

	const int iMaxInv = pkInv->GetMaxIDX(IT_HOME);
	for( int ii = 0; ii < iMaxInv; ++ii )
	{
		PgBase_Item kItem;
		SItemPos kPos(IT_HOME, ii);
		if(S_OK != pkInv->GetItem(kPos, kItem))
		{
			continue;
		}

		if(!kItem.IsEmpty())
		{
			PgFurniture* pkFurn = AddFurniture(kItem);
			if(pkFurn)
			{
				pkFurn->SetGuid(kItem.Guid());
				if( kItem.GetUseAbleTime() < 0 )
				{
					pkFurn->SetFurnitureColor(NiColor(0.25,0.25,0.25));
				}
			}
		}
	}

	//CONT_HAVE_ITEM_DATA kCont;
	//if(S_OK!=pkInv->GetItems(IT_HOME, kCont))
	//{
	//	return;
	//}

	//NiMatrix3 kRot;
	//CONT_HAVE_ITEM_DATA::const_iterator item_it = kCont.begin();

	//while(kCont.end()!=item_it)
	//{
	//	PgBase_Item const& rkBaseItem = (*item_it).second;
	//	if(!rkBaseItem.IsEmpty())
	//	{
	//		PgFurniture* pkFurn = AddFurniture(rkBaseItem);
	//		if(pkFurn)
	//		{
	//			pkFurn->SetGuid(rkBaseItem.Guid());
	//			if( rkBaseItem.GetUseAbleTime() < 0 )
	//			{
	//				pkFurn->SetFurnitureColor(NiColor(0.25,0.25,0.25));
	//			}
	//		}
	//	}
	//	++item_it;
	//}
}

void SettingFog(NiAVObject *pkAVObject,bool bTurnOn)
{
	NiGeometry	*pkGeom = NiDynamicCast(NiGeometry,pkAVObject);
	if(pkGeom)
	{
		NiFogProperty	*pkFog = pkGeom->GetPropertyState()->GetFog();
		if(pkFog)
		{
			pkFog->SetFog(bTurnOn);
		}
		return;
	}

	NiNode	*pkNode = NiDynamicCast(NiNode,pkAVObject);
	if(pkNode)
	{
		int	iCount = pkNode->GetArrayCount();
		for(int i=0;i<iCount;++i)
		{
			NiAVObject	*pkChild = pkNode->GetAt(i);
			if(pkChild)
			{
				SettingFog(pkChild,bTurnOn);
			}
		}
	}
}

void PgHouse::EquipAllExteria()	//외관 아이템 한번에 장착(AddUnit될 때)
{
	PgPlayer* pkPlayer = dynamic_cast<PgPlayer*>(GetPilot()->GetUnit());
	if(!pkPlayer)
	{
		return;
	}
	PgIWorldObject* pObject = GetPilot()->GetWorldObject();
	if( pObject )
	{
		NiAVObject* pAVObject = pObject->GetNIFRoot();
		if( pAVObject )
		{
			NiNode* pkNode = NiDynamicCast(NiNode, pAVObject);
			if( pkNode )
			{
				int iCount = pkNode->GetChildCount();
				for( int ii = 0; ii < iCount; ++ii )
				{
					pkNode->DetachChildAt(ii);
				}
			}
		}
	}
	PLAYER_ABIL kInfo;
	pkPlayer->GetPlayerAbil(kInfo);

	for(int i = 0; i<DefaultExteriaCount; ++i)
	{
		int iItemNo = DefaultExteriaArray[i];
		switch(EquipPosArray[i])
		{
		case EQUIP_POS_HOME_COLOR:
			{
				if( kInfo.iHairColor )
				{
					iItemNo = kInfo.iHairColor;
				}
			}break;
		case EQUIP_POS_HOME_STYLE:
			{
				if( kInfo.iHairStyle )
				{
					iItemNo = kInfo.iHairStyle;
				}
			}break;
		case EQUIP_POS_HOME_FENCE:
			{
				if( kInfo.iFace )
				{
					iItemNo = kInfo.iFace;
				}
			}break;
		case EQUIP_POS_HOME_GARDEN:
			{
				if( kInfo.iJacket )
				{
					iItemNo = kInfo.iJacket;
				}
			}break;
		}

		if( iItemNo )
		{
			EquipExteria(iItemNo, NiPoint3(0,0,0));
		}
	}

	SettingFog(this, true);
	UpdateProperties();
}

bool PgHouse::EquipExteria(int const iNo, NiPoint3 const& rkPos, float const fDir)//외관 아이템 장착
{
	PgFurniture* pkFurniture = NULL;
	pkFurniture = PgFurniture::GetFurnitureFromDef(iNo);
	if (pkFurniture == NULL)
	{
		PgError1("[PgHouse] EquipExteria, can't create %d Item\n", iNo);
		return false;
	}

	NiAVObject* pMeshRoot = pkFurniture->GetNIFRoot();
	
	CItemDef const* pkDef = pkFurniture->GetItemDef();
	if( !pMeshRoot && !pkDef )
	{
		return false;
	}

	EEquipPos const ePos = static_cast<EEquipPos>(pkDef->EquipPos());

	ExteriaMap::iterator ex_it = m_kExteriaMap.find(ePos);
	if(m_kExteriaMap.end() != ex_it)
	{
		PgFurniture* pkFur = (*ex_it).second;
		if( pkFur )
		{
			NiNode* pkParent = pkFur->GetParent();
			if( pkParent )
			{
				pkParent->DetachChild(pkFur->GetNIFRoot());
			}
			//NiNode::DetachChild(pkFur->GetNIFRoot());
			PgFurniture::DeleteFurniture(pkFur);
			(*ex_it).second = NULL;
		}
		m_kExteriaMap.erase(ex_it);
	}

	NiPoint3 const& rkHousePos = GetWorldTranslate();
	pMeshRoot->SetWorldTranslate(rkHousePos+rkPos);

	NiMatrix3 kRot;
	kRot.MakeZero();
	kRot.MakeZRotation(fDir/180.0f*NI_PI);
	pMeshRoot->SetWorldRotate(kRot);

	if( g_pkWorld )
	{
		g_pkWorld->LightObjectRecurse(g_pkWorld->GetLightRoot(), NiDynamicCast(NiNode, pMeshRoot), PgWorld::LT_ALL);
		pMeshRoot->UpdateEffects();
	}

	m_kExteriaMap.insert(std::make_pair(ePos, pkFurniture));

	if( EQUIP_POS_HOME_GARDEN == pkDef->EquipPos() )
	{
		BuildWalls();
		ClearArrangingFurniture();
		m_kWall.RemoveAllFurniture();
		AddAllFurniture();
	}
	else
	{
		NiNode::AttachChild(pMeshRoot, true);
	}
	NiNode::Update(0.0f);

	return true;
}

bool PgHouse::EquipExteria(SMYHOME const& rkBaseItem)//외관 아이템 장착
{
	bool bRet = true;
	if(false==EquipExteria(rkBaseItem.iHomeColor, NiPoint3(0,0,0)))
	{
		bRet = false;
	}

	if(false==EquipExteria(rkBaseItem.iHomeFence, NiPoint3(0,0,0)))
	{
		bRet = false;
	}

	if(false==EquipExteria(rkBaseItem.iHomeStyle, NiPoint3(0,0,0)))
	{
		bRet = false;
	}

	if(false==EquipExteria(rkBaseItem.iHomeGarden, NiPoint3(0,0,0)))
	{
		bRet = false;
	}

	return bRet;
}

PgFurniture* PgHouse::AddFurniture(int iFurnitureNo, NiPoint3& kPos, int iRotate, BM::GUID const& rkGuid, bool bLocate)
{
	PgFurniture* pkFurniture = NULL;
	pkFurniture = PgFurniture::GetFurnitureFromDef(iFurnitureNo);
	if (pkFurniture == NULL)
	{
		PgError1("[PgHouse::AddFurniture]can't create %d Item\n", iFurnitureNo);
		return NULL;
	}

	EFurnitureType eFurnitureType = pkFurniture->GetFurnitureType();

	switch(eFurnitureType)
	{
	case FURNITURE_TYPE_OBJECT:
	case FURNITURE_TYPE_OBJECT_ATTACH:
	case FURNITURE_TYPE_FLOOR:
	case FURNITURE_TYPE_FLOOR_ATTACH:
	case FURNITURE_TYPE_ETC:
		{

		}break;
	case FURNITURE_TYPE_WALL:
	case FURNITURE_TYPE_WALL_ATTACH:
	default:
		{
			PgError2("[PgHouse::AddFurniture] %d furniture has invalid type(%d)\n", pkFurniture->GetFurnitureNo(), pkFurniture->GetFurnitureType());
			PgFurniture::DeleteFurniture(pkFurniture);
			return NULL;
		}break;
	}

	if(false==m_kWall.AttachFurniture(pkFurniture, kPos, iRotate, rkGuid))
	{
		PgFurniture::DeleteFurniture(pkFurniture);
		return NULL;
	}

	if( bLocate )
	{
		m_kWall.LocateFurniture(pkFurniture);
	}

	if (g_pkWorld)
	{
		g_pkWorld->LightObjectRecurse(g_pkWorld->GetLightRoot(), NiDynamicCast(NiNode, pkFurniture->GetNIFRoot()), PgWorld::LT_ALL);
		pkFurniture->UpdateEffects();
	}
	
	return pkFurniture;
}
PgFurniture* PgHouse::AddFurniture(PgBase_Item const& rkBaseItem)
{
	SHomeItem kHomeItem;
	rkBaseItem.Get(kHomeItem);

	return AddFurniture(rkBaseItem.ItemNo(), NiPoint3(kHomeItem.Pos().x, kHomeItem.Pos().y, kHomeItem.Pos().z), kHomeItem.Dir(), rkBaseItem.Guid(), false);
}

void PgHouse::RemoveFurniture(PgBase_Item const& rkBaseItem)	//외부배치 아이템 제거
{
	m_kWall.RemoveFurniture(rkBaseItem.Guid());
}

//void PgHouse::Net_PT_C_M_REQ_HOME_EQUIP(SItemPos const& rkItemPos, POINT3 const& rkPos, short siDir, BM::GUID const& rkParentGuid, BYTE btLinkIdx)
//{
//	if(GetPilot()->GetGuid())
//	{
//		BM::Stream kPacket(PT_C_M_REQ_HOME_EQUIP);
//		kPacket.Push(GetPilot()->GetGuid());
//		kPacket.Push(rkItemPos);
//		kPacket.Push(rkPos);
//		kPacket.Push(siDir);
//		kPacket.Push(rkParentGuid);
//		kPacket.Push(btLinkIdx);
//
//		NETWORK_SEND(kPacket);
//	}
//}

void PgHouse::Net_PT_C_M_REQ_HOME_UNEQUIP(SItemPos const& rkItemPos)
{
	if(GetPilot()->GetGuid())
	{
		BM::Stream kPacket(PT_C_M_REQ_HOME_UNEQUIP);
		kPacket.Push(GetPilot()->GetGuid());
		kPacket.Push(rkItemPos);

		NETWORK_SEND(kPacket);
	}
}

void PgHouse::RecvHouse_Command(WORD const wPacketType, BM::Stream &rkPacket)
{
	switch(wPacketType)
	{
	case PT_M_C_ANS_HOME_EQUIP:
		{
			HRESULT kErr = S_OK;
			rkPacket.Pop(kErr);
			if(S_OK!=kErr)
			{
				//추가 실패
			}
			m_pkPickedFurniture = NULL;
		}break;
	case PT_C_M_REQ_HOME_ITEM_MODIFY:
		{
			HRESULT kErr = S_OK;
			rkPacket.Pop(kErr);
			if(S_OK!=kErr)
			{
				//정보 변경 실패
			}
			m_pkPickedFurniture = NULL;
		}break;
	case PT_M_C_ANS_HOME_UNEQUIP:
		{
			HRESULT kErr = S_OK;
			rkPacket.Pop(kErr);
			if(S_OK!=kErr)
			{
				//제거 실패
			}
			m_pkPickedFurniture = NULL;
		}break;
	case PT_M_C_ANS_HOME_USE_HOME_STYLEITEM:
		{
			HRESULT kErr = S_OK;
			rkPacket.Pop(kErr);
			if(S_OK!=kErr)
			{
				//제거 실패
			}
		}break;
	}
}

//! PgIWorldObject 재정의
bool PgHouse::ProcessAction(PgAction *pkAction,bool bInvalidateDirection,bool bForceToTransit)
{
	return true;
}

bool PgHouse::ParseXml(const TiXmlNode *pkNode, void *pArg, bool bUTF8)
{
	return true;
}

bool PgHouse::BuildWalls()
{
	ExteriaMap::iterator itr = m_kExteriaMap.find(EQUIP_POS_HOME_GARDEN);
	if( itr == m_kExteriaMap.end() )
	{
		return false;
	}

	NiAVObjectPtr spWallCellObject = itr->second->GetNIFRoot();
	if( !spWallCellObject )
	{
		return false;
	}
	//NiNode* pkNode = NiDynamicCast(NiNode, spWallCellObject);
	//if( !pkNode )
	//{
	//	return false;
	//}

	if( !GetPilot() || !GetPilot()->GetWorldObject() )
	{
		return false;
	}

	m_kWall.SetCellObject(spWallCellObject, true);
	m_kWall.BuildGardenCells();
	m_spWallRoot->AttachChild(m_kWall.GetWallRoot(), true);

	if (g_pkWorld)
	{
		g_pkWorld->LightObjectRecurse(g_pkWorld->GetLightRoot(), m_spWallRoot, PgWorld::LT_ALL);
	}

	//pkNode->AttachChild(m_spWallRoot);
	NiTransform kTrn = GetPilot()->GetWorldObject()->GetWorldTransform();
	m_spWallRoot->SetLocalFromWorldTransform(kTrn);

	m_spWallRoot->UpdateProperties();
	m_spWallRoot->UpdateNodeBound();
	m_spWallRoot->UpdateEffects();
	m_spWallRoot->Update(0.0f);

	return true;
}

void PgHouse::SetArrangeMode(bool bArrangeMode)
{

	if( !IsMyHouse() )
	{
		return;
	}

	m_bArrangeMode = bArrangeMode;
}

bool PgHouse::IsMyHouse()
{
	return g_kPilotMan.IsMyPlayer(GetOwnerGuid());
}

BM::GUID PgHouse::GetOwnerGuid()
{
	if(GetPilot()->GetUnit())
	{
		PgMyHome* pkMyHome = dynamic_cast<PgMyHome*>(GetPilot()->GetUnit());
		if(pkMyHome)
		{
			return pkMyHome->OwnerGuid();
		}
	}

	return BM::GUID::NullData();
}

void PgHouse::CancelLocateArrangingFurniture()
{
	if( m_pkArrangingFurniture )
	{
		PgInventory* pInv = GetPilot()->GetUnit()->GetInven();
		if(!pInv)
		{
			ClearArrangingFurniture();
		}

		PgBase_Item kOutItem;
		if(S_OK == pInv->GetItem(m_pkArrangingFurniture->GetGuid(), kOutItem ) )
		{
			AttachFurniture(m_pkArrangingFurniture->GetGuid());
		}
		else
		{
			ClearArrangingFurniture();
		}
	}

	m_pkPickedFurniture = NULL;
}

void PgHouse::ClearArrangingFurniture()
{
	if( m_pkArrangingFurniture )
	{
		m_kWall.RemoveFurniture(m_pkArrangingFurniture->GetGuid());
	}
	m_pkArrangingFurniture = NULL;
}

void PgHouse::AttachFurniture(BM::GUID const& kGuid)
{
	PgInventory* pkInv = GetPilot()->GetUnit()->GetInven();
	if(!pkInv)
	{
		return;
	}

	PgBase_Item kOutItem;
	if( S_OK != pkInv->GetItem(kGuid, kOutItem) )
	{
		return;
	}

	SHomeItem kHomeItemInfo;
	if( !kOutItem.Get(kHomeItemInfo) )
	{
		return;
	}

	int iItemNumber = kOutItem.ItemNo();
	NiPoint3 kPos(kHomeItemInfo.Pos().x, kHomeItemInfo.Pos().y, kHomeItemInfo.Pos().z);
	int iDir = kHomeItemInfo.Dir() / ROTATION_ANGLE2;

	if( m_pkArrangingFurniture && m_pkArrangingFurniture->GetGuid() == kGuid)
	{
		m_kWall.MoveFurniture(m_pkArrangingFurniture, kPos, iDir, true);
		SetArrangingFurniture();
	}
	else
	{
		PgFurniture* pkFurniture = m_kWall.GetFurniture(kGuid);
		if( pkFurniture )
		{
			m_pkArrangingFurniture = pkFurniture;
			UnlocateArrangingFurniture(m_pkArrangingFurniture);
			m_kWall.MoveFurniture(m_pkArrangingFurniture, kPos, iDir, true);
			SetArrangingFurniture();
		}
		else
		{
			int iItemNumber = kOutItem.ItemNo();
			NiPoint3 kItemPos(kHomeItemInfo.Pos().x, kHomeItemInfo.Pos().y, kHomeItemInfo.Pos().z);
			int iDir = kHomeItemInfo.Dir() / ROTATION_ANGLE2;

			pkFurniture = AddFurniture(iItemNumber, kItemPos, iDir, kOutItem.Guid());
			if( pkFurniture )
			{
				m_pkArrangingFurniture = pkFurniture;
				UnlocateArrangingFurniture(m_pkArrangingFurniture);
				m_kWall.MoveFurniture(m_pkArrangingFurniture, kPos, iDir, true);
				SetArrangingFurniture();
			}
		}
	}

	//int iItemNumber = kOutItem.ItemNo();
	//NiPoint3 kPos(kHomeItemInfo.Pos().x, kHomeItemInfo.Pos().y, kHomeItemInfo.Pos().z);
	//int iDir = kHomeItemInfo.Dir() / ROTATION_ANGLE2;

	//if( m_pkArrangingFurniture && m_pkArrangingFurniture->GetGuid() == kGuid)
	//{
	//	m_pkArrangingFurniture->SetGuid(kOutItem.Guid());
	//	m_kWall.MoveFurniture(m_pkArrangingFurniture, kPos, iDir, true);
	//	SetArrangingFurniture();
	//}
}

void PgHouse::SetArrangingFurniture()
{
	if( !m_pkArrangingFurniture )
	{
		return;
	}
	if( !LocateArrangingFurniture() )
	{
		return;
	}

	m_pkArrangingFurniture->SetFurnitureColor(NiColor::WHITE);
	m_pkArrangingFurniture = NULL;
}

bool PgHouse::LocateArrangingFurniture()
{
	if(!m_pkArrangingFurniture || IsArrangeMode() == false)
	{
		return false;
	}

	if( !m_kWall.CheckCellsBound(m_pkArrangingFurniture ) )
	{
		return false;
	}

	if( !m_kWall.LocateFurniture(m_pkArrangingFurniture) )
	{
		return false;
	}

	SetArrangeMode(false);
	return true;
}

bool PgHouse::IsArrangeMode()
{
	return m_bArrangeMode;
}

bool PgHouse::ProcessInput(PgInput *pkInput)
{
	if (pkInput == NULL || g_pkWorld == NULL)
		return false;

	if (pkInput->IsLocal())
	{
		if(1003 == (pkInput->GetUKey() - PgInput::UR_LOCAL_BEGIN))
		{
			if( !m_pkArrangingFurniture )
			{
				XUI::CXUI_Wnd* pPopMenu = XUIMgr.Get(L"CallMyHomeFurnitureMenu");
				if( !pPopMenu )
				{
					SetPickFurniture();
				}
			}
		}
	}

	if (IsArrangeMode() == false)
		return false;
	
	if (pkInput->IsLocal())
	{
		switch(pkInput->GetUKey() - PgInput::UR_LOCAL_BEGIN)
		{
		case 1001:	// mouse left down
			{
				if (m_pkArrangingFurniture && !m_pkPickedFurniture )
				{
					if( m_kWall.CheckCellsBound(m_pkArrangingFurniture) && m_kWall.CheckOtherFurniture(m_pkArrangingFurniture) )
					{
						HomeEquipRequest(m_pkArrangingFurniture);
					}
				}
				else if( m_pkArrangingFurniture && m_pkPickedFurniture)
				{
					if( m_kWall.CheckCellsBound(m_pkArrangingFurniture) && m_kWall.CheckOtherFurniture(m_pkArrangingFurniture) )
					{
						HomeItemModifyRequest(m_pkArrangingFurniture);
					}
				}
			}
			break;
		case 1003:
			{
				if (m_pkArrangingFurniture)
				{
					m_pkArrangingFurniture->SetRotation(m_pkArrangingFurniture->GetRotation() + 1);
				}
			}
			break;
		case NiInputKeyboard::KEY_ESCAPE:
			{
				CancelLocateArrangingFurniture();
				SetArrangeMode(false);
			}break;
		}
	}

	return true;
}

bool PgHouse::SetPickFurniture()
{
	if( NULL != m_pkArrangingFurniture || !g_pkWorld)
	{
		return false;
	}

	if( !IsMyHouse() )
	{
		return false;
	}

	NiCamera* pkCamera = g_pkWorld->GetCameraMan()->GetCamera();
	if( !pkCamera )
	{
		return false;
	}

	NiPoint3 kOrgPt;
	NiPoint3 kRayDir;
	POINT2 ptXUIPos = XUIMgr.MousePos();
	pkCamera->WindowPointToRay(ptXUIPos.x, ptXUIPos.y, kOrgPt, kRayDir);

	PgFurniture* pkPickedFurniture = m_kWall.PickFurniture(kOrgPt, kRayDir);

	if( !pkPickedFurniture )
	{
		return false;
	}

	int iState = GetPilot()->GetUnit()->GetAbil(AT_MYHOME_STATE);
	if( MAS_NOT_BIDDING != iState )
	{
		lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 70097, true);
		return false;
	}

	m_pkPickedFurniture = pkPickedFurniture;
	// 메뉴를 띄운다.
	lua_tinker::call<void>("CallMyHomeFurnitureMenu");

	return true;
}

bool PgHouse::HomeEquipRequest(PgFurniture* pkFurniture)
{
	if( NULL == pkFurniture)
	{
		return false;
	}

	if( !IsMyHouse() )
	{
		return false;
	}
	
	// guid
	BM::GUID kGuid(GetPilot()->GetGuid());

	// 아이템 찾기
	PgPlayer* pkMyPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkMyPlayer )
	{
		return false;
	}
	PgInventory* pkInv = pkMyPlayer->GetInven();
	if( !pkInv )
	{
		return false;
	}

	PgBase_Item kOutItem;
	SItemPos kOutPos;
	if( S_OK != pkInv->GetItem(pkFurniture->GetGuid(), kOutItem, kOutPos) )
	{
		return false;
	}

	// 배치 위치
	NiPoint3 kFurniturePos(pkFurniture->GetWorldTransform().m_Translate);
	POINT3 kPos(kFurniturePos.x, kFurniturePos.y, kFurniturePos.z);

	// 방향
	int iDir = (pkFurniture->GetRotation() % (360 / ROTATION_ANGLE2)) * ROTATION_ANGLE2;

	// 부모 가구 guid( 오브젝트 위에 올라가는 오브젝트 경우)
	BM::GUID kParentGuid(BM::GUID::NullData());

	// 부모 가구 위치( 오브젝트 위에 올라가는 오브젝트 경우 1~ )
	BYTE byPos = 0;

	Send_PT_C_M_REQ_HOME_EQUIP(kGuid, kOutPos, kPos, iDir, kParentGuid, byPos);

	return true;
}

void PgHouse::Send_PT_C_M_REQ_HOME_EQUIP(BM::GUID const& kUserGuid, SItemPos kItemPos, POINT3 kPos, SHORT kDir, BM::GUID const& kParentFurnitureGuid, BYTE byPosIndex)
{
	BM::Stream kPacket;
	kPacket.Push(PT_C_M_REQ_HOME_EQUIP);
	kPacket.Push(kUserGuid);
	kPacket.Push(kItemPos);
	kPacket.Push(kPos);
	kPacket.Push(kDir);
	kPacket.Push(kParentFurnitureGuid);
	kPacket.Push(byPosIndex);
	NETWORK_SEND(kPacket);
}

bool PgHouse::HomeItemModifyRequest(PgFurniture* pkFurniture)
{
	if( NULL == pkFurniture )
	{
		return false;
	}

	if( !IsMyHouse() )
	{
		return false;
	}
	
	// guid
	BM::GUID kGuid(GetPilot()->GetGuid());

	// 아이템 찾기
	PgInventory* pkInv = GetPilot()->GetUnit()->GetInven();
	if( !pkInv )
	{
		return false;
	}

	PgBase_Item kOutItem;
	SItemPos kOutPos;
	if( S_OK != pkInv->GetItem(pkFurniture->GetGuid(), kOutItem, kOutPos) )
	{
		return false;
	}

	// 배치 위치
	NiPoint3 kFurniturePos(pkFurniture->GetWorldTransform().m_Translate);
	POINT3 kPos(kFurniturePos.x, kFurniturePos.y, kFurniturePos.z);

	// 방향
	int iDir = (pkFurniture->GetRotation() % (360 / ROTATION_ANGLE2)) * ROTATION_ANGLE2;

	// 부모 가구 guid( 오브젝트 위에 올라가는 오브젝트 경우)
	BM::GUID kParentGuid(BM::GUID::NullData());

	// 부모 가구 위치( 오브젝트 위에 올라가는 오브젝트 경우 1~ )
	BYTE byPos = 0;

	Send_PT_C_M_REQ_HOME_ITEM_MODIFY(kGuid, kOutPos, kPos, iDir, kParentGuid, byPos);

	return true;
}

void PgHouse::Send_PT_C_M_REQ_HOME_ITEM_MODIFY(BM::GUID const& kUserGuid, SItemPos kItemPos, POINT3 kPos, SHORT kDir, BM::GUID const& kParentFurnitureGuid, BYTE byPosIndex)
{
	BM::Stream kPacket;
	kPacket.Push(PT_C_M_REQ_HOME_ITEM_MODIFY);
	kPacket.Push(kUserGuid);
	kPacket.Push(kItemPos);
	kPacket.Push(kPos);
	kPacket.Push(kDir);
	kPacket.Push(kParentFurnitureGuid);
	kPacket.Push(byPosIndex);
	NETWORK_SEND(kPacket);
}

bool PgHouse::SelectArrangingFurnitureByMenu()
{
	if( NULL != m_pkArrangingFurniture || NULL == m_pkPickedFurniture)
	{
		return false;
	}
	
	if( !UnlocateArrangingFurniture(m_pkPickedFurniture) )
	{
		return false;
	}
	m_pkArrangingFurniture = m_pkPickedFurniture;
	m_pkArrangingFurniture->SetFurnitureColor(NiColor::WHITE * 3);

	SetArrangeMode(true);
	return true;
}

void PgHouse::RemovePickedFurniture()
{
	if( !m_pkPickedFurniture )
	{
		return;
	}

	HomeUnEquipRequest(m_pkPickedFurniture);
}

bool PgHouse::HomeUnEquipRequest(PgFurniture* pkFurniture)
{
	if( NULL == pkFurniture )
	{
		return false;
	}

	if( !IsMyHouse() )
	{
		return false;
	}
	
	// guid
	BM::GUID kGuid(GetPilot()->GetGuid());

	// 가구 위치 찾기
	PgInventory* pkInv = GetPilot()->GetUnit()->GetInven();
	if( !pkInv )
	{
		return false;
	}
	PgBase_Item kOutItem;
	SItemPos kItemPos;
	if( S_OK != pkInv->GetItem(pkFurniture->GetGuid(), kOutItem, kItemPos) )
	{
		return false;
	}

	Send_PT_C_M_REQ_HOME_UNEQUIP(kGuid, kItemPos);
	
	return true;
}

void PgHouse::Send_PT_C_M_REQ_HOME_UNEQUIP(BM::GUID const& kGuid, SItemPos kItemPos)
{
	BM::Stream kPacket;
	kPacket.Push(PT_C_M_REQ_HOME_UNEQUIP);
	kPacket.Push(kGuid);
	kPacket.Push(kItemPos);
	NETWORK_SEND(kPacket);
}

bool PgHouse::UnlocateArrangingFurniture(PgFurniture* pkFurniture)
{
	if(!pkFurniture || IsArrangeMode() == false)
	{
		return false;
	}

	if( !m_kWall.UnlocateArrangingFurniture(pkFurniture) )
	{
		return false;
	}

	return true;
}

void PgHouse::AddFurniturebyInven(int iItemNo, BM::GUID const& kGuid)
{	
	if( !m_pkArrangingFurniture )
	{
		NiPoint3 kPos(0, -100, 20);
		PgFurniture* pkFurniture = AddFurniture(iItemNo, kPos, 0, kGuid);
		if( pkFurniture )
		{
			SetArrangeMode(true);

			UnlocateArrangingFurniture(pkFurniture);
			m_pkArrangingFurniture = pkFurniture;
			m_pkArrangingFurniture->SetFurnitureColor(NiColor::WHITE * 3);

			m_pkPickedFurniture = NULL;
		}
	}
}

void PgHouse::Send_PT_C_M_REQ_HOME_USE_HOME_STYLEITEM(SItemPos kItemPos, DWORD dwClientTime, BM::GUID const& kHomeGuid)
{
	BM::Stream kPacket;
	kPacket.Push(PT_C_M_REQ_HOME_USE_HOME_STYLEITEM);
	kPacket.Push(kItemPos);
	kPacket.Push(dwClientTime);
	kPacket.Push(kHomeGuid);
	NETWORK_SEND(kPacket);
}

bool PgHouse::HomeItemUseStyleItem(SItemPos const& rkItemPos)
{
	if( !GetPilot() )
	{
		return false;
	}

	Send_PT_C_M_REQ_HOME_USE_HOME_STYLEITEM(rkItemPos, lwGetServerElapsedTime32(), GetPilot()->GetGuid() );
	return true;
}

void PgHouse::UpdateHouse(float fAccumTime,float fFrameTime)
{
	if(IsArrangeMode())
	{
		SnapArrangingFurniture(fAccumTime);
	}
}

bool PgHouse::SnapArrangingFurniture(float fAccumTime)
{
	if(!m_pkArrangingFurniture || IsArrangeMode() == false)
	{
		return false;
	}

	NiPoint3 kCellPos = NiPoint3::ZERO;
	if( !PickCell(kCellPos) )
	{
		m_pkArrangingFurniture->SetFurnitureColor(NiColor(3,1,1));
		return false;
	}

	m_kWall.MoveFurniture(m_pkArrangingFurniture, kCellPos, m_pkArrangingFurniture->GetRotation());
	return true;
}

bool PgHouse::PickCell(NiPoint3& kCellPos)
{
	if( !g_pkWorld )
	{
		return false;
	}
	POINT2 ptXUIPos = XUIMgr.MousePos();

	NiPoint3 kOrgPt = NiPoint3::ZERO;
	NiPoint3 kRayDir = NiPoint3::ZERO;
	kCellPos = NiPoint3::ZERO;
	g_pkWorld->m_kCameraMan.GetCamera()->WindowPointToRay(ptXUIPos.x, ptXUIPos.y, kOrgPt, kRayDir);

	NiAVObject* pkCell = m_kWall.PickCell(kOrgPt, kRayDir, kCellPos);
	if (pkCell == NULL)
	{
		return false;
	}

	return true;		
}
