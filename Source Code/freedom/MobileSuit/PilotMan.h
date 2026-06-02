#pragma once

class CPilot;
class CWorldMan;

class CPilotMan
{
	// typedef NiTPointerMap<BM::GUID, CPilot*> Container;
	typedef std::map<BM::GUID, CPilot*> Container;

public:
	CPilotMan(void);
	~CPilotMan(void);


	//// Update
	//
	void Update(float fTime);

	//// Render
	//
	void Draw(NiCamera *pkCamera);			// why do pilotman draw actos?

	//// Pilot
	//
	bool AddPilot(BM::GUID& rkGUID, char *pcKFMPath, NiPoint3 &rkLoc, CPilot **ppkPilot_out);
	CPilot *GetPilot(BM::GUID& rkGUID);

	CWorldMan *GetWorldMan();
	void SetWorldMan(CWorldMan *pkWorldMan);

protected:
	NiVisibleArray m_kVisibleActor;
	Container m_kContainer;

	CWorldMan *m_pkWorldMan;
};
