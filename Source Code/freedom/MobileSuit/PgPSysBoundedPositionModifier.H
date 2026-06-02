#ifndef FREEDOM_DRAGONICA_RENDER_PARTICLE_MODIFIER_PGPSYSBOUNDEDPOSITIONMODIFIER_H
#define FREEDOM_DRAGONICA_RENDER_PARTICLE_MODIFIER_PGPSYSBOUNDEDPOSITIONMODIFIER_H

#include <NiPSysPositionModifier.H>

class	PgPSysBoundedPositionModifier	:	public	NiPSysPositionModifier
{
    NiDeclareRTTI;

public:

	PgPSysBoundedPositionModifier(const char *pcName);

    virtual void Update(float fTime, NiPSysData* pkData);
	void	SetBoundCenter(NiPoint3	const	&kCenter)
	{
		m_kWorldBound.SetCenter(kCenter);
	}
	NiPoint3	const&	GetBoundCenter()	const
	{
		return	m_kWorldBound.GetCenter();
	}
	void	SetBoundRadius(float fRadius)
	{
		m_kWorldBound.SetRadius(fRadius);
	}
	float	GetBoundRadius()	const
	{
		return	m_kWorldBound.GetRadius();
	}
private:

	NiBound	const&	GetWorldBound()	const
	{
		return	m_kWorldBound;
	}

	void	AdjustParticlePositionToBound(NiPSysData* pkData);
	void	AdjustParticlePositionToBound(NiPSysData* pkData,unsigned int uiParticleIndex);

	NiPoint3	CropVertexPosition(NiPoint3 const &kCurrentPos)
	{
		NiBound	const	&kWorldBound = GetWorldBound();
		NiPoint3	vVertexToBoundCenter = kCurrentPos - kWorldBound.GetCenter();

		NiPoint3	kNewVertexPosition = kCurrentPos;

		kNewVertexPosition.x = kWorldBound.GetCenter().x + CropPosition(vVertexToBoundCenter.x,kWorldBound.GetRadius());
		kNewVertexPosition.y = kWorldBound.GetCenter().y + CropPosition(vVertexToBoundCenter.y,kWorldBound.GetRadius());
		kNewVertexPosition.z = kWorldBound.GetCenter().z + CropPosition(vVertexToBoundCenter.z,kWorldBound.GetRadius());

		return	kNewVertexPosition;
	}
	float	CropPosition(float fCurrentPos,float fCropRadius)
	{
		float	fNewPos = fCurrentPos;
		if(fCurrentPos>fCropRadius
			&& 0 < fCropRadius
			)
		{
			float	fDCrop = fCurrentPos - fCropRadius;
			while(fDCrop>fCropRadius*2)
			{
				fDCrop = fDCrop - fCropRadius*2;
			}

			fNewPos = -(fCropRadius-fDCrop);
		}

		if(fCurrentPos<-fCropRadius
			&& 0 < fCropRadius
			)
		{
			float	fDCrop = fCurrentPos + fCropRadius;
			while(fDCrop<(-fCropRadius*2))
			{
				fDCrop = fDCrop + fCropRadius*2;
			}

			fNewPos = (fCropRadius+fDCrop);
		}

		return	fNewPos;
	}

private:

	NiBound	m_kWorldBound;
};

NiSmartPointer(PgPSysBoundedPositionModifier);

#endif // FREEDOM_DRAGONICA_RENDER_PARTICLE_MODIFIER_PGPSYSBOUNDEDPOSITIONMODIFIER_H