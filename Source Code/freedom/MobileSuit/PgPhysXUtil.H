#ifndef FREEDOM_DRAGONICA_PHYSIX_PGPHYSXUTIL_H
#define FREEDOM_DRAGONICA_PHYSIX_PGPHYSXUTIL_H

#include <NiPhysX.H>

namespace PgPhysXUtil
{

	void	MakeStaticMeshNameUnique(NiPhysXScene *pkPhysXScene,std::string const &kPath,std::string const &kEntityName,float fScale);
	void	MakeStaticActorNameUnique(NiPhysXActorDesc *pkActorDesc,std::string const &kPath,std::string const &kEntityName,float fScale,unsigned int uiActorCount);
	void	MakeStaticShapeNameUnique(NiPhysXShapeDesc *pkShapeDesc,std::string const &kPath,std::string const &kEntityName,float fScale,unsigned int uiActorCount,unsigned int uiShapeCount);

	void	ChangeScale(NiPhysXScene *pkPhysXScene,float fScale);

};

#endif // FREEDOM_DRAGONICA_PHYSIX_PGPHYSXUTIL_H