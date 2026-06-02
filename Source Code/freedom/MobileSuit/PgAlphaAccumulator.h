#ifndef FREEDOM_DRAGONICA_RENDER_RENDERER_PGALPHAACCUMULATOR_H
#define FREEDOM_DRAGONICA_RENDER_RENDERER_PGALPHAACCUMULATOR_H

#include "NiAlphaAccumulator.h"
#include "NiCamera.h"
#include "NiGeometry.h"

class	PgBatchRender;
class PgAlphaAccumulator : public NiAlphaAccumulator
{
	NiDeclareRTTI;
	NiDeclareClone(PgAlphaAccumulator);

public:
	PgAlphaAccumulator();
	virtual ~PgAlphaAccumulator();

    virtual void StartAccumulating(const NiCamera* pkCamera);
	virtual void RegisterObjectArray(NiVisibleArray& kArray);
    virtual void FinishAccumulating();

	void SetFrameDebug(bool bFrameDebug) { m_bFrameDebug = bFrameDebug; }
	void PrintItems();

	static void SetUseAlphaGroup(bool bUse) { m_bUseAlphaGroup = bUse; }
	static bool GetUseAlphaGroup() { return m_bUseAlphaGroup; }

    void SetBatchRender( PgBatchRender* pkBatchRender ) { m_pkBatchRender = pkBatchRender; }
    PgBatchRender* GetBatchRender() { return m_pkBatchRender; }

	virtual void Sort();

protected:
	int* m_piGroups;
	void SortObjectsByAlphaGroup(int iTotalNum);
	void SortObjectsByDepth(int l, int r);
	//void SortObjectsByAlphaGroup(int l, int r);
	//int ChooseAlphaGroupPivot(int l, int r) const;

    PgBatchRender* m_pkBatchRender;

	static bool m_bUseAlphaGroup;
	bool m_bFrameDebug;
};
#endif // FREEDOM_DRAGONICA_RENDER_RENDERER_PGALPHAACCUMULATOR_H