#ifndef FREEDOM_DRAGONICA_CONTENTS_SAFEFOAM_PGSAFEFOAMMGR_H
#define FREEDOM_DRAGONICA_CONTENTS_SAFEFOAM_PGSAFEFOAMMGR_H

#include "lwGUID.h"
class	lwGUID;

class PgSafeFoamMgr
{
public:
	PgSafeFoamMgr();
	~PgSafeFoamMgr();

// 	void	Clear();
// 	void	SetSafeFoamUsed();
// 	void	CheckSafeFoam();

	bool	IsFollow();
	void    SetFollow(const BM::GUID &kFollowGUID);
	void	RemoveFollow();
	BM::GUID const &PgSafeFoamMgr::GetFollowGuid();
protected:
//	void	Update();
private:
//	static float const fUpTime;
//	float fEndTime;	

	CLASS_DECLARATION_S(BM::GUID, FollowGuid);
};

namespace lwSafeFoam
{
	void RegisterWrapper(lua_State *pkState);
//	void CheckSafeFoam();
	bool IsFollow();
	void SetFollow(lwGUID kFollowGUID);
	void RemoveFollow();
	lwGUID GetFollowGuid();
}

#define g_kSafeFoamMgr SINGLETON_STATIC(PgSafeFoamMgr)

#endif // FREEDOM_DRAGONICA_CONTENTS_SAFEFOAM_PGSAFEFOAMMGR_H