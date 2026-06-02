#ifndef FREEDOM_DRAGONICA_XML_FREEDOMPOOL_H
#define FREEDOM_DRAGONICA_XML_FREEDOMPOOL_H
#include "BM/ObjectPool.h"
#include "CreateUsingNiNew.inl"

#include "PgActionTargetList.h"
#include "PgTrail.H"

extern BM::TObjectPool< PgActionTargetList > g_kActionTargetListPool;
extern BM::TObjectPool< PgTrailNode::stTrailRibbonLine > g_kTrailRibbonLinePool;

extern void InitFreedomPool();
extern void TerminateFreedomPool();
#endif // FREEDOM_DRAGONICA_XML_FREEDOMPOOL_H