#include "stdafx.h"
#include "PgNifOptimizer.h"


NiAVObject*	PgNifOptimizer::CreateOptimizedObject(NiAVObject *pkOriginal)
{
	//	겜브리오의 Strify Geometry 기능으로 묶여져 있는 여러개의 오브젝트를
	//	TriShape 로 떼어 놓는 기능을 한다.

	if(NiIsKindOf(NiTriStrips,pkOriginal))
	{
		NiTriStrips		*pkTriStrips = NiDynamicCast(NiTriStrips,pkOriginal);

		if(pkTriStrips && pkTriStrips->GetAppCulled() == false)
		{
			//	먼저, 유효한 삼각형이 몇개인지 카운트한다.
			int	iVaildTriCount = AnalyseVaildTriCount(pkTriStrips);

			//	최적화된 새로운 TriShape 를 만들어 기존의 TriStrips 와 교체한다.
			NiTriShape	*pkNewShape = CreateOptimizedTriShape(pkTriStrips,iVaildTriCount);

			pkNewShape->DetachAllProperties();

			if(pkTriStrips->GetPropertyState())
			{
				pkNewShape->AttachProperty(pkTriStrips->GetPropertyState()->GetAlpha());
				pkNewShape->AttachProperty(pkTriStrips->GetPropertyState()->GetDither());
				pkNewShape->AttachProperty(pkTriStrips->GetPropertyState()->GetFog());
				pkNewShape->AttachProperty(pkTriStrips->GetPropertyState()->GetMaterial());
				pkNewShape->AttachProperty(pkTriStrips->GetPropertyState()->GetRendererSpecific());
				pkNewShape->AttachProperty(pkTriStrips->GetPropertyState()->GetShade());
				pkNewShape->AttachProperty(pkTriStrips->GetPropertyState()->GetSpecular());
				pkNewShape->AttachProperty(pkTriStrips->GetPropertyState()->GetStencil());
				pkNewShape->AttachProperty(pkTriStrips->GetPropertyState()->GetTexturing());
				pkNewShape->AttachProperty(pkTriStrips->GetPropertyState()->GetVertexColor());
				pkNewShape->AttachProperty(pkTriStrips->GetPropertyState()->GetWireframe());
				pkNewShape->AttachProperty(pkTriStrips->GetPropertyState()->GetZBuffer());
			}

			pkNewShape->SetLocalTransform(pkTriStrips->GetLocalTransform());
			pkNewShape->UpdateProperties();
			pkNewShape->UpdateEffects();
			pkNewShape->Update(0);
			
			if(pkTriStrips->GetParent())
			{
				pkTriStrips->GetParent()->AttachChild(pkNewShape, true);

				//	기존의 TriStrips 는 제거한다.
				pkTriStrips->GetParent()->DetachChild(pkTriStrips);
			}
		}
	}
	else if(NiIsKindOf(NiNode,pkOriginal))
	{
		NiNode	*pkNode = NiDynamicCast(NiNode,pkOriginal);
		if(pkNode)
		{
			for(int i=0;i<pkNode->GetArrayCount();i++)
			{
				NiAVObject	*pkChild = pkNode->GetAt(i);
				if(pkChild)
				{
					CreateOptimizedObject(pkChild);
				}
			}
		}
	}

	return	NULL;
}
NiTriShape*	PgNifOptimizer::CreateOptimizedTriShape(NiTriStrips *pkTriStrips,int iValidTriCount)
{

	NiTriShape	*pkTriShape = NULL;


	int	iVertCount = pkTriStrips->GetVertexCount();
	NiPoint3	*pkVerts = NiNew NiPoint3[iVertCount];
	memcpy(pkVerts,pkTriStrips->GetVertices(),sizeof(NiPoint3)*iVertCount);

	NiPoint3	*pkNormals = NULL;
	if(pkTriStrips->GetNormals())
	{
		pkNormals = NiNew NiPoint3[iVertCount];
		memcpy(pkNormals,pkTriStrips->GetNormals(),sizeof(NiPoint3)*iVertCount);
	}

	NiColorA	*pkColors = NULL;
	if(pkTriStrips->GetColors())
	{
		pkColors = NiNew NiColorA[iVertCount];
		memcpy(pkColors,pkTriStrips->GetColors(),sizeof(NiColorA)*iVertCount);
	}

	unsigned	short	*pkIndicies = NiAlloc(unsigned short,iValidTriCount*3);

	NiTriStripsData	*pkStripsData = (NiTriStripsData*)pkTriStrips->GetModelData();
	int	iOrigTriCount = pkStripsData->GetTriangleCount();

	unsigned short	usInd[3]={0,0,0};
	NiPoint3	*pkOrigVertArray = pkStripsData->GetVertices();
	unsigned short	*pkOrigIndices = pkStripsData->GetStripLists();

	int	iTriIndex = 0;
	for(int i=0;i<iOrigTriCount;i++)
	{
		usInd[0] = *(pkOrigIndices+i);
		usInd[1] = *(pkOrigIndices+i+1);
		usInd[2] = *(pkOrigIndices+i+2);

		if(usInd[0] == usInd[1] ||
			usInd[0] == usInd[2] ||
			usInd[1] == usInd[2])
			continue;

		if(i%2 == 0)
		{	
			*(pkIndicies+iTriIndex*3+0) = usInd[0];
			*(pkIndicies+iTriIndex*3+1) = usInd[1];
			*(pkIndicies+iTriIndex*3+2) = usInd[2];
		}
		else
		{
			*(pkIndicies+iTriIndex*3+0) = usInd[2];
			*(pkIndicies+iTriIndex*3+1) = usInd[1];
			*(pkIndicies+iTriIndex*3+2) = usInd[0];
		}

		iTriIndex++;
	}


	pkTriShape = NiNew NiTriShape(iVertCount,pkVerts,pkNormals,pkColors,NULL,0,
		pkTriStrips->GetNormalBinormalTangentMethod(),iValidTriCount,pkIndicies);

	for(int i=0;i<pkStripsData->GetTextureSets();i++)
	{
		const	NiPoint2	*pkOrigTextures = pkStripsData->GetTextureSet(i);
		NiPoint2	*pkTextures = NiNew NiPoint2[iVertCount];
		memcpy(pkTextures,pkOrigTextures,sizeof(NiPoint2)*iVertCount);
		pkTriShape->AppendTextureSet(pkTextures);
	}

	return	pkTriShape;
}

int	PgNifOptimizer::AnalyseVaildTriCount(NiTriStrips *pkTriStrips)
{

	NiTriStripsData	*pkStripsData = (NiTriStripsData*)pkTriStrips->GetModelData();
	if(!pkStripsData) return 0;

	//	연속한 3개의 점중 2개의 점이 같은 위치라면 이것은 삼각형이 아니다.
	int	iTotalTri = pkStripsData->GetTriangleCount();

	int	iValidTriCount = 0;

	unsigned short	usInd[3]={0,0,0};
	NiPoint3	*pkVertArray = pkStripsData->GetVertices();

	unsigned short	*pkIndices = pkStripsData->GetStripLists();
	
	for(int i=0;i<iTotalTri;i++)
	{
		usInd[0] = *(pkIndices+i);
		usInd[1] = *(pkIndices+i+1);
		usInd[2] = *(pkIndices+i+2);

		if(usInd[0] == usInd[1] ||
			usInd[0] == usInd[2] ||
			usInd[1] == usInd[2])
			continue;

		iValidTriCount++;
	}

	return	iValidTriCount;

}