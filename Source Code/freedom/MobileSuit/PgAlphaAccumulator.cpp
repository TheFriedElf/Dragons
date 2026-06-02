#include "stdafx.h"
#include "PgStat.h"
#include "PgMobileSuit.h"
#include "PgBatchRender.H"
#include "PgAlphaAccumulator.h"

bool PgAlphaAccumulator::m_bUseAlphaGroup = true;

NiImplementRTTI(PgAlphaAccumulator, NiAlphaAccumulator);

//---------------------------------------------------------------------------
PgAlphaAccumulator::PgAlphaAccumulator() :
	m_piGroups(NULL), m_bFrameDebug(false),m_pkBatchRender(NULL)
{
	NiAlphaAccumulator();

	m_iMaxItems = 2000;	// 미리 Alloc을 크게 잡는다.
	NiFree(m_ppkItems);

	m_iMaxItems = m_iNumItems;

	m_ppkItems = NiAlloc(NiGeometry*, m_iMaxItems);
	assert(m_ppkItems);

	NiFree(m_pfDepths);
	m_pfDepths =  NiAlloc(float, m_iMaxItems);
	assert(m_pfDepths);

	NiFree(m_piGroups);
	m_piGroups =  NiAlloc(int, m_iMaxItems);
	assert(m_piGroups);
}

//---------------------------------------------------------------------------
PgAlphaAccumulator::~PgAlphaAccumulator()
{
	NiFree(m_piGroups);
}
//---------------------------------------------------------------------------
void PgAlphaAccumulator::RegisterObjectArray(NiVisibleArray& kArray)
{
    PROFILE_FUNC();

	NiRenderer* pkRenderer = NiRenderer::GetRenderer();

	const unsigned int uiQuantity = kArray.GetCount();
	for (unsigned int i = 0; i < uiQuantity; i++)
	{
		NiGeometry& kObject = kArray.GetAt(i);

		const NiPropertyState* pkState = kObject.GetPropertyState();

		assert(pkState);

		const NiAlphaProperty *pkAlpha = pkState->GetAlpha();

		// Every property state should have a valid alpha property
		assert(pkAlpha);

		if (pkAlpha->GetAlphaBlending() && 
			!(m_bObserveNoSortHint && pkAlpha->GetNoSorter()) && 
			kObject.GetSortObject())
		{
			m_kItems.AddTail(&kObject);
		}
		else
		{
			if(m_pkBatchRender)
			{
				if(false == m_pkBatchRender->AddGeometry(&kObject))
				{
					kObject.RenderImmediate(pkRenderer);
				}
			}
			else
			{
				kObject.RenderImmediate(pkRenderer);
			}
		}
	}
}
void PgAlphaAccumulator::StartAccumulating(const NiCamera* pkCamera)
{
    PROFILE_FUNC();
    NiAlphaAccumulator::StartAccumulating( pkCamera );
}
void PgAlphaAccumulator::FinishAccumulating()
{
    PROFILE_FUNC();
    NiAlphaAccumulator::FinishAccumulating();
}
//---------------------------------------------------------------------------
void PgAlphaAccumulator::Sort()
{
	if (m_bUseAlphaGroup == false)
	{
		NiAlphaAccumulator::Sort();
		return;
	}

	m_iNumItems = m_kItems.GetSize();
	if (!m_iNumItems)
		return;

	if (m_iNumItems > m_iMaxItems || m_piGroups == NULL)
	{
		NiFree(m_ppkItems);

		m_iMaxItems = m_iNumItems;

		m_ppkItems = NiAlloc(NiGeometry*, m_iMaxItems);
		assert(m_ppkItems);

		NiFree(m_pfDepths);
		m_pfDepths =  NiAlloc(float, m_iMaxItems);
		assert(m_pfDepths);

		NiFree(m_piGroups);
		m_piGroups =  NiAlloc(int, m_iMaxItems);
		assert(m_piGroups);
	}

	NiTListIterator pkPos = m_kItems.GetHeadPos();

	NiPoint3 kViewDir = m_pkCamera->GetWorldDirection();
	NiAlphaProperty* pkProp = NULL;

	if (m_bSortByClosestPoint)
	{
		for (int i = 0; i < m_iNumItems; i++)
		{
			m_ppkItems[i] = m_kItems.GetNext(pkPos);
			m_pfDepths[i] = 
				m_ppkItems[i]->GetWorldBound().GetCenter() * kViewDir - 
				m_ppkItems[i]->GetWorldBound().GetRadius();
			pkProp = (NiAlphaProperty*)m_ppkItems[i]->GetProperty(NiProperty::ALPHA);
			assert(pkProp);
			if (pkProp)
				m_piGroups[i] = pkProp->GetAlphaGroup();
		}
	}
	else
	{
		for (int i = 0; i < m_iNumItems; i++)
		{
			m_ppkItems[i] = m_kItems.GetNext(pkPos);
			m_pfDepths[i] = 
				m_ppkItems[i]->GetWorldBound().GetCenter() * kViewDir;
			pkProp = (NiAlphaProperty*)m_ppkItems[i]->GetProperty(NiProperty::ALPHA);
			assert(pkProp);
			if (pkProp)
				m_piGroups[i] = pkProp->GetAlphaGroup();

			if (m_bFrameDebug)
				PrintItems();
		}
	}
	PgAlphaAccumulator::SortObjectsByDepth(0, m_iNumItems - 1);
	SortObjectsByAlphaGroup(m_iNumItems);

	if (m_bFrameDebug)
	{
		PrintItems();
		m_bFrameDebug = false;
	}
}
void PgAlphaAccumulator::SortObjectsByDepth(int l, int r)
{
	if (r > l)
	{
		int i, j;

		i = l - 1;
		j = r + 1;
		float fPivot = ChoosePivot(l, r);

		for (;;)
		{
			do 
			{
				j--;
			} while (fPivot < m_pfDepths[j]);

			do
			{
				i++;
			} while (m_pfDepths[i] < fPivot);

			if (i < j)
			{
				NiGeometry* pkObjTemp = m_ppkItems[i];
				m_ppkItems[i] = m_ppkItems[j];
				m_ppkItems[j] = pkObjTemp;
				float fTemp = m_pfDepths[i];
				m_pfDepths[i] = m_pfDepths[j];
				m_pfDepths[j] = fTemp;
				int iTemp = m_piGroups[i];
				m_piGroups[i] = m_piGroups[j];
				m_piGroups[j] = iTemp;
			}
			else
			{
				break;
			}
		}

		if (j == r)
		{
			PgAlphaAccumulator::SortObjectsByDepth(l, j - 1);
		}
		else
		{
			PgAlphaAccumulator::SortObjectsByDepth(l, j);
			PgAlphaAccumulator::SortObjectsByDepth(j + 1, r);
		}
	}
}
void PgAlphaAccumulator::SortObjectsByAlphaGroup(int iTotalNum)
{
	// sorting algorithm is gnome sort. if you want to know about gnome sort, search wikipedia.
	int i = 0;
	int j = 0;
	NiGeometry* pkObjTemp = NULL;
	float fTemp = 0.0f;
	int iTemp = 0;

	while (i < iTotalNum)
	{
		if (i == 0 || m_piGroups[i-1] <= m_piGroups[i])
			i++;
		else
		{
			j = i - 1;
			pkObjTemp = m_ppkItems[i];
			m_ppkItems[i] = m_ppkItems[j];
			m_ppkItems[j] = pkObjTemp;
			fTemp = m_pfDepths[i];
			m_pfDepths[i] = m_pfDepths[j];
			m_pfDepths[j] = fTemp;
			iTemp = m_piGroups[i];
			m_piGroups[i] = m_piGroups[j];
			m_piGroups[j] = iTemp;
			i = j;			
		}
	}
}

/*
//---------------------------------------------------------------------------
void PgAlphaAccumulator::SortObjectsByAlphaGroup(int l, int r)
{
	if (r > l)
	{
		int i, j;

		i = l - 1;
		j = r + 1;
		int iPivot = ChooseAlphaGroupPivot(l, r);
		NiGeometry* pkObjTemp = NULL;
		float fTemp = 0.0f;
		int iTemp = 0;

		for (;;)
		{
			do 
			{
				j--;
			} while (iPivot < m_piGroups[j]);

			do
			{
				i++;
			} while (m_piGroups[i] < iPivot);

			if (i < j)
			{
				pkObjTemp = m_ppkItems[i];
				m_ppkItems[i] = m_ppkItems[j];
				m_ppkItems[j] = pkObjTemp;
				fTemp = m_pfDepths[i];
				m_pfDepths[i] = m_pfDepths[j];
				m_pfDepths[j] = fTemp;
				iTemp = m_piGroups[i];
				m_piGroups[i] = m_piGroups[j];
				m_piGroups[j] = iTemp;
			}
			else
			{
				break;
			}
		}

		if (j == r)
		{
			SortObjectsByAlphaGroup(l, j - 1);
		}
		else
		{
			SortObjectsByAlphaGroup(l, j);
			SortObjectsByAlphaGroup(j + 1, r);
		}
	}
	//indent--;
}

int PgAlphaAccumulator::ChooseAlphaGroupPivot(int l, int r) const
{
	// Check the first, middle, and last element. Choose the one which falls
	// between the other two. This has a good chance of discouraging 
	// quadratic behavior from qsort.
	// In the case when all three are equal, this code chooses the middle
	// element, which will prevent quadratic behavior for a list with 
	// all elements equal.

	int m = (l + r) >> 1;

	if (m_piGroups[l] < m_piGroups[m])
	{
		if (m_piGroups[m] < m_piGroups[r])
		{
			return m_piGroups[m];
		}
		else
		{
			if (m_piGroups[l] < m_piGroups[r])
			{
				return m_piGroups[r];
			}
			else
			{
				return m_piGroups[l];
			}
		}
	}
	else
	{
		if (m_piGroups[l] < m_piGroups[r])
		{
			return m_piGroups[l];
		}
		else
		{
			if (m_piGroups[m] < m_piGroups[r])
			{
				return m_piGroups[r];
			}
			else
			{
				return m_piGroups[m];
			}
		}
	}
}
*/
//---------------------------------------------------------------------------
// cloning
//---------------------------------------------------------------------------
NiImplementCreateClone(PgAlphaAccumulator);
//---------------------------------------------------------------------------
void PgAlphaAccumulator::CopyMembers(PgAlphaAccumulator* pkDest,
									 NiCloningProcess& kCloning)
{
	NiAlphaAccumulator::CopyMembers(pkDest, kCloning);
}

void PgAlphaAccumulator::PrintItems()
{
	NiTListIterator pkPos = m_kItems.GetHeadPos();

	for (int i = 0; i < m_iNumItems; i++)
	{
		NiGeometry* geom = m_ppkItems[i];
		if (geom)
		{
			NILOG(PGLOG_CONSOLE, "[PgAlphaAccumulator] %s(%s)(parent %s)(%f, %d) %d node\n", geom->GetName(), geom->GetRTTI()->GetName(), geom->GetParent() ? geom->GetParent()->GetName() : "", m_pfDepths[i], m_piGroups ? m_piGroups[i] : 0, i);
		}		
	}
}