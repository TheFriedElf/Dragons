#ifndef FREEDOM_DRAGONICA_RENDER_ENVIRONMENT_PGEVIRONMENTMAN_H
#define FREEDOM_DRAGONICA_RENDER_ENVIRONMENT_PGEVIRONMENTMAN_H

#include "PgEnvStateSet.H"
#include <stack>
#include "CreateUsingNiNew.inl"

class	PgRenderer;
class	NiCamera;


typedef	std::list<PgEnvStateSet*> EnvStateSetCont;

class	PgEnvironmentMan	
{

	friend	struct CreateUsingNiNew<PgEnvironmentMan>;

protected:

	PgEnvironmentMan(){};
	virtual	~PgEnvironmentMan()	{	Terminate();	}

public:

	void	SetEnvElementValue(int iEnvStateSetID,PgEnvElement::ENV_ELEMENT_TYPE kEnvElementType,float fIntensity, float fTransitTime,float fAccumTime);
	bool	SetActiveEnvStateSet(int iEnvStateSetID,float fTransitTime,float fAccumTime);
	int		AddEnvStateSet();
	bool	ReleaseEnvStateSet(int iEnvStateSetID);
	void	ReleaseAllEnvStateSet();

	void	Update(NiCamera *pkCamera,float fAccumTime,float fFrameTime);
	void	DrawImmediate(PgRenderer *pkRenderer, NiCamera *pkCamera, float fFrameTime);
	void	Draw(PgRenderer *pkRenderer, NiCamera *pkCamera, float fFrameTime);

	PgEnvStateSet*	GetEnvStateSet(int iEnvStateSetID);

public:

	void	Init();
	void	Terminate();

private:

	int	GenerateNextStateSetID();

private:

	int	m_iActivatedStateSetID;

	EnvStateSetCont	m_kEnvStateSetStack;
};

#define g_kEnvironmentMan SINGLETON_CUSTOM(PgEnvironmentMan, CreateUsingNiNew)

#endif // FREEDOM_DRAGONICA_RENDER_ENVIRONMENT_PGEVIRONMENTMAN_H