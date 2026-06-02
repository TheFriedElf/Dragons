#ifndef FREEDOM_DRAGONICA_INPUT_PGIINPUTOBSERVER_H
#define FREEDOM_DRAGONICA_INPUT_PGIINPUTOBSERVER_H


class PgInput;

class PgIInputObserver
{
protected:
	PgIInputObserver(unsigned int uiInputPriority);

public:
	virtual ~PgIInputObserver() = 0;

	//! 입력 처리 우선순위를 반환한다.
	unsigned int GetInputPriority();

	//! 입력을 처리한다.
	virtual bool ProcessInput(PgInput* pkInput) = 0;

protected:
	unsigned int m_uiInputPriority;
};

#endif // FREEDOM_DRAGONICA_INPUT_PGIINPUTOBSERVER_H