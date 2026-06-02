#ifndef FREEDOM_DRAGONICA_CONTENTS_WORLDACTION_PGWORLDACTIONOBJECT_H
#define FREEDOM_DRAGONICA_CONTENTS_WORLDACTION_PGWORLDACTIONOBJECT_H

#include "NiMain.h"
#include "lwPacket.h"

class	PgWorldActionMan;
class PgWorldActionObject : public NiMemObject
{
	enum	WorldActionParamType
	{
		WAPT_UNKNOWN=0,
		WAPT_INT,
		WAPT_FLOAT,
		WAPT_STRING,
		WAPT_GUID
	};

	struct	stWorldActionParam
	{
		WorldActionParamType	Type;
		stWorldActionParam()	{	Type = WAPT_UNKNOWN;	}
		virtual	~stWorldActionParam(){}
	};
	struct	stWAPT_FLOAT	:	public	stWorldActionParam
	{
		float	m_fValue;
		stWAPT_FLOAT()	{	Type = WAPT_FLOAT;	}
		virtual	~stWAPT_FLOAT(){}
	};
	struct	stWAPT_INT	:	public	stWorldActionParam
	{
		int	m_iValue;
		stWAPT_INT()	{	Type = WAPT_INT;	}
		virtual	~stWAPT_INT(){}
	};
	struct	stWAPT_STRING	:	public	stWorldActionParam
	{
		std::string	m_Value;
		stWAPT_STRING()	{	Type = WAPT_STRING;	}
		virtual	~stWAPT_STRING(){}
	};
	struct	stWAPT_GUID	:	public	stWorldActionParam
	{
		BM::GUID	m_Value;
		stWAPT_GUID()	{	Type = WAPT_GUID;	}
		virtual	~stWAPT_GUID(){}
	};


	PgWorldActionMan	*m_pWorldActionMan;
	int m_iCurrentStage;
	int	m_iWorldActionID;
	int m_iElapsedTimeAtStart;	//	월드 액션이 시작되었을 때, 이미 흐른 시간(월드 액션이 시작된 중간에 클라이언트가 등장한 것이다)
	unsigned	long	m_ulStartTime;
	float m_fStartTime;

	typedef std::map<unsigned int, stWorldActionParam*> ParamContainer;
	typedef std::map<std::string, std::string> StringParamContainer;

	ParamContainer m_Params;
	StringParamContainer m_kStringParamContainer;

public:
	
	PgWorldActionObject(PgWorldActionMan *pMan, int iWorldActionID);
	virtual	~PgWorldActionObject()	{	Destroy();	}

	//	스크립트 호출
	bool OnUpdate(float fFrameTime,float fAccumTime);	//	return이 false일 경우, Leave 스크립트를 실행하고, 이 오브젝트를 제거시킨다.
	void OnEnter(int iElapsedTime);
	void OnLeave();
	void OnPacketReceive(lwPacket &Packet);
	int	 GetCurrentStage();
	void NextStage();
	DWORD GetDuration();

	//	파라메터 추가
	void SetParam(int iID,int iValue);
	void SetParam(int iID,float fValue);
	void SetParam(int iID,std::string Value);
	void SetParam(int iID,BM::GUID Value);
	void SetParam(char const *pcKey, char const *pcValue);

	int	GetParamInt(int iID);
	float GetParamFloat(int iID);
	std::string const &GetParamString(int iID);
	BM::GUID const &GetParamGUID(int iID);
	char const *GetParam(char const *pcKey);

	int	GetElapsedTime(float fAccumTime);//	{	return	(BM::GetTime32()-m_ulStartTime)+m_iElapsedTimeAtStart;	}

private:

	void	Init(PgWorldActionMan *pMan,int iWorldActionID);
	void	Destroy();
};

#endif // FREEDOM_DRAGONICA_CONTENTS_WORLDACTION_PGWORLDACTIONOBJECT_H