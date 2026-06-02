#include "StdAfx.h"
#include "PgDamper.h"

NiPoint3 PgDamper::SpringDamp(
	NiPoint3 const &rkFrom, 
	NiPoint3 const &rkTo, 
	NiPoint3 const &rkPrev, 
	float fDeltaTime, 
	float fSpringConst, 
	float fDampConst, 
	float fSpringLen,
	float fMaxLength,
	bool bCheckHeight)
{
	NiPoint3 kDisp = NiPoint3::ZERO;
	NiPoint3 kVel = NiPoint3::ZERO;
	NiPoint3 kResult = NiPoint3::ZERO;
	
	float fForceMag = 0.0f;
	float fDispWeight = 0.0f;
	float fDispLength = 0.0f;

	kDisp = rkFrom - rkTo;	

	if(kDisp.SqrLength() < 0.1f)
	{
		return rkFrom;
	}

	fDispLength = kDisp.Length();
	kVel = (rkPrev - rkTo) * fDeltaTime;
	fForceMag = fSpringConst * (fSpringLen - fDispLength) + fDampConst * kDisp.Dot(kVel) / fDispLength;
	fDispWeight = fForceMag * fDeltaTime / 2;
	if (fDispWeight > 0.0f)
		fDispWeight = 0.0f;
	else if (fDispWeight < -fDispLength)
	{
		fDispWeight = 0.0f; // To 보다 더 나가는 경우.
	}
	
	kDisp.Unitize();
	kResult = rkFrom + kDisp * fDispWeight;
	if (fMaxLength > 0.0f)
	{
		NiPoint3 kDiff = kResult - rkTo;
		kDiff.z = 0.0f;

		if (kDiff.Length() > fMaxLength)
		{
			//TODO : 100이 점프 높이인데.. define을 해야 할듯.
			if (NiAbs(rkFrom.z - rkTo.z) >= fMaxLength * 2.1f && bCheckHeight) // 높이 차에 대해서는 좀 관대해지자;; 
				return kResult;
			
			float ratio = 1 - fMaxLength / NiAbs(fDispLength);
			kResult = rkFrom + kDisp * ratio * -1.0f * fDispLength;
		}
	}
	
	return kResult;
}