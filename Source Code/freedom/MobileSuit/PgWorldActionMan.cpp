#include "stdafx.h"
#include "PgWorldActionMan.h"
#include "PgWorldActionObject.h"

bool	PgWorldActionMan::ParseXml(const TiXmlNode *pkNode, void *pArg, bool bUTF8)
{
	int const iType = pkNode->Type();
	
	switch(iType)
	{
	case TiXmlNode::ELEMENT:
		{
			TiXmlElement *pkElement = (TiXmlElement *)pkNode;
			assert(pkElement);
			
			char const *pcTagName = pkElement->Value();

			if(strcmp(pcTagName, "WORLD_ACTION_SETTING") == 0)
			{
				const TiXmlNode * pkChildNode = pkNode->FirstChild();
				int iTotalChild=0;
				while(pkChildNode)
				{
					if(pkChildNode->Type() == TiXmlNode::ELEMENT && strcmp(((TiXmlElement*)pkChildNode)->Value(),"WORLD_ACTION")==0)
						iTotalChild++;
					pkChildNode = pkChildNode->NextSibling();
				}
				m_paWorldActionXML = new stWorldActionXML[iTotalChild];
				m_iTotalWorldActionXML = iTotalChild;

				m_iCounter = 0;
				pkChildNode = pkNode->FirstChild();
				if(pkChildNode != 0)
				{
					if(!ParseXml(pkChildNode))
					{
						return false;
					}
				}
			}
			else if(strcmp(pcTagName, "WORLD_ACTION") == 0)
			{
				TiXmlAttribute *pkAttr = ((TiXmlElement*)pkNode)->FirstAttribute();
				unsigned int uiKey = 0;
				char const *pcScript = 0;

				stWorldActionXML	*pNewXML = (m_paWorldActionXML+m_iCounter++);

				while(pkAttr)
				{
					if(strcmp(pkAttr->Name(), "ID") == 0)
					{
						pNewXML->m_iWorldActionID = atoi(pkAttr->Value());
					}
					else if(strcmp(pkAttr->Name(), "SCRIPT") == 0)
					{
						pNewXML->m_ScriptName = pkAttr->Value();
					}
					else if(strcmp(pkAttr->Name(), "DURATION") == 0)
					{
						pNewXML->m_dwDuration = atoi(pkAttr->Value());
					}					
					pkAttr = pkAttr->Next();
				}
			}
		}

	default:
		break;
	}

	// 같은 층의 다음 노드를 재귀적으로 파싱한다.
	const TiXmlNode* pkNextNode = pkNode->NextSibling();
	if(pkNextNode)
	{
		if(!ParseXml(pkNextNode))
		{
			return false;
		}
	}
	return	true;
}
PgWorldActionObject*	PgWorldActionMan::CreateNewWorldActionObject(int iWorldActionID)
{
	PgWorldActionObject	*pObj = NiNew PgWorldActionObject(this,iWorldActionID);

	m_ObjList.push_back(pObj);

	return	pObj;
}

void	PgWorldActionMan::Update(float fFrameTime,float fAccumTime)
{
	PgWorldActionObject	*pObj;
	for(WorldActionObjectList::iterator itor = m_ObjList.begin(); itor != m_ObjList.end(); )
	{
		pObj = *itor;
		if ( pObj->OnUpdate(fAccumTime,fFrameTime) == false)
		{
			pObj->OnLeave();
			SAFE_DELETE_NI(pObj);
			itor = m_ObjList.erase(itor);
			continue;
		}
		itor++;
	}
}
std::string	PgWorldActionMan::GetWorldActionScript(int iWorldActionID)
{
	for(int i=0;i<m_iTotalWorldActionXML;i++)
	{
		if((m_paWorldActionXML+i)->m_iWorldActionID == iWorldActionID) return (m_paWorldActionXML+i)->m_ScriptName;
	}

	//::MessageBox(0,_T("Not Exist WorldActionID"),_T("GetWorldActionScript() Error"),MB_OK);
	_PgMessageBox("GetWorldActionScript() Error", "Not Exist WorldActionID");
	return std::string("");
}

DWORD PgWorldActionMan::GetWorldActionDuration(int iWorldActionID)
{
	for(int i=0;i<m_iTotalWorldActionXML;i++)
	{
		if((m_paWorldActionXML+i)->m_iWorldActionID == iWorldActionID) return (m_paWorldActionXML+i)->m_dwDuration;
	}

	//::MessageBox(0,_T("Not Exist WorldActionID"),_T("GetWorldActionScript() Error"),MB_OK);
	_PgMessageBox("GetWorldActionScript() Error", "Not Exist WorldActionID");
	return 0;	
}

void	PgWorldActionMan::Init()
{
	m_iTotalWorldActionXML = 0;
	m_paWorldActionXML = 0;
}
void	PgWorldActionMan::Destroy()
{
	SAFE_DELETE_ARRAY(m_paWorldActionXML);
	m_iTotalWorldActionXML = 0;
}
