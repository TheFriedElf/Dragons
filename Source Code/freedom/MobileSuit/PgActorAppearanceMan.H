#ifndef FREEDOM_DRAGONICA_RENDER_WORDOBJECT_ACTOR_PGACTORAPPEARANCEMAN_H
#define FREEDOM_DRAGONICA_RENDER_WORDOBJECT_ACTOR_PGACTORAPPEARANCEMAN_H
#include <NiMain.H>
class	PgActorAppearanceMan
{
public:
	enum	CHANGE_TYPE
	{
		CT_NONE=0,
		CT_COLOR,
		CT_KFM
	};
	struct	stChangeInfo
	{
		enum EFFECT_ATTACH_TYPE
		{
			EAT_NONE=0,
			EAT_NODE,
			EAT_POS
		};
		CHANGE_TYPE	m_kChangeType;

		std::string	m_kKFMPath;
		NiColorA	m_kColor;

		std::string	m_kEffectID;
		std::string	m_kEffectAttachNodeName;
		NiPoint3	m_kEffectPos;
		EFFECT_ATTACH_TYPE m_eAttachType;
		stChangeInfo():m_kChangeType(CT_NONE)
					  ,m_kEffectPos(0.0f,0.0f,0.0f)
					  ,m_eAttachType(EAT_NONE)
		{
		};
	};
	typedef std::vector<stChangeInfo> VChangeInfo;

	struct	stConditionInfo
	{
		int	m_iHPMin,m_iHPMax;
		VChangeInfo	m_vChangeInfos;

		stConditionInfo():m_iHPMin(0),m_iHPMax(0)
		{
		};
	};

	typedef std::vector<stConditionInfo*> VConditionInfo;

private:

	VConditionInfo	m_vConditionInfos;
	PgActor	*m_pkActor;

	int	m_iLastUpdatedHP;
	int	m_iLastUpdatedCondition;

public:

	PgActorAppearanceMan(PgActor *pkActor);
	virtual	~PgActorAppearanceMan();

	bool ParseXml(const TiXmlNode *pkNode);
	PgActorAppearanceMan*	CreateCopy(PgActor *pkActor);

	void	UpdateAppearance(bool const bOnlyChangeApperance = false);

private:

	void	UpdateAppearanceTo(stConditionInfo *pkCondition,bool const bOnlyChangeApperance = false);

	bool ParseChangeInfo(const TiXmlNode *pkNode,stConditionInfo *pkConditionInfo);
};
#endif // FREEDOM_DRAGONICA_RENDER_WORDOBJECT_ACTOR_PGACTORAPPEARANCEMAN_H