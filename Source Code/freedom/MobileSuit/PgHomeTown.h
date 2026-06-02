#ifndef FREEDOM_DRAGONICA_CONTENTS_HOMETOWN_PGHOMETOWN_H
#define FREEDOM_DRAGONICA_CONTENTS_HOMETOWN_PGHOMETOWN_H

#include <vector>
#include "PgHomeUI.h"

typedef struct tagTblDefMap TBL_DEF_MAP;
class PgHomeTown
	: public PgHomeUI
{
public:
	typedef std::vector<TBL_DEF_MAP const *> HomeTownList;
	size_t const GetHomeTownCount() const { return m_kHomeTownList.size(); }
	int GetHomeTownMapNoAt(size_t const iAt) const;
	void Build();	//초기에 홈타운 리스트를 만들자
private:
	
	void GetFirstHomeTown(HomeTownList::const_iterator & rkIter);
	TBL_DEF_MAP const * GetNextHomeTown(HomeTownList::const_iterator & rkIter) const;

	HomeTownList m_kHomeTownList;
};

#define g_kHomeTownMgr SINGLETON_STATIC(PgHomeTown)

#endif // FREEDOM_DRAGONICA_CONTENTS_HOMETOWN_PGHOMETOWN_H