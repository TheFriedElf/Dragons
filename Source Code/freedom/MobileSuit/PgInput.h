#ifndef FREEDOM_DRAGONICA_INPUT_PGINPUT_H
#define FREEDOM_DRAGONICA_INPUT_PGINPUT_H

class PgInput
{

	// TODO : Input Type을 정의해야 한다.
public:
	typedef enum
	{
		UR_LOCAL_BEGIN = 1000,	//앞으로는 키 이벤트 번호에 -1000 하면 해당되는 키가 나오지 않을 수도 있습니다.
								//PgOption::GetUKeyToKey(int iUKey)를 통해서 UKey의 근원 Key번호를 얻을 수 있습니다.
		UR_LOCAL_MOUSE_BEGIN = 2000,//마우스 시작 값.
		UR_EXTENDED_BEGIN = 3000,

		UR_REMOTE_BEGIN = 100000,
		UR_TEMP_BEGIN = 1000000,
	}EUKeyRange;

public:
	//! Constructor
	PgInput();

	bool IsLocal() const;
	bool IsRemote() const;
	bool IsExtendedSlot() const;

	//! Set Pilot Guid
	void SetPilotGuid(BM::GUID const &kGuid);
	
	//! Get Pilot Guid
	const	BM::GUID& GetPilotGuid();

	//! Set Pressed
	void SetPressed(bool bPressed);

	//! Get Pressed
	bool GetPressed();

	//! Get Unique Key NO.
	unsigned int GetUKey() const;
	
	//! Set Unique Key No.
	void SetUKey(unsigned int uiUKey);

	void Set(NiActionData *pData)
	{
		m_Data = *pData;
	}

	NiActionData* Data()
	{
		return &m_Data;
	}

	BM::Stream *GetPacket();
	void SetPacket(BM::Stream *pkPacket);

	//NiPoint3 &GetActionLoc();
	//void SetActionLoc(NiPoint3 &kLoc);

private:
	BM::GUID m_kPilotGuid;

	//! 키를 눌렀는지, 뗐는지 알 수 있다.
	bool m_bPressed;

	// TODO UI 에서는 UI 단축키를 위해서도 기타 마우스 이동을 위해서도 Input의 모든 정보가 필요하다.
	NiActionData m_Data;

	unsigned int m_uiUKey;

	BM::Stream *m_pkPacket;

	//NiPoint3 m_kActionLoc;
};

#endif // FREEDOM_DRAGONICA_INPUT_PGINPUT_H