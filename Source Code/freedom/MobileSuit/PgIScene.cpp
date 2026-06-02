
#include "StdAfx.h"
#include "PgIScene.h"


PgIScene::PgIScene(unsigned int uiRenderPriority, unsigned int uiInputPriority)
	: PgIInputObserver(uiInputPriority)
{
	m_uiRenderPriority = uiRenderPriority;
	m_eSceneState = PG_SCENE_STATE_INSTANCED;
}

PgIScene::~PgIScene(void)
{
    m_spSceneRoot = 0;
}

unsigned int PgIScene::GetRenderPriority()
{
	return m_uiRenderPriority;
}

bool PgIScene::CompareRenderPriority(PgIScene *left, PgIScene *right)
{
	return left->GetRenderPriority() < right->GetRenderPriority();
}
