#ifndef FREEDOM_DRAGONICA_RENDER_WORDOBJECT_DIRECTIONARROW_PGDIRECTIONARROW_H
#define FREEDOM_DRAGONICA_RENDER_WORDOBJECT_DIRECTIONARROW_PGDIRECTIONARROW_H

#include "PgIWorldObject.h"

class PgDirectionArrow : public PgIWorldObject
{
public:
	PgDirectionArrow(void);
	~PgDirectionArrow(void);

	bool Initialize();
	void Terminate();

	//! PgIWorldObject 재정의
	virtual bool Update(float fAccumTime, float fFrameTime);

	//! PgIWorldObject 재정의
    virtual void Draw(PgRenderer *pkRenderer, NiCamera *pkCamera, float fFrameTime) { /* hook-method */ };
    virtual void DrawImmediate(PgRenderer *pkRenderer, NiCamera *pkCamera, float fFrameTime) { /* hook-method */ };
	virtual bool ProcessAction(PgAction *pkAction,bool bInvalidateDirection=false,bool bForceToTransit = false){	return true; }
	virtual bool ParseXml(const TiXmlNode *pkNode, void *pArg = 0, bool bUTF8 = false) { return true; } 

public:
	NiAVObject* GetDestObject() { return m_pkDestObject; }
	void SetDestObject(NiAVObject* pkDestObject);

protected:
	int m_iDirArrowSlot;
	PgParticle* m_pkDirArrow;
	NiAVObject* m_pkDestObject;
};
#endif // FREEDOM_DRAGONICA_RENDER_WORDOBJECT_DIRECTIONARROW_PGDIRECTIONARROW_H