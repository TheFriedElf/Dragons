#ifndef FREEDOM_DRAGONICA_RENDER_WORDOBJECT_ENTITY_PGENTITYZONE_H
#define FREEDOM_DRAGONICA_RENDER_WORDOBJECT_ENTITY_PGENTITYZONE_H

#include "PgIWorldObject.h"

class PgEntityZone : public PgIWorldObject
{
	NiDeclareRTTI;
public:
	PgEntityZone();
	virtual ~PgEntityZone();

	//! Overriding
	virtual bool Update(float fAccumTime, float fFrameTime);
    virtual void Draw(PgRenderer *pkRenderer, NiCamera *pkCamera, float fFrameTime) { /* hook-method */ };
	virtual	void DrawImmediate(PgRenderer *pkRenderer, NiCamera *pkCamera, float fFrameTime);

	virtual bool ProcessAction(PgAction *pkAction,bool bInvalidateDirection=false,bool bForceToTransit=false);
	virtual bool ParseXml(const TiXmlNode *pkNode, void *pArg = 0, bool bUTF8 = false);
	virtual void InitPhysX(NiPhysXScene *pkPhysXScene, int uiGroup);
	virtual void ReleasePhysX();

	//! Overriding
	virtual NiObject* CreateClone(NiCloningProcess& kCloning);
	virtual void ProcessClone(NiCloningProcess& kCloning);

	//! 윗 방향을 변경한다.
	void SetNormal(NiPoint3 const &rkNormal);
	void IncRotate(float fRadian);
protected:
	NiPoint3 m_kNormal;
	float m_fRotation;

	PgParticle* m_pkParticle;
};

#endif // FREEDOM_DRAGONICA_RENDER_WORDOBJECT_ENTITY_PGENTITYZONE_H