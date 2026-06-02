#include "stdafx.h"
#include "Variant/Global.h"
#include "Variant/PgJobSkill.h"
#include "PgJobSkillLearn.h"
#include "PgPilotMan.h"
#include "ServerLib.h"

namespace JobSkillGuide {
	// local function
	bool GetGuideItemName(int const iSaveIdx, std::wstring& rkOut) {
		CONT_DEF_JOBSKILL_SAVEIDX const* pkDefJobSkillSaveIdx = NULL;
		g_kTblDataMgr.GetContDef(pkDefJobSkillSaveIdx);
		CONT_DEF_JOBSKILL_SAVEIDX::const_iterator iter = pkDefJobSkillSaveIdx->find(iSaveIdx);
		if( pkDefJobSkillSaveIdx->end() != iter )
		{
			if( ::GetItemName((*iter).second.iBookItemNo, rkOut) )
			{
				return true;
			}
		}
		return true;
	}
}

void PgJobSkill_NfySaveIdx::Push(int const iSaveIdx)
{ 
	m_kContSaveIdx.push(iSaveIdx); 
}
int PgJobSkill_NfySaveIdx::Pop() 
{ 
	if( Empty() )
	{
		return 0;
	}
	int const iSaveIdx = m_kContSaveIdx.front(); 
	m_kContSaveIdx.pop();
	return iSaveIdx;
}
bool PgJobSkill_NfySaveIdx::Empty() const
{ 
	return m_kContSaveIdx.empty(); 
}
void PgJobSkill_NfySaveIdx::Clear()
{
	while( !m_kContSaveIdx.empty() )
	{
		m_kContSaveIdx.pop();
	}
}
