#ifndef FREEDOM_DRAGONICA_CONTENTS_JOBSKILL_LWJOBSKILLITEM_H
#define FREEDOM_DRAGONICA_CONTENTS_JOBSKILL_LWJOBSKILLITEM_H

#include "Variant/PgJobSkill.h"

namespace lwJobSkillItem
{
	void RegisterWrapper(lua_State *pkState);
	void lwCallJobSkillItemUI();
	void SetFilterItemType();
	void InsertFilterItem(XUI::CXUI_List* pSelectList, int iJobSkillNo, CONT_DEF_JOBSKILL_SKILL::mapped_type const &rkJobSkillInfo, bool const bGray);
	bool lwShowList(int iPage, int const iFilterType, bool const bShowOnlyLearn = false);
	bool lwNextPage();
	bool lwBeforePage();
	void lwClearAll();
	void lwClearList(XUI::CXUI_Wnd* pItem);
	void SetFilterTitle(std::wstring const& kTitle);
	void lwSetGatherType(std::wstring const& kText, int const iGatherType);
	bool GetLearnedSaveIndex(CONT_DEF_JOBSKILL_SAVEIDX& rkContSaveIndex, CONT_DEF_JOBSKILL_SAVEIDX& rkOutCont);
	bool FilterGatherType(CONT_DEF_JOBSKILL_SAVEIDX& rkContSaveIndex, int const iGatherType);
	void SetPageUI(int const iCurrent, int const iMax);
	void SetFilterText(BM::vstring& kText, int const iJobType);
	void SetIconInfo(XUI::CXUI_Wnd* pItem, CItemDef const *pDef, int const iSaveIndex);
	void SetTitleText(XUI::CXUI_Wnd* pItem, int const iNameNo);
	void SetInfoText(XUI::CXUI_Wnd* pItem, CONT_DEF_JOBSKILL_SAVEIDX::mapped_type const &rkSaveIndexInfo);
	void lwCloseJobSkillItemUI();
	void RecvContJobSkillHistoryItem(BM::Stream & rkPacket);
	void lwJS3_SelectMakeItem(int const iSaveIndex);
	void JS3_RecvCreateItem(BM::Stream & rkPacket);
	void JS3_AddResItem(XUI::CXUI_Wnd* pWnd, SItemPos const& kItemPos, bool const bDivideReserve=true);
	void JS3_AddResItem(SItemPos const& kItemPos);
	std::wstring GetJobSkill3_UpgradeNeedItemName(int const iGroupNo, int const iGrade);
	void lwLearnFilter_JobSkillItem(bool const bCheck);
	void FilterOnlyLearn(CONT_DEF_JOBSKILL_SAVEIDX& rkContSaveIndex);
	bool IsOnlyLearnView();
}

#endif // FREEDOM_DRAGONICA_CONTENTS_JOBSKILL_LWJOBSKILLITEM_H