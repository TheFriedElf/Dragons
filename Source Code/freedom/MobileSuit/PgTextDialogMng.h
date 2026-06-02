#ifndef FREEDOM_DRAGONICA_CONTENTS_QUEST_PGTEXTDIALOGMNG_H
#define FREEDOM_DRAGONICA_CONTENTS_QUEST_PGTEXTDIALOGMNG_H

typedef struct tagTextDialg : public SQuestDialog
{
	int iTitleID;
} STextDialg;

class PgTextDialogMng : public PgIXmlObject
{
	typedef std::map< int, STextDialg > ContTextDialog;
	typedef ContTextDialog::iterator iterator;
	typedef ContTextDialog::const_iterator const_iterator;
	typedef ContTextDialog::key_type key_type;
	typedef ContTextDialog::mapped_type mapped_type;

public:
	PgTextDialogMng();
	~PgTextDialogMng();

	void Reload(); 
	STextDialg const* Get(int const iDialogID) const;

	virtual bool ParseXml(const TiXmlNode *pkNode, void *pArg = 0, bool bUTF8 = false);

private:
	ContTextDialog m_kMap;
};

namespace lwTextDialog
{
	void RegisterWrapper(lua_State *pkState);
	void CallTextDialog(int const iDialogID, lwGUID kNpcGuid);
};

#define g_kTextDialogMng SINGLETON_STATIC(PgTextDialogMng)

#endif // FREEDOM_DRAGONICA_CONTENTS_QUEST_PGTEXTDIALOGMNG_H