#ifndef FREEDOM_DRAGONICA_RENDER_RENDERER_PGAVOBJECTALPHAPROCESSMANAGER_H
#define FREEDOM_DRAGONICA_RENDER_RENDERER_PGAVOBJECTALPHAPROCESSMANAGER_H

#include <list>

class NiAVObject;
class PgRenderer;

enum eAlphaManagerAlphaProcessType
{
	ALPHA_MANAGER_PROCESS_TYPE_ALPHA_ADD = 0,		//알파 증가
	ALPHA_MANAGER_PROCESS_TYPE_ALPHA_SUB = 1,		//알파 감소
};

enum eAlphaManagerObjectProcessType
{
	ALPHA_MANAGER_PROCESS_TYPE_OBJECT_DELETE = 0,   //알파 처리후 삭제
	ALPHA_MANAGER_PROCESS_TYPE_OBJECT_MAINTAIN = 1, //알파 처리후 유지(아무일도 하지 않음)
};

typedef struct tagObjectProcessInfo
{
	NiAVObjectPtr	m_spAVObject;
	float			m_fStartTime;
	float			m_fCurTime;
	float			m_fEndTime;
	BYTE			m_cAlphaProcessType;
	BYTE			m_cObjectProcessType;
	bool			m_bDetach;	
} ObjectProcessInfo;

//! 오브젝트들의 알파를 처리하는 매니저
//! 월드에 있는 오브젝트들이 시야에 벗어나면서 사라지거나 파티클이 떨어질때 자연스럽게 처리 되도록 해준다.

class PgAVObjectAlphaProcessManager	:	public	NiMemObject
{
public :
	typedef std::list<ObjectProcessInfo>	ObjectInfoContainer;
	typedef ObjectInfoContainer::iterator	ObjectInfoItor;

	typedef std::list<NiNode*>				NodeContainer;

	typedef std::map<NiAVObject*, NiAlphaPropertyPtr>	ObjectAlphaContainer;
	ObjectAlphaContainer::iterator						ObjectAlphaItor;

public :
	PgAVObjectAlphaProcessManager();
	virtual ~PgAVObjectAlphaProcessManager();

public :
	void Release();
	//! 알파 처리가 필요한 오브젝트를 등록한다. 오브젝트 / 끝나는 시간 / 알파를 증가/감소 /알파 처리후 삭제/유지
	void AddAVObject(NiAVObject* pkAVObject, float const fEndTime, BYTE const cAlphaProcessType, BYTE const cObjectProcessType, bool const bOverride = false, bool const bDetach = false);

	//! 알파 처리가 필요한 오브젝트들을 업데이트 한다.
	virtual void Update(float fAccumTime,float fFrameTime);

	//! 알파 처리가 필요한 오브젝트들을 렌더링 한다.(부모가 없는 오브젝트들만 렌더링 됨)
	virtual void Draw(PgRenderer* pkRenderer, NiCamera* pkCamera, float fFrameTime, NiVisibleArray& kVisibleScene);

	//! 알파 처리가 끝난 오브젝트들을 저장하는 리스트(알파가 0이 되는 경우)
	void SetAppCulledToTrueMainTainList(NodeContainer* pkMainMTainContainer);
	//! 알파 처리가 끝난 오브젝트들을 저장하는 리스트(알파가 1이 되는 경우)
	void SetAppCulledToFalseMainTainList(NodeContainer* pkMainMTainContainer);

protected :
	ObjectInfoContainer				m_kObjectInfoContainer; // 알파처리 되는 오브젝트들의 정보를 저장하는 컨테이너

	//외부에서 세팅하여 사용한다. SetAppCulledToTrueMainTainList / SetAppCulledToFalseMainTainList 사용 하여 세팅한다.
	NodeContainer*					m_pkCulledToTrueMainTainContainer;	//컬링이 되어야하는 오브젝트들이 들어가는 컨테이너
	NodeContainer*					m_pkCulledToFalseMainTainContainer; //컬링이 되지 말아야 하는 오브젝트들이 들어가는 컨테이너	
	
	ObjectAlphaContainer			m_kObjectAlphaContainer; // 기존의 AlphaProperty를 저장하는 컨테이너


private :
	// 알파 프로퍼티가 없을 경우를 체크하여 프로퍼티를 추가해 준다.
	void AddAlphaProperty(NiAVObject* pkAVObject, ObjectAlphaContainer& m_kObjectAlphaContainer);
	// 원래의 알파 프로퍼티로 복구해 준다.
	void RestoreAlphaProperty(NiAVObject* pkAVObject, ObjectAlphaContainer& m_kObjectAlphaContainer);
	// 알파를 변경 해준다.
	void SetChangeAlpha(NiAVObject* pkAVObject, float const fAlpha, BYTE const cAlphaProcessType, bool const bSetForceAlpha = false);
	// 알파 프로세스 정보를 리스트에 추가한다.
	void AddObjectProcessInfo(NiAVObject* pkAVObject, float const fEndTime, BYTE const cAlphaProcessType, BYTE const cObjectProcessType, bool const bDetach);
	// AlphaTest 값을 변경
	void SetChangeAlphaTest(NiGeometry* pkGeometry, float const fAlpha);

};

#endif // FREEDOM_DRAGONICA_RENDER_RENDERER_PGAVOBJECTALPHAPROCESSMANAGER_H