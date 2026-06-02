#ifndef FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_EFFECT_LWLINKEDPLANE_H
#define FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_EFFECT_LWLINKEDPLANE_H

#include "PgScripting.h"
#include "PgLinkedPlane.h"

class	lwPoint3;

LW_CLASS(PgLinkedPlane, LinkedPlane)

	int		GetInstanceID();

	void	SetStartPos(lwPoint3 kPos);
	void	SetEndPos(lwPoint3 kPos);

	void	SetWidth(float fWidth);
	float	GetWidth();

	void	SetEnable(bool bEnable);
	bool	GetEnable();

LW_CLASS_END;

LW_CLASS(PgLinkedPlaneGroup, LinkedPlaneGroup)
	
	lwLinkedPlane	AddNewPlane(int const iAlphaTestRef);
	lwLinkedPlane	GetPlane(int iInstanceID);

	void	SetTexture(char const *strPath);

	void	ReleasePlane(int iInstanceID);
	void	ReleaseAllPlane();

	int		GetInstanceID();

	void	SetTextureCoordinateUpdateDelay(float fDelay);
	float	GetTextureCoordinateUpdateDelay();

LW_CLASS_END;

LW_CLASS(PgLinkedPlaneGroupMan, LinkedPlaneGroupMan)

	lwLinkedPlaneGroup	CreateNewLinkedPlaneGroup();
	lwLinkedPlaneGroup	GetLinkedPlaneGroup(int iInstanceID);

	void	ReleaseLinkedPlaneGroup(int iInstanceID);

LW_CLASS_END;
#endif // FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_EFFECT_LWLINKEDPLANE_H