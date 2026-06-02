#ifndef FREEDOM_DRAGONICA_CONTENTS_EVENTSCRIPTSYSTEM_PGEVENTSCRIPTINSTANCE_H
#define FREEDOM_DRAGONICA_CONTENTS_EVENTSCRIPTSYSTEM_PGEVENTSCRIPTINSTANCE_H

#include "PgEventScriptData.H"

class	PgEventScriptInstance
{

	PgEventScriptData	*m_pkData;
	int	m_iNextCmdIndex;

	PgEventScriptCmd::ParamMap	m_kCmdCustomParamMap;

public:

	static	PgEventScriptInstance*	Create(int iEventID);

public:

	PgEventScriptInstance(PgEventScriptData *pkData);
	virtual	~PgEventScriptInstance();

	virtual	bool	Update(float fAccumTime,float fFrameTime);

	int	GetEventID()	const
	{
		if(!m_pkData)
		{
			return	-1;
		}

		return	m_pkData->GetEventID();
	}
};

#endif // FREEDOM_DRAGONICA_CONTENTS_EVENTSCRIPTSYSTEM_PGEVENTSCRIPTINSTANCE_H