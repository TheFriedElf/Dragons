#include "stdafx.h"
#include "PgItemEx.h"
#include "PgXmlLoader.h"
#include "PgAMPool.h"
#include "ServerLib.h"
#include "PgNifMan.h"
#include "PgBowString.h"
#include "PgWorkerThread.h"
#include "PgItemMan.h"
#include "variant/unit.H"
#include "PgParticle.H"
#include "PgParticleMan.H"
#include "PgActorManager.H"
#include "PgPilot.H"
#include "PgActor.H"
#include "PgStatusEffect.H"
#include "PgRenderer.H"

#include "NewWare/Renderer/DrawWorkflow.h"
#include "NewWare/Renderer/Kernel/RenderStateTagExtraData.h"
#include "NewWare/Scene/ApplyTraversal.h"


char const * const pcPtHair = "p_pt_hair";
char const * const pcPtHead = "Bip01 Head";
char const * const pcPtChest = "p_pt_chest";
char const * const pcPtLeftHand = "p_wp_l_hand";
char const * const pcPtRightHand = "p_wp_r_hand";
char const * const pcPtLeftForearm = "Bip01 L Forearm";
char const * const pcPtRightForearm = "Bip01 R Forearm";
char const * const pcPtShield = "p_wp_shield";
char const * const pcCharRoot = "char_root";
char const * const pcPtLeftHand2 = "p_pt_l_hand";
char const * const pcPtRightHand2 = "p_pt_r_hand";

char const * const pcHairNodePropertyID = "PG_HAIR_NODE";

NiImplementRTTI(PgItemEx, NiAVObject);

TexturePair::TexturePair(char const *kSrcTexture) : 
	m_kSrcTexture(kSrcTexture),
	m_pkDestTexture(NULL)
{
}

TexturePair::TexturePair(char const *kSrcTexture, char const *kDestTexture) :
	m_kSrcTexture(kSrcTexture), 
	m_kDestTexture(kDestTexture),
	m_pkDestTexture(NULL)
{
}

bool TexturePair::operator ==(const TexturePair &rhs)
{
	return m_kSrcTexture == rhs.m_kSrcTexture;
}

bool TexturePair::ApplyTexture(NiNode *pkRoot, TexturePathContainer& rkTexturePathCont, bool bCheckNoChange)
{
	if(!pkRoot)
	{
		return false;
	}

	if (rkTexturePathCont.size() <= 0)
	{
		return true;
	}

	unsigned int uiArrayCount = pkRoot->GetArrayCount();

	for (unsigned int i = 0; i < uiArrayCount; i++)
	{
		NiAVObject* pkChild = pkRoot->GetAt(i);

		if(!pkChild)
		{
			continue;
		}
		
		if(bCheckNoChange)
		{
			NiStringExtraData *pkExtra = NiDynamicCast(NiStringExtraData, pkChild->GetExtraData("UserPropBuffer"));
			if(pkExtra)
			{
				// Geometry에 NoColorChange라고 UserProp에 적어뒀으면, 그에 해당하는 Material은 리스트에 추가하지 않는다.
				NiFixedString kUserDefined = pkExtra->GetValue();
				if(kUserDefined.ContainsNoCase("NoColorChange"))
				{
					continue;
				}
			}
		}

		if(NiIsKindOf(NiGeometry, pkChild))
		{
			NiGeometry *pkGeometry = (NiGeometry*)pkChild;

			// 기존의 TextureProperty를 가져온다.
			NiTexturingProperty *pkTextureProp = NiDynamicCast(NiTexturingProperty, pkGeometry->GetProperty(NiTexturingProperty::GetType()));
			
			if (!pkTextureProp || pkTextureProp->GetBaseMap() == NULL)
			{
				continue;
			}

			// BaseMap의 텍스쳐를 가져온다.
			NiTexture *pkTex = pkTextureProp->GetBaseMap()->GetTexture();
			NiSourceTexture *pkSrcTex = NiDynamicCast(NiSourceTexture, pkTex);

			if (pkSrcTex == NULL)
			{
				continue;
			}

			// BaseMap 텍스쳐 파일 이름을 알아온다.
			const NiFixedString& rkFileName = pkSrcTex->GetFilename();
			if (rkFileName.GetLength() <= 0)
			{
				continue;
			}

			char const *pcSrcTexture = strrchr((char const*)rkFileName, '\\');
			pcSrcTexture = (pcSrcTexture ? pcSrcTexture + 1 : pkSrcTex->GetFilename());
			//std::string kSrcTexture(pcSrcTexture);

			// 기존에 붙어 있는 텍스쳐가 변경해야 하는 텍스쳐 리스트에 있는가?
			NiString kSrcTexture(pcSrcTexture);
			if (kSrcTexture.Length() == 0)
			{
				continue;
			}
			kSrcTexture.ToLower();
			TexturePair kTexturePair(kSrcTexture);
			TexturePathContainer::iterator itr = std::find(rkTexturePathCont.begin(), rkTexturePathCont.end(), kTexturePair);

			if(rkTexturePathCont.end() == itr)
			{
				continue;
			}
			
			if (itr->m_pkDestTexture == NULL)
				itr->m_pkDestTexture = g_kNifMan.GetTexture(itr->m_kDestTexture);
			NiSourceTexture *pkNewTex = itr->m_pkDestTexture;

			if(!pkNewTex)
			{
				NILOG(PGLOG_ERROR, "[PgItemEx] CreateTexutre(%s) Failed\n", itr->m_kDestTexture.c_str());
				PG_ASSERT_LOG(!"Can't create New Texture : Invalid Path!");
				continue;
			}

			// 새로운 텍스쳐를 설정한다.
			// TODO : 기존의 텍스쳐를 버리고 그냥 Set해도 Memory Leak이 없는가? 체크하자.
			//pkSrcTex->DestroyAppPixelData();			
			if (pkGeometry->GetModelData())
			{
				pkTextureProp->GetBaseMap()->SetTexture(pkNewTex);
				pkGeometry->GetModelData()->MarkAsChanged(NiGeometryData::TEXTURE_MASK);
				//NiDX9Renderer::GetRenderer()->PurgeGeometryData(pkGeometry->GetModelData());				
			}
		}
		else if(NiIsKindOf(NiNode, pkChild))
		{
			ApplyTexture((NiNode *)pkChild, rkTexturePathCont, bCheckNoChange);
		}
	}

	return true;
}

int PgItemEx::m_iStatusEffectIDCount = 0;

int PgItemEx::GenerateStatusEffectID()
{
	//int의 최대치를 넘어서면 0으로 만들어준다.
	if(0 > m_iStatusEffectIDCount)
	{
		m_iStatusEffectIDCount = 0;
	}

	return m_iStatusEffectIDCount++;
}

PgItemEx::PgItemEx() :
	m_spMeshRoot(0),
	m_spOriginalMeshRoot(0),
	m_iItemType(0),
	m_iUserGender(0),
	m_iItemFilter(0),
	m_iWeaponType(0),
	m_bWeapon(0),
	m_spAM(0),
	m_pkItemDef(0),
	m_iMeshType(1),
	m_pkAdditionalParts(0),
	m_pcTargetPoint(0),
	m_kBowStringColor(1.0f, 1.0f, 1.0f, 1.0f)
{
	m_eItemLimit = EQUIP_LIMIT_NONE;
	m_eWeaponAttachPos = PgItemEx::UNKNOWN;
	m_kTrailInfo.SetDefault();
	m_kDamageEffNormal = "e_dmg";	//디폴트
	m_kDamageEffCritical = "e_dmg_cri";
}

PgItemEx::~PgItemEx()
{
	m_spAM = 0;
	m_spMeshRoot = 0;
	m_spOriginalMeshRoot = 0;

	if(m_pkAdditionalParts)
	{
		SAFE_DELETE_NI(m_pkAdditionalParts);
	}
	m_kAttachedObjectList.clear();
}

eEquipLimit PgItemEx::EquipLimit()const
{
	return m_eItemLimit;
}

bool PgItemEx::IsWeapon()const
{
	return m_bWeapon;
}

int PgItemEx::GetMeshType()const
{
	return m_iMeshType;
}

PgItemEx::WeaponAttachPos PgItemEx::GetWeaponAttachPos()
{
	return m_eWeaponAttachPos;
}

void PgItemEx::SetUserGender(int const iGender)
{
	m_iUserGender = iGender;
}

void PgItemEx::SetUserClass(int const iClassNo)
{
	m_iUserClass = iClassNo;
}

NiNodePtr PgItemEx::GetMeshRoot()
{
	return m_spMeshRoot;
}
NiNodePtr PgItemEx::GetOriginalMeshRoot()
{
	return	m_spOriginalMeshRoot;
}

char const *PgItemEx::GetMeshPath()const
{
	return m_kMeshPath.c_str();
}
bool	PgItemEx::IsEqual(PgItemEx *pkItem)
{
	if(!pkItem) return false;

	CItemDef *pkItemDef = GetItemDef();
	CItemDef *pkItemDef2 = pkItem->GetItemDef();

	if(!pkItemDef || !pkItemDef2) return false;

	if(pkItemDef->No() != pkItemDef2->No()) return false;

	if(GetEnchantLevel() != pkItem->GetEnchantLevel()) return false;

	return	true;
}
int	PgItemEx::GetEnchantLevel()
{

	int	iEnchantLevel = -1;

//#ifndef EXTERNAL_RELEASE
//	iEnchantLevel = lua_tinker::call<int>("GetEnchantLevel");
//#endif

	if(iEnchantLevel == -1)
	{
		iEnchantLevel = GetItemInfo().EnchantInfo().PlusLv();
	}

	return	iEnchantLevel;
}

BM::vstring GetWeaponTagName(int const iWeaponType)
{
	BM::vstring kWeaponTag;
	switch(iWeaponType)
	{
	case PgItemEx::IT_SWORD		: { kWeaponTag = _T("swd"); }break;
	case PgItemEx::IT_BTS		: { kWeaponTag = _T("swd"); }break;
	case PgItemEx::IT_STF		: { kWeaponTag = _T("spr"); }break;
	case PgItemEx::IT_SPR		: { kWeaponTag = _T("spr"); }break;
	case PgItemEx::IT_BOW		: { kWeaponTag = _T("bow"); }break;
	case PgItemEx::IT_CROSSBOW	: { kWeaponTag = _T("csb"); }break;
	case PgItemEx::IT_CLAW		: { kWeaponTag = _T("clw"); }break;
	case PgItemEx::IT_KAT		: { kWeaponTag = _T("kat"); }break;
	case PgItemEx::IT_GUN_STAFF : { kWeaponTag = _T("spr");}break;
	case PgItemEx::IT_GLOVE		: { kWeaponTag = _T("clw"); }break;
	default : { kWeaponTag = _T("swd"); } break;
	}

	return kWeaponTag;

}

BM::vstring GetEnchantLevelName(int const iEnchantLevel)
{
	// 각 레벨 구간 별로 붙는 이름이 달라 진다.
	// 구간 레벨이 변경 될 시에는 밑에 코드 중 레벨 별 구간 위치만 변경 해주면 된다.
	BM::vstring kLevel;
	switch(iEnchantLevel)
	{
	case 5:
	case 6:
	case 7:
		{
			kLevel = _T("01");
		}break;
	case 8:
	case 9:
	case 10:
		{
			kLevel = _T("02");
		}break;
	case 11:
	case 12:
	case 13:
	case 14:
	case 15:
		{
			kLevel = _T("03");
		}break;
	case 16:
	case 17:
	case 18:
	case 19:
		{
			kLevel = _T("04");
		}break;
	case 20:
	case 21:
	case 22:
	case 23:
	case 24:
		{
			kLevel = _T("05");
		}break;	
	case 25:
	case 26:
	case 27:
	case 28:
	case 29:
		{
			kLevel = _T("06");
		}break;	
	case 30:
	case 31:
		{
			kLevel = _T("07");
		}break;
	default :
		{
			kLevel = _T("0");
		}break;
	}

	return kLevel;
}

void	PgItemEx::ApplyEnchantEffect()	//	아이템의 Enchant 레벨에 따라 Enchant 이펙트를 붙여주자.
{
	ClearEnchantEffect();	//	기존의 것은 지운다.

	PgItemEx* pkAdditionalItem = GetAdditionalItem();
	if(pkAdditionalItem)
	{
		pkAdditionalItem->ApplyEnchantEffect();
	}

	NiNodePtr spMeshRoot = m_spOriginalMeshRoot;
	if(!spMeshRoot)
	{
		spMeshRoot = m_spMeshRoot;
	}
	if(!spMeshRoot)
	{
		return;
	}

	int	const iEnchantLevel = GetEnchantLevel();
	int const iWeaponType = GetWeaponType();

	bool bEnchant = false;
	// 인챈트 파티클이 붙는 시점은 레벨5부터
	if(4 < iEnchantLevel && 0 < iWeaponType)
	{
		bEnchant = true;
	}	

	//무기 종류에 따라 Particle Name이 달라진다.
	//무기 종류에 따라 붙는 위치가 달라진다.
	
	NiNode* pkEnchantTargetNode = NiDynamicCast(NiNode,spMeshRoot->GetObjectByName("ef_enchant_01"));
	if(pkEnchantTargetNode)
	{
		NiAVObject*	pkPrevObject = NULL;
		int	iTotalChild = pkEnchantTargetNode->GetArrayCount();
		for(int i = 0; i < iTotalChild; ++i)
		{
			NiAVObject* pkChild = pkEnchantTargetNode->GetAt(i);
			if(pkChild)
			{
				pkPrevObject = pkChild;
				break;
			}
		}

		if(bEnchant)
		{
			// 파일명 규칙 : ef_Enchant_무기이름_레벨에따른 인덱스 번호
			// 클로우 / 카타르 :  ef_Enchant_무기이름_레벨에따른 인덱스 번호_장착위치
			BM::vstring kEffectName = BM::vstring(_T("ef_Enchant_")) << ::GetWeaponTagName(iWeaponType) << BM::vstring(_T("_")) << ::GetEnchantLevelName(iEnchantLevel);

			// 클로우 / 카타르 일 경우는 왼쪽 오른쪽 무기에 따라 붙는 이펙트 이름이 바뀌어야 한다.
			if(PgItemEx::IT_CLAW == iWeaponType 
				|| PgItemEx::IT_KAT == iWeaponType
				|| PgItemEx::IT_GLOVE == iWeaponType
				)
			{
				kEffectName << BM::vstring(_T("_"));

				int const iWeaponAttachPos = GetWeaponAttachPos();
				if(LEFT_HAND == iWeaponAttachPos || LEFT_FOREARM == iWeaponAttachPos)
				{
					kEffectName << BM::vstring(_T("l"));
				}
				else if(RIGHT_HAND == iWeaponAttachPos || RIGHT_FOREARM == iWeaponAttachPos)
				{
					kEffectName << BM::vstring(_T("r"));
				}
			}			

			PgParticle* pkPrevParticle = NiDynamicCast(PgParticle,pkPrevObject);
			if(pkPrevParticle)
			{
				if(pkPrevParticle->GetID() == std::string(MB(kEffectName)))
				{
					return;	//	이미 같은 ID의 파티클이 붙어있다.
				}
			}

			PgParticle* pkParticle = g_kParticleMan.GetParticle(MB(kEffectName));
			if(pkParticle)
			{
				// 한손 무기는 0.9 양손무기는 1.1 그래픽팀 요청에 의한 크기 수정(모델링 더미를 수정하기엔 무기 수가 너무 많음)
				if(PgItemEx::IT_SWORD == iWeaponType)
				{
					pkParticle->SetScale(0.9f);
				}
				if(PgItemEx::IT_BTS == iWeaponType)
				{
					pkParticle->SetScale(1.1f);
				}

				pkEnchantTargetNode->AttachChild(pkParticle, true);
			}
		}

		pkEnchantTargetNode->DetachChild(pkPrevObject);
	}
}
void PgItemEx::ClearEnchantEffect()
{
	PgItemEx* pkAdditionalItem = GetAdditionalItem();
	if(pkAdditionalItem)
	{
		pkAdditionalItem->ClearEnchantEffect();
	}


	NiNodePtr spMeshRoot = m_spOriginalMeshRoot;
	if(!spMeshRoot)
	{
		spMeshRoot = m_spMeshRoot;
	}
	if(!spMeshRoot)
	{
		return;
	}

	NiNode* pkEnchantTargetNode = NiDynamicCast(NiNode,spMeshRoot->GetObjectByName("ef_enchant_01"));
	if(pkEnchantTargetNode)
	{
		int	iTotalChild = pkEnchantTargetNode->GetArrayCount();
		for(int i = 0;i < iTotalChild; ++i)
		{
			NiAVObject* pkChild = pkEnchantTargetNode->GetAt(i);
			if(pkChild)
			{
				pkEnchantTargetNode->DetachChild(pkChild);
				break;
			}
		}
	}
}

void*	PgItemEx_Terminate_CallBack(NiAVObject *pkObject)
{
	__asm	int	3;
}

bool PgItemEx::Initialize()
{
	m_spMeshRoot = g_kNifMan.GetNif(m_kMeshPath);

	switch( g_kLocal.ServiceRegion() )
	{
	case LOCAL_MGR::NC_DEVELOP:
	case LOCAL_MGR::NC_VIETNAM:
		{
			if( m_bWeapon )
			{
				m_spMeshRoot->SetScale(m_spMeshRoot->GetScale() * g_fWeaponSize);
			}
		}break;
	}
	bool bRet = TexturePair::ApplyTexture(m_spMeshRoot, m_kTextureContainer);

	// AM이 있다면 AM의 Root에 Mesh를 붙여야 한다.
	if(m_spAM)
	{
		m_spAM->ChangeNIFRoot(m_spMeshRoot);
	}

	if(m_pcTargetPoint == 0)
	{
		switch(m_eItemLimit)
		{
		case EQUIP_LIMIT_GLASS:
		case EQUIP_LIMIT_HELMET:
		case EQUIP_LIMIT_HAIR:
			{
				m_pcTargetPoint = pcPtHair;
			}break;
		case EQUIP_LIMIT_FACE:
			{
				m_pcTargetPoint = pcPtHead;
			}break;
		case EQUIP_LIMIT_SHOULDER:
		case EQUIP_LIMIT_CLOAK:
			{
				m_pcTargetPoint = pcPtChest;
			}break;
		case EQUIP_LIMIT_WEAPON:
			{
				switch(m_eWeaponAttachPos)
				{
				case LEFT_HAND:
					{
						m_pcTargetPoint = pcPtLeftHand;
					}break;
				case RIGHT_HAND:
					{
						m_pcTargetPoint = pcPtRightHand;
					}break;
				case LEFT_FOREARM:
					{
						m_pcTargetPoint = pcPtLeftForearm;
					}break;
				case RIGHT_FOREARM:
					{
						m_pcTargetPoint = pcPtRightForearm;
					}break;
				case LEFT_HAND2:
					{
						m_pcTargetPoint = pcPtLeftHand2;
					}break;
				case RIGHT_HAND2:
					{
						m_pcTargetPoint = pcPtRightHand2;
					}break;
				default:
					{
					}break;
				}
			}break;
		case EQUIP_LIMIT_SHEILD:
			{
				m_pcTargetPoint = pcPtShield;
			}break;
		}
	}

	//	파티클 붙이기
	int	iTotalParticle = m_kParticles.size();
	for(int i=0;i<iTotalParticle;i++)
	{
		stParticle	const	&kParticle = m_kParticles[i];
		
		NiNode	*pkTarget = NiDynamicCast(NiNode,m_spMeshRoot->GetObjectByName(kParticle.m_kTargetNode.c_str()));
		if(!pkTarget)
		{
			continue;
		}

		PgParticle	*pkNewParticle = g_kParticleMan.GetParticle(kParticle.m_kParticleID.c_str());
		if(!pkNewParticle)
		{
			continue;
		}

		pkNewParticle->SetTranslate(kParticle.m_kTranslate);
		pkNewParticle->SetScale(kParticle.m_fScale);
		pkNewParticle->SetRotate(kParticle.m_kRotate);

		pkTarget->AttachChild(pkNewParticle, true);
	}
	return bRet;
}

char const *PgItemEx::GetTargetPoint()
{
	return m_pcTargetPoint;
}

void	PgItemEx::DetachUselessHairNode(NiAVObject *pkMeshRoot, char const *pcMeshPath,bool bIsRoot)
{
	if(!pkMeshRoot || !pcMeshPath)
	{
		return;
	}

	if(bIsRoot)
	{
		RestoreUselessHairNode(pkMeshRoot);
	}

	NiFixedString kMeshName(pcMeshPath);
	NiFixedString kUserDefined;

	NiNode	*pkNode = NiDynamicCast(NiNode,pkMeshRoot);
	if(pkNode)
	{

		int	iTotalChild = pkNode->GetArrayCount();
		for(int i=0;i<iTotalChild;++i)
		{
			NiAVObject	*pkChild = pkNode->GetAt(i);
			if(!pkChild)
			{
				continue;
			}

			NiStringExtraData *pkExtra = NiDynamicCast(NiStringExtraData, pkChild->GetExtraData("UserPropBuffer"));
			if(pkExtra)
			{
				kUserDefined = pkExtra->GetValue();
				if(kUserDefined.Contains(pcHairNodePropertyID) && pkChild->GetName().ContainsNoCase(kMeshName) == false)
				{
					m_kHairNodes.push_back(stHairNode(pkChild));
					pkNode->DetachChild(pkChild);
					continue;
				}
			}

			DetachUselessHairNode(pkChild,pcMeshPath,false);
		}
		return;
	}

	NiGeometry	*pkGeom = NiDynamicCast(NiGeometry,pkMeshRoot);
	if(pkGeom)
	{
		NiNode *pkParent = pkGeom->GetParent();
		if(pkParent)
		{
			bool bCulled = (pkParent->GetName().ContainsNoCase(kMeshName) == false);
			pkGeom->SetAppCulled(bCulled);
		}
	}

}
void	PgItemEx::RestoreUselessHairNode(NiAVObject *pkMeshRoot)
{
	if(!pkMeshRoot)
	{
		return;
	}

	for(int i=0;i<m_kHairNodes.size();++i)
	{
		stHairNode	&kHairNode = m_kHairNodes[i];
		if(kHairNode.m_spParent)
		{
			kHairNode.m_spParent->AttachChild(kHairNode.m_spHairNode,true);
		}
	}
	m_kHairNodes.clear();
}

bool PgItemEx::ReloadMeshRoot(NiNode *pkMeshRoot, char const *pcMeshName, bool bForceHide)
{
	if(!pcMeshName)
	{
		pkMeshRoot->SetAppCulled(bForceHide);
		return true;
	}

	if(!pkMeshRoot)
	{
		return true;
	}

	unsigned int uiArrayCount = pkMeshRoot->GetArrayCount();
	for(unsigned int uiIndex=0; uiIndex < uiArrayCount; ++uiIndex)
	{
		NiAVObject *pkChild = pkMeshRoot->GetAt(uiIndex);
		if(!pkChild)
		{
			continue;
		}
		if(NiIsKindOf(NiGeometry, pkChild))
		{
			NiNode *pkParent = pkChild->GetParent();
			if(pkParent)
			{
				NiFixedString kMeshName(pcMeshName);
				bool bCulled = (pkParent->GetName().ContainsNoCase(kMeshName) == false);
				pkChild->SetAppCulled(bCulled);
			}
		}
		else if(NiIsKindOf(NiNode, pkChild))
		{
			ReloadMeshRoot((NiNode *)pkChild, pcMeshName, bForceHide);			
		}
	}
	return true;
}

bool PgItemEx::IsAvailableAnimation()const
{
	return (m_spAM != 0);
}

void	PgItemEx::ApplyCustomColor(std::vector<NiColor> const &kColorCont,NiAVObject *pkAVObject)
{
	if(kColorCont.size() == 0)
	{
		return;
	}

	bool	bIsRootMesh = false;
	if(pkAVObject == 0)
	{
		bIsRootMesh = true;
		pkAVObject = GetMeshRoot();
	}

	NiNode	*pkNode = NiDynamicCast(NiNode,pkAVObject);
	if(pkNode)
	{

		for(int i=0;i<kColorCont.size();++i)
		{
			NiColor	const	&kColor = kColorCont[i];

			char	strExtraDataName[256];
			sprintf_s(strExtraDataName,256,"PG_CUSTOM_ITEM_COLOR_%d",i);
			NiStringExtraData *pkExtra = NiDynamicCast(NiStringExtraData, pkNode->GetExtraData("UserPropBuffer"));
			if(pkExtra)
			{
				// Geometry에 NoColorChange라고 UserProp에 적어뒀으면, 그에 해당하는 Material은 리스트에 추가하지 않는다.
				NiFixedString kUserDefined = pkExtra->GetValue();
				if(kUserDefined.ContainsNoCase(strExtraDataName) && pkNode->GetArrayCount()>0)
				{
					NiGeometry	*pkGeom = NiDynamicCast(NiGeometry,pkNode->GetAt(0));
					if(pkGeom)
					{
						NiMaterialProperty	*pkMaterial = NiDynamicCast(NiMaterialProperty,pkGeom->GetProperty(NiMaterialProperty::GetType()));
						if(pkMaterial)
						{
							NiMaterialProperty	*pkNewMaterial = NiDynamicCast(NiMaterialProperty,pkMaterial->Clone());
							pkNewMaterial->SetEmittance(kColor);
							pkGeom->DetachProperty(pkMaterial);
							pkGeom->AttachProperty(pkNewMaterial);
							pkGeom->UpdateProperties();

                            NewWare::Renderer::SetRenderStateTagExtraDataNumber( *pkGeom, 
                                NewWare::Renderer::Kernel::RenderStateTagExtraData::RESET_RENDERSTATE );
						}
					}
				}
			}
		}

		int	const	iTotalChild = pkNode->GetArrayCount();
		for(int i=0;i<iTotalChild;++i)
		{
			NiAVObject	*pkChild = pkNode->GetAt(i);
			if(!pkChild)
			{
				continue;
			}
			if(pkChild->GetExtraData("HAS_NO_VALID_GEOMETRY"))
			{
				continue;
			}
			ApplyCustomColor(kColorCont,pkChild);
		}

	}

	if(bIsRootMesh)
	{
        NewWare::Scene::ApplyTraversal::Geometry::SetDefaultMaterialNeedsUpdateFlag( pkAVObject, false );
	}
}

NiActorManagerPtr PgItemEx::GetActorManager()
{
	return m_spAM;
}
//!	외형을 다른 아이템으로 바꾼다.
void	PgItemEx::ChangeModel(char const *kNewItemXMLPath)
{
	_PgOutputDebugString("PgItemEx::ChangeModel kNewItemXMLPath:%s\n",kNewItemXMLPath);
	if(m_spOriginalMeshRoot == 0)
	{
		_PgOutputDebugString("PgItemEx::ChangeModel Set Original Mesh Root to %s\n", m_spMeshRoot->GetName());

		m_spOriginalMeshRoot= m_spMeshRoot;
	}

	PgItemEx *pkItem = GetItemFromXMLPath(this,kNewItemXMLPath,0);

	if(pkItem && m_spAM)
	{
		m_spMeshRoot = pkItem->GetMeshRoot();

		m_spAM->ChangeNIFRoot(m_spMeshRoot);

		SAFE_DELETE_NI(pkItem);
		_PgOutputDebugString("PgItemEx::ChangeModel New Mesh Root :%s\n",m_spMeshRoot->GetName());
	}
	else
	{
		_PgOutputDebugString("PgItemEx::ChangeModel GetItemFromXMLPath Failed\n");
	}
}
//!	외형을 원래 아이템의 것으로 돌려놓는다.
void	PgItemEx::RestoreOriginalModel()
{
	_PgOutputDebugString("PgItemEx::RestoreOriginalModel\n");

	if(m_spOriginalMeshRoot == 0) 
	{
		_PgOutputDebugString("PgItemEx::RestoreOriginalModel m_spOriginalMeshRoot == 0\n");
		return;
	}

	m_spMeshRoot = m_spOriginalMeshRoot;
	m_spOriginalMeshRoot = 0;


	_PgOutputDebugString("PgItemEx::RestoreOriginalModel Restored Mesh Root : %s\n",m_spMeshRoot->GetName());

	if(m_spAM)
	{
		m_spAM->ChangeNIFRoot(m_spMeshRoot);
	}
}

unsigned int PgItemEx::GetWeaponType()const
{
	return m_iWeaponType;
}

void PgItemEx::AddAttachedObject(NiAVObject *pkObject)
{
	m_kAttachedObjectList.insert(pkObject);
	//_PgOutputDebugString("PgItemEx::AddAttachedNode ItemNo : %d, AttachedNode : %s [%p] Parents: %s [%p]\n", m_pkItemDef->No(), pkObject->GetName(), pkObject, pkObject->GetParent()->GetName(), pkObject->GetParent());
}
void PgItemEx::Reset()
{
	RestoreUselessHairNode(GetMeshRoot());
	ResetHide();
	RestoreOriginalModel();
	
	if (m_spAM)
	{
		m_spAM->Reset();
		m_spAM->Update(0);
		m_spAM->ClearAllRegisteredCallbacks();
		if (m_spAM->GetNIFRoot())
		{
			m_spAM->GetNIFRoot()->Update(0.0f);
		}
	}

	m_kAttachedObjectList.clear();

	ClearEnchantEffect();

}
void PgItemEx::ResetHide()	//	숨김을 완전 해제한다.
{
	for(AVObjectSet::iterator itr = m_kAttachedObjectList.begin();
		itr != m_kAttachedObjectList.end();
		++itr)
	{
		(*itr)->SetAppCulled(false);
	}
	if(m_pkAdditionalParts)
	{
		m_pkAdditionalParts->ResetHide();
	}
}
void PgItemEx::Hide(bool const bHide)
{
	for(AVObjectSet::iterator itr = m_kAttachedObjectList.begin();
		itr != m_kAttachedObjectList.end();
		++itr)
	{
		(*itr)->SetAppCulled(bHide);
	}

	if(m_pkAdditionalParts)
	{
		m_pkAdditionalParts->Hide(bHide);
	}
}

NiNode *PgItemEx::GetFirstAttachedObject()
{
	AVObjectSet::iterator itr = m_kAttachedObjectList.begin();
	if(itr == m_kAttachedObjectList.end())
	{
		return 0;
	}

	return NiDynamicCast(NiNode,*itr);
}

void PgItemEx::RestoreAttachedObject()
{
	//_PgOutputDebugString("PgItemEx::RestoreAttachedNode() IitemNo : %d\n",GetItemDef()->No());

	AVObjectSet::iterator setItr = m_kAttachedObjectList.begin();
	while(setItr != m_kAttachedObjectList.end())
	{
		NiAVObject *pkSrcObject = *setItr;
		if(pkSrcObject)
		{
			NiNode *pkParent = pkSrcObject->GetParent();
			PG_ASSERT_LOG(pkParent);

			NiNode	*pkSrcNode = NiDynamicCast(NiNode,pkSrcObject);

			if (pkParent)
			{
				//_PgOutputDebugString("PgItemEx::RestoreAttachedNode() DetachChild Parent : %s [%p] SrcNode : %s [%p]\n",pkParent->GetName(), pkParent, pkSrcNode->GetName(), pkSrcNode);

				NiAVObjectPtr spObject = pkParent->DetachChild(pkSrcObject);
				spObject->UpdateProperties();
				spObject->UpdateEffects();
				spObject->Update(0.0f);

				if (m_spMeshRoot != spObject)
				{
					NiNode *pkOriginalParent = NiDynamicCast(NiNode,m_spMeshRoot->GetObjectByName(pkParent->GetName()));
					if(!pkOriginalParent)
					{
						for(AVObjectSet::iterator setItr2 = m_kAttachedObjectList.begin();setItr2 != m_kAttachedObjectList.end();setItr2++)
						{
							NiAVObject *pkSrcObject2 = *setItr2;
							NiNode	*pkNode2 = NiDynamicCast(NiNode,pkSrcObject2);
							if(pkNode2)
							{
								pkOriginalParent = NiDynamicCast(NiNode,pkNode2->GetObjectByName(pkParent->GetName()));
								if(pkOriginalParent)
								{
									break;
								}
							}

						}
					}

					if(pkOriginalParent)
					{
						//_PgOutputDebugString("PgItemEx::RestoreAttachedNode() AttachChild OriginalParent : %s [%p] spNode : %s [%p]\n",pkOriginalParent->GetName(), pkOriginalParent, spNode->GetName(), spNode);

						pkOriginalParent->AttachChild(spObject, true);
						pkOriginalParent->UpdateProperties();
						pkOriginalParent->UpdateEffects();
						pkOriginalParent->Update(0.0f);
						++setItr;
						continue;
					}
					else
					{
						PG_ASSERT_LOG(!"Can't find Original Parent. I'm Orphan.. p.p\n");
					}
				}
			}
			else
			{
				PG_ASSERT_LOG(!"This object has no parent.\n");
			}
		}
		setItr = m_kAttachedObjectList.erase(setItr);
	}

	setItr = m_kAttachedObjectList.begin();
	while(setItr != m_kAttachedObjectList.end())
	{
		NiAVObject *pkSrcObject = *setItr;
		if(pkSrcObject)
		{
			NiNode *pkParent = pkSrcObject->GetParent();
			PG_ASSERT_LOG(pkParent);

			NiNode	*pkSrcNode = NiDynamicCast(NiNode,pkSrcObject);

			if(pkParent && pkSrcNode && m_spMeshRoot && m_eItemLimit >= EQUIP_LIMIT_HELMET)
			{
				UnskinningParts(m_spMeshRoot, pkSrcNode);
			}
		}
		++setItr;
	}
	m_kAttachedObjectList.clear();
}

void PgItemEx::UnskinningParts(NiNode *pkItemRoot, NiAVObject *pkAttachedNode)
{
	if (pkItemRoot == NULL || pkAttachedNode == NULL)
		return;

	_PgOutputDebugString("[PgItemEx::UnskinningParts] ItemNo : %d pkItemRoot:%s pkAttachedNode:%s\n", GetItemDef() ? GetItemDef()->No() : -1, pkItemRoot->GetName(), pkAttachedNode->GetName());

	// Unskinning한다.
	NiNode *pkAttachedRoot = NiDynamicCast(NiNode,pkAttachedNode);
	if(!pkAttachedRoot)
	{
		return;
	}
	unsigned int uiArrayCnt = pkAttachedRoot->GetArrayCount();
	for(unsigned int uiIndex = 0; uiArrayCnt > uiIndex; ++uiIndex)
	{
		NiAVObject *pkChild = pkAttachedRoot->GetAt(uiIndex);
		if(NiIsKindOf(NiGeometry, pkChild) && !pkChild->GetAppCulled())
		{
			NiGeometry *pkGeometry = (NiGeometry *)pkChild;
			NiSkinInstance *pkSkin = pkGeometry->GetSkinInstance();
			if(!pkSkin)
			{
				continue;
			}

			NiSkinData *pkSkinData = pkSkin->GetSkinData();
			unsigned int uiBoneCount = pkSkinData->GetBoneCount();
			NiAVObject *pkRootParent = pkSkin->GetRootParent();
			NiAVObject *const *pkBones = pkSkin->GetBones();

			NiAVObject* pkDup = NULL;
			for(unsigned int ui = 0; ui < uiBoneCount; ui++)
			{
				NiAVObject* pkCurBone = pkBones[ui];
				pkDup = pkItemRoot->GetObjectByName(pkCurBone->GetName());
				if(pkDup)
				{
					pkSkin->SetBone(ui, pkDup);
				}
			}

			pkDup = pkItemRoot->GetObjectByName(pkRootParent->GetName());
			if(!pkDup)
			{
				continue;
			}

			pkSkin->SetRootParent(pkDup);
		}
		else if(NiIsKindOf(NiNode, pkChild))
		{
			UnskinningParts(pkItemRoot, pkChild);
		}
	}
}

bool PgItemEx::SetTextureBrightness(int iLevel)
{
	int iPos = 0;
	TexturePathContainer::iterator itr = m_kTextureContainer.begin();
	while(itr != m_kTextureContainer.end())
	{
		iPos = itr->m_kDestTexture.rfind(".dds") - 1;
		itr->m_kDestTexture[iPos] = iLevel + '0';
		itr->m_pkDestTexture = g_kNifMan.GetTexture(itr->m_kDestTexture);
		++itr;
	}

	if(!TexturePair::ApplyTexture(m_spMeshRoot, m_kTextureContainer, true))
	{
		return false;
	}
	
	itr = m_kTextureContainer.begin();
	while(itr != m_kTextureContainer.end())
	{
		iPos = itr->m_kSrcTexture.rfind(".dds") - 1;
		itr->m_kSrcTexture[iPos] = iLevel + '0';
		++itr;
	}
	return true;
}

PgItemEx *PgItemEx::GetAdditionalItem()
{
	return m_pkAdditionalParts;
}

int const PgItemEx::GetItemAbil(int const iAbilNo) const
{
	if(!m_pkItemDef)
	{
		return -1;
	}

	return m_pkItemDef->GetAbil(iAbilNo);
}

CItemDef *PgItemEx::GetItemDef()
{
	return m_pkItemDef;
}

NiMaterialProperty *PgItemEx::GetNextMaterial(bool bFirst)
{
	if(bFirst)
	{
		m_kMaterialItr = m_kMaterialList.begin();
	}
	else
	{
		++m_kMaterialItr;
	}

	if(m_kMaterialItr == m_kMaterialList.end())
	{
		return 0;
	}

	return *m_kMaterialItr;
}

void PgItemEx::SetItemColor(NiColor &rkItemColor, int iBrightness)
{
	if(iBrightness != -1)
	{
		SetTextureBrightness(iBrightness);
	}

	FindMaterialProp(m_spMeshRoot);
	for(MaterialContainer::iterator itr = m_kMaterialList.begin();
		itr != m_kMaterialList.end();
		++itr)
	{
		NiMaterialProperty *pkMaterialProp = *itr;
		pkMaterialProp->SetEmittance(rkItemColor);
	}
	
	m_spMeshRoot->UpdateProperties();
	m_spMeshRoot->UpdateEffects();
}

void PgItemEx::FindMaterialProp(NiNode *pkRoot, bool bCheckNoColorChange)
{
	unsigned int uiArrayCount = pkRoot->GetArrayCount();
	for (unsigned int i = 0; i < uiArrayCount; i++)
	{
		NiAVObject* pkChild = pkRoot->GetAt(i);
		if(!pkChild)
		{
			continue;
		}
		
		const NiFixedString &rkChildName = pkChild->GetName();
		if(bCheckNoColorChange)
		{
			NiStringExtraData *pkExtra = NiDynamicCast(NiStringExtraData, pkChild->GetExtraData("UserPropBuffer"));
			if(pkExtra)
			{
				// Geometry에 NoColorChange라고 UserProp에 적어뒀으면, 그에 해당하는 Material은 리스트에 추가하지 않는다.
				NiFixedString kUserDefined = pkExtra->GetValue();
				if(kUserDefined.ContainsNoCase("NoColorChange"))
				{
					continue;
				}
			}
		}

		if(NiIsKindOf(NiGeometry, pkChild))
		{
			NiGeometry *pkGeometry = (NiGeometry *)pkChild;

			// MaterialProperty를 가져온다.
			NiMaterialProperty *pkMaterialProp = NiDynamicCast(NiMaterialProperty, pkGeometry->GetProperty(NiMaterialProperty::GetType()));
			if(!pkMaterialProp)
			{
				continue;
			}
			
			m_kMaterialList.push_back(pkMaterialProp);
		}
		else if(NiIsKindOf(NiNode, pkChild))
		{
			// 본과 숨겨진 Geometry는 추가하지 않는다.
			if(rkChildName != NiFixedString("char_root") || !pkChild->GetAppCulled())
			{
				FindMaterialProp((NiNode *)pkChild, bCheckNoColorChange);
			}
		}
	}
}

void	PgItemEx::ApplyStatusEffect(PgPilot *pkTargetPilot)
{
	if(!pkTargetPilot)
	{
		return;
	}

	int	const iTotal = m_kStatusEffectVec.size();

	if(0 == iTotal)
	{
		return;
	}
	
	for(int i = 0; i < iTotal; ++i)
	{
		stStatusEffect &kStatusEffect = m_kStatusEffectVec[i];

		g_kStatusEffectMan.RemoveStatusEffectFromActor2(pkTargetPilot,kStatusEffect.m_iInstanceID);
		kStatusEffect.m_iInstanceID = g_kStatusEffectMan.AddStatusEffectToActor(pkTargetPilot, pkTargetPilot, kStatusEffect.m_kEffectID, GenerateStatusEffectID(), 0, 0,false,true);

	}
}

void	PgItemEx::ClearStatusEffect(PgPilot *pkTargetPilot)
{
	if(!pkTargetPilot)
	{
		return;
	}
	if(m_kStatusEffectVec.size() == 0)
	{
		return;
	}

	int	const iTotal = m_kStatusEffectVec.size();

	if(iTotal == 0)
	{
		return;
	}
	
	for(int i=0;i<iTotal;i++)
	{
		stStatusEffect &kStatusEffect = m_kStatusEffectVec[i];

		g_kStatusEffectMan.RemoveStatusEffectFromActor2(pkTargetPilot,kStatusEffect.m_iInstanceID);
		kStatusEffect.m_iInstanceID = -1;

	}
}

bool PgItemEx::ParseXml(const TiXmlNode *pkNode, void *pArg, bool bUTF8)
{
	int iItemGender = 0;
	int	iItemNo = 0;
	if(pArg)
	{
		// Get Pos!
		m_pkItemDef = (CItemDef *) pArg;
		m_eItemLimit = (eEquipLimit) GetItemAbil(AT_EQUIP_LIMIT);
		iItemGender = GetItemAbil(AT_GENDERLIMIT);

		if(m_pkItemDef)
		{
			iItemNo = m_pkItemDef->No();
		}
	}

	TiXmlElement *pkFirstElement = (TiXmlElement *)pkNode;
	PG_ASSERT_LOG(pkFirstElement);

	char const *pcFirstTagName = pkFirstElement->Value();

	// ITEM Tag 파싱
	if(strcmp(pcFirstTagName, "ITEM") == 0)
	{
		TiXmlAttribute* pkAttr = pkFirstElement->FirstAttribute();
		while(pkAttr)
		{
			char const *pcAttrName = pkAttr->Name();
			char const *pcAttrValue = pkAttr->Value();

			if(strcmp(pcAttrName, "NAME") == 0)
			{
				m_kID = pcAttrValue;
			}
			else if(strcmp(pcAttrName, "FILTER") == 0)
			{
				m_iItemFilter = atoi(pcAttrValue);
			}
			else if(strcmp(pcAttrName, "WEAPON") == 0)
			{
				m_bWeapon = true;
				m_iWeaponType = atoi(pcAttrValue);
			}
			else if(strcmp(pcAttrName, "ANIM_FOLDER_NUM") ==0)
			{
				m_byAnimFolderNum = (BYTE)(atoi(pcAttrValue));
			}
			else if(strcmp(pcAttrName, "WEAPON_ATTACH_POS") == 0)
			{
				if(strcmp(pcAttrValue, "LEFT") == 0)
				{
					m_eWeaponAttachPos = PgItemEx::LEFT_HAND;
				}
				else if(strcmp(pcAttrValue, "RIGHT") == 0)
				{
					m_eWeaponAttachPos = PgItemEx::RIGHT_HAND;
				}
				else if(strcmp(pcAttrValue, "LEFT_FOREARM") == 0)
				{
					m_eWeaponAttachPos = PgItemEx::LEFT_FOREARM;
				}
				else if(strcmp(pcAttrValue, "RIGHT_FOREARM") == 0)
				{
					m_eWeaponAttachPos = PgItemEx::RIGHT_FOREARM;
				}
				else if(strcmp(pcAttrValue, "LEFT2") == 0)
				{// 신종족이 채집도구, 싱글이모션시 표판(O,X)등 RIGHT_HAND 부분에 붙일수 없을때 이곳에 붙임
					m_eWeaponAttachPos = PgItemEx::LEFT_HAND2;
				}
				else if(strcmp(pcAttrValue, "RIGHT2") == 0)
				{
					m_eWeaponAttachPos = PgItemEx::RIGHT_HAND2;
				}
				
			}
			else if(strcmp(pcAttrName, "ADDITIANL_ITEM") == 0)
			{
				m_pkAdditionalParts = NiNew PgItemEx();
				m_pkAdditionalParts->SetUserGender(iItemGender);
				//m_pkAdditionalParts->SetUserClass(m_iUserClass); 				// 베트남
				m_pkAdditionalParts->SetXMLPath(pcAttrValue);
				m_pkAdditionalParts = (PgItemEx *)PgXmlLoader::CreateObjectFromFile(pcAttrValue, pArg, m_pkAdditionalParts);
			}
			else if(strcmp(pcAttrName, "ATTACH_TO") == 0)
			{
				m_pcTargetPoint = pcAttrValue;	// 약간 위험하다..
			}
			else
			{
				PgError1("PgItemEx : Unknown attribute - %s", pcAttrName);
			}

			pkAttr = pkAttr->Next();
		}
	}
	else
	{
		PgError1("%s xml has no ITEM Attribute!", pkNode->GetDocument()->Value());
		return false;
	}

	// ITEM의 Child Element 파싱
	TiXmlElement *pkElement = pkFirstElement->FirstChildElement();

	while(pkElement)
	{
		char const *pcTagName = pkElement->Value();
		if(0==strcmp(pcTagName, "ATTACK_EFFECT"))
		{
			TiXmlAttribute* pkAttr = pkElement->FirstAttribute();
			SAttackEffect kTemp;
			kTemp.m_kNodeID = "char_root";//기본
			while(pkAttr)
			{
				char const *pcAttrName = pkAttr->Name();
				char const *pcAttrValue = pkAttr->Value();

				if(strcmp(pcAttrName, "ACTION_ID") == 0)
				{
					kTemp.m_kActionID = pcAttrValue;
				}
				else if(strcmp(pcAttrName, "TO") == 0)
				{
					kTemp.m_kNodeID = pcAttrValue;
				}
				else if(strcmp(pcAttrName, "EFFECT") == 0)
				{
					kTemp.m_kEffectID = pcAttrValue;
				}
				else if(strcmp(pcAttrName, "SCALE") == 0)
				{
					kTemp.m_fScale = static_cast<float>(atof(pcAttrValue));
				}

				pkAttr = pkAttr->Next();
			}

			if(/*kTemp.m_kActionID.empty() || */kTemp.m_kEffectID.empty() || 0>=kTemp.m_fScale)
			{
				NILOG(PGLOG_WARNING, "[PgItemEx::ParseXml] Can't Parse ATTACK_EFFECT %s\n", m_kID);
			}
			else
			{
				m_kAttackEffMap.insert(std::make_pair(kTemp.m_kActionID, kTemp));
			}

			pkElement = pkElement->NextSiblingElement();
		}
		else if(0==strcmp(pcTagName, "POT_PARTICLE"))
		{
			TiXmlAttribute* pkAttr = pkElement->FirstAttribute();
			while(pkAttr)
			{
				char const *pcAttrName = pkAttr->Name();
				char const *pcAttrValue = pkAttr->Value();

				if(strcmp(pcAttrName, "ID") == 0)
				{
					m_kPOTParticleInfo.kEffectID = pcAttrValue;
				}
				else if(strcmp(pcAttrName, "NODE") == 0)
				{
					m_kPOTParticleInfo.kAttachNode = pcAttrValue;
				}
				else if(strcmp(pcAttrName, "EVENT_KEY") == 0)
				{
					m_kPOTParticleInfo.kEventKey = pcAttrValue;
				}
				else if(strcmp(pcAttrName, "POT") == 0)
				{
					m_kPOTParticleInfo.iAttachPointOfTime = ::atoi(pcAttrValue);
				}
				else if(strcmp(pcAttrName, "SCALE") == 0)
				{
					m_kPOTParticleInfo.fScale = static_cast<float>(::atof(pcAttrValue));
				}
				else if(strcmp(pcAttrName, "DETACHE_WHEN_ACTION_END") == 0)
				{
					m_kPOTParticleInfo.bDetachWhenActionEnd = (0 == strcmp("TRUE", pcAttrValue));
				}

				pkAttr = pkAttr->Next();
			}

			pkElement = pkElement->NextSiblingElement();
		}
		else if(0==strcmp(pcTagName, "DAMAGE_EFFECT"))
		{
			TiXmlAttribute* pkAttr = pkElement->FirstAttribute();
			while(pkAttr)
			{
				char const *pcAttrName = pkAttr->Name();
				char const *pcAttrValue = pkAttr->Value();
				if(strcmp(pcAttrName, "NORMAL") == 0)
				{
					m_kDamageEffNormal = pcAttrValue;
				}
				else if(strcmp(pcAttrName, "CRITICAL") == 0)
				{
					m_kDamageEffCritical = pcAttrValue;
				}

				pkAttr = pkAttr->Next();
			}

			pkElement = pkElement->NextSiblingElement();
		}
		else
		{
			break;
		}
	}

	////
	// 알맞은 성별의 매쉬 정보를 찾는다.
	char const *pcTagName = pkElement->Value();
	char const *pcReqGender = (m_iUserGender == 2 ? "FEMALE" : 
								m_iUserGender == 1 ? "MALE" :
								m_iUserGender == 0 ? "COMMON" :
								m_iUserGender == 4 ? "PET" :
								"UNKNOWN");

	while(pkElement)
	{
		char const *pcCurGender = pkElement->Value();

		// 같은 성별의 매쉬 정보를 찾았다면
		if(strcmp(pcCurGender, pcReqGender) == 0)
		{
			break;
		}
		// 공통 성별의 아이템일 경우, 공통 성별의 매쉬 정보를 찾았다면
		else if(iItemGender == 3 && strcmp(pcCurGender, "COMMON") == 0)
		{
			break;
		}

		pkElement = pkElement->NextSiblingElement();
	}

	// 알맞은 매쉬 정보가 없음.
	if(!pkElement)
	{
		PG_ASSERT_LOG(m_iUserGender >= 0 && m_iUserGender <= 2);
		NILOG(PGLOG_ERROR, "[PgItemEx] Doesn't fit Item Gender(%d) (Item No : %d)\n", m_iUserGender, m_pkItemDef->No());
		return false;
	}

	// 매쉬 정보의 첫 태그로 이동.
	pkElement = pkElement->FirstChildElement();

	////
	// 알맞은 클래스의 매쉬 정보를 찾는다.
	pcTagName = pkElement->Value();
	char const *pcReqClass = (m_iUserClass == 1 ? "FIGHTER" : 
					m_iUserClass == 2 ? "MAGICIAN" :
					m_iUserClass == 3 ? "ARCHER" : 
					m_iUserClass == 4 ? "THIEF" :
					m_iUserClass == 51 ? "DR_SUMMONER" :
					m_iUserClass == 52 ?"DR_TWINS"	: "THIEF"
					);
	TiXmlElement *pkClassElement = pkElement;
	while(pkClassElement)
	{
		char const *pcCurClass = pkClassElement->Value();

		// 같은 클래스의 매쉬 정보를 찾았다면
		if(strcmp(pcCurClass, pcReqClass) == 0)
		{
			break;
		}

		pkClassElement = pkClassElement->NextSiblingElement();
	}

	if(pkClassElement)
	{
		pkElement = pkClassElement->FirstChildElement();
	}

	////
	// 아이템 매쉬 생성.
	while(pkElement)
	{
		char const *pcTagName = pkElement->Value();
		if(strcmp(pcTagName, "KFMPATH") == 0)
		{
			// AM풀에서 가져온다.
			m_spAM = g_kAMPool.LoadActorManager(pkElement->GetText(), PgIXmlObject::ID_ITEM);
			if (m_spAM == NULL)
			{
				PgError2("can't load %s item's kfm %s", m_kID.c_str(), pkElement->GetText());
			}
			SetObjectID(PgIXmlObject::ID_ITEM);
		}
		else if(strcmp(pcTagName, "MESH") == 0)
		{
			// TODO : MESH풀에서 가져온다.
			m_kMeshPath = pkElement->GetText();
			TiXmlAttribute* pkAttr = pkElement->FirstAttribute();
			if(pkAttr)
			{
				char const *pcAttrName = pkAttr->Name();
				char const *pcAttrValue = pkAttr->Value();

				if(strcmp(pcAttrName, "TYPE") == 0)
				{
					m_iMeshType = atoi(pcAttrValue);
				}
			}
		}
		else if(strcmp(pcTagName, "STATUSEFFECT") == 0)
		{
			TiXmlAttribute* pkAttr = pkElement->FirstAttribute();
			if(pkAttr)
			{
				char const *pcAttrName = pkAttr->Name();
				char const *pcAttrValue = pkAttr->Value();

				if(strcmp(pcAttrName, "ID") == 0)
				{
					m_kStatusEffectVec.push_back(stStatusEffect(pcAttrValue));
				}
			}
		}
		else if(strcmp(pcTagName, "PARTICLE") == 0)
		{
			TiXmlAttribute* pkAttr = pkElement->FirstAttribute();
			std::string	kID,kTO;
			float	fScale = 1.0f;
			NiPoint3	kTranslate;
			NiPoint3	kRotAxis;
			float	fRotAngle = 0.0f;

			while(pkAttr)
			{
				char const *pcAttrName = pkAttr->Name();
				char const *pcAttrValue = pkAttr->Value();

				if(strcmp(pcAttrName, "ID") == 0)
				{
					kID = pcAttrValue;
				}
				else if(strcmp(pcAttrName, "TO") == 0)
				{
					kTO = pcAttrValue;
				}
				else if(strcmp(pcAttrName, "SCALE") == 0)
				{
					fScale = static_cast<float>(atof(pcAttrValue));
				}
				else if(strcmp(pcAttrName, "ROTATE") == 0)
				{
					sscanf_s(pcAttrValue, "%f, %f, %f, %f",
								&kRotAxis.x, &kRotAxis.y, &kRotAxis.z, &fRotAngle);
				}
				else if(strcmp(pcAttrName, "LOCATE") == 0)
				{
					sscanf_s(pcAttrValue, "%f, %f, %f",
								&kTranslate.x, &kTranslate.y, &kTranslate.z);
				}

				pkAttr = pkAttr->Next();
			}

			m_kParticles.push_back(stParticle(kID,kTO,fScale,kTranslate,kRotAxis,fRotAngle*3.141592f/180.0f));
		}
		else if(strcmp(pcTagName,"BOWSTRING_COLOR")==0)
		{
			TiXmlAttribute* pkAttr = pkElement->FirstAttribute();
			while(pkAttr)
			{
				char const *pcAttrName = pkAttr->Name();
				char const *pcAttrValue = pkAttr->Value();

				if(strcmp(pcAttrName, "A") == 0)
				{
					m_kBowStringColor.a = (float)atof(pcAttrValue);
				}
				else if(strcmp(pcAttrName, "R") == 0)
				{
					m_kBowStringColor.r = (float)atof(pcAttrValue);
				}
				else if(strcmp(pcAttrName, "G") == 0)
				{
					m_kBowStringColor.g = (float)atof(pcAttrValue);
				}
				else if(strcmp(pcAttrName, "B") == 0)
				{
					m_kBowStringColor.b = (float)atof(pcAttrValue);
				}

				pkAttr = pkAttr->Next();
			}	
		}
		else if(strcmp(pcTagName,"PROJECTILE") == 0)
		{
			TiXmlAttribute* pkAttr = pkElement->FirstAttribute();
			while(pkAttr)
			{
				char const *pcAttrName = pkAttr->Name();
				char const *pcAttrValue = pkAttr->Value();

				if(strcmp(pcAttrName, "ID") == 0)
				{
					m_kProjectileID = pcAttrValue;
				}
				else
				{
					PgError1("PgItemEx : Unknown attribute - %s", pcAttrName);
				}

				pkAttr = pkAttr->Next();
			}
		}
		else if(strcmp(pcTagName,"CUSTOM_ANI_ID_CHANGE_SETTING") == 0)
		{
			m_stCustomAniIDChangeSetting.m_bUse= true;

			TiXmlAttribute* pkAttr = pkElement->FirstAttribute();
			while(pkAttr)
			{
				char const *pcAttrName = pkAttr->Name();
				char const *pcAttrValue = pkAttr->Value();

				if(strcmp(pcAttrName, "DEFAULT_ANI_ID") == 0)
				{
					m_stCustomAniIDChangeSetting.m_iDefaultAniID = atoi(pcAttrValue);
				}
				else
				{
					PgError1("PgItemEx : Unknown attribute - %s", pcAttrName);
				}

				pkAttr = pkAttr->Next();
			}

			// 자식 노드들을 파싱한다.
			const TiXmlNode * pkChildNode = pkElement->FirstChild();
			while(pkChildNode != NULL)
			{
				int const iType = pkChildNode->Type();
				
				switch(iType)
				{
				case TiXmlNode::ELEMENT:
					{
						TiXmlElement *pkElement2 = (TiXmlElement *)pkChildNode;
						char const *pcTagName2 = pkElement2->Value();

						if(strcmp(pcTagName2,"ANI_ID_CHANGE")==0)
						{
							int	iSrcID=0;
							int	iNewID=0;
							pkAttr = pkElement2->FirstAttribute();
							while(pkAttr)
							{
								char const *pcAttrName = pkAttr->Name();
								char const *pcAttrValue = pkAttr->Value();

								if(strcmp(pcAttrName, "SRC_ID") == 0)
								{
									iSrcID = atoi(pcAttrValue);
								}
								else if(strcmp(pcAttrName, "NEW_ID") == 0)
								{
									iNewID = atoi(pcAttrValue);
								}
								else
								{
									PgError1("PgItemEx : Unknown attribute - %s", pcAttrName);
								}

								pkAttr = pkAttr->Next();
							}

							m_stCustomAniIDChangeSetting.m_AniIDChangeInfoMap.insert(std::make_pair(iSrcID,iNewID));
						}
					}
					break;
				}

				pkChildNode = pkChildNode->NextSibling();
			}
		}
		else if(strcmp(pcTagName, "TEXTURE") == 0)
		{
			// TODO : 텍스쳐풀에서 가져와야 함
			char const *pcSrc = 0;
			char const *pcDest = 0;

			TiXmlAttribute *pkAttr = pkElement->FirstAttribute();
			while(pkAttr)
			{
				char const *pcAttrName = pkAttr->Name();
				char const *pcAttrValue = pkAttr->Value();

				if(strcmp(pcAttrName, "SRC") == 0)
				{
					pcSrc = pcAttrValue;
					pcDest = pkElement->GetText();
				}
				else
				{
					_PgError("PgItemEx", "PgItemEx - Unknown tag : %s[%d]", pcAttrName,iItemNo);
				}

				pkAttr = pkAttr->Next();
			}

			if(pcSrc && pcDest)
			{
				NiString kSrc(pcSrc);
				NiString kDest(pcDest);
				
				kSrc.ToLower();
				kDest.ToLower();

				TexturePair kTexturePair(kSrc, kDest);
				kTexturePair.m_pkDestTexture = g_kNifMan.GetTexture(kTexturePair.m_kDestTexture);
				m_kTextureContainer.push_back(kTexturePair);
			}
		}
		else if(strcmp(pcTagName,"TRAIL") == 0)	//	궤적 정보 파싱 leesg213 2007-01-09
		{
			m_kTrailInfo.m_bTrailActivate = true;

			TiXmlElement *pkTrailElement = pkElement->FirstChildElement();

			while(pkTrailElement)
			{
				char const* pTrailElemTagName = pkTrailElement->Value();
				if(strcmp(pTrailElemTagName,"TEXTURE") == 0)
				{
					m_kTrailInfo.m_kTexturePath = pkTrailElement->GetText();
				}
				else if(strcmp(pTrailElemTagName,"TOTAL_TIME") == 0)
				{
					m_kTrailInfo.m_iTotalTime = atoi(pkTrailElement->GetText());
				}
				else if(strcmp(pTrailElemTagName,"BRIGHT_TIME") == 0)
				{
					m_kTrailInfo.m_iBrightTime = atoi(pkTrailElement->GetText());
				}
				else if(strcmp(pTrailElemTagName,"EXTEND_LENGTH") == 0)
				{
					m_kTrailInfo.m_fExtendLength = (float)atof(pkTrailElement->GetText());
				}
				else
				{
					_PgError("PgItemEx", "PgItemEx - Unknown tag : %s[%d]", pTrailElemTagName,iItemNo);
					break;
				}

				pkTrailElement = pkTrailElement->NextSiblingElement();
			}

		}
		else if(strcmp(pcTagName,"HIDE_NODE_NAME") == 0)	
		{
			TiXmlAttribute* pkAttr = pkElement->FirstAttribute();
			if(pkAttr)
			{
				char const *pcAttrName = pkAttr->Name();
				char const *pcAttrValue = pkAttr->Value();

				if(strcmp(pcAttrName, "ID") == 0)
				{
					typedef std::list< std::string > Contstr;
					Contstr kList;
					BM::vstring::CutTextByKey(std::string(pcAttrValue), std::string("/"), kList);

					for(Contstr::const_iterator it = kList.begin(); it!=kList.end(); ++it)
					{
						std::back_inserter(m_kHideNodeContainer) = (*it);
					}
				}
			}
		}
		else
		{
			_PgError("PgItemEx", "PgItemEx - Unknown tag : %s[%d]", pcTagName,iItemNo);
			break;
		}

		pkElement = pkElement->NextSiblingElement();
	}

	return Initialize();
}

PgItemEx *PgItemEx::GetItemFromXMLPath(PgItemEx *pkSrcItem, char const *pkXMLPath, int iItemSex, int iClassNo)
{
	// 아이템을 생성한다.
	PgItemEx *pkItemEx = NiNew PgItemEx();
	pkItemEx->SetUserGender(iItemSex);
	pkItemEx->SetUserClass(iClassNo);
	pkItemEx->SetXMLPath(pkXMLPath);
	pkItemEx = (PgItemEx *)PgXmlLoader::CreateObjectFromFile(pkXMLPath, (void *)pkSrcItem->GetItemDef(), pkItemEx);
	PG_ASSERT_LOG(pkItemEx != NULL);
	if (pkItemEx == NULL)
	{
		NILOG(PGLOG_ERROR, "[PgItemEx] GetItemFromXMLPath, CreateObjectFromFile Failed(%s,%d)\n", pkXMLPath, iItemSex);
	}
	return pkItemEx;
}

PgItemEx *PgItemEx::GetItemFromDef(int iItemDefNo, int iUserGender, int iClassNo)
{
#ifdef PG_USE_ITEM_REUSE
	return g_kItemMan.GetItem(iItemDefNo, iUserGender, iClassNo);
#else
	if(!iItemDefNo)
	{
		return NULL;
	}

	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const *pkItemDef = kItemDefMgr.GetDef(iItemDefNo);
	if (!pkItemDef)
	{
		PG_ASSERT_LOG(!"Can't Find Item from ItemDefMgr");
		return 0;
	}

	CONT_DEFRES* pContDefRes = NULL;
	g_kTblDataMgr.GetContDef(pContDefRes);

	CONT_DEFRES::iterator itrDest = pContDefRes->find(pkItemDef->ResNo());
	if(itrDest == pContDefRes->end())
	{
		// 아이템 테이블에서 못 찾으면 패스!
		PG_ASSERT_LOG(!"Can't Find ContDefRes");
		return 0;
	}

	// 아이템을 생성한다.
	PgItemEx *pkItemEx = NiNew PgItemEx();
	pkItemEx->SetUserGender(iUserGender);
	pkItemEx->SetUserClass(iClassNo);
	pkItemEx = (PgItemEx *)PgXmlLoader::CreateObjectFromFile(MB(itrDest->second.strXmlPath), (void *)pkItemDef, pkItemEx);
	PG_ASSERT_LOG(pkItemEx != NULL);
	if (pkItemEx == NULL)
	{
		NILOG(PGLOG_ERROR, "[PgItemEx] GetItemFromDef, CreateObjectFromFile Failed(%d,%d,%s)\n", iItemDefNo, iUserGender, MB(itrDest->second.strXmlPath));
	}
	return pkItemEx;
#endif
}

void PgItemEx::PrintItemInfo()
{
#ifndef EXTERNAL_RELEASE
	NILOG(PGLOG_MINOR, "[PgItemEx] PrintItemInfo, %d, %d, %d, %d, %d, %d\n", m_eItemLimit, m_iItemType, m_bWeapon, m_iWeaponType, m_pkItemDef->No(), m_pkItemDef->EquipPos());
#endif
}

bool PgItemEx::FindAttackEffect(char const* szActionName, SAttackEffect& rkEffect)
{
	AttackEffMap::iterator eff_it = m_kAttackEffMap.find(szActionName);
	if(m_kAttackEffMap.end() != eff_it)
	{
		rkEffect = (*eff_it).second;
		return true;
	}
	return false;
}

PgItemEx::SAttackEffect const* PgItemEx::FindAttackEffect(char const* szActionName) const
{
	AttackEffMap::const_iterator eff_it = m_kAttackEffMap.find(szActionName);
	if(m_kAttackEffMap.end() != eff_it)
	{
		return &(*eff_it).second;
	}
	return NULL;
}

void PgItemEx::ApplyTextureChange(int const iNo, NiAVObject* pkAVObject)
{
	NiNode* pkNode = NiDynamicCast(NiNode,pkAVObject);
	if(!pkNode)	{return;}

	NiStringExtraData* pkExtra = NiDynamicCast(NiStringExtraData, pkNode->GetExtraData("UserPropBuffer"));
	if(pkExtra)
	{
		NiFixedString const& rkUserDefined = pkExtra->GetValue();
		if(g_pkWorld
			&& rkUserDefined.Contains("ChangedTexture"))
		{
			NiObjectList kGeometries;
			g_pkWorld->GetAllGeometries(pkNode, kGeometries);//내 자식들 다 검사
			BM::vstring kIndex;	//미리 만들어 놓자
			if(10 > iNo)
			{
				kIndex += L"0";
			}
			kIndex += iNo;
			while(!kGeometries.IsEmpty())
			{
				NiGeometry* pkGeo = NiDynamicCast(NiGeometry, kGeometries.GetTail());
				kGeometries.RemoveTail();

				if (!pkGeo || !pkGeo->GetPropertyState())
				{
					continue;
				}

				bool bPropertyUpdate = false;
				NiTexturingProperty* pkTextureProp = pkGeo->GetPropertyState()->GetTexturing();
				if(pkTextureProp)
				{
					NiTexturingProperty::NiMapArray const& kMaps = pkTextureProp->GetMaps();
					int	const iTotal = kMaps.GetSize();
					for(int i = 0;i < iTotal; ++i)
					{
						NiTexturingProperty::Map* pkMap = kMaps.GetAt(i);
						if(!pkMap || !pkMap->GetTexture())	{continue;}

						NiSourceTexture* pkSrc = NiDynamicCast(NiSourceTexture, pkMap->GetTexture());
						if(!pkSrc)	{continue;}

						std::string kPath = pkSrc->GetFilename();
						size_t kPos = kPath.find(".dds");
						if(2<kPos)	//두글자가 바뀔거라 적어도 2보단 커야지
						{
							kPath.replace(kPos-2, 2, MB(kIndex));
						}

						NiSourceTexture* pTexture = g_kNifMan.GetTexture(kPath);
						if (pTexture)
						{
							pkMap->SetTexture(pTexture);
							bPropertyUpdate = true;
						}
						else
						{
							std::string strError = "[PgItemEx::ApplyTextureChange] Texture load failed : ";
							strError += kPath.c_str();

							PgError(strError.c_str());
							PG_ASSERT_LOG(!strError.c_str());
						}						
					}
				}

				if(bPropertyUpdate)
				{
					pkGeo->GetModelData()->SetConsistency(NiGeometryData::MUTABLE);
					pkGeo->GetModelData()->MarkAsChanged(NiGeometryData::TEXTURE_MASK);
					pkGeo->UpdateProperties();

                    NewWare::Renderer::SetRenderStateTagExtraDataNumber( *pkGeo, 
                            NewWare::Renderer::Kernel::RenderStateTagExtraData::RESET_RENDERSTATE );
				}
			}
			return;	//내 자식들 다 검사 했으니 내 밑으로는 더 돌 필요 없음
		}
	}

	unsigned int iCount = pkNode->GetArrayCount();
	for(unsigned int i = 0; i<iCount; ++i)
	{
		NiAVObject* pkChild = pkNode->GetAt(i);
		if (!pkChild) {continue;}

		ApplyTextureChange(iNo, pkChild);
	}
}

void PgItemEx::SetActorNodesHide(PgActor* pActor, bool bHide)
{
	if(NULL==pActor)	
	{
		return;
	}

	for(StringCont::const_iterator it = m_kHideNodeContainer.begin(); it != m_kHideNodeContainer.end(); ++it)
	{
		pActor->SetNodeHide((*it).c_str(), bHide);
	}
}