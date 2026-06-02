#ifndef FREEDOM_DRAGONICA_RENDER_WORDOBJECT_ITEM_PGITEMEX_H
#define FREEDOM_DRAGONICA_RENDER_WORDOBJECT_ITEM_PGITEMEX_H

#include "PgIXmlObject.h"

class TexturePair;
typedef std::vector<TexturePair> TexturePathContainer;

class TexturePair
{
public:
	TexturePair(char const *kSrcTexture);
	TexturePair(char const *kSrcTexture, char const *kDestTexture);

	std::string m_kSrcTexture;
	std::string m_kDestTexture;
	NiSourceTexture* m_pkDestTexture;

	bool operator ==(const TexturePair &rhs);
	//! 텍스쳐를 적용한다.
	static bool ApplyTexture(NiNode *pkRoot, TexturePathContainer& rkTexturePathCont, bool bCheckNoChange = false);
};

struct SPOTParticleInfo;
class	CUnit;
class	PgPilot;
class PgItemEx : public PgIXmlObject, public NiAVObject
{
	NiDeclareRTTI;
public:
	typedef enum
	{
		LOAD_TYPE_NONE = -1,
		LOAD_TYPE_DEFAULT = 0,
		LOAD_TYPE_USEQUEUE = 1,
		LOAD_TYPE_INSTANT = 2,
		MAX_LOAD_TYPE,
	} ItemLoadType;
	typedef enum
	{
		HELM_TYPE_NONE = 1,
		HELM_TYPE_HELM = 2,
		HELM_TYPE_OPEN_COWL = 3,
		HELM_TYPE_COWL = 4,
		HELM_TYPE_DIADEM = 5,
		HELM_TYPE_BEANIE = 6,
		HELM_TYPE_HOOD = 7,
		HELM_TYPE_HAT = 8
	}HelmType;

	typedef enum
	{
		UNKNOWN = 0,
		LEFT_HAND = 1,
		RIGHT_HAND = 2,
		LEFT_FOREARM = 3,
		RIGHT_FOREARM = 4,
		// 신종족이 채집도구, 싱글이모션시 표판(O,X)등 RIGHT_HAND 부분에 붙일수 없을때 이곳에 붙임
		LEFT_HAND2	= 5,
		RIGHT_HAND2 = 6,
		//
	} WeaponAttachPos;

	typedef enum 
	{
		IT_FST = 0,
		IT_SWORD = 1,
		IT_BTS = 2,
		IT_STF = 3,
		IT_SPR = 4,
		IT_BOW = 5,
		IT_CROSSBOW = 6,
		IT_CLAW = 7,
		IT_KAT = 8,
		IT_SPECIAL = 9,
		IT_JOB_TOOL = 10,
		IT_GUN_STAFF = 11,
		IT_GLOVE = 12,
		IT_UNKNOWN = 100
	} WeaponType;

	// 궤적 관련 작업 추가, leesg213 2007-01-09
	struct	stTrailInfo
	{
		bool	m_bTrailActivate;
		std::string	m_kTexturePath;
		int	m_iTotalTime,m_iBrightTime;
		float m_fExtendLength;

		void	SetDefault()
		{
			m_bTrailActivate = false;
			m_kTexturePath = "../Data/5_Effect/9_Tex/Trail.dds";
			m_iTotalTime = 300;
			m_iBrightTime = 100;
			m_fExtendLength = 5.0f;
		}
	};

	struct	stCustomAniIDChangeSetting
	{
		bool	m_bUse;

		typedef	std::map<int,int> AniIDChangeInfoMap;

		AniIDChangeInfoMap	m_AniIDChangeInfoMap;
		int	m_iDefaultAniID;

		stCustomAniIDChangeSetting()
		{
			m_iDefaultAniID  = 0;
			m_bUse = false;
		}

		int	GetChangedAniID(int iSrcID)
		{
			if(!m_bUse) return iSrcID;

			AniIDChangeInfoMap::iterator	itor = m_AniIDChangeInfoMap.find(iSrcID);
			if(itor != m_AniIDChangeInfoMap.end())
			{
				return	itor->second;
			}
			return	m_iDefaultAniID;
		}
	};

	struct	stStatusEffect
	{
		std::string	m_kEffectID;
		int	m_iInstanceID;
		stStatusEffect(std::string const &kEffectID):m_iInstanceID(-1),m_kEffectID(kEffectID)
		{
		}
	};

	struct	stParticle
	{
		std::string	m_kParticleID;
		std::string	m_kTargetNode;
		float	m_fScale;
		NiPoint3	m_kTranslate;
		NiQuaternion	m_kRotate;

		stParticle(std::string const &kID,
			std::string const &kTO,
			float fScale,
			NiPoint3 const &kTranslate,
			NiPoint3 const &kRotAxis,
			float fRotAngle):
				m_kParticleID(kID),
				m_kTargetNode(kTO),
				m_fScale(fScale),
				m_kTranslate(kTranslate),
				m_kRotate(fRotAngle,kRotAxis)
		{
		};
	};

	typedef struct stAttackEffect
	{
		std::string m_kActionID;
		std::string m_kNodeID;
		std::string m_kEffectID;
		float		m_fScale;
		stAttackEffect()
		{
			Clear();
		}

		void Clear()
		{
			m_kActionID.clear();
			m_kNodeID.clear();
			m_kEffectID.clear();
			m_fScale = 1.0f;
		}
	}SAttackEffect;

	struct	stHairNode
	{
		NiNodePtr	m_spParent;
		NiAVObjectPtr	m_spHairNode;

		stHairNode(NiAVObject *pkHair)
		{
			m_spHairNode = pkHair;
			m_spParent = pkHair->GetParent();
		}

	};

	typedef std::vector<stHairNode> HairNodeVec;
	typedef std::set<NiAVObject*> AVObjectSet;
	typedef std::vector<NiMaterialPropertyPtr> MaterialContainer;
	typedef std::vector<stStatusEffect> StatusEffectVec;
	typedef std::vector<stParticle> ParticleVec;
	typedef std::map<std::string, SAttackEffect> AttackEffMap;

	PgItemEx();
	PgItemEx(int iSex);
	~PgItemEx();

	//! 초기화 한다.
	bool Initialize();

	//!	삭제되지 않고 Cache 될때 아이템 상태를 초기화 하기 위해 쓰인다.
	void	Reset();
	
	//! 메쉬 루트 노드를 반환한다.
	NiNodePtr GetMeshRoot();
	
	//! 오리지널 메쉬 루트 노드를 반환한다.
	NiNodePtr GetOriginalMeshRoot();

	
	char const *GetMeshPath()const;//! 메쉬 경로를 반환한다.
	eEquipLimit EquipLimit()const;//! 아이템 타입을 반환한다.
	int GetMeshType()const;//! Mesh Type을 반환한다.	
	bool IsWeapon()const;//! 무기인가?
	unsigned int GetWeaponType()const;//! 무기 종류를 반환한다.
	bool IsAvailableAnimation()const;//! 파츠에 애니가 있는가?

	
	bool ReloadMeshRoot(NiNode *pkMeshRoot, char const *pcMeshPath, bool bForceHide);//! 메쉬를 다른 경로의 것으로 리로딩 한다.

	void SetUserGender(int const iGender);//! 필요한 아이템 성별을 설정한다.
	void SetUserClass(int const iClassNo);//! 필요한 아이템의 클래스를 설정한다.
	int GetUserGender()	{ return m_iUserGender; }
	int GetUserClass()	{ return m_iUserClass; }

	//! Xml을 파싱한다.
	virtual bool ParseXml(const TiXmlNode *pkNode, void *pArg = 0, bool bUTF8 = false);

	//! Item Abil을 가져온다.
	int const GetItemAbil(int const iAbilNo) const;

	//! ActorManager를 반환한다.
	NiActorManagerPtr GetActorManager();

	void	ApplyCustomColor(std::vector<NiColor> const &kColorCont,NiAVObject *pkAVObject = 0);
	
	void	ChangeModel(char const *kNewItemXMLPath);//!	외형을 다른 아이템으로 바꾼다.
	void	RestoreOriginalModel();//!	외형을 원래 아이템의 것으로 돌려놓는다.

	void	DetachUselessHairNode(NiAVObject *pkMeshRoot, char const *pcMeshPath,bool bIsRoot = true);
	void	RestoreUselessHairNode(NiAVObject *pkMeshRoot);
	
	void AddAttachedObject(NiAVObject *pkObject);//! Attached Node List에 추가한다.
	NiNode *GetFirstAttachedObject();//! 처음으로 붙인 노드를 반환한다.
	void RestoreAttachedObject();//! Attach했던 노드를을 모두 떼어낸다.
	void UnskinningParts(NiNode *pkItemRoot, NiAVObject *pkAttachedNode);//! Skinning했던 것을 제거한다.
	void Hide(bool const bHide);//! Attach한 노드들을 숨긴다
	void ResetHide();	//	숨김을 완전 해제한다.

	//! Texture Container를 얻어온다.
	bool SetTextureBrightness(int iLevel);

	//! Weapon이 붙어야 하는 위치를 알려준다.
	WeaponAttachPos GetWeaponAttachPos();
	
	//! Get Additional Item
	PgItemEx *GetAdditionalItem();
	
	//! ItemDef를 반환한다.
	CItemDef *GetItemDef();

	//! Equip될 Target Point를 리턴한다.
	char const *PgItemEx::GetTargetPoint();

	//! Item No로 부터 PgItemEx를 생성한다.
	static PgItemEx *GetItemFromDef(int iItemNo, int iItemSex, int iClassNo = 0);
	//! XML Path로 부터 PgItemEx를 생성한다.
	static PgItemEx *GetItemFromXMLPath(PgItemEx *pkSrcItem,char const *pkXMLPath,int iItemSex, int iClassNo = 0);

	//! Item의 Material을 읽어온다.
	void FindMaterialProp(NiNode *pkRoot, bool bCheckNoColorChange = true);

	//! 아이템 색깔을 바꾼다.
	void SetItemColor(NiColor &rkItemColor, int iBrightness = -1);

	//! Material을 가져다 준다.
	NiMaterialProperty *GetNextMaterial(bool bFisrt = false);
	
	//! 궤적 정보를 얻어온다.
	const	stTrailInfo&	GetTrailInfo()	{	return	m_kTrailInfo;	}

	stCustomAniIDChangeSetting*	GetCustomAniIDChangeSetting()	{	return	&m_stCustomAniIDChangeSetting;	}

	const	std::string &GetProjectileID()	{	return	m_kProjectileID;	}

	BYTE	GetAnimFolderNum()	{	return	m_byAnimFolderNum;	}

	std::string& GetXMLPath() { return m_kXMLPath; }
	void SetXMLPath(char const* path) { m_kXMLPath = path; }
	void PrintItemInfo();

	void	ApplyEnchantEffect();	//	아이템의 Enchant 레벨에 따라 Enchant 이펙트를 붙여주자.
	void	ClearEnchantEffect();	

	bool	IsEqual(PgItemEx *pkItem);


	int	GetEnchantLevel();

	void	SetItemInfo(PgBase_Item const &kItem)	
	{	
		m_kItemInfo = kItem;	

		if(GetAdditionalItem())
		{
			GetAdditionalItem()->SetItemInfo(kItem);
		}
	}
	PgBase_Item&	GetItemInfo()	{	return	m_kItemInfo;	}

	void	ApplyStatusEffect(PgPilot *pkTargetPilot);
	void	ClearStatusEffect(PgPilot *pkTargetPilot);

	bool	FindAttackEffect(char const* szActionName, SAttackEffect& rkEffect);
	SAttackEffect const* FindAttackEffect(char const* szActionName) const;
	std::string const& GetDamageEffect(bool const bIsCri = false) const 
	{ return bIsCri ? m_kDamageEffCritical : m_kDamageEffNormal; }

	static void	ApplyTextureChange(int const iNo, NiAVObject *pkAVObject);

	SPOTParticleInfo const& GetPOTParticleInfo() { return m_kPOTParticleInfo; }

	void SetActorNodesHide(PgActor* pActor, bool bHide);
private:

	NiNodePtr m_spMeshRoot;
	NiNodePtr	m_spOriginalMeshRoot;
	std::string m_kXMLPath;

	//! 무기인가
	bool m_bWeapon;

	//! 무기를 어디에 붙여야 하는가 (왼손, 오른손, 양손 다)
	WeaponAttachPos m_eWeaponAttachPos;

	//! 무기 종류
	int m_iWeaponType;

	//! 아이템 타입(포션, 포탈, 아이템 등)
	int m_iItemType;

	//!	KFM 내에서의 애니메이션 폴더 번호
	BYTE	m_byAnimFolderNum;

	//! 아이템 착용 위치
	eEquipLimit m_eItemLimit;

	//! 아이템 사용하는 유저의 성별
	int m_iUserGender;

	//! 아이템을 사용하려는 유저의 클래스
	int m_iUserClass;

	//! 아이템 필터 
	int m_iItemFilter;

	//! Bow String 헬퍼가 있다면, Bow String 의 컬러
	NiColorA m_kBowStringColor;

	//! 메쉬 패스
	std::string m_kMeshPath;

	//! 텍스쳐 목록
	TexturePathContainer m_kTextureContainer;

	//!	궤적 정보 leesg213 2007-01-09
	stTrailInfo	m_kTrailInfo;

	//! ActorManager
	NiActorManagerPtr m_spAM;

	//! Attach한 Object리스트
	AVObjectSet m_kAttachedObjectList;

	//! 애니메이션 ID 수정 정보 leesg213
	stCustomAniIDChangeSetting	m_stCustomAniIDChangeSetting;

	//!	발사체 ID
	std::string m_kProjectileID;

	//! Item Def
	CItemDef *m_pkItemDef;

	//!	착용 했을 때 발생하는 상태이상효과
	StatusEffectVec	m_kStatusEffectVec;

	//!	메쉬 타입
	int m_iMeshType;

	//! Attach Point
	char const *m_pcTargetPoint;

	//! Additional Item
	PgItemEx *m_pkAdditionalParts;

	//! Material List
	MaterialContainer m_kMaterialList;

	//! Material List's Iterator;
	MaterialContainer::iterator m_kMaterialItr;

	//! 무기의 검기 이펙트
	AttackEffMap	m_kAttackEffMap;

	PgBase_Item	m_kItemInfo;

	ParticleVec	m_kParticles;

	std::string		m_kDamageEffNormal;
	std::string		m_kDamageEffCritical;

	HairNodeVec	m_kHairNodes;
	
	SPOTParticleInfo m_kPOTParticleInfo;

	typedef std::list<std::string>			StringCont;
	StringCont		m_kHideNodeContainer;
private :
	static int m_iStatusEffectIDCount;

	static int GenerateStatusEffectID();

};

extern char const * const pcPtHair;
extern char const * const pcPtHead;
extern char const * const pcPtChest;
extern char const * const pcPtLeftHand;
extern char const * const pcPtRightHand;
extern char const * const pcPtLeftForearm;
extern char const * const pcPtRightForearm;
extern char const * const pcPtShield;
extern char const * const pcCharRoot;
extern char const * const pcPtLeftHand2;
extern char const * const pcPtRightHand2;
extern char const * const pcHairNodePropertyID;
#endif // FREEDOM_DRAGONICA_RENDER_WORDOBJECT_ITEM_PGITEMEX_H