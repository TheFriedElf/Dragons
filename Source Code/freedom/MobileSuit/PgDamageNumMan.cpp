#include "stdafx.h"
#include "PgDamageNumMan.h"
#include "XUI/XUI_Font.h"
#include "PgRenderer.h"
#include "PgNifMan.h"
#include "PgMobileSuit.H"
#include "PgWorld.H"
#include "PgCameraMan.H"
#include "PgMath.H"
#include "PgActor.h"

#include "NewWare/Scene/ApplyTraversal.h"

NiImplementRTTI(PgDamageNumMan, PgIWorldObject);

int const MAX_NUM_DAMAGE = 7;
int const MAX_NUM_DAMAGE_FOR_ARRAY = 20;
bool PgDamageNumMan::m_sbInitExpVertexSetting = false;

////////////////////////////////////////////////////////////////////////////////////////////////////
//	class	PgDamageNumMan
////////////////////////////////////////////////////////////////////////////////////////////////////
void	PgDamageNumMan::Init()
{
	m_fAccumTimeSaved = 0.0f;
	m_bIsRedColor = true;	
	
	{// 스마트 포인터들이 들고 있는 리소스들을 제거하고 (NULL을 넣음으로서 연결 해제 또는 자동 삭제됨 구문 지우면 안됨) 
		m_spRedNum = NULL;
		m_spRedNumSmall = NULL;
		m_spYellowNum = NULL;
		m_spSkillTextTex = NULL;
		m_spExpTex = NULL;
		m_spHpTex = NULL;
		m_spMpTex = NULL;
		m_spCriticalTex = NULL;
		m_spWhiteNum2 = NULL;
		m_spKillCountTex = NULL;
		m_spMissionTex = NULL;
		m_spCriticalSubTex1 = NULL;
		m_spCriticalSubTex2 = NULL;
	}
	
	if(NULL == m_spRedNum)			{ m_spRedNum = g_kNifMan.GetTexture("../Data/6_UI/main/mnNumDmg.dds"); }
	if(NULL == m_spRedNumSmall)	{ m_spRedNumSmall = g_kNifMan.GetTexture("../Data/6_UI/main/mnNumDmgR.dds"); }
	if(NULL == m_spYellowNum)		{ m_spYellowNum = g_kNifMan.GetTexture("../Data/6_UI/main/mnNumDmgG.dds"); }
	if(NULL == m_spExpTex)			{ m_spExpTex = g_kNifMan.GetTexture("../Data/6_UI/main/mnNumExp.dds"); }
	if(NULL == m_spHpTex)			{ m_spHpTex = g_kNifMan.GetTexture("../Data/6_UI/main/mnNumHp.dds"); }
	if(NULL == m_spMpTex)			{ m_spMpTex = g_kNifMan.GetTexture("../Data/6_UI/main/mnNumMp.dds"); }
	if(NULL == m_spCriticalTex)	{ m_spCriticalTex = g_kNifMan.GetTexture("../Data/6_UI/main/mnNumDmgCr.dds"); }
	if(NULL == m_spWhiteNum2)		{ m_spWhiteNum2 = g_kNifMan.GetTexture("../Data/6_UI/main/mnNumDmgW.dds"); }
	if(NULL == m_spGreenTex)		{ m_spGreenTex = g_kNifMan.GetTexture("../Data/6_UI/main/mnNumDmgP.dds"); }
	if(NULL == m_spKillCountTex)	{ m_spKillCountTex = g_kNifMan.GetTexture( "../Data/6_UI/pvp/pvKillfont.tga"); }
	if(NULL == m_spMissionTex)		{ m_spMissionTex = g_kNifMan.GetTexture( "../Data/6_UI/mission/msFont.tga"); }
	if(NULL == m_spCriticalSubTex1)		{ m_spCriticalSubTex1 = g_kNifMan.GetTexture("../Data/6_UI/main/mnCrBg.dds"); }
	if(NULL == m_spCriticalSubTex2)		{ m_spCriticalSubTex2 = g_kNifMan.GetTexture("../Data/6_UI/main/mnCrFt.dds"); }
	
	{//Exp 글씨쪽 Vertex를 변경 해 주는 작업을 하고
		NiNodePtr spExpNumNif = g_kNifMan.GetNif("../Data/6_ui/Combo/DamageNum_02.nif");
		NiAVObject	*pNifNode = spExpNumNif;
		SetExpNumber((NiNode*)pNifNode,0, true);	
	}

	{// 미리 한번 읽은 후
		g_kNifMan.GetNif("../Data/6_ui/Combo/DamageNum_01.nif");
		g_kNifMan.GetNif("../Data/6_ui/Combo/DamageSmallNum_01.nif");
		g_kNifMan.GetNif("../Data/6_ui/Combo/DamageNum_03.nif");
		g_kNifMan.GetNif("../Data/6_ui/Combo/DamageNumCr_01.nif");
	}
	{
		lua_State *pkState = *lua_wrapper_user(g_kLuaTinker);
		m_kCont_ScaleTime.resize(stDamageNumNode::T_MAX);
		for(size_t i=0; i < m_kCont_ScaleTime.size(); ++i)
		{
			float& rfVal = m_kCont_ScaleTime.at(i);
			switch(i)
			{
			case stDamageNumNode::T_NUM:
				{
					rfVal = lua_tinker::get<float>(pkState, "g_fDmgScaleTime_NUM");
				}break;
			case stDamageNumNode::T_CRITICAL:
				{
					rfVal = lua_tinker::get<float>(pkState, "g_fDmgScaleTime_CRITICAL");
				}break;
			default:
				{
					rfVal = lua_tinker::get<float>(pkState, "g_fDmgNumScaleTime");
				}break;
			}
		}

		m_kCont_MoveTime.resize(stDamageNumNode::T_MAX);
		for(size_t i=0; i < m_kCont_ScaleTime.size(); ++i)
		{
			float& rfVal = m_kCont_MoveTime.at(i);
			switch(i)
			{
			case stDamageNumNode::T_NUM:
				{
					rfVal = lua_tinker::get<float>(pkState,"g_fDmgNumMoveTimeScale_NUM");
				}break;
			case stDamageNumNode::T_CRITICAL:
				{
					rfVal = lua_tinker::get<float>(pkState,"g_fDmgNumMoveTimeScale_CRITICAL");
				}break;
			default:
				{
					rfVal = lua_tinker::get<float>(pkState,"g_fDmgNumMoveTime");					
				}break;
			}
		}
	}

	// 연출에 사용하는 값들을 script로 부터 얻어 온다.
	
	m_fStartScale = lua_tinker::call<float>("GetDmgNumInitialScale");
	m_fTargetScale = lua_tinker::call<float>("GetDmgNumScale");
	m_fHoldingTime = lua_tinker::call<float>("GetDmgNumHoldingTime");	
	m_fMoveSpeed = lua_tinker::call<float>("GetDmgNumMoveSpeed");
	m_fMoveTargetScale = lua_tinker::call<float>("GetDmgNumMoveTargetScale");

}
void	PgDamageNumMan::Destroy()
{
	DeleteAllNum();

	m_spSkillTextTex = NULL;
	m_spRedNum = NULL;
	m_spRedNumSmall = NULL;
	m_spYellowNum = NULL;
	m_spExpTex = NULL;
	m_spHpTex = NULL;
	m_spMpTex = NULL;
	m_spCriticalTex = NULL;
	m_spWhiteNum2 = NULL;
	m_spGreenTex = NULL;
	m_spKillCountTex = NULL;
	m_spMissionTex = NULL;
	m_spCriticalSubTex1 = NULL;
	m_spCriticalSubTex2 = NULL;
	
}
void	PgDamageNumMan::DeleteAllNum()
{
	for(CONT_NODE::iterator itr = m_NodeList.begin(); itr != m_NodeList.end(); ++itr)
	{
		SAFE_DELETE_NI(*itr);
	}
	m_NodeList.clear();

}

//! WorldObject를 fFrameTime(AccumTime)에 대한 시각으로 갱신
bool PgDamageNumMan::Update(float fAccumTime, float fFrameTime)
{
	m_fAccumTimeSaved = fAccumTime;

	stDamageNumNode *pNode = NULL;
	
	for(CONT_NODE::iterator itr = m_NodeList.begin(); itr != m_NodeList.end(); )
	{
		pNode = (*itr);

		switch(pNode->m_byState)
		{
		case stDamageNumNode::ES_START:	//	m_fStartScale -> m_fTargetScale 로 스케일링 되면서, Alpha 가 0 -> 1  로 바뀐다.
			{
				float	fElapsedTime = fAccumTime - pNode->m_fCreateTime;
				
				float	fRate = fElapsedTime / pNode->m_fScaleTime;
				if(1.0f < fRate)
				{
					fRate = 1.0f; 
				}
				if(0.0f > fRate)
				{
					fRate = 0.0f; 
				}
				if(stDamageNumNode::T_NUM != pNode->m_Type
					&& stDamageNumNode::T_CRITICAL!= pNode->m_Type
					)
				{
					float	fScale = m_fStartScale * (1.0f - fRate) + m_fTargetScale * fRate;

					NiPoint3 pt3Pos = pNode->m_vStartPos;
					switch ( pNode->m_Type )
					{
					case stDamageNumNode::T_EXP_NUM:
						{
							fScale *= 0.4f;
						}break;
					case stDamageNumNode::T_HP_OR_MP:
						{
							fScale *= 0.35f;
						}break;
					case stDamageNumNode::T_SMALL_NUM:
						{
							fScale *= 0.5f;
						}break;
					case stDamageNumNode::T_KILL_COUNT_TEXT:
						{
							fRate *= 2.8f;
							if(1.0f < fRate)
							{
								fRate = 1.0f;
							}

							NiPoint3 vec = pNode->m_vTargetPos - pNode->m_vStartPos;
							vec *= fRate;
							pt3Pos = pNode->m_vStartPos + vec;
							fScale = 0.5f;
						}break;
					case stDamageNumNode::T_ENCHANT_LEVEL:
						{
							fScale *= 1.5f;
						}break;
					}

					if(fScale<0.0f) fScale = 0.0f;
					pNode->m_spNif->SetScale(fScale);
					pNode->m_spNif->SetTranslate(pt3Pos);
				}

				SetAlpha(pNode->m_spNif,fRate);				
				pNode->m_spNif->Update(fAccumTime);

				if( fRate == 1.0f ) 
				{
					pNode->m_fHoldingStartTime = fAccumTime;
					++pNode->m_byState;
				}
			}
			break;
		case stDamageNumNode::ES_STOP:	//	잠시 멈춰있는 시간.
			{
				float	fElapsedTime = fAccumTime - pNode->m_fHoldingStartTime;
				if ( stDamageNumNode::T_KILL_COUNT_TEXT == pNode->m_Type )
				{ 
					if ( fElapsedTime > 2.3f )
					{
						pNode->m_fMoveStartTime = fAccumTime;
						++pNode->m_byState;

						NiPoint3 vec = pNode->m_vTargetPos - pNode->m_vStartPos;
						pNode->m_vStartPos = pNode->m_vTargetPos;
						pNode->m_vTargetPos = pNode->m_vStartPos + vec;
					}
				}
				else
				{
					if(fElapsedTime>=m_fHoldingTime)
					{
						pNode->m_fMoveStartTime = fAccumTime;
						++pNode->m_byState;
					}
				}
			}
			break;
		case stDamageNumNode::ES_END:
			{
				float	fElapsedTime = fAccumTime - pNode->m_fMoveStartTime;
				float	fRate = fElapsedTime/ pNode->m_fMoveTime;
				if(fRate>1.0f) fRate = 1.0f;
				if(fRate<0.0f) fRate = 0.0f;

				if(stDamageNumNode::T_NUM != pNode->m_Type
					&& stDamageNumNode::T_CRITICAL!= pNode->m_Type
					)
				{
					float	fScale = m_fTargetScale*(1.0f-fRate)+m_fMoveTargetScale*fRate;

					float	fMoveDistance = m_fMoveSpeed*fElapsedTime;
					NiPoint3 kNewPos = pNode->m_vStartPos;
					kNewPos.z += fMoveDistance;

					// 타입에 따라 최대 크기를 조절해 줄 필요가 있음.
					switch ( pNode->m_Type )
					{
					case stDamageNumNode::T_EXP_NUM:
						{
							fScale *= 0.4f;
						}break;
					case stDamageNumNode::T_HP_OR_MP:
						{
							fScale *= 0.35f;
						}break;
					case stDamageNumNode::T_SMALL_NUM:
						{
							fScale *= 0.5f;
						}break;
					case stDamageNumNode::T_KILL_COUNT_TEXT:
						{
							fRate *= 2.8f;
							if(fRate>1.0f) fRate = 1.0f;

							NiPoint3 vec = pNode->m_vTargetPos - pNode->m_vStartPos;
							vec *= fRate;
							kNewPos = pNode->m_vStartPos + vec;
							fScale = 0.5f;
						}break;
					}
					pNode->m_spNif->SetScale(fScale);
					pNode->m_spNif->SetTranslate(kNewPos);
				}
				SetAlpha( pNode->m_spNif, 1.0f - fRate );
				
				pNode->m_spNif->Update(fAccumTime);

				if(fRate == 1.0f)
				{
					SAFE_DELETE_NI(pNode);
					itr = m_NodeList.erase(itr);
					continue;
				}
			}
			break;
		}

		if(pNode->m_bClampScreen)
		{
			ClampScreen(pNode);
		}

		++itr;
	}
	return	true;
}
void	PgDamageNumMan::ClampScreen(stDamageNumNode *pNode)
{
	if(!g_pkWorld) 
	{
		return;
	}

	PgCameraMan	*pkCameraMan = g_pkWorld->GetCameraMan();
	if(!pkCameraMan)
	{
		return;
	}

	if(!pNode)
	{
		return;
	}

	NiAVObjectPtr	spNifNode = pNode->m_spNif;
	if(!spNifNode)
	{
		return;
	}

	NiCameraPtr	spCamera = pkCameraMan->GetCamera();
	if(NULL == spCamera)
	{
		return;
	}

	NiPoint3	kCurrentPos = spNifNode->GetTranslate();
	float const fBuffer = 20.0f;

	kCurrentPos.z+=fBuffer;

	//	뷰 프러스텀을 구하자.
	NiFrustumPlanes const &rkFP = PgRenderer::GetPgRenderer()->GetFrustumPlanes();

	NiPlane	const& rkTopPlane = rkFP.GetPlane(NiFrustumPlanes::TOP_PLANE);

	if(rkTopPlane.WhichSide(kCurrentPos) == NiPlane::NEGATIVE_SIDE)
	{
		//	화면 위쪽에 있는것이다. 밑으로 끌어내리자
		NiPoint3 const kLineP1 = kCurrentPos;
		NiPoint3 const kLineP2 = kCurrentPos+NiPoint3(0.0f,0.0f,-100.0f);
		NiPoint3 const kPlaneNormal = rkTopPlane.GetNormal();
		NiPoint3 const kPlaneP3 = kPlaneNormal*rkTopPlane.GetConstant();

		NiPoint3 const kCollPos = GetCollPos_LinePlane(
			kLineP1,kLineP2,kPlaneP3,kPlaneNormal);

		if(kCollPos != NiPoint3(-1.0f,-1.0f,-1.0f))
		{
			float const fDistance = (kCollPos - kCurrentPos).Length();

			kCurrentPos += NiPoint3(0.0f,0.0f,-(fDistance+fBuffer));
			spNifNode->SetTranslate(kCurrentPos);
			spNifNode->Update(0.0f);
		}
	}

}
void	PgDamageNumMan::SetAlpha(NiAVObject *pkObject,float fAlpha)
{
	if(NiIsKindOf(NiNode,pkObject))
	{
		NiNode	*pkNode = NiDynamicCast(NiNode,pkObject);
		int	iArrayCount = pkNode->GetArrayCount();
		NiAVObject	*pkChild = NULL;
		for(int i=0;i<iArrayCount;++i)
		{
			pkChild = pkNode->GetAt(i);
			if(pkChild)
			{
				SetAlpha(pkChild,fAlpha);
			}
		}
	}
	else if(NiIsKindOf(NiGeometry,pkObject))
	{
		NiGeometry	*pkGeom = NiDynamicCast(NiGeometry,pkObject);
		NiMaterialProperty	*pkProperty = pkGeom->GetPropertyState()->GetMaterial();
		if(pkProperty)
		{
			pkProperty->SetAlpha(fAlpha);
		}
	}
}

//! pkRenderer를 이용해서 Draw
void PgDamageNumMan::Draw(PgRenderer *pkRenderer, NiCamera *pkCamera, float fFrameTime)
{
	if(NULL == pkRenderer)
	{
		return;
	}
	NiDX9Renderer *pDX9Renderer = static_cast<NiDX9Renderer*>( pkRenderer->GetRenderer() );
	if(NULL == pDX9Renderer)
	{
		return;
	}
	LPDIRECT3DDEVICE9 pkDevice = pDX9Renderer->GetD3DDevice();
	if(NULL == pkDevice)
	{
		return;
	}
	pkDevice->SetRenderState(D3DRS_ZENABLE,false);
	
	for ( CONT_NODE::iterator itr = m_NodeList.begin(); itr != m_NodeList.end(); ++itr )
	{
		pkRenderer->PartialRenderClick_Deprecated((*itr)->m_spNif);
	}
	pkDevice->SetRenderState(D3DRS_ZENABLE,true);
}

void	PgDamageNumMan::AddNewSimpleText(int iTextType,NiPoint3 const &vLoc)
{
	if (!g_pkWorld)
	{
		return;
	}
	//	iTextType
	//	0 : Miss
	//	1 : Dodge
	//	2 : Block
	

	static float const fTextLoc[]=
	{
		0,0,128,32,
		0,32,128,32,
		0,64,128,32,
	};

	if(NULL == m_spSimpleTextTex)
	{
		m_spSimpleTextTex = g_kNifMan.GetTexture("../Data/6_UI/main/TMiss2.dds");
	}

	NiNodePtr	spTextNode = g_kNifMan.GetNif_DeepCopy("../Data/6_UI/SimpleTT.nif");
	if(NULL == spTextNode)
	{
		return;
	}

	NiGeometry	*pkTextGeom = (NiGeometry*)spTextNode->GetObjectByName("Plane");
	pkTextGeom->GetModelData()->SetConsistency(NiGeometryData::VOLATILE);

	NiTexturingProperty	*pkProperty = pkTextGeom->GetPropertyState()->GetTexturing();
	pkProperty->SetBaseTexture(m_spSimpleTextTex);
	
	static float const fTexWidth = 128.0f;
	static float const fTexHeight = 128.0f;
	static float const fBoxWidth = 200.0f;
	static float const fBoxHeight = fBoxWidth*0.25f;//(32.0f/128.0f);
	
	NiPoint2	*pkUV = pkTextGeom->GetModelData()->GetTextures();
	NiPoint3	*pkVertex = pkTextGeom->GetModelData()->GetVertices();

	pkVertex->x = -fBoxWidth/2; pkVertex->y = fBoxHeight; ++pkVertex;
	pkVertex->x = -fBoxWidth/2; pkVertex->y = 0; ++pkVertex;
	pkVertex->x = fBoxWidth/2; pkVertex->y = fBoxHeight; ++pkVertex;
	pkVertex->x = fBoxWidth/2; pkVertex->y = 0; ++pkVertex;

	pkUV->x = fTextLoc[iTextType*4+0]/fTexWidth;	pkUV->y = fTextLoc[iTextType*4+1]/fTexHeight;	++pkUV;
	pkUV->x = fTextLoc[iTextType*4+0]/fTexWidth;	pkUV->y = (fTextLoc[iTextType*4+1]+fTextLoc[iTextType*4+3])/fTexHeight;	++pkUV;
	pkUV->x = (fTextLoc[iTextType*4+0]+fTextLoc[iTextType*4+2])/fTexWidth;	pkUV->y = fTextLoc[iTextType*4+1]/fTexHeight;	++pkUV;
	pkUV->x = (fTextLoc[iTextType*4+0]+fTextLoc[iTextType*4+2])/fTexWidth;	pkUV->y = (fTextLoc[iTextType*4+1]+fTextLoc[iTextType*4+3])/fTexHeight;	

	pkTextGeom->GetModelData()->MarkAsChanged(NiGeometryData::TEXTURE_MASK | NiGeometryData::VERTEX_MASK);

	AddDamageNumNode( stDamageNumNode::T_SIMPLE_TEXT, spTextNode, 0, vLoc, g_pkWorld->GetAccumTime(), false, false, vLoc );
}

void PgDamageNumMan::AddNewMissionText(int iTextType, NiPoint3 const &vLoc)
{
	if (!g_pkWorld)
	{
		return;	
	}

	static float const fTextLoc[]=
	{
		0,0,256,64,
		0,64,256,64,
		0,128,256,64,
		0,192,256,64,
	};

	if(NULL == m_spMissionTex)
	{
		m_spMissionTex = g_kNifMan.GetTexture("../Data/6_UI/mission/msFont.tga");
	}

	NiNodePtr	spTextNode = g_kNifMan.GetNif_DeepCopy("../Data/6_UI/billboard_quad.nif");
	if(NULL == spTextNode)
	{
		return;
	}

	NiGeometry	*pkTextGeom = (NiGeometry*)spTextNode->GetObjectByName("Plane");
	pkTextGeom->GetModelData()->SetConsistency(NiGeometryData::VOLATILE);

	NiTexturingProperty	*pkProperty = pkTextGeom->GetPropertyState()->GetTexturing();
	pkProperty->SetBaseTexture(m_spMissionTex);
	

	static float const fTexWidth = 256.0f;
	static float const fTexHeight = 256.0f;
	static float const fBoxWidth = 200.0f;
	static float const fBoxHeight = fBoxWidth*0.25f;//static_cast<float>(fBoxWidth*(64.0/256.0));
	
	NiPoint2	*pkUV = pkTextGeom->GetModelData()->GetTextures();
	NiPoint3	*pkVertex = pkTextGeom->GetModelData()->GetVertices();

	pkVertex->x = -fBoxWidth/2; pkVertex->y = fBoxHeight; ++pkVertex;
	pkVertex->x = -fBoxWidth/2; pkVertex->y = 0; ++pkVertex;
	pkVertex->x = fBoxWidth/2; pkVertex->y = fBoxHeight; ++pkVertex;
	pkVertex->x = fBoxWidth/2; pkVertex->y = 0; ++pkVertex;

	pkUV->x = fTextLoc[iTextType*4+0]/fTexWidth;	pkUV->y = fTextLoc[iTextType*4+1]/fTexHeight;	++pkUV;
	pkUV->x = fTextLoc[iTextType*4+0]/fTexWidth;	pkUV->y = (fTextLoc[iTextType*4+1]+fTextLoc[iTextType*4+3])/fTexHeight;	++pkUV;
	pkUV->x = (fTextLoc[iTextType*4+0]+fTextLoc[iTextType*4+2])/fTexWidth;	pkUV->y = fTextLoc[iTextType*4+1]/fTexHeight;	++pkUV;
	pkUV->x = (fTextLoc[iTextType*4+0]+fTextLoc[iTextType*4+2])/fTexWidth;	pkUV->y = (fTextLoc[iTextType*4+1]+fTextLoc[iTextType*4+3])/fTexHeight;	

	pkTextGeom->GetModelData()->MarkAsChanged(NiGeometryData::TEXTURE_MASK | NiGeometryData::VERTEX_MASK);

	AddDamageNumNode( stDamageNumNode::T_MISSION_TEXT, spTextNode, 0, vLoc, g_pkWorld->GetAccumTime(), false, false, vLoc );
}

void PgDamageNumMan::AddNewKillCountText( int const iTextType, PgActor *pkActor )
{
	if ( NULL == g_pkWorld 
		|| NULL == pkActor
		)
	{
		return;
	}
	if( NULL == m_spKillCountTex )
	{
		m_spKillCountTex = g_kNifMan.GetTexture( "../Data/6_UI/pvp/pvKillfont.tga");
	}

	if ( m_spKillCountTex )
	{
		NiNodePtr spTextNode = g_kNifMan.GetNif("../Data/6_UI/main/KiilCnt.nif");
		if ( spTextNode )
		{
			NiNode	*pkTextNode = (NiNode*)spTextNode->GetObjectByName("name");
			//				NiNode	*pkUpDownNode = (NiNode*)spTextNode->GetObjectByName("up");

			NiGeometry	*pkTextGeom = (NiGeometry*)pkTextNode->GetAt(0);
			//				NiGeometry	*pkUpDownGeom = (NiGeometry*)pkUpDownNode->GetAt(0);

			pkTextGeom->GetModelData()->SetConsistency(NiGeometryData::VOLATILE);
			//				pkUpDownGeom->GetModelData()->SetConsistency(NiGeometryData::VOLATILE);

			NiTexturingProperty	*pkProperty = pkTextGeom->GetPropertyState()->GetTexturing();
			pkProperty->SetBaseTexture(m_spKillCountTex);

			//				pkProperty = pkUpDownGeom->GetPropertyState()->GetTexturing();
			//				pkProperty->SetBaseTexture(m_spKillCountTex);

			SetKillCountText( spTextNode, iTextType );

			NiPoint3 pt3StartPos = pkActor->GetPos();
			//NiNodePtr pkDummy = (NiNode*)(pkActor->GetObjectByName(ATTACH_POINT_STAR));
			NiNodePtr pkDummy = pkActor->GetNodePointStar();
			if ( pkDummy )
			{
				pt3StartPos = pkDummy->GetWorldTranslate();
			}

			NiPoint3 pt3TargetPos = pt3StartPos;

			NiCamera *pkCamera = g_pkWorld->m_kCameraMan.GetCamera();
			if ( pkCamera )
			{
				pt3StartPos += (( pkCamera->GetWorldRightVector() * 20.0f ) + ( pkCamera->GetWorldUpVector() * 20.0f ));
				pt3TargetPos = pt3StartPos + ( pkCamera->GetWorldRightVector() * 50.0f );
			}

			AddDamageNumNode(stDamageNumNode::T_KILL_COUNT_TEXT, spTextNode, 0, pt3TargetPos, g_pkWorld->GetAccumTime(), false, false, pt3StartPos );
		}
	}
}

void	PgDamageNumMan::AddNewSkillText(int iTextType,bool bIsUP,NiPoint3 const &vLoc)
{
	if (!g_pkWorld)
	{
		return;
	}

	if(NULL == m_spSkillTextTex)
	{
		m_spSkillTextTex = g_kNifMan.GetTexture("../Data/6_UI/main/mnSkillText.dds");
	}

	NiNodePtr	spTextNode = g_kNifMan.GetNif_DeepCopy("../Data/6_UI/main/skillTextEX.nif");
	if(NULL == spTextNode)
	{
		return;
	}

	NiNode	*pkTextNode = (NiNode*)spTextNode->GetObjectByName("name");
	if(NULL == pkTextNode)
	{
		return;
	}
	NiNode	*pkUpDownNode = (NiNode*)spTextNode->GetObjectByName("up");
	if(NULL == pkUpDownNode)
	{
		return;
	}

	NiGeometry	*pkTextGeom = (NiGeometry*)pkTextNode->GetAt(0);
	if(NULL == pkTextGeom)
	{
		return;
	}
	NiGeometry	*pkUpDownGeom = (NiGeometry*)pkUpDownNode->GetAt(0);
	if(NULL == pkUpDownGeom)
	{
		return;
	}

	NiGeometryData* pkModel = pkTextGeom->GetModelData();
	if(NULL == pkModel)
	{
		return;
	}
	pkModel->SetConsistency(NiGeometryData::VOLATILE);
	
	pkModel = pkUpDownGeom->GetModelData();
	if(NULL == pkModel)
	{
		return;
	}
	pkModel->SetConsistency(NiGeometryData::VOLATILE);

	NiPropertyStatePtr spState = pkTextGeom->GetPropertyState();
	if(NULL == spState)
	{
		return;
	}

	NiTexturingProperty	*pkProperty = spState->GetTexturing();
	if(NULL == pkProperty)
	{
		return;
	}
	
	pkProperty->SetBaseTexture(m_spSkillTextTex);
	pkProperty = pkUpDownGeom->GetPropertyState()->GetTexturing();
	pkProperty->SetBaseTexture(m_spSkillTextTex);

	SetSkillText(spTextNode,iTextType,bIsUP);
	AddDamageNumNode(stDamageNumNode::T_SKILL_TEXT, spTextNode, 0, vLoc, g_pkWorld->GetAccumTime(), false, false, vLoc);
}

void	PgDamageNumMan::AddNewNum(int const iNumber,NiPoint3 const &vLoc,bool const bRedColor,bool const bClampScreen, bool const bCritical, int const iEnchantLevel, int const iExceptAbil)
{	
	if (!g_pkWorld)
	{
		return;
	}
	m_bIsRedColor = bRedColor;

	NiNodePtr	spDamageNumNif = 0;

	NiSourceTexturePtr	spEnchantLevelTex = 0;
	NiSourceTexturePtr	spExceptAbilTex = 0; // 어빌에 의해서 예외 처리가 되어야하는 폰트 연출

	if(m_spYellowNum == 0 || m_spRedNum == 0)
	{
		m_spRedNum = g_kNifMan.GetTexture("../Data/6_UI/main/mnNumDmg.dds");
		m_spYellowNum = g_kNifMan.GetTexture("../Data/6_UI/main/mnNumDmgG.dds");		
	}

	if(0 < iEnchantLevel && bRedColor)
	{
		spEnchantLevelTex = g_kNifMan.GetTexture("../Data/6_UI/main/mnNumDmgVb.tga");
	}
	else if(0 > iEnchantLevel && bRedColor)
	{
		spEnchantLevelTex = g_kNifMan.GetTexture("../Data/6_UI/main/mnNumDmg2.tga");
	}
	else if(0 > iEnchantLevel)
	{
		spEnchantLevelTex = g_kNifMan.GetTexture("../Data/6_UI/main/mnNumDmgG2.tga");
	}

	if(NULL == m_spCriticalTex)
	{
		m_spCriticalTex = g_kNifMan.GetTexture("../Data/6_UI/main/mnNumDmgCr.dds");
	}

	switch(iExceptAbil)	
	{// 예외 어빌로 특정한 데미지 폰트를 출력해주어야 하는 경우.
	case 1:
		{
			spExceptAbilTex = g_kNifMan.GetTexture("../Data/6_UI/main/mnNumDmgB.tga");
		}break;
	default:
		{
			spExceptAbilTex = 0;
		}break;
	}
	if(bCritical)
	{
		spDamageNumNif = g_kNifMan.GetNif("../Data/6_ui/Combo/DamageNumCr_01.nif");
	}
	else
	{
		spDamageNumNif = g_kNifMan.GetNif("../Data/6_ui/Combo/DamageNum_01.nif");
	}
	
	if(NULL == spDamageNumNif)
	{
		 return;
	}
	
	NiSourceTexturePtr	spTexture = m_spYellowNum;
	//크리티컬이 가장 최우선 순위
	if(bCritical)
	{
		spTexture = m_spCriticalTex;
	}
	else if(bRedColor)
	{
		if(0 != iEnchantLevel)
		{
			spTexture = spEnchantLevelTex;
		}
		else
		{
			spTexture = m_spRedNum;
		}

		//예외 처리가 우선 순위
		if(0 < iExceptAbil)
		{
			spTexture = spExceptAbilTex;
		}
	}
	else
	{
		if(0 > iEnchantLevel)
		{
			spTexture = spEnchantLevelTex;
		}
	}
	
	BM::vstring vStr;
	for(int i = 1; i <= MAX_NUM_DAMAGE; ++i)
	{
		vStr = "Plane0";
		vStr += i;
		PgDamageNumManUtil::SetBaseTexture( spDamageNumNif, vStr.operator const std::string().c_str(), spTexture );
	}
	
	stDamageNumNode::TYPE eType = stDamageNumNode::T_NUM;
	if(bCritical)
	{
		eType = stDamageNumNode::T_CRITICAL;
		PgDamageNumManUtil::SetBaseTexture( spDamageNumNif, "Plane08", m_spCriticalSubTex1 );
		PgDamageNumManUtil::SetBaseTexture( spDamageNumNif, "Plane09", m_spCriticalSubTex2 );
	}

	if(bRedColor && (0 < iEnchantLevel))
	{
		eType = stDamageNumNode::T_ENCHANT_LEVEL;
	}

	AddDamageNumNode(eType, spDamageNumNif, iNumber, vLoc, g_pkWorld->GetAccumTime(), bClampScreen, true, vLoc);
}

void PgDamageNumMan::SetKillCountText(NiNode *pNifNode, int const iTextType )
{
	static float const iTextCoord[]={
		79.0f,	10.0f,	245.0f,	55.0f,
		79.0f,	80.0f,	245.0f,	55.0f,
		79.0f,	150.0f,	245.0f,	55.0f,
		79.0f,	230.0f,	245.0f,	55.0f,
		79.0f,	286.0f,	245.0f,	55.0f,
		79.0f,	367.0f,	245.0f,	55.0f,
		79.0f,	430.0f,	245.0f,	55.0f
 	};

	static float const fTexWidth = 512.0f	,fTexHeight = 512.0f;

	NiNode	*pkTextNode = (NiNode*)pNifNode->GetObjectByName("name");
	if ( pkTextNode )
	{
		NiGeometry	*pkTextGeom = (NiGeometry*)pkTextNode->GetAt(0);
		if ( pkTextGeom )
		{
			NiGeometryData	*pkGeomData = pkTextGeom->GetModelData();
			if ( pkGeomData )
			{
				int	iVertexCount = pkGeomData->GetVertexCount();

				NiPoint2	*pkUV = pkGeomData->GetTextures();

				float x = iTextCoord[iTextType*4] / fTexWidth;
				float y = iTextCoord[iTextType*4+1] / fTexHeight;
				float w = iTextCoord[iTextType*4+2] / fTexWidth;
				float h = iTextCoord[iTextType*4+3] / fTexHeight;

				pkUV->x = x+w;
				pkUV->y = y+h;
				++pkUV;

				pkUV->x = x+w;
				pkUV->y = y;
				++pkUV;

				pkUV->x = x;
				pkUV->y = y+h;
				++pkUV;

				pkUV->x = x;
				pkUV->y = y;
				pkGeomData->MarkAsChanged(NiGeometryData::TEXTURE_MASK);
			}
		}
	}

	NiNode	*pkUpDownNode = (NiNode*)pNifNode->GetObjectByName("up");
	if ( pkUpDownNode )
	{
		pkUpDownNode->SetAppCulled( true );
	}
}

void	PgDamageNumMan::SetSkillText(NiNode *pNifNode,int iTextType,bool bIsUP)
{
	static int const	iSkillTextCoord[]={
		0,0,112,23,	// POWER //	x,y,width,height
		112,0,112,23,	//	DEF
		0,23,112,23,	//	MDFF
		112,23,112,23,	//	ATK
		0,46,112,23,	//	MATK
		112,46,112,23,	//	MOVE
		0,69,112,23,	//	MAX HP
		112,69,112,23,	//	MAX MP
		0,92,112,23,	//	CRITICAL
		112,92,112,23,	//	FLEE
		0,115,112,23,	//	ACCURY
	};

	static int const	iUpDownTextCoord[]={
		0,138,152,52,	//	UP
		0,190,152,52	//	DOWN
	};

	static float const fTexWidth = 256.0,fTexHeight = 256.0;

	NiNode	*pkTextNode = (NiNode*)pNifNode->GetObjectByName("name");
	NiNode	*pkUpDownNode = (NiNode*)pNifNode->GetObjectByName("up");

	NiGeometry	*pkTextGeom = (NiGeometry*)pkTextNode->GetAt(0);
	NiGeometry	*pkUpDownGeom = (NiGeometry*)pkUpDownNode->GetAt(0);

	NiGeometryData	*pkGeomData = pkTextGeom->GetModelData();

	int	iVertexCount = pkGeomData->GetVertexCount();

	NiPoint2	*pkUV = pkGeomData->GetTextures();
	
	float x = iSkillTextCoord[iTextType*4+0]/fTexWidth;
	float y = iSkillTextCoord[iTextType*4+1]/fTexHeight;
	float w = iSkillTextCoord[iTextType*4+2]/fTexWidth;
	float h = iSkillTextCoord[iTextType*4+3]/fTexHeight;

	pkUV->x = x+w;
	pkUV->y = y+h;
	++pkUV;

	pkUV->x = x+w;
	pkUV->y = y;
	++pkUV;

	pkUV->x = x;
	pkUV->y = y+h;
	++pkUV;

	pkUV->x = x;
	pkUV->y = y;
	pkGeomData->MarkAsChanged(NiGeometryData::TEXTURE_MASK);


	pkGeomData = pkUpDownGeom->GetModelData();
	iVertexCount = pkGeomData->GetVertexCount();
	pkUV = pkGeomData->GetTextures();

	int	iUpDown = 0;
	if(!bIsUP) iUpDown = 1;

	x = iUpDownTextCoord[iUpDown*4+0]/fTexWidth;
	y = iUpDownTextCoord[iUpDown*4+1]/fTexHeight;
	w = iUpDownTextCoord[iUpDown*4+2]/fTexWidth;
	h = iUpDownTextCoord[iUpDown*4+3]/fTexHeight;

	for(int i=0;i<iVertexCount;++i)
	{
		pkUV->x = x+w*pkUV->x;
		pkUV->y = y+h*pkUV->y;
		++pkUV;
	}
	pkGeomData->MarkAsChanged(NiGeometryData::TEXTURE_MASK);

}
void	PgDamageNumMan::SetNumber(NiNode *pNifNode,int iNumber)
{
	static int const iNumTexCoord[] = {
		0,0,50,50,	//	x,y,width,height
		50,0,50,50,
		100,0,50,50,
		150,0,50,50,
		200,0,50,50,
		0,50,50,50,
		50,50,50,50,
		100,50,50,50,
		150,50,50,50,
		200,50,50,50
	};
	NiGeometryData *pGeomData=NULL;
	NiPoint2 *pTexCoord = NULL;

	char strNum[MAX_NUM_DAMAGE_FOR_ARRAY];
	sprintf_s(strNum,MAX_NUM_DAMAGE_FOR_ARRAY,"%d",iNumber);
	int iLen = strlen(strNum);

	static const float fTexWidth = 256,fTexHeight = 128;

	BM::vstring vStr;
	NiGeometry *pGeom[MAX_NUM_DAMAGE] = { NULL, };
	for(int i = 0; i < MAX_NUM_DAMAGE; ++i)
	{
		vStr = "Plane0";
		vStr += i+1;
		pGeom[i] = (NiGeometry*)pNifNode->GetObjectByName(vStr.operator const std::string().c_str());	//	숫자 첫째 자리
		pGeom[i]->SetAppCulled(true);
	}

	int iVertexCount = 4;
	
	float	tx,ty,tw,th;
	char chNum;

	if(iLen>MAX_NUM_DAMAGE) { iLen = MAX_NUM_DAMAGE; }

	int iCount=0;

	if(5 == iLen || 4 == iLen)		{ iCount = 1;	}
	else if(3 == iLen || 2 == iLen) { iCount = 2;	}
	else if(1 == iLen)				{ iCount = 3;	}
	
	int iLen2 = iCount + iLen;
	
	for(int i = iCount ;i<iLen2;++i)
	{
		chNum = (*(strNum+i-iCount))-'0';

		pGeomData = pGeom[i]->GetModelData();
		pTexCoord = pGeomData->GetTextures();

		tx = static_cast<float>(iNumTexCoord[(chNum*4+0)]);
		ty = static_cast<float>(iNumTexCoord[(chNum*4+1)]);
		tw = static_cast<float>(iNumTexCoord[(chNum*4+2)]);
		th = static_cast<float>(iNumTexCoord[(chNum*4+3)]);

		pTexCoord->x = (tx+tw)/fTexWidth;
		pTexCoord->y = (ty+th)/fTexHeight;

		++pTexCoord;

		pTexCoord->x = (tx+tw)/fTexWidth;
		pTexCoord->y = ty/fTexHeight;

		++pTexCoord;

		pTexCoord->x = tx/fTexWidth;
		pTexCoord->y = (ty+th)/fTexHeight;

		++pTexCoord;

		pTexCoord->x = tx/fTexWidth;
		pTexCoord->y = ty/fTexHeight;

		pGeom[i]->SetAppCulled(false);
		pGeomData->MarkAsChanged(NiGeometryData::TEXTURE_MASK);
	}
}

void PgDamageNumMan::AddNewExpNum(int iNumber,NiPoint3 const &vLoc)
{
	if(!g_pkWorld)
	{
		return;
	}

	if(m_spExpTex == 0) { m_spExpTex = g_kNifMan.GetTexture("../Data/6_UI/main/mnNumExp.dds"); }

	NiNodePtr	spExpNumNif = 0;
	spExpNumNif = g_kNifMan.GetNif("../Data/6_ui/Combo/DamageNum_02.nif");
	if(spExpNumNif == 0) { return; }

	NiNode *pDummy = (NiNode*)spExpNumNif->GetObjectByName("Dummy01");
	
	NiSourceTexturePtr	spTexture = m_spExpTex;
	
	BM::vstring vStr;
	for(int i = 1; i <= MAX_NUM_DAMAGE; ++i)
	{
		vStr = "Plane0";
		vStr += i;
		PgDamageNumManUtil::SetBaseTexture( spExpNumNif, vStr.operator const std::string().c_str(), spTexture );
	}

	BM::vstring vNum(iNumber);
	int iLen = vNum.size();
	if(iLen>MAX_NUM_DAMAGE) { iLen = MAX_NUM_DAMAGE; }

	AddDamageNumNode(stDamageNumNode::T_EXP_NUM, spExpNumNif, iNumber, vLoc, g_pkWorld->GetAccumTime(), true, true, vLoc);
}

void	PgDamageNumMan::SetExpNumber(NiNode *pNifNode,int iNumber, bool bExp)
{
	static int const iNumTexCoord[] = {
		0,4,21,21,	//	x,y,width,height 0
		20,4,21,21, // 1
		39,4,21,21, // 2
		59,4,21,21, // 3
		79,4,21,21, // 4
		99,4,21,21, // 5
		0,34,21,21,  // 6
		20,34,21,21, // 7
		39,34,21,21, // 8
		59,34,21,21, // 9
		79,34,49,21, // + exp
		0,65,42,36, // x
		94,34,34,21, // exp

	};

	if(NULL==pNifNode)
	{
		return;
	}

	NiGeometryData *pGeomData;
	NiPoint2 *pTexCoord;

	char strNum[MAX_NUM_DAMAGE_FOR_ARRAY];
	sprintf_s(strNum,MAX_NUM_DAMAGE_FOR_ARRAY,"%d",iNumber);
	int iLen = strlen(strNum);

	static const float fTexWidth = 128,fTexHeight = 100;
	
	BM::vstring vStr;
	NiGeometry *pGeom[MAX_NUM_DAMAGE] = { NULL, };
	for(int i = 0; i < MAX_NUM_DAMAGE; ++i)
	{
		vStr = "Plane0";
		vStr += i+1;
		pGeom[i] = (NiGeometry*)pNifNode->GetObjectByName(vStr.operator const std::string().c_str());	//	숫자 첫째 자리
		pGeom[i]->SetAppCulled(true);
	}

	int iVertexCount = 4;
	
	float tx = 0.0f;
	float ty = 0.0f;
	float tw = 0.0f;
	float th = 0.0f;
	char chNum = 0;

	if(iLen>6) { iLen = 6; }

	int iCount=1;

	if(2 == iLen || 1 == iLen) { iCount = 2; }
	
	int iLen2 = iCount + iLen;
	
	for(int i = iCount ;i<iLen2;++i)
	{
		chNum = (*(strNum+i-iCount))-'0';
		if(0 == iNumber)
		{
			chNum = 11;
		}

		pGeomData = pGeom[i]->GetModelData();
		pTexCoord = pGeomData->GetTextures();

		tx = static_cast<float>(iNumTexCoord[(chNum*4+0)]);
		ty = static_cast<float>(iNumTexCoord[(chNum*4+1)]);
		tw = static_cast<float>(iNumTexCoord[(chNum*4+2)]);
		th = static_cast<float>(iNumTexCoord[(chNum*4+3)]);

		pTexCoord->x = (tx+tw)/fTexWidth;
		pTexCoord->y = (ty+th)/fTexHeight;

		++pTexCoord;

		pTexCoord->x = (tx+tw)/fTexWidth;
		pTexCoord->y = ty/fTexHeight;

		++pTexCoord;

		pTexCoord->x = tx/fTexWidth;
		pTexCoord->y = (ty+th)/fTexHeight;

		++pTexCoord;

		pTexCoord->x = tx/fTexWidth;
		pTexCoord->y = ty/fTexHeight;

		pGeom[i]->SetAppCulled(false);
		pGeomData->MarkAsChanged(NiGeometryData::TEXTURE_MASK);
	}

	pGeom[0]->SetAppCulled(false);

//	if(bExp)
	{
		//! 단한번만 초기화 한다.
//		if(m_sbInitExpVertexSetting) { return; }
//		m_sbInitExpVertexSetting = true;

		pGeomData = pGeom[0]->GetModelData();
		pTexCoord = pGeomData->GetTextures();
		NiPoint3* pkVex1 =  pGeomData->GetVertices();
		NiPoint3* pkVex2 =  pGeomData->GetVertices() + 1;
		NiPoint3* pkVex3 =  pGeomData->GetVertices() + 2;
		NiPoint3* pkVex4 =  pGeomData->GetVertices() + 3;

		if(0 == iNumber)
		{
			pkVex1->x = 4.5f;
			pkVex2->x = 4.5f;
			pkVex3->x = -110.0f;
			pkVex4->x = -110.0f;
		}
		else
		{
			pkVex1->x = 4.5f;
			pkVex2->x = 4.5f;
			pkVex3->x =	-122.5f;
			pkVex4->x = -122.5f;
		}
		
		int iIndex = 10;
		if(0 == iNumber)
		{
			iIndex = 12;
		}

		tx = static_cast<float>(iNumTexCoord[(iIndex*4+0)]);
		ty = static_cast<float>(iNumTexCoord[(iIndex*4+1)]);
		tw = static_cast<float>(iNumTexCoord[(iIndex*4+2)]);
		th = static_cast<float>(iNumTexCoord[(iIndex*4+3)]);

		pTexCoord->x = (tx+tw)/fTexWidth;
		pTexCoord->y = (ty+th)/fTexHeight;

		++pTexCoord;

		pTexCoord->x = (tx+tw)/fTexWidth;
		pTexCoord->y = ty/fTexHeight;

		++pTexCoord;

		pTexCoord->x = tx/fTexWidth;
		pTexCoord->y = (ty+th)/fTexHeight;

		++pTexCoord;

		pTexCoord->x = tx/fTexWidth;
		pTexCoord->y = ty/fTexHeight;

		pGeom[0]->SetAppCulled(false);
		pGeomData->MarkAsChanged(NiGeometryData::TEXTURE_MASK | NiGeometryData::VERTEX_MASK);
	}
}

void	PgDamageNumMan::AddNewHpMp(int iNumber,NiPoint3 const &vLoc, bool bHp)
{
	if(!g_pkWorld) { return; }

	if(m_spHpTex == 0) { m_spHpTex = g_kNifMan.GetTexture("../Data/6_UI/main/mnNumHp.dds"); }
	if(m_spMpTex == 0) { m_spMpTex = g_kNifMan.GetTexture("../Data/6_UI/main/mnNumMp.dds"); }

	NiNodePtr	spHpMpNif = 0;
	spHpMpNif = g_kNifMan.GetNif("../Data/6_ui/Combo/DamageNum_03.nif");
	if(spHpMpNif == 0) { return; }

	NiNode *pDummy = (NiNode*)spHpMpNif->GetObjectByName("Dummy01");
	
	NiSourceTexturePtr	spTexture = NULL;
	if(bHp) { spTexture = m_spHpTex; }
	else	{ spTexture = m_spMpTex; }
	
	BM::vstring vStr;
	for(int i = 1; i <= MAX_NUM_DAMAGE; ++i)
	{
		vStr = "Plane0";
		vStr += i;
		PgDamageNumManUtil::SetBaseTexture( spHpMpNif, vStr.operator const std::string().c_str(), spTexture );
	}


	BM::vstring vNum(iNumber);
	int iLen = vNum.size();
	if(iLen>MAX_NUM_DAMAGE) { iLen = MAX_NUM_DAMAGE; }

	AddDamageNumNode(stDamageNumNode::T_HP_OR_MP, spHpMpNif, iNumber, vLoc, g_pkWorld->GetAccumTime(), true, true, vLoc);
}

void	PgDamageNumMan::SetHpMpNumber(NiNode *pNifNode,int iNumber)
{
	static int const iNumTexCoord[] = {
		0,4,18,22,	//	x,y,width,height 0
		21,4,18,22, // 1
		42,4,18,22, // 2
		61,4,18,22, // 3
		81,4,18,22, // 4
		101,4,18,22, // 5
		0,34,18,22,  // 6
		21,34,18,22, // 7
		42,34,18,22, // 8
		61,34,18,22, // 9
		81,34,18,22, // +
	};

	NiGeometryData *pGeomData;
	NiPoint2 *pTexCoord;

	char strNum[MAX_NUM_DAMAGE_FOR_ARRAY];
	sprintf_s(strNum,MAX_NUM_DAMAGE_FOR_ARRAY,"%d",iNumber);
	int iLen = strlen(strNum);

	static const float fTexWidth = 128.0f,fTexHeight = 64.0f;

	BM::vstring vStr;
	NiGeometry *pGeom[MAX_NUM_DAMAGE] = { NULL, };
	for(int i = 0; i < MAX_NUM_DAMAGE; ++i)
	{
		vStr = "Plane0";
		vStr += i+1;
		pGeom[i] = (NiGeometry*)pNifNode->GetObjectByName(vStr.operator const std::string().c_str());	//	숫자 첫째 자리
		pGeom[i]->SetAppCulled(true);
	}

	int iVertexCount = 4;
	
	float	tx,ty,tw,th;
	char chNum;

	if(iLen>6) { iLen = 6; }

	int iCount=1;
	if(5 <= iLen)		{ iCount = 1; }
	else if(4 <= iLen)	{ iCount = 2; }
	else if(1 <= iLen)	{ iCount = 3; }

	int iLen2 = iCount + iLen;
	if(iLen2>7) 
	{
		iLen2 = 6;
		iCount = 1;
		iNumber = 999999; //최대값을 세팅
	}
	
	for(int i = iCount ;i<iLen2;++i)
	{
		chNum = (*(strNum+i-iCount))-'0';

		pGeomData = pGeom[i]->GetModelData();
		pTexCoord = pGeomData->GetTextures();

		tx = static_cast<float>(iNumTexCoord[(chNum*4+0)]);
		ty = static_cast<float>(iNumTexCoord[(chNum*4+1)]);
		tw = static_cast<float>(iNumTexCoord[(chNum*4+2)]);
		th = static_cast<float>(iNumTexCoord[(chNum*4+3)]);

		pTexCoord->x = (tx+tw)/fTexWidth;
		pTexCoord->y = (ty+th)/fTexHeight;

		++pTexCoord;

		pTexCoord->x = (tx+tw)/fTexWidth;
		pTexCoord->y = ty/fTexHeight;

		++pTexCoord;

		pTexCoord->x = tx/fTexWidth;
		pTexCoord->y = (ty+th)/fTexHeight;

		++pTexCoord;

		pTexCoord->x = tx/fTexWidth;
		pTexCoord->y = ty/fTexHeight;

		pGeom[i]->SetAppCulled(false);
		pGeomData->MarkAsChanged(NiGeometryData::TEXTURE_MASK);
	}

	if(iCount < 1) { iCount = 1; }
	// + 표시
	pGeomData = pGeom[iCount - 1]->GetModelData();
	pTexCoord = pGeomData->GetTextures();

	tx = static_cast<float>(iNumTexCoord[(10*4+0)]);
	ty = static_cast<float>(iNumTexCoord[(10*4+1)]);
	tw = static_cast<float>(iNumTexCoord[(10*4+2)]);
	th = static_cast<float>(iNumTexCoord[(10*4+3)]);

	pTexCoord->x = (tx+tw)/fTexWidth;
	pTexCoord->y = (ty+th)/fTexHeight;

	++pTexCoord;

	pTexCoord->x = (tx+tw)/fTexWidth;
	pTexCoord->y = ty/fTexHeight;

	++pTexCoord;

	pTexCoord->x = tx/fTexWidth;
	pTexCoord->y = (ty+th)/fTexHeight;

	++pTexCoord;

	pTexCoord->x = tx/fTexWidth;
	pTexCoord->y = ty/fTexHeight;

	pGeom[iCount - 1]->SetAppCulled(false);
	pGeomData->MarkAsChanged(NiGeometryData::TEXTURE_MASK);
}
void	PgDamageNumMan::SetCriticalNumber(NiNode *pNifNode,int iNumber)
{
	static int const iNumTexCoord[] = {
		0,0,54,54,	//	x,y,width,height
		49,0,54,54,
		98,0,54,54,
		146,0,54,54,
		200,0,54,54,
		0,54,54,54,
		49,54,54,54,
		98,54,54,54,
		146,54,54,54,
		200,54,54,54
	};
	static const float fTexWidth = 256.0f,fTexHeight = 128.0f;

	NiGeometryData *pGeomData;
	NiPoint2 *pTexCoord;

	char strNum[MAX_NUM_DAMAGE_FOR_ARRAY];
	sprintf_s(strNum,MAX_NUM_DAMAGE_FOR_ARRAY,"%d",iNumber);
	int iLen = strlen(strNum);
	
	BM::vstring vStr;
	NiGeometry *pGeom[MAX_NUM_DAMAGE] = { NULL, };
	for(int i = 0; i < MAX_NUM_DAMAGE; ++i)
	{
		vStr = "Plane0";
		vStr += i+1;
		pGeom[i] = (NiGeometry*)pNifNode->GetObjectByName(vStr.operator const std::string().c_str());	//	숫자 첫째 자리
		pGeom[i]->SetAppCulled(true);
	}

	int iVertexCount = 4;
	
	float	tx,ty,tw,th;
	char chNum;

	if(iLen>MAX_NUM_DAMAGE) { iLen = MAX_NUM_DAMAGE; }
	
	int iCount=0;
	if(5 == iLen || 4 == iLen)		{ iCount = 1;}
	else if(3 == iLen || 2 == iLen) { iCount = 2;}
	else if(1 == iLen)				{ iCount = 3;}
	
	int iLen2 = iCount + iLen;
	if(iLen2 > MAX_NUM_DAMAGE)
	{
		iLen2 = MAX_NUM_DAMAGE;
		iCount = 0;
	}
	
	for(int i = iCount ;i<iLen2;++i)
	{
		chNum = (*(strNum+i-iCount))-'0';

		pGeomData = pGeom[i]->GetModelData();
		pTexCoord = pGeomData->GetTextures();

		tx = static_cast<float>(iNumTexCoord[(chNum*4+0)]);
		ty = static_cast<float>(iNumTexCoord[(chNum*4+1)]);
		tw = static_cast<float>(iNumTexCoord[(chNum*4+2)]);
		th = static_cast<float>(iNumTexCoord[(chNum*4+3)]);

		pTexCoord->x = (tx+tw)/fTexWidth;
		pTexCoord->y = (ty+th)/fTexHeight;

		++pTexCoord;

		pTexCoord->x = (tx+tw)/fTexWidth;
		pTexCoord->y = ty/fTexHeight;

		++pTexCoord;

		pTexCoord->x = tx/fTexWidth;
		pTexCoord->y = (ty+th)/fTexHeight;

		++pTexCoord;

		pTexCoord->x = tx/fTexWidth;
		pTexCoord->y = ty/fTexHeight;

		pGeom[i]->SetAppCulled(false);
		pGeomData->MarkAsChanged(NiGeometryData::TEXTURE_MASK);
	}
}

void	PgDamageNumMan::AddNewSmallNum(int const iNumber,NiPoint3 const &vLoc,bool const bClampScreen, bool const bCritical, BYTE btColor, int const iEnchantLevel, int const iExceptAbil)
{
	if(!g_pkWorld)
	{
		return;
	}

	//타유저가 때릴때 크리티컬은 따로 구분되어 있지 않음/ 추후 작업이 쉽게 하기 위해 bCritical 변수만 남겨둠
	NiNodePtr	spDamageNumNif = 0;
	if(m_spWhiteNum2 == 0)	{ m_spWhiteNum2 = g_kNifMan.GetTexture("../Data/6_UI/main/mnNumDmgW.dds"); }
	if(m_spGreenTex == 0)	{ m_spGreenTex = g_kNifMan.GetTexture("../Data/6_UI/main/mnNumDmgP.dds"); }
	if(m_spRedNumSmall == 0)	{ m_spRedNumSmall = g_kNifMan.GetTexture("../Data/6_UI/main/mnNumDmgR.dds"); }
	spDamageNumNif = g_kNifMan.GetNif("../Data/6_ui/Combo/DamageSmallNum_01.nif");
	
	if(spDamageNumNif == 0) { return; }

	NiNode *pDummy = (NiNode*)spDamageNumNif->GetObjectByName("Dummy01");
	
	NiSourceTexturePtr	spTexture = NULL;
	if(C_WHITE == btColor)		{ spTexture = m_spWhiteNum2; }
	else if(C_GREEN == btColor) { spTexture = m_spGreenTex; }
	else if(C_RED == btColor )
	{
		spTexture = m_spRedNumSmall;
	}

	NiSourceTexturePtr	spEnchantLevelTex = 0;
	if(0 < iEnchantLevel)
	{
		spEnchantLevelTex = g_kNifMan.GetTexture("../Data/6_UI/main/mnNumDmgVb.tga");
	}
	else if(0 > iEnchantLevel)
	{
		spEnchantLevelTex = g_kNifMan.GetTexture("../Data/6_UI/main/mnNumDmg2.tga");
	}

	NiSourceTexturePtr	spExceptAbilTex = 0; // 어빌에 의해서 예외 처리가 되어야하는 폰트 연출
	// 예외 어빌로 특정한 데미지 폰트를 출력해주어야 하는 경우.
	switch(iExceptAbil)	
	{
	case 1:
		{
			spExceptAbilTex = g_kNifMan.GetTexture("../Data/6_UI/main/mnNumDmgB.tga");
		}break;
	default:
		{
			spExceptAbilTex = 0;
		}break;
	}

	stDamageNumNode::TYPE eType = stDamageNumNode::T_SMALL_NUM;

	if(0 < iExceptAbil)
	{
		eType = stDamageNumNode::T_NUM;
		spTexture = spExceptAbilTex;
	}
	
	BM::vstring vStr;
	for(int i = 1; i <= MAX_NUM_DAMAGE; ++i)
	{
		vStr = "Plane0";
		vStr += i;
		PgDamageNumManUtil::SetBaseTexture( spDamageNumNif, vStr.operator const std::string().c_str(), spTexture );
	}

	BM::vstring vNum(iNumber);
	int iLen = vNum.size();
	if(iLen>MAX_NUM_DAMAGE) { iLen = MAX_NUM_DAMAGE; }
	
	AddDamageNumNode(eType, spDamageNumNif, iNumber, vLoc, g_pkWorld->GetAccumTime(), bClampScreen, true, vLoc);
}

void	PgDamageNumMan::SetSmallNumber(NiNode *pNifNode,int iNumber)
{
	static int const iNumTexCoord[] = {
		0,0,39,39,	//	x,y,width,height
		42,0,39,39,
		81,0,39,39,
		120,0,39,39,
		159,0,39,39,
		0,39,39,39,
		42,39,39,39,
		81,39,39,39,
		120,39,39,39,
		159,39,39,39
	};
	NiGeometryData *pGeomData = NULL;
	NiPoint2 *pTexCoord = NULL;

	char strNum[MAX_NUM_DAMAGE_FOR_ARRAY]= {};
	sprintf_s(strNum,MAX_NUM_DAMAGE_FOR_ARRAY,"%d",iNumber);
	int iLen = strlen(strNum);

	static const float fTexWidth = 256.0f,fTexHeight = 128.0f;
	
	BM::vstring vStr;
	NiGeometry *pGeom[MAX_NUM_DAMAGE] = { NULL, };
	for(int i = 0; i < MAX_NUM_DAMAGE; ++i)
	{
		vStr = "Plane0";
		vStr += i+1;
		pGeom[i] = (NiGeometry*)pNifNode->GetObjectByName(vStr.operator const std::string().c_str());	//	숫자 첫째 자리
		pGeom[i]->SetAppCulled(true);
	}

	int iVertexCount = 4;
	
	float tx=0.0f;
	float ty=0.0f;
	float tw=0.0f;
	float th=0.0f;
	char chNum = 0;

	if(iLen>MAX_NUM_DAMAGE) { iLen = MAX_NUM_DAMAGE; }

	int iCount=0;

	if(5 == iLen || 4 == iLen)		{ iCount = 1; }
	else if(3 == iLen || 2 == iLen) { iCount = 2; }
	else if(1 == iLen)				{ iCount = 3; }
	
	int iLen2 = iCount + iLen;
	if(iLen2 > MAX_NUM_DAMAGE)
	{
		iLen2 = MAX_NUM_DAMAGE;
		iCount = 0;
	}
	
	for(int i = iCount ;i<iLen2;i++)
	{
		chNum = (*(strNum+i-iCount))-'0';

		pGeomData = pGeom[i]->GetModelData();
		pTexCoord = pGeomData->GetTextures();

		tx = static_cast<float>(iNumTexCoord[(chNum*4+0)]);
		ty = static_cast<float>(iNumTexCoord[(chNum*4+1)]);
		tw = static_cast<float>(iNumTexCoord[(chNum*4+2)]);
		th = static_cast<float>(iNumTexCoord[(chNum*4+3)]);

		pTexCoord->x = (tx+tw)/fTexWidth;
		pTexCoord->y = (ty+th)/fTexHeight;

		++pTexCoord;

		pTexCoord->x = (tx+tw)/fTexWidth;
		pTexCoord->y = ty/fTexHeight;

		++pTexCoord;

		pTexCoord->x = tx/fTexWidth;
		pTexCoord->y = (ty+th)/fTexHeight;

		++pTexCoord;

		pTexCoord->x = tx/fTexWidth;
		pTexCoord->y = ty/fTexHeight;

		pGeom[i]->SetAppCulled(false);
		pGeomData->MarkAsChanged(NiGeometryData::TEXTURE_MASK);
	}
}

void PgDamageNumMan::AddDamageNumNode(stDamageNumNode::TYPE eType, NiNodePtr pkNode, int iNumber, NiPoint3 vStartPos, float fCreateTime, bool bClampScreen, bool bNifUpdate, NiPoint3 const &vTargetPos )
{
	stDamageNumNode	*pNewNode = NiNew stDamageNumNode();
	if(!pNewNode)
	{
		return;
	}

	pNewNode->m_Type = eType;
	pNewNode->m_spNif = pkNode;
	pNewNode->m_iNumber = iNumber;
	pNewNode->m_vStartPos = vStartPos;
	pNewNode->m_fCreateTime = fCreateTime;
	pNewNode->m_bClampScreen = bClampScreen;
	pNewNode->m_vTargetPos = vTargetPos;
	
	if(false == m_NodeList.empty())
	{//숫자가 동시에 뜰 경우 겹치는 것을 방지하기 위해
		CONT_NODE::reverse_iterator kItor = m_NodeList.rbegin();
		if(m_NodeList.rend() != kItor)
		{// 마지막에 생성된 노드와
			stDamageNumNode* pkNode = (*kItor);
			if( pkNode
				&& pkNode->m_Type == pNewNode->m_Type				// 타입이 같고
				&& pkNode->m_vStartPos == pNewNode->m_vStartPos		// 같은 위치이면서
				&& pkNode->m_fCreateTime >= pNewNode->m_fCreateTime	// 새로운 노드가 기존노드와 생성시간이 같거나 더 빠르면
				)
			{
				pNewNode->m_fCreateTime = pkNode->m_fCreateTime + 0.15f;	// 생성 시간을 조정 해주고
			}
		}
	}

	switch(eType)
	{
	case stDamageNumNode::T_NUM:
	case stDamageNumNode::T_CRITICAL:
		{
			pNewNode->m_spNif->SetScale(1.0f);
			pNewNode->m_spNif->SetTranslate(vStartPos);
		}break;	
	}
	pNewNode->m_fScaleTime = GetScaleTime(eType);
	pNewNode->m_fMoveTime = GetMoveTime(eType);
		
	{// 사용할 텍스쳐 UV등 설정
		NiAVObject	*pNifNode = pNewNode->m_spNif;
		switch(pNewNode->m_Type)
		{
		case stDamageNumNode::T_NUM:
		case stDamageNumNode::T_ENCHANT_LEVEL:
			{
				SetNumber((NiNode*)pNifNode,pNewNode->m_iNumber);
			}break;
		case stDamageNumNode::T_EXP_NUM:
			{
				SetExpNumber((NiNode*)pNifNode,pNewNode->m_iNumber);
			}break;
		case stDamageNumNode::T_HP_OR_MP:
			{
				SetHpMpNumber((NiNode*)pNifNode,pNewNode->m_iNumber);
			}break;
		case stDamageNumNode::T_CRITICAL:
			{
				SetCriticalNumber((NiNode*)pNifNode,pNewNode->m_iNumber);
			}break;
		case stDamageNumNode::T_SMALL_NUM:
			{
				SetSmallNumber((NiNode*)pNifNode,pNewNode->m_iNumber);
			}break;
		}
	}

	m_NodeList.push_back(pNewNode);

	if(bNifUpdate)
	{
		pkNode->UpdateProperties();
		NiTimeController::StartAnimations(pkNode, 0.0f);
        NewWare::Scene::ApplyTraversal::Geometry::SetDefaultMaterialNeedsUpdateFlag( pkNode, false );
	}
}

namespace PgDamageNumManUtil
{
	bool SetBaseTexture(NiNodePtr spNumNif, char const* const pcNodeName, NiSourceTexturePtr spTexture)
	{
		if(!spNumNif
			|| !pcNodeName
			|| !spTexture)
		{
			return false;
		}
		NiGeometry *pkGeom = static_cast<NiGeometry*>( spNumNif->GetObjectByName(pcNodeName) ); //	숫자 첫째 자리
		if(!pkGeom)
		{
			return false;
		}
		NiGeometryData* pkData = pkGeom->GetModelData();
		if(!pkData)
		{
			return false;
		}
		pkData->SetConsistency(NiGeometryData::VOLATILE);
		NiPropertyStatePtr spState = pkGeom->GetPropertyState();
		if(!spState)
		{
			return false;
		}
		NiTexturingProperty	*pkProperty = spState->GetTexturing();
		if(!pkProperty)
		{
			return false;
		}
		pkProperty->SetBaseTexture( spTexture );
		return true;
	}
}
