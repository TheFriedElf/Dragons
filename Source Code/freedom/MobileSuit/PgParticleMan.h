#ifndef FREEDOM_DRAGONICA_RENDER_STREAM_PGPARTICLEMAN_H
#define FREEDOM_DRAGONICA_RENDER_STREAM_PGPARTICLEMAN_H

#include "PgWorkerThread.h"
#include "PgParticle.H"

NiSmartPointer(PgParticleCache);

class PgParticleMan : public PgIClientWorker
{
public:

	PgParticleMan();
	~PgParticleMan();

	//! 새 이펙트 인스턴스를 생성한다.
	PgParticle *GetParticle(char const *pcEffectID,PgParticle::OPTION kValidOptions = PgParticle::O_NONE,float fScale = 1.0f, bool bLoop = false, bool bZTest = true, float bSoundTime = 0.0f,bool bAutoGround = false,bool bUseAppAccumTime=false, bool bNoFollowParentRotation = false);

	bool ParseXml(char const *pcXmlPath, void *pArg = 0);

	void	Terminate();

	static	void GetAllParticleNode(const NiNode *pkRootNode, NiObjectList &rkNodeList);

	static void ChangeParticleGeneration(NiAVObject *pkObject, bool bGenerate);

	//! 두 벡터간의 쿼터니온을 구한다.  (단위벡터, 단위..)
	static NiQuaternion GetBetweenQuaternion(NiPoint3 const &rkSrc, NiPoint3 const &rkDest);

	virtual bool DoClientWork(WorkData& rkWorkData);

	void	ReleaseCacheBySourceType(std::string const &kSourceType);

private:

	PgParticleCachePtr	m_spParticleCache;
};

#define g_kParticleMan SINGLETON_STATIC(PgParticleMan)

#endif // FREEDOM_DRAGONICA_RENDER_STREAM_PGPARTICLEMAN_H