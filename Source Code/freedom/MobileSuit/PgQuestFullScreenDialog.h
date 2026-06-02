#ifndef FREEDOM_DRAGONICA_CONTENTS_QUEST_UI_DIALOG_PGQUESTFULLSCREENDIALOG_H
#define FREEDOM_DRAGONICA_CONTENTS_QUEST_UI_DIALOG_PGQUESTFULLSCREENDIALOG_H

typedef std::map< int, std::wstring > ContDialogStep;

//
typedef struct tagNpcFaceInfo
{
	tagNpcFaceInfo()
	{
		Clear();
	}

	tagNpcFaceInfo(std::wstring const ID, std::wstring const FacePath, int const LimitWidth)
	{
		Clear();
		kFaceID = ID;
		kFaceImgPath = FacePath;
		iLimitWidth = LimitWidth;
	}

	void Clear()
	{
		kFaceID.clear();
		kFaceImgPath.clear();
		iLimitWidth = 0;
	}

	std::wstring kFaceID;
	std::wstring kFaceImgPath;
	int iLimitWidth;
} SNpcFaceInfo;

//
typedef struct tagSayItem SSayItem;
typedef struct tagFullScreenDialog
{
	BM::GUID kGuid;
	EQuestDialogType eType;
	std::wstring kObjectName;
	std::wstring kTitleText;
	std::wstring kDialogText;
	std::wstring kFaceID;
	int iFaceAniID;	//Only 3d Model
	std::wstring kModelID;
	std::wstring kStitchImageID;
	POINT3 kCamPos;
	int iMovieNo;

	tagFullScreenDialog(SSayItem const &rkItem, std::wstring const &rkName);
	tagFullScreenDialog(EQuestDialogType const Type, SQuestDialogText const &rkDialogText);
	tagFullScreenDialog(EQuestDialogType const Type);
	explicit tagFullScreenDialog();
	void Clear();
	bool Empty();
} SFullScreenDialogInfo;




//
class PgQuestFullScreenDialog
{
	typedef std::map< std::wstring, SNpcFaceInfo > FacePathContainer;

public:
	PgQuestFullScreenDialog();
	~PgQuestFullScreenDialog();

	bool ParseXmlForNpcFace(char const* pcFileName);
	bool GetNpcFacePath(std::wstring const &kFaceID, SNpcFaceInfo& rkOut) const;
	
	bool CallFullScreenTalk(const SFullScreenDialogInfo& rkInfo, const PgQuestInfo* pkQuestInfo = NULL, const SQuestDialog* pkDialogInfo = NULL);
	bool NextFullScreenTalk();

	void UpdateWanted(const SFullScreenDialogInfo& rkInfo, const PgQuestInfo* pkQuestInfo, const SQuestDialog* pkQuestDialog);

	bool IsQuestDialog();
	bool IsLastStepDialog() const;
	bool CanNextSelect() const;

	CLASS_DECLARATION_S(int, SelectItem1);
	CLASS_DECLARATION_S(int, SelectItem2);
	CLASS_DECLARATION_NO_SET(size_t, m_iNowDialogStep, NowDialogStep);
	CLASS_DECLARATION_NO_SET(size_t, m_iNowDialogMaxStep, NowDialogMaxStep);
	CLASS_DECLARATION_S_NO_SET(SFullScreenDialogInfo, NowInfo);
	
	CLASS_DECLARATION_S_NO_SET(bool, NowSelectMenu);//

	int NowQuestID()		{ return (m_pkNowQuestInfo)? m_pkNowQuestInfo->ID(): 0; }
	int NowDialogID()		{ return (m_pkNowDialogInfo)? m_pkNowDialogInfo->iDialogID: 0; }
protected:
	void ClearDialogStep();
	void ClearSelectedItem();
private:
	bool UpdateFullScreenTalk();

private:
	PgQuestInfo const *m_pkNowQuestInfo;
	SQuestDialog const *m_pkNowDialogInfo;

	FacePathContainer m_ContNPCFacePath;
	ContDialogStep m_ContDialogStep;

	mutable Loki::Mutex m_kFullScreenMutex;
};

#endif // FREEDOM_DRAGONICA_CONTENTS_QUEST_UI_DIALOG_PGQUESTFULLSCREENDIALOG_H