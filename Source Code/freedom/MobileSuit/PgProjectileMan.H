#ifndef FREEDOM_DRAGONICA_RENDER_WORDOBJECT_PROJECTILE_PGPROJECTILEMAN_H
#define FREEDOM_DRAGONICA_RENDER_WORDOBJECT_PROJECTILE_PGPROJECTILEMAN_H
#include "NiMain.h"
#include <list>
#include "PgProjectile.H"

class	PgRenderer;
class	NiCamera;
class	PgAction;
class	PgActionTargetList;
class	PgMobileSuit;

typedef struct tagProjectileActionReserve : public SProjectileAction
{
	tagProjectileActionReserve()
	{
		Init();
	}
	void Init()
	{
		kCasterGuid.Clear();
		iActionID = 0;
		iActionInstanceID = 0;
		uProjectileUID = 0;
		kTargetList.clear();
		fLifeTime = 0.0f;
	}

	void clone(SProjectileAction const& rkAction)
	{
		kCasterGuid =rkAction.kCasterGuid;
		iActionID = rkAction.iActionID;
		iActionInstanceID = rkAction.iActionInstanceID;
		uProjectileUID = rkAction.uProjectileUID;
	}

	bool IsNull()
	{
		return kCasterGuid.IsNull() || !iActionID || !iActionInstanceID;
	}

	bool operator == (PgProjectile const* rhs)
	{
		if(!rhs) {return false;}
		return (iActionID==rhs->GetParentActionNo()) && (uProjectileUID==rhs->GetUID()) && (kCasterGuid==rhs->GetParentPilotGUID());
	}

	bool operator == (tagProjectileActionReserve const& rhs) const
	{
		return (iActionID==rhs.iActionID) && (uProjectileUID==rhs.uProjectileUID) && (kCasterGuid==rhs.kCasterGuid);
	}

	tagProjectileActionReserve const& operator = (tagProjectileActionReserve const& rhs)
	{
		iActionID = rhs.iActionID;
		iActionInstanceID = rhs.iActionInstanceID;
		uProjectileUID = rhs.uProjectileUID;
		kTargetList = rhs.kTargetList;
		fLifeTime = rhs.fLifeTime;

		return (*this);
	}

	float fLifeTime;
		
	PgActionTargetList kTargetList;
}SProjectileActionReserve;

typedef	std::list<PgProjectilePtr> ProjectileList;
typedef	std::map<std::string,PgProjectilePtr> ProjectileMap;
typedef std::list<SProjectileActionReserve> ReserveProjectileList;
class	PgProjectileMan	
{
	friend	class	PgMobileSuit;

	ProjectileMap	m_ProjectileMap;	//	원본 소스
	ProjectileList	m_ProjectileInstanceList;	//	복사본(실제로는 이놈이 사용된다)

public:

	PgProjectileMan()	{	Create();	}
	virtual	~PgProjectileMan()	{	Destroy();	}

	void	Init(); // 땜빵 코드
	PgProjectile*	CreateNewProjectile(std::string const& kProjectileID,PgAction *pkParentAction,BM::GUID const &kParentPilotGUID);
	void	Update(float fAccumeTime,float fFrameTime);
	void	Draw(PgRenderer *pkRenderer, NiCamera *pkCamera, float fFrameTime);
	void DrawImmediate(PgRenderer *pkRenderer, NiCamera *pkCamera, float fFrameTime);

	void	DeleteProjectile(PgProjectile *pkProjectile);
	void	DeleteProjectile(BM::GUID const &kPilotGUID,int iActionInstanceID,int iUID);

	PgProjectile*	GetProjectile(BM::GUID const &kPilotGUID,int iActionInstanceID,int iUID);

	void	RemoveAll();

protected:

	void	Create();
	void	Destroy();

public:

	void	BroadCast_PAction(PgProjectile *pkProjectile,PgActionTargetList *pkTargetList);
	void	Notify_PAction(BM::Stream *pkPacket);
	
private:
	void	ReserveProjectileInfo(SProjectileAction const& rkAction, PgActionTargetList const& rkTargetList, float const fLifeTime = 0.1f);
	bool	FineFromReserveProjectile(PgProjectile const* pkProj, SProjectileActionReserve& kReserv);
	ReserveProjectileList	m_kReserveList;
};

extern	PgProjectileMan	g_kProjectileMan;
#endif // FREEDOM_DRAGONICA_RENDER_WORDOBJECT_PROJECTILE_PGPROJECTILEMAN_H