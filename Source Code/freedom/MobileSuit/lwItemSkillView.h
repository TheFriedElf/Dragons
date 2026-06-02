#ifndef FREEDOM_DRAGONICA_CONTENTS_SKILL_LWITEMSKILLVIEW_H
#define FREEDOM_DRAGONICA_CONTENTS_SKILL_LWITEMSKILLVIEW_H

namespace lwItemSkillUI
{
	void RegisterWrapper(lua_State *pkState);//루아 노출
	void lwCallItemSkillUI();//UI 열기
	void UpdateList();//목록 갱신
	void SetSkill( XUI::SListItem* pListItem, int const iSkillNo, int const iExplainNo );//스킬 정보 설정
	void SetIcon( XUI::CXUI_Wnd* pItem, int const iSkillNo );//스킬정보 - 아이콘 설정
	void SetInfo( XUI::CXUI_Wnd* pItem, int const iSkillNo, int const iExplainNo );//스킬정보 - 텍스트 설정
	void ClearList();//목록 초기화
	int GetExplainNo(int const iSkillNo);//아이템스킬의 설명 텍스트 설정
}

#endif // FREEDOM_DRAGONICA_CONTENTS_SKILL_LWITEMSKILLVIEW_H