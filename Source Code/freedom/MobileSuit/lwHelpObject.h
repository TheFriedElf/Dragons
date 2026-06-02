#ifndef FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_LWHELPOBJECT_H
#define FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_LWHELPOBJECT_H
#include "PgScripting.h"
#include "PgHelpSystem.H"

LW_CLASS(PgHelpObject, HelpObject)

	int GetUIType();
	int	GetID();
	int	GetTitleTTID();
	int	GetMessageTTID();
	int	GetHelperMsgTTID();
	char const*	GetRelatedUIName();
	int GetCallEventID();

	void	SetDoNotActivateAgain(bool bDoNotActivateAgain);
	bool	GetDoNotActivateAgain();

LW_CLASS_END;
#endif // FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_LWHELPOBJECT_H