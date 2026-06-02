#include "StdAfx.h"
#include "PgIInputObserver.h"

PgIInputObserver::PgIInputObserver(unsigned int uiInputPriority)
{
	m_uiInputPriority = uiInputPriority;
}

PgIInputObserver::~PgIInputObserver(void)
{
}

unsigned int PgIInputObserver::GetInputPriority()
{
	return m_uiInputPriority;
}