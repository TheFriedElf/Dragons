#include "stdafx.h"
#include "PgBowString.h"
#include "PgNifMan.h"

void	PgBowString::UpdateDownwardPass(float fTime, bool bUpdateControllers)
{
	NiNode::UpdateDownwardPass(fTime,bUpdateControllers);

	if(!m_spStringPoint[0] || !m_spStringPoint[1] || !m_spStringPoint[2]) return;

	NiPoint3	vPoint[3],vPoint2[2];

	vPoint[0] = m_spStringPoint[0]->GetTranslate();
	vPoint[1] = m_spStringPoint[1]->GetTranslate();
	vPoint[2] = m_spStringPoint[2]->GetTranslate();

	if(vPoint[0] == m_kBeforePoint[0] &&
		vPoint[1] == m_kBeforePoint[1] &&
		vPoint[2] == m_kBeforePoint[2])
		return;

	m_kBeforePoint[0] = vPoint[0];
	m_kBeforePoint[1] = vPoint[1];
	m_kBeforePoint[2] = vPoint[2];

	NiNodePtr	spParent01 = m_spStringPoint[0]->GetParent();
	NiPoint3	kParentTranslate = spParent01->GetTranslate();
	NiMatrix3	kParentRotate = spParent01->GetRotate();
	float	fParentScale = spParent01->GetScale();

	NiMatrix3::TransformVertices(kParentRotate,kParentTranslate,2,vPoint,vPoint2);

	NiPoint3	*pkVert = m_spStringLine->GetVertices();
	*pkVert = vPoint2[0];	pkVert++;
	*pkVert = vPoint[2];	pkVert++;
	*pkVert = vPoint2[1];	

	m_spStringLine->GetModelData()->MarkAsChanged(NiGeometryData::VERTEX_MASK);
}
bool	PgBowString::HasBowStringHelper(NiNodePtr spNode)
{
	NiNodePtr	spHelperNode[3];
	spHelperNode[0]=(NiNode*)spNode->GetObjectByName("p_bow_string_01");
	spHelperNode[1]=(NiNode*)spNode->GetObjectByName("p_bow_string_02");
	spHelperNode[2]=(NiNode*)spNode->GetObjectByName("p_bow_string_03");

	if(!spHelperNode[0] || !spHelperNode[1] || !spHelperNode[2]) return false;
	return	true;
}

bool	PgBowString::AttachTo(NiNodePtr	m_spAttachTargetNode,const NiColorA& kBowStringColor)
{
	m_kBeforePoint[0] = NiPoint3(0,0,0);
	m_kBeforePoint[1] = NiPoint3(0,0,0);
	m_kBeforePoint[2] = NiPoint3(0,0,0);

	m_spStringPoint[0]=(NiNode*)m_spAttachTargetNode->GetObjectByName("p_bow_string_01");
	m_spStringPoint[1]=(NiNode*)m_spAttachTargetNode->GetObjectByName("p_bow_string_02");
	m_spStringPoint[2]=(NiNode*)m_spAttachTargetNode->GetObjectByName("p_bow_string_03");

	if(!m_spStringPoint[0] || !m_spStringPoint[1] || !m_spStringPoint[2]) return false;

	m_spAttachTargetNode = m_spStringPoint[2]->GetParent();

	m_spAttachTargetNode->AttachChild(this, true);

   NiPoint3* pkVertices = NiNew NiPoint3[3];
    NiColorA* pkColors = NiNew NiColorA[3];
    NiBool* pkConnections = NiAlloc(NiBool, 3);

    pkConnections[0] = true;
    pkConnections[1] = true;
    pkConnections[2] = false;

    pkColors[0] = kBowStringColor;
    pkColors[1] = kBowStringColor;
    pkColors[2] = kBowStringColor;

    pkVertices[0] = NiPoint3(0,0,0);
    pkVertices[1] = NiPoint3(1,0,0);
    pkVertices[2] = NiPoint3(0,0,0);

    m_spStringLine = NiNew NiLines(3, pkVertices, 
        pkColors, NULL, 0, NiGeometryData::NBT_METHOD_NONE, pkConnections);

    NiMaterialProperty* pkMatProp = NiNew NiMaterialProperty();
    pkMatProp->SetEmittance(NiColor(1.0f,1.0f,1.0f));

    NiVertexColorProperty* pkVertProp = NiNew NiVertexColorProperty();
    pkVertProp->SetSourceMode(NiVertexColorProperty::SOURCE_EMISSIVE);
    pkVertProp->SetLightingMode(NiVertexColorProperty::LIGHTING_E);
    m_spStringLine->AttachProperty(pkVertProp);

    m_spStringLine->AttachProperty(pkMatProp);

    NiZBufferProperty* pkZProp = NiNew NiZBufferProperty();
    pkZProp->SetZBufferTest(true);
    pkZProp->SetZBufferWrite(true);
    m_spStringLine->AttachProperty(pkZProp);

    NiWireframeProperty* pkWireProp = NiNew NiWireframeProperty();
    pkWireProp->SetWireframe(true);
    m_spStringLine->AttachProperty(pkWireProp);

    m_spStringLine->SetTranslate(NiPoint3(0, 0, 0.0f));
    NiMatrix3 kRot;
    kRot.MakeIdentity();
    m_spStringLine->SetRotate(kRot);
    m_spStringLine->SetScale(1);

	m_spStringLine->SetConsistency(NiGeometryData::VOLATILE);

    m_spStringLine->Update(0.0f);
    m_spStringLine->UpdateProperties();
    m_spStringLine->UpdateEffects();
    m_spStringLine->UpdateNodeBound();

	AttachChild(m_spStringLine, true);

	return	true;
}