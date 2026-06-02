#pragma once

///////////////////////////
// temporary
namespace ActorState
{
	enum
	{ 
		IDLE = 0, 
		WALK, 
		RUN, 
		JUMP,		// physics..
		SKILL 
	};
}
////////////////////////////

class CActor;
class CWorldMan;

class CPilot : public NiMemObject
{
public:
	CPilot(CWorldMan *pkWorldMan, CActor *pkActor);
	~CPilot();

	void Update(float fTime);
	void Draw(float fTime);

	//// Path
	//
	bool FindPathNormal();
	void ConcilDirection();

	// set/get
	void SetVelocity();
	void SetDirection(NiPoint2 ptDir);

	CWorldMan *GetWorldMan()
	{
		return m_pkWorldMan;
	}

	void SetWorldMan(CWorldMan *pkWorldMan)
	{
		m_pkWorldMan = pkWorldMan;
	}

	CActor *GetActor()
	{
		return m_pkActor;
	}

	void SetActor(CActor *pkActor)
	{
		m_pkActor = pkActor;
	}

	bool ToLeft()
	{
		return m_bToLeft;
	}

	void ToLeft(bool bLeft)
	{
		m_bToLeft = bLeft;
	}

protected:
	CActor *m_pkActor;

	//// Path
	//
	NiPoint3 m_kPathNormal;
	bool m_bToLeft;

	CWorldMan *m_pkWorldMan;
};