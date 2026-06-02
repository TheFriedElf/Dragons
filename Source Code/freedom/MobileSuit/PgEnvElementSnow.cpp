#include "stdafx.h"
#include "PgEnvElementSnow.H"
#include "PgParticleMan.H"
#include "PgParticle.H"
#include "PgWorld.H"
#include "PgRenderer.H"
#include "PgRendererUtil.H"
#include "PgEnvElementFactory.H"

PgEnvElementSnow::PgEnvElementSnow()
:PgEnvElementSimple( lua_tinker::call<char const*>("GetSnowParticleID"), 500.0f )
{
}

PgEnvElementSnow::~PgEnvElementSnow()
{
}