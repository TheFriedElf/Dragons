#ifndef FREEDOM_DRAGONICA_RENDER_ENVIRONMENT_PGENVSTATESET_H
#define FREEDOM_DRAGONICA_RENDER_ENVIRONMENT_PGENVSTATESET_H

#include "PgEnvElement.H"

class	PgRenderer;
class	NiCamera;

#define INVALID_ENVSTATESET_ID	(-1)


class	PgEnvStateSet
{
public:

	typedef	std::vector<PgEnvElementPtr> EnvElementCont;

public:

	PgEnvStateSet(int iEnvStateSetID):m_iStateSetID(iEnvStateSetID)
	{	
		Init();	
	}

	virtual	~PgEnvStateSet()	{	Terminate();	}

	void	SetEnvElementValue(PgEnvElement::ENV_ELEMENT_TYPE kEnvElementType,float fIntensity, float fTransitTime,float fAccumTime);
	bool	SetActive(PgEnvStateSet* pkPrevActiveSet, float fTransitTime,float fAccumTime);

	void	AddEnvElement(PgEnvElement *pkElement);
	PgEnvElement*	GetEnvElement(PgEnvElement::ENV_ELEMENT_TYPE kElementTypeID)	const;

	void	Update(NiCamera *pkCamera,float fAccumTime,float fFrameTime);
	void	DrawImmediate(PgRenderer *pkRenderer, NiCamera *pkCamera, float fFrameTime);
	void	Draw(PgRenderer *pkRenderer, NiCamera *pkCamera, float fFrameTime);

	int	GetStateSetID()	const	{	return	m_iStateSetID;	}

private:

	void	Init();
	void	Terminate();

private:

	int	m_iStateSetID;
	EnvElementCont	m_kEnvElementVec;
};

#endif // FREEDOM_DRAGONICA_RENDER_ENVIRONMENT_PGENVSTATESET_H