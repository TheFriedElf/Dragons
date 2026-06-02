#include "stdafx.h"
#include "PgEventScriptDataMan.H"


PgEventScriptDataMan	g_kEventScriptDataMan;

void	PgEventScriptDataMan::Init()
{
}
void	PgEventScriptDataMan::Terminate()
{
	ReleaseAll();
}
PgEventScriptData*	PgEventScriptDataMan::GetEventScriptData(int iEventID)
{

	//	이미 로딩해 놨는지 체크.
	int	iTotal = m_kEventScriptDataCont.size();

	for(int i=0;i<iTotal;i++)
	{
		PgEventScriptData *pkData = m_kEventScriptDataCont[i];
		if(pkData->GetEventID() == iEventID)
		{
			return	pkData;
		}
	}

	PgEventScriptData	*pkNewData = new PgEventScriptData();
	
	if(pkNewData->LoadFromFile(iEventID) == false)	//	로딩 실패
	{
		_PgMessageBox("GetEventScriptData Failed","Event %d LoadFromFile Failed.",iEventID);
		SAFE_DELETE(pkNewData);
		return NULL;
	}

	m_kEventScriptDataCont.push_back(pkNewData);

	return	pkNewData;
}
void	PgEventScriptDataMan::ReloadAll()	//	실행중인 스크립트는 재로딩 할 수 없다.
{
	int	iTotal = m_kEventScriptDataCont.size();

	for(int i=0;i<iTotal;i++)
	{
		PgEventScriptData *pkData = m_kEventScriptDataCont[i];
		pkData->Reload();
	}
}
void	PgEventScriptDataMan::Reload(int iEventID)	//	실행중인 스크립트는 재로딩 할 수 없다.
{
	int	iTotal = m_kEventScriptDataCont.size();

	for(int i=0;i<iTotal;i++)
	{
		PgEventScriptData *pkData = m_kEventScriptDataCont[i];
		if(pkData->GetEventID() == iEventID)
		{
			pkData->Reload();
			return;
		}
	}
}
void	PgEventScriptDataMan::ReleaseAll()	//	모든 스크립트를 메모리 해제시킨다.
{
	int	iTotal = m_kEventScriptDataCont.size();

	for(int i=0;i<iTotal;i++)
	{
		SAFE_DELETE(m_kEventScriptDataCont[i]);
	}

	m_kEventScriptDataCont.clear();
}
