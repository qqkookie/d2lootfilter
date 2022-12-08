#include <queue>
#include <sstream>
#include "ItemFilter.h"
#include "Configuration.h"
#include "Hooking.h"
#include "D2Ptrs.h"
#include "Globals.h"


namespace ItemFilter {

void ReloadFilter();
void RunRules(Unit* pItem);
void DoChatAlert(Unit* pUnit);

namespace Config = Configuration;

std::unordered_map<uint32_t, ActionResult*> ITEM_ACTIONS;

void ReloadFilter()
{
    // Make sure game session started
    _ASSERT(IsTxtDataLoaded);
    if (!IsTxtDataLoaded)
	return;

    Config::LoadRules();

    INFO_LOG(L"Item Filter Loaded (PingLevel: {}, FilterLevel: {})", PingLevel, FilterLevel);

    ITEM_ACTIONS.clear();
    for (uint8_t i = 0; i < 128; i++) {
	Unit* pUnit = FindUnitFromTable(i, UnitType::ITEM);
	while (pUnit) {
	    RunRules(pUnit);
	    DoChatAlert(pUnit);
	    pUnit = pUnit->pRoomNext;
	}
    }
}

void RunRules(Unit* pItem)
{
    ActionResult *result = ITEM_ACTIONS[pItem->dwUnitId];
    if (result)
	delete result;
    result = new ActionResult();
    ITEM_ACTIONS[pItem->dwUnitId] = result;

    EvaluteTokenValues(result, pItem);
    for (Rule* rule : GlobalRules) {
	if (rule->Evaluate(pItem)) {
	    result->vMatchedRules.push_back(rule->GetLineNumber());
	    rule->EvaluateActionResult(result, pItem);

	    if (IsFilterDebug && result->bHide)
		result->wsItemName = std::format( L"{}{}{}*",
		    TEXT_GRAY, result->wsItemName, TEXT_GRAY);

	    if (!result->bCheck) 
		return;
	}
    }
}


// ****************************************************************************

void Setup();
void RestoreHook();
void ToggleDebug();
void DebugRule(uint32_t nLineNumber);
void Clip();
bool HasActions(Unit* pUnit);
POINT ScreenToAutomap(int nX, int nY);

#pragma region Hooks

//Hooked Methods

typedef void(__fastcall* D2GSServerToClientPacketHandlerFn)(uint8_t* pBitstream);

LRESULT WndProc(HWND hWnd, int msg, WPARAM wParam, LPARAM lParam);

void __stdcall DATATBLS_LoadAllTxts(void* pMemPool, int a2, int a3);
void __stdcall DrawGameUI();
void HandlePacket(uint8_t* pBitstream, D2GSServerToClientPacketHandlerFn pHandler);

BOOL __fastcall GetItemName_114d(Unit* pItem, wchar_t* pBuffer, uint32_t dwSize);
BOOL __stdcall GetItemName_113c(Unit* pItem, wchar_t* pBuffer, uint32_t dwSize);
void __stdcall HandleItemName(Unit* pItem, wchar_t* pBuffer, uint32_t dwSize);
void __stdcall GetItemDesc(wchar_t* pBuffer);
void __fastcall ItemActionOwned(uint8_t* pBitstream);
void __fastcall ItemActionWorld(uint8_t* pBitstream);

void __stdcall AUTOMAP_Draw();
BOOL __fastcall UNITDRAW_DrawUnit(Unit* pUnit, uint32_t dwColorTint, int nXpos, int nYpos, BOOL bFade, BOOL bDrawOverlays);
BOOL __fastcall IsUnitNoDraw(Unit* pUnit);
void __stdcall UNITS_FreeUnit(Unit* pUnit);

void __stdcall DrawGroundItemRect(DWORD retAddress, BOOL isHovered, Unit* pItem, uint32_t nXStart, uint32_t nYStart, uint32_t nXEnd, uint32_t nYEnd, uint8_t nPaletteIndex, DrawMode eDrawMode);
void __stdcall DrawInventoryItemRect(Unit* pItem, uint32_t nXStart, uint32_t nYStart, uint32_t nXEnd, uint32_t nYEnd, uint8_t nPaletteIndex, DrawMode eDrawMode);

BOOL __stdcall CallCommand(char* wCmd);

void __stdcall DrawDebugInfo(Unit* pItem, uint32_t nXStart, uint32_t nYStart, uint32_t nXEnd, uint32_t nYEnd);

#pragma endregion

#pragma region Stubs

//Stubs

void __stdcall DrawAltDownItemRect_STUB_110f();
void __stdcall DrawAltDownItemRect_STUB();
void __stdcall DrawAltDownItemRect_STUB_114d();

void __stdcall DrawHoverItemRect_STUB();

void __stdcall DrawInventoryItemRect_STUB_110f();
void __stdcall DrawInventoryItemRect_STUB();
void __stdcall DrawInventoryItemRect_STUB_114d();

void __stdcall CheckUnitNoDraw1_STUB();
void __stdcall CheckUnitNoDraw2_STUB();
void __stdcall CheckUnitNoDraw2_STUB_114d();

void __stdcall GetItemDesc_STUB();
void __stdcall GetItemDesc_STUB_114d();

void __stdcall CallCommand_STUB();
void __stdcall CallCommand_STUB_114d();

#pragma endregion

D2CLIENT_ItemActionWorld_t fpItemActionWorld;
D2CLIENT_ItemActionOwned_t fpItemActionOwned;
D2CLIENT_GetItemName_113c_t fpD2CLIENT_GetItemName_113c;
D2CLIENT_GetItemName_114d_t fpD2CLIENT_GetItemName_114d;
D2CLIENT_UNITDRAW_DrawUnit_t fpUNITDRAW_DrawUnit;
D2CLIENT_AUTOMAP_Draw_t fpAUTOMAP_Draw;
D2COMMON_UNITS_FreeUnit_t fpUNITS_FreeUnit;
D2COMMON_DATATBLS_LoadAllTxts_t fpDATATBLS_LoadAllTxts;
D2CLIENT_DrawGameUI_t fpD2CLIENT_DrawGameUI;
D2WIN_WndProc_t fpWndProc;
BOOL(__stdcall* fpCallCommand)(char* sCmd);

void Setup() {

    //check if plugy already installed a command hook
    //if so we do a trampoline, if not we isntall our own stub.
    bool IsPlugyCommandHook = (*(BYTE*)D2CLIENT_callCommand) == 0xE8;

    //alot of 114d functions need diff stubs or they changed from __stdcall to __fastcall
    if (GameVersion() == D2Version::V114d) {
	//Item Action Own/World Packet Hooks
	Hooking::TrampolineHook(D2CLIENT_ItemActionWorld, &ItemActionWorld, reinterpret_cast<void**>(&fpItemActionWorld), 9);
	Hooking::TrampolineHook(D2CLIENT_ItemActionOwned, &ItemActionOwned, reinterpret_cast<void**>(&fpItemActionOwned), 9);

	//Item Name Hook
	Hooking::TrampolineHook(D2CLIENT_GetItemName_114d, &GetItemName_114d, reinterpret_cast<void**>(&fpD2CLIENT_GetItemName_114d), 8);

	//Item Desc Hook
	Hooking::SetJmp(D2CLIENT_fpGetItemDescPatch, &GetItemDesc_STUB_114d, 6);

	//Automap Draw Hook
	Hooking::TrampolineHook(D2CLIENT_AUTOMAP_Draw, &AUTOMAP_Draw, reinterpret_cast<void**>(&fpAUTOMAP_Draw), 9);

	//Unit Draw Hook
	Hooking::TrampolineHook(D2CLIENT_UNITDRAW_DrawUnit, &UNITDRAW_DrawUnit, reinterpret_cast<void**>(&fpUNITDRAW_DrawUnit), 6);

	//No draw hooks
	Hooking::SetCall(D2CLIENT_checkUnitNoDrawPatch_1, &CheckUnitNoDraw1_STUB, 9);
	Hooking::SetCall(D2CLIENT_checkUnitNoDrawPatch_2, &CheckUnitNoDraw2_STUB_114d, 9);

	//Call Command Hook
	Hooking::SetCall(D2CLIENT_callCommand, &CallCommand_STUB_114d, 5);

	//Item Rect on Ground Hooks
	Hooking::SetCall(D2WIN_callDrawAltDownItemRectPatch, &DrawAltDownItemRect_STUB_114d, 5);
	Hooking::SetCall(D2WIN_callDrawHoverItemRectPatch, &DrawHoverItemRect_STUB, 5);

	//Item Rect in Inv Hook
	Hooking::SetCall(D2CLIENT_callDrawInventoryItemRectPatch, &DrawInventoryItemRect_STUB_114d, 5);

	//Cleanup Cached Item Data Hook
	Hooking::TrampolineHook(D2COMMON_UNITS_FreeUnit, &UNITS_FreeUnit, reinterpret_cast<void**>(&fpUNITS_FreeUnit), 7);

	//Load Datatables Hook
	Hooking::TrampolineHook(D2COMMON_DATATBLS_LoadAllTxts, &DATATBLS_LoadAllTxts, reinterpret_cast<void**>(&fpDATATBLS_LoadAllTxts), 5);
	Hooking::TrampolineHook(D2CLIENT_DrawGameUI, &DrawGameUI, reinterpret_cast<void**>(&fpD2CLIENT_DrawGameUI), 6);
    }
    else if (GameVersion() == D2Version::V113c) {
	//Item Action Own/World Packet Hooks
	Hooking::TrampolineHook(D2CLIENT_ItemActionWorld, &ItemActionWorld, reinterpret_cast<void**>(&fpItemActionWorld), 6);
	Hooking::TrampolineHook(D2CLIENT_ItemActionOwned, &ItemActionOwned, reinterpret_cast<void**>(&fpItemActionOwned), 6);

	//Item Name Hook
	Hooking::TrampolineHook(D2CLIENT_GetItemName_113c, &GetItemName_113c, reinterpret_cast<void**>(&fpD2CLIENT_GetItemName_113c), 5);

	//Item Desc Hook
	Hooking::SetJmp(D2CLIENT_fpGetItemDescPatch, &GetItemDesc_STUB, 6);

	//Automap Draw Hook
	Hooking::TrampolineHook(D2CLIENT_AUTOMAP_Draw, &AUTOMAP_Draw, reinterpret_cast<void**>(&fpAUTOMAP_Draw), 6);

	//Unit Draw Hook
	Hooking::TrampolineHook(D2CLIENT_UNITDRAW_DrawUnit, &UNITDRAW_DrawUnit, reinterpret_cast<void**>(&fpUNITDRAW_DrawUnit), 5);

	//No draw hooks
	Hooking::SetCall(D2CLIENT_checkUnitNoDrawPatch_1, &CheckUnitNoDraw1_STUB, 9);
	Hooking::SetCall(D2CLIENT_checkUnitNoDrawPatch_2, &CheckUnitNoDraw2_STUB, 9);

	//Call Command Hook
	Hooking::SetCall(D2CLIENT_callCommand, &CallCommand_STUB, 5);

	//Item Rect on Ground Hooks
	Hooking::SetCall(D2WIN_callDrawAltDownItemRectPatch, &DrawAltDownItemRect_STUB, 5);
	Hooking::SetCall(D2WIN_callDrawHoverItemRectPatch, &DrawHoverItemRect_STUB, 5);

	//Item Rect in Inv Hook
	Hooking::SetCall(D2CLIENT_callDrawInventoryItemRectPatch, &DrawInventoryItemRect_STUB, 5);

	//Cleanup Cached Item Data Hook
	Hooking::TrampolineHook(D2COMMON_UNITS_FreeUnit, &UNITS_FreeUnit, reinterpret_cast<void**>(&fpUNITS_FreeUnit), 5);

	//Load Datatables Hook
	Hooking::TrampolineHook(D2COMMON_DATATBLS_LoadAllTxts, &DATATBLS_LoadAllTxts, reinterpret_cast<void**>(&fpDATATBLS_LoadAllTxts), 6);
	Hooking::TrampolineHook(D2CLIENT_DrawGameUI, &DrawGameUI, reinterpret_cast<void**>(&fpD2CLIENT_DrawGameUI), 5);
    }
    else if (GameVersion() == D2Version::V110f) {
	//Item Action Own/World Packet Hooks
	Hooking::TrampolineHook(D2CLIENT_ItemActionWorld, &ItemActionWorld, reinterpret_cast<void**>(&fpItemActionWorld), 6);
	Hooking::TrampolineHook(D2CLIENT_ItemActionOwned, &ItemActionOwned, reinterpret_cast<void**>(&fpItemActionOwned), 6);

	//Item Name Hook
	Hooking::TrampolineHook(D2CLIENT_GetItemName_114d, &GetItemName_114d, reinterpret_cast<void**>(&fpD2CLIENT_GetItemName_114d), 5);

	//Item Desc Hook
	Hooking::SetJmp(D2CLIENT_fpGetItemDescPatch, &GetItemDesc_STUB, 6);

	//Automap Draw Hook
	Hooking::TrampolineHook(D2CLIENT_AUTOMAP_Draw, &AUTOMAP_Draw, reinterpret_cast<void**>(&fpAUTOMAP_Draw), 6);

	//Unit Draw Hook
	Hooking::TrampolineHook(D2CLIENT_UNITDRAW_DrawUnit, &UNITDRAW_DrawUnit, reinterpret_cast<void**>(&fpUNITDRAW_DrawUnit), 6);

	//No draw hooks
	Hooking::SetCall(D2CLIENT_checkUnitNoDrawPatch_1, &CheckUnitNoDraw1_STUB, 9);
	Hooking::SetCall(D2CLIENT_checkUnitNoDrawPatch_2, &CheckUnitNoDraw2_STUB, 9);

	//Call Command Hook
	Hooking::SetCall(D2CLIENT_callCommand, &CallCommand_STUB, 5);

	//Item Rect on Ground Hooks
	Hooking::SetCall(D2WIN_callDrawAltDownItemRectPatch, &DrawAltDownItemRect_STUB_110f, 5);
	Hooking::SetCall(D2WIN_callDrawHoverItemRectPatch, &DrawHoverItemRect_STUB, 5);

	//Item Rect in Inv Hook
	Hooking::SetCall(D2CLIENT_callDrawInventoryItemRectPatch, &DrawInventoryItemRect_STUB_110f, 5);

	//Cleanup Cached Item Data Hook
	Hooking::TrampolineHook(D2COMMON_UNITS_FreeUnit, &UNITS_FreeUnit, reinterpret_cast<void**>(&fpUNITS_FreeUnit), 5);

	//Load Datatables Hook
	Hooking::TrampolineHook(D2COMMON_DATATBLS_LoadAllTxts, &DATATBLS_LoadAllTxts, reinterpret_cast<void**>(&fpDATATBLS_LoadAllTxts), 6);
	Hooking::TrampolineHook(D2CLIENT_DrawGameUI, &DrawGameUI, reinterpret_cast<void**>(&fpD2CLIENT_DrawGameUI), 8);
    }
    //User Input/WndProc Hook
    Hooking::TrampolineHook(D2WIN_WndProc, &WndProc, reinterpret_cast<void**>(&fpWndProc), 5);
}

void RestoreHook() {
    // Restore WndProc() to saved original
    Hooking::TrampolineHook(D2WIN_WndProc, fpWndProc, NULL, 5);
}

void DoChatAlert(Unit* pUnit) 
{
    if (HasActions(pUnit)
	&& ITEM_ACTIONS[pUnit->dwUnitId]->bChatAlert
	&& (pUnit->eItemAnimMode == ItemAnimationMode::DROPPING
	    || pUnit->eItemAnimMode == ItemAnimationMode::GROUND)) {
	wchar_t buffer[0x100] = L"";
	if (GameVersion() == D2Version::V114d || GameVersion() == D2Version::V110f) {
	    D2CLIENT_GetItemName_114d(pUnit, buffer, 0x100);
	}
	else {
	    D2CLIENT_GetItemName_113c(pUnit, buffer, 0x100);
	}
	std::wstring result = std::wstring(buffer);
	replace(result, L"\n", L" - ");
	PrintGameString(result, TextColor::WHITE);
    }
}

bool HasActions(Unit* pUnit) {
    return (pUnit != NULL && pUnit->dwUnitType == UnitType::ITEM
	&& ITEM_ACTIONS.contains(pUnit->dwUnitId));
}

POINT ScreenToAutomap(int nX, int nY)
{
    nX *= 32; nY *= 32;
    int x = ((nX - nY) / 2 / (*(int*)D2CLIENT_Divisor)) - (*D2CLIENT_Offset).x + 8;
    int y = ((nX + nY) / 4 / (*(int*)D2CLIENT_Divisor)) - (*D2CLIENT_Offset).y - 8;
    if (D2CLIENT_GetAutomapSize()) {
	--x;
	y += 5;
    }
    return { x, y };
}

//Hooked Methods

LRESULT WndProc(HWND hWnd, int msg, WPARAM wParam, LPARAM lParam) 
{
    if (msg == WM_KEYUP || msg == WM_SYSKEYUP ) {
	//Z debug
	if (wParam == 0x5A
	    && (GetKeyState(VK_LSHIFT) & 0x80) || (GetKeyState(VK_RSHIFT) & 0x80)
	    && (GetKeyState(VK_LCONTROL) & 0x80) || (GetKeyState(VK_RCONTROL) & 0x80)) {
	    ToggleDebug();
	}
	//R reload
	if (wParam == 0x52
	    && (GetKeyState(VK_LCONTROL) & 0x80) || (GetKeyState(VK_RCONTROL) & 0x80)) {
	    if (IsTxtDataLoaded) {
		ReloadFilter();
	    }
	}
    }
    return fpWndProc(hWnd, msg, wParam, lParam);
}

void __stdcall DATATBLS_LoadAllTxts(void* pMemPool, int a2, int a3)
{
    // Reload settings & filter for each new game session
    Config::UnloadSettings();

    fpDATATBLS_LoadAllTxts(pMemPool, a2, a3);
    IsTxtDataLoaded = true;

}

void __stdcall DrawGameUI() 
{
    fpD2CLIENT_DrawGameUI();

    // Load config & filter at start of game session
    if (IsTxtDataLoaded && !Config::IsLoadedSettings())
	ReloadFilter();
}

void HandlePacket(uint8_t* pBitStream, D2GSServerToClientPacketHandlerFn pHandler) 
{
    /*
    cmd = 0x9c (world)/0x9d (owned)
    [uint8_t bCmd] [uint8_t bAction] [uint8_t bMessageSize?] [uint8_t bUnk?] [uint32_t dwUnitId] ...
    0x9c: [uint8_t[...] itemData]
    0x9d: [uint8_t[0x28] unk?] [uint8_t[...] itemData]
    */
    uint32_t unitId = *(uint32_t*)&pBitStream[4];

    pHandler(pBitStream);

    Unit* pItem = FindUnit(unitId, UnitType::ITEM);
    if (pItem == NULL) return;

    // TODO: exclude NPC items?

    RunRules(pItem);
    DoChatAlert(pItem);
}


BOOL __fastcall GetItemName_114d(Unit* pItem, wchar_t* pBuffer, uint32_t dwSize) 
{
    BOOL ret = fpD2CLIENT_GetItemName_114d(pItem, pBuffer, dwSize);
    HandleItemName(pItem, pBuffer, dwSize);
    return ret;
}

BOOL __stdcall GetItemName_113c(Unit* pItem, wchar_t* pBuffer, uint32_t dwSize) 
{
    BOOL ret = fpD2CLIENT_GetItemName_113c(pItem, pBuffer, dwSize);
    HandleItemName(pItem, pBuffer, dwSize);
    return ret;
}

void __stdcall HandleItemName(Unit* pItem, wchar_t* pBuffer, uint32_t dwSize) 
{
    std::wstring& tokval = ITEM_ACTIONS[pItem->dwUnitId]->wsItemName;
    if (HasActions(pItem) && tokval.length()) {
	std::wstring copy = tokval;
	ireplace(copy, TOK_NAME, pBuffer);
	wcsncpy(pBuffer, copy.c_str(), 0x7d);
    }
}

void __stdcall GetItemDesc(wchar_t* pBuffer)
{
    Unit* pItem = *D2CLIENT_GetHoverItem;
    if (!pItem) return;
    std::wstring& tokval = (ITEM_ACTIONS[pItem->dwUnitId]->wsItemDesc);
    if (HasActions(pItem) && tokval.length()) {
	std::wstring copy = tokval;
	ireplace(copy, TOK_DESC, pBuffer);
	wcsncpy(pBuffer, copy.c_str(), 0x800);
    }
}

void __fastcall ItemActionWorld(uint8_t* pBitstream) {
    HandlePacket(pBitstream, fpItemActionWorld);
}

void __fastcall ItemActionOwned(uint8_t* pBitstream) {
    HandlePacket(pBitstream, fpItemActionOwned);
}

std::queue<uint32_t> AUTOMAP_ITEMS;

void __stdcall AUTOMAP_Draw() {
    fpAUTOMAP_Draw();
    while (!AUTOMAP_ITEMS.empty()) {
	Unit* pItem = FindUnit(AUTOMAP_ITEMS.front(), UnitType::ITEM);
	AUTOMAP_ITEMS.pop();
	if (pItem == NULL)
	    continue;

	if (HasActions(pItem)
	    && ITEM_ACTIONS[pItem->dwUnitId]->bMinimapIcon) {
	    POINT p = ScreenToAutomap(pItem->pStaticPath->nXPos, pItem->pStaticPath->nYPos);
	    D2GFX_DrawSolidRectEx(p.x - 5, p.y - 5, p.x + 5, p.y + 5, 
		ITEM_ACTIONS[pItem->dwUnitId]->nMinimapIconPaletteIndex, DrawMode::NORMAL);
	}
    }
}

//nXpos and nYpos don't appear to be right...
BOOL __fastcall UNITDRAW_DrawUnit(Unit* pUnit, uint32_t dwColorTint, int nXpos, int nYpos, BOOL bFade, BOOL bDrawOverlays) {
    if (HasActions(pUnit)
	&& ITEM_ACTIONS[pUnit->dwUnitId]->bMinimapIcon) {
	//this seems janky, but is used to queue up items to be drawn on automap
	AUTOMAP_ITEMS.push(pUnit->dwUnitId);
    }
    return fpUNITDRAW_DrawUnit(pUnit, dwColorTint, nXpos, nYpos, bFade, bDrawOverlays);
}

//returns result of nodraw flag shift
BOOL __fastcall IsUnitNoDraw(Unit* pUnit)
{
    if (HasActions(pUnit)) {
	if (ITEM_ACTIONS[pUnit->dwUnitId]->bHide && !IsFilterDebug) 
	    pUnit->dwFlagEx |= static_cast<std::underlying_type_t<UnitFlagEx>>(UnitFlagEx::NODRAW);
	else
	    pUnit->dwFlagEx &= ~(static_cast<std::underlying_type_t<UnitFlagEx>>(UnitFlagEx::NODRAW));
    }
    return pUnit->dwFlagEx >> 0x12;
}

void __stdcall UNITS_FreeUnit(Unit* pUnit) 
{
    if (HasActions(pUnit))
	ITEM_ACTIONS.erase(pUnit->dwUnitId);

    fpUNITS_FreeUnit(pUnit);
}

/*
Handles the following.
1) Item rect on ground when holding alt
2) Item rect on ground when you hover over
*/
void __stdcall DrawGroundItemRect(DWORD retAddress, BOOL isHovered, Unit* pItem, uint32_t nXStart, uint32_t nYStart, uint32_t nXEnd, uint32_t nYEnd, uint8_t nPaletteIndex, DrawMode eDrawMode) {
    if (retAddress == reinterpret_cast<DWORD>(D2CLIENT_callDrawAltDownItemRectRet)) {
	//drawing a rect that isnt for an item. this is kinda a hack checking ret address of caller.
	D2GFX_DrawSolidRectEx(nXStart, nYStart, nXEnd, nYEnd, nPaletteIndex, eDrawMode);
	return;
    }
    if (pItem == nullptr) {
	pItem = *D2CLIENT_GetHoverItem;
    }
    if ((isHovered || eDrawMode == DrawMode::NORMAL) && IsFilterDebug) {
	DrawDebugInfo(pItem, nXStart, nYStart, nXEnd, nYEnd);
    }
    if (HasActions(pItem)
	&& (pItem->eItemAnimMode == ItemAnimationMode::DROPPING
	    || pItem->eItemAnimMode == ItemAnimationMode::GROUND)) {
	nPaletteIndex = ITEM_ACTIONS[pItem->dwUnitId]->bBackgroundPaletteIndexSet ? ITEM_ACTIONS[pItem->dwUnitId]->nBackgroundPaletteIndex : nPaletteIndex;
	if (ITEM_ACTIONS[pItem->dwUnitId]->bBorderPaletteIndexSet) {
	    auto pad = 1;
	    RECT rect = { (LONG)(nXStart + pad), (LONG)(nYStart + pad), (LONG)(nXEnd - pad), (LONG)(nYEnd - pad) };
	    D2GFX_DrawRect(&rect, ITEM_ACTIONS[pItem->dwUnitId]->nBorderPaletteIndex);
	}
	if (eDrawMode == DrawMode::TRANS_50) {
	    eDrawMode = ITEM_ACTIONS[pItem->dwUnitId]->eDrawModeAlt;
	}
	else {
	    eDrawMode = ITEM_ACTIONS[pItem->dwUnitId]->eDrawModeHover;
	}
    }
    //call original
    D2GFX_DrawSolidRectEx(nXStart, nYStart, nXEnd, nYEnd, nPaletteIndex, eDrawMode);
}

void __stdcall DrawInventoryItemRect(Unit* pItem, uint32_t nXStart, uint32_t nYStart, uint32_t nXEnd, uint32_t nYEnd, uint8_t nPaletteIndex, DrawMode eDrawMode) {
    //0x08: red, 0xea: blue, 0x76: green.
    if (nPaletteIndex == 0xEA
	&& HasActions(pItem)) {
	nPaletteIndex = ITEM_ACTIONS[pItem->dwUnitId]->bInvBackgroundPaletteIndexSet ? ITEM_ACTIONS[pItem->dwUnitId]->nInvBackgroundPaletteIndex : nPaletteIndex;
    }
    //call original
    D2GFX_DrawSolidRectEx(nXStart, nYStart, nXEnd, nYEnd, nPaletteIndex, eDrawMode);
}

// Chat commands
BOOL __stdcall CallCommand(char* sCmd) {

    std::wstring cmdstr; std::string scmdstr(sCmd);
    cmdstr.assign(scmdstr.begin(), scmdstr.end());

    size_t tail = 0;
    if (BeginWith(cmdstr, L"/reload") > 0) {
	ReloadFilter();
    }
    else if ((tail = BeginWith(cmdstr, L"/pinglevel ")) > 0
	|| (tail = BeginWith(cmdstr, L"/pl ")) > 0) {

	PingLevel = std::stoi(cmdstr.substr(tail));
	Config::SaveSettings();
	ReloadFilter();
    }
    else if ((tail = BeginWith(cmdstr, L"/filterlevel ")) > 0
	|| (tail = BeginWith(cmdstr, L"/fl ") > 0)) {

	FilterLevel = std::stoi(cmdstr.substr(tail));
	Config::SaveSettings();
	ReloadFilter();
    }
    else if (BeginWith(cmdstr, L"/debug") > 0) {
	ToggleDebug();
    }
    else if ((tail = BeginWith(cmdstr, L"/test ")) > 0) {
	DebugRule(std::stoi(cmdstr.substr(tail)));
    }
    else if (BeginWith(cmdstr, L"/clip") > 0) {
	Clip();
    }
    else if (BeginWith(cmdstr, L"/help"))
	ShowHelp();
    else 
	return TRUE;

    return FALSE;
}



void ToggleDebug() {
    IsFilterDebug = !IsFilterDebug;
    INFO_LOG(L"Debug {}", (IsFilterDebug ? L"On" : L"Off"));
}

Unit* GetHoverItem() 
{
    Unit* pUnit = *D2CLIENT_GetHoverItem;
    if (IsUnitItem(pUnit))
	return pUnit;

    ERROR_LOG(L"No item found under cursor.");
    return nullptr;
}

void DebugRule(uint32_t nLineNumber) 
{
    Rule* rule = GlobalRules[nLineNumber];
    if (!rule) {
	ERROR_LOG(L"No rule found on line {}.", nLineNumber);
	return;
    }
    Unit* pItem = GetHoverItem();
    if (!pItem)
	return;
    for (Condition* condition : rule->GetConditions()) {
	TextColor color = condition->Evaluate(pItem) ? TextColor::GREEN : TextColor::RED;
	PrintGameString(condition->ToString(pItem), color);
    }
}

void Clip() 
{
    Unit* pItem = GetHoverItem();
    if (!pItem)
	return;

    LPTSTR  lptstrCopy;
    HGLOBAL hglbCopy;

    if (!OpenClipboard(NULL)) {
	return;
    }

    // Allocate a global memory object for the text.
    ItemsTxt txt = GetItemsTxt(pItem);

    std::wstring wCode = std::wstring(4, L' ');
    mbstowcs(&wCode[0], txt.szCode, 4);
    wCode = trim(wCode);

    std::wstring s = std::format(L"Show\n\tType {}\n\tCode {}\n\tQuality {}\n\tRarity {}\n\tClass {}\n\n",
	std::wstring(D2LANG_GetStringFromTblIndex(txt.wNameStr)),
	wCode,
	QualitiesLookup[GetQualityIndex(pItem)],
	RaritiesLookup[static_cast<int32_t>(pItem->pItemData->dwRarity)],
	ItemTypesLookup[static_cast<int32_t>(txt.wType[0])]);
    hglbCopy = GlobalAlloc(GMEM_MOVEABLE, (s.size() + 1) * sizeof(wchar_t));
    if (hglbCopy == NULL)
    {
	CloseClipboard();
	return;
    }

    lptstrCopy = (LPTSTR)GlobalLock(hglbCopy);
    if (!lptstrCopy)
	return;
    memcpy(lptstrCopy, s.c_str(), s.size() * sizeof(wchar_t));
    GlobalUnlock(hglbCopy);

    EmptyClipboard();
    SetClipboardData(CF_UNICODETEXT, hglbCopy);

    CloseClipboard();

    INFO_LOG(L"Item successfully copied to clipboard.");
}

void __stdcall DrawDebugInfo(Unit* pItem, uint32_t nXStart, uint32_t nYStart, uint32_t nXEnd, uint32_t nYEnd) {
    if (!HasActions(pItem)) {
	return;
    }
    ActionResult* actions = ITEM_ACTIONS[pItem->dwUnitId];
    std::vector<std::wstring> lines;
    std::wostringstream os;
    os << L"Matched Lines #: " << TEXT_GREEN;
    for (auto& match : actions->vMatchedRules) {
	if (&match != &actions->vMatchedRules.front()) {
	    os << L", ";
	}
	os << match;
    }
    ItemsTxt txt = GetItemsTxt(pItem);

    lines.push_back(std::format(L"ID: {}{}", TEXT_WHITE, pItem->dwUnitId));
    lines.push_back(std::format(L"File Index: {}{}", TEXT_WHITE, pItem->pItemData->dwFileIndex));
    lines.push_back(std::format(L"Type: {}{}", TEXT_WHITE, std::wstring(D2LANG_GetStringFromTblIndex(txt.wNameStr))));
    std::wstring wCode = std::wstring(4, L' ');
    mbstowcs(&wCode[0], txt.szCode, 4);
    lines.push_back(std::format(L"Code: {}{}", TEXT_WHITE, wCode));
    lines.push_back(std::format(L"Quality: {}{}", TEXT_WHITE, QualitiesLookup[GetQualityIndex(pItem)]));
    lines.push_back(std::format(L"Rarity: {}{}", TEXT_WHITE, RaritiesLookup[static_cast<int32_t>(pItem->pItemData->dwRarity)]));
    lines.push_back(std::format(L"Class: {}{}", TEXT_WHITE, ItemTypesLookup[static_cast<int32_t>(txt.wType[0])]));
    lines.push_back(os.str());
    lines.push_back(std::format(L"Shown: {}{}", TEXT_WHITE, actions->bHide ? L"False" : L"True"));
    if (actions->wsItemName.length()) 
	lines.push_back(std::format(L"Name: {}{}", TEXT_WHITE, actions->wsItemName));

    if (actions->wsItemDesc.length())
	lines.push_back(std::format(L"Description: {}{}", TEXT_WHITE, actions->wsItemDesc));

    if (actions->bChatAlert)
	lines.push_back(std::format(L"Chat Alert: {}{}", TEXT_WHITE, actions->bChatAlert ? L"True" : L"False"));

    if (actions->bMinimapIcon)
	lines.push_back(std::format(L"Minimap Icon: {}{:#04x}", TEXT_WHITE, actions->nMinimapIconPaletteIndex));

    if (actions->bBorderPaletteIndexSet)
	lines.push_back(std::format(L"Border Color: {}{:#04x}", TEXT_WHITE, actions->nBorderPaletteIndex));

    if (actions->bInvBackgroundPaletteIndexSet)
	lines.push_back(std::format(L"Inventory Color: {}{:#04x}", TEXT_WHITE, actions->nInvBackgroundPaletteIndex));

    uint32_t width = 0;
    for (auto& line : lines) {
	uint32_t w, fileNo;
	if (GameVersion() == D2Version::V114d || GameVersion() == D2Version::V110f)
	    w = D2WIN_GetTextPixelWidth(line.c_str());
	else
	    D2WIN_GetTextPixelWidthFileNo(line.c_str(), &w, &fileNo);

	if (w > width)
	    width = w;
    }

    auto mid = (nXEnd - nXStart) / 2 + nXStart;
    bool left = (mid >= (*D2CLIENT_ResolutionX / 2));
    auto pad = 10;
    auto xStart = 0, xEnd = 0;
    if (left) {
	xStart = nXStart - width - pad * 2;
	xEnd = nXStart;
    }
    else {
	xStart = nXEnd;
	xEnd = nXEnd + width + pad * 2;
    }
    auto lineHeight = 15;
    auto y = nYStart;

    D2GFX_DrawSolidRectEx(xStart, y, xEnd, y + (lines.size() * lineHeight), 0x0, DrawMode::TRANS_75);
    for (auto& line : lines)
	D2WIN_D2DrawText(line.c_str(), xStart + pad, y += lineHeight, TextColor::GOLD, TRUE);

}

// Stubs 

void __declspec(naked) __stdcall GetItemDesc_STUB() 
{
    __asm {
	add esp, 0x808;
	push eax;
	call GetItemDesc;
	ret 0xc;
    }
}

void __declspec(naked) __stdcall GetItemDesc_STUB_114d() 
{
    __asm {
	mov esp, ebp
	pop ebp
	push eax;
	call GetItemDesc;
	ret 0xc;
    }
}

void __declspec(naked) __stdcall CheckUnitNoDraw1_STUB() 
{
    __asm
    {
	push ecx;
	push edx;
	mov ecx, esi;
	call IsUnitNoDraw;
	pop edx;
	pop ecx;
	ret;
    }
}

void __declspec(naked) __stdcall CheckUnitNoDraw2_STUB() 
{
    __asm
    {
	push eax;
	push ecx;
	mov ecx, edi;
	call IsUnitNoDraw;
	mov edx, eax;
	pop ecx;
	pop eax;
	ret;
    }
}

void __declspec(naked) __stdcall CheckUnitNoDraw2_STUB_114d() 
{
    __asm
    {
	push ecx;
	push edx;
	mov ecx, ebx;
	call IsUnitNoDraw;
	pop edx;
	pop ecx;
	ret;
    }
}

void __declspec(naked) __stdcall DrawAltDownItemRect_STUB_110f()
{
    __asm
    {
	push[esp + 0x18];
	push[esp + 0x18];
	push[esp + 0x18];
	push[esp + 0x18];
	push[esp + 0x18];
	push[esp + 0x18];
	mov eax, [esp + 0x4c];
	push[eax - 0x4];
	push 0;
	push 0;
	call DrawGroundItemRect;
	ret 0x18;
    }
}

void __declspec(naked) __stdcall DrawAltDownItemRect_STUB() 
{
    __asm
    {
	push[esp + 0x18];
	push[esp + 0x18];
	push[esp + 0x18];
	push[esp + 0x18];
	push[esp + 0x18];
	push[esp + 0x18];
	push[esi - 0x4];
	push 0;
	push 0;
	call DrawGroundItemRect;
	ret 0x18;
    }
}

void __declspec(naked) __stdcall DrawAltDownItemRect_STUB_114d()
{
    __asm
    {
	push[esp + 0x18];
	push[esp + 0x18];
	push[esp + 0x18];
	push[esp + 0x18];
	push[esp + 0x18];
	push[esp + 0x18];
	mov eax, [esp + 0x40];
	push[eax - 0x4];
	push 0;
	push[esp + 0x60];
	call DrawGroundItemRect;
	ret 0x18;
    }
}

void __declspec(naked) __stdcall DrawHoverItemRect_STUB()
{
    __asm
    {
	push[esp + 0x18];
	push[esp + 0x18];
	push[esp + 0x18];
	push[esp + 0x18];
	push[esp + 0x18];
	push[esp + 0x18];
	//seems to return item cursor is over on ground
	call D2CLIENT_fpGroundHoverUnit;
	push eax;
	push 1;
	push 0;
	call DrawGroundItemRect;
	ret 0x18;
    }
}

//Copy what D2Client.6FB5B0F0(guessed Arg1, Arg2, Arg3, Arg4) is doing and put ptItem
void __declspec(naked) __stdcall DrawInventoryItemRect_STUB_110f()
{
    __asm
    {
	push[esp + 0x10];
	push[esp + 0x10];
	mov eax, [esp + 0x10];
	add eax, edx;
	push eax;
	mov eax, [esp + 0x10];
	add eax, ecx;
	push eax;
	push edx;
	push ecx;
	push[esp + 0x38];
	call DrawInventoryItemRect;
	ret 0x10;
    }
}

void __declspec(naked) __stdcall DrawInventoryItemRect_STUB()
{
    __asm
    {
	push[esp + 0x18];
	push[esp + 0x18];
	push[esp + 0x18];
	push[esp + 0x18];
	push[esp + 0x18];
	push[esp + 0x18];
	push[esp + 0x3C];
	call DrawInventoryItemRect;
	ret 0x18;
    }
}

void __declspec(naked) __stdcall DrawInventoryItemRect_STUB_114d()
{
    __asm
    {
	push[esp + 0x10];
	push[esp + 0x10];
	mov eax, [esp + 0x10];
	add eax, edx;
	push eax;
	mov eax, [esp + 0x10];
	add eax, ecx;
	push eax;
	push edx;
	push ecx;
	push[esp + 0x2c];
	call DrawInventoryItemRect;
	ret 0x10;
    }
}

//straight from plugy source but after (nopickup instead of soundchoasdebug)
void __declspec(naked) __stdcall CallCommand_STUB_114d()
{
    __asm
    {
	TEST EAX, EAX
	JE MANAGESOUNDCHAOSDEBUG
	PUSH EDI
	CALL CallCommand
	TEST EAX, EAX
	JNZ MANAGESOUNDCHAOSDEBUG
	ADD DWORD PTR SS : [ESP] , 19
	MANAGESOUNDCHAOSDEBUG :
	RETN 8
    }
}

void __declspec(naked) __stdcall CallCommand_STUB() 
{
    __asm
    {
	TEST EAX, EAX
	JE MANAGESOUNDCHAOSDEBUG
	PUSH ESI
	CALL CallCommand
	TEST EAX, EAX
	JNZ MANAGESOUNDCHAOSDEBUG
	ADD DWORD PTR SS : [ESP] , 21
	MANAGESOUNDCHAOSDEBUG :
	RETN 8
    }
}

} // end of namespace ItemFilter
