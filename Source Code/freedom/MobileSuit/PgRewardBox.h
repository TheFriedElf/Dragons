#ifndef FREEDOM_DRAGONICA_RENDER_REWARDBOX_PGREWARDBOX_H
#define FREEDOM_DRAGONICA_RENDER_REWARDBOX_PGREWARDBOX_H
#include "PgNetwork.h"
#include "PgRenderer.h"

class PgRewardBox
{
public:
	PgRewardBox();
	~PgRewardBox();

	void Init();
	void Destroy();

	virtual bool Update(float const fAccumTime, float const fFrameTime);
	virtual void Draw(PgRenderer *pkRenderer, float const fFrameTime);
	void SetTranslate(NiPoint3 const &kTrn);
	NiPoint3 GetTranslate() const;
	void SetRotate(NiMatrix3 const& kRot);
	NiMatrix3 GetRotate() const;
	void SetOwnerName(std::wstring const& wstrName) { m_wstrOwner = wstrName; }
	bool Picked(NiPoint3 const& rkOrgPt, NiPoint3 const& rkRayDir);	// 이놈이 마우스에 걸렸는지
	NiAlphaProperty *RecursiveProcessAlpha(NiAVObject* pkObject);
	typedef enum eBoxState
	{
		E_REWORD_BOX_NONE					= 0x00,	
		E_REWORD_BOX_DROP					=	0x01,	
		E_REWORD_BOX_IDLE					= 0x02,
		E_REWORD_BOX_OVER					= 0x04,
		E_REWORD_BOX_SELECTING		= E_REWORD_BOX_IDLE|E_REWORD_BOX_OVER,
		E_REWORD_BOX_SELECT				= 0x08,
		E_REWORD_BOX_OPEN					= 0x10,
		E_REWORD_BOX_DISABLE				= 0x20,
	}E_REWORD_BOX_STATE; 

	void BoxState(E_REWORD_BOX_STATE const eState);
	E_REWORD_BOX_STATE BoxState() const	{ return m_kBoxState; }

	void FaceToCamera(NiPoint3 const& rkRightVec);
private:

	int					m_iNumber;
	std::wstring		m_wstrOwner;
	NiPoint3			m_kPos;
	NiQuaternion		m_kRot;
	NiActorManagerPtr	m_spAM;
	NiAVObject			*m_pkNIFRoot;
	E_REWORD_BOX_STATE	m_kBoxState;

	NiAVObjectPtr		m_spObject;	//파티클용 스마트 포인터.

	CLASS_DECLARATION_S(float, TimeStamp);
	CLASS_DECLARATION_S(float, StartTime);
	CLASS_DECLARATION_S(float, RotDegree);
	CLASS_DECLARATION_S(bool, UpDown);
	CLASS_DECLARATION_S(NiPoint3, UpDownVec);
};
#endif // FREEDOM_DRAGONICA_RENDER_REWARDBOX_PGREWARDBOX_H