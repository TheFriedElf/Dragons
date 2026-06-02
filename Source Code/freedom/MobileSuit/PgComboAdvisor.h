#ifndef FREEDOM_DRAGONICA_UI_PGCOMBOADVISOR_H
#define FREEDOM_DRAGONICA_UI_PGCOMBOADVISOR_H
#include <nimain.h>
#include "PgRenderer.H"

const	int MAX_COMBO_NODE_CHILD =10;
const	int MAX_COMBO_NODE	=5;

class	PgMobileSuit;
class	PgComboAdvisor	
{
	friend	class	PgMobileSuit;

	enum	ComboTreeNodeState
	{
		CTNS_DISABLED	= 0,	//	화면에 나오지 않음
		CTNS_NEXT		= 1,	//	다음에 사용 가능
		CTNS_CURRENT	= 2,	//	현재 사용 중
		CTNS_PASSED		= 3,	//	사용해서 지나갔음
	};

	struct	stComboIconData
	{
		std::string	m_kActionID;
		std::string	m_kDir;
		RECT	m_kIconRect;
	};

	typedef std::vector<stComboIconData> ContIconData;


	struct	stComboTreeNode
	{
		std::string m_kActionID;

		int	m_iChildCount;
		stComboTreeNode	*m_pkChilds[MAX_COMBO_NODE_CHILD];

		int	m_iChildIndex;
		stComboTreeNode	*m_pkParent;

		ComboTreeNodeState	m_State;
		NiPoint3	m_ptPosition;

		int	m_iScreenElementIndex;
		NiScreenElementsPtr	m_spScreenElement[3];

		float	m_fTargetScale;
		float	m_fStartScale;
		float	m_fScaleStartTime;

		float	m_fCreationTime;

		bool m_bEnableInputKey;

		bool m_bCompleteCombo;

		stComboTreeNode()
		{
			m_State = CTNS_DISABLED;
			m_bEnableInputKey = true;
			m_bCompleteCombo = false;

			m_pkParent = NULL;

			m_iChildCount = 0;
			m_iChildIndex=0;

			m_iScreenElementIndex = 0;

			m_spScreenElement[0]=m_spScreenElement[1]= NULL;

			m_fTargetScale=1.0;
			m_fStartScale = 1.0;
			m_fScaleStartTime = 0.0;

			m_fCreationTime = 0;
			

			memset(m_pkChilds,0,sizeof(stComboTreeNode*)*MAX_COMBO_NODE_CHILD);
		}
		~stComboTreeNode()
		{
			ClearChilds();

			m_spScreenElement[0]=m_spScreenElement[1]= NULL;
		}

		void	ClearChilds()
		{
			for(int i=0;i<m_iChildCount;i++)
				SAFE_DELETE(m_pkChilds[i]);

			m_iChildCount = 0;
		}
	};

	typedef std::list<stComboTreeNode*> NodeList;

	ContIconData	m_vIconData;
	NodeList m_NodeList;

	NiSourceTexturePtr	m_spComboTexture;
	NiSourceTexturePtr	m_spComboEffectTexture;

	NiMaterialPropertyPtr	m_spMaterialProperty;
	NiAlphaPropertyPtr	m_spAlphaProperty;
	NiVertexColorPropertyPtr	m_spVertexColorProperty;
	NiTexturingPropertyPtr	m_spTexturingProperty;
	NiTexturingPropertyPtr	m_spTexturingProperty_Effect;

	int m_iHitEffectIndex;
	bool m_iAlwaysBlink;

public:

	void Update(float fAccumTime,float fFrameTime);
	void DrawImmediate(PgRenderer *pkRenderer);

	void	OnNewActionEnter(char const* strActionID, bool const bAlwaysBlink);
	void	AddNextAction(char const* strActionID);
	void	ClearNextAction();
	void	ResetComboAdvisor();
//넥스트 타입 강조/불가 셋팅 함수
	void ChangeChildNode(bool const bEnableInput);
	void CompleteNode(bool const bCompleteCombo);
	void DeleteChildNode();
protected:

	void	Init();
	void	Destroy();

private:

	void	UpdateComboNode(stComboTreeNode *pkNode,float fAccumTime,float fFrameTime);

	void	UpdateNodeIcon( stComboTreeNode *pkNode );

	void	UpdateNodeAction( stComboTreeNode *pkNode );

	void	ResetComboAdvisor(stComboTreeNode *pkNode);

	void	ParseXML();

	void	ParseComboTreeIcon(const TiXmlElement *pkElement);
	//void	ParseComboTreeData(const TiXmlElement *pkElement,stComboTreeNode *pkParentNode);

	int	CountChildNode(const TiXmlElement *pkElement);

	NiScreenElementsPtr	CreateScreenElement(std::string kActionID,std::string kDir);

	void	SetComboNodeState(stComboTreeNode *pkNode,ComboTreeNodeState kState,stComboTreeNode *pkPrevNode);
	void	GetComboNodeRect(stComboTreeNode *pkNode,float &fLeft,float &fTop,float &fWidth,float &fHeight);

	void	SetComboNodeScale(stComboTreeNode *pkNode,ComboTreeNodeState kState);
	void	SetComboNodePos(stComboTreeNode *pkNode,ComboTreeNodeState kState, stComboTreeNode *pkPrevNode);
};

extern	PgComboAdvisor	g_kComboAdvisor;
#endif // FREEDOM_DRAGONICA_UI_PGCOMBOADVISOR_H