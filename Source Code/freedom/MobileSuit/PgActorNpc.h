#ifndef FREEDOM_DRAGONICA_RENDER_WORDOBJECT_ACTOR_PGACTORNPC_H
#define FREEDOM_DRAGONICA_RENDER_WORDOBJECT_ACTOR_PGACTORNPC_H

class PgQuestMarkHelp;

//
class PgActorNpc : public PgActor
{
	NiDeclareRTTI;
public:
	PgActorNpc();	

	virtual void Terminate();
	virtual bool BeforeCleanUp();
	virtual	void DoLoadingFinishWork();	//	로딩이 완료되었을때 처리
	virtual bool Update(float fAccumTime, float fFrameTime);
	virtual void DrawImmediate(PgRenderer *pkRenderer, NiCamera *pkCamera, float fFrameTime);
	virtual bool AddSayAction(const ContSayItem &rkVec);
	virtual void UpdatePhysX(float fAccumTime, float fFrameTime);

	virtual bool OnClickSay();//말풍선
	virtual bool OnClickTalk();//Full Screen
	virtual bool OnClickWarning();

	virtual bool IsActivate(void)const;
	virtual bool IsHaveTalk(void)const;
	virtual bool IsHaveWarning(void)const;

	//!	줄 수 있는 퀘스트 정보 업데이트
	virtual void	ClearQuestInfo();
	virtual void	PopSavedQuestSimpleInfo();	//	퀘스트 매니저가 저장하고 있는 퀘스트 정보를 가져온다.

	virtual	PgIWorldObject*	CreateCopy()	{	return	NULL;	}
protected:
	bool ShowChatBaloon_ByItem(const SSayItem& rkItem);

protected:
	virtual bool checkVisible();
	PgActionSay m_kActionSay;

	PgQuestMarkHelp* m_pkQuestMarkHelp;

private:
	NiAudioSourcePtr m_spNowSaySound;
};
#endif // FREEDOM_DRAGONICA_RENDER_WORDOBJECT_ACTOR_PGACTORNPC_H