#include "stdafx.h"
#include "PgPhysXUtil.H"

namespace PgPhysXUtil
{
	void	MakeStaticMeshNameUnique(NiPhysXScene *pkPhysXScene,std::string const &kPath,std::string const &kEntityName,float fScale)
	{
		if(!pkPhysXScene)
		{
			return;
		}

		NiPhysXSceneDesc *pkDesc = pkPhysXScene->GetSnapshot();
		if(!pkDesc)
		{
			return;
		}

		unsigned	int uiActorTotal = pkDesc->GetActorCount();
		for (unsigned int uiActorCount=0 ; uiActorCount<uiActorTotal ; ++uiActorCount)
		{
			NiPhysXActorDesc *pkActorDesc = pkDesc->GetActorAt(uiActorCount);
			MakeStaticActorNameUnique(pkActorDesc,kPath,kEntityName,fScale,uiActorCount);
		}
	}
	void	MakeStaticActorNameUnique(NiPhysXActorDesc *pkActorDesc,std::string const &kPath,std::string const &kEntityName,float fScale,unsigned int uiActorCount)
	{
		if(!pkActorDesc)
		{
			return;
		}

		unsigned	int uiShapeTotal = pkActorDesc->GetActorShapes().GetSize();
		for (unsigned	int uiShapeCount=0 ; uiShapeCount<uiShapeTotal ; uiShapeCount++)
		{
			NiPhysXShapeDesc *pkShapeDesc =
				pkActorDesc->GetActorShapes().GetAt(uiShapeCount);

			MakeStaticShapeNameUnique(pkShapeDesc,kPath,kEntityName,fScale,uiActorCount,uiShapeCount);
		}
	}
	void	MakeStaticShapeNameUnique(NiPhysXShapeDesc *pkShapeDesc,std::string const &kPath,std::string const &kEntityName,float fScale,unsigned int uiActorCount,unsigned int uiShapeCount)
	{
		if(!pkShapeDesc || !pkShapeDesc->GetMeshDesc())
		{
			return;
		}

		NiString strDescName = kEntityName.c_str();
		strDescName += "_";
		char szCount[256];
		_itoa_s(uiActorCount, szCount, 10);
		strDescName += szCount;
		strDescName += "_";
		_itoa_s(uiShapeCount, szCount, 10);
		strDescName += szCount;
		strDescName += "_";
		_itoa_s((int)(fScale*10000), szCount, 10);
		strDescName += szCount;
		strDescName += "_";
		strDescName += kPath.c_str();
		NiFixedString strDescName_((char const*)strDescName);
		pkShapeDesc->GetMeshDesc()->SetName(strDescName_);
	}

	void	ChangeScale(NiPhysXScene *pkPhysXScene,float fScale)
	{
		if(fScale == 1)
		{
			return;
		}

		NiPhysXManager* pkManager = NiPhysXManager::GetPhysXManager();
		if (!pkManager)
		{
			return;
		}

		if(!pkPhysXScene)
		{
			return;
		}

		NiPhysXSceneDesc	*pkDesc = pkPhysXScene->GetSnapshot();
		if(!pkDesc)
		{
			return;
		}

		unsigned	int uiActorTotal = pkDesc->GetActorCount();
		for (unsigned int uiActorCount =0 ; uiActorCount <uiActorTotal ; uiActorCount++)
		{
			NiPhysXActorDesc *pkActorDesc = pkDesc->GetActorAt(uiActorCount);
			if (pkActorDesc == NULL)
			{
				continue;
			}

			// 정확한 위치 계산을 위한 Matrix.
			NxMat34 kMultMat, kMultMatInv;
			kMultMat = pkActorDesc->GetPose(0);
			kMultMat.getInverse(kMultMatInv);

			int iShapeDescTotal = pkActorDesc->GetActorShapes().GetSize();
			for (int iShapeDescCount = 0 ;
				iShapeDescCount < iShapeDescTotal ;
				iShapeDescCount++)
			{
				NiPhysXShapeDesc *pkShapeDesc =
					pkActorDesc->GetActorShapes().GetAt(iShapeDescCount);

				if (pkShapeDesc && pkShapeDesc->GetMeshDesc() && (pkShapeDesc->GetType() == NX_SHAPE_CONVEX || pkShapeDesc->GetType() == NX_SHAPE_MESH))
				{
					NxTriangleMeshShapeDesc kTriMeshShapeDesc;
					size_t kSize = 0;
					unsigned char *pucData = 0;
					pkShapeDesc->GetMeshDesc()->GetData(kSize, &pucData);
					if (!kSize)
					{
						continue;
					}
					pkShapeDesc->GetMeshDesc()->ToTriMeshDesc(kTriMeshShapeDesc, true);

					NxTriangleMesh *pkTriMesh = kTriMeshShapeDesc.meshData;
					if(!pkTriMesh)
					{
						continue;
					}

					NxTriangleMeshDesc kTriMeshDesc;
					pkTriMesh->saveToDesc(kTriMeshDesc);

					unsigned short usNumNxVerts = kTriMeshDesc.numVertices;
					NxVec3* pkNxVerts = (NxVec3*)kTriMeshDesc.points;
					NxVec3* pkNxCloneVerts = NiAlloc(NxVec3, usNumNxVerts);

					for (unsigned short us = 0; us < usNumNxVerts; us++)
					{
						NxVec3 kVec = kMultMat * pkNxVerts[us];
						pkNxCloneVerts[us] = kMultMatInv * (kVec*fScale);
					}
					kTriMeshDesc.points = pkNxCloneVerts;

					// ReCook Triangle
					NxInitCooking();
					NiPhysXMemStream kMemStream;
					NIVERIFY(NxCookTriangleMesh(kTriMeshDesc, kMemStream));
					NxCloseCooking();
					kMemStream.Reset();

					if(pucData)
					{
						NiFree(pucData);
					}

					std::string kClonedName((char const*)pkShapeDesc->GetMeshDesc()->GetName());
					kClonedName += "_Clonned";

					pkShapeDesc->GetMeshDesc()->SetData(kMemStream.GetSize(), (unsigned char *)kMemStream.GetBuffer());
					pkShapeDesc->GetMeshDesc()->SetName(kClonedName.c_str());

					NiFree(pkNxCloneVerts);
				}
				else if (pkShapeDesc && pkShapeDesc->GetMeshDesc())
				{
					NILOG(PGLOG_LOG, "%s isn't NX_SHAPE_MESH \n", pkShapeDesc->GetMeshDesc()->GetName());
				}
			}
		}

	}



}