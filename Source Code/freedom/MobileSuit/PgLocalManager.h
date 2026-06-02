#ifndef FREEDOM_DRAGONICA_INPUT_PGLOCALMANAGER_H
#define FREEDOM_DRAGONICA_INPUT_PGLOCALMANAGER_H

#include "PgIInputManager.h"
#include "PgIXmlObject.h"

class PgInput;


enum
{
    ACTION_M_AXIS_X = 0xFF+ 1,
    ACTION_M_AXIS_Y	,
    ACTION_M_AXIS_Z,
    ACTION_M_BUTTON_LEFT,
	ACTION_M_BUTTON_RIGHT,
    ACTION_M_BUTTON_MIDDLE,
};

#define	MAX_KEY_SLOT	10
class PgLocalManager : public PgIInputManager, public PgIXmlObject
{
public:

	class PgBaseInput
	{
	public:
		PgBaseInput();
		void Reset();
			
		unsigned int m_uiKey;
		DWORD m_dwTimeStamp;
		int m_iSequence;
		bool m_bPressing;
	};

	class PgInputSlotEx
	{
	public:
		typedef enum
		{
			UNKNOWN = 0,
			DOUBLE_STROKE,
			COMBINATION,
			SEQUENCE,
			SUSTAIN
		} ESlotType;

		PgInputSlotEx();
		~PgInputSlotEx();

		static PgInputSlotEx *CreateInputSlotEx(unsigned int uiUKey, char *pcFirstKey, DWORD iParam = 0, bool bKeepLastKey = false, bool bReset = false);
		unsigned int ProcessExInput(PgInput &pkInput);
		bool ProcessDoubleStrokeType();
		bool ProcessCombinationType();
		bool ProcessSequenceType();
		bool ProcessSustainType();
		void ShiftLeftKeySequence();
		bool IsEmpty(int iIndex);
		bool KeepLastKey();
		bool DoReset();
		void Reset();

		void	SetActivate(bool bActivate)	{	m_bActivated = bActivate;	}
		bool	GetActivate()	const	{	return	m_bActivated;	}

		int	GetNBKeys()	const	{	return	m_iNbKeys;	}
		unsigned	int	GetUKey()	const	{	return	m_uiUKey;	}

	private:
		static int ms_iMaxKey;
		static int ms_iInvalidSequence;
	
		unsigned int m_uiUKey;
		bool m_bKeepLastKey;
		bool m_bReset;
		int m_iKeyOption;
		int m_iNbKeys;

		bool	m_bActivated;
		
		DWORD m_dwParam;
		PgBaseInput m_kKeySequences[MAX_KEY_SLOT];
		ESlotType m_eSlotType;
	};

	typedef unsigned int SlotID;
	typedef std::vector<PgInputSlotEx *> InputSlotExContainer;
	typedef std::map<int, unsigned int> KeyCodeContainer;	// 모든 키가 어떤 순서로 눌러져 있는지, 떼어져 있는지 체크
public:

	//! 소멸자
	PgLocalManager();
	virtual ~PgLocalManager();

	//! leesg213 2006-11-22 추가
	static	void	DestroySingleton();

	//! Accessor for Singleton
	static PgLocalManager *Get();

	//! PgLocalManager를 생성
	static bool Create(char const *pcID, void *pArg);

	//! PgLocalManager를 업데이트 한다.
	bool UpdateInput();

	//! Xml파일을 파싱한다.
	virtual bool ParseXml(const TiXmlNode *pkNode, void *pArg = 0, bool bUTF8 = false);

	//! Create PgInput
	bool CreateInput(NiActionData *pkActionData, PgInput &rkInput);

	//! 확장 슬롯에 대한 처리
	bool ProcessExtendedInput(PgInput &rkInput);
	void	DeactivateAllExtenedSlot();
	void	ActivateExtendedSlot(int iSlotUKey,bool bActivate= true);

	NiInputSystemPtr GetInputSystem(){return m_spInputSystem;}

	void	ProcessDelayedActivation();

	//새로 설정
	bool ActionKeyboard_Set(char const *szName, const unsigned int iUKey);
	bool ActionKeyboard_Swap(char const *szFromName, char const *szToName);

	bool	HandleWindowMessage(unsigned int uiMsg,WPARAM wParam,LPARAM lParam);

	void SetInputDirReverse()		{ m_bInputDirReverse = true; }
	void ClearInputDirReverse()		{ m_bInputDirReverse = false; }
	
private:
	//! PgLocalManager 초기화
	bool Initialize(void *pArg);

	//! Action Map에 Action을 추가
	void AddActionKeyboard(unsigned int uiUKey, NiInputKeyboard::KeyCode eKeyCode);
	void AddActionMouseAxes(std::string const &kSlotName, int const KeyCode, BM::GUID const &kGuid);
	void AddActionMouseButton(std::string const &kSlotName, int const KeyCode, BM::GUID const &kGuid);

	void	SortExContainer();

	//! Self Ptr
	static PgLocalManager *m_pkSelf;

	//! 키를 누른 순서
	static unsigned int ms_uiLastSequence;

	//! 게임브리오의 InputSystem, Action Mapping Mode를 사용
	NiInputSystemPtr m_spInputSystem;

	//! 게임브리오의 ActionMap
	NiActionMapPtr m_spActionMap;

	//! 게임브리오의 Keyboard
	NiInputKeyboardPtr m_spKeyboard;

	//! 게임브리오의 Mouse
	NiInputMousePtr m_spMouse;

	//! 게임브리오의 GamePad
	NiInputGamePadPtr m_spGamePad;

	//! 특수 키 목록
	InputSlotExContainer m_kInputSlotExContainer;

	//! 입력 방향을 뒤집으라고 명령
	int m_bInputDirReverse;
	bool GetInputDirReverse() const { return m_bInputDirReverse; }
};

#define g_pkLocalManager PgLocalManager::Get()

#endif // FREEDOM_DRAGONICA_INPUT_PGLOCALMANAGER_H