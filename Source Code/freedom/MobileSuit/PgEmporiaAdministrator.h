#ifndef FREEDOM_DRAGONICA_CONTENTS_EMPORIA_PGEMPORIAADMINISTRATOR_H
#define FREEDOM_DRAGONICA_CONTENTS_EMPORIA_PGEMPORIAADMINISTRATOR_H

namespace lwPgEmporiaAdministrator
{
	void RegisterWrapper(lua_State *pkState);

	void EmporiaTalkAdministrator(lwGUID kGuid);
	
	void EmporiaAddonBuildingConstruct(int const iBuildIDX, lwGUID kGuid);
	void EmporiaAddonBuildingDestruct(int const iBuildIDX, lwGUID kGuid);
	void EmporiaAddonBuildingMaintenancePayment(int const iBuildIDX, lwGUID kGuid);
	bool EmporiaAddonBuildingCreated(int const iBuildIDX);
	bool EmporiaAddonBuildingUseable(int const iBuildIDX);
	bool EmporiaEntryOpenState();
	void EmporiaEntryOpenToOtherUser(BYTE const bState, lwGUID kGuid);
	void BuyEmporiaTeleCard(int const iCommand, lwGUID kGuid);

	bool CheckExistGuildEmporia();
	bool CheckEmporiaGuildOwner();

	//private:
	bool RecvEmporia_AdminPacket(WORD wPacketType, BM::Stream& kPacket);
	bool CheckNpc(BM::GUID const& kGuid);
	void GetBuildingUseTimeToText(__int64 const iRemainTime, std::wstring& kText);
	void SendEmporiaAddonBuildingOrder(short const iBuildIDX, BM::GUID const& kGuid, short const iWeekDate);
	std::wstring const UpdateNpcTalkBuildingUseTime(int const iBuildingNo);
	void UpdateNpcTalkDialog(std::wstring const& kText);
}

#endif // FREEDOM_DRAGONICA_CONTENTS_EMPORIA_PGEMPORIAADMINISTRATOR_H