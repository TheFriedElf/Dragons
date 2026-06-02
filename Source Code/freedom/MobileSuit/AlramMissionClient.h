#ifndef FREEDOM_DRAGONICA_CONTENTS_ALRAMMISSIONCLIENT_H
#define FREEDOM_DRAGONICA_CONTENTS_ALRAMMISSIONCLIENT_H

template< BM::Stream::DEF_STREAM_TYPE kType > class PgAlramMissionClient;

template< >
class PgAlramMissionClient< PT_M_C_NFY_ALRAMMISSION_BEGIN >
{
public:
	bool operator()( BM::Stream &kPacket );
};

template< >
class PgAlramMissionClient< PT_M_C_NFY_ALRAMMISSION_END >
{
public:
	bool operator()( BM::Stream &rkPacket );
	static bool Call( PgAlramMission &rkAlramMission );
};


template< >
class PgAlramMissionClient< PT_M_C_NFY_ALRAMMISSION_SUCCESS >
{
public:
	bool operator()( BM::Stream &rkPacket );
	static bool Call( PgAlramMission &rkAlramMission );
};

template< >
class PgAlramMissionClient< PT_M_C_NFY_ALRAMMISSION_PARAM >
{
public:
	bool operator()( BM::Stream &rkPacket );
};

#endif // FREEDOM_DRAGONICA_CONTENTS_ALRAMMISSIONCLIENT_H