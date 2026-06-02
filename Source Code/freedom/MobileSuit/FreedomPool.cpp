#include "stdafx.h"
#include "FreedomPool.h"

BM::TObjectPool< PgActionTargetList > g_kActionTargetListPool;
BM::TObjectPool< PgTrailNode::stTrailRibbonLine > g_kTrailRibbonLinePool;

void InitFreedomPool()
{
	g_kActionTargetListPool.Init(20,10);
	g_kTrailRibbonLinePool.Init(20,10);
}

void TerminateFreedomPool()
{
	//PG_ASSERT_LOG(g_kActionTargetListPool.GetUsedCount() == 0);
	//if (g_kActionTargetListPool.GetUsedCount() > 0)
	//{
	//	NILOG(PGLOG_LOG, "[TerminateFreedomPool] - not all g_kActionTargetListPool released!(%d)\n", g_kActionTargetListPool.GetUsedCount());
	//}
	g_kActionTargetListPool.Terminate();

	//PG_ASSERT_LOG(g_kTrailRibbonLinePool.GetUsedCount() == 0);
	//if (g_kTrailRibbonLinePool.GetUsedCount() > 0)
	//{
	//	NILOG(PGLOG_LOG, "[TerminateFreedomPool] - not all g_kTrailRibbonLinePool released!(%d)\n", g_kActionTargetListPool.GetUsedCount());
	//}
	g_kTrailRibbonLinePool.Terminate();
}