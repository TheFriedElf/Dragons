#include "stdafx.h"
#include "PgEnvElementSakura.H"
#include "PgParticleMan.H"
#include "PgParticle.H"
#include "PgWorld.H"
#include "PgRenderer.H"
#include "PgRendererUtil.H"
#include "PgEnvElementFactory.H"

PgEnvElementSakura::PgEnvElementSakura()
:PgEnvElementSimple( lua_tinker::call<char const*>("GetSakuraParticleID"), 500.0f )
{
}

PgEnvElementSunflower::PgEnvElementSunflower()
:PgEnvElementSimple( "eff_sunflower_screen00", 500.0f )
{
}

PgEnvElementIceCream::PgEnvElementIceCream()
:PgEnvElementSimple( "efk_summer_buff", 500.0f )
{
}