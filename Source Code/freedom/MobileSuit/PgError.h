#ifndef FREEDOM_DRAGONICA_UTIL_PGERROR_H
#define FREEDOM_DRAGONICA_UTIL_PGERROR_H

#define PgError(msg)							_PgError(__FUNCTION__, msg)
#define PgError1(msg, p)						_PgError(__FUNCTION__, msg, p)
#define PgError2(msg, p, p2)					_PgError(__FUNCTION__, msg, p, p2)
#define PgError3(msg, p, p2, p3)				_PgError(__FUNCTION__, msg, p, p2, p3)
#define PgError4(msg, p, p2, p3, p4)			_PgError(__FUNCTION__, msg, p, p2, p3, p4)
#define PgError5(msg, p, p2, p3, p4, p5)		_PgError(__FUNCTION__, msg, p, p2, p3, p4, p5)
#define PgError6(msg, p, p2, p3, p4, p5, p6)	_PgError(__FUNCTION__, msg, p, p2, p3, p4, p5, p6)

#define PgOutputPrint(msg)								_PgOutputDebugString(msg)
#define PgOutputPrint1(msg, p)							_PgOutputDebugString(msg, p)
#define PgOutputPrint2(msg, p, p2)						_PgOutputDebugString(msg, p, p2)
#define PgOutputPrint3(msg, p, p2, p3)					_PgOutputDebugString(msg, p, p2, p3)
#define PgOutputPrint4(msg, p, p2, p3, p4)				_PgOutputDebugString(msg, p, p2, p3, p4)
#define PgOutputPrint5(msg, p, p2, p3, p4, p5)			_PgOutputDebugString(msg, p, p2, p3, p4, p5)
#define PgOutputPrint6(msg, p, p2, p3, p4, p5, p6)		_PgOutputDebugString(msg, p, p2, p3, p4, p5, p6)

#define PgXmlError(xml, msg)							_PgXmlError(__FUNCTION__, xml, msg)
#define PgXmlError1(xml, msg, p)						_PgXmlError(__FUNCTION__, xml, msg, p)
#define PgXmlError2(xml, msg, p, p2)					_PgXmlError(__FUNCTION__, xml, msg, p, p2)
#define PgXmlError3(xml, msg, p, p2, p3)				_PgXmlError(__FUNCTION__, xml, msg, p, p2, p3)
#define PgXmlError4(xml, msg, p, p2, p3, p4)			_PgXmlError(__FUNCTION__, xml, msg, p, p2, p3, p4)
#define PgXmlError5(xml, msg, p, p2, p3, p4, p5)		_PgXmlError(__FUNCTION__, xml, msg, p, p2, p3, p4, p5)
#define PgXmlError6(xml, msg, p, p2, p3, p4, p5, p6)	_PgXmlError(__FUNCTION__, xml, msg, p, p2, p3, p4, p5, p6)

extern void CALLBACK PgOutputDebugStringFromXUI(const wchar_t* pcOut);
extern int g_iWinMessageLogTab;

#ifndef EXTERNAL_RELEASE
	extern void PgErrorFunc(char const *pcCaption, char const *pcMsg, ...);
	extern void PgOutputDebugString(char const *pcMsg, ...);
	extern void PgOutputDebugStringLevel(char const *pcMsg, int const iLevel=0);
	extern void WriteToConsole(char const *pcMsg, ...);
	extern void PgOutputDebugStringFromGameBryo(char const* pcOut);
	extern void PgMessageBox(char const *pcCaption, char const *pcMsg, ...);	//	Debug,Release 모드에서만 동작한다
	extern void PgXmlErrorFunc(char const *pcCaption, const TiXmlNode *pkXml, char const *pcMsg, ...);
#else
	#define WriteToConsole __noop
#endif


#ifdef EXTERNAL_RELEASE //컴파일시 포함되지 않게끔.
	#define _PgOutputDebugStringLevel /##/
	#define _PgOutputDebugString	/##/
	#define _PgMessageBox	/##/
	#define _PgError	/##/
	#define _PgXmlError	/##/
	
#else
	#define _PgOutputDebugStringLevel	PgOutputDebugStringLevel
	#define _PgOutputDebugString	PgOutputDebugString
	#define _PgMessageBox	PgMessageBox
	#define _PgError	PgErrorFunc
	#define _PgXmlError	PgXmlErrorFunc
#endif
#endif //FREEDOM_DRAGONICA_UTIL_PGERROR_H