

//PG_CLIENT_TYPE 설정
//externalR 이면	
//{E
//	USE_INB 이면
//	{I
//	}
//	USE_INB 아니면
//	{I
//	}
//}
//externalR 아니면
//{N
//}
#if defined(USE_INB)
#define PG_CLIENT_TYPE "I"		//External_Release일때
#elif defined(EXTERNAL_RELEASE)
#define PG_CLIENT_TYPE "E"		//INB일때
#endif

#ifndef EXTERNAL_RELEASE
#ifndef USE_INB
#define PG_CLIENT_TYPE "N"	
#endif
#endif

#ifdef DragonSaga
#define PG_CLIENT_NAME "DragonSaga"
#else
#define PG_CLIENT_NAME "Dragonica"
#endif

//PG_ANTIHACK_NAME 설정
//GameGuard면
//{G
//}
//핵실드면
//{H
//}
//게임가드아니면
//{
//	핵실드아니면
//	{N
//	}
//}

#if defined(USE_GAMEGUARD)
#define PG_ANTIHACK_NAME "G"	//게임가드 사용시
#endif

#if defined(USE_HACKSHIELD)		//핵실드 사용시
	#if defined(HACKSHIELD_UNAUTO)
		#define PG_ANTIHACK_NAME "H_UNAUTO"	//오토툴 미허용 버전(기본)
	#elif defined(HACKSHIELD_AUTO)
		#define PG_ANTIHACK_NAME "H_AUTO"	//오토툴 허용 버전(중국/일본)
	#endif
#endif

#ifndef USE_GAMEGUARD
#ifndef USE_HACKSHIELD
#define PG_ANTIHACK_NAME "N"	//아무것도 안할시
#endif
#endif


