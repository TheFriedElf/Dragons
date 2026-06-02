#ifndef FREEDOM_DRAGONICA_UTIL_PGNIENTITYSTREAMINGASCII_H
#define FREEDOM_DRAGONICA_UTIL_PGNIENTITYSTREAMINGASCII_H

#define PG_USE_PGENTITYSTREAM

class PgNiEntityStreamingAscii : public NiEntityStreamingAscii
{
public:
	PgNiEntityStreamingAscii(void);
	virtual ~PgNiEntityStreamingAscii(void);

    virtual NiBool Load(char const* pcFileName);
	NiBool PgStoreWorkingPath(char const* pcFileName);

public:
    static void _SDMInit();
    static void _SDMShutdown();
};

NiFactoryDeclareStreaming(PgNiEntityStreamingAscii);
#endif // FREEDOM_DRAGONICA_UTIL_PGNIENTITYSTREAMINGASCII_H