#ifndef FREEDOM_DRAGONICA_SCRIPTING_UI_LWELEMENT_DUNGEON_H
#define FREEDOM_DRAGONICA_SCRIPTING_UI_LWELEMENT_DUNGEON_H


#include "PgScripting.h"
#include "XUI/XUI_Manager.h"

namespace lwElementDungeonUI
{
	bool RegisterWrapper(lua_State *pkState);
	void lwCallElementDungeonUI();
	void lwEnterElementDungeon(int const iSuperDungeonNo);
	bool lwCheckElementDungeon();
	bool lwGetEnterLevel(int const iPlayerLevel);
	bool Find_ElementCousumItem(ContHaveItemNoCount & ContItemNoCount);
}

#endif