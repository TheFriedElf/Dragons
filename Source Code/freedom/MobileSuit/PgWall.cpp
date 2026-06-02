#include "StdAfx.h"
#include "PgWall.h"
#include "PgInterpolator.h"
#include "PgRenderer.h"
#include "PgNifMan.h"
#include "PgFurniture.h"

#include "NewWare/Scene/ApplyTraversal.h"


#define CELL_MESH_NAME	"wallCell"
#define WALL_LEFT	1
#define WALL_RIGHT	2

int const DOOR_NO = 70000820;
char const* FURNITURE_DUMMY = "Deco_Dummy";

PgWall::PgWall()
{
	m_kLeftTop = NiPoint3::ZERO;
	m_kRightBottom = NiPoint3::ZERO;
	m_kNormal = NiPoint3::ZERO;
	m_kCellObject = NULL;
	m_kDefaultCellObject = NULL;
	m_kWallRoot = NULL;
	m_eWallType = WALL_TYPE_NONE;
	m_kWallAttachments.clear();
	m_kWalls.clear();
	m_pkArrCells = NULL;
	m_iCellsBound1Count = 0;
	m_iCellsBound2Count = 0;
	m_pkCellsBound1 = NULL;
	m_pkCellsBound2 = NULL;
	m_iPhysxNum = 0;
	m_pkDoorTile = NULL;
}

PgWall::~PgWall()
{
	Terminate();
}

void PgWall::Terminate()
{
	m_kLeftTop = NiPoint3::ZERO;
	m_kRightBottom = NiPoint3::ZERO;
	m_kNormal = NiPoint3::ZERO;
	m_kCellObject = NULL;
	m_kDefaultCellObject = NULL;
	//m_kWallAttachments.clear();
	//m_kWalls.clear();
	if( m_pkDoorTile )
	{
		if (m_kWallRoot)
		{
			m_kWallRoot->DetachChild(m_pkDoorTile->GetNIFRoot());
		}
		PgFurniture::DeleteFurniture(m_pkDoorTile);
		m_pkDoorTile = NULL;
	}
	RemoveAllFurniture();
	RemovePhysxWalls();
	removeAllArrow();
	ClearAllCells();
	m_kWallRoot = NULL;
}

void PgWall::UpdateBound()
{
	if (m_kRightBottom == NiPoint3::ZERO && m_kLeftTop == NiPoint3::ZERO)
		return;

	// 크기를 구한다.
	NiPoint3 kExtent = m_kRightBottom - m_kLeftTop;
	kExtent.x = NiAbs(kExtent.x);
	kExtent.y = NiAbs(kExtent.y);
	kExtent.z = NiAbs(kExtent.z);

	// 노멀을 구한다.
	// 변위 가장 작은 축이 노멀이 된다.
	m_kNormal.x = 1.0f / NiMax(kExtent.x, 0.001f);
	m_kNormal.y = 1.0f / NiMax(kExtent.y, 0.001f);
	m_kNormal.z = 1.0f / NiMax(kExtent.z, 0.001f);
	m_kNormal.Unitize();

	// 크기가 없는 축(노멀)쪽으로 크기를 준다.
	kExtent += m_kNormal * 3.0f;

	// 중앙을 구한다.
	NiPoint3 kCenter = PgInterpolator::Lerp(m_kLeftTop, m_kRightBottom, 0.5f);

	// BV를 생성한다.
	m_kBound = NiBoxBV(kExtent, kCenter);
}

bool PgWall::IsInside(NiPoint3 const &rkPt)
{
	return m_kBound.BoxSphereTestIntersect(0.0f, 
		m_kBound, NiPoint3::ZERO, 
		NiSphereBV(1.0f, rkPt), NiPoint3::ZERO);
}

void PgWall::SetWallInfo(WallType eWallType, NiPoint3& rkLeftTop, NiPoint3& rkRightBottom)
{
	if (eWallType == WALL_TYPE_NONE)
		return;

	m_eWallType = eWallType;
	m_kLeftTop = rkLeftTop;
	m_kRightBottom = rkRightBottom;

	UpdateBound();
}

void PgWall::SetCellObject(NiAVObjectPtr spCellObject, bool bDefault)
{
	PG_ASSERT_LOG(spCellObject);
	if (spCellObject == NULL)
		return;

	m_kCellObject = spCellObject;
	if (bDefault)
		m_kDefaultCellObject = spCellObject;

	m_kCellObject->SetName(CELL_MESH_NAME);
}

void PgWall::SetCellObject(PgFurniture* pCellObject, bool bDefault)
{
	PG_ASSERT_LOG(pCellObject);
	if (pCellObject == NULL)
		return;

	m_kCellObject = pCellObject;
	if (bDefault)
		m_kDefaultCellObject = pCellObject;

	m_kCellObject->SetName(CELL_MESH_NAME);
}

void PgWall::AddCellBoundUnit(int iPos, NiPoint3 const& kOffset, NiQuaternion const& kRot, int iKind)
{
	NiNodePtr kCellUnit = NiNew NiNode();
	if( kCellUnit )
	{
		if( 1 == iKind )
		{
			m_pkCellsBound1[iPos].m_spCellTile = kCellUnit;
		}
		else
		{
			if( m_pkCellsBound2 )
			{
				m_pkCellsBound2[iPos].m_spCellTile = kCellUnit;
			}
		}
		kCellUnit->SetTranslate(kOffset);
		kCellUnit->SetRotate(kRot);
		m_kWallRoot->AttachChild(kCellUnit, true);
	}
}

void PgWall::SetCellsBound(int iColCellNum, int iColCellNum2, float fZOffset)
{
	switch(GetWallType())
	{
	case WALL_TYPE_FLOOR:
		{
			int ii;
			NiPoint3 kOffset;
			NiQuaternion kRot;

			kRot.FromAngleAxis(NI_HALF_PI, NiPoint3::UNIT_Z);
			kOffset.z = fZOffset;

			int iCount = 0;
			
			// step1 아래
			kOffset.y = (-WALL_CELL_UNIT_SIZE / 2);
			for( ii = 0; ii < m_kTotalCellCount.x; ++ii )
			{
				kOffset.x = static_cast<float>((WALL_CELL_UNIT_SIZE / 2) + (WALL_CELL_UNIT_SIZE * ii));
				AddCellBoundUnit(iCount, kOffset, kRot);
				++iCount;
			}
			// step2 위
			kOffset.y = (-WALL_CELL_UNIT_SIZE / 2) + ((m_kTotalCellCount.y + 1) * WALL_CELL_UNIT_SIZE);
			for( ii = 0; ii < m_kTotalCellCount.x ; ++ii )
			{
				kOffset.x = static_cast<float>((WALL_CELL_UNIT_SIZE / 2) + (WALL_CELL_UNIT_SIZE * ii));
				AddCellBoundUnit(iCount, kOffset, kRot);
				++iCount;
			}
			// step3 좌
			kOffset.x = (-WALL_CELL_UNIT_SIZE / 2);
			for( ii = 0; ii < m_kTotalCellCount.y; ++ii )
			{
				kOffset.y = static_cast<float>((WALL_CELL_UNIT_SIZE / 2) + (WALL_CELL_UNIT_SIZE * ii));
				AddCellBoundUnit(iCount, kOffset, kRot);
				++iCount;
			}
			// step4 우
			kOffset.x = (-WALL_CELL_UNIT_SIZE / 2) + ((m_kTotalCellCount.x + 1) * WALL_CELL_UNIT_SIZE);
			for( ii = 0; ii < m_kTotalCellCount.y; ++ii )
			{
				kOffset.y = static_cast<float>((WALL_CELL_UNIT_SIZE / 2) + (WALL_CELL_UNIT_SIZE * ii));
				AddCellBoundUnit(iCount, kOffset, kRot);
				++iCount;
			}
		}break;
	case WALL_TYPE_WALL:
		{
			int ii;
			NiPoint3 kOffset = NiPoint3::ZERO;
			NiQuaternion kRot;

			kRot.FromAngleAxis(NI_HALF_PI, NiPoint3::UNIT_Z);

			int iCount = 0;
			
			////////////////////// y,z
			// step1 아래
			kOffset.y = (-WALL_CELL_UNIT_SIZE / 2);
			for( ii = 0; ii < m_kTotalCellCount.x ; ++ii )
			{
				kOffset.z = static_cast<float>((WALL_CELL_UNIT_SIZE / 2) + (WALL_CELL_UNIT_SIZE * ii) + fZOffset);
				AddCellBoundUnit(iCount, kOffset, kRot);
				++iCount;
			}
			// step2 위
			kOffset.y = static_cast<float>((-WALL_CELL_UNIT_SIZE / 2) + ((iColCellNum + 1) * WALL_CELL_UNIT_SIZE));
			for( ii = 0; ii < m_kTotalCellCount.x ; ++ii )
			{
				kOffset.z = static_cast<float>((WALL_CELL_UNIT_SIZE / 2) + (WALL_CELL_UNIT_SIZE * ii) + fZOffset);
				AddCellBoundUnit(iCount, kOffset, kRot);
				++iCount;
			}
			// step3 좌
			kOffset.z = (WALL_CELL_UNIT_SIZE / 2) + fZOffset;
			for( ii = 0; ii < iColCellNum; ++ii )
			{
				kOffset.y = static_cast<float>((WALL_CELL_UNIT_SIZE / 2) + (WALL_CELL_UNIT_SIZE * ii));
				AddCellBoundUnit(iCount, kOffset, kRot);
				++iCount;
			}
			// step4 우
			kOffset.z = (-WALL_CELL_UNIT_SIZE / 2) + ((m_kTotalCellCount.x + 1) * WALL_CELL_UNIT_SIZE) + fZOffset;
			for( ii = 0; ii < iColCellNum; ++ii )
			{
				kOffset.y = static_cast<float>((WALL_CELL_UNIT_SIZE / 2) + (WALL_CELL_UNIT_SIZE * ii));
				AddCellBoundUnit(iCount, kOffset, kRot);
				++iCount;
			}
			///////////// x,z
			iCount = 0;
			// step1 아래
			kOffset.x = (-WALL_CELL_UNIT_SIZE / 2);
			for( ii = 0; ii < m_kTotalCellCount.x ; ++ii )
			{
				kOffset.z = (WALL_CELL_UNIT_SIZE / 2) + (WALL_CELL_UNIT_SIZE * ii) + fZOffset;
				AddCellBoundUnit(iCount, kOffset, kRot, 2);
				++iCount;
			}
			// step2 위
			kOffset.x = static_cast<float>((-WALL_CELL_UNIT_SIZE / 2) + ((iColCellNum2 + 1) * WALL_CELL_UNIT_SIZE));
			for( ii = 0; ii < m_kTotalCellCount.x ; ++ii )
			{
				kOffset.z = (WALL_CELL_UNIT_SIZE / 2) + (WALL_CELL_UNIT_SIZE * ii) + fZOffset;
				AddCellBoundUnit(iCount, kOffset, kRot, 2);
				++iCount;
			}
			// step3 좌
			kOffset.z = (WALL_CELL_UNIT_SIZE / 2) + fZOffset;
			for( ii = 0; ii < iColCellNum2; ++ii )
			{
				kOffset.x = static_cast<float>((WALL_CELL_UNIT_SIZE / 2) + (WALL_CELL_UNIT_SIZE * ii));
				AddCellBoundUnit(iCount, kOffset, kRot, 2);
				++iCount;
			}
			// step4 우
			kOffset.z = (-WALL_CELL_UNIT_SIZE / 2) + ((m_kTotalCellCount.x + 1) * WALL_CELL_UNIT_SIZE) + fZOffset;
			for( ii = 0; ii < iColCellNum2; ++ii )
			{
				kOffset.x = static_cast<float>((WALL_CELL_UNIT_SIZE / 2) + (WALL_CELL_UNIT_SIZE * ii));
				AddCellBoundUnit(iCount, kOffset, kRot, 2);
				++iCount;
			}
		}break;
	}
}

void PgWall::SetGardenCellsBound( float fZOffset )
{
	int ii;
	NiPoint3 kOffset;
	NiQuaternion kRot;

	kRot.FromAngleAxis(0, NiPoint3::UNIT_Z);
	kOffset.z = fZOffset;

	int iCount = 0;
	
	// step1 위
	kOffset.y = (WALL_CELL_UNIT_SIZE / 2);
	for( ii = 0; ii < m_kTotalCellCount.x; ++ii )
	{
		kOffset.x = ((-m_kTotalCellCount.x / 2) * WALL_CELL_UNIT_SIZE) + (WALL_CELL_UNIT_SIZE / 2) + ( WALL_CELL_UNIT_SIZE * ii );
		AddCellBoundUnit(iCount, kOffset, kRot);
		++iCount;
	}
	// step2 아래
	
	kOffset.y = ((-m_kTotalCellCount.y) * WALL_CELL_UNIT_SIZE) - (WALL_CELL_UNIT_SIZE / 2);
	for( ii = 0; ii < m_kTotalCellCount.x ; ++ii )
	{
		kOffset.x = ((-m_kTotalCellCount.x / 2) * WALL_CELL_UNIT_SIZE) + (WALL_CELL_UNIT_SIZE / 2) + ( WALL_CELL_UNIT_SIZE * ii );
		AddCellBoundUnit(iCount, kOffset, kRot);
		++iCount;
	}
	// step3 좌
	kOffset.x = ((-m_kTotalCellCount.x / 2) * WALL_CELL_UNIT_SIZE) - (WALL_CELL_UNIT_SIZE / 2);
	for( ii = 0; ii < m_kTotalCellCount.y; ++ii )
	{
		kOffset.y = static_cast<float>((-WALL_CELL_UNIT_SIZE / 2) - (WALL_CELL_UNIT_SIZE * ii));
		AddCellBoundUnit(iCount, kOffset, kRot);
		++iCount;
	}
	// step4 우
	kOffset.x = ((m_kTotalCellCount.x / 2) * WALL_CELL_UNIT_SIZE) + (WALL_CELL_UNIT_SIZE / 2);
	for( ii = 0; ii < m_kTotalCellCount.y; ++ii )
	{
		kOffset.y = static_cast<float>((-WALL_CELL_UNIT_SIZE / 2) - (WALL_CELL_UNIT_SIZE * ii));
		AddCellBoundUnit(iCount, kOffset, kRot);
		++iCount;
	}
}

bool PgWall::BuildWallCells()
{
	if (m_kCellObject == NULL)
		return false;

	if (m_eWallType == WALL_TYPE_NONE || m_eWallType >= MAX_WALL_TYPE)
		return false;

	if (m_kRightBottom == NiPoint3::ZERO && m_kLeftTop == NiPoint3::ZERO)
		return false;

	int iRowCellNum = 0;
	int iColCellNum = 0;
	int iColCellNum2 = 0;

	switch(GetWallType())
	{
	case WALL_TYPE_FLOOR:
		iRowCellNum = (int)ceil((m_kLeftTop.x - m_kRightBottom.x) / WALL_CELL_UNIT_SIZE);
		iColCellNum = (int)ceil((m_kLeftTop.y - m_kRightBottom.y) / WALL_CELL_UNIT_SIZE);
		break;
	case WALL_TYPE_WALL:
		iRowCellNum = (int)ceil((m_kLeftTop.z - m_kRightBottom.z) / WALL_CELL_UNIT_SIZE);
		iColCellNum = (int)ceil((m_kLeftTop.y - m_kRightBottom.y) / WALL_CELL_UNIT_SIZE);
		iColCellNum2 = (int)ceil((m_kLeftTop.x - m_kRightBottom.x) / WALL_CELL_UNIT_SIZE);
		break;
	}

	if (iRowCellNum == 0 || iColCellNum == 0)
		return false;

	m_kTotalCellCount.x = static_cast<float>(iRowCellNum);
	m_kTotalCellCount.y = static_cast<float>(iColCellNum + iColCellNum2);

	if (IsWall())
		iRowCellNum = 1;

	if (m_kWallRoot == NULL)
		m_kWallRoot = NiNew NiNode;
	else
	{
		for (unsigned int i = 0; i < m_kWallRoot->GetArrayCount(); i++)
		{
			NiAVObject* pkObject = m_kWallRoot->GetAt(i);
			if (pkObject)
			{
				if (pkObject->GetName().Equals(CELL_MESH_NAME))
				{
					m_kWallRoot->DetachChildAt(i);
				}
			}
		}
	}

	if (m_kWallRoot == NULL)
		return false;

	if (m_pkArrCells)
	{
		ClearAllCells();
	}

	m_pkArrCells = new PgCell[static_cast<int>(m_kTotalCellCount.x * m_kTotalCellCount.y)];
	switch(GetWallType())
	{
	case WALL_TYPE_FLOOR:
		{
			m_iCellsBound1Count = static_cast<int>((m_kTotalCellCount.x * 2) + (m_kTotalCellCount.y * 2));
			m_pkCellsBound1 = new PgCell[m_iCellsBound1Count];
		}break;
	case WALL_TYPE_WALL:
		{
			m_iCellsBound1Count = static_cast<int>((m_kTotalCellCount.x * 2) + (iColCellNum * 2));
			m_iCellsBound2Count = static_cast<int>((m_kTotalCellCount.x * 2) + (iColCellNum2 * 2));
			m_pkCellsBound1 = new PgCell[m_iCellsBound1Count];
			m_pkCellsBound2 = new PgCell[m_iCellsBound2Count];
		}break;
	}

	PG_ASSERT_LOG(m_pkArrCells);
	if ( NULL == m_pkArrCells || NULL == m_pkCellsBound1 || (NULL == m_pkCellsBound2 && GetWallType() == WALL_TYPE_WALL) )
	{
		ClearAllCells();
		return false;
	}

	const float fZOffset = 0.0f;

	SetCellsBound(iColCellNum, iColCellNum2, fZOffset);

	float fCountInCellLength = CELL_OBJECT_LENGTH / WALL_CELL_UNIT_SIZE;		// 셀오브젝트 한변에 들어가는 셀의 겟수 4
	float fCountInCellObject = FLOOR_CELL_OBJECT_SIZE / FLOOR_CELL_OBJECT_SIZE;	// 셀오브젝트에 들어가는 셀의 겟수 16
	float fCellCountInRow = m_kTotalCellCount.x / FLOOR_CELL_OBJECT_SIZE;
	float fCellCountInCol = m_kTotalCellCount.y / FLOOR_CELL_OBJECT_SIZE;
	for (int row = 0 ; row < iRowCellNum; row += FLOOR_CELL_OBJECT_SIZE)
	{
		for (int col = 0; col < iColCellNum + iColCellNum2; col += FLOOR_CELL_OBJECT_SIZE)
		{
			PgFurniture* pkFurniture = NULL;
			if( GetPhysxNum() )
			{
				pkFurniture = PgFurniture::GetFurnitureFromDef(GetPhysxNum());
				if( pkFurniture )
				{
					if( g_pkWorld )
					{
						pkFurniture->InitPhysX(g_pkWorld->GetPhysXScene(), OGT_FURNITURE);
					}
				}
			}

			NiAVObjectPtr kCellClone = (NiAVObject*)m_kCellObject->Clone();
			PG_ASSERT_LOG(kCellClone);
			NiNode* kCellObj = NiDynamicCast(NiNode, kCellClone);
			if( !kCellObj )
			{
				continue;
			}

			if( pkFurniture )
			{
				TexturePair::ApplyTexture(kCellObj, pkFurniture->GetTexturePathContainer());
			}

			NiPoint3 kCellOffset = NiPoint3::ZERO;
			NiPoint3 kWallOffset = NiPoint3::ZERO;
			NiQuaternion kCellRot = NiQuaternion::IDENTITY;

			int iCellStart = 0;
			int iCellEnd = 0;

			switch(m_eWallType)
			{
			case WALL_TYPE_FLOOR:
				{
					kCellOffset.x = static_cast<float>((row + FLOOR_CELL_OBJECT_SIZE / 2) * WALL_CELL_UNIT_SIZE);
					kCellOffset.y = static_cast<float>((col + FLOOR_CELL_OBJECT_SIZE / 2) * WALL_CELL_UNIT_SIZE);
					kCellOffset.z = fZOffset;
					kWallOffset = kCellOffset;
					kCellRot.FromAngleAxis(NI_HALF_PI, NiPoint3::UNIT_Z);

					iCellStart = static_cast<int>((row * fCountInCellLength * fCellCountInRow) + (col * fCountInCellLength));
					//iCellEnd = iCellStart + fCountInCellObject;
					for(int ii = 0; ii < fCountInCellLength; ++ii)
					{
						float fXPos = static_cast<float>((-(WALL_CELL_UNIT_SIZE * 2) + 5) + (ii * WALL_CELL_UNIT_SIZE));
						for(int jj = 0; jj < fCountInCellLength; ++jj)
						{
							m_pkArrCells[iCellStart].m_spBaseCell = kCellObj;

							float fYPos = static_cast<float>((-(WALL_CELL_UNIT_SIZE * 2) + 5) + (jj * WALL_CELL_UNIT_SIZE));
							NiNodePtr kCellUnit = NiNew NiNode();
							if( kCellUnit )
							{
								m_pkArrCells[iCellStart].m_spCellTile = kCellUnit;
								NiPoint3 kPos(fXPos, fYPos, 0);
								kCellUnit->SetTranslate(kPos);
								kCellObj->AttachChild(kCellUnit, true);
							}

							++iCellStart;
						}
					}

				}break;
			case WALL_TYPE_WALL:
				{
					NiQuaternion kWall2Rot = NiQuaternion::IDENTITY;
					NiPoint3 kAddOffset = NiPoint3::ZERO;
					if (col < iColCellNum) // left
					{
						kCellOffset.z = fZOffset; //(row + FLOOR_CELL_OBJECT_SIZE / 2) * WALL_CELL_UNIT_SIZE;	// todo 나중엔 0
						kCellOffset.y = static_cast<float>((col + FLOOR_CELL_OBJECT_SIZE / 2) * WALL_CELL_UNIT_SIZE);
						kWallOffset = kCellOffset;
						kWallOffset.x = WALL_CELL_UNIT_SIZE;
						kCellRot.FromAngleAxis(NI_HALF_PI, NiPoint3::UNIT_Z);
						kWall2Rot.FromAngleAxis(NI_HALF_PI+NI_PI, NiPoint3::UNIT_Z);

						kAddOffset.x = static_cast<float>((iColCellNum - 1 ) * WALL_CELL_UNIT_SIZE);
					}
					else
					{
						kCellOffset.z = fZOffset; //(row + FLOOR_CELL_OBJECT_SIZE / 2) * WALL_CELL_UNIT_SIZE; // todo 나중엔 0
						kCellOffset.x = static_cast<float>((col - iColCellNum + FLOOR_CELL_OBJECT_SIZE / 2) * WALL_CELL_UNIT_SIZE);
						kWallOffset = kCellOffset;
						kWallOffset.y = WALL_CELL_UNIT_SIZE;
						kCellRot.FromAngleAxis(NI_PI, NiPoint3::UNIT_Z);
						kWall2Rot.FromAngleAxis(0, NiPoint3::UNIT_Z);

						kAddOffset.y = static_cast<float>((iColCellNum2 - 1 ) * WALL_CELL_UNIT_SIZE);
					}

					PgFurniture* pkFurniture2 = NULL;
					if( GetPhysxNum() )
					{
						int iSize = iColCellNum2 / FLOOR_CELL_OBJECT_SIZE;
						if( iSize % 2 )
						{
							iSize = iSize / 2;
						}
						else
						{
							iSize = iSize / 2 - 1;
						}
						if( (iSize * FLOOR_CELL_OBJECT_SIZE + iColCellNum ) != col )
						{
							pkFurniture2 = PgFurniture::GetFurnitureFromDef(GetPhysxNum());
							if( pkFurniture2 )
							{
								if( g_pkWorld )
								{
									pkFurniture2->InitPhysX(g_pkWorld->GetPhysXScene(), OGT_FURNITURE);

									pkFurniture2->SetTranslate(kCellOffset + kAddOffset);
									pkFurniture2->SetRotate(kWall2Rot);
									m_kWallRoot->AttachChild(pkFurniture2->GetNIFRoot(), true);
									pkFurniture2->SetAppCulled(true);
									m_kWalls.push_back(pkFurniture2);
								}
							}
						}
					}

					iCellStart = static_cast<int>((row * fCountInCellLength * fCellCountInRow) + (col * fCountInCellLength * fCellCountInRow));
					//iCellEnd = iCellStart + (fCountInCellObject * fCellCountInRow);
					for(int ii = 0; ii < fCountInCellLength * fCellCountInRow; ++ii)
					{
						float fXPos = static_cast<float>((-(WALL_CELL_UNIT_SIZE * 2) + 5) + (ii * WALL_CELL_UNIT_SIZE));
						for(int jj = 0; jj < fCountInCellLength; ++jj)
						{
							m_pkArrCells[iCellStart].m_spBaseCell = kCellObj;

							float fYPos = static_cast<float>((-(WALL_CELL_UNIT_SIZE * 2) + 5) + (jj * WALL_CELL_UNIT_SIZE));
							NiNodePtr kCellUnit = NiNew NiNode();
							if( kCellUnit )
							{
								m_pkArrCells[iCellStart].m_spCellTile = kCellUnit;
								NiPoint3 kPos(0,0,0);
								if (col < iColCellNum) // left
								{
									kPos.y = fYPos;
									kPos.z = fXPos;
								}
								else
								{
									kPos.x = fYPos;
									kPos.z = fXPos;
								}
								kCellUnit->SetTranslate(kPos);
								kCellObj->AttachChild(kCellUnit, true);
							}

							++iCellStart;
						}
					}

				}break;
			}

			kCellObj->SetTranslate(kCellOffset);
			kCellObj->SetRotate(kCellRot);

			m_kWallRoot->AttachChild(kCellObj, true);

			if( pkFurniture )
			{
				pkFurniture->SetTranslate(kWallOffset);
				pkFurniture->SetRotate(kCellRot);
				m_kWallRoot->AttachChild(pkFurniture->GetNIFRoot(), true);
				pkFurniture->SetAppCulled(true);
				m_kWalls.push_back(pkFurniture);
			}
		}
	}

	if( WALL_TYPE_WALL == m_eWallType )
	{
		m_pkDoorTile = PgFurniture::GetFurnitureFromDef(DOOR_NO);
		if( m_pkDoorTile )
		{
			if( g_pkWorld )
			{
				m_pkDoorTile->InitPhysX(g_pkWorld->GetPhysXScene(), OGT_FURNITURE);

				NiPoint3 kPos = NiPoint3::ZERO;
				int iSize = iColCellNum2 / FLOOR_CELL_OBJECT_SIZE;
				if( iSize % 2 )
				{
					iSize = iSize / 2;
				}
				else
				{
					iSize = iSize / 2 - 1;
				}
				kPos.x = (iSize * FLOOR_CELL_OBJECT_SIZE + FLOOR_CELL_OBJECT_SIZE / 2) * WALL_CELL_UNIT_SIZE;
				kPos.y = (iColCellNum + FLOOR_CELL_OBJECT_SIZE / 2) * WALL_CELL_UNIT_SIZE;
				kPos.z = fZOffset;
				m_pkDoorTile->SetTranslate(kPos);

				NiQuaternion kRot = NiQuaternion::IDENTITY;
				kRot.FromAngleAxis(NI_HALF_PI, NiPoint3::UNIT_Z);
				m_pkDoorTile->SetRotate(kRot);

				m_kWallRoot->AttachChild(m_pkDoorTile->GetNIFRoot(), true);
			}
		}
	}

	m_kWallRoot->UpdateNodeBound();
	m_kWallRoot->UpdateProperties();
	m_kWallRoot->UpdateEffects();
    NewWare::Scene::ApplyTraversal::Geometry::SetDefaultMaterialNeedsUpdateFlag( m_kWallRoot, false );
	m_kWallRoot->Update(0.0f);

	HideAllCellTiles(true);

	return true;
}

bool PgWall::BuildGardenCells()
{
	if (m_kCellObject == NULL)
		return false;

	if (m_eWallType != WALL_TYPE_FLOOR)
		return false;

	if (m_kRightBottom == NiPoint3::ZERO && m_kLeftTop == NiPoint3::ZERO)
		return false;

	int iRowCellNum = (int)ceil((m_kRightBottom.x - m_kLeftTop.x) / WALL_CELL_UNIT_SIZE);
	int iColCellNum = (int)ceil((m_kRightBottom.y - m_kLeftTop.y) / WALL_CELL_UNIT_SIZE);

	if (iRowCellNum <= 0 || iColCellNum <= 0)
		return false;

	m_kTotalCellCount.x = static_cast<float>(iRowCellNum);
	m_kTotalCellCount.y = static_cast<float>(iColCellNum);

	if (m_kWallRoot == NULL)
		m_kWallRoot = NiNew NiNode;
	else
	{
		for (unsigned int i = 0; i < m_kWallRoot->GetArrayCount(); i++)
		{
			NiAVObject* pkObject = m_kWallRoot->GetAt(i);
			if (pkObject)
			{
				if (pkObject->GetName().Equals(CELL_MESH_NAME))
				{
					m_kWallRoot->DetachChildAt(i);
				}
			}
		}
	}

	if (m_kWallRoot == NULL)
		return false;

	if (m_pkArrCells)
	{
		ClearAllCells();
	}

	m_pkArrCells = new PgCell[static_cast<int>(m_kTotalCellCount.x * m_kTotalCellCount.y)];

	m_iCellsBound1Count = static_cast<int>((m_kTotalCellCount.x * 2) + (m_kTotalCellCount.y * 2));
	m_pkCellsBound1 = new PgCell[m_iCellsBound1Count];

	if ( NULL == m_pkArrCells || NULL == m_pkCellsBound1 )
	{
		ClearAllCells();
		return false;
	}

	const float fZOffset = 0.0f;

	SetGardenCellsBound(fZOffset);

	int iIndex = 0;
	for (int row = 0 ; row < iRowCellNum; ++row)
	{
		for (int col = 0; col < iColCellNum; ++col)
		{
			NiPoint3 kCellOffset = NiPoint3::ZERO;
			NiQuaternion kCellRot = NiQuaternion::IDENTITY;

			kCellOffset.x = ((-m_kTotalCellCount.x / 2) * WALL_CELL_UNIT_SIZE) + (WALL_CELL_UNIT_SIZE / 2) + ( WALL_CELL_UNIT_SIZE * row);
			kCellOffset.y = static_cast<float>((-WALL_CELL_UNIT_SIZE / 2) - (WALL_CELL_UNIT_SIZE * col));
			kCellOffset.z = fZOffset;
			kCellRot.FromAngleAxis(0, NiPoint3::UNIT_Z);

			m_pkArrCells[iIndex].m_spBaseCell = m_kCellObject;
			NiNodePtr kCellUnit = NiNew NiNode();
			if( kCellUnit )
			{
				m_pkArrCells[iIndex].m_spCellTile = kCellUnit;

				kCellUnit->SetTranslate(kCellOffset);
				kCellUnit->SetRotate(kCellRot);
				m_kWallRoot->AttachChild(kCellUnit, true);
			}
			++iIndex;
		}
	}

	m_kWallRoot->AttachChild(m_kCellObject, true);
	m_kWallRoot->UpdateNodeBound();
	m_kWallRoot->UpdateProperties();
	m_kWallRoot->UpdateEffects();
    NewWare::Scene::ApplyTraversal::Geometry::SetDefaultMaterialNeedsUpdateFlag( m_kWallRoot, false );
	m_kWallRoot->Update(0.0f);

	HideAllCellTiles(true);

	return true;
}

bool PgWall::ChangeWallCell(NiAVObjectPtr spCellObject)
{
	if (spCellObject == NULL)
		return false;

	SetCellObject(spCellObject);
	return BuildWallCells();
}

bool PgWall::ChangeWallCell(PgFurniture* pCellObject)
{
	if (pCellObject == NULL)
		return false;

	SetCellObject(pCellObject);
	return BuildWallCells();
}

bool PgWall::ChangeWallTextures(NiFixedString strTextureName, NiFixedString strNewTexture)
{
	return true;
}

bool PgWall::AttachFurniture(PgFurniture* pkAttachment, NiPoint3 const& rkPos, int iRotate, BM::GUID const& kGuid)
{
	if (pkAttachment == NULL)
		return false;

	if (m_kWallRoot == NULL || m_eWallType == WALL_TYPE_NONE)
		return false;

	switch(pkAttachment->GetFurnitureType())
	{
	case FURNITURE_TYPE_OBJECT_ATTACH:
		{
			if (IsFloor() == false)
			{
				return false;
			}
			m_kWallAttachments.push_back(pkAttachment);
			pkAttachment->SetGuid(kGuid);
			return true;
		}break;
	case FURNITURE_TYPE_OBJECT:
	case FURNITURE_TYPE_NPC:
	case FURNITURE_TYPE_FLOOR_ATTACH:
	case FURNITURE_TYPE_ETC:
		if (IsFloor() == false)
		{
			return false;
		}
		m_kWallAttachments.push_back(pkAttachment);
		break;
	case FURNITURE_TYPE_WALL_ATTACH:
		if (IsWall() == false)
		{
			return false;
		}
		m_kWallAttachments.push_back(pkAttachment);
		break;
	case FURNITURE_TYPE_WALL:	// 벽 오브젝트를 바꾸자.
		if (IsWall() == false)
		{
			return false;
		}
		m_kWallAttachments.push_back(pkAttachment);
		return ChangeWallCell(pkAttachment->GetNIFRoot());
		break;
	case FURNITURE_TYPE_FLOOR:	// 바닥 오브젝트를 바꾸자.
		if (IsFloor() == false)
		{
			return false;
		}
		m_kWallAttachments.push_back(pkAttachment);
		return ChangeWallCell(pkAttachment->GetNIFRoot());
		break;
	default:
		{
			NILOG(PGLOG_ERROR, "[PgWall] AttachFurniture, %d furniture has invalid type(%d)\n", pkAttachment->GetFurnitureNo(), pkAttachment->GetFurnitureType());
		}return false;
	}

	NiAVObject* pkFurnitureRoot = pkAttachment->GetNIFRoot();
	if (pkFurnitureRoot)
	{
		NiQuaternion kRot = NiQuaternion::IDENTITY;
		kRot.FromAngleAxis(NI_HALF_PI*0.5f, NiPoint3::UNIT_Z);
		pkAttachment->SetRotate(kRot);

		m_kWallRoot->AttachChild(pkFurnitureRoot, true);
		m_kWallRoot->UpdateProperties();
		m_kWallRoot->UpdateNodeBound();
		m_kWallRoot->UpdateEffects();
		m_kWallRoot->Update(0.0f);

		if( FURNITURE_TYPE_OBJECT == pkAttachment->GetFurnitureType())
		{
			NiTransform kTrn;
			kTrn.MakeIdentity();
			//kTrn.m_Translate = rkInterPt;
			NiQuaternion kRot = NiQuaternion::IDENTITY;
			NiQuaternion kNormalQuat(NiACos(NiPoint3::UNIT_Z.Dot(m_kNormal)), NiPoint3::UNIT_Z.UnitCross(m_kNormal));
			//NiQuaternion kRotateQuat(iRotate * ROTATION_ANGLE + (NI_HALF_PI*0.5f), NiPoint3::UNIT_Z);
			NiQuaternion kRotateQuat(iRotate * ROTATION_ANGLE, NiPoint3::UNIT_Z);
			kRot = kNormalQuat * kRotateQuat;
			kRot.ToRotation(kTrn.m_Rotate);
			if( pkAttachment->GetItemDef() )
			{
				if( UICT_HOME_SIDEJOB_NPC == pkAttachment->GetItemDef()->GetAbil(AT_USE_ITEM_CUSTOM_TYPE) )
				{
					NiQuaternion kNpcRot = kNormalQuat;
					NiQuaternion kRotateTemp(ROTATION_ANGLE*(-0.5), NiPoint3::UNIT_Z);
					kNpcRot = kNpcRot * kRotateTemp;
					kNpcRot.ToRotation(kTrn.m_Rotate);
				}
			}
			pkFurnitureRoot->SetLocalFromWorldTransform(kTrn);
		}
	}

	MoveFurniture(pkAttachment, rkPos, iRotate, true);
	pkAttachment->SetGuid(kGuid);

	return true;
}

bool PgWall::MoveFurniture(PgFurniture* pkAttachment, NiPoint3 const& rkPos, int iRotate, bool bForce)
{
	if (pkAttachment == NULL)
		return false;

	if (m_kWallRoot == NULL || m_eWallType == WALL_TYPE_NONE)
		return false;

	FurnitureContainer::iterator furniture_iter = std::find(m_kWallAttachments.begin(), m_kWallAttachments.end(), pkAttachment);
	if (furniture_iter == m_kWallAttachments.end())
	{
		// container에 없는 furniture를 움직일 수 있을까?
		return false;
	}

	NiAVObject* pkFurnitureRoot = pkAttachment->GetNIFRoot();
	if (pkFurnitureRoot)
	{
		NiTransform kTrn;
		kTrn.MakeIdentity();
		NiPoint3 kStartPt = rkPos;
		NiPoint3 kRayDir;
		NiPoint3 kNormal;
		switch(m_eWallType)
		{
		case WALL_TYPE_FLOOR:
			kNormal = NiPoint3::UNIT_Z;
			break;
		case WALL_TYPE_WALL:
			{
				int iSide = whichSide(rkPos);
				if (iSide == WALL_LEFT)
				{
					kNormal = m_kWallRoot->GetWorldTransform().m_Rotate * NiPoint3::UNIT_X;
				}
				else if (iSide == WALL_RIGHT)
				{
					kNormal = m_kWallRoot->GetWorldTransform().m_Rotate * NiPoint3::UNIT_Y;
				}
				else
				{
					return false;
				}
			}
			
			break;
		}
		
		kRayDir = -kNormal;
		kStartPt = rkPos + kNormal * 500;

		NiPick kPick;
	
		kPick.SetFrontOnly(true);
		kPick.SetIntersectType(NiPick::TRIANGLE_INTERSECT);
		kPick.SetReturnNormal(true);
		kPick.SetPickType(NiPick::FIND_ALL);
		kPick.SetObserveAppCullFlag(false);
		kPick.SetSortType(NiPick::SORT);		
		kPick.SetTarget(m_kWallRoot);
		
		NiPick::Record *pkRec = NULL;

		if(kPick.PickObjects(kStartPt, kRayDir))
		{
			NiPick::Results &rkRes = kPick.GetResults();
			for (unsigned int i = 0; i < rkRes.GetEffectiveSize(); i++)
			{
				pkRec = rkRes.GetAt(i);
				if (pkRec)
				{
					NiAVObject* pkPicked = pkRec->GetAVObject();
					NiAVObject* pkCellMesh = NULL;
					PgFurniture* pkPickedFurniture = NULL;
					if (IsCellMesh(pkPicked, pkCellMesh))
					{
						break;
					}
					//else if (PgFurniture::IsFurnitureMesh(pkPicked, pkPickedFurniture))
					//{
					//	if (pkPickedFurniture != pkAttachment)
					//		break;
					//}
				}
				pkRec = NULL;
			}
#ifdef _DEBUG
			removeAllArrow();
			NiPick::Record *pkRec2 = NULL;
			for (int i = 0; i < rkRes.GetEffectiveSize(); i++)
			{
				pkRec2 = rkRes.GetAt(i);
				if (pkRec2)
				{
					NiPoint3 const &rkInterPt = pkRec2->GetIntersection();
					NiPoint3 const &rkNormal = pkRec2->GetNormal();
					attachArrowToPickPoint(rkInterPt, rkNormal);
				}
			}
#endif

			if (pkRec)
			{
				NiPoint3 const &rkInterPt = pkRec->GetIntersection();
				NiPoint3 const &rkNormal = pkRec->GetNormal();
				kTrn.m_Translate = rkInterPt + (rkNormal);

			}
			else
			{
				return false;
			}
		}
		else
		{
			return false;
		}

		if( pkAttachment->GetFurnitureType() == FURNITURE_TYPE_OBJECT_ATTACH )
		{
			int iDummyPos = 0;
			NiQuaternion kRot = NiQuaternion::IDENTITY;
			NiQuaternion kNormalQuat(NiACos(NiPoint3::UNIT_Z.Dot(kNormal)), NiPoint3::UNIT_Z.UnitCross(kNormal));
			NiQuaternion kRotateQuat(iRotate * ROTATION_ANGLE, NiPoint3::UNIT_Z);
			kRot = kNormalQuat * kRotateQuat;

			PgFurniture* pkParent = CheckParentFurniture(pkAttachment, kTrn.m_Translate, iDummyPos);
			if( pkParent )
			{
				NiAVObject* pkNode = GetDummyNode(pkParent, iDummyPos);
				if( pkNode )
				{	
					NiTransform kTran;
					kTran.MakeIdentity();
					kRot.ToRotation(kTran.m_Rotate);
					kTran.m_Rotate = kTran.m_Rotate * pkNode->GetWorldRotate();
					kTran.m_Translate = pkNode->GetWorldTransform().m_Translate;

					pkFurnitureRoot->SetLocalFromWorldTransform(kTran);
					pkAttachment->SetParentFurniture(pkParent);
					pkAttachment->SetParentIndex(iDummyPos);
					if( !bForce )
					{
						pkAttachment->SetFurnitureColor(NiColor::WHITE * 2);
					}
				}
				else
				{
					kRot.ToRotation(kTrn.m_Rotate);
					pkFurnitureRoot->SetLocalFromWorldTransform(kTrn);
					if( !bForce )
					{
						pkAttachment->SetFurnitureColor(NiColor(3,1,1));
					}
					pkAttachment->SetParentFurniture(NULL);
				}
			}
			else
			{
				kRot.ToRotation(kTrn.m_Rotate);
				pkFurnitureRoot->SetLocalFromWorldTransform(kTrn);
				if( !bForce )
				{
					pkAttachment->SetFurnitureColor(NiColor(3,1,1));
				}
				pkAttachment->SetParentFurniture(NULL);
			}
		}
		else
		{
		
			NiQuaternion kRot = NiQuaternion::IDENTITY;
			NiQuaternion kNormalQuat(NiACos(NiPoint3::UNIT_Z.Dot(kNormal)), NiPoint3::UNIT_Z.UnitCross(kNormal));
			if(FURNITURE_TYPE_OBJECT == pkAttachment->GetFurnitureType())
			{
				//NiQuaternion kRotateQuat(iRotate * ROTATION_ANGLE + (NI_HALF_PI*0.5f), NiPoint3::UNIT_Z);
				NiQuaternion kRotateQuat(iRotate * ROTATION_ANGLE, NiPoint3::UNIT_Z);
				kRot = kNormalQuat * kRotateQuat;
			}
			else
			{
				float fRot = 0;
				if( m_eWallType == WALL_TYPE_WALL )
				{
					int iSide = whichSide(rkPos);
					if (iSide == WALL_LEFT)
					{
						fRot = -NI_HALF_PI;
					}
				}
				NiQuaternion kRotateQuat(fRot, NiPoint3::UNIT_Z);
				kRot = kRotateQuat;
			}

			kRot.ToRotation(kTrn.m_Rotate);
			if( pkAttachment->GetItemDef() )
			{
				if( UICT_HOME_SIDEJOB_NPC == pkAttachment->GetItemDef()->GetAbil(AT_USE_ITEM_CUSTOM_TYPE) )
				{
					NiQuaternion kNpcRot = kNormalQuat;
					NiQuaternion kRotateTemp(ROTATION_ANGLE*(-0.5), NiPoint3::UNIT_Z);
					kNpcRot = kNpcRot * kRotateTemp;
					kNpcRot.ToRotation(kTrn.m_Rotate);
				}
			}

			pkAttachment->SetNormal(kNormal);
			pkFurnitureRoot->SetLocalFromWorldTransform(kTrn);

			if( !bForce )
			{
				if( !CheckCellsBound(pkAttachment) || !CheckOtherFurniture(pkAttachment) )
				{
					pkAttachment->SetFurnitureColor(NiColor(3,1,1));
				}
				else
				{
					pkAttachment->SetFurnitureColor(NiColor::WHITE * 2);
				}
			}
		}

		
	}
	else
	{
		return false;
	}

	m_kWallRoot->Update(0.0f);
	
	// check attachment can attached
	
	// update cell info
	
	return true;
}

void PgWall::RemoveAllFurniture()
{
	FurnitureContainer::iterator kIter = m_kWallAttachments.begin();

	while (kIter != m_kWallAttachments.end())
	{
		PgFurniture* pkFurniture = *kIter;
		if( pkFurniture )
		{
			NiNode* pkParent = pkFurniture->GetParent();
			if( pkParent )
			{
				pkParent->DetachChild(pkFurniture->GetNIFRoot());
			}
		}
		++kIter;
	}

	kIter = m_kWallAttachments.begin();
	while (kIter != m_kWallAttachments.end())
	{
		PgFurniture::DeleteFurniture(*kIter);
		++kIter;
	}
	m_kWallAttachments.clear();
}

void PgWall::RemovePhysxWalls()
{
	FurnitureContainer::iterator itr = m_kWalls.begin();
	while( itr != m_kWalls.end() )
	{
		PgFurniture* pkFurniture = *(itr);
		if( pkFurniture )
		{
			if (m_kWallRoot)
			{
				m_kWallRoot->DetachChild(pkFurniture->GetNIFRoot());
			}
			PgFurniture::DeleteFurniture(pkFurniture);
		}
		++itr;
	}
	m_kWalls.clear();
}

void PgWall::RemoveFurniture(PgFurniture* pkFurniture)
{
	if (pkFurniture == NULL)
		return;

	FurnitureContainer::iterator kIter = std::find(m_kWallAttachments.begin(), m_kWallAttachments.end(), pkFurniture);
	if (kIter != m_kWallAttachments.end())
	{
		PgFurniture* pkFurniture = *(kIter);
		removeFurniture(pkFurniture);
		m_kWallAttachments.erase(kIter);
	}
}

void PgWall::RemoveFurniture(BM::GUID const& kGuid)
{
	FurnitureContainer::iterator kIter = m_kWallAttachments.begin();
	while(kIter != m_kWallAttachments.end())
	{
		PgFurniture* pkFurniture = *(kIter);
		if( pkFurniture->GetGuid() == kGuid )
		{
			removeFurniture(pkFurniture);
			m_kWallAttachments.erase(kIter);
			return;
		}
		++kIter;
	}
}


void PgWall::ShowCellTile(int iCellX, int iCellY, bool bShow, bool bOccupied)
{
	return;
}

void PgWall::ShowCellTile(PgFurniture* pkFurniture)
{

}

void PgWall::HideAllCellTiles(bool bHide)
{
	if( m_pkArrCells )
	{
		for (int i = 0; i < m_kTotalCellCount.x * m_kTotalCellCount.y; i++)
		{
			if (m_pkArrCells[i].m_spCellTile)
			{
				m_pkArrCells[i].m_spCellTile->SetAppCulled(bHide);
			}
		}
	}

	if( m_pkCellsBound1 )
	{
		for( int ii = 0; ii < m_iCellsBound1Count; ++ii )
		{
			if( m_pkCellsBound1[ii].m_spCellTile )
			{
				m_pkCellsBound1[ii].m_spCellTile->SetAppCulled(bHide);
			}
		}
	}

	if( m_pkCellsBound2 )
	{
		for( int ii = 0; ii < m_iCellsBound2Count; ++ii )
		{
			if( m_pkCellsBound2[ii].m_spCellTile )
			{
				m_pkCellsBound2[ii].m_spCellTile->SetAppCulled(bHide);
			}
		}
	}
}

bool PgWall::Update(float fAccumTime, float fFrameTime)
{
	if (m_kWallRoot)
		m_kWallRoot->Update(fAccumTime);

	FurnitureContainer::iterator kIter = m_kWallAttachments.begin();

	while (kIter != m_kWallAttachments.end())
	{
		PgFurniture* pkFurniture = *(kIter);
		if (pkFurniture)
			pkFurniture->Update(fAccumTime, fFrameTime);
		++kIter;
	}
	return true;
}

void PgWall::Draw(PgRenderer *pkRenderer, NiCamera *pkCamera, float fFrameTime)
{
	if (pkRenderer == NULL || pkCamera == NULL)
		return;

	FurnitureContainer::iterator kIter = m_kWallAttachments.begin();
	while (kIter != m_kWallAttachments.end())
	{
		PgFurniture* pkFurniture = *(kIter);
		if (pkFurniture)
			pkFurniture->Draw(pkRenderer, pkCamera, fFrameTime);
		++kIter;
	}
}

NiNodePtr PgWall::getCellTile()
{
	NiNodePtr spCellTile;
	spCellTile = g_kNifMan.GetNif_DeepCopy("../Data/6_UI/Quad.Nif");
	if (spCellTile == NULL)
		return NULL;

	NiGeometry *pGeom = NiDynamicCast(NiGeometry, spCellTile->GetObjectByName("Plane01"));
	if (pGeom == NULL)
		return NULL;

	pGeom->SetConsistency(NiGeometryData::MUTABLE);
	pGeom->GetPropertyState()->GetTexturing()->SetApplyMode(NiTexturingProperty::APPLY_MODULATE);

	pGeom->GetPropertyState()->GetAlpha()->SetAlphaTesting(true);
	pGeom->GetPropertyState()->GetAlpha()->SetTestRef(100);
	pGeom->GetPropertyState()->GetAlpha()->SetTestMode(NiAlphaProperty::TEST_GREATEREQUAL);

	pGeom->GetPropertyState()->GetVertexColor()->SetLightingMode(NiVertexColorProperty::LIGHTING_E);
	pGeom->GetPropertyState()->GetVertexColor()->SetSourceMode(NiVertexColorProperty::SOURCE_EMISSIVE);

	NiGeometryData *pGeomData = pGeom->GetModelData();
	if (pGeomData == NULL)
		return NULL;

	const	int	iVerCount = pGeomData->GetVertexCount();
	if (iVerCount <= 0)
		return NULL;

	NiPoint2	*pUV = pGeomData->GetTextureSet(0);
	NiColorA	*pColor = pGeomData->GetColors();
	NiPoint3	*pVertex = pGeomData->GetVertices();

	if (!pUV)
	{
		pUV = NiNew NiPoint2[4];
		pGeomData->AppendTextureSet(pUV);
	}

	if (!pColor)
	{
		pGeomData->CreateColors(true);
		pColor = pGeomData->GetColors();
	}

	NiPropertyState	*pkPS = pGeom->GetPropertyState();
	if(pkPS)
	{
		NiZBufferProperty	*pkZBuffer = pkPS->GetZBuffer();
		if(!pkZBuffer)
		{
			pkZBuffer = NiNew NiZBufferProperty();
			pkZBuffer->SetZBufferTest(true);
			pGeom->AttachProperty(pkZBuffer);
		}

		pkZBuffer->SetZBufferWrite(false);

		NiTexturingProperty* pkTexturing = pkPS->GetTexturing();
		if (pkTexturing == NULL)
		{
			pkTexturing = NiNew NiTexturingProperty();
			pkTexturing->SetBaseTexture(g_kNifMan.GetTexture(CELL_CURSOR_OK));
		}
	}

	spCellTile->UpdateProperties();

	NiColorA kColor = NiColorA::WHITE;
	*pColor = kColor;	pColor++;
	*pColor = kColor;	pColor++;
	*pColor = kColor;	pColor++;
	*pColor = kColor;

	NiPoint3 kOrigin = NiPoint3::ZERO;
	NiPoint3 kX = NiPoint3::UNIT_X * WALL_CELL_UNIT_SIZE;
	NiPoint3 kY = NiPoint3::UNIT_Y * WALL_CELL_UNIT_SIZE;

	*pVertex = kOrigin + kX + kY;	pVertex++;
	*pVertex = kOrigin + kX;	pVertex++;
	*pVertex = kOrigin + kY;	pVertex++;
	*pVertex = kOrigin;

	*pUV = NiPoint2(1, 1);	pUV++;
	*pUV = NiPoint2(1, 0);	pUV++;
	*pUV = NiPoint2(0, 1);	pUV++;
	*pUV = NiPoint2(0, 0);

	pGeomData->MarkAsChanged(NiGeometryData::VERTEX_MASK | NiGeometryData::TEXTURE_MASK | NiGeometryData::COLOR_MASK);
	pGeom->SetRotate(NiQuaternion::IDENTITY);

	NiAVObject *pNode = NiDynamicCast(NiAVObject,spCellTile);

	pNode->SetRotate(NiQuaternion::IDENTITY);
	pNode->UpdateNodeBound();
	pNode->Update(0.0f);
    NewWare::Scene::ApplyTraversal::Geometry::SetDefaultMaterialNeedsUpdateFlag( spCellTile, false );
	
	return spCellTile;
}

void PgWall::SetWallOrigin(NiPoint3& rkOrigin)
{
	if (m_kWallRoot)
	{
		m_kWallRoot->SetTranslate(rkOrigin);
		m_kWallRoot->SetWorldTranslate(rkOrigin);
		m_kWallRoot->UpdateNodeBound();
		m_kWallRoot->Update(0.0f);
	}
}

PgFurniture* PgWall::PickFurniture(NiPoint3& kStartPt, NiPoint3& kRayDir)
{
	NiPick kPick;

	kPick.SetFrontOnly(true);
	kPick.SetIntersectType(NiPick::TRIANGLE_INTERSECT);
	kPick.SetReturnNormal(true);
	kPick.SetPickType(NiPick::FIND_ALL);
	kPick.SetObserveAppCullFlag(false);
	kPick.SetSortType(NiPick::SORT);
	
	kPick.SetTarget(m_kWallRoot);
	//kPick.SetTarget(g_pkWorld->GetObjectGroup(OGT_FURNITURE));
	if(kPick.PickObjects(kStartPt, kRayDir))
	{
		NiPick::Results &rkRes = kPick.GetResults();

		for (unsigned int i = 0; i < rkRes.GetEffectiveSize(); i++)
		{
			NiPick::Record *pkRec = rkRes.GetAt(i);
			if (pkRec)
			{
				NiPoint3 const &rkInterPt = pkRec->GetIntersection();
				NiPoint3 const &rkNormal = pkRec->GetNormal();
				NiAVObject* pkObject = pkRec->GetAVObject();

				PgFurniture* pkPickedFurniture = NULL;
				if (PgFurniture::IsFurnitureMesh(pkObject, pkPickedFurniture))
					return pkPickedFurniture;
			}
		}
	}

	return NULL;
}

NiAVObject* PgWall::PickCell(NiPoint3& kStartPt, NiPoint3& kRayDir, NiPoint3& kCellPosition)
{
	NiPick kPick;

	kPick.SetFrontOnly(true);
	kPick.SetIntersectType(NiPick::TRIANGLE_INTERSECT);
	kPick.SetReturnNormal(true);
	kPick.SetPickType(NiPick::FIND_ALL);
	kPick.SetObserveAppCullFlag(false);
	kPick.SetSortType(NiPick::SORT);
	
	kPick.SetTarget(m_kWallRoot);
	if(kPick.PickObjects(kStartPt, kRayDir))
	{
		NiPick::Results &rkRes = kPick.GetResults();

		for (unsigned int i = 0; i < rkRes.GetEffectiveSize(); i++)
		{
			NiPick::Record *pkRec = rkRes.GetAt(i);
			if (pkRec)
			{
				NiPoint3 const &rkInterPt = pkRec->GetIntersection();
				NiPoint3 const &rkNormal = pkRec->GetNormal();
				NiAVObject* pkObject = pkRec->GetAVObject();

				NiAVObject* pkCell = NULL;
				if (IsCellMesh(pkObject, pkCell))
				{
					kCellPosition = findCellPosition(pkCell, rkInterPt);
					return pkCell;
				}				
			}
		}
	}

	return NULL;
}

bool PgWall::IsCellMesh(NiAVObject* pkObject, NiAVObject*& rkCell)
{
	if (pkObject == NULL)
		return false;

	NiNode* pkParent = (NiNode*)pkObject;
	while (pkParent)
	{
		if (pkParent->GetName().Equals(CELL_MESH_NAME))
		{
			rkCell = (NiAVObject*)pkParent;
			return true;
		}

		pkParent = pkParent->GetParent();
	}

	return false;
}

NiPoint3 PgWall::findCellPosition(NiAVObject* pkCellMesh, NiPoint3 const& rkPos)
{
	if (pkCellMesh == NULL || GetWallType() == WALL_TYPE_NONE)
		return rkPos;

	NiPoint3 kCellPos = rkPos;
	NiTransform kWorldTrn = pkCellMesh->GetWorldTransform();
	NiPoint2 kStartPos;
	NiPoint3 kLocalPos = kWorldTrn.m_Rotate.Inverse() * (rkPos - m_kWallRoot->GetWorldTranslate());
	
	POINT3 kCell;

	switch(GetWallType())
	{
	case WALL_TYPE_FLOOR:
		kCell.x = (int)(kLocalPos.x / WALL_CELL_UNIT_SIZE);
		kCell.y = (int)(kLocalPos.y / WALL_CELL_UNIT_SIZE);
		kLocalPos.x = kCell.x * WALL_CELL_UNIT_SIZE;
		kLocalPos.y = kCell.y * WALL_CELL_UNIT_SIZE;
		break;
	case WALL_TYPE_WALL:
		{
			//int iSide = whichSide(rkPos);
			//if (iSide == WALL_LEFT)
			//{
			//	kCell.x = (int)(kLocalPos.z / WALL_CELL_UNIT_SIZE);
			//	kCell.y = (int)(kLocalPos.y / WALL_CELL_UNIT_SIZE);
			//	kLocalPos.z = kCell.x * WALL_CELL_UNIT_SIZE;
			//	kLocalPos.y = kCell.y * WALL_CELL_UNIT_SIZE;
			//}
			//else if (iSide == WALL_RIGHT)
			//{
			//	kCell.x = (int)(kLocalPos.z / WALL_CELL_UNIT_SIZE);
			//	kCell.y = (int)(kLocalPos.x / WALL_CELL_UNIT_SIZE);
			//	kLocalPos.z = kCell.x * WALL_CELL_UNIT_SIZE;
			//	kLocalPos.x = kCell.y * WALL_CELL_UNIT_SIZE;
			//}
			//else
			//{
			//	return kCellPos;
			//}

			kCell.x = (int)(kLocalPos.x / WALL_CELL_UNIT_SIZE);
			kCell.y = (int)(kLocalPos.y / WALL_CELL_UNIT_SIZE);
			kCell.z = (int)(kLocalPos.z / WALL_CELL_UNIT_SIZE);
			kLocalPos.x = kCell.x * WALL_CELL_UNIT_SIZE;
			kLocalPos.y = kCell.y * WALL_CELL_UNIT_SIZE;
			kLocalPos.z = kCell.z * WALL_CELL_UNIT_SIZE;
		}
		break;
	}

	kCellPos =  kWorldTrn.m_Rotate * (kLocalPos)+ m_kWallRoot->GetWorldTranslate();

	return kCellPos;
}

void PgWall::removeFurniture(PgFurniture* pkFurniture)
{
	if (pkFurniture)
	{
		switch(pkFurniture->GetFurnitureType())
		{
		case FURNITURE_TYPE_OBJECT:
		case FURNITURE_TYPE_OBJECT_ATTACH:
		case FURNITURE_TYPE_FLOOR_ATTACH:
		case FURNITURE_TYPE_ETC:			
		case FURNITURE_TYPE_WALL_ATTACH:
			break;
		case FURNITURE_TYPE_NPC:	// 어찌 해야 할까.
			break;
		case FURNITURE_TYPE_WALL:	// Default로 되돌린다.
			ChangeWallCell(m_kDefaultCellObject);
			PgFurniture::DeleteFurniture(pkFurniture);
			return;			
		case FURNITURE_TYPE_FLOOR:	// Default로 되돌린다.
			ChangeWallCell(m_kDefaultCellObject);
			PgFurniture::DeleteFurniture(pkFurniture);
			return;
		default:
			{
				NILOG(PGLOG_ERROR, "[PgWall] removeFurniture, %d furniture has invalid type(%d)\n", pkFurniture->GetFurnitureNo(), pkFurniture->GetFurnitureType());
			}return;
		}
		
		UnlocateArrangingFurniture(pkFurniture);
		if( pkFurniture->GetParent() )
		{
			pkFurniture->GetParent()->DetachChild(pkFurniture->GetNIFRoot());
			pkFurniture->SetParentFurniture(NULL);
		}

		FurnitureContainer kFCont;
		if( GetChildList(pkFurniture, kFCont) )
		{
			FurnitureContainer::iterator itr = kFCont.begin();
			for(; itr != kFCont.end(); ++itr)
			{
				PgFurniture* pkChild = *itr;
				if( pkChild )
				{
					pkChild->SetParentFurniture(NULL);
					pkFurniture->DetachChild(pkChild->GetNIFRoot());
				}
			}
		}
		PgFurniture::DeleteFurniture(pkFurniture);
	}
}

int PgWall::whichSide(NiPoint3 const& rkPos)
{
	if (IsWall() == false)
		return 0;

	NiPoint3 kCellPos = rkPos;
	NiTransform kWorldTrn = m_kWallRoot->GetWorldTransform();
	NiPoint3 kLocalPos = kWorldTrn.m_Rotate.Inverse() * (rkPos - kWorldTrn.m_Translate);
	
	if( 0.f == kLocalPos.y && 0.f == kLocalPos.x )
	{
		return 0;
	}

	if (kLocalPos.y > kLocalPos.x)
		return WALL_LEFT;

	return WALL_RIGHT;
}

NiPoint3 PgWall::getWallCenter()
{
	NiPoint3 kLocalPos = NiPoint3::ZERO;
	NiTransform kWorldTrn = m_kWallRoot->GetWorldTransform();

	for (unsigned int i = 0; i < m_kWallRoot->GetArrayCount(); i++)
	{
		NiAVObject* pkObject = m_kWallRoot->GetAt(i);
		if (pkObject)
		{
			if (pkObject->GetName().Equals(CELL_MESH_NAME))
			{
				kLocalPos = pkObject->GetLocalTransform().m_Translate;
				break;
			}
		}
	}

	switch(GetWallType())
	{
	case WALL_TYPE_WALL:
		kLocalPos.z = m_kTotalCellCount.x / 2 * WALL_CELL_UNIT_SIZE;
		kLocalPos.x = m_kTotalCellCount.y / 4 * WALL_CELL_UNIT_SIZE;	// 2배라서 4로 나눔.
		break;
	case WALL_TYPE_FLOOR:
		kLocalPos.x = m_kTotalCellCount.x / 2 * WALL_CELL_UNIT_SIZE;
		kLocalPos.y = m_kTotalCellCount.y / 2 * WALL_CELL_UNIT_SIZE;
		break;
	}
	
	return kWorldTrn.m_Rotate * (kLocalPos)+ kWorldTrn.m_Translate;
}

#ifndef _DEBUG
void PgWall::attachArrowToPickPoint(NiPoint3 const& rkPos, NiPoint3 const& rkNormal)
{
	NiAVObjectPtr spArrow = g_kNifMan.GetNif("../Data/5_Effect/4_UI/Arrow_01.nif");
	if (spArrow == NULL)
		return;

	m_kWallRoot->AttachChild(spArrow, true);
	m_kWallRoot->UpdateProperties();
	m_kWallRoot->UpdateNodeBound();
	m_kWallRoot->UpdateEffects();
	m_kWallRoot->Update(0.0f);

	NiQuaternion kRot;
	kRot.FromAngleAxis(0, rkNormal);
	spArrow->SetWorldTranslate(rkPos);
	spArrow->SetRotate(kRot);
	//spArrow->SetScale(0.3f);
}

void PgWall::removeAllArrow()
{
	for (unsigned int i = 0; i < m_kArrowArray.GetSize(); i++)
	{
		NiAVObject* pkObject = m_kArrowArray.GetAt(i);
		if (pkObject)
		{
			m_kWallRoot->DetachChild(pkObject);
		}
	}
	m_kArrowArray.RemoveAll();
}
#endif

NiPoint3 PgWall::GetNormal()
{
	return m_kNormal;
}

bool PgWall::LocateFurniture(PgFurniture* pkFurniture)
{
	if( NULL == pkFurniture )
	{
		return false;
	}

	int iStart = 0;
	int iEnd = static_cast<int>(m_kTotalCellCount.x * m_kTotalCellCount.y);
	NiPoint3 kNormal;
	switch(m_eWallType)
	{
	case WALL_TYPE_FLOOR:
		kNormal = NiPoint3::UNIT_Z;
		break;
	case WALL_TYPE_WALL:
		{
			int iSide = whichSide(pkFurniture->GetWorldTransform().m_Translate);
			if (iSide == WALL_LEFT)
			{
				kNormal = m_kWallRoot->GetWorldTransform().m_Rotate * NiPoint3::UNIT_X;
				iEnd = iEnd / 2 - 1;
			}
			else if (iSide == WALL_RIGHT)
			{
				kNormal = m_kWallRoot->GetWorldTransform().m_Rotate * NiPoint3::UNIT_Y;
				iStart = iEnd / 2 - 1;
			}
			else
			{
				return false;
			}
		}
		
		break;
	}

	NiPoint3 kStartPt;
	NiPoint3 kRayDir = -kNormal;
	
	NiPick kPick;
	kPick.SetFrontOnly(true);
	kPick.SetIntersectType(NiPick::TRIANGLE_INTERSECT);
	kPick.SetReturnNormal(true);
	kPick.SetPickType(NiPick::FIND_ALL);
	kPick.SetObserveAppCullFlag(false);
	kPick.SetSortType(NiPick::SORT);		
	kPick.SetTarget(pkFurniture);
	
	bool bOccupied = false;
	NiPick::Record *pkRec = NULL;
	for( int j = iStart; j < iEnd; ++j )
	{
		kStartPt = m_pkArrCells[j].m_spCellTile->GetWorldTransform().m_Translate + (kNormal * 200);

		if(kPick.PickObjects(kStartPt, kRayDir))
		{
			m_pkArrCells[j].m_bOccupied = true;
			m_pkArrCells[j].m_pkOccupiedObject = pkFurniture;
			bOccupied = true;
		}
	}

	if( !bOccupied && m_kWallRoot)// 물체가 작아서 픽킹이 안될경우
	{
		int iPos = 0;
		if( FindUnderCellofFurniture(pkFurniture, kRayDir, iStart, iEnd, iPos) )
		{
			m_pkArrCells[iPos].m_bOccupied = true;
			m_pkArrCells[iPos].m_pkOccupiedObject = pkFurniture;
			bOccupied = true;
		}
	}
	return bOccupied;
}

//bool PgWall::LocateArrangingFurniture(PgFurniture* pkFurniture)
//{
//	if( NULL == pkFurniture )
//	{
//		return false;
//	}
//
//	int iStart = 0;
//	int iEnd = m_kTotalCellCount.x * m_kTotalCellCount.y;
//	NiPoint3 kNormal;
//	switch(m_eWallType)
//	{
//	case WALL_TYPE_FLOOR:
//		kNormal = NiPoint3::UNIT_Z;
//		break;
//	case WALL_TYPE_WALL:
//		{
//			int iSide = whichSide(pkFurniture->GetWorldTransform().m_Translate);
//			if (iSide == WALL_LEFT)
//			{
//				kNormal = m_kWallRoot->GetWorldTransform().m_Rotate * NiPoint3::UNIT_X;
//				iEnd = iEnd / 2 - 1;
//			}
//			else if (iSide == WALL_RIGHT)
//			{
//				kNormal = m_kWallRoot->GetWorldTransform().m_Rotate * NiPoint3::UNIT_Y;
//				iStart = iEnd / 2 - 1;
//			}
//			else
//			{
//				return false;
//			}
//		}
//		
//		break;
//	}
//
//	NiPoint3 kStartPt;
//	NiPoint3 kRayDir = -kNormal;
//	
//	NiPick kPick;
//	kPick.SetFrontOnly(true);
//	kPick.SetIntersectType(NiPick::TRIANGLE_INTERSECT);
//	kPick.SetReturnNormal(true);
//	kPick.SetPickType(NiPick::FIND_ALL);
//	kPick.SetObserveAppCullFlag(false);
//	kPick.SetSortType(NiPick::SORT);		
//	kPick.SetTarget(pkFurniture);
//	
//	NiPick::Record *pkRec = NULL;
//
//	std::vector<int> vecList;
//	
//	for( int j = iStart; j < iEnd; ++j )
//	{
//		kStartPt = m_pkArrCells[j].m_spCellTile->GetWorldTransform().m_Translate + (kNormal * 200);
//
//		if(kPick.PickObjects(kStartPt, kRayDir))
//		{
//			if( m_pkArrCells[j].m_bOccupied )
//			{
//				return false;
//			}
//			
//			vecList.push_back(j);
//		}
//	}
//
//	if( vecList.empty() )
//	{
//		return false;
//	}
//
//	for( int i = 0; i < vecList.size(); ++i )
//	{
//		m_pkArrCells[vecList.at(i)].m_bOccupied = true;
//		m_pkArrCells[vecList.at(i)].m_pkOccupiedObject = pkFurniture;
//	}
//
//	return true;
//}

bool PgWall::UnlocateArrangingFurniture(PgFurniture* pkFurniture)
{
	if( !pkFurniture )
	{
		return false;
	}

	if( pkFurniture->GetFurnitureType() == FURNITURE_TYPE_OBJECT_ATTACH )
	{
		PgFurniture* pkParent = pkFurniture->GetParentFurniture();
		if( pkParent )
		{
			if( pkFurniture->GetParent() )
			{
				pkFurniture->GetParent()->DetachChild(pkFurniture->GetNIFRoot());
				pkFurniture->SetParentFurniture(NULL);
				if(m_kWallRoot)
				{
					m_kWallRoot->AttachChild(pkFurniture->GetNIFRoot());
				}
			}
			//NiAVObject* pkAVObject = GetDummyNode(pkParent, pkFurniture->GetParentIndex());
			//if( pkAVObject )
			//{
			//	NiNode* pkNode = NiDynamicCast(NiNode, pkAVObject);
			//	if( pkNode )
			//	{
			//		pkNode->DetachChild(pkFurniture->GetNIFRoot());
			//		if(m_kWallRoot)
			//		{
			//			m_kWallRoot->AttachChild(pkFurniture->GetNIFRoot());
			//		}
			//	}
			//}
		}
	}
	else if( m_pkArrCells )
	{
		int iEnd = static_cast<int>(m_kTotalCellCount.x * m_kTotalCellCount.y);
		for( int j = 0; j < iEnd; ++j )
		{
			if( m_pkArrCells[j].m_pkOccupiedObject == pkFurniture )
			{
				m_pkArrCells[j].m_pkOccupiedObject = NULL;
				m_pkArrCells[j].m_bOccupied = false;
			}
		}
	}
	return true;
}

void PgWall::ClearAllCells()
{
	if( m_pkArrCells )
	{
		for( int ii = 0; ii < m_kTotalCellCount.x + m_kTotalCellCount.y; ++ii )
		{
			m_pkArrCells[ii].Clear();
		}

		delete[] m_pkArrCells;
		m_pkArrCells = NULL;
	}

	if( m_pkCellsBound1 )
	{
		for( int ii = 0; ii < m_iCellsBound1Count; ++ii )
		{
			m_pkCellsBound1[ii].Clear();
		}

		delete[] m_pkCellsBound1;
		m_pkCellsBound1 = NULL;
	}

	if( m_pkCellsBound2 )
	{
		for( int ii = 0; ii < m_iCellsBound2Count; ++ii )
		{
			m_pkCellsBound2[ii].Clear();
		}

		delete[] m_pkCellsBound2;
		m_pkCellsBound2 = NULL;
	}
}

bool PgWall::CheckCellsBound(PgFurniture* pkFurniture)
{
	if( NULL == pkFurniture )
	{
		return false;
	}

	bool bUse1 = true;
	NiPoint3 kNormal;
	switch(m_eWallType)
	{
	case WALL_TYPE_FLOOR:
		kNormal = NiPoint3::UNIT_Z;
		break;
	case WALL_TYPE_WALL:
		{
			int iSide = whichSide(pkFurniture->GetWorldTransform().m_Translate);
			if (iSide == WALL_LEFT)
			{
				kNormal = m_kWallRoot->GetWorldTransform().m_Rotate * NiPoint3::UNIT_X;
			}
			else if (iSide == WALL_RIGHT)
			{
				kNormal = m_kWallRoot->GetWorldTransform().m_Rotate * NiPoint3::UNIT_Y;
				bUse1 = false;
			}
			else
			{
				return false;
			}
		}
		
		break;
	}

	NiPoint3 kStartPt;
	NiPoint3 kRayDir = -kNormal;
	
	NiPick kPick;
	kPick.SetFrontOnly(true);
	kPick.SetIntersectType(NiPick::TRIANGLE_INTERSECT);
	kPick.SetReturnNormal(true);
	kPick.SetPickType(NiPick::FIND_ALL);
	kPick.SetObserveAppCullFlag(false);
	kPick.SetSortType(NiPick::SORT);		
	kPick.SetTarget(pkFurniture);
	
	NiPick::Record *pkRec = NULL;

	if( bUse1 )
	{
		for( int ii = 0; ii < m_iCellsBound1Count; ++ii )
		{
			kStartPt = m_pkCellsBound1[ii].m_spCellTile->GetWorldTransform().m_Translate + (kNormal * 200);
			if(kPick.PickObjects(kStartPt, kRayDir))
			{
				return false;
			}
		}
	}
	else
	{
		for( int ii = 0; ii < m_iCellsBound2Count; ++ii )
		{
			kStartPt = m_pkCellsBound2[ii].m_spCellTile->GetWorldTransform().m_Translate + (kNormal * 200);
			if(kPick.PickObjects(kStartPt, kRayDir))
			{
				return false;
			}
		}
	}

	return true;
}

bool PgWall::CheckOtherFurniture(PgFurniture* pkFurniture)
{
	if( NULL == pkFurniture )
	{
		return false;
	}

	int iStart = 0;
	int iEnd = static_cast<int>(m_kTotalCellCount.x * m_kTotalCellCount.y);
	NiPoint3 kNormal;
	switch(m_eWallType)
	{
	case WALL_TYPE_FLOOR:
		kNormal = NiPoint3::UNIT_Z;
		break;
	case WALL_TYPE_WALL:
		{
			int iSide = whichSide(pkFurniture->GetWorldTransform().m_Translate);
			if (iSide == WALL_LEFT)
			{
				kNormal = m_kWallRoot->GetWorldTransform().m_Rotate * NiPoint3::UNIT_X;
				iEnd = iEnd / 2 - 1;
			}
			else if (iSide == WALL_RIGHT)
			{
				kNormal = m_kWallRoot->GetWorldTransform().m_Rotate * NiPoint3::UNIT_Y;
				iStart = iEnd / 2 - 1;
			}
			else
			{
				return false;
			}
		}
		
		break;
	}

	NiPoint3 kStartPt;
	NiPoint3 kRayDir = -kNormal;
	
	NiPick kPick;
	kPick.SetFrontOnly(true);
	kPick.SetIntersectType(NiPick::TRIANGLE_INTERSECT);
	kPick.SetReturnNormal(true);
	kPick.SetPickType(NiPick::FIND_ALL);
	kPick.SetObserveAppCullFlag(false);
	kPick.SetSortType(NiPick::SORT);		
	kPick.SetTarget(pkFurniture);
	
	NiPick::Record *pkRec = NULL;

	bool bPicked = false;
	for( int j = iStart; j < iEnd; ++j )
	{
		kStartPt = m_pkArrCells[j].m_spCellTile->GetWorldTransform().m_Translate + (kNormal * 200);

		if(kPick.PickObjects(kStartPt, kRayDir))
		{
			bPicked = true;
			if( m_pkArrCells[j].m_bOccupied )
			{
				return false;
			}
			
		}
	}

	if( !bPicked && m_kWallRoot)// 물체가 작아서 픽킹이 안될경우
	{
		int iPos = 0;
		if( FindUnderCellofFurniture(pkFurniture, kRayDir, iStart, iEnd, iPos) )
		{
			if( m_pkArrCells[iPos].m_bOccupied )
			{
				return false;
			}
		}
		else
		{
			return false;
		}
	}

	return true;
}

PgFurniture* PgWall::CheckParentFurniture(PgFurniture* pkFurniture, NiPoint3 const& kPos, int& iDummyPos)
{
	int iStart = 0;
	int iEnd = static_cast<int>(m_kTotalCellCount.x * m_kTotalCellCount.y);
	NiPoint3 kNormal= NiPoint3::UNIT_Z;

	NiPoint3 kStartPt;
	NiPoint3 kRayDir = -kNormal;
	
	NiPick kPick;
	kPick.SetFrontOnly(true);
	kPick.SetIntersectType(NiPick::TRIANGLE_INTERSECT);
	kPick.SetReturnNormal(true);
	kPick.SetPickType(NiPick::FIND_ALL);
	kPick.SetObserveAppCullFlag(false);
	kPick.SetSortType(NiPick::SORT);		
	kPick.SetTarget(m_kWallRoot);
	
	NiPick::Record *pkRec = NULL;

	kStartPt = kPos + (kNormal * 200);
	if( kPick.PickObjects(kStartPt, kRayDir) )
	{
		PgFurniture* pkPickedFurniture = NULL;
		NiPick::Results &rkRes = kPick.GetResults();
		for (unsigned int i = 0; i < rkRes.GetEffectiveSize(); i++)
		{
			pkRec = rkRes.GetAt(i);
			if (pkRec)
			{
				NiAVObject* pkPicked = pkRec->GetAVObject();
				NiAVObject* pkCellMesh = NULL;
				if (PgFurniture::IsFurnitureMesh(pkPicked, pkPickedFurniture))
				{
					if( pkFurniture != pkPickedFurniture )
					{
						FurnitureDummyPos kDummyPos;
						if( GetCanUseDummyPos(pkPickedFurniture, kDummyPos) )
						{
							float fLength = 0;
							FurnitureDummyPos::iterator itr = kDummyPos.begin();
							for(; itr!=kDummyPos.end(); ++itr )
							{
								NiPoint3 kPoint = kPos - itr->second;
								float fTemp = kPoint.SqrLength();
								if( fLength )
								{
									if( fTemp < fLength )
									{
										iDummyPos = itr->first;
										fLength = fTemp;
									}
								}
								else
								{
									iDummyPos = itr->first;
									fLength = fTemp;
								}
							}
							return pkPickedFurniture;
						}
					}
				}
			}
		}
	}

	return NULL;
}

PgFurniture* PgWall::GetFurniture(BM::GUID const& kGuid)
{
	FurnitureContainer::iterator kIter = m_kWallAttachments.begin();
	while(kIter != m_kWallAttachments.end())
	{
		PgFurniture* pkFurniture = *(kIter);
		if( pkFurniture->GetGuid() == kGuid )
		{
			return pkFurniture;
		}
		++kIter;
	}

	return NULL;
}

bool PgWall::LocateObjectAttachFurniture(PgFurniture* pkFurniture)
{
	if( !pkFurniture->GetParentFurniture() )
	{
		return false;
	}

	PgFurniture* pkParent = GetFurniture(pkFurniture->GetParentFurniture()->GetGuid());
	if( !pkParent )
	{
		return false;
	}
	
	NiAVObject* pkAVObject = GetDummyNode(pkParent, pkFurniture->GetParentIndex());
	if( !pkAVObject )
	{
		return false;
	}
	NiNode* pkNode = NiDynamicCast(NiNode, pkAVObject);
	if( !pkNode )
	{
		return false;
	}

	if( pkFurniture->GetParent() )
	{
		pkFurniture->GetParent()->DetachChild(pkFurniture->GetNIFRoot());
	}

	NiTransform kTrn;
	kTrn.MakeIdentity();
	NiQuaternion kRot = NiQuaternion::IDENTITY;
	NiQuaternion kNormalQuat(NiACos(NiPoint3::UNIT_Z.Dot(NiPoint3::UNIT_Z)), NiPoint3::UNIT_Z.UnitCross(NiPoint3::UNIT_Z));
	NiQuaternion kRotateQuat(pkFurniture->GetRotation() * ROTATION_ANGLE, NiPoint3::UNIT_Z);
	kRot = kNormalQuat * kRotateQuat;
	kRot.ToRotation(kTrn.m_Rotate);
	pkFurniture->SetLocalFromWorldTransform(kTrn);
	//pkFurniture->SetTranslate(NiPoint3::ZERO);

	pkNode->AttachChild(pkFurniture->GetNIFRoot());
	m_kWallRoot->UpdateNodeBound();
	m_kWallRoot->UpdateProperties();
	m_kWallRoot->UpdateEffects();
    NewWare::Scene::ApplyTraversal::Geometry::SetDefaultMaterialNeedsUpdateFlag( m_kWallRoot, false );
	m_kWallRoot->Update(0.0f);
	return true;
}

bool PgWall::GetChildList(PgFurniture* pkFurniture, FurnitureContainer& contList)
{
	bool bRtn = false;
	FurnitureContainer::iterator kIter = m_kWallAttachments.begin();
	while(kIter != m_kWallAttachments.end())
	{
		PgFurniture* pkFur = *(kIter);
		if( pkFur )
		{
			if( pkFur->GetParentFurniture() == pkFurniture )
			{
				contList.push_back(pkFur);
				bRtn = true;
			}
		}
		++kIter;
	}

	return bRtn;
}

bool PgWall::GetCanUseDummyPos(PgFurniture* pkFurniture, FurnitureDummyPos& contPos)
{
	if( NULL == pkFurniture )
	{
		return false;
	}
	bool bLoop = true;
	int iCount = 1;

	while(bLoop)
	{
		NiAVObject* pkAVObject = GetDummyNode(pkFurniture, iCount);
		if( pkAVObject )
		{
			NiNode* pkNode = NiDynamicCast(NiNode, pkAVObject);
			if( !pkNode->GetChildCount() )
			{
				NiPoint3 kPos = pkNode->GetWorldTransform().m_Translate;
				contPos.insert(std::make_pair(iCount, kPos));
			}
		}
		else
		{
			bLoop = false;
		}
		++iCount;
	}

	return !contPos.empty();
}

NiAVObject* PgWall::GetDummyNode(PgFurniture* pkFurniture, int iIndex )
{
	BM::vstring kPosNode(FURNITURE_DUMMY);
	kPosNode+=iIndex;
	return pkFurniture->GetObjectByName(MB(kPosNode));
}

bool PgWall::IsHaveSetPropPlace()
{
	FurnitureContainer::iterator kIter = m_kWallAttachments.begin();
	while(kIter != m_kWallAttachments.end())
	{
		PgFurniture* pkFur = *(kIter);
		if( pkFur )
		{
			FurnitureDummyPos contPos;
			if( GetCanUseDummyPos(pkFur, contPos) )
			{
				return true;
			}
		}
		++kIter;
	}

	return false;
}

bool PgWall::FindUnderCellofFurniture(PgFurniture* pkFurniture, NiPoint3 kRayDir, int iStart, int iEnd, int& iPos)
{
	if( !pkFurniture )
	{
		return false;
	}

	if( g_pkWorld && g_pkWorld->IsHaveAttr(GATTR_HOMETOWN) )// 홈타운 일때
	{
		NiPoint3 pkFurPos = pkFurniture->GetTranslate();
		if( pkFurPos.y > 0 )// 마당 아래쪽만 배치 할수 있다.
		{
			return false;
		}
	}

	NiPoint3 kNormal = -kRayDir;
	NiPick::Record *pkRec = NULL;

	NiPick kPick;
	kPick.SetFrontOnly(true);
	kPick.SetIntersectType(NiPick::TRIANGLE_INTERSECT);
	kPick.SetReturnNormal(true);
	kPick.SetPickType(NiPick::FIND_ALL);
	kPick.SetObserveAppCullFlag(false);
	kPick.SetSortType(NiPick::SORT);		
	kPick.SetTarget( m_kWallRoot );
	NiPoint3 kStartPt = pkFurniture->GetWorldTransform().m_Translate + (kNormal * 200);
	if(kPick.PickObjects(kStartPt, kRayDir))
	{
		NiPick::Results &rkRes = kPick.GetResults();
		for (unsigned int i = 0; i < rkRes.GetEffectiveSize(); i++)
		{
			pkRec = rkRes.GetAt(i);
			if (pkRec)
			{
				NiAVObject* pkPicked = pkRec->GetAVObject();
				NiAVObject* pkCellMesh = NULL;
				if (IsCellMesh(pkPicked, pkCellMesh))
				{
					float fLength = 0;
					int iFind = -1;
					NiPoint3 kFurPos = pkFurniture->GetWorldTransform().m_Translate;
					for( int ii = iStart; ii < iEnd; ++ii )
					{
						if( m_pkArrCells[ii].m_spBaseCell == pkCellMesh )
						{
							NiPoint3 kCellPos = m_pkArrCells[ii].m_spCellTile->GetWorldTransform().m_Translate;
							NiPoint3 kPos = kFurPos - kCellPos;
							float fCurLength = kPos.SqrLength();
							if( fLength )
							{
								if( fLength > fCurLength )
								{
									fLength = fCurLength;
									iFind = ii; 
								}
							}
							else
							{
								fLength = fCurLength;
								iFind = ii; 
							}
						}
					}
					if( iFind > -1 )
					{
						iPos = iFind;
						return true;
					}
					break;
				}
			}
		}
	}

	return false;
}

void PgWall::ReleaseCellsOccupancy()
{
	if( m_pkArrCells )
	{
		int iEnd = static_cast<int>(m_kTotalCellCount.x * m_kTotalCellCount.y);
		for( int j = 0; j < iEnd; ++j )
		{
			m_pkArrCells[j].m_pkOccupiedObject = NULL;
			m_pkArrCells[j].m_bOccupied = false;
		}
	}
}
