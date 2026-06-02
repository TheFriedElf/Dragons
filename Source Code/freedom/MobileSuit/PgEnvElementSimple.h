#ifndef FREEDOM_DRAGONICA_RENDER_ENVIRONMENT_PGENVELEMENTSIMPLE_H
#define FREEDOM_DRAGONICA_RENDER_ENVIRONMENT_PGENVELEMENTSIMPLE_H
#include "PgEnvElement.H"
#include "PgPSysBoundedPositionModifier.H"
#include "PgPSysColorModifierDecorator.H"

class PgEnvElementSimple
	: public PgEnvElement
{
private:
	struct stModifierInfo
	{
		stModifierInfo()
		{}
		stModifierInfo(PgPSysBoundedPositionModifier *pkPositionModifier, PgPSysColorModifierDecorator *pkColorModifierDecorator)
			:m_spPositionModifier(pkPositionModifier)
			,m_spColorModifierDecorator(pkColorModifierDecorator)
		{}
		PgPSysBoundedPositionModifierPtr m_spPositionModifier;
		PgPSysColorModifierDecoratorPtr m_spColorModifierDecorator;
	};

	typedef std::vector<stModifierInfo> ModifierInfoVector;

public:
	PgEnvElementSimple(std::string const kParticleID, float const fRadius);
	virtual ~PgEnvElementSimple();
	
	virtual ENV_ELEMENT_TYPE GetType() const = 0;

	virtual void Update(NiCamera *pkCamera, float fAccumTime, float fFrameTime);
	virtual void SetValue(float fIntensity, float fTransitTime, float fAccumTime);

protected:
	virtual void SetIntensity(float fIntensity);

private:

	void CreateParticle();
	void ReleaseParticle();

	void UpdateParticlePosition(NiCamera *pkCamera);
	void UpdateModifier(NiCamera *pkCamera);
	void UpdateModifier(NiCamera *pkCamera, stModifierInfo &kModifierInfo);

	void ModifyParticleRecursive(NiAVObject *pkAVObject);
	void ModifyParticleSystem(NiParticleSystem *pkParticleSystem);

	void AddModifierInfo(PgPSysBoundedPositionModifier *pkPositionModifier, PgPSysColorModifierDecorator *pkColorModifierDecorator);

	void RemoveRedundantModifiers(NiParticleSystem *pkParticleSystem);
	PgPSysBoundedPositionModifier* AppendBoundedPositionModifier(NiParticleSystem *pkParticleSystem);
	PgPSysColorModifierDecorator* AppendColorModifierDecorator(NiParticleSystem *pkParticleSystem);

	void Activate();
	void Deactivate();

	void SetActivated(bool bActivate) { m_bActivated = bActivate; }
	bool GetActivated() const{ return m_bActivated; }

	void AttachParticleToWorld();
	void DetachParticleFromWorld();

	float GetPositionBoundRadius(NiParticleSystem *pkParticleSystem);

private:
	NiNodePtr m_spParticle;
	int m_iWorldParticleSlotID;

	float m_fBoundRadius;
	NiPoint3 m_kPreviousCameraPosition;

	ModifierInfoVector m_kModifierInfoCont;

	bool m_bActivated;

	std::string m_kParticleID;
};

#endif // FREEDOM_DRAGONICA_RENDER_ENVIRONMENT_PGENVELEMENTSIMPLE_H