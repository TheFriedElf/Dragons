#include "StdAfx.h"
#include "PgHome.h"
#include "PgWall.h"
#include "XUI/XUI_Manager.h"
#include "PgWorld.h"
#include "PgFurniture.h"
#include "PgNifMan.h"
#include "PgRenderer.h"
#include "PgInput.h"

#include "PgPilotMan.h"
#include "PgNetwork.h"
#include "lwUI.h"
#include "lwWorld.h"
#include "PgUISound.h"
#include "PgOption.h"
#include "PgRenderMan.h"
#include "PgPilot.h"
#include "PgHomeRenew.h"
#include "NewWare/Scene/ApplyTraversal.h"
#include "lwHomeUI_Script.h"


EEquipPos const PgHome::TilePosArray[DefaultTileCount] = {EQUIP_POS_ROOM_WALL, EQUIP_POS_ROOM_FLOOR};
int const PgHome::DefaultTileArray[DefaultTileCount]= {70000540, 70000550};
int const CORNER_NO = 70000800;

PgHome::PgHome(PgWorld *pkWorld)
{
	PG_ASSERT_LOG(pkWorld);
	m_pkWorld = pkWorld;
	m_bArrangeMode = false;
	m_pkArrangingFurniture = NULL;
	m_kRoomSize = POINT3I(0, 0, 0);
	m_spRoot = NULL;
	m_spWallRoot = NULL;
	m_kCenterPos = NiPoint3::ZERO;
	m_pkHomeUnit = NULL;
	m_pkPickedFurniture = NULL;
	m_pkWallTile = NULL;
	m_pkWallCornerTile = NULL;
	m_pkFloorTile = NULL;
	m_bAnitAlias = true;
}

PgHome::~PgHome()
{
	Terminate();
}

void PgHome::Initialize()
{
	if (m_spRoot == NULL)
	{
		m_spRoot = NiNew NiNode();
	}
	PG_ASSERT_LOG(m_spRoot);

	if (m_spWallRoot == NULL)
	{
		m_spWallRoot = NiNew NiNode();
	}
	PG_ASSERT_LOG(m_spWallRoot);

	if (m_spRoot && m_spWallRoot)
	{
		m_spRoot->AttachChild(m_spWallRoot, true);
	}
	
	if( !GET_OPTION_ANTI_ALIAS )
	{
		m_bAnitAlias = false;
		g_kGlobalOption.SetCurrentGraphicOption(STR_OPTION_ANTI_ALIAS, 1);
		g_kRenderMan.SetGraphicOption(STR_OPTION_ANTI_ALIAS, 1);
	}

	m_kRemoveFurniture.clear();
	while( !m_kUnsetFurniture.empty() )
	{
		m_kUnsetFurniture.pop();
	}
	//ParseXml("HomeType.xml");
}

void PgHome::Terminate()
{
	CancelLocateArrangingFurniture();
	RemoveAllFurniture();
	RemoveWall();

	if (m_spRoot)
	{
		NiNode* pkParent = m_spRoot->GetParent();
		if (pkParent)
		{
			pkParent->DetachChild(m_spRoot);			
		}
		m_spRoot = NULL;
	}

	if (m_spWallRoot)
	{
		if( m_pkWallCornerTile )
		{
			m_spWallRoot->DetachChild(m_pkWallCornerTile->GetNIFRoot());
		}
		NiNode* pkParent = m_spWallRoot->GetParent();
		if (pkParent)
		{
			pkParent->DetachChild(m_spWallRoot);			
		}
		m_spWallRoot = NULL;
	}
	ReleaseWallTile();

	if( !m_bAnitAlias )
	{
		g_kGlobalOption.SetCurrentGraphicOption(STR_OPTION_ANTI_ALIAS, 0);
		g_kRenderMan.SetGraphicOption(STR_OPTION_ANTI_ALIAS, 0);
	}
}

BM::GUID PgHome::GetOwnerGuid()
{
	if( GetHomeUnit() )
	{
		return GetHomeUnit()->OwnerGuid();
	}

	return BM::GUID::NullData();
}

void PgHome::BuildWalls()
{
	if (NULL == m_pkFloorTile || NULL == m_pkWallTile/* || NULL == m_pkWallCornerTile*/)
	{
		return;
	}

	NiAVObjectPtr spFloorCellObject = g_kNifMan.GetNif(m_pkFloorTile->GetMeshPath());
	NiAVObjectPtr spWallCellObject = g_kNifMan.GetNif(m_pkWallTile->GetMeshPath());
	if (spFloorCellObject == NULL || spWallCellObject == NULL)
	{
		return;
	}

	if( 0 == m_kRoomSize.x || 0 == m_kRoomSize.y )
	{
		SetRoomSize(POINT3I(200,200,160));// 기본 사이즈
	}
	PgWall* pkWallLeft = NiNew PgWall();
	if (pkWallLeft)
	{
		pkWallLeft->SetWallInfo(PgWall::WALL_TYPE_WALL, NiPoint3(m_kRoomSize.x, m_kRoomSize.y, m_kRoomSize.z), NiPoint3(0, 0, 0));
		pkWallLeft->SetCellObject(spWallCellObject, true);
		pkWallLeft->SetPhysxNum(m_pkWallTile->GetFurnitureNo());
		pkWallLeft->BuildWallCells();
		m_spWallRoot->AttachChild(pkWallLeft->GetWallRoot(), true);
		m_kWallContainer.push_back(pkWallLeft);
	}

	PgWall* pkWallFloor = NiNew PgWall();
	if (pkWallFloor)
	{
		pkWallFloor->SetWallInfo(PgWall::WALL_TYPE_FLOOR, NiPoint3(m_kRoomSize.x, m_kRoomSize.y, 0), NiPoint3(0, 0, 0));
		pkWallFloor->SetCellObject(spFloorCellObject, true);
		pkWallFloor->SetPhysxNum(m_pkFloorTile->GetFurnitureNo());
		pkWallFloor->BuildWallCells();
		m_spWallRoot->AttachChild(pkWallFloor->GetWallRoot(), true);
		m_kWallContainer.push_back(pkWallFloor);
	}

	//m_spWallRoot->AttachChild(m_pkWallCornerTile->GetNIFRoot(), true);

	SetLight();
	m_spWallRoot->UpdateProperties();
	m_spWallRoot->UpdateNodeBound();
	m_spWallRoot->UpdateEffects();
	m_spWallRoot->Update(0.0f);
}

PgWall* PgHome::GetWall(PgWall::WallType eWallType)
{
	if (eWallType <= PgWall::WALL_TYPE_NONE || eWallType >= PgWall::MAX_WALL_TYPE)
		return NULL;

	for(WallContainer::iterator itr = m_kWallContainer.begin();
		itr != m_kWallContainer.end();
		++itr)
	{
		if((*itr) && (*itr)->GetWallType() == eWallType)
		{
			return (*itr);
		}
	}

	return NULL;
}

bool PgHome::IsInside(const NiPoint3 &rkPt)
{
	for(WallContainer::iterator itr = m_kWallContainer.begin();
		itr != m_kWallContainer.end();
		++itr)
	{
		if((*itr)->IsInside(rkPt))
		{
			return true;
		}
	}

	return false;
}

void PgHome::Build()
{

}

void PgHome::SetArrangeMode(bool bArrangeMode)
{

	if( !IsMyHome() )
	{
		return;
	}

	m_bArrangeMode = bArrangeMode;
}

bool PgHome::SelectArrangingFurniture()
{
	if( NULL != m_pkArrangingFurniture )
	{
		return false;
	}

	if( !IsMyHome() )
	{
		return false;
	}

	NiCamera* pkCamera = m_pkWorld->GetCameraMan()->GetCamera();
	if( !pkCamera )
	{
		return false;
	}

	NiPoint3 kOrgPt;
	NiPoint3 kRayDir;
	POINT2 ptXUIPos = XUIMgr.MousePos();
	pkCamera->WindowPointToRay(ptXUIPos.x, ptXUIPos.y, kOrgPt, kRayDir);

	PgFurniture* pkPickedFurniture = pickFurniture(kOrgPt, kRayDir);

	if( !pkPickedFurniture )
	{
		return false;
	}

	if( !UnlocateArrangingFurniture(pkPickedFurniture) )
	{
		return false;
	}
	m_pkArrangingFurniture = pkPickedFurniture;
	m_pkArrangingFurniture->SetFurnitureColor(NiColor::WHITE * 2);

	m_pkPickedFurniture = m_pkArrangingFurniture;

	return true;
}
void PgHome::ClearArrangingFurniture()
{
	if( m_pkArrangingFurniture )
	{
		RemoveFurniturebyServer(m_pkArrangingFurniture->GetGuid());
	}
	m_pkArrangingFurniture = NULL;
	m_kLastMousePos.Set(0, 0);
}

bool PgHome::IsArrangeMode()
{
	return m_bArrangeMode;
}

void PgHome::Update(float fAccumTime,float fFrameTime)
{
	if(IsArrangeMode())
	{
		SnapArrangingFurniture(fAccumTime);
	}
	
	
	for(WallContainer::iterator itr = m_kWallContainer.begin();
		itr != m_kWallContainer.end();
		++itr)
	{
		if ((*itr) != NULL)
			(*itr)->Update(fAccumTime, fFrameTime);
	}

	FurnitureContainer::iterator kIter = m_kHomeFurnitures.begin();

	while (kIter != m_kHomeFurnitures.end())
	{
		PgFurniture* pkFurniture = *(kIter);
		if (pkFurniture)
			pkFurniture->Update(fAccumTime, fFrameTime);
		++kIter;
	}
}

void PgHome::Draw(PgRenderer *pkRenderer, NiCameraPtr spCamera, float fFrameTime)
{
	for(WallContainer::iterator itr = m_kWallContainer.begin();
		itr != m_kWallContainer.end();
		++itr)
	{
		if ((*itr) != NULL)
			(*itr)->Draw(pkRenderer, spCamera, fFrameTime);
	}

	FurnitureContainer::iterator kIter = m_kHomeFurnitures.begin();

	while (kIter != m_kHomeFurnitures.end())
	{
		PgFurniture* pkFurniture = *(kIter);
		if (pkFurniture)
			pkFurniture->Draw(pkRenderer, spCamera, fFrameTime);
		++kIter;
	}
}

bool PgHome::ProcessInput(PgInput *pkInput)
{
	if (pkInput == NULL || m_pkWorld == NULL)
		return false;

	if (pkInput->IsLocal())
	{
		PgFurniture* pkTempPrevPickedFurniture = m_pkPickedFurniture;

		switch(pkInput->GetUKey() - PgInput::UR_LOCAL_BEGIN)
		{
		case 1003:
			{// mouse right down
				if( !m_pkArrangingFurniture )
				{
					XUI::CXUI_Wnd* pPopMenu = XUIMgr.Get(L"CallMyHomeFurnitureMenu");
					if( !pPopMenu )
					{
						if(SetPickFurniture())
						{// 자신일때만 마이홈 가구 팝업을 띄움
							lua_tinker::call<void>("CallMyHomeFurnitureMenu");
						}
					}
				}
			}break;
		default:
			{
			}break;
		}
	}

	if (IsArrangeMode() == false)
	{// 가구 배치 모드가 아니라면
		return false;
	}
	
	if (pkInput->IsLocal())
	{
		switch(pkInput->GetUKey() - PgInput::UR_LOCAL_BEGIN)
		{
		case 1001:	// mouse left down
			{
				HomeItemRequest();
			}
			break;
		case 1002:	// mouse left up
			{
			}
			break;
		case 1003:
			{
				if (m_pkArrangingFurniture)
				{
					if( IsMaidItem(m_pkArrangingFurniture->GetFurnitureNo() ) )
					{
						break;
					}
					m_pkArrangingFurniture->SetRotation(m_pkArrangingFurniture->GetRotation() + 1);
					lwPlaySoundByID("MyHome_Item_Spin");
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

bool PgHome::SnapArrangingFurniture(float fAccumTime)
{
	if(!m_pkArrangingFurniture || IsArrangeMode() == false)
	{
		return false;
	}

	PgWall::WallType eWallType = GetWallTypetoFurniture(m_pkArrangingFurniture->GetFurnitureType());

	if (eWallType == PgWall::WALL_TYPE_NONE)
		return false;

	PgWall* pkWall = GetWall(eWallType);
	if (pkWall == NULL)
		return false;

	NiPoint3 kCellPos = NiPoint3::ZERO;
	if( !PickCell(pkWall, kCellPos) )
	{
		m_pkArrangingFurniture->SetFurnitureColor(NiColor(3,1,1));
		return false;
	}

	pkWall->MoveFurniture(m_pkArrangingFurniture, kCellPos, m_pkArrangingFurniture->GetRotation());
	return true;
}

void PgHome::SetArrangingFurniture(bool bForce)
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
	if( m_pkArrangingFurniture->GetItemDef() && IsMaidItem(m_pkArrangingFurniture->GetItemDef()->No()) )
	{
		AddMaidNpc(m_pkArrangingFurniture->GetItemDef()->No(), m_pkArrangingFurniture->GetWorldTranslate());
		SetMaidColor(m_pkArrangingFurniture->GetItemDef()->No(), NiColor::WHITE);
		m_pkArrangingFurniture->SetAppCulled(true);
	}
	m_pkArrangingFurniture = NULL;
	m_kLastMousePos.Set(0, 0);
	lwPlaySoundByID("MyHome_Item_Drop");
}

void PgHome::SetArrangingFurniture(PgFurniture *pkFurniture)
{
	if(m_pkArrangingFurniture)
	{
		m_pkWorld->RemoveObject(m_pkArrangingFurniture->GetGuid());
	}

	POINT2 ptXUIPos = XUIMgr.MousePos();
	NiPoint3 kOrgPt;
	NiPoint3 kRayDir;
	m_pkWorld->m_kCameraMan.GetCamera()->WindowPointToRay(ptXUIPos.x, ptXUIPos.y, kOrgPt, kRayDir);

	m_pkArrangingFurniture = pkFurniture;
	m_pkWorld->AddObject(m_pkArrangingFurniture->GetGuid(), pkFurniture, kOrgPt + kRayDir * 400.0f, OGT_NPC);
}

PgFurniture *PgHome::GetArrangingFurniture()
{
	return m_pkArrangingFurniture;
}

void PgHome::SetHomeOrigin(NiPoint3& rkOrigin)
{
	for(WallContainer::iterator itr = m_kWallContainer.begin();
		itr != m_kWallContainer.end();
		++itr)
	{
		if ((*itr) != NULL)
			(*itr)->SetWallOrigin(rkOrigin);
	}
}

void PgHome::SetLight()
{
	if (m_pkWorld)
	{
		m_pkWorld->LightObjectRecurse(m_pkWorld->GetLightRoot(), m_spRoot, PgWorld::LT_ALL);
		m_spRoot->UpdateEffects();
        NewWare::Scene::ApplyTraversal::Geometry::SetDefaultMaterialNeedsUpdateFlag( m_spRoot, false );
	}
}

PgFurniture* PgHome::AddFurniture(int iFurnitureNo, NiPoint3& kPos, int iRotate, BM::GUID const& kGuid, bool bLocate, BM::GUID const& kParentGuid, BYTE byLinkIndex)
{
	PgFurniture* pkFurniture = NULL;
	pkFurniture = PgFurniture::GetFurnitureFromDef(iFurnitureNo);
	if (pkFurniture == NULL)
	{
		PgError1("[PgHome] AddFurniture, can't create %d Item\n", iFurnitureNo);
		return NULL;
	}

	if( !m_pkWorld )
	{
		return NULL;
	}

	if( m_pkArrangingFurniture )
	{
		return NULL;
	}

	PgWall::WallType eFindWallType = GetWallTypetoFurniture(pkFurniture->GetFurnitureType());

	if (eFindWallType == PgWall::WALL_TYPE_NONE)
	{
		PgFurniture::DeleteFurniture(pkFurniture);
		return NULL;
	}

	PgWall* pkWall = GetWall(eFindWallType);
	if (pkWall == NULL)
	{
		PgFurniture::DeleteFurniture(pkFurniture);
		return NULL;
	}

	pkFurniture->InitPhysX(m_pkWorld->GetPhysXScene(), OGT_FURNITURE);

	if (pkWall->AttachFurniture(pkFurniture, kPos, iRotate, kGuid) == false )
	{
		PgFurniture::DeleteFurniture(pkFurniture);
		return NULL;
	}

	if( pkFurniture->GetFurnitureType() == FURNITURE_TYPE_OBJECT_ATTACH )
	{
		pkFurniture->SetParentFurniture(GetFurniture(kParentGuid));
		pkFurniture->SetParentIndex(byLinkIndex);

		if( bLocate )
		{	
			if( !pkWall->LocateObjectAttachFurniture(pkFurniture) )
			{
				m_kRemoveFurniture.push_back(pkFurniture);
				//pkWall->RemoveFurniture(pkFurniture);
				//PgFurniture::DeleteFurniture(pkFurniture);
				return NULL;
			}
		}
		else
		{
			if( pkWall->GetWallRoot() )
			{
				NiNode* pkNode = NiDynamicCast(NiNode, pkWall->GetWallRoot());
				if( pkNode )
				{
					pkNode->AttachChild(pkFurniture->GetNIFRoot());
				}
			}
		}
	}
	else
	{
		if( bLocate )
		{
			pkWall->LocateFurniture(pkFurniture);

			if( IsMaidItem(iFurnitureNo) )
			{
				AddMaidNpc(iFurnitureNo, kPos);
				pkFurniture->SetAppCulled(true);

			}
		}
	}

	//m_pkArrangingFurniture = pkFurniture;

	m_pkWorld->LightObjectRecurse(m_pkWorld->GetLightRoot(), NiDynamicCast(NiNode, pkFurniture->GetNIFRoot()), PgWorld::LT_ALL);
	pkFurniture->UpdateEffects();
    NewWare::Scene::ApplyTraversal::Geometry::SetDefaultMaterialNeedsUpdateFlag( pkFurniture->GetNIFRoot(), false );

	//pkFurniture->Guid(kGuid);
	return pkFurniture;
}

void PgHome::RemoveAllFurniture()
{
	for(WallContainer::iterator itr = m_kWallContainer.begin();
		itr != m_kWallContainer.end();
		++itr)
	{
		if ((*itr) != NULL)
			(*itr)->RemoveAllFurniture();
	}

	FurnitureContainer::iterator kIter = m_kHomeFurnitures.begin();

	while (kIter != m_kHomeFurnitures.end())
	{
		PgFurniture* pkFurniture = *(kIter);
		if( pkFurniture )
		{
			pkFurniture->Release();
			PgFurniture::DeleteFurniture(pkFurniture);
		}
		++kIter;
	}
	m_kHomeFurnitures.clear();
}

void PgHome::AddWall(NiPoint3& rkPos, int iRotate)
{
	BuildWalls();
	if (m_spWallRoot)
	{
		NiQuaternion kRot = NiQuaternion::IDENTITY;
		kRot.FromAngleAxis(NI_HALF_PI*0.5f*iRotate, NiPoint3::UNIT_Z);
		m_spWallRoot->SetTranslate(rkPos);
		m_spWallRoot->SetRotate(kRot);
		m_spWallRoot->Update(0.0f);
	}
}

void PgHome::RemoveWall()
{
	for(WallContainer::iterator itr = m_kWallContainer.begin();
		itr != m_kWallContainer.end();
		++itr)
	{
		if ((*itr) != NULL)
		{
			(*itr)->Terminate();
			NiDelete (*itr);
		}
	}

	m_kWallContainer.clear();
}

PgFurniture* PgHome::pickFurniture(NiPoint3& kStartPt, NiPoint3& kRayDir)
{
	PgFurniture* pkPickedFurniture = NULL;
	PgWall* pkWall = GetWall(PgWall::WALL_TYPE_FLOOR);
	if (pkWall)
	{
		pkPickedFurniture = pkWall->PickFurniture(kStartPt, kRayDir);
	}

	if (pkPickedFurniture)
		return pkPickedFurniture;

	pkWall = GetWall(PgWall::WALL_TYPE_WALL);
	if (pkWall)
	{
		pkPickedFurniture = pkWall->PickFurniture(kStartPt, kRayDir);
	}

	if (pkPickedFurniture)
		return pkPickedFurniture;
	
	return NULL;
}

//bool PgHome::ParseXml(char const *pcXmlPath)
//{
//	TiXmlDocument kXmlDoc(pcXmlPath);
//	if(!PgXmlLoader::LoadFile(kXmlDoc, UNI(pcXmlPath)))
//	{
//		PgError1("Parse Failed [%s]", pcXmlPath);
//		return false;
//	}
//	
//	// Root 'HOME'
//	const TiXmlElement *pkElement = kXmlDoc.FirstChildElement();
//
//	assert(strcmp(pkElement->Value(), "HOME") == 0);
//
//	pkElement = pkElement->FirstChildElement();
//	while(pkElement)
//	{
//		char const *pcTagName = pkElement->Value();
//
//		if(strcmp(pcTagName, "ITEM") == 0)
//		{
//			const TiXmlAttribute *pkAttr = pkElement->FirstAttribute();
//			int iType = 0;
//			int iPos = 0;
//
//			while(pkAttr)
//			{
//				char const *pcAttrName = pkAttr->Name();
//				char const *pcAttrValue = pkAttr->Value();
//
//				if(strcmp(pcAttrName, "TYPE") == 0)
//				{
//					iType = atoi(pcAttrValue);
//				}
//				else if(strcmp(pcAttrName, "POS") == 0)
//				{
//					iPos = atoi(pcAttrValue);
//				}
//				else
//				{
//					PgXmlError1(pkElement, "XmlParse: Incoreect Attr '%s'", pcAttrName);
//				}
//
//				pkAttr = pkAttr->Next();
//			}
//
//			STypeContainer sType;
//			sType.iType = iType;
//			sType.iPos = iPos;
//
//			m_kTypeContainer.insert(std::make_pair(iType, sType));
//		}
//		else
//		{
//			PgXmlError1(pkElement, "XmlParse: Incoreect Tag '%s'", pcTagName);
//		}
//
//		pkElement = pkElement->NextSiblingElement();
//	}
//	
//	return true;
//}

//PgFurniture* PgHome::AddFurnitureFirst(int iFurnitureNo)
//{
//	PgFurniture* pkFurniture = NULL;
//	pkFurniture = PgFurniture::GetFurnitureFromDef(iFurnitureNo);
//	if (pkFurniture == NULL)
//	{
//		PgError1("[PgHome] AddFurniture, can't create %d Item\n", iFurnitureNo);
//		return NULL;
//	}
//
//	// m_kTypeContainer 참조해서 해당 위치에 오브젝트를 붙인다.
//	int iDetailType = pkFurniture->GetFurnitureDetailType();
//	int iPos = 0;
//	TypeContainer::iterator iter = m_kTypeContainer.find(iDetailType);
//	if( iter != m_kTypeContainer.end() )
//	{
//		STypeContainer pType = iter->second;
//		iPos = pType.iPos;
//	}
//	BM::vstring kNodeName("MapDummy");
//	kNodeName+=iPos;
//	NiAVObject *pkNode = m_pkWorld->GetSceneRoot()->GetObjectByName(MB(kNodeName));
//	NiPoint3 kPos(0,0,0);
//	if( pkNode )
//	{
//		kPos = pkNode->GetTranslate();
//	}
//	// 임시
//	PgActor* pPlayer = g_kPilotMan.GetPlayerActor();
//	if( pPlayer )
//	{
//		kPos = pPlayer->GetTranslate();
//	}
//	/////
//	
//	m_kHomeFurnitures.push_back(pkFurniture);
//	GetHomeRoot()->AttachChild(pkFurniture);
//	
//	NiQuaternion kRot = NiQuaternion::IDENTITY;
//	kRot.FromAngleAxis(NI_HALF_PI*0.5f, NiPoint3::UNIT_Z);
//	
//	pkFurniture->SetTranslate(kPos);
//	pkFurniture->SetRotate(kRot);
//	pkFurniture->SetWorldTranslate(kPos);
//
//	if (m_pkWorld)
//	{
//		m_pkWorld->LightObjectRecurse(m_pkWorld->GetLightRoot(), NiDynamicCast(NiNode, pkFurniture->GetNIFRoot()));
//		pkFurniture->UpdateEffects();
//      NewWare::Scene::ApplyTraversal::Geometry::SetDefaultMaterialNeedsUpdateFlag( pkFurniture->GetNIFRoot(), false );
//	}
//
//	return pkFurniture;
//}

//void PgHome::RemoveFurnitureFirst(int iFurnitureNo)
//{
//	FurnitureContainer::iterator itor = m_kHomeFurnitures.begin();
//
//	while (itor != m_kHomeFurnitures.end())
//	{
//		PgFurniture* pkFurniture = *(itor);
//		if( pkFurniture )
//		{
//			if( pkFurniture->GetFurnitureNo() == iFurnitureNo )
//			{
//				PgFurniture::DeleteFurniture(pkFurniture);
//				return;
//			}
//		}
//		++itor;
//	}
//}

bool PgHome::LocateArrangingFurniture()
{
	if(!m_pkArrangingFurniture || (IsArrangeMode() == false && IsMyHome() ) )
	{
		return false;
	}

	PgWall::WallType eWallType = GetWallTypetoFurniture(m_pkArrangingFurniture->GetFurnitureType());

	if (eWallType == PgWall::WALL_TYPE_NONE)
	{
		return false;
	}

	PgWall* pkWall = GetWall(eWallType);
	if (pkWall == NULL)
	{
		return false;
	}

	if( m_pkArrangingFurniture->GetFurnitureType() == FURNITURE_TYPE_OBJECT_ATTACH )
	{
		pkWall->LocateObjectAttachFurniture(m_pkArrangingFurniture);
	}
	else
	{
		pkWall->LocateFurniture(m_pkArrangingFurniture);
	}

	SetLight();
	SetArrangeMode(false);
	return true;
}

bool PgHome::UnlocateArrangingFurniture(PgFurniture* pkFurniture)
{
	if(!pkFurniture || (IsArrangeMode() == false && IsMyHome()) )
	{
		return false;
	}

	PgWall::WallType eWallType = GetWallTypetoFurniture(pkFurniture->GetFurnitureType());

	if (eWallType == PgWall::WALL_TYPE_NONE)
	{
		return false;
	}

	PgWall* pkWall = GetWall(eWallType);
	if (pkWall == NULL)
	{
		return false;
	}

	if( !pkWall->UnlocateArrangingFurniture(pkFurniture) )
	{
		return false;
	}

	return true;
}

void PgHome::SetHomeUnit(CUnit* pkUnit)
{
	m_pkHomeUnit = static_cast<PgMyHome*>(pkUnit);
}
PgMyHome* PgHome::GetHomeUnit()
{
	return m_pkHomeUnit;
}
bool PgHome::HomeUnitAddProcess()
{
	if( !GetHomeUnit() )
	{
		return false;
	}
	if( !g_kPilotMan.FindPilot(GetHomeUnit()->GetID()) )
	{
		return false;
	}
	PgInventory* pkInv = GetHomeUnit()->GetInven();
	if( !pkInv )
	{
		return false;
	}

	SetAllWallTile();
	AddWall(GetCenterPos(), 4);

	RemoveAllFurniture();
	ReleaseCellsOccupancy();
	m_pkArrangingFurniture = NULL;

	const int iMaxInv = pkInv->GetMaxIDX(IT_HOME);
	for( int ii = 0; ii < iMaxInv; ++ii )
	{
		PgBase_Item kItem;
		SItemPos kPos(IT_HOME, ii);
		if(S_OK != pkInv->GetItem(kPos, kItem))
		{
			continue;
		}

		SHomeItem kHomeItemInfo;
		if( !kItem.Get(kHomeItemInfo) )
		{
			continue;
		}

		int iItemNumber = kItem.ItemNo();
		NiPoint3 kItemPos(kHomeItemInfo.Pos().x, kHomeItemInfo.Pos().y, kHomeItemInfo.Pos().z);
		int iDir = kHomeItemInfo.Dir() / ROTATION_ANGLE2;

		//if( IsMaidItem(iItemNumber) )
		//{
		//	AddMaidNpc(iItemNumber, kItemPos);
		//	if( kItem.GetUseAbleTime() < 0 )
		//	{
		//		SetMaidColor(iItemNumber, NiColor(0.25,0.25,0.25));
		//	}
		//	continue;
		//}

		PgFurniture* pkFurniture = AddFurniture(iItemNumber, kItemPos, iDir, kItem.Guid(), true, kHomeItemInfo.ParentGuid(), kHomeItemInfo.LinkIdx());
		if( pkFurniture )
		{
			if( kItem.GetUseAbleTime() < 0 )
			{
				pkFurniture->SetFurnitureColor(NiColor(0.25,0.25,0.25));
				if( IsMaidItem(iItemNumber) )
				{
					SetMaidColor(iItemNumber, NiColor(0.25,0.25,0.25));
				}
			}
		}
	}

	SetCharacterRegenPos(g_kPilotMan.GetPlayerActor());

	RemoveFurnitreProcess();

	lwHomeUI::OnCallHomeInOwnerAndUserUI();

	return true;
}

void PgHome::SetCharacterRegenPos(PgActor* pkActor)
{
	if( !pkActor )
	{
		return;
	}

	PgWall* pkWall = GetWall(PgWall::WALL_TYPE_WALL);
	if( pkWall )
	{
		PgFurniture* pkFurniture = pkWall->GetDoorTile();
		if( pkFurniture )
		{
			NiPoint3 kPos = pkFurniture->GetWorldTranslate();
			if( pkActor )
			{
				kPos.z += 35;
				pkActor->SetPosition(kPos);
			}
		}
	}
}

bool PgHome::CheckErrorMsg(HRESULT kResult, bool bShowMessageBox)
{
	switch(kResult)
	{
	case EC_OK:
		{
			return true;
		}break;
	case E_MYHOME_ALREADY_STARTED_JOB:
		{
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201536, true);
		}break;
	case E_MYHOME_NOT_FOUND_JOB:
		{
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201537, true);
		}break;
	case E_MYHOME_HAVE_SIDE_JOB:
		{
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201538, true);
		}break;
	case E_MYHOME_HOME_INVEN_FULL:
		{
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 3074, true);
		}break;
	}

	return false;
}
void PgHome::ReceivePacket_Command(WORD const wPacketType, BM::Stream& kPacket)
{
	switch(wPacketType)
	{
	case PT_M_C_ANS_HOME_EQUIP:
		{
			HRESULT kErr = S_OK;
			kPacket.Pop(kErr);
			if(S_OK!=kErr)
			{
				CheckErrorMsg(kErr, true);
			}
			m_pkPickedFurniture = NULL;
		}break;
	case PT_C_M_REQ_HOME_ITEM_MODIFY:
		{
			HRESULT kErr = S_OK;
			kPacket.Pop(kErr);
			if(S_OK!=kErr)
			{
				CheckErrorMsg(kErr, true);
			}
			m_pkPickedFurniture = NULL;
		}break;
	case PT_M_C_ANS_HOME_UNEQUIP:
		{
			HRESULT kErr = S_OK;
			kPacket.Pop(kErr);
			if(S_OK!=kErr)
			{
				CheckErrorMsg(kErr, true);
			}
			else
			{
				RemoveFurnitreProcess();
			}
			m_pkPickedFurniture = NULL;
		}break;
	case PT_M_C_ANS_HOME_USE_HOME_STYLEITEM:
		{
			HRESULT kErr = S_OK;
			kPacket.Pop(kErr);
			if(S_OK!=kErr)
			{
				CheckErrorMsg(kErr, true);
			}
		}break;
	}

	XUIMgr.Close(L"FRM_MYHOME_EDIT_MODE");
}

void PgHome::Send_PT_C_M_REQ_HOME_EQUIP(BM::GUID const& kUserGuid, SItemPos kItemPos, POINT3 kPos, SHORT kDir, BM::GUID const& kParentFurnitureGuid, BYTE byPosIndex)
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

void PgHome::Send_PT_C_M_REQ_HOME_ITEM_MODIFY(BM::GUID const& kUserGuid, SItemPos kItemPos, POINT3 kPos, SHORT kDir, BM::GUID const& kParentFurnitureGuid, BYTE byPosIndex)
{
	//m_pkArrangingFurniture = NULL;
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

void PgHome::Send_PT_C_M_REQ_HOME_UNEQUIP(BM::GUID const& kGuid, SItemPos kItemPos)
{
	BM::Stream kPacket;
	kPacket.Push(PT_C_M_REQ_HOME_UNEQUIP);
	kPacket.Push(kGuid);
	kPacket.Push(kItemPos);
	NETWORK_SEND(kPacket);
}

bool PgHome::HomeEquipRequest(PgFurniture* pkFurniture, BM::GUID const& kParentGuid, BYTE byPos)
{
	if( NULL == pkFurniture || !GetHomeUnit())
	{
		return false;
	}

	if( !IsMyHome() )
	{
		return false;
	}
	
	// guid
	BM::GUID kGuid(GetHomeUnit()->GetID());

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
	//BM::GUID kParentGuid(BM::GUID::NullData());

	// 부모 가구 위치( 오브젝트 위에 올라가는 오브젝트 경우 1~ )
	//BYTE byPos = 0;

	Send_PT_C_M_REQ_HOME_EQUIP(kGuid, kOutPos, kPos, iDir, kParentGuid, byPos);

	return true;
}

bool PgHome::HomeItemModifyRequest(PgFurniture* pkFurniture, BM::GUID const& kParentGuid, BYTE byPos)
{
	if( NULL == pkFurniture || !GetHomeUnit())
	{
		return false;
	}

	if( !IsMyHome() )
	{
		return false;
	}
	
	// guid
	BM::GUID kGuid(GetHomeUnit()->GetID());

	// 아이템 찾기
	PgInventory* pkInv = GetHomeUnit()->GetInven();
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
	//BM::GUID kParentGuid(BM::GUID::NullData());

	// 부모 가구 위치( 오브젝트 위에 올라가는 오브젝트 경우 1~ )
	//BYTE byPos = 0;

	Send_PT_C_M_REQ_HOME_ITEM_MODIFY(kGuid, kOutPos, kPos, iDir, kParentGuid, byPos);

	return true;
}

bool PgHome::HomeUnEquipRequest(PgFurniture* pkFurniture)
{
	if( NULL == pkFurniture || !GetHomeUnit())
	{
		return false;
	}

	if( !IsMyHome() )
	{
		return false;
	}
	
	// guid
	BM::GUID kGuid(GetHomeUnit()->GetID());

	// 가구 위치 찾기
	if( !GetHomeUnit() )
	{
		return false;
	}
	PgInventory* pkInv = GetHomeUnit()->GetInven();
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
// 아이템위치, 아이템번호, 놓일 위치, 방향, 아이템guid
void PgHome::AddFurniturebyGuid(BM::GUID const& kGuid)
{
	PgInventory* pkInv = GetHomeUnit()->GetInven();
	if( !pkInv )
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

	//if( IsMaidItem(iItemNumber) )
	//{
	//	AddMaidNpc(iItemNumber);
	//	return;
	//}

	if( m_pkArrangingFurniture && m_pkArrangingFurniture->GetGuid() == kGuid)
	{
		PgWall::WallType eWallType = GetWallTypetoFurniture(m_pkArrangingFurniture->GetFurnitureType());
		if( PgWall::WALL_TYPE_NONE == eWallType )
		{
			return;
		}
		PgWall* pkWall = GetWall(eWallType);
		if( NULL == pkWall )
		{
			return;
		}

		if( m_pkArrangingFurniture->GetFurnitureType() != FURNITURE_TYPE_OBJECT_ATTACH )
		{
			pkWall->MoveFurniture(m_pkArrangingFurniture, kPos, iDir, true);
		}
		else
		{	
			m_pkArrangingFurniture->SetParentFurniture(GetFurniture(kHomeItemInfo.ParentGuid()));
			m_pkArrangingFurniture->SetParentIndex(kHomeItemInfo.LinkIdx());
		}
		SetArrangingFurniture(true);
	}
	else
	{
		PgFurniture* pkFurniture = GetFurniture(kGuid);
		if( pkFurniture )
		{
			PgWall::WallType eWallType = GetWallTypetoFurniture(pkFurniture->GetFurnitureType());
			if( PgWall::WALL_TYPE_NONE == eWallType )
			{
				return;
			}
			PgWall* pkWall = GetWall(eWallType);
			if( NULL == pkWall )
			{
				return;
			}

			m_pkArrangingFurniture = pkFurniture;
			UnlocateArrangingFurniture(m_pkArrangingFurniture);
			if( m_pkArrangingFurniture->GetFurnitureType() != FURNITURE_TYPE_OBJECT_ATTACH )
			{
				pkWall->MoveFurniture(m_pkArrangingFurniture, kPos, iDir, true);
			}
			else
			{	
				m_pkArrangingFurniture->SetParentFurniture(GetFurniture(kHomeItemInfo.ParentGuid()));
				m_pkArrangingFurniture->SetParentIndex(kHomeItemInfo.LinkIdx());
			}
			SetArrangingFurniture(true);
		}
		else
		{
			int iItemNumber = kOutItem.ItemNo();
			NiPoint3 kItemPos(kHomeItemInfo.Pos().x, kHomeItemInfo.Pos().y, kHomeItemInfo.Pos().z);
			int iDir = kHomeItemInfo.Dir() / ROTATION_ANGLE2;

			pkFurniture = AddFurniture(iItemNumber, kItemPos, iDir, kOutItem.Guid());
			if( pkFurniture )
			{
				PgWall::WallType eWallType = GetWallTypetoFurniture(pkFurniture->GetFurnitureType());
				if( PgWall::WALL_TYPE_NONE == eWallType )
				{
					return;
				}
				PgWall* pkWall = GetWall(eWallType);
				if( NULL == pkWall )
				{
					return;
				}

				m_pkArrangingFurniture = pkFurniture;
				UnlocateArrangingFurniture(m_pkArrangingFurniture);
				if( m_pkArrangingFurniture->GetFurnitureType() != FURNITURE_TYPE_OBJECT_ATTACH )
				{
					pkWall->MoveFurniture(m_pkArrangingFurniture, kPos, iDir, true);
				}
				else
				{	
					m_pkArrangingFurniture->SetParentFurniture(GetFurniture(kHomeItemInfo.ParentGuid()));
					m_pkArrangingFurniture->SetParentIndex(kHomeItemInfo.LinkIdx());
				}
				SetArrangingFurniture(true);
			}
		}
	}

	lwHomeRenew::OnMyhomeBuffDisplay();
}

PgWall::WallType PgHome::GetWallTypetoFurniture(EFurnitureType eFurnitureType)
{
	PgWall::WallType eWallType = PgWall::WALL_TYPE_NONE;

	switch( eFurnitureType )
	{
	case FURNITURE_TYPE_OBJECT:
	case FURNITURE_TYPE_OBJECT_ATTACH:
	case FURNITURE_TYPE_FLOOR_ATTACH:
		eWallType = PgWall::WALL_TYPE_FLOOR;
		break;	

	case FURNITURE_TYPE_WALL_ATTACH:
		eWallType = PgWall::WALL_TYPE_WALL;
		break;
	
	case FURNITURE_TYPE_FLOOR:
	case FURNITURE_TYPE_WALL:
	case FURNITURE_TYPE_ETC:
		PG_ASSERT_LOG(0);
		break;
	}

	return eWallType;
}

void PgHome::RemoveFurniturebyServer(BM::GUID const& kGuid, int iItemNo)
{
	if( IsMaidItem(iItemNo) )
	{
		RemoveMaidNpc(iItemNo);
	}

	for(WallContainer::iterator itr = m_kWallContainer.begin(); itr != m_kWallContainer.end(); ++itr)
	{
		PgWall* pkWall = *itr;
		if( pkWall )
		{
			pkWall->RemoveFurniture(kGuid);
		}
	}

	FurnitureContainer::iterator itor = m_kHomeFurnitures.begin();
	while (itor != m_kHomeFurnitures.end())
	{
		PgFurniture* pkFurniture = *(itor);
		if( pkFurniture )
		{
			if( kGuid == pkFurniture->GetGuid() )
			{
				pkFurniture->Release();
				PgFurniture::DeleteFurniture(pkFurniture);
				return;
			}
		}
		++itor;
	}

	lwHomeRenew::OnMyhomeBuffDisplay();
}

bool PgHome::IsExistMaidNpc()
{
	if( !GetHomeUnit() )
	{
		return false;
	}
	if( !g_kPilotMan.FindPilot(GetHomeUnit()->GetID()) )
	{
		return false;
	}
	PgInventory* pkInv = GetHomeUnit()->GetInven();
	if( !pkInv )
	{
		return false;
	}

	PgBase_Item kOutItem;
	const int iMaxInv = pkInv->GetMaxIDX(IT_HOME);
	for( int ii = 0; ii < iMaxInv; ++ii )
	{
		PgBase_Item kItem;
		SItemPos kPos(IT_HOME, ii);
		if(S_OK != pkInv->GetItem(kPos, kItem))
		{
			continue;
		}

		bool bMaid = IsMaidItem(kItem.ItemNo());
		if( bMaid )
		{
			return true;
		}
	}

	return false;
}

bool PgHome::AddFurniturebyInven(int iItemNo, BM::GUID const& kGuid)
{	
	if( !m_pkArrangingFurniture )
	{
		//if( IsMaidItem(iItemNo) )// 메이드 npc 처리
		//{
		//	XUIMgr.Close(L"FRM_MYHOME_EDIT_MODE");
		//	if( IsExistMaidNpc() )
		//	{
		//		lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201656, true);
		//		return false;
		//	}
		//	SendHomeMaidEquip(iItemNo, kGuid);
		//	XUIMgr.Close(L"Inv");
		//	return false;
		//}

		NiPoint3 kPos(0, -100, 20);
		PgFurniture* pkFurniture = AddFurniture(iItemNo, kPos, 0, kGuid, false);
		if( pkFurniture )
		{
			SetArrangeMode(true);

			UnlocateArrangingFurniture(pkFurniture);
			m_pkArrangingFurniture = pkFurniture;
			m_pkArrangingFurniture->SetFurnitureColor(NiColor::WHITE * 2);

			m_pkPickedFurniture = NULL;
		}
		else
		{
			return false;
		}
	}

	return true;
}

bool PgHome::SetPickFurniture(bool const bOnlyMyActor)
{
	if(!g_pkWorld)
	{
		return false;
	}
	if( NULL != m_pkArrangingFurniture )
	{
		return false;
	}

	if(bOnlyMyActor && !IsMyHome() )
	{
		return false;
	}

	if( !GetHomeUnit() )
	{
		return false;
	}

	NiCamera* pkCamera = m_pkWorld->GetCameraMan()->GetCamera();
	if( !pkCamera )
	{
		return false;
	}

	NiPoint3 kOrgPt;
	NiPoint3 kRayDir;
	POINT2 ptXUIPos = XUIMgr.MousePos();
	pkCamera->WindowPointToRay(ptXUIPos.x, ptXUIPos.y, kOrgPt, kRayDir);

	PgFurniture* pkPickedFurniture = pickFurniture(kOrgPt, kRayDir);

	if( !pkPickedFurniture )
	{
		return false;
	}

	if( IsMaidItem(pkPickedFurniture->GetFurnitureNo()) )
	{
		return false;
	}

	int iState = g_pkWorld->GetHome()->GetHomeUnit()->GetAbil(AT_MYHOME_STATE);
	if( MAS_NOT_BIDDING != iState )
	{
		lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 799789, true);
		return false;
	}

	m_pkPickedFurniture = pkPickedFurniture;

	return true;
}

bool PgHome::SelectArrangingFurnitureByMenu()
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
	m_pkArrangingFurniture->SetFurnitureColor(NiColor::WHITE * 2);

	SetArrangeMode(true);
	return true;
}

bool PgHome::SetMaidNpcMoving(BM::GUID const& kNpcGuid)
{
	if( NULL != m_pkArrangingFurniture )
	{
		return false;
	}

	if( !GetHomeUnit() )
	{
		return false;
	}

	PgPilot* pkPilot = g_kPilotMan.FindPilot(kNpcGuid);
	if( !pkPilot )
	{
		return false;
	}

	int iClassNo = pkPilot->GetAbil(AT_CLASS);
	PgInventory* pkInv = GetHomeUnit()->GetInven();
	if( !pkInv )
	{
		return false;
	}

	GET_DEF(CItemDefMgr, kItemDefMgr);
	const int iMaxInv = pkInv->GetMaxIDX(IT_HOME);
	for( int ii = 0; ii < iMaxInv; ++ii )
	{
		PgBase_Item kItem;
		SItemPos kPos(IT_HOME, ii);
		if(S_OK != pkInv->GetItem(kPos, kItem))
		{
			continue;
		}

		CItemDef const *pkItemDef = kItemDefMgr.GetDef(kItem.ItemNo());
		if( pkItemDef )
		{
			if( pkItemDef->GetAbil(AT_MYHOME_NPC_CLASS_NO) == iClassNo )
			{
				m_pkPickedFurniture = GetFurniture(kItem.Guid());
				if( m_pkPickedFurniture )
				{
					SetArrangeMode(true);
					if( SelectArrangingFurnitureByMenu() )
					{
						SetHideMaidNpc(kItem.ItemNo(), true, NiPoint3(0,0,0));
						m_pkArrangingFurniture->SetAppCulled(false);
						return true;
					}
				}
			}
		}
	}

	return false;
}

bool PgHome::RemoveArrangingFurniture()
{
	if( !m_pkArrangingFurniture )
	{
		return false;
	}

	if( !GetHomeUnit() )
	{
		m_pkArrangingFurniture = NULL;
	}

	BM::GUID kGuid(m_pkArrangingFurniture->GetGuid());
	PgInventory* pInv = GetHomeUnit()->GetInven();
	if( !pInv )
	{
		return false;
	}

	PgBase_Item kItem;
	if( S_OK != pInv->GetItem(kGuid, kItem) )
	{
		return false;
	}

	return true;
}

void PgHome::RemovePickedFurniture()
{
	if( !m_pkPickedFurniture )
	{
		return;
	}

	bool bChild = false;
	m_kRemoveFurniture.clear();
	if( m_pkPickedFurniture->GetFurnitureType() == FURNITURE_TYPE_OBJECT )
	{
		PgWall* pkWall = GetWall(PgWall::WALL_TYPE_FLOOR);
		if( pkWall )
		{
			FurnitureContainer contFur;
			if( pkWall->GetChildList(m_pkPickedFurniture, contFur) )
			{
				FurnitureContainer::iterator itr = contFur.begin();
				for(; itr != contFur.end(); ++itr)
				{
					PgFurniture* pkChild = *itr;
					if( pkChild )
					{
						if( !bChild )// 하나는 먼저 보낸다.
						{
							HomeUnEquipRequest(pkChild);
							bChild = true;
						}
						else
						{
							m_kRemoveFurniture.push_back(pkChild);
						}
					}
				}
			}
		}
	}

	if( bChild )
	{
		m_kRemoveFurniture.push_back(m_pkPickedFurniture);
	}
	else
	{
		HomeUnEquipRequest(m_pkPickedFurniture);
	}
}

bool PgHome::IsMyHome()
{
	return g_kPilotMan.IsMyPlayer(GetOwnerGuid());
}

void PgHome::CancelLocateArrangingFurniture()
{
	if( m_pkArrangingFurniture )
	{
		if( !GetHomeUnit() )
		{
			ClearArrangingFurniture();
		}

		PgInventory* pInv = GetHomeUnit()->GetInven();
		if( !pInv )
		{
			ClearArrangingFurniture();
		}
		PgBase_Item kOutItem;
		if(S_OK == pInv->GetItem(m_pkArrangingFurniture->GetGuid(), kOutItem ) )
		{
			AddFurniturebyGuid(m_pkArrangingFurniture->GetGuid());
		}
		else
		{
			ClearArrangingFurniture();
		}
	}

	m_pkPickedFurniture = NULL;
	XUIMgr.Close(L"FRM_MYHOME_EDIT_MODE");
}

void PgHome::HideAllCellTiles(bool bHide)
{
	for(WallContainer::iterator itr = m_kWallContainer.begin(); itr != m_kWallContainer.end(); ++itr)
	{
		PgWall* pkWall = *itr;
		if( pkWall )
		{
			pkWall->HideAllCellTiles(bHide);
		}
	}
}

void PgHome::ReleaseWallTile()
{
	if( m_pkFloorTile )
	{
		PgFurniture::DeleteFurniture(m_pkFloorTile);
		m_pkFloorTile = NULL;
	}

	if( m_pkWallTile )
	{
		PgFurniture::DeleteFurniture(m_pkWallTile);
		m_pkWallTile = NULL;
	}

	if( m_pkWallCornerTile )
	{
		PgFurniture::DeleteFurniture(m_pkWallCornerTile);
		m_pkWallCornerTile = NULL;
	}
}

void PgHome::SetAllWallTile()
{
	if( !GetHomeUnit() )
	{
		return;
	}

	PLAYER_ABIL kInfo;
	GetHomeUnit()->GetPlayerAbil(kInfo);

	for(int i = 0; i<DefaultTileCount; ++i)
	{
		int iItemNo = DefaultTileArray[i];
		switch(TilePosArray[i])
		{
		case EQUIP_POS_ROOM_WALL:
			{
				if( kInfo.iPants)
				{
					iItemNo = kInfo.iPants;
				}
			}break;
		case EQUIP_POS_ROOM_FLOOR:
			{
				if( kInfo.iShoes)
				{
					iItemNo = kInfo.iShoes;
				}
			}break;
		}

		if( iItemNo )
		{
			SetWallTile( iItemNo );
		}
	}

	if( kInfo.iHairStyle )
	{
		GET_DEF(CItemDefMgr, kItemDefMgr);
		CItemDef const *pkItemDef = kItemDefMgr.GetDef(kInfo.iHairStyle);
		if( pkItemDef )
		{
			int ix = pkItemDef->GetAbil(AT_MYHOME_ROOM_X);
			int iy = pkItemDef->GetAbil(AT_MYHOME_ROOM_Y);
			if( ix && iy )
			{
				SetRoomSize(POINT3I(ix, iy, 160));
			}

			SetGrade(pkItemDef->GetAbil(AT_USE_ITEM_CUSTOM_VALUE_3));
			SetMyhomeCameraPos(GetGrade());
		}
	}
}

bool PgHome::SetWallTile(int iItemNo)
{
	if( !iItemNo || !m_pkWorld)
	{
		return false;
	}
	PgFurniture* pkFurniture = NULL;
	pkFurniture = PgFurniture::GetFurnitureFromDef(iItemNo);
	if (pkFurniture == NULL)
	{
		PgError1("[PgHome] AddFurniture, can't create %d Item\n", iItemNo);
		return false;
	}

	NiAVObject* pMeshRoot = pkFurniture->GetNIFRoot();
	CItemDef const* pkDef = pkFurniture->GetItemDef();
	if( !pMeshRoot && !pkDef )
	{
		return false;
	}

	int iType = pkDef->GetAbil(AT_FURNITURE_TYPE);
	if( FURNITURE_TYPE_FLOOR == iType)
	{
		if( m_pkFloorTile )
		{
			PgFurniture::DeleteFurniture(m_pkFloorTile);
			m_pkFloorTile = NULL;
		}
		m_pkFloorTile = pkFurniture;
		//m_pkFloorTile->InitPhysX(m_pkWorld->GetPhysXScene(), OGT_FURNITURE);
	}
	else if( FURNITURE_TYPE_WALL == iType )
	{
		if( m_pkWallTile )
		{
			PgFurniture::DeleteFurniture(m_pkWallTile);
			m_pkWallTile = NULL;
		}
		if( m_pkWallCornerTile )
		{
			PgFurniture::DeleteFurniture(m_pkWallCornerTile);
			m_pkWallCornerTile = NULL;
		}
		m_pkWallTile = pkFurniture;
		//m_pkWallTile->InitPhysX(m_pkWorld->GetPhysXScene(), OGT_FURNITURE);

		int iCornerNo = pkDef->GetAbil(AT_HOME_WALL_SUB_NO);
		if( !iCornerNo )
		{
			iCornerNo = CORNER_NO;
		}
		//m_pkWallCornerTile = PgFurniture::GetFurnitureFromDef(iCornerNo);
		//if( !m_pkWallCornerTile )
		//{
		//	return false;
		//}
		//NiQuaternion kRot = NiQuaternion::IDENTITY;
		//kRot.FromAngleAxis(NI_PI, NiPoint3::UNIT_Z);
		//m_pkWallCornerTile->SetRotate(kRot);
		//m_pkWallCornerTile->SetTranslate(NiPoint3(-WALL_CELL_UNIT_SIZE, -WALL_CELL_UNIT_SIZE, 0));
	}

	return true;
}

void PgHome::Send_PT_C_M_REQ_HOME_USE_HOME_STYLEITEM(SItemPos kItemPos, DWORD dwClientTime, BM::GUID const& kHomeGuid)
{
	BM::Stream kPacket;
	kPacket.Push(PT_C_M_REQ_HOME_USE_HOME_STYLEITEM);
	kPacket.Push(kItemPos);
	kPacket.Push(dwClientTime);
	kPacket.Push(kHomeGuid);
	NETWORK_SEND(kPacket);
}

bool PgHome::HomeItemUseStyleItem(SItemPos const& rkItemPos)
{
	if( !GetHomeUnit() )
	{
		return false;
	}

	Send_PT_C_M_REQ_HOME_USE_HOME_STYLEITEM(rkItemPos, lwGetServerElapsedTime32(), GetHomeUnit()->GetID() );
	return true;
}

BM::GUID PgHome::GetHomeGuid()
{
	if( GetHomeUnit() )
	{
		return GetHomeUnit()->GetID();
	}

	return BM::GUID::NullData();
}

bool PgHome::PickCell(PgWall* pkWall, NiPoint3& kCellPos)
{
	if( !m_pkWorld || !pkWall)
	{
		return false;
	}
	POINT2 ptXUIPos = XUIMgr.MousePos();

	NiPoint3 kOrgPt = NiPoint3::ZERO;
	NiPoint3 kRayDir = NiPoint3::ZERO;
	kCellPos = NiPoint3::ZERO;
	m_pkWorld->m_kCameraMan.GetCamera()->WindowPointToRay(ptXUIPos.x, ptXUIPos.y, kOrgPt, kRayDir);

	NiAVObject* pkCell = pkWall->PickCell(kOrgPt, kRayDir, kCellPos);
	if (pkCell == NULL)
	{
		return false;
	}

	return true;		
}

PgFurniture* PgHome::GetFurniture(BM::GUID const& kGuid)
{
	WallContainer::iterator itr = m_kWallContainer.begin();
	while( itr != m_kWallContainer.end() )
	{
		PgWall* pkWall = *itr;
		if( pkWall )
		{
			PgFurniture* pkFurniture = pkWall->GetFurniture(kGuid);
			if( pkFurniture )
			{
				return pkFurniture;
			}
		}
		++itr;
	}

	FurnitureContainer::iterator kItor = m_kHomeFurnitures.begin();
	while(m_kHomeFurnitures.end() != kItor)
	{
		PgFurniture* pkFurniture = (*kItor);
		if(pkFurniture
			&& pkFurniture->GetGuid() == kGuid
			)
		{
			return pkFurniture;
		}
		++kItor;
	}
	
	return NULL;
}

void PgHome::HomeItemRequest()
{
	if( !m_pkArrangingFurniture )
	{
		return;
	}

	PgWall::WallType eWallType = GetWallTypetoFurniture(m_pkArrangingFurniture->GetFurnitureType());
	if( PgWall::WALL_TYPE_NONE == eWallType )
	{
		return;
	}
	PgWall* pkWall = GetWall(eWallType);
	if( NULL == pkWall )
	{
		return;
	}
	NiPoint3 kCellPos = NiPoint3::ZERO;
	if( !PickCell(pkWall, kCellPos) )
	{
		return;
	}

	if( m_pkArrangingFurniture->GetFurnitureType() == FURNITURE_TYPE_OBJECT_ATTACH )
	{
		int iDummyPos = 0;
		PgFurniture* pkParent = pkWall->CheckParentFurniture(m_pkArrangingFurniture, kCellPos, iDummyPos);
		if( pkParent )
		{
			if( !m_pkArrangingFurniture->GetParentFurniture() )
			{
				return;
			}
			if ( !m_pkPickedFurniture )
			{
				HomeEquipRequest(m_pkArrangingFurniture, m_pkArrangingFurniture->GetParentFurniture()->GetGuid(), m_pkArrangingFurniture->GetParentIndex());
			}
			else
			{
				HomeItemModifyRequest(m_pkArrangingFurniture, m_pkArrangingFurniture->GetParentFurniture()->GetGuid(), m_pkArrangingFurniture->GetParentIndex());
			}
		}
	}
	else
	{
		if( pkWall->CheckCellsBound(m_pkArrangingFurniture) && pkWall->CheckOtherFurniture(m_pkArrangingFurniture) )
		{
			if ( !m_pkPickedFurniture )
			{
				HomeEquipRequest(m_pkArrangingFurniture);
			}
			else
			{
				HomeItemModifyRequest(m_pkArrangingFurniture);
			}
		}
	}
}

bool PgHome::IsHaveSetPropPlace()
{
	PgWall* pkWall = GetWall(PgWall::WALL_TYPE_FLOOR);
	if( !pkWall )
	{
		return false;
	}

	return pkWall->IsHaveSetPropPlace();
}

void PgHome::SetMyhomeCameraPos(int iPos)
{
	if( !m_pkWorld )
	{
		return;
	}

	if( iPos < 2 || iPos > 4 )
	{
		return;
	}
	
	NiCamera* pkCamera = m_pkWorld->GetMinimapCamera();
	if( !pkCamera )
	{
		return;
	}

	static NiPoint3 kCameraPos = pkCamera->GetTranslate();
	float fAdd = 0;
	switch( iPos )
	{
	case 2:
		{
			fAdd = -40;
		}break;
	case 3:
		{
			fAdd = -80;
		}break;
	case 4:
		{
			fAdd = -160;
		}break;
	}

	NiPoint3 kPos = kCameraPos;
	kPos.x += fAdd;
	kPos.y += fAdd;

	pkCamera->SetTranslate(kPos);
	pkCamera->Update(0.0f);
}

bool PgHome::IsHavePostbox()
{
	if( !GetHomeUnit() )
	{
		return false;
	}

	PgInventory* pkInv = GetHomeUnit()->GetInven();
	if(!pkInv)
	{
		return false;
	}

	CONT_HAVE_ITEM_DATA kCont;
	if(S_OK!=pkInv->GetItems(IT_HOME, kCont))
	{
		return false;
	}

	CONT_HAVE_ITEM_DATA::const_iterator item_it = kCont.begin();
	while(kCont.end()!=item_it)
	{
		PgBase_Item const& rkBaseItem = (*item_it).second;
		if(!rkBaseItem.IsEmpty())
		{
			GET_DEF(CItemDefMgr, kItemDefMgr);
			CItemDef const *pkItemDef = kItemDefMgr.GetDef(rkBaseItem.ItemNo());
			if( pkItemDef )
			{
				if( UICT_HOME_POSTBOX == pkItemDef->GetAbil(AT_USE_ITEM_CUSTOM_TYPE) )
				{
					return true;
				}
			}
		}
		++item_it;
	}
	return false;
}

bool PgHome::IsHaveSafebox()
{
	if( !GetHomeUnit() )
	{
		return false;
	}

	PgInventory* pkInv = GetHomeUnit()->GetInven();
	if(!pkInv)
	{
		return false;
	}

	CONT_HAVE_ITEM_DATA kCont;
	if(S_OK!=pkInv->GetItems(IT_HOME, kCont))
	{
		return false;
	}

	CONT_HAVE_ITEM_DATA::const_iterator item_it = kCont.begin();
	while(kCont.end()!=item_it)
	{
		PgBase_Item const& rkBaseItem = (*item_it).second;
		if(!rkBaseItem.IsEmpty())
		{
			GET_DEF(CItemDefMgr, kItemDefMgr);
			CItemDef const *pkItemDef = kItemDefMgr.GetDef(rkBaseItem.ItemNo());
			if( pkItemDef )
			{
				if( UICT_HOME_SAFEBOX == pkItemDef->GetAbil(AT_USE_ITEM_CUSTOM_TYPE) )
				{
					return true;
				}
			}
		}
		++item_it;
	}
	return false;
}

bool PgHome::IsUseAbleTimeofPickFurniture()
{
	if( NULL == m_pkPickedFurniture || !GetHomeUnit() )
	{
		return false;
	}

	PgInventory* pkInv = GetHomeUnit()->GetInven();
	if( !pkInv )
	{
		return false;
	}

	PgBase_Item kItem;
	if( S_OK != pkInv->GetItem(m_pkPickedFurniture->GetGuid(), kItem) )
	{
		return false;
	}

	if( kItem.GetUseAbleTime() < 0 )
	{
		return false;
	}

	return true;
}

void PgHome::UnsetAllFurniture()
{
	if(!GetHomeUnit())
	{
		return;
	}

	if( !IsMyHome() )
	{
		return;
	}

	int iState = GetHomeUnit()->GetAbil(AT_MYHOME_STATE);
	if( MAS_NOT_BIDDING != iState )
	{
		lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 799789, true);
		return;
	}

	PgInventory* pkInv = GetHomeUnit()->GetInven();
	if( !pkInv )
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

		SInvenFurniture kInven;
		kInven.kGuid = GetHomeUnit()->GetID();
		kInven.kPos = kPos;
		m_kUnsetFurniture.push(kInven);
	}

	if( !m_kUnsetFurniture.empty() )
	{
		RemoveAllFurniture();
		ReleaseCellsOccupancy();
	}
	RemoveFurnitreProcess();
}

void PgHome::RemoveFurnitreProcess()
{
	if( m_kRemoveFurniture.size() )
	{
		FurnitureContainer::iterator fur_it = m_kRemoveFurniture.begin();
		if( fur_it != m_kRemoveFurniture.end() )
		{
			if( *fur_it )
			{
				HomeUnEquipRequest(*fur_it);
				m_kRemoveFurniture.erase(fur_it);
			}
		}
	}

	if( !m_kUnsetFurniture.empty() )
	{
		SInvenFurniture kInven = m_kUnsetFurniture.front();
		Send_PT_C_M_REQ_HOME_UNEQUIP(kInven.kGuid, kInven.kPos);
		m_kUnsetFurniture.pop();
	}
}

void PgHome::ReleaseCellsOccupancy()
{
	for(WallContainer::iterator itr = m_kWallContainer.begin(); itr != m_kWallContainer.end(); ++itr)
	{
		if ((*itr) != NULL)
		{
			(*itr)->ReleaseCellsOccupancy();
		}
	}
}

bool PgHome::IsMaidItem(int iItemNo)
{
	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const *pkItemDef = kItemDefMgr.GetDef(iItemNo);
	if( pkItemDef )
	{
		int iType = pkItemDef->GetAbil(AT_USE_ITEM_CUSTOM_TYPE);
		if(  UICT_HOME_SIDEJOB_NPC == iType )
		{
			return true;
		}
	}

	return false;
}

bool PgHome::SendHomeMaidEquip(int iItemNo, BM::GUID const& kGuid)
{
	if( !GetHomeUnit())
	{
		return false;
	}

	if( !IsMyHome() )
	{
		return false;
	}

	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const *pkItemDef = kItemDefMgr.GetDef(iItemNo);
	if( !pkItemDef )
	{
		return false;
	}

	int iClassNo = pkItemDef->GetAbil(AT_MYHOME_NPC_CLASS_NO);
	PgActor* pkActor = g_kPilotMan.FindActorByClassNo(iClassNo);
	if( !pkActor )
	{
		return false;
	}
	// guid
	BM::GUID kHomeGuid(GetHomeUnit()->GetID());

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
	if( S_OK != pkInv->GetItem(kGuid, kOutItem, kOutPos) )
	{
		return false;
	}

	// 배치 위치
	NiPoint3 kFurniturePos(pkActor->GetWorldTransform().m_Translate);
	POINT3 kPos(kFurniturePos.x, kFurniturePos.y, kFurniturePos.z);

	Send_PT_C_M_REQ_HOME_EQUIP(kHomeGuid, kOutPos, kPos, 0, BM::GUID::NullData(), 0);

	return true;
}

bool PgHome::SendHomeMaidUnEquip(BM::GUID const& kNpcGuid)
{
	if( !GetHomeUnit() )
	{
		return false;
	}

	if( !IsMyHome() )
	{
		return false;
	}

	PgPilot* pkPilot = g_kPilotMan.FindPilot(kNpcGuid);
	if( !pkPilot )
	{
		return false;
	}

	int iClassNo = pkPilot->GetAbil(AT_CLASS);

	if( !g_kPilotMan.FindPilot(GetHomeUnit()->GetID()) )
	{
		return false;
	}

	BM::GUID kHomeGuid(GetHomeUnit()->GetID());

	PgInventory* pkInv = GetHomeUnit()->GetInven();
	if( !pkInv )
	{
		return false;
	}

	GET_DEF(CItemDefMgr, kItemDefMgr);
	const int iMaxInv = pkInv->GetMaxIDX(IT_HOME);
	for( int ii = 0; ii < iMaxInv; ++ii )
	{
		PgBase_Item kItem;
		SItemPos kPos(IT_HOME, ii);
		if(S_OK != pkInv->GetItem(kPos, kItem))
		{
			continue;
		}

		CItemDef const *pkItemDef = kItemDefMgr.GetDef(kItem.ItemNo());
		if( pkItemDef )
		{
			if( pkItemDef->GetAbil(AT_MYHOME_NPC_CLASS_NO) == iClassNo )
			{
				Send_PT_C_M_REQ_HOME_UNEQUIP(kHomeGuid, kPos);
				return true;
			}
		}
	}
	
	return false;
}

void PgHome::SetHideMaidNpc(int iItemNo, bool bHide, NiPoint3 kPos)
{
	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const *pkItemDef = kItemDefMgr.GetDef(iItemNo);
	if( pkItemDef )
	{
		int iClassNo = pkItemDef->GetAbil(AT_MYHOME_NPC_CLASS_NO);
		PgActor* pkActor = g_kPilotMan.FindActorByClassNo(iClassNo);
		if( pkActor )
		{
			if( !bHide )
			{
				NiPoint3 kTrans(kPos);
				kTrans.z =+ 25;
				pkActor->SetFreeMove(true);
				pkActor->SetPosition(kTrans);
			}
			pkActor->SetHide(bHide);
		}
	}
}

void PgHome::SetMaidColor(int iItemNo, const NiColor &kColor)
{
	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const *pkItemDef = kItemDefMgr.GetDef(iItemNo);
	if( pkItemDef )
	{
		int iClassNo = pkItemDef->GetAbil(AT_MYHOME_NPC_CLASS_NO);
		PgActor* pkActor = g_kPilotMan.FindActorByClassNo(iClassNo);
		if( pkActor )
		{
			pkActor->SetColor(kColor);
		}
	}
}

void PgHome::AddMaidNpc(int iItemNo, NiPoint3 kPos)
{
	SetHideMaidNpc(iItemNo, false, kPos);
}

void PgHome::RemoveMaidNpc(int iItemNo)
{
	SetHideMaidNpc(iItemNo, true, NiPoint3(0,0,0));
}

bool PgHome::IsUseAbleTimeMaidNPC(BM::GUID const& kNpcGuid)
{
	if( !GetHomeUnit() )
	{
		return false;
	}

	PgPilot* pkPilot = g_kPilotMan.FindPilot(kNpcGuid);
	if( !pkPilot )
	{
		return false;
	}

	int iClassNo = pkPilot->GetAbil(AT_CLASS);
	PgInventory* pkInv = GetHomeUnit()->GetInven();
	if( !pkInv )
	{
		return false;
	}

	GET_DEF(CItemDefMgr, kItemDefMgr);
	const int iMaxInv = pkInv->GetMaxIDX(IT_HOME);
	for( int ii = 0; ii < iMaxInv; ++ii )
	{
		PgBase_Item kItem;
		SItemPos kPos(IT_HOME, ii);
		if(S_OK != pkInv->GetItem(kPos, kItem))
		{
			continue;
		}

		CItemDef const *pkItemDef = kItemDefMgr.GetDef(kItem.ItemNo());
		if( pkItemDef )
		{
			if( pkItemDef->GetAbil(AT_MYHOME_NPC_CLASS_NO) == iClassNo )
			{
				if( kItem.GetUseAbleTime() >= 0 )// 기간이 지나지 않았으면
				{
					return true;
				}
			}
		}
	}
	
	return false;
}