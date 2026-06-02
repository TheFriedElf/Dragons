#ifndef FREEDOM_DRAGONICA_CONTENTS_QUEST_PGMISSIONCARD_H
#define FREEDOM_DRAGONICA_CONTENTS_QUEST_PGMISSIONCARD_H
class PgMissionCard
{
protected:
	ContHaveItemNoCount	m_MQCardList;

public:
	void MQCardList_Clear();
	void MQCardList_Insert(ContHaveItemNoCount const &List);
	int const MQCardList_SelectCard(int const iCnt) const;
	size_t const MQCardList_GetCardCount() const;

	PgMissionCard(void);
	virtual ~PgMissionCard(void);
};

#endif // FREEDOM_DRAGONICA_CONTENTS_QUEST_PGMISSIONCARD_H