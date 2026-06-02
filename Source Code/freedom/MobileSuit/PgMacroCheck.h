#include "lwUI.h"

class PgMacroCheck
{
public:
	PgMacroCheck();
	virtual ~PgMacroCheck();
	
	void RememberCheckInfo(wchar_t const wcKey, int const iIteration, short const sType, int const iLimitTime);
	void Clear();
	
	bool	IsComplete()	const;
	wchar_t GetKey()		const { return m_wcKey;		 }
	int		GetIteration()	const { return m_iIteration; }
	short	GetType()		const { return m_sType;		 }

	int		GetLimitTime() const { return m_iLimitTime; }
	
	void	SetCurLimitTime(int const iCurLimitTime) { m_iCurLimitTime = iCurLimitTime; }
	int		GetCurLimitTime() const { return m_iCurLimitTime; }
	
protected:
	wchar_t m_wcKey;
	int		m_iIteration;
	short	m_sType;
	int		m_iLimitTime;
	int		m_iCurLimitTime;
};

#define g_kMacroChk SINGLETON_STATIC(PgMacroCheck)