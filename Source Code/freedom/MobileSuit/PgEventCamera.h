#ifndef FREEDOM_DRAGONICA_CONTENTS_EVENTMODE_PGEVENTCAMERA_H
#define FREEDOM_DRAGONICA_CONTENTS_EVENTMODE_PGEVENTCAMERA_H

#include "CreateUsingNiNew.inl"

class PgEventCamera
{
public:
	PgEventCamera();
	~PgEventCamera();

	NiCamera* FindCamera(NiAVObject* pObject);
	void InsertEventObject(std::string strName, NiNode* pkNode);
	void ClearEventObject();

	NiNode* GetEventObject(std::string strName);
	NiCamera* GetCamera(std::string strName);

	bool StartAnimation(std::string strName);
	bool StopAnimation(std::string strName);

protected:
	typedef std::map<std::string, NiNode*> ContEventNode;
	ContEventNode m_smEventNode;
};

#define g_kEventObject SINGLETON_CUSTOM(PgEventCamera, CreateUsingNiNew)

#endif // FREEDOM_DRAGONICA_CONTENTS_EVENTMODE_PGEVENTCAMERA_H