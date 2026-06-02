#ifndef FREEDOM_DRAGONICA_CONTENTS_JOBSKILL_PGJOBSKILLLERAN_H
#define FREEDOM_DRAGONICA_CONTENTS_JOBSKILL_PGJOBSKILLLERAN_H

namespace JobSkillGuide {
	// local function
	bool GetGuideItemName(int const iSaveIdx, std::wstring& rkOut);
}

class PgJobSkill_NfySaveIdx
{
private:
	typedef std::queue<int> CONT_NFY_SAVEIDX;
	CONT_NFY_SAVEIDX m_kContSaveIdx;
public:
	PgJobSkill_NfySaveIdx() {};
	~PgJobSkill_NfySaveIdx() {};
	void Push(int const iSaveIdx);
	int Pop();
	bool Empty() const;
	void Clear();
};
#define g_kJS_NfySaveIdxMgr SINGLETON_STATIC(PgJobSkill_NfySaveIdx)


#endif // FREEDOM_DRAGONICA_CONTENTS_JOBSKILL_PGJOBSKILLLERAN_H