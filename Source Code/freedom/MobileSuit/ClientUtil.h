#ifndef __CLIENTUTIL_H__48AA3A2E_92B7_4d7a_A8D4_B35CEE999A0D__
#define __CLIENTUTIL_H__48AA3A2E_92B7_4d7a_A8D4_B35CEE999A0D__

#include <list>
#include <map>

#include <atlstr.h>

namespace ClientUtil
{
	namespace Settings
	{
		BOOL			GetPatchOption();
		BOOL			GetUseFriendModuleOption();
		BOOL			GetUseGuildOnlineInfo();
		BOOL			GetLoginAuthOption();
		NMLOCALEID		GetLocaleId();
		NMREGIONCODE	GetRegionCode();
		NMGameCode		GetGameCode();
		CAtlString		GetAuthServerIp();
		UINT16			GetAuthServerPort();
		CAtlString		GetMessengerServerIp();
		UINT16			GetMessengerServerPort();
		CAtlString		GetSessionServerIp();
		UINT16			GetSessionServerPort();
		CAtlString		GetNexonId();
		CAtlString		GetPassword();
		CAtlString		GetSecondaryNexonId();
		CAtlString		GetSecondaryPassword();
		CAtlString		GetCharacterId();
		INT32			GetCharacterSn();
		BOOL			GetForceCharacterMatchOption();
		CAtlString		GetRequestNewFriendId();
		CAtlString		GetExternalPassport();
	};

	namespace Convert
	{
		CAtlString	BOOLToString( BOOL bValue );
		CAtlString	LocaleIdToString( NMLOCALEID uLocaleId );
		CAtlString	RegionCodeToString( NMREGIONCODE uRegionCode );
		CAtlString	GameCodeToString( NMGameCode uGameCode );
		CAtlString	AuthResultToString( NMLoginAuthReplyCode uAuthResult );
		CAtlString	AuthConnectionClosedEventToString( UINT32 uType );
		CAtlString	MessengerResultToString( NMMessengerReplyCode uMessengerResult );
		CAtlString	MessengerConnectionClosedEventToString( UINT32 uType );
		CAtlString	SexTypeToString( NMSEXTYPE uSexType );
		CAtlString	ResultToString( BOOL bResult );
		CAtlString	ConfirmCodeToString( NMCONFIRMCODE uConfirmCode );
		CAtlString	RefreshEventTypeToString( INT32 nRefreshEventType );
	};

	namespace Log
	{
		void		InitLog( HWND hWnd );
		void		LogTime( LPCTSTR pszFormat, ... );	//	Log and time stamp
		void		LogInfo( LPCTSTR pszFormat, ... );	//	Log only
		HWND		GetLogWindowHandle();
	};

	class MatrixCard
	{
	public:
		MatrixCard( LPCTSTR pszFileName );
		~MatrixCard();

		CAtlString GetMatrixData( LPCTSTR pszMatrixInfo ) const;
	private:
		typedef std::map< CAtlString, CAtlString > CMatrixCardMap;
		CMatrixCardMap m_mapMatrixCard;
	};

	void			MessageBox( LPCTSTR pszTitle, UINT uType, LPCTSTR pszFormat, ... );
}

#endif	//	#ifndef __CLIENTUTIL_H__48AA3A2E_92B7_4d7a_A8D4_B35CEE999A0D__
