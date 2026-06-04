g_iHiddenTimeLimit = 0
g_iHiddenTotalTimeLimit = 0
g_fHiddenStartTimeLimit = 0
COLOR_RED = 0xFFFF0000
COLOR_DEFAULT= 0xFF7b2e00
function OnClick_DeadDlg_BtnOK()
	CloseUI("DeadDlg")
	if Get_CMP_STATE() == 0 then
		local packet = NewPacket(8226)--PT_C_M_REQ_DEFAULT_MAP_MOVE
		Net_Send(packet)
		DeletePacket(packet)
	end
end

function OnTick_DeadDlg_Text(kSelf)
	local kText = GetTT(3):GetStr()
end

function OnDisplay_DeadDlg_AniBar(kSelf)
	local kTailTxt = GetTT(7):GetStr()
	local kMinTxt = GetTT(8):GetStr()
	local kSecTxt = GetTT(9):GetStr()
	if( CheckNil(g_world == nil) ) then return false end
	if( CheckNil(g_world:IsNil()) ) then return false end
	local fCountDown = g_kRevivalTime - g_world:GetAccumTime()
	local kResultTxt = ""
	if 0 > fCountDown then
		--Nothing
	elseif 60 < fCountDown then
		local iMin = math.floor(fCountDown / 60)
		kResultTxt = kResultTxt ..  iMin .. kMinTxt .. " " .. kTailTxt
	elseif 60 > fCountDown then
		local iSec = math.floor(fCountDown)
		kResultTxt = kResultTxt ..  iSec .. kSecTxt .. " " .. kTailTxt
	end
	kSelf:DisplayTimeBarMsg(kResultTxt, false, true)
end

function SetMyEquipBGIndex(wnd)
	if wnd:IsNil() == true then
		return
	end
	
	local kMyPilot = g_pilotMan:GetPlayerPilot()
	if kMyPilot:IsNil() == false then
		local job = kMyPilot:GetAbil(AT_CLASS)
		wnd:SetUVIndex(job)
	end
end 

function SetOtherEquipBGIndex(wnd)
	if wnd:IsNil() == true then
		return
	end
	local winfo = GetUIWnd("SFRM_OTHER_CharInfo")
	if winfo:IsNil() == false then
		local kPilot = g_pilotMan:FindPilot(winfo:GetCustomDataAsGuid())
		if kPilot:IsNil() == false then
			local job = kPilot:GetAbil(AT_CLASS)
			wnd:SetUVIndex(job)
		end
	end
end

function DisplayBossGaugeBar(kSelf)
	if kSelf:IsNil() then
		ODS("DisplayBossGaugeBar Self Is Nil\n")
		return
	end
	
	local kAniBar = kSelf:GetControl("BAR_HP")
	if kAniBar:IsNil() then
	    ODS("DisplayBossGaugeBar's BAR_HP Is Nil\n")
	    return
	end
	
	local kBG = kSelf:GetControl("BG")
	if kBG:IsNil() then
	   ODS("DisplayBossGaugeBar's BG Is Nil\n")
	   return
	end
	
	local iIndex = kAniBar:GetUVIndex()-1
	if iIndex < 1 then
	    kBG:Visible(false)
	else
	    kBG:Visible(true)
		kBG:SetUVIndex(iIndex)
	end
end

g_kOtherGuid = GUID("")
function SetOtherGuid(kGuid)
	g_kOtherGuid = kGuid
end

function GetOtherGuid()
	return g_kOtherGuid
end

g_InvGrp = {}
g_InvGrp[1] = { ["BG"] = "SFRM_EQUIP_BG", ["BTN"] = "CBTN_EQUIP"}
g_InvGrp[2] = { ["BG"] = "SFRM_CONSUM_BG", ["BTN"] = "CBTN_CONSUM"}
g_InvGrp[3] = { ["BG"] = "SFRM_ETC_BG", ["BTN"] = "CBTN_ETC"}
g_InvGrp[4] = { ["BG"] = "SFRM_CASH_BG", ["BTN"] = "CBTN_CASH"}

function NextInvViewGroup()
	local iGroup = g_InvNiewGroup
	if false==KeyIsDown(1042) then--KEY_LSHIFT
		iGroup = iGroup + 1
		if 4 < iGroup then
			iGroup = 1
		end
	else
		iGroup = iGroup - 1
		if 1 > iGroup then
			iGroup = 4
		end
	end
	
	if nil == g_InvGrp[iGroup] then return end
	
	local wnd = GetUIWnd("Inv")
	if wnd:IsNil() then return end
	local wndParent = wnd:GetControl("SFRM_TITLE")
	if wndParent:IsNil() then return end
	local wndBG = wndParent:GetControl(g_InvGrp[iGroup]["BG"])
	if wndBG:IsNil() then return end
	local wndBtn = wndBG:GetControl(g_InvGrp[iGroup]["BTN"])
	if wndBtn:IsNil() then return end

	PreChangeInvViewGroup(iGroup, wndBtn)
end

function PreChangeInvViewGroup(iGrp, wnd)
	if 0>=iGrp or 4<iGrp or wnd:IsNil() then return end
	if wnd:GetCheckState() then return end
	CallNewInvItem_Tab(g_InvNiewGroup)
	g_InvNiewGroup = iGrp

	local wndParent = wnd:GetParent():GetParent()
	if wndParent:IsNil() then return end
	for i=1, table.getn(g_InvGrp) do
		local wndBG = wndParent:GetControl(g_InvGrp[i]["BG"])
		if false == wndBG:IsNil() then
			local wndBtn = wndBG:GetControl(g_InvGrp[i]["BTN"])
			if false == wndBtn:IsNil() then
				ODS("BG : " .. g_InvGrp[i]["BG"] .. " BTN : " .. g_InvGrp[i]["BTN"] .. "\n")
--				if wnd:GetID():GetStr() ~= g_InvGrp[i]["BTN"] then
--					ODS("false " ..  g_InvGrp[i]["BTN"] .. "\n") 
					wndBtn:CheckState(false)
					wndBtn:LockClick(false)
--				end
			end
		end
	end

	wnd:CheckState(true) 
	wnd:LockClick(true)

	ChangeInvViewGroup(g_InvNiewGroup)
	SetInvViewSize(wnd:GetParent():GetParent():GetParent())
	
	CallNewInvItem(g_InvNiewGroup)
end

function PreChangeInvViewGroupTop(iGrp)
	if g_InvNiewGroup == iGrp or 0>=iGrp or 4<iGrp then return end
	g_InvNiewGroup = iGrp

	local wnd = GetUIWnd("Inv")
	if wnd:IsNil() then return end
	local wndParent = wnd:GetControl("SFRM_TITLE")
	if wndParent:IsNil() then return end
	for i=1, table.getn(g_InvGrp) do
		local wndBG = wndParent:GetControl(g_InvGrp[i]["BG"])
		if false == wndBG:IsNil() then
			local wndBtn = wndBG:GetControl(g_InvGrp[i]["BTN"])
			if false == wndBtn:IsNil() then
				wndBtn:CheckState(i==iGrp)
				wndBtn:LockClick(i==iGrp)
			end
		end
	end
	ChangeInvViewGroup(g_InvNiewGroup)
	SetInvViewSize(wnd)
	
	CallNewInvItem(g_InvNiewGroup)
end
 
function CloseAllWarnMessage()
	for i=0,3 do
		CloseUI("WarnOut_" .. i)
	end
end

function SocketSystemProcessItemCommit(UIParent)
	if true == UIParent:IsNil() then
		return
	end
	local ListWnd = UIParent:GetControl("LST_MONSTERCARD_LIST")
	if false == ListWnd:IsNil() then
		local SRItem = ListWnd:ListFirstItem()
		while false == SRItem:IsNil() do
			local SRItemWnd = SRItem:GetWnd()
			local SRIconWnd = SRItemWnd:GetControl("IMG_ICON")
			if false == SRIconWnd:IsNil() then
				local SRCheckWnd = SRIconWnd:GetControl("CBTN_ITEM_SELECT")
				if false == SRCheckWnd:IsNil() then
					if true == SRCheckWnd:GetCheckState() then
						ItemSocketSystemProcessRequest(SRItemWnd)
					end
				end
			end
			SRItem = ListWnd:ListNextItem(SRItem)
		end
	end
end

function SetSocketIndexSelect(UISelf, iIndex)	
	if 1 == iIndex then
		GetUIWnd("SFRM_SOCKET_SYSTEM"):GetControl("BTN_2"):CheckState(false)
		GetUIWnd("SFRM_SOCKET_SYSTEM"):GetControl("BTN_3"):CheckState(false)
	elseif 2 == iIndex then
		GetUIWnd("SFRM_SOCKET_SYSTEM"):GetControl("BTN_1"):CheckState(false)
		GetUIWnd("SFRM_SOCKET_SYSTEM"):GetControl("BTN_3"):CheckState(false)
	else
		GetUIWnd("SFRM_SOCKET_SYSTEM"):GetControl("BTN_1"):CheckState(false)
		GetUIWnd("SFRM_SOCKET_SYSTEM"):GetControl("BTN_2"):CheckState(false)
	end
	
	UISelf:LockClick(false)
	UISelf:CheckState(true)
	UISelf:LockClick(true)
	
	local wnd = ItemSocketSystemWnd(UISelf)
	if wnd == nil then
		return
	end		
	wnd:SetSelectOrderMenu(iIndex)
end

function IsSocketIndexCheck(UISelf, iIndex)
	local wnd = ItemSocketSystemWnd(UISelf)
	if wnd == nil then
		return
	end
	if true == wnd:IsSelectOrderMenuIndex(iIndex) then
		SetSocketIndexSelect(UISelf, iIndex)
	else
		UISelf:LockClick(false)
		UISelf:CheckState(true)
		UISelf:LockClick(true)
		
		if false == wnd:IsSocketItemEmpty() then
			wnd:DisplaySocketRemoveText(iIndex)
		end
	end
end

function RankingSelectedSet(UISelf)
	local	MainWnd = UISelf:GetParent():GetParent()
	for idx = 0, 17 do
		local	ChildWnd = MainWnd:GetControl("FRM_ITEM_"..idx)
		if false == ChildWnd:IsNil() then
			local pChild = ChildWnd:GetControl("CBTN_ITEM_SELECT")
			if false ~= pChild:IsNil() then
			SetBreak()
				return
			end
			pChild:CheckState(false)
			local pImage = pChild:GetControl("IMG_SELECT")
			if false == pImage:IsNil() then
				pImage:Visible(pChild:GetCheckState())
			end
		end
	end
	UISelf:CheckState(true)
	local Image = UISelf:GetControl("IMG_SELECT")
	if false == Image:IsNil() then
		Image:Visible(UISelf:GetCheckState())
	end
end

function AddEmoticon(idx)
	local ChatBar = ActivateUI("ChatBar")
	if false == ChatBar:IsNil() then
		local pEditWnd = ChatBar:GetControl("EDT_CHAT")
		if false == pEditWnd:IsNil() then
			local curText = pEditWnd:GetEditText()
			local iNeedLength = 2;
			if idx < 10 then
				iNeedLength = 2
			elseif idx < 100 then
				iNeedLength = 3
			elseif idx < 1000 then
				iNeedLength = 4
			else
				SetBreak()
			end
				
			if (curText:Length() + iNeedLength) <= pEditWnd:GetEditLimitLength() then
				ODS("edit control has "..pEditWnd:GetEditLimitLength().." cur: "..curText:Length().."\n", 0, 9999)
				pEditWnd:SetEditText(pEditWnd:GetEditText():GetStr().."#"..idx)
				pEditWnd:SetEditFocus(true)
			end
		end
	end
end

function UpdateChatColorButtonPreview()
	local ChatBar = ActivateUI("ChatBar")
	local Btn = ChatBar:GetControl("BTN_TEXT_COLOR")
	if false == Btn:IsNil() then
		local pChild = Btn:GetControl("FRM_COLOR_VIEW")
		if false == pChild:IsNil() then
			pChild:VOnCall()
		end
	end
end

function NormalizeChatColorValue(iColor)
	if iColor == nil then
		return 4294967295
	end

	if iColor < 0 then
		iColor = iColor + 4294967296
	end

	return iColor
end

function GetCurrentChatUIColor()
	return NormalizeChatColorValue(g_ChatMgrClient:GetFontColor())
end

function GetChatColorRGB(iColor)
	iColor = NormalizeChatColorValue(iColor)
	local iRGB = math.mod(iColor, 16777216)
	local iRed = math.floor(iRGB / 65536)
	local iGreen = math.floor(math.mod(iRGB, 65536) / 256)
	local iBlue = math.mod(iRGB, 256)
	return iRed, iGreen, iBlue
end

function UpdateChatColorSliderPreview(kWnd, iColor)
	if kWnd == nil or true == kWnd:IsNil() then
		return
	end

	local kBG = kWnd:GetControl("SFRM_BG")
	if true == kBG:IsNil() then
		return
	end

	local iRed, iGreen, iBlue = GetChatColorRGB(iColor)
	local kPreview = kBG:GetControl("SFRM_COLOR_PREVIEW")
	if false == kPreview:IsNil() then
		kPreview:SetColor(NormalizeChatColorValue(iColor))
	end

	kBG:GetControl("FRM_HEX_VALUE"):SetStaticText(string.format("#%02X%02X%02X", iRed, iGreen, iBlue))
	kBG:GetControl("FRM_COLOR_VALUE_R"):SetStaticText(tostring(iRed))
	kBG:GetControl("FRM_COLOR_VALUE_G"):SetStaticText(tostring(iGreen))
	kBG:GetControl("FRM_COLOR_VALUE_B"):SetStaticText(tostring(iBlue))
end

function InitChatColorSlider(kWnd)
	if kWnd == nil or true == kWnd:IsNil() then
		return
	end

	local kBG = kWnd:GetControl("SFRM_BG")
	if true == kBG:IsNil() then
		return
	end

	local iColor = GetCurrentChatUIColor()
	local iRed, iGreen, iBlue = GetChatColorRGB(iColor)
	kBG:GetControl("HSCRL_COLOR_R"):SetScrollCur(iRed)
	kBG:GetControl("HSCRL_COLOR_G"):SetScrollCur(iGreen)
	kBG:GetControl("HSCRL_COLOR_B"):SetScrollCur(iBlue)
	UpdateChatColorSliderPreview(kWnd, iColor)
end

function ApplyChatColorFromSlider(kWnd)
	if kWnd == nil or true == kWnd:IsNil() then
		return
	end

	local kBG = kWnd:GetControl("SFRM_BG")
	local iRed = kBG:GetControl("HSCRL_COLOR_R"):GetScrollCur()
	local iGreen = kBG:GetControl("HSCRL_COLOR_G"):GetScrollCur()
	local iBlue = kBG:GetControl("HSCRL_COLOR_B"):GetScrollCur()
	local iColor = 4278190080 + (iRed * 65536) + (iGreen * 256) + iBlue
	g_ChatMgrClient:SetFontColor(iColor)
	UpdateChatColorSliderPreview(kWnd, iColor)
	UpdateChatColorButtonPreview()
end

function ChangeChatOutSizeCheck(CalcValue)
	local lwChatBar = GetUIWnd("ChatBar")
	if lwChatBar:IsNil() then
		return
	end
	local Child = lwChatBar:GetControl("BTN_EMOTION")
	if Child:IsNil() or Child:IsDisable() then
		return
	end
	local pEdit = lwChatBar:GetControl("EDT_CHAT")
	if pEdit:IsNil() or pEdit:IsEditFocus() then
		return
	end
	ChangeChatOutSize(CalcValue)
end

function OnInitTradeUnsealingScrollVal()
	g_fUnsealingScrollProgressTime = 0
	g_iUnsealingScrollProgressCount = 0
end
function OnIncTradeUnsealingScrollCount(kTop)
	local kSelf = UISelf
	local kTop = UISelf:GetParent()
	local iCount = kSelf:GetCustomDataAsInt()
	local iMaxCount = GetCanMaxTradeMaking(9001, 10)
	if iMaxCount < iCount + 1 then
		return
	end
	UpdateTradeUnselingScroll( kTop, iCount+1)
end
function OnDecTradeUnsealingScrollCount(kTop)
	local kSelf = UISelf
	local kTop = UISelf:GetParent()
	local iCount = kSelf:GetCustomDataAsInt()
	if 1 > iCount - 1 then
		return
	end
	UpdateTradeUnselingScroll( kTop, iCount-1)
end
function OnMaxTradeUnsealingScrollCount(kTop)
	local iMaxCount = GetCanMaxTradeMaking(9001, 10)
	UpdateTradeUnselingScroll(kTop, iMaxCount)
end
function OnSendTradeUnsealingScroll(iCount)
	if 0 == iCount then
		AddWarnDataTT(400450);
		return
	end
	if 0 == g_fUnsealingScrollProgressTime then
		
		if not PreCheckItemMaking(9001, iCount) then
			AddWarnDataTT(400450);
			return
		end
		
		g_iUnsealingScrollProgressCount = iCount
		g_fUnsealingScrollProgressTime = GetAccumTime()
	end
end

g_fUnsealingScrollProgressTime = 0
g_iUnsealingScrollProgressCount = 0
function OnProgressUnsealingScroll( kSelf )
	local kSize = kSelf:GetSize()
	
	if 0 == g_fUnsealingScrollProgressTime then
		kSelf:SetImgSize(kSize)
		return
	end
	
	local fDelta = GetAccumTime() - g_fUnsealingScrollProgressTime
	if fDelta >= 0.5 then
		local iCount = g_iUnsealingScrollProgressCount
		g_fUnsealingScrollProgressTime = 0
		g_iUnsealingScrollProgressCount = 0
		
		SendTradeUnsealingScroll( iCount )
	else
		local iH = fDelta/0.5*kSize:GetY()
		local iMinH = math.min(iH, kSize:GetY())
		kSize:SetY(iMinH)
		kSelf:SetImgSize(kSize)
	end
end

function OnIncItemConvertCount(kTop)
	local kSelf = UISelf
	local kTop = UISelf:GetParent()
	local iCount = kSelf:GetCustomDataAsInt()
	--local iMaxCount = GetCanMaxTradeMaking(9001, 10)	
	local wnd = ItemConvertSystemWnd(UISelf)
	local iMaxCount = wnd:ItemConvertMaxCount()
	if iMaxCount > 10 then
		iMaxCount = 10
	end	

	if iMaxCount < iCount + 1 then
		AddWarnDataTT(790535)
		return
	end	
	if true == wnd:InProgress() then
		AddWarnDataTT(790537)
	else	
		local iMaxCount = wnd:ItemConvertMaxCount()
		if 0 == iMaxCount then
			wnd:OnItemConvertInit(0)
			AddWarnDataTT(790538)
			return
		end
		if (iCount+1) > iMaxCount then
			wnd:OnItemConvertInit(iMaxCount)
			AddWarnDataTT(790535)
		else	
			wnd:OnItemConvertInit(iCount+1)
		end
	end
end
function OnDecItemConvertCount(kTop)
	local kSelf = UISelf
	local kTop = UISelf:GetParent()
	local iCount = kSelf:GetCustomDataAsInt()
	if 1 > iCount - 1 then		
		AddWarnDataTT(790535)
		return
	end
	local wnd = ItemConvertSystemWnd(UISelf)
	if true == wnd:InProgress() then
		AddWarnDataTT(790537)
	else
		local iMaxCount = wnd:ItemConvertMaxCount()
		if 0 == iMaxCount then
			wnd:OnItemConvertInit(0)
			AddWarnDataTT(790538)
			return
		end		
		if (iCount-1) > iMaxCount then
			wnd:OnItemConvertInit(iMaxCount)
		else
			wnd:OnItemConvertInit(iCount-1)
		end
	end
end
function OnMaxItemConvertCount(kTop)
	local iMaxCount = GetCanMaxTradeMaking(9001, 10)
	local wnd = ItemConvertSystemWnd(UISelf)
	if true == wnd:InProgress() then
		AddWarnDataTT(790537)
	else
		iMaxCount = wnd:ItemConvertMaxCount()
		if 0 == iMaxCount then
			wnd:OnItemConvertInit(0)
			AddWarnDataTT(790538)
			return
		end		
		if iMaxCount > 10 then
			iMaxCount = 10
		end
		wnd:OnItemConvertInit(iMaxCount)
	end
end
function OnSendItemConvertSystem(iCount)
	if 0 == iCount then
		AddWarnDataTT(790539);
		return
	end
	if 0 == g_fUnsealingScrollProgressTime then
		local wnd = ItemConvertSystemWnd(UISelf)
		if not wnd:PreCheckItemConvert(iCount) then
			AddWarnDataTT(790538);
			return
		end
		wnd:SetInProgress()
		
		g_iUnsealingScrollProgressCount = iCount
		g_fUnsealingScrollProgressTime = GetAccumTime()
		
		local	kActor = GetMyActor()
		if false == kActor:IsNil() then
			kActor:AttachSound(2998, "Enchant")
		end		
	end
end
function OnProgressItemConvert( kSelf )
	local kSize = kSelf:GetSize()
	
	if 0 == g_fUnsealingScrollProgressTime then
		kSelf:SetImgSize(kSize)
		return
	end
	
	local fDelta = GetAccumTime() - g_fUnsealingScrollProgressTime
	if fDelta >= 2.3 then
		local iCount = g_iUnsealingScrollProgressCount
		g_fUnsealingScrollProgressTime = 0
		g_iUnsealingScrollProgressCount = 0
		
		local wnd = ItemConvertSystemWnd(UISelf)
		if wnd == nil then
			return
		end
		wnd:SendItemConvert()
	else
		local iH = fDelta/2.3*kSize:GetY()
		local iMinH = math.min(iH, kSize:GetY())
		kSize:SetY(iMinH)
		kSelf:SetImgSize(kSize)
	end
end

g_kEQPosIconSRCTable = {}
g_kEQPosIconSRCTable[1] = { SRC="../Data/6_ui/info/ifIconBp.tga", U=6, V=6, W=240, H=240 }
g_kEQPosIconSRCTable[2] = { SRC="../Data/6_ui/info/ifIconEtc.tga", U=3, V=5, W=120, H=200 }
g_kEQPosIconSRCTable[3] = { SRC="../Data/6_ui/info/ifIconWp.tga", U=6, V=6, W=240, H=240 }

g_kEQPosIconTable = {}
g_kEQPosIconTable[1804] = { UVIndex=13, SRC=2 } --펫
g_kEQPosIconTable[70002] = { UVIndex=7, SRC=2 } --얼굴
g_kEQPosIconTable[70003] = { UVIndex=2, SRC=2 }	--어깨
g_kEQPosIconTable[70004] = { UVIndex=2, SRC=1 } --백팩
g_kEQPosIconTable[70005] = { UVIndex=7, SRC=2 } --안경
g_kEQPosIconTable[70006] = { UVIndex=12, SRC=3 } --무기
g_kEQPosIconTable[70007] = { UVIndex=8, SRC=2 } --방패
g_kEQPosIconTable[70008] = { UVIndex=10, SRC=2 } --목걸이
g_kEQPosIconTable[70009] = { UVIndex=9, SRC=2 } --귀걸이
g_kEQPosIconTable[70010] = { UVIndex=12, SRC=2 } --반지
g_kEQPosIconTable[70011] = { UVIndex=11, SRC=2 } --벨트
g_kEQPosIconTable[70020] = { UVIndex=1, SRC=2 } --투구
g_kEQPosIconTable[70021] = { UVIndex=3, SRC=2 } --상의
g_kEQPosIconTable[70022] = { UVIndex=5, SRC=2 } --하의
g_kEQPosIconTable[70023] = { UVIndex=6, SRC=2 } --신발
g_kEQPosIconTable[70024] = { UVIndex=4, SRC=2 } --장갑
g_kEQPosIconTable[70025] = { UVIndex=14, SRC=2 } --훈장
g_kEQPosIconTable[399906] = { UVIndex=15, SRC=2 } --풍선

g_kEquipUIDefaultIconArray = { 1804, 70011, 70008, 70022, 70020, 70023, 70003, 70010, 70004, 70010, 70021, 70009, 70024, 70025, 70006, 70007 }
g_kEquipUICashIconArray = { 70005, 70011, 70008, 70022, 70020, 70023, 70003, 70010, 70004, 70010, 70021, 70009, 70024, 399906, 70006, 70007 }

function EquipUIIconDefaultInitialize(Wnd)
	if Wnd:IsNil() then 
		return 
	end
	VisibleHideCashInven(Wnd:GetParent(), false)
	VisibleHideEquipInven(Wnd:GetParent(), true)
	for idx = 0, 15 do
		local ChildIconBg = Wnd:GetControl("FRM_ICON_BG"..idx)
		if not ChildIconBg:IsNil() then
			ChildIconBg:ChangeImage("../Data/6_ui/info/ifIconBg.tga")
		end
		local ChildIcon = Wnd:GetControl("FRM_ICON_NAME"..idx)
		if not ChildIcon:IsNil() then
			local TTWNo = g_kEquipUIDefaultIconArray[ idx + 1 ]
			ChildIcon:SetStaticTextW(GetTextW(TTWNo))
			
			local IconTable = g_kEQPosIconTable[ TTWNo ]
			if IconTable ~= nil then
				local SRCTable = g_kEQPosIconSRCTable[ IconTable.SRC ]
				if SRCTable ~= nil then
					ChildIcon:ChangeImage(SRCTable.SRC)					
					local Size = ChildIcon:GetImgSize()
					Size:SetX(SRCTable.W)
					Size:SetY(SRCTable.H)
					ChildIcon:SetImgSize(Size)
					ChildIcon:SetMaxUVIndex(SRCTable.U, SRCTable.V)
					if TTWNo == 70006 then
						SetMyEquipBGIndex(ChildIcon)
					else
						ChildIcon:SetUVIndex(IconTable.UVIndex)
					end
				end
			end
		end
	end
end

function EquipUIIconCashInitialize(Wnd)
	if Wnd:IsNil() then 
		return 
	end
	VisibleHideCashInven(Wnd:GetParent(), true)
	VisibleHideEquipInven(Wnd:GetParent(), false)
	for idx = 0, 15 do
		local ChildIconBg = Wnd:GetControl("FRM_ICON_BG"..idx)
		if not ChildIconBg:IsNil() then
			ChildIconBg:ChangeImage("../Data/6_ui/info/ifIconBgCS.tga")
		end
		local ChildIcon = Wnd:GetControl("FRM_ICON_NAME"..idx)
		if not ChildIcon:IsNil() then
			local TTWNo = g_kEquipUICashIconArray[ idx + 1 ]
			ChildIcon:SetStaticTextW(GetTextW(TTWNo))
			
			local IconTable = g_kEQPosIconTable[ TTWNo ]
			if IconTable ~= nil then
				local SRCTable = g_kEQPosIconSRCTable[ IconTable.SRC ]
				if SRCTable ~= nil then
					ChildIcon:ChangeImage(SRCTable.SRC)
					
					local Size = ChildIcon:GetImgSize()
					Size:SetX(SRCTable.W)
					Size:SetY(SRCTable.H)
					ChildIcon:SetImgSize(Size)
					ChildIcon:SetMaxUVIndex(SRCTable.U, SRCTable.V)
					ChildIcon:SetUVIndex(IconTable.UVIndex)
					if TTWNo == 70006 then
						SetMyEquipBGIndex(ChildIcon)
					else
						ChildIcon:SetUVIndex(IconTable.UVIndex)
					end
				end
			end
		end
	end
end

--/////////////////////

function VisibleHideCashInven(kTopWnd, bVisible)
	if nil == kTopWnd then return end
	if kTopWnd:IsNil() then return end
	
	local kCashHideWndName = {}
	kCashHideWndName[5]="BTN_VISIBLECASH_1"
	kCashHideWndName[20]="BTN_VISIBLECASH_3"
	kCashHideWndName[3]="BTN_VISIBLECASH_4"
	kCashHideWndName[4]="BTN_VISIBLECASH_5"
	kCashHideWndName[21]="BTN_VISIBLECASH_6"
	kCashHideWndName[24]="BTN_VISIBLECASH_7"
	kCashHideWndName[22]="BTN_VISIBLECASH_9"
	kCashHideWndName[23]="BTN_VISIBLECASH_10"
	kCashHideWndName[6]="BTN_VISIBLECASH_14"
	kCashHideWndName[13]="BTN_VISIBLECASH_15"
	for kKey, kVal in pairs(kCashHideWndName) do
		local kBtn = kTopWnd:GetControl(kVal)
		if not kBtn:IsNil() then
			kBtn:CheckState( not Option_GetCashItemEquipHide(kKey) )
			kBtn:Visible( bVisible )
		end
	end
end

function SetCashHideItemInvenPos(kSelf, iPos)
	if nil == kSelf then return end
	if kSelf:IsNil() then return end
	local bNowState = kSelf:GetCheckState()
	Option_SetCashItemEquipHide( iPos, bNowState )
	kSelf:CheckState( not bNowState )
end

function VisibleHideEquipInven(kTopWnd, bVisible)
	if nil == kTopWnd then return end
	if kTopWnd:IsNil() then return end
	
	local kEquipHideWndName = {}
	--kEquipHideWndName[5]="BTN_VISIBLE_EQUIP_1"
	--kEquipHideWndName[8]="BTN_VISIBLE_EQUIP_2"
	kEquipHideWndName[20]="BTN_VISIBLE_EQUIP_3"
	kEquipHideWndName[3]="BTN_VISIBLE_EQUIP_4"
	kEquipHideWndName[4]="BTN_VISIBLE_EQUIP_5"
	kEquipHideWndName[21]="BTN_VISIBLE_EQUIP_6"
	kEquipHideWndName[24]="BTN_VISIBLE_EQUIP_7"
	
	--kEquipHideWndName[11]="BTN_VISIBLE_EQUIP_8"
	kEquipHideWndName[22]="BTN_VISIBLE_EQUIP_9"
	kEquipHideWndName[23]="BTN_VISIBLE_EQUIP_10"
	--kEquipHideWndName[10]="BTN_VISIBLE_EQUIP_11"
	--kEquipHideWndName[30]="BTN_VISIBLE_EQUIP_12"
	--kEquipHideWndName[9]="BTN_VISIBLE_EQUIP_13"
	--kEquipHideWndName[12]="BTN_VISIBLE_EQUIP_14"
	
	for kKey, kVal in pairs(kEquipHideWndName) do
		local kBtn = kTopWnd:GetControl(kVal)
		if not kBtn:IsNil() then
			kBtn:CheckState( not Option_GetItemEquipHide(kKey) )
			kBtn:Visible( bVisible )
		end
	end
end

function SetEquipHideItemInvenPos(kSelf, iPos)
	if nil == kSelf then return end
	if kSelf:IsNil() then return end
	local bNowState = kSelf:GetCheckState()
	if true == Option_SetItemEquipHide( iPos, bNowState ) then
		kSelf:CheckState( not bNowState )
	end
end
--/////////////////////


function AchievementNotifyItemOnTick(Wnd)
	if Wnd:IsNil() then
		return
	end
	local iCount = Wnd:GetListItemCount()
	if iCount > 0 then
		local kItem = Wnd:ListFirstItem()
		if kItem:IsNil() == false then
			local kItemWnd = kItem:GetWnd()
			local fStartTime = kItemWnd:GetCustomDataAsFloat()
			if fStartTime == 0 then
				kItemWnd:SetCustomDataAsFloat(GetAccumTime())
				return
			else
				if (GetAccumTime() - fStartTime) > 20.0 then
					Wnd:ListDeleteItem(kItem)
				end
			end
		end
	else	
		Wnd:Close()
	end
end

function NotifyAlertFlash(Wnd1, Wnd2)
	if Wnd1:IsNil() or Wnd2:IsNil() then
		return
	end
	local TickCount = Wnd1:GetCustomDataAsInt()
	local fTickTime = Wnd2:GetCustomDataAsFloat()
	if TickCount < 6 then
		if (GetAccumTime() - fTickTime) > 1 then
			Wnd1:Visible(not Wnd1:IsVisible())
			Wnd1:SetCustomDataAsInt(TickCount+1)
			Wnd2:SetCustomDataAsFloat(GetAccumTime())
		end
	end
end

function SocketSystemProcessListSelect(UISelf)
	local SRParentWnd = UISelf:GetParent():GetParent()
	local ItemCount = SRParentWnd:GetListItemCount()
	if 0 < ItemCount then
		local SRItem = SRParentWnd:ListFirstItem()
		while false == SRItem:IsNil() do
			local SRItemWnd = SRItem:GetWnd()
			local SRIconWnd = SRItemWnd:GetControl("IMG_ICON")
			if false == SRIconWnd:IsNil() then
				local SRCheckWnd = SRIconWnd:GetControl("CBTN_ITEM_SELECT")
				if false == SRCheckWnd:IsNil() then
					SRCheckWnd:CheckState(false)
				end
			end
			SRItem = SRParentWnd:ListNextItem(SRItem)
		end
	end
	local CheckWnd = UISelf:GetControl("CBTN_ITEM_SELECT")
	if false == CheckWnd:IsNil() then
		CheckWnd:CheckState(true)
	end
end

function CheckMenuSocketItem(UISelf)
	local wnd = ItemSocketSystemWnd(UISelf)
	if wnd == nil then
		return
	end	
	
	wnd:CheckMenuSocketItem()
end

function CheckInvenTotalCount(UISelf,iHaveCount,iNeedCount)
	if UISelf == nil then
		return
	end	
	if(iNeedCount <= iHaveCount) then
		UISelf:SetFontColor(COLOR_DEFAULT)
	else
		UISelf:SetFontColor(COLOR_RED)
	end	
end

function SocketSystemUIUpdate(UISelf, iRet)
	--UISelf:CursorToIcon() 
	local kScrp = UISelf:GetParent():GetControl("SFRM_SHADOW"):GetControl("SFRM_COLOR_ITEM_NEED")
	if false == kScrp:IsNil() then
		local wnd = ItemSocketSystemWnd(UISelf)
		local bCreate = wnd:GetSocketSystemEmpty()
		local bSocketItem = wnd:IsSocketItemEmpty()
		local iCount = 0
		
		if wnd == nil then
			return
		end
		
		if true == wnd:IsSocketItemEmpty() then
			return
		end
		
		--[[local bCreateUpdate = true
		if 0 == iRet then
			bCreateUpdate = ((true == bCreate) or (true == bSocketItem))
		elseif 1 == iRet then
			bCreateUpdate = true
		else
			bCreateUpdate = false
		end--]]
		
		local iMenu = wnd:GetMenuType()
		UISelf:GetParent():GetControl("SFRM_MATERIAL_ITEM_COUNT"):Visible(false)
		if 1 == iMenu then
			if false == bSocketItem then
				iCount = wnd:GetSoulCreateCount()
				iHaveCount = wnd:GetHaveCount()
			end						
			UISelf:GetParent():GetControl("IMG_LOCK2"):Visible(true)	
			UISelf:GetParent():GetControl("BTN_REG2"):Visible(false)
			UISelf:GetParent():GetControl("BTN_DEREG2"):Visible(false)
			
			UISelf:GetParent():GetControl("IMG_LOCK3"):Visible(true)	
			UISelf:GetParent():GetControl("BTN_REG3"):Visible(false)
			UISelf:GetParent():GetControl("BTN_DEREG3"):Visible(false)
			

			UISelf:GetParent():GetControl("BTN_DEREG1"):Visible(false)
			UISelf:GetParent():GetControl("BTN_REG1"):Visible(true)	
			UISelf:GetParent():GetControl("BTN_EXTRACTION_REG"):Visible(false)
			UISelf:GetParent():GetControl("BTN_EXTRACTION_DEREG"):Visible(false)
			UISelf:GetParent():GetControl("SFRM_SHADOW"):GetControl("ICN_IN_GAME_ITEM"):Visible(false)
			UISelf:GetParent():GetControl("SFRM_SHADOW"):GetControl("ICN_SOULITEM"):Visible(true)
			UISelf:GetParent():GetControl("SFRM_SHADOW"):GetControl("ICN_SOULITEM"):SetCustomDataAsInt(79000030)
			UISelf:GetParent():GetControl("SFRM_SHADOW"):GetControl("ICN_SOULITEM"):GetControl("FRM_SOULITEM_TEXT"):SetStaticTextW(GetTT(400452))
			UISelf:GetParent():GetControl("SFRM_SHADOW"):GetControl("SFRM_COLOR_ITEM_NEED"):Visible(true)
			UISelf:GetParent():GetControl("ICN_MATERIAL1"):Visible(true)
			wnd:SetMaterialItem(1, true, true)
				
			local wnd = ItemSocketSystemWnd(UISelf:GetParent():GetControl("IMG_LOCK1"))
			wnd:SetLockSlot(1)
			iTTW_Title = 790130
			iTTW = 790105
			
			local iSelectOrder = wnd:GetMenuTypeOrder()
			wnd:SetSelectOrderMenu(iSelectOrder)
			
			--SocketSystemUICreateUpdate(UISelf, true)
		elseif 2 == iMenu then	
			UISelf:GetParent():GetControl("IMG_LOCK1"):Visible(true)	
			UISelf:GetParent():GetControl("BTN_REG1"):Visible(false)
			UISelf:GetParent():GetControl("BTN_DEREG1"):Visible(false)
			
			UISelf:GetParent():GetControl("IMG_LOCK3"):Visible(true)	
			UISelf:GetParent():GetControl("BTN_REG3"):Visible(false)
			UISelf:GetParent():GetControl("BTN_DEREG3"):Visible(false)			
			
			if true == wnd:IsInsureItemPos() then
				UISelf:GetParent():GetControl("BTN_DEREG2"):Visible(true)
				UISelf:GetParent():GetControl("BTN_REG2"):Visible(false)
			else
				UISelf:GetParent():GetControl("BTN_DEREG2"):Visible(false)
				UISelf:GetParent():GetControl("BTN_REG2"):Visible(true)
				UISelf:GetParent():GetControl("BTN_EXTRACTION_REG"):Visible(false)
				UISelf:GetParent():GetControl("BTN_EXTRACTION_DEREG"):Visible(false)
				UISelf:GetParent():GetControl("SFRM_SHADOW"):GetControl("ICN_IN_GAME_ITEM"):Visible(false)
				UISelf:GetParent():GetControl("SFRM_SHADOW"):GetControl("ICN_SOULITEM"):Visible(true)
				UISelf:GetParent():GetControl("SFRM_SHADOW"):GetControl("ICN_SOULITEM"):SetCustomDataAsInt(79000030)
				UISelf:GetParent():GetControl("SFRM_SHADOW"):GetControl("ICN_SOULITEM"):GetControl("FRM_SOULITEM_TEXT"):SetStaticTextW(GetTT(400452))
				UISelf:GetParent():GetControl("SFRM_SHADOW"):GetControl("SFRM_COLOR_ITEM_NEED"):Visible(true)
				UISelf:GetParent():GetControl("ICN_MATERIAL1"):Visible(true)
			end
			wnd:SetMaterialItem(2, false, true)
			
			iCount = wnd:GetSoulRemoveCount()
			iHaveCount = wnd:GetHaveCount()
			local wnd = ItemSocketSystemWnd(UISelf:GetParent():GetControl("IMG_LOCK2"))					
			wnd:SetLockSlot(2)
			iTTW_Title = 790131
			iTTW = 790119
			
			if 0 == wnd:GetSelectOrderMenu() then
				local iSelectOrder = wnd:GetMenuTypeOrder()
				wnd:SetSelectOrderMenu(iSelectOrder)
			end
			
			--SocketSystemUICreateUpdate(UISelf, false)		
		elseif 3 == iMenu then
			UISelf:GetParent():GetControl("IMG_LOCK1"):Visible(true)	
			UISelf:GetParent():GetControl("BTN_REG1"):Visible(false)
			UISelf:GetParent():GetControl("BTN_DEREG1"):Visible(false)
			
			UISelf:GetParent():GetControl("IMG_LOCK2"):Visible(true)	
			UISelf:GetParent():GetControl("BTN_REG2"):Visible(false)
			UISelf:GetParent():GetControl("BTN_DEREG2"):Visible(false)			
			
			if true == wnd:IsInsureItemPos() then
				UISelf:GetParent():GetControl("BTN_DEREG3"):Visible(true)
				UISelf:GetParent():GetControl("BTN_REG3"):Visible(false)
			else
				UISelf:GetParent():GetControl("BTN_DEREG3"):Visible(false)
				UISelf:GetParent():GetControl("BTN_REG3"):Visible(true)
				UISelf:GetParent():GetControl("BTN_EXTRACTION_REG"):Visible(false)
				UISelf:GetParent():GetControl("BTN_EXTRACTION_DEREG"):Visible(false)
				UISelf:GetParent():GetControl("SFRM_SHADOW"):GetControl("ICN_IN_GAME_ITEM"):Visible(false)
				UISelf:GetParent():GetControl("SFRM_SHADOW"):GetControl("ICN_SOULITEM"):Visible(true)
				UISelf:GetParent():GetControl("SFRM_SHADOW"):GetControl("ICN_SOULITEM"):SetCustomDataAsInt(79000030)
				UISelf:GetParent():GetControl("SFRM_SHADOW"):GetControl("ICN_SOULITEM"):GetControl("FRM_SOULITEM_TEXT"):SetStaticTextW(GetTT(400452))
				UISelf:GetParent():GetControl("SFRM_SHADOW"):GetControl("SFRM_COLOR_ITEM_NEED"):Visible(true)
				UISelf:GetParent():GetControl("ICN_MATERIAL1"):Visible(true)
			end
			wnd:SetMaterialItem(3, false, true)
			
			iCount = wnd:GetSoulResetCount()
			iHaveCount = wnd:GetHaveCount()
			local wnd = ItemSocketSystemWnd(UISelf:GetParent():GetControl("IMG_LOCK3"))					
			wnd:SetLockSlot(3)
			iTTW_Title = 790132
			iTTW = 790119
			
			local iSelectOrder = wnd:GetMenuTypeOrder()
			wnd:SetSelectOrderMenu(iSelectOrder)
			
			--SocketSystemUICreateUpdate(UISelf, false)
		else
			UISelf:GetParent():GetControl("IMG_LOCK2"):Visible(true)	
			UISelf:GetParent():GetControl("BTN_REG1"):Visible(false)
			UISelf:GetParent():GetControl("BTN_DEREG1"):Visible(false)
			
			UISelf:GetParent():GetControl("IMG_LOCK3"):Visible(true)	
			UISelf:GetParent():GetControl("BTN_REG3"):Visible(false)
			UISelf:GetParent():GetControl("BTN_DEREG3"):Visible(false)			
			
			if true == wnd:IsInsureItemPos() then
				UISelf:GetParent():GetControl("BTN_DEREG1"):Visible(false)
				UISelf:GetParent():GetControl("BTN_REG1"):Visible(false)
			else
				UISelf:GetParent():GetControl("BTN_DEREG1"):Visible(false)
				UISelf:GetParent():GetControl("BTN_REG1"):Visible(false)
				UISelf:GetParent():GetControl("SFRM_SHADOW"):GetControl("ICN_IN_GAME_ITEM"):Visible(true)
				UISelf:GetParent():GetControl("SFRM_SHADOW"):GetControl("ICN_SOULITEM"):Visible(false)
				UISelf:GetParent():GetControl("SFRM_SHADOW"):GetControl("ICN_SOULITEM"):GetControl("FRM_SOULITEM_TEXT"):SetStaticText("")
				UISelf:GetParent():GetControl("SFRM_SHADOW"):GetControl("SFRM_COLOR_ITEM_NEED"):Visible(true)
				UISelf:GetParent():GetControl("SFRM_MATERIAL_ITEM_COUNT"):Visible(true)
				UISelf:GetParent():GetControl("ICN_MATERIAL1"):Visible(false)
			end
									
			iCount = wnd:GetExtractionCount()
			iHaveCount = wnd:GetHaveCount()
			local wnd = ItemSocketSystemWnd(UISelf:GetParent():GetControl("IMG_LOCK1"))					
			wnd:SetLockSlot(2)
			iTTW_Title = 790163
			iTTW = 790162
			
			local iSelectOrder = wnd:GetMenuTypeOrder()
			wnd:SetSelectOrderMenu(iSelectOrder)
		end					
		UISelf:GetParent():GetControl("SFRM_TITLE"):GetControl("SFRM_COLOR"):GetControl("SFRM_TITLE_SDW"):SetStaticText(GetTextW(iTTW_Title):GetStr())
		UISelf:GetParent():GetControl("SFRM_EXPLANE"):SetStaticText(GetTextW(iTTW):GetStr())
		kScrp:SetStaticText( string.format(GetTextW(790177):GetStr(), iCount) )
		
		CheckInvenTotalCount(kScrp,iHaveCount,iCount)
	end
end

-- InGameGacha [Start] --------------------------------------------------------------------------
g_kIGGachaEff = {}
g_kIGGachaEff["START"] = 0
g_kIGGachaEff["ENDTIME"] = 2
g_kIGGachaEff["STARTTIME"] = 0
g_kIGGachaEff["COINTYPE"] = 1
g_kIGGachaEff["IS_SEND_PACKET"] = false

function ClearIGGachaUIData()
	g_kIGGachaEff["START"] = 0
	g_kIGGachaEff["ENDTIME"] = 2
	g_kIGGachaEff["STARTTIME"] = 0
	g_kIGGachaEff["COINTYPE"] = 1
	g_kIGGachaEff["IS_SEND_PACKET"] = false
end

function OnBuildIGGacha(kSelf, iBldIdx)
	if(true == kSelf:IsNil()) then return end
	
	local kBtnImg
	if(GetLocale() == LOCALE.NC_TAIWAN) then
		-- use only cash
		kBtnImg = "../Data/6_ui/gacha/gcBtCoin0"..(iBldIdx+1).."_2.tga"
		local kText = string.format(GetTextW(790621+iBldIdx):GetStr(), GetNeedCashIGGacha(iBldIdx+1) )
		local kHotKeyText = GetTT(2200 + iBldIdx)		
		kSelf:SetStaticText(kText..kHotKeyText:GetStr())
	else
		-- use only coin item
		kBtnImg = "../Data/6_ui/gacha/gcBtCoin0"..(iBldIdx+1)..".tga"
		SetTextOnThisUI(790611+iBldIdx, 2200+iBldIdx)
	end	
	
	kSelf:ButtonChangeImage(kBtnImg)	
end

function OnCallIGGachaButton(kSelf, iBldIdx)
	if(true == kSelf:IsNil()) then return end
	
	local kBtnImg
	if(GetLocale() == LOCALE.NC_TAIWAN) then
		-- use only cash
		local kText = string.format(GetTextW(790621+iBldIdx):GetStr(), GetNeedCashIGGacha(iBldIdx+1) )
		local kHotKeyText = GetTT(2200 + iBldIdx)		
		kSelf:SetStaticText(kText..kHotKeyText:GetStr())
	else
		-- use only coin item		
		SetTextOnThisUI(790611+iBldIdx, 2200+iBldIdx)
	end	
end

function SelectTabIGGacha(kParent, iBldIdx)
	if(true == kParent:IsNil()) then return end

	local i=0
	for  i=0, 3 do
		local kTabName = "CBTN_TAB"..i
		local kTabWnd = kParent:GetControl(kTabName)
		if(true ~= kTabWnd:IsNil()) then 
			if(iBldIdx == i) then 
				kTabWnd:CheckState(true)
				kTabWnd:LockClick(true)
			else
				kTabWnd:CheckState(false)
				kTabWnd:LockClick(false)
			end
		end	
	end
	
	ShowItemListIGGacha(iBldIdx+1) -- 빌드Idx는 0부터 시작, 코인Type은 1부터 시작 이므로
end

function UseCoinIGGacha(kParent, fEndTime, iCoinType)
	if(nil == kParent) then return end
	if(true == kParent:IsNil()) then return end	
	
	if(GetLocale() == LOCALE.NC_TAIWAN) then
		if(false == CheckEnoughIGGachaCash(iCoinType)) then
			CallUI("SFRM_CASHCHARGE_YESNO", true)
			--AddWarnDataTT(1900)
			return
		end
	else
		if(0 >= GetMyPlayerIGGachaCoinCnt(iCoinType)) then  
			AddWarnDataTT(550005)
			return 
		end		
	end
	
	if(1 == g_kIGGachaEff["START"]) then 
		SendUseIGGachaCoin(kParent)
		return
	end
	
	local kEffWnd = kParent:GetControl("FRM_EFFECT")
	if(true == kEffWnd:IsNil()) then return end
	kEffWnd:Visible(false)
	local kIconWnd = kParent:GetControl("ICON")
	if(true == kIconWnd:IsNil()) then return end
	kIconWnd:Visible(true)		
	
	local kQuestionWnd = kParent:GetControl("FRM_QUESTION")
	if(true == kQuestionWnd:IsNil()) then return end	
	kQuestionWnd:Visible(false)
	
	g_kIGGachaEff["START"] = 1
	g_kIGGachaEff["ENDTIME"] = fEndTime
	g_kIGGachaEff["STARTTIME"] = GetAccumTime()
	g_kIGGachaEff["COINTYPE"] = iCoinType
end

function OnTickIGGacha(kSelf)
	if(nil == kSelf) then return end
	if(true == kSelf:IsNil()) then return end
	
	local kIcon = kSelf:GetControl("ICON")
	local kQuestionWnd = kSelf:GetControl("FRM_QUESTION")
	if(false == kIcon:IsNil() and  false == kQuestionWnd:IsNil()) then	
		if(true == kIcon:IsVisible()) then
			kQuestionWnd:Visible(false)
		else
			kQuestionWnd:Visible(true)
		end
	end
	
	if(GetLocale() == LOCALE.NC_TAIWAN) then 
		--UpdateCashEnoughIGGacha(kSelf) -- 비활성화 사용 안함 
	else
		UpdateCoinCntIGGacha(kSelf)		
	end
	 
	if(1 ~= g_kIGGachaEff["START"]) then return end
	local fElapsTime = GetAccumTime() - g_kIGGachaEff["STARTTIME"]	
	
	if(fElapsTime >= g_kIGGachaEff["ENDTIME"] and false == g_kIGGachaEff["IS_SEND_PACKET"]) then 
	--충분한 지연시간이 됐으면 코인 사용함
		SendUseIGGachaCoin(kSelf)
	else
	end	
end

function SendUseIGGachaCoin(kSelf)
	if(nil == kSelf) then return end
	if(true == kSelf:IsNil()) then return end	
		
	local iCoinType = g_kIGGachaEff["COINTYPE"]
	
	if(GetLocale() == LOCALE.NC_TAIWAN) then 
		if(false == SendUseCashIGGacha(iCoinType)) then
			return -- 캐시부족
		end
	else		
		if(false == SendUseCoinIGGacha(iCoinType)) then 
			return  -- 코인부족
		end
	end
	g_kIGGachaEff["IS_SEND_PACKET"] = true
--[[
	local iAliveTime = 3000
	local kEffWnd = kSelf:GetControl("FRM_EFFECT")
	kEffWnd:Visible(true)
	kEffWnd:RefreshCalledTime()
	kEffWnd:SetAliveTime(iAliveTime)
	
	local kIconWnd = kSelf:GetControl("ICON")
	kIconWnd:Visible(true)	
	kIconWnd:RefreshCalledTime()
	kIconWnd:SetAliveTime(iAliveTime)

	local kQuestionWnd = kSelf:GetControl("FRM_QUESTION")
	kQuestionWnd:Visible(true)
	g_kIGGachaEff["START"] = 2
]]
end

function OnTickIconIGGacha(kSelf)
	if(nil == kSelf) then return end
	if(true == kSelf:IsNil()) then return end	
	if (1 == g_kIGGachaEff["START"]) then 
	local iItemNo = GetNextItemNo(g_kIGGachaEff["COINTYPE"])
		if(0 ~= iItemNo) then
			kSelf:SetCustomDataAsInt(iItemNo)
			PlaySoundByID("Item_Gachapon_Loop")
			kSelf:RefreshCalledTime()
		end
	end
	DrawIconToItemNo(kSelf, kSelf:GetCustomDataAsInt(), 2, Point2F(0.5,0.5))
end

function NotEnoughCoinOrCashIGGachaMessage()
	if(GetLocale() == LOCALE.NC_TAIWAN) then 		
		CallUI("SFRM_CASHCHARGE_YESNO", true)
		--AddWarnDataTT(1900)
	else
		AddWarnDataTT(550005)	
	end	
end

function ArrivedEarnPacketIGGacha()
	local kMain = GetUIWnd("SFRM_INGAME_GACHA")
	if(true == kMain:IsNil()) then return end
	
	local iAliveTime = 3000
	local kEffWnd = kMain:GetControl("FRM_EFFECT")
	if(true == kEffWnd:IsNil()) then return end
	kEffWnd:Visible(true)
	kEffWnd:RefreshCalledTime()
	kEffWnd:SetAliveTime(iAliveTime)
	
	local kIconWnd = kMain:GetControl("ICON")
	if(true == kIconWnd:IsNil()) then return end
	kIconWnd:Visible(true)	
	kIconWnd:RefreshCalledTime()
	kIconWnd:SetAliveTime(iAliveTime)

	local kQuestionWnd = kMain:GetControl("FRM_QUESTION")
	if(true == kQuestionWnd:IsNil()) then return end
	kQuestionWnd:Visible(true)
	g_kIGGachaEff["START"] = 2
	g_kIGGachaEff["IS_SEND_PACKET"] = false
end
-- InGameGacha [End]--------------------------------------------------------------------------

function Net_PT_M_C_NFY_HIDDEN_TIME_LIMIT(packet)
	g_iHiddenTotalTimeLimit = packet:PopInt()	
	g_fHiddenStartTimeLimit = GetCurrentTimeInSec()
	
	ActivateUI("FRM_HIDDEN_TIMELIMIT")
end

function Net_PT_M_C_NFY_HIDDEN_REWORDITEM(packet)
	if(GetLocale() ~= LOCALE.NC_DEVELOP) then return end -- 미개방 컨텐츠 #NOT_OPEN#
	
	local iStoneItemNo = packet:PopInt()
	local iStoneCount = packet:PopInt()
	local iItemCount = packet:PopInt()
	local iItemNo = packet:PopInt()
	
	ActivateUI("FRM_HIDDEN_REWORDITEM")
	
	local TopWnd = GetUIWnd("FRM_HIDDEN_REWORDITEM")
	
	if TopWnd:IsNil() == false then
		local Text1 = TopWnd:GetControl("FRM_TEXT1")
		local Text2 = TopWnd:GetControl("FRM_TEXT2")
		local Icon = TopWnd:GetControl("ICN_ITEM")
		local ItemCountWnd = TopWnd:GetControl("FRM_COUNT")
		
		if Text1:IsNil() == true then
			return
		end
		
		if Text2:IsNil() == true then
			return
		end
		
		if Icon:IsNil() == true then
			return
		end			

		if ItemCountWnd:IsNil() == true then
			return
		end
		
		TopWnd:SetCustomDataAsInt(iStoneItemNo)
		Text1:SetCustomDataAsInt(iStoneCount)
		Text2:SetCustomDataAsInt(iItemCount)
		Icon:SetCustomDataAsInt(iItemNo)
		
		local kMsg = string.format(GetTextW(400994):GetStr(), tonumber(Text1:GetCustomDataAsInt()), tonumber(Text2:GetCustomDataAsInt()))
		ItemCountWnd:SetStaticText(kMsg)
	end	
end

function HiddenRewordItemCheck()
	if(GetLocale() ~= LOCALE.NC_DEVELOP) then return end -- 미개방 컨텐츠 #NOT_OPEN#
	local TopWnd = GetUIWnd("FRM_HIDDEN_REWORDITEM")
	
	if TopWnd:IsNil() == false then
		local Text1 = TopWnd:GetControl("FRM_TEXT1")
		local Text2 = TopWnd:GetControl("FRM_TEXT2")	
		local Icon = TopWnd:GetControl("ICN_ITEM")
		local ItemCountWnd = TopWnd:GetControl("FRM_COUNT")
		
		if Text1:IsNil() == true then
			return
		end
		
		if Text2:IsNil() == true then
			return
		end
		
		if Icon:IsNil() == true then
			return
		end		
		
		if ItemCountWnd:IsNil() == true then
			return
		end
		
		local iItemNo = TopWnd:GetCustomDataAsInt()		
		local iItemCount = HiddenRewordItemView(iItemNo)
		local iItemCount2 = HiddenRewordItemView2(iItemCount)
		local iViewItemNo = HiddenRewordItemViewIcon(iItemCount)
		
		Text1:SetCustomDataAsInt(iItemCount)
		Text2:SetCustomDataAsInt(iItemCount2)
		Icon:SetCustomDataAsInt(iViewItemNo)
			
		local kMsg = string.format(GetTextW(400994):GetStr(), tonumber(iItemCount), tonumber(iItemCount2))
		ItemCountWnd:SetStaticText(kMsg)
		
		--[[if iItemCount >= Text2:GetCustomDataAsInt() then
			local packet = NewPacket(156)--PT_C_M_REQ_HIDDEN_REWORDITEM
			Net_Send(packet)
			DeletePacket(packet)
		end--]]
	end
end

function GetHiddenRewordItem(iResultItemNo)
	if(GetLocale() ~= LOCALE.NC_DEVELOP) then return end -- 미개방 컨텐츠 #NOT_OPEN#
	
	ActivateUI("SFRM_HIDDEN_REWORDITEM_USE")
	local TopWnd = GetUIWnd("SFRM_HIDDEN_REWORDITEM_USE")
	
	if TopWnd:IsNil() == false then
		local Icon = TopWnd:GetControl("ICN_RESULT")
		
		if Icon:IsNil() == true then
			return
		end
		
		local ItemWnd = GetUIWnd("FRM_HIDDEN_REWORDITEM")
		
		if ItemWnd:IsNil() == true then
			return
		end
		
		local iItemNo = ItemWnd:GetCustomDataAsInt()	
		local iMyCount = HiddenRewordItemView(iItemNo)
		local iMinCount = HiddenRewordItemView2(1)
		if iMyCount < iMinCount then
			CommonMsgBox(GetTT(400998):GetStr(), true)
			CloseUI("SFRM_HIDDEN_REWORDITEM_USE")
			return
		end
		
		Icon:SetCustomDataAsInt(iResultItemNo)
	end	
end

function SendHiddenRewordItem()
	if(GetLocale() ~= LOCALE.NC_DEVELOP) then return end -- 미개방 컨텐츠 #NOT_OPEN#
	
	local TopWnd = GetUIWnd("SFRM_HIDDEN_REWORDITEM_USE")
	
	if TopWnd:IsNil() == false then
		local Icon = TopWnd:GetControl("ICN_RESULT")
		
		if Icon:IsNil() == true then
			return
		end
	end
	
	local packet = NewPacket(157)--PT_C_M_REQ_HIDDEN_GIVE_REWORDITEM
	Net_Send(packet)
	DeletePacket(packet)		
	
	CloseUI("SFRM_HIDDEN_REWORDITEM_USE")
end

function DisplayHiddenTimeLimit(wnd)
	if wnd:IsNil() == true then
		return
	end

	local fNow = GetCurrentTimeInSec()*1000
	
	local iHiddenTime
	
	iHiddenTime = g_iHiddenTotalTimeLimit - (g_iHiddenTimeLimit + fNow - g_fHiddenStartTimeLimit*1000)
	if iHiddenTime < 0 then
		iHiddenTime = 0
	end
	

	local iDelta = math.floor(iHiddenTime*0.1)
	local iSec100 = iDelta%10 + 1
	local iSec1000 = math.floor((iDelta%100)/10) + 1	
	iDelta = math.floor(iHiddenTime*0.001)
	local iSec1 = iDelta%10 + 1
	local iSec10 = math.floor((iDelta%60)/10) + 1
	local iMin1 = math.floor(iDelta/60)%10 + 1
	local iMin10 = math.floor(iDelta/600)%10 + 1	
	
	local wnd100sec = wnd:GetControl("FRM_SEC100")
	if wnd100sec:IsNil() == false then
		wnd100sec:SetUVIndex(iSec100)
	end

	local wnd1000sec = wnd:GetControl("FRM_SEC1000")
	if wnd1000sec:IsNil() == false then
		wnd1000sec:SetUVIndex(iSec1000)
	end
	
	local wnd1sec = wnd:GetControl("FRM_SEC1")
	if wnd1sec:IsNil() == false then
		wnd1sec:SetUVIndex(iSec1)
	end

	local wnd10sec = wnd:GetControl("FRM_SEC10")
	if wnd10sec:IsNil() == false then
		wnd10sec:SetUVIndex(iSec10)
	end

	local wnd1min = wnd:GetControl("FRM_MIN1")
	if wnd1min:IsNil() == false then
		wnd1min:SetUVIndex(iMin1)
	end

	local wnd10min = wnd:GetControl("FRM_MIN10")
	if wnd10min:IsNil() == false then
		wnd10min:SetUVIndex(iMin10)
	end

	local kPathDefault = "../Data/6_ui/mission/msNumYs.tga"
	local kPathsDefault = "../Data/6_ui/mission/msNumTmYs2.tga"	
	local kPath = "../Data/6_ui/mission/msNumYs.tga"
	local kPaths = "../Data/6_ui/mission/msNumTmYs2.tga"
	
	
	if iDelta <= 180 then	
		wnd100sec:ChangeImage(kPaths)
		wnd100sec:SetInvalidate(true)
		wnd1000sec:ChangeImage(kPaths)
		wnd1000sec:SetInvalidate(true)
		wnd10sec:ChangeImage(kPath)
		wnd10sec:SetInvalidate(true)
		wnd1sec:ChangeImage(kPath)
		wnd1sec:SetInvalidate(true)
		wnd1min:ChangeImage(kPath)
		wnd1min:SetInvalidate(true)
		wnd10min:ChangeImage(kPath)
		wnd10min:SetInvalidate(true)
	else
		wnd100sec:ChangeImage(kPathsDefault)
		wnd100sec:SetInvalidate(true)
		wnd1000sec:ChangeImage(kPathsDefault)
		wnd1000sec:SetInvalidate(true)
		wnd10sec:ChangeImage(kPathDefault)
		wnd10sec:SetInvalidate(true)
		wnd1sec:ChangeImage(kPathDefault)
		wnd1sec:SetInvalidate(true)
		wnd1min:ChangeImage(kPathDefault)
		wnd1min:SetInvalidate(true)
		wnd10min:ChangeImage(kPathDefault)
		wnd10min:SetInvalidate(true)	
	end
	
end

function SortInv(InvType, wnd)
	if nil==wnd or wnd:IsNil() then return end
	
	local iSendInvType = 0
	if 1 == InvType then
		-- ID="Inv" : 1
		local wndParent = wnd:GetParent():GetParent()
		if wndParent:IsNil() then return end
		
		for i=1, table.getn(g_InvGrp) do
			local wndBG = wndParent:GetControl(g_InvGrp[i]["BG"])
			if false == wndBG:IsNil() then
				local wndBtn = wndBG:GetControl(g_InvGrp[i]["BTN"])
				if false == wndBtn:IsNil() then
					if true == wndBtn:GetCheckState() then
						iSendInvType = i
						break
					end
				end
			end
		end
	elseif 2==InvType or 3==InvType then
		-- ID="SFRM_INV_SAFE" : 2
		-- ID="SFRM_SHARE_SAFE" : 3
		local wndParent = wnd:GetParent():GetParent():GetParent()
		local Builder = wndParent:GetControl("BLD_TITLE_BTN")
		if Builder:IsNil() then return end
		
		local BuildCountPt = Builder:GetBuildCount()
		if nil == BuildCountPt then return end
		local cnt = BuildCountPt:GetX()
		for i=0,cnt do
			local wndBtn = wndParent:GetControl("CBTN_BAG"..i)
			if false == wndBtn:IsNil() then
				if true == wndBtn:GetCheckState() then
					if 2==InvType then
						if 0==i then
							iSendInvType = 5
						else
							iSendInvType = 11+i
						end
					elseif 3==InvType then
						iSendInvType = 16 + i
					end
					break
				end
			end
		end
	end
	
	if 0 < iSendInvType then
		ODS("[SortInv Send]".. iSendInvType .."\n")
		local kPacket = NewPacket(17706);
		kPacket:PushInt(iSendInvType);
		Net_Send(kPacket)
		DeletePacket(kPacket)
	end
end

function FRM_Arkah()
	CallUI("FRM_Arkah")
end

function FRM_Pythanous()
	CallUI("FRM_Pythanous")
end

function FRM_Karkharous()
	CallUI("FRM_Karkharous")
end

function FRM_PaulBurnaun()
	CallUI("FRM_PaulBurnaun")
end

function FRM_Elluman()
	CallUI("FRM_Elluman")
end

function FRM_Vella()
	CallUI("FRM_Vella")
end

function FRM_Artis()
	CallUI("FRM_Artis")
end

function FRM_Rokko()
	CallUI("FRM_Rokko")
end

function FRM_LordofNight()
	CallUI("FRM_LordofNight")
end

function FRM_SpectersTower()
	CallUI("FRM_SpectersTower")
end

g_bAwakeChargeState = false
function SetAwakeChargeState(bChangeUI)
	g_bAwakeChargeState = bChangeUI	-- 0: 노멀, 1: 챠지 스킬 사용중
end

function GetAwakeChargeState()
	return g_bAwakeChargeState
end

function OnTick_AwakeGaugeText(kSelf)	-- 각성 게이지 숫자값 업데이트
	if(nil == kSelf) then return end 
	if(kSelf:IsNil()) then return end 
	local kMyActor = GetMyActor()
	if(kMyActor:IsNil()) then return end	
	local iAwakeVal = kMyActor:GetAbil(AT_AWAKE_VALUE)
	local Str = iAwakeVal.." / "..AWAKE_VALUE_MAX
	kSelf:SetStaticText(Str)
end


---인벤 개선
g_bCheckNewInv = false
function OnTick_CheckNewInvItem()
	if false == g_bCheckNewInv then
		g_bCheckNewInv = CallNewInvItem(g_InvNiewGroup)
		if true == g_bCheckNewInv then
			for i=1, table.getn(g_InvGrp) do
				if i ~= g_InvNiewGroup then
					CallNewInvItem_Tab(i)
				end
			end
		end
	end
end

g_ElitePattenState = 0
g_FrenzyBgAni = 0
g_GroggyTextAni = 0
g_kGroggyMonsterGuid = nil
g_fGroggyRecoveryInterval = 0.0
g_fGroggyElapsedTime = 0.0

function GetEliteMonActor()
	if nil~=g_kGroggyMonsterGuid and false==g_kGroggyMonsterGuid:IsNil() then
		local kPilot = g_pilotMan:FindPilot(g_kGroggyMonsterGuid)
		if false==kPilot:IsNil() then
			return kPilot:GetActor()
		end
	end
end

function DieElitePattenMonster(kGuid)
	if nil==kGuid or kGuid:IsNil() then
		return
	end

	if nil==g_kGroggyMonsterGuid or g_kGroggyMonsterGuid:IsNil() then
		return
	end

	if kGuid:IsEqual(g_kGroggyMonsterGuid) then
		SetElitePattenStateUI(GetUIWnd("FRM_MAIN_GAUGEBAR"), false)
	end
end

function SetElitePattenStateUI(kWnd, bShow)
	if nil==kWnd or kWnd:IsNil() then
		return
	end

	kWnd:GetControl("IMG_GROGGY_BG"):Visible(bShow)
	kWnd:GetControl("BAR_GROGGY"):Visible(bShow)
	kWnd:GetControl("IMG_FRENZY_GROGGY_TEXT"):Visible(bShow)
	kWnd:GetControl("FRM_GROGGY_TEXT"):Visible(bShow)
	kWnd:GetControl("FRM_FRENZY_BG"):Visible(bShow)
	kWnd:GetControl("IMG_FRENZY_TICK"):Visible(bShow)
	kWnd:GetControl("BAR_FRENZY"):Visible(bShow)
end

function SetElitePattenState(kGuid, iState, iMaxGroggy)
	g_kGroggyMonsterGuid = kGuid
	g_fGroggyElapsedTime = GetAccumTime()
	
	local kActor = GetEliteMonActor()
	if nil~=kActor and false==kActor:IsNil() then
		g_fGroggyRecoveryInterval = kActor:GetAbil(7713) / 1000.0		--AT_C_GROGGY_RECOVERY_INTERVAL	= 7713
	end

	local kWnd = GetUIWnd("FRM_MAIN_GAUGEBAR")
	if kWnd:IsNil() then
		return
	end

	g_ElitePattenState = iState
	if 2==iState then --EPS_FRENZY
		SetElitePattenStateUI(kWnd, true)
		kWnd:GetControl("FRM_GROGGY_TEXT"):Visible(false)
		kWnd:GetControl("BAR_FRENZY"):SetCustomDataAsFloat(GetAccumTime())
	elseif 3==iState then --EPS_GROGGY
		SetElitePattenStateUI(kWnd, true)
		kWnd:GetControl("IMG_FRENZY_GROGGY_TEXT"):Visible(false)
		kWnd:GetControl("FRM_FRENZY_BG"):Visible(false)
		kWnd:GetControl("IMG_FRENZY_TICK"):Visible(false)
		kWnd:GetControl("BAR_FRENZY"):Visible(false)
	else
		SetElitePattenStateUI(kWnd, false)
	end
	
	SetGroggyGaugebarStateUI(iState,0)
	kWnd:GetControl("BAR_GROGGY"):BarMax(iMaxGroggy)
end

g_fFrenzyElapsedTime = 0
g_type = 0
function UpdateElitePattenState(kSelf)
	if 2==g_ElitePattenState then --EPS_FRENZY
		local kWnd = kSelf:GetControl("BAR_FRENZY")
		local iVisible = true
		g_fFrenzyElapsedTime = g_fFrenzyElapsedTime + GetFrameTime()*6
		if g_fFrenzyElapsedTime > kWnd:GetMaxAlpha() then
			g_fFrenzyElapsedTime = 0
			g_type = not g_type
		end

		if g_type then
			kWnd:SetAlpha(kWnd:GetMaxAlpha()-g_fFrenzyElapsedTime)
		else
			kWnd:SetAlpha(g_fFrenzyElapsedTime)
		end
			
	end
end

function GetEliteMonGuid()
	return g_kGroggyMonsterGuid
end

function SetGroggyGaugebarStateUI(iState, iRate)
	local iLv = GetGroggyRelaxRateLv(iRate)
	local iUVIndex = 1
	if 3==iState then --EPS_GROGGY
		iUVIndex = 5
	else
		if iLv >= 6 then
			iUVIndex = 4
		elseif iLv >= 4 then
			iUVIndex = 3
		elseif iLv >= 1 then
			iUVIndex = 2
		end
	end
	
	local kWnd = GetUIWnd("FRM_MAIN_GAUGEBAR")
	if false==kWnd:IsNil() then
		kWnd:GetControl("BAR_GROGGY"):SetUVIndex(iUVIndex)
	end
end

function GetGroggyRecoveryValue()
	local kActor = GetEliteMonActor()
	if nil~=kActor and false==kActor:IsNil() then
		return kActor:GetGroggyRecoveryValue()
	end
	return 0
end

g_OldGroggyPoint = 0
function CheckChangeGroggyNowPoint(kSelf)
	if nil==kSelf or kSelf:IsNil() then
		return
	end

	local kActor = GetEliteMonActor()
	if nil~=kActor and false==kActor:IsNil() then
		local iNow = kActor:GetAbil(6331)		-- AT_GROGGY_NOW = 6331
		if g_OldGroggyPoint ~= iNow then
			kSelf:BarNow(iNow)
			--kSelf:SetStaticText(iNow.."/"..kSelf:GetBarMax())
			g_OldGroggyPoint = iNow
		end
	end
end

function UpdateGroggyNow(kSelf)
	CheckChangeGroggyNowPoint(kSelf)

	local fNowAccumTime = GetAccumTime()
	local fRate = 0
	if  g_fGroggyRecoveryInterval then
		fRate = (fNowAccumTime-g_fGroggyElapsedTime) / g_fGroggyRecoveryInterval
	end

	g_fGroggyElapsedTime = fNowAccumTime
	
	if nil~=kSelf and false==kSelf:IsNil() then
		local iNow = kSelf:GetBarNow() + math.floor(GetGroggyRecoveryValue() * fRate)
		if 0 > iNow then
			iNow = 0
		end

		kSelf:BarNow(iNow)
		--kSelf:SetStaticText(iNow.."/"..kSelf:GetBarMax())		
	end
end

function SetElitePattenFrenzyTime(iTime)	
	local kWnd = GetUIWnd("FRM_MAIN_GAUGEBAR")
	if kWnd:IsNil() then
		return
	end
	
	local kBar = kWnd:GetControl("BAR_FRENZY")
	if kBar:IsNil() then
		return
	end

	kBar:BarNow(iTime)
	kBar:BarMax(iTime)
end

function UpdateElitePattenFrenzyTime(kSelf)
	if nil==kSelf or kSelf:IsNil() then
		return
	end
	
	local iMaxValue = kSelf:GetBarMax()
	local iPassTime = math.floor((GetAccumTime()-kSelf:GetCustomDataAsFloat())*1000)
	local iNow = iMaxValue-iPassTime
	if iNow>=0 then
		kSelf:BarNow(iNow)
	else
		kSelf:GetParent():GetControl("FRM_FRENZY_BG"):Visible(bShow)
	end
end

function RoadSignInit(kSelf)
	g_RoadSign["AniScrTime"] = GetAccumTime()
	local sz = Point2(115, 105)
	kSelf:SetSize(sz)
	local wndCenter = kSelf:GetControl("IMG_CENTER")
	wndCenter:SetStaticText("")
	sz:SetX(15)
	wndCenter:SetSize(sz)
	wndCenter:SetImgSize(sz)
	wndCenter:SetTextPos(Point2(0, 33))
	kSelf:GetControl("IMG_RIGHT"):SetLocation(Point2(65, 0)) 
	kSelf:SetLocation(Point2(455+(kSelf:GetResolutionGap():GetX()/2),50))
end

function SetSocketNpc(kOwnerGuid)
	if true == kOwnerGuid:IsNil() then
		return
	end
	local kWnd = ActivateUI("SFRM_SOCKET_MAKE")
	if true == kWnd:IsNil() then
		return
	end
	kWnd:SetCustomDataAsGuid(kOwnerGuid)
	RegistUIAction(kWnd, "CloseUI")
end

function CallSocketMenuUI()
	local kWnd = ActivateUI("SFRM_SOCKET_MAKE")
	if true == kWnd:IsNil() then
		return
	end
	RegistUIAction(kWnd, "CloseUI")
end

function UICallBossAttrUI(bShow, bossBar, bossno)
	if false==bShow then
		CloseUI("FRM_BOSS_ATTR_INFO")
		return
	end

	local AttrWnd = ActivateUI("FRM_BOSS_ATTR_INFO")

	if nil==AttrWnd or AttrWnd:IsNil() then
		return
	end

	local kPlayer = g_pilotMan:GetPlayerUnit()
	if kPlayer:IsNil() then
		return
	end

	local bossactor = g_pilotMan:FindActorByClassNo(bossno)
	if bossactor:IsNil() then
		return
	end
	local pilot = bossactor:GetPilot()
	if pilot:IsNil() then
		return
	end
	local iCount = -1
	for i=0,4 do
		local ctrl = AttrWnd:GetControl("FRM_BG"..i)
		if false==ctrl:IsNil() then
			ctrl:Visible(false)
		end
	end
	local ele4 = pilot:GetAbil(AT_RESIST_ADD_4ELEMENT)
	local ele5 = pilot:GetAbil(AT_RESIST_ADD_5ELEMENT)
	local eleRate4 = pilot:GetAbil(AT_RESIST_RATE_ADD_4ELEMENT)
	local eleRate5 = pilot:GetAbil(AT_RESIST_RATE_ADD_5ELEMENT)

	for i=0,4 do		
		local iBossAbil = pilot:GetAbil(AT_C_RESIST_ADD_FIRE + i)
		local iBossAbilAdd = pilot:GetAbil(AT_RESIST_RATE_ADD_FIRE + i)
		if 0~=iBossAbil or 0~=ele4 or 0~=ele5 or 0~=eleRate4 or 0~=eleRate5 or 0~=iBossAbilAdd then
			iCount = iCount + 1
			local ctrl = AttrWnd:GetControl("FRM_BG"..iCount)
			if false==ctrl:IsNil() then
				local icon = ctrl:GetControl("FRM_ICON")
				if false==icon:IsNil() then
					icon:SetUVIndex(i+1)
				end
				ctrl:Visible(true)
				local result = GetTotalElementalResistRate(kPlayer, pilot:GetUnit(), AT_C_RESIST_ADD_FIRE + i)
				result = result*100
				if 0 > result then
					result = math.floor(result) --음수의 경우 소숫점 이하를 강제로 버리는 듯?
				end
				result = (-result%1) + result
				ctrl:SetStaticText(tostring(result).."%")
				UI_T_Y_ToCenter(ctrl)
			end
		end
	end
	if -1<iCount then	--하나라도 켜졌으면 키자
		if false==bossBar:IsNil() then
			local bossloc = bossBar:GetLocation()
			bossloc:SetY(bossloc:GetY() + bossBar:GetSize():GetY())
			AttrWnd:SetLocation(bossloc)
		end
	end
end

function CallElga02_Groggy()
	local kWnd =  GetUIWnd("FRM_MAIN_GAUGEBAR")
	if nil==kWnd or kWnd:IsNil() then
		return
	end
	
	local kTargetGuid = GetReqAddUnitGuid("elga_2")
	if nil==kTargetGuid or kTargetGuid:IsNil() then
		return
	end

	local kTargetPilot = g_pilotMan:FindPilot(kTargetGuid)
	if nil == kTargetPilot then
		return
	end

	local kTarget = kTargetPilot:GetUnit()
	if nil == kTarget then
		return
	end
	CallUI("FRM_WARNING2")
	local kEffect = kTarget:FindEffect( 3542320 );	--TB_DefEffect_Mon 엘가 날개짓 데미지 누적
	if kEffect:IsNil() then
		kEffect = kTarget:FindEffect( 3542321 );
	end
	if false == kEffect:IsNil() then
		local bShow = true
		kWnd:GetControl("IMG_GROGGY_BG"):Visible(bShow)
		kWnd:GetControl("BAR_ELGA02_GROGGY"):Visible(bShow)
		kWnd:GetControl("BAR_ELGA02_DOWN_GROGGY"):Visible(not bShow)
		kWnd:GetControl("IMG_FRENZY_GROGGY_TEXT"):Visible(bShow)
		--kWnd:GetControl("FRM_GROGGY_TEXT"):Visible(bShow)
		-- kWnd:GetControl("FRM_FRENZY_BG"):Visible(bShow)
		kWnd:GetControl("IMG_FRENZY_TICK"):Visible(bShow)
		-- kWnd:GetControl("BAR_FRENZY"):Visible(bShow)
		
		kWnd:GetControl("BAR_ELGA02_GROGGY"):BarMax( kEffect:GetAbil(AT_ACCUMULATE_DAMAGED_MAX) )
	end	
end

function UpdateElga02_GroggyNow(kSelf)
	if nil==kSelf then
		return
	end
	
	local kTargetGuid = GetReqAddUnitGuid("elga_2")
	if nil==kTargetGuid or kTargetGuid:IsNil() then
		return
	end

	local kTargetPilot = g_pilotMan:FindPilot(kTargetGuid)
	if nil == kTargetPilot then
		return
	end

	kSelf:BarNow( kTargetPilot:GetAbil(AT_ACCUMULATE_DAMAGED_NOW) )
end

function CloseElga02_Groggy()
	local kWnd =  GetUIWnd("FRM_MAIN_GAUGEBAR")
	if nil==kWnd or kWnd:IsNil() then
		return
	end

	local bShow = false
	kWnd:GetControl("IMG_GROGGY_BG"):Visible(bShow)
	kWnd:GetControl("BAR_ELGA02_GROGGY"):Visible(bShow)
	kWnd:GetControl("BAR_ELGA02_DOWN_GROGGY"):Visible(bShow)
	kWnd:GetControl("IMG_FRENZY_GROGGY_TEXT"):Visible(bShow)
	--kWnd:GetControl("FRM_GROGGY_TEXT"):Visible(bShow)
	-- kWnd:GetControl("FRM_FRENZY_BG"):Visible(bShow)
	kWnd:GetControl("IMG_FRENZY_TICK"):Visible(bShow)
	-- kWnd:GetControl("BAR_FRENZY"):Visible(bShow)
		
end

g_Elga02_DownGroggyTime = 0
function CallElga02_DownGroggy()
	local kWnd =  GetUIWnd("FRM_MAIN_GAUGEBAR")
	if nil==kWnd or kWnd:IsNil() then
		return
	end

	local bShow = true
	kWnd:GetControl("IMG_GROGGY_BG"):Visible(bShow)
	local kGroggyWnd = kWnd:GetControl("BAR_ELGA02_DOWN_GROGGY")
	kGroggyWnd:Visible(bShow)
	kGroggyWnd:BarNow(12000)
	kGroggyWnd:BarMax(12000)
	if( CheckNil(g_world == nil) ) then return false end
	if( CheckNil(g_world:IsNil()) ) then return false end
	g_Elga02_DownGroggyTime = g_world:GetAccumTime()
end

function UpdateElga02_DownGroggy(kSelf)
	if( CheckNil(g_world == nil) ) then return false end
	if( CheckNil(g_world:IsNil()) ) then return false end
	local kTick = g_world:GetAccumTime()-g_Elga02_DownGroggyTime
	if kSelf:GetTickInterval()/1000.0 > kTick then
		return
	end
	
	g_Elga02_DownGroggyTime = g_world:GetAccumTime()
	if kSelf:IsVisible() then
		local now = kSelf:GetBarNow() - (kTick*1000.0)
		if 0 > now then
			local bShow = false
			kSelf:GetParent():GetControl("IMG_GROGGY_BG"):Visible(bShow)
			kSelf:Visible(bShow)
			return
		end
		kSelf:BarNow(now)
	end
end