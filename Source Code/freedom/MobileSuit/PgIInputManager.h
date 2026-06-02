#ifndef FREEDOM_DRAGONICA_INPUT_PGIINPUTMANAGER_H
#define FREEDOM_DRAGONICA_INPUT_PGIINPUTMANAGER_H

#include "PgInput.h"

class PgIInputObserver;

class PgIInputManager
{
	typedef std::vector<PgIInputObserver*> ObserverContainer;
public:

	//! ObserverContainer의 compare function
	static bool CompareObserver(PgIInputObserver *lhs, PgIInputObserver *rhs);

	//! InputSystem을 갱신한다.
	virtual bool UpdateInput() = 0;

	//! Input Observer를 추가
	static bool AddInputObserver(PgIInputObserver *pkIInputObserver);

	//! Input Observer를 삭제
	static bool RemoveInputObserver(PgIInputObserver *pkIInputObserver);

	static bool InvalidateInputObserverContainer();

	//! Input Observer를 Get!
	PgIInputObserver* GetInputObserver(unsigned int uiIndex);

	//! Input Observer 클리어
	void ClearInputObserver();

	//! 인풋을 Observer들이 처리할 수 있도록 우선순위에 따라 넘긴다.
	bool TransferToObserver(PgInput &rkInput);

protected:
	//! PgInputObserver의 Container이다.
	static ObserverContainer m_kObserverContainer;
	static ObserverContainer m_kWaitObserverContainer;
	static ObserverContainer m_kRemoveObserverContainer;
};

#endif // FREEDOM_DRAGONICA_INPUT_PGIINPUTMANAGER_H