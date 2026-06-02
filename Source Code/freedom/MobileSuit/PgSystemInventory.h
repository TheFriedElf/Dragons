#ifndef FREEDOM_DRAGONICA_CONTENTS_SUPERGROUND_PGSYSTEMINVENTORY_H
#define FREEDOM_DRAGONICA_CONTENTS_SUPERGROUND_PGSYSTEMINVENTORY_H

class PgSystemInventory
{
protected:
	static float const fUpTime;
	typedef PgBase_Item value_type;
	typedef std::list< value_type > ContRecvSystemInven;
	typedef std::list< size_t > ContEmptySetNo;
	typedef std::vector< value_type > ContSystemInven;
	typedef ContSystemInven::const_iterator const_iterator;
	typedef std::vector< size_t > ContSetNoVec;

	typedef struct tagSystemInven
	{
		tagSystemInven(size_t const SetNo, ContRecvSystemInven const &rkRecvInven, float const StartTime)
		{
			Clear();

			iSetNo = SetNo;
			kInven.insert( kInven.end(), rkRecvInven.begin(), rkRecvInven.end() );
			iCount = kInven.size();
			fStartTime = StartTime;
			fEndTime = StartTime + fUpTime;
		}

		void Clear()
		{
			iSetNo = 0;
			kInven.clear();
			iCount = 0;
			fStartTime = 0.f;
		}

		value_type* Get(size_t const iCur)
		{
			if( kInven.size() > iCur )
			{
				return &kInven.at(iCur);
			}
			return NULL;
		}

		bool Remove(CONT_ITEMGUID const &rkVec)
		{
			size_t iRemovedCount = 0;
			ContSystemInven::iterator iter = kInven.begin();
			while( kInven.end() != iter )
			{
				ContSystemInven::value_type &rkElement = (*iter);
				CONT_ITEMGUID::const_iterator find_iter = std::find( rkVec.begin(), rkVec.end(), rkElement.Guid() );
				if( rkVec.end() != find_iter )
				{
					rkElement = PgBase_Item::NullData();
					--iCount;
					++iRemovedCount;
				}
				++iter;
			}
			return 0 != iRemovedCount;
		}

		bool operator == (size_t const& rhs) const
		{
			return iSetNo == rhs;
		}

		bool Empty() const				{ return 0 == iCount; }
		const_iterator begin() const	{ return kInven.begin(); }
		const_iterator end() const		{ return kInven.end(); }

		size_t iSetNo;
		ContSystemInven kInven;
		float fStartTime;
		float fEndTime;
	private:
		size_t iCount;
	} SSystemInven;
	typedef std::list< SSystemInven > ContSystemInvenList;

public:
	PgSystemInventory();
	~PgSystemInventory();

	void Clear();

	value_type const* GetAt(size_t const iCur);

	void RecvSystemInven(BM::Stream &rkPacket);
	void RecvModifySystemInven(BM::Stream &rkPacket);
	void RecvRemove(BM::Stream &rkPacket);
	void SendItemGetAt(int const iAt, SItemPos const &rkTargetPos);
	void SendItemGet(CONT_SYS2INV_INFO const &rkReqList);
	void SendDropSysInven(size_t const iSetNo);
	void SendDropSysInven(CONT_ITEMGUID const &rkVec);
	void SendGetSysInven(size_t const iSetNo);
	void CheckSystemInventory();
	void TimeOutSystemInventory(size_t const iSetNo);

protected:
	bool SysInvenSetToGuid(size_t const iSetNo, CONT_ITEMGUID &rkOut);
	void Update(SSystemInven const &rkSystemInven);
	void Remove(size_t const iSetNo);
	void RemoveProcess(CONT_ITEMGUID const &rkVec);

private:
	mutable Loki::Mutex m_kMutex;
	ContSystemInvenList m_kSysInv;
	ContEmptySetNo m_kEmptyList;
};

namespace lwSystemInventory
{
	void RegisterWrapper(lua_State *pkState);
	void SendGetSystemInventory(int const iSetNo);
	void DropSystemInventory(int const iSetNo);
	void CheckSystemInventory();
	void TimeOutSystemInventory(int const iSetNo);
}

#define g_kSystemInven SINGLETON_STATIC(PgSystemInventory)

#endif // FREEDOM_DRAGONICA_CONTENTS_SUPERGROUND_PGSYSTEMINVENTORY_H