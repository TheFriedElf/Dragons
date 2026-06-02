#ifndef FREEDOM_DRAGONICA_CONTENTS_WORLDEVENTSYSTEM_PGWEOBEJCTSTATE_H
#define FREEDOM_DRAGONICA_CONTENTS_WORLDEVENTSYSTEM_PGWEOBEJCTSTATE_H
#include "variant/PgWorldEventObject.h"

namespace NiAVObjectUtil
{
	struct FindPuppetNode
	{
		FindPuppetNode(std::string const& rkObjectName, NiAVObject*& pkFindReturn);
		FindPuppetNode(FindPuppetNode const& rhs);
		bool operator() (NiNode* pkRootNode);

	private:
		std::string const& m_rkObjectName;
		NiAVObject*& m_pkFindReturn;
	};
};

class	PgWEObjectState	:	public	PgWEClientObjectState
{

public:

	enum	OBJECT_TYPE
	{
		OT_UNKOWN			= 0,
		OT_PUPPET,
		OT_RIDABLE_OBJECT,
		OT_CAMERA,
		OT_MAPOBJECT,
		OT_NPC,
	};

private:

	OBJECT_TYPE	m_kObjectType;
	BM::GUID	m_kObjectGUID;
	std::string	m_kObjectName;

public:

	PgWEObjectState();
	explicit PgWEObjectState(PgWEObjectState const& rhs);
	virtual	~PgWEObjectState();

	virtual	bool SetStatus(int const iNewState);
	virtual bool Parse(TiXmlElement const* pkNode);

protected:
	virtual void OnStateChange();

private:

	void	ValidateXmlData();
};

#endif // FREEDOM_DRAGONICA_CONTENTS_WORLDEVENTSYSTEM_PGWEOBEJCTSTATE_H