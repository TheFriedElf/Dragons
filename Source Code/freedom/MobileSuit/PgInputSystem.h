#ifndef FREEDOM_DRAGONICA_INPUT_PGINPUTSYSTEM_H
#define FREEDOM_DRAGONICA_INPUT_PGINPUTSYSTEM_H

#include "NiDI8InputSystem.H"
#include "NiInputDI8Keyboard.H"
#include "NiInputDI8Mouse.H"
#include "NiInputDI8GamePad.h"

//	윈도우 메세지 기반 입력 시스템
//	게임브리오의 입력시스템에 비해 단점 : 윈도우에서만 된다. 조이스틱 지원 불가.

class	PgInputMouse;
class	PgInputKeyboard;
class	PgInputGamePad;

struct SPadButtonInformation
{
	//패드 버튼과 연결된 키(U키 기반)
	int m_iUkey;
	//패드 버튼과 연결된 UI버튼 인덱스(1~24)
	int m_iUIButtonIndex;

	SPadButtonInformation()
		: m_iUkey(0), m_iUIButtonIndex(0)
	{}
};

class	PgInputSystem	:	public	NiInputSystem
{
	NiDeclareRTTI;

private:
	//GamePad Stuff
	static NiInputGamePad::Button	eButtons[NiInputGamePad::NIGP_NUMBUTTONS];
	static NiInputGamePad::Button	eAxisToButtons[NiInputGamePad::NIGP_NUM_DEFAULT_AXIS*2]; //음수부터
	static bool bAxisToButtonsPressedBefore[NiInputGamePad::NIGP_NUMBUTTONS];
	static bool bAxisToButtonsPressed[NiInputGamePad::NIGP_NUMBUTTONS];
	//게임패드 개체 생성을 위해 NiInput을 생성하고 담아둘 개체
	NiInputSystemPtr m_spInputForGamePad;
	float m_fGamePadAxisSensitivityThreshold;
	//패드 버튼이 Down됐을 때 처리
	void HandleButtonDown(NiInputGamePad::Button eButton);
	//패드 버튼이 Release됐을 때 처리
	void HandleButtonRelease(NiInputGamePad::Button eButton);
	
	//패드 키 셋팅을 담아둘 배열(UI에서 설정 수정 후 확인을 누르면 밑의 임시 배열 내용이 이 배열로 복사)
	SPadButtonInformation m_kPadSetting[NiInputGamePad::NIGP_NUMBUTTONS];
	//패드 셋팅을 임시로 담아둘 배열(UI에서 설정 수정 후 확인을 누르기 전까진 이 배열에만 담김다)
	SPadButtonInformation m_kTempPadSetting[NiInputGamePad::NIGP_NUMBUTTONS];


public:

	PgInputSystem(CreateParams* pkParams);
	virtual ~PgInputSystem();

	virtual	bool	HandleWindowMessage(unsigned int uiMsg,WPARAM wParam,LPARAM lParam);

	virtual NiInputMouse* OpenMouse();
	virtual NiInputKeyboard* OpenKeyboard();
	virtual NiInputGamePad* OpenGamePad(unsigned int uiPort, unsigned int uiSlot);

	NiInputErr CheckEnumerationStatus(void);

	//패드 버튼 설정
	void BindButtonPairWithNiKeycode(NiInputGamePad::Button eButton, int iUkey, int iUIButtonIndex);
	//패드 버튼과 연결된 UI버튼의 인덱스를 가져옴
	int GetUIButtonIndex(NiInputGamePad::Button eButton);
	//이미 패드 버튼이 이미 바인드되어있는가 이미 바인드되어있으면 기존에 바인드되어있던 UI버튼의 인덱스를 리턴한다
	int IsAlreadyBind(NiInputGamePad::Button kPadButton);
	//인자로 받은 UI버튼 인덱스를 사용하는 버튼을 리턴한다.
	NiInputGamePad::Button GetPadButtonRefPressedIndex(int const iIndex);
	//패드 저장을 되돌리기 위해 백업해두는 함수
	void BackupPadSettingTable();
	//저장 내용을 되돌림
	void RestorePaddSettingTable();
	bool GetAxisToButtonPressed(NiInputGamePad::Button eButton);

	float GetGamePadAxisSensitivityThreshold() { return m_fGamePadAxisSensitivityThreshold; }
	void SetGamePadAxisSensitivityThreshold(float fValue);

	//여기부터 패드입력 처리 PgInput을 별도로 생성하지 않고
	//NiInput의 게임패드를 사용한다.
	//해당 패드의 입력을 처리한다.
	void HandleGamePad(NiInputGamePad* pkTargetPad);

	//설정 저장 테이블의 포인터를 얻어온다. 컨피그 저장용
	SPadButtonInformation* GetPadSettingTable() { return m_kPadSetting; }

	void	ReleaseAllKey();

public:

	static	void SetConversionStatus(HWND hWnd, bool const bIsOn, DWORD const dwNewConvMode, DWORD const dwNewSentMode);

};

class	PgInputMouse	:	public	NiInputMouse
{
	NiDeclareRTTI;

public:
	PgInputMouse();

	virtual	bool	HandleWindowMessage(NiInputSystem* pkInputSystem,unsigned int uiMsg,WPARAM wParam,LPARAM lParam);
	virtual NiInputErr UpdateActionMappedDevice(NiInputSystem* pkInputSystem);

	void	ReleaseAllKey(PgInputSystem *pkInputSystem);

private:

	NiPoint2	m_ptMousePos;
};

#define	MAX_WIN_KEY_NUM	256
class	PgInputKeyboard	:	public	NiInputKeyboard
{
	NiDeclareRTTI;

public:
	PgInputKeyboard();

	NiAction* GetMappedAction(unsigned long uiVirtualKey);

	virtual	bool	HandleWindowMessage(NiInputSystem* pkInputSystem,unsigned int uiMsg,WPARAM wParam,LPARAM lParam);
	virtual NiInputErr UpdateActionMappedDevice(NiInputSystem* pkInputSystem);

	void	ReleaseAllKey(PgInputSystem *pkInputSystem);

	DWORD	GetWinKeyFromNIKey(KeyCode kNiKeyCode)	const;

	KeyCode GetNiKey(int iVirtualKey) const;

private:

	bool	HandleKeyDown(NiInputSystem* pkInputSystem,unsigned int uiKeyID);
	bool	HandleKeyUp(NiInputSystem* pkInputSystem,unsigned int uiKeyID);

	KeyCode m_aeWinKeyToNIIMapping[MAX_WIN_KEY_NUM];
	
	unsigned	int	GetVirtualKeyByScanCode(BYTE byScanCode,unsigned	int iDefaultKeyID);
};

class PgInputGamePad : public NiInputGamePad
{
	NiDeclareRTTI;

public:
	explicit PgInputGamePad(unsigned int uiPort);

	NiAction* GetMappedAction(unsigned long uiKeyScanCode);

	bool HandleGamePad();

	void ConfigButtons();

	virtual unsigned int GetMotorCount() const;

    // The maximum allowable value for the given motor.
    virtual unsigned int GetRumbleRange(unsigned int uiMotor) const;
    
    // uiValue in [0..value returned by GetRumbleRange]
    // If bCommit is true, the value and all uncommited values for other 
    // motors will be set on the device.
    virtual void SetRumbleValue(unsigned int uiMotor, unsigned int uiValue,
        bool bCommit);

    // Access the current value for a motor, even if that value is uncommited.
    virtual void GetRumbleValue(unsigned int uiMotor, unsigned int& uiValue)
        const;

    // Starts rumble - commits current values
    virtual void StartRumble();
    
    // Halts rumble
    virtual void StopRumble(bool bQuickStop = false);

private:
	Button m_aePadButtonToNIIMapping[NIGP_NUMBUTTONS];
};

#endif // FREEDOM_DRAGONICA_INPUT_PGINPUTSYSTEM_H