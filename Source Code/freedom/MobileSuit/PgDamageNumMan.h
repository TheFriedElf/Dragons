#ifndef FREEDOM_DRAGONICA_UI_PGDAMAGENUMMAN_H
#define FREEDOM_DRAGONICA_UI_PGDAMAGENUMMAN_H
#include <string>
#include <map>
#include <list>
#include "PgIWorldObject.h"

class XUI::CXUI_FontManager;
class PgRenderer;
class PgActor;

// 캐릭터 머리 위에 대미지 숫자를 보여주는 클래스
class PgDamageNumMan	:	public PgIWorldObject
{
public :
	enum	SIMPLE_TEXT_TYPE
	{
		STT_MISS=0,
		STT_DODGE,
		STT_BLOCK,
//		STT_DEFENCE_IGNORE,
	};

	enum	COLOR
	{
		C_RED = 0,	//빨강
		C_BLUE,		//파랑
		C_YELLOW,	//노랑
		C_GREEN,	//녹색
		C_ORANGE,	//주황
		C_WHITE,
	};
	
	static bool m_sbInitExpVertexSetting;

public:
	PgDamageNumMan()		{	Init();	}
	virtual	~PgDamageNumMan()	{	Destroy();	}

	void	Init();
	void	Destroy();

	void	DeleteAllNum();

	void	AddNewNum(int const iNumber, NiPoint3 const &vLoc,bool const bRedColor,bool const bClampScreen = false, bool const bCritical = false, int const iEnchantLevel = 0, int const iExceptAbil = 0);

	void	AddNewSkillText(int iTextType,bool bIsUP,NiPoint3 const &vLoc);
	void	AddNewSimpleText(int iTextType,NiPoint3 const &vLoc);
	void	AddNewExpNum(int iNumber,NiPoint3 const &vLoc);
	void	AddNewHpMp(int iNumber,NiPoint3 const &vLoc, bool bHp);
	void	AddNewSmallNum(int const iNumber,NiPoint3 const &vLoc,bool const bClampScreen = false, bool const  bCritical = false, BYTE const btColor = C_WHITE, int const iEnchantLevel = 0, int const iExceptAbil = 0);
	void	AddNewKillCountText( int const iTextType, PgActor *pkActor );
	void	AddNewMissionText(int iTextType, NiPoint3 const &vLoc);

public:
	virtual bool ProcessAction(PgAction *pkAction,bool bInvalidateDirection=false,bool bForceToTransit = false)
	{
		return true;
	}
	virtual bool Update(float fAccumTime, float fFrameTime);//! WorldObject를 fFrameTime(AccumTime)에 대한 시각으로 갱신

	virtual void Draw(PgRenderer *pkRenderer, NiCamera *pkCamera, float fFrameTime);	//! pkRenderer를 이용해서 Draw
	virtual bool ParseXml(const TiXmlNode *pkNode, void *pArg = 0, bool bUTF8 = false){return true;}
private:
	NiDeclareRTTI;
	struct	stDamageNumNode	:	public	NiMemObject
	{
		enum	TYPE
		{// 배열을 만들고 접근하는 값으로도 사용함. 순차적 증가를 끊지 말것
			T_NUM				= 0,
			T_SKILL_TEXT		= 1,
			T_SIMPLE_TEXT		= 2,
			T_EXP_NUM			= 3,
			T_HP_OR_MP			= 4,
			T_CRITICAL			= 5,
			T_SMALL_NUM			= 6,
			T_KILL_COUNT_TEXT	= 7,
			T_MISSION_TEXT		= 8,
			T_ENCHANT_LEVEL		= 9,
			T_MAX
		};
		
		enum eState : BYTE
		{
			ES_START = 0,	// 축소되는 상태
			ES_STOP = 1,	// 멈춰있는 상태
			ES_END = 2,		// 올라가면서 사라지는 상태
		};
		TYPE	m_Type;

		NiAVObjectPtr	m_spNif;
		BYTE	m_byState;	//	eState의 상태
		int	m_iNumber;
		NiPoint3	m_vStartPos;
		NiPoint3	m_vTargetPos;
		float	m_fCreateTime;	//	생성된 시간(스케일링 시작 시간)
		float	m_fMoveStartTime;	//	이동 시작 시간
		float	m_fHoldingStartTime;
		bool	m_bClampScreen;	//	숫자가 화면 밖으로 벗어날 경우, 화면 안으로 조정해주는 기능.
		float	m_fScaleTime;
		float	m_fMoveTime;

		stDamageNumNode()
		{
			m_Type = T_NUM;
			m_byState = 0;
			m_fScaleTime = 0.0f;
			m_spNif = NULL;
			m_bClampScreen = false;
			m_fMoveTime = 0.0f;
		}
		~stDamageNumNode()
		{
			m_spNif = NULL;
		}
	};
	typedef std::list<stDamageNumNode*> CONT_NODE;
	typedef std::vector<float> CONT_FLOAT;
private:
	void	SetNumber(NiNode *pNifNode,int iNumber); //! 버텍스의 U,V 를 조정해서 숫자를 맞춘다.
	void	SetAlpha(NiAVObject *pkObject,float fAlpha);
	void	SetSkillText(NiNode *pNifNode,int iTextType,bool bIsUP);
	void	SetExpNumber(NiNode *pNifNode,int iNumber, bool bExp = false);
	void	SetHpMpNumber(NiNode *pNifNode,int iNumber);
	void	SetCriticalNumber(NiNode *pNifNode,int iNumber);
	void	SetSmallNumber(NiNode *pNifNode,int iNumber);
	void	SetKillCountText(NiNode *pNifNode, int const iKillCountType );

	void	ClampScreen(stDamageNumNode *pNode);
	void	AddDamageNumNode(stDamageNumNode::TYPE eType, NiNodePtr pkNode, int iNumber, NiPoint3 vStartPos, float fCreateTime, bool bClampScreen, bool bNifUpdate, NiPoint3 const &vTargetPos );

	float	GetScaleTime(stDamageNumNode::TYPE eType) { if( eType >= m_kCont_ScaleTime.size() ) {return 0.0f;} return m_kCont_ScaleTime.at(static_cast<int>(eType));}
	float	GetMoveTime(stDamageNumNode::TYPE eType) { if( eType >= m_kCont_MoveTime.size() ) {return 0.0f;} return m_kCont_MoveTime.at(static_cast<int>(eType));}

private:
	float	m_fStartScale,m_fTargetScale;	//	얼마나 스케일링 시킬 것인가
	float	m_fHoldingTime;	//	축소된 후 멈춰있는 시간	
	float	m_fMoveSpeed;	//	이동 속도
	float	m_fAccumTimeSaved;	
	float	m_fMoveTargetScale;	//	이동 후의 스케일

	CONT_FLOAT m_kCont_ScaleTime;		//  축소되는 시간이자, 나타나는 시간(alpha)
	CONT_FLOAT m_kCont_MoveTime;	//	이동 후의 스케일이자, 사라지는 시간(alpha)

	bool	m_bIsRedColor;

	CONT_NODE	m_NodeList;	

	NiSourceTexturePtr	m_spRedNum,m_spYellowNum,m_spSkillTextTex,m_spSimpleTextTex, m_spExpTex, m_spHpTex, m_spMpTex, m_spCriticalTex, m_spWhiteNum2, m_spGreenTex,m_spRedNumSmall;
	NiSourceTexturePtr	m_spKillCountTex;
	NiSourceTexturePtr	m_spMissionTex;
	NiSourceTexturePtr	m_spCriticalSubTex1,m_spCriticalSubTex2;
};

namespace PgDamageNumManUtil
{
	bool SetBaseTexture(NiNodePtr spDamageNumNif, char const* const pcNodeName, NiSourceTexturePtr spTexture);
}

#endif // FREEDOM_DRAGONICA_UI_PGDAMAGENUMMAN_H