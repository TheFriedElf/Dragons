#include "stdafx.h"
#include "PgGMCommand.h"
#include "PgChatMgrClient.h"

namespace GMCOMMAND
{

#ifdef USE_GM
#include "Lohengrin/GMCommand.h"

void ProcessPacket( BM::Stream &kPacket )
{
	EGMCmdType iCmdType = GMCMD_NONE;
	kPacket.Pop( iCmdType );

	switch ( iCmdType )
	{
	case GMCMD_KICKUSER:
	case GMCMD_MAPMOVE_RECALL:
	case GMCMD_MAPMOVE_TARGET:
	case GMCMD_MAPMOVE_TARGET_TOMAP:
		{
			std::wstring wstrCharName;
			short nChannelNo = 0;
			kPacket.Pop( wstrCharName );
			kPacket.Pop( nChannelNo );

			std::wstring wstMsg;
			if ( nChannelNo < 0 )
			{
				WstringFormat( wstMsg, MAX_PATH, TTW(99001).c_str(), wstrCharName.c_str() );
			}
			else if ( PUBLIC_CHANNEL_NUM == nChannelNo )
			{
				WstringFormat( wstMsg, MAX_PATH, TTW(99003).c_str(), wstrCharName.c_str() );
			}
			else
			{
				WstringFormat( wstMsg, MAX_PATH, TTW(99002).c_str(), wstrCharName.c_str(), nChannelNo );
			}
			
			Notice_Show( wstMsg, EL_GMMsg );
		}break;
	}
}

#else

void ProcessPacket( BM::Stream &kPacket ){}

#endif

};
