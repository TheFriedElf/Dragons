#ifndef FREEDOM_DRAGONICA_RENDER_ENVIRONMENT_PGENVELEMENT_H
#define FREEDOM_DRAGONICA_RENDER_ENVIRONMENT_PGENVELEMENT_H

#include <NiMain.H>

class	PgRenderer;
class	NiCamera;

class	PgEnvElement
	: public NiObject
{
public:
	enum ENV_ELEMENT_TYPE
	{// EWorldEnvironmentStatusType와 값이 같아야함
		EET_NONE=0,
		EET_SNOW=1,
		EET_SAKURA=2,
		EET_SUNFLOWER=3,
		EET_ICECREAM=4,
		EET_ETC3=5,
		EET_ETC4=6,
	};

public:
	PgEnvElement();
	virtual ~PgEnvElement();
	
	virtual ENV_ELEMENT_TYPE GetType() const = 0;
	virtual void DrawImmediate(PgRenderer *pkRenderer, NiCamera *pkCamera, float fFrameTime) {};
	virtual void Draw(PgRenderer *pkRenderer, NiCamera *pkCamera, float fFrameTime) {};

	virtual void Update(NiCamera *pkCamera, float fAccumTime, float fFrameTime);
	virtual void SetValue(float fIntensity, float fTransitTime, float fAccumTime);

protected:
	float GetIntensity() const;
	virtual void SetIntensity(float fIntensity);
	virtual void UpdateIntensityTransition(float fAccumTime, float fFrameTime);

private:
	float	m_fIntensity;
	float	m_fTargetIntensity;
	float	m_fTransitStartIntensity;
	float	m_fTransitStartTime;
	float	m_fTransitTotalTime;
};

NiSmartPointer(PgEnvElement);

#endif // FREEDOM_DRAGONICA_RENDER_ENVIRONMENT_PGENVELEMENT_H