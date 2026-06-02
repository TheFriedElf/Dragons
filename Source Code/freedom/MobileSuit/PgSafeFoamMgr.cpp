#include "stdafx.h"
#include "PgMobileSuit.h"
#include "PgSafeFoamMgr.h"

//int const SAFE_FOAM_TIME_LIMIT = 36000000;

//TCHAR const *szSAFE_FOAM_FRM = _T("SFRM_MSG_COMMON_CANCEL");
//float const PgSafeFoamMgr::fUpTime = SAFE_FOAM_TIME_LIMIT * 0.0001f;

PgSafeFoamMgr::PgSafeFoamMgr()
{
//	Clear();
	RemoveFollow();
}

PgSafeFoamMgr::~PgSafeFoamMgr()
{
}

/*
void PgSafeFoamMgr::Clear()
{
	fEndTime = 0.f;
}

void PgSafeFoamMgr::SetSafeFoamUsed()
{
	if( fEndTime > 0 ) 
	{
		return;
	}
	float const StartTime = g_pkApp->GetAccumTime();
	fEndTime = StartTime + fUpTime;
}

void PgSafeFoamMgr::Update()
{
	float const fCurTime = g_pkApp->GetAccumTime();
	float const fDiffTime = fEndTime - fCurTime;
	if( 0 > fDiffTime )
	{
		return;
	}

	std::wstring const kCurWndName(szSAFE_FOAM_FRM);

	XUI::CXUI_Wnd *pkTopWnd = XUIMgr.Get( kCurWndName );
	if( !pkTopWnd )
	{
		return;
	}

	XUI::CXUI_Wnd *pkTimeWnd = pkTopWnd->GetControl(_T("SFRM_TIMER_COUNT"));
	if( pkTimeWnd )
	{
		pkTimeWnd->SetCustomData(&fEndTime, sizeof(float));
	}

	pkTopWnd->SetCustomData(0, sizeof(size_t));
}

void PgSafeFoamMgr::CheckSafeFoam()
{
	float const fNowTime = g_pkApp->GetAccumTime();

	float const fDiffTime = fEndTime - fNowTime;
	if( 0 < fDiffTime && fEndTime )
	{
		Update();
	}
	else
	{
		return;
	}
}
*/

bool PgSafeFoamMgr::IsFollow()
{
	return (FollowGuid() != BM::GUID::NullData());
}

void PgSafeFoamMgr::SetFollow(const BM::GUID &kFollowGUID)
{
	FollowGuid(kFollowGUID);
}

void PgSafeFoamMgr::RemoveFollow()
{
	FollowGuid(BM::GUID::NullData());
}

BM::GUID const &PgSafeFoamMgr::GetFollowGuid()
{
	return FollowGuid();
}

void lwSafeFoam::RegisterWrapper(lua_State *pkState)
{
	using namespace lua_tinker;
//	def(pkState, "CheckSafeFoam", CheckSafeFoam);

	def(pkState, "SetFollow", SetFollow);
	def(pkState, "RemoveFollow", RemoveFollow);
	def(pkState, "IsFollow", IsFollow);	
	def(pkState, "GetFollowGuid", GetFollowGuid);	
}

// void lwSafeFoam::CheckSafeFoam()
// {
// 	g_kSafeFoamMgr.CheckSafeFoam();
// }

void lwSafeFoam::SetFollow(lwGUID kFollowGUID)
{
	g_kSafeFoamMgr.SetFollow(kFollowGUID());
}

void lwSafeFoam::RemoveFollow()
{
	g_kSafeFoamMgr.RemoveFollow();
}

bool lwSafeFoam::IsFollow()
{
	return g_kSafeFoamMgr.IsFollow();
}

lwGUID lwSafeFoam::GetFollowGuid()
{
	return lwGUID(g_kSafeFoamMgr.GetFollowGuid());
}