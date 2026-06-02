#ifndef FREEDOM_DRAGONICA_CONTENTS_HOMETOWN_LWHOMETOWN_H
#define FREEDOM_DRAGONICA_CONTENTS_HOMETOWN_LWHOMETOWN_H

class lwGUID;
class PgIWorldObject;
namespace lwHomeTown
{
	void RegisterWrapper(lua_State *pkState);
	void lwOnClickHomeTown(int const iIndex);
	int lwGetHomeTownCount();
	int lwGetHomeTownNoAt(int iAt);
	lwGUID lwOnClickNamePlate(PgIWorldObject *pkObject);
	void OnClickRemoveFurniture();
	void OnClickMoveFurniture();
}

#endif // FREEDOM_DRAGONICA_CONTENTS_HOMETOWN_LWHOMETOWN_H