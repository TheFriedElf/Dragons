#ifndef FREEDOM_DRAGONICA_RENDER_SHINESTONE_PGSHINESTONE_H
#define FREEDOM_DRAGONICA_RENDER_SHINESTONE_PGSHINESTONE_H

#include "PgIWorldObject.h"

char const* const SHINESTONE_PREFIX_STRING = "ShineStone_ID_";

class PgActor;
class PgCircleShadow;

class PgShineStone : public PgIWorldObject
{
	NiDeclareRTTI;
public:
	typedef enum
	{
		CT_TOUCH,
		CT_ACTION,
		CT_SCRIPT,
	} ConditionType;

	typedef enum estonestate
	{
		E_STONE_NONE = 0,
		E_STONE_JUMP,
		E_STONE_IDLE,
		E_STONE_EAT,
	}E_STONE_STATE ;

public:
	PgShineStone();
	virtual ~PgShineStone();

	bool Initialize();
	//! PgIWorldObject 재정의
	virtual void Terminate();
	virtual bool BeforeUse() { return true; };
	virtual bool BeforeCleanUp() { return true; };

	virtual bool Update(float fAccumTime, float fFrameTime);
	virtual void UpdateDownwardPass(float fTime, bool bUpdateControllers);

    virtual void Draw(PgRenderer *pkRenderer, NiCamera *pkCamera, float fFrameTime) { /* hook-method */ };
	virtual void DrawImmediate(PgRenderer *pkRenderer, NiCamera *pkCamera, float fFrameTime);

	virtual bool ProcessAction(PgAction *pkAction,bool bInvalidateDirection=false,bool bForceToTransit  = false){	return true;	}

	virtual bool ParseXml(const TiXmlNode *pkNode, void *pArg = 0, bool bUTF8 = false);

	//! 스크립트를 호출한다.
	bool OnEnter(PgActor *pkActor);
	bool OnUpdate(PgActor *pkActor);
	bool OnLeave(PgActor *pkActor);
	bool OnAction(PgActor *pkActor);

	std::string &GetConditionAction();
	ConditionType	GetConditionType()	{	return	m_eConditionType;	}
	E_STONE_STATE	GetStoneState()	const { return m_eStoneState; }
	
	CLASS_DECLARATION_S(bool, Enable);

	void ChaseOwner();

	int	GetShineStoneTitleTextID()	{	return	m_iShineStoneTitleTextID;	}
	const	NiPoint3&	GetTitleTextAdjust()	{	return	m_ptTitleTextAdjust;	}
	float	GetShineStoneTitleTextScale()	{	return	m_kShineStoneTitleTextScale;	}
	const	NiColorA	&GetShineStoneTitleTextColor()	{	return	m_kShineStoneTitleTextColor;	}
	const	std::wstring& GetShineStoneTitleTextFontName()	{	return	m_kTitleTextFontName;	}

	int const GetIndex() { return m_iIndex; }
	NiPoint3 GetPos() { return m_kPos; }
	float const GetRate() { return m_fRate; }
	void SetPos(POINT3 const &ptPos);

	void SetPhysXObj(NiPhysXScenePtr spPhysxObj) { m_spPhysXObject = spPhysxObj; }
	const NiPhysXScenePtr GetPhysXObj() { return m_spPhysXObject; }
	void SetItemNo(int iItemNo) { m_iItemNo = iItemNo; }
	int GetItemNo() { return m_iItemNo; }

	void ReserveGetShineStone(int iIndex);

protected:
	E_STONE_STATE			m_eStoneState;

	typedef enum eStoneEatState
	{
		E_EAT_NONE = 0,
		E_EAT_UP,
		E_EAT_UP_WAIT,
		E_EAT_CHASE,
		E_EAT_END,
	}EStoneEatState;
	EStoneEatState			m_eStoneEatState;

	static float		ms_fGravity;				//! Actor가 점프시에 영향을 받는 중력
	NiPoint3			m_kptOriginPos;
	float				m_fSinProgress;
	bool				m_bChaseOwner;
	float				m_fChaseStartTime;

	int	m_iShineStoneTitleTextID;
	NiColorA	m_kShineStoneTitleTextColor;
	float	m_kShineStoneTitleTextScale;
	NiPoint3	m_ptTitleTextAdjust;
	std::wstring	m_kTitleTextFontName;

	ConditionType m_eConditionType;
	std::string m_kConditionAction;
	std::string m_kScript;

	int m_iIndex;
	NiPoint3 m_kPos;
	float m_fRate;

	NiPhysXScenePtr m_spPhysXObject;

	bool m_bHideShadow;

	NiNodePtr m_spShineStone;
	NiPhysXScenePtr m_spShineStonePhysX;
	NiObjectList m_kShineStoneGeoList;

	NiNodePtr m_spDetailStone;
	NiNodePtr m_spParticleNode;	// Detail의 파티클.
	NiBillboardNode* m_pkBillboardNode;
	NiGeometry* m_pkDetailGeo;

	int m_iItemNo;
};
#endif // FREEDOM_DRAGONICA_RENDER_SHINESTONE_PGSHINESTONE_H