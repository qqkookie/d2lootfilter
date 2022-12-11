#include <algorithm>
#include "D2Ptrs.h"
#include "Globals.h"
#include "Utils.h"

// This should be initialized before initialing "D2Ptr.h" constants. 
D2Version D2GameVersion = D2Version::NONE;

D2Version GameVersion()
{
	_ASSERT(D2GameVersion != D2Version::NONE && D2GameVersion != D2Version::ERROR);
	return D2GameVersion;
}

void  PrintGameString(std::wstring wStr, TextColor color) 
{
	if (GameVersion() == D2Version::V114d || GameVersion() == D2Version::V110f)
		D2CLIENT_PrintGameStringe_114d(wStr.c_str(), color);
	else
		D2CLIENT_PrintGameString(wStr.c_str(), color);
}

Unit* FindUnitFromTable(uint32_t unitId, UnitType type) 
{
	UnitHashTable serverSideUnitTable = D2CLIENT_ServerSideUnitHashTables[static_cast<int32_t>(type)];
	Unit* unit = serverSideUnitTable.table[unitId];
	if (unit == NULL) {
		UnitHashTable clientSideUnitTable = D2CLIENT_ClientSideUnitHashTables[static_cast<int32_t>(type)];
		unit = clientSideUnitTable.table[unitId];
	}
	return unit;
}

Unit* FindUnit(uint32_t unitId, UnitType type)
{
	Unit* unit = D2CLIENT_FindServerSideUnit(unitId, type);
	if (unit == NULL)
	    unit = D2CLIENT_FindClientSideUnit(unitId, type);
	return unit;
}

uint32_t __fastcall GetDllOffset(uint32_t baseAddress, int offset) 
{
	if (IsTxtDataLoaded) {
		// Don't use DEBUG_LOG() before PrintGameString() is working
		DEBUG_LOG(L"baseAddress: {}\n", baseAddress);
		DEBUG_LOG(L"offset: {}\n", offset);
		DEBUG_LOG(L"return: {}\n", baseAddress + offset);
	}
	if (offset < 0)
		return (uint32_t)GetProcAddress((HMODULE)baseAddress, (LPCSTR)(-offset));

	return baseAddress + offset;
}


ItemsTxt GetItemsTxt(Unit* pItem) {
	return D2COMMON_ItemDataTbl->pItemsTxt[pItem->dwLineId];
}

std::wstring GetItemCode(Unit* pItem) 
{
	ItemsTxt txt = GetItemsTxt(pItem);
	std::wstring wCode = std::wstring(4, L' ');
	mbstowcs(&wCode[0], txt.szCode, 4);
	wCode = trim(wCode);
	return wCode;
}

int GetQualityIndex(Unit* pItem) 
{
	ItemsTxt txt = GetItemsTxt(pItem);
	int32_t quality = -1;
	if (txt.dwCode == txt.dwUltraCode)
		quality = 2;
	else if (txt.dwCode == txt.dwUberCode)
		quality = 1;
	else if (txt.dwCode == txt.dwNormCode)
		quality = 0;
	
	return quality;
}

bool IsUnitItem(Unit* pUnit) {
	return pUnit != NULL && pUnit->dwUnitType == UnitType::ITEM;
}

namespace ItemFilter 
{
	extern D2CLIENT_GetItemName_113c_t fpD2CLIENT_GetItemName_113c;
	extern D2CLIENT_GetItemName_114d_t fpD2CLIENT_GetItemName_114d;

	bool GetItemNameBuf(Unit* pItem, wchar_t* pBuffer, uint32_t dwSize)
	{
		if (GameVersion() == D2Version::V113c)
			return  fpD2CLIENT_GetItemName_113c(pItem, pBuffer, dwSize);
		else
			return  fpD2CLIENT_GetItemName_114d(pItem, pBuffer, dwSize);
	}
}

std::wstring GetItemName(Unit* pItem)
{
	wchar_t buf[100] = { 0, };
	ItemFilter::GetItemNameBuf(pItem, buf, 99);
	return (std::wstring)buf;
}

int GetItemLevel(Unit* pItem) {
	return pItem->pItemData->dwItemLevel;
}

int GetQualityLevel(Unit* pItem) {
	return GetItemsTxt(pItem).nLevel;
}

int GetMagicLevel(Unit* pItem) {
	return GetItemsTxt(pItem).nMagicLevel;
}

static int CalcAffixLevel(int iLvl, int qLvl, int mLvl) 
{
	if (iLvl > 99) iLvl = 99;
	if (qLvl > iLvl) iLvl = qLvl;
	if (mLvl > 0) 
		return ((iLvl + mLvl > 99) ? 99 : (iLvl + mLvl));
	else 
		return ((iLvl < (99 - qLvl / 2)) ? (iLvl- qLvl/ 2) : (iLvl * 2 - 99));
}

int GetAffixLevel(Unit* pItem)
{
	ItemsTxt itxt = GetItemsTxt(pItem);
	return CalcAffixLevel(pItem->pItemData->dwItemLevel, itxt.nLevel, itxt.nMagicLevel);
}

int GetCraftAffixLevel(Unit* pItem) 
{
	int32_t clvl = GetCharacterLevel();
	ItemsTxt itxt = GetItemsTxt(pItem);
	int32_t ilvl = pItem->pItemData->dwItemLevel;
	return CalcAffixLevel(ilvl / 2 + clvl / 2, itxt.nLevel, itxt.nMagicLevel);
}

int GetItemPrice(Unit* pItem)
{
	Unit* pPlayer = D2CLIENT_GetPlayerUnit();
	if (pItem == NULL || pPlayer == NULL)
		return 0;
	// nPrice = D2COMMON_ITEMS_GetTransactionCost(pPlayer, pItem, 
	//	D2CLIENT_GetDifficulty(), D2CLIENT_GetQuestFlags(), 0x201, 1);
	static uint8_t d = D2CLIENT_GetDifficulty();   // to avoid optimization
	//  CRASHS here on loading when optimized for release.
	return D2COMMON_ITEMS_GetTransactionCost(pPlayer, pItem, d, D2CLIENT_GetQuestFlags(), 0x201, 1);
}

// Averge damege * damage enhancement
int GetWeaponDamage(Unit* pItem) 
{
	if (D2COMMON_ITEMS_CheckItemTypeId(pItem, ItemType::WEAPON))
		return ((GetD2UnitStat(pItem, Stat::MINDAMAGE, 0) * GetD2UnitStat(pItem, Stat::ITEM_ARMOR_PERCENT, 0)
			+ GetD2UnitStat(pItem, Stat::MAXDAMAGE, 0) * GetD2UnitStat(pItem, Stat::ITEM_ARMOR_PERCENT, 0)) + 100) / 200;
	return 0;
}

// F8(STD, D2Common, 10519, 10519, 10519, 11092, 10061, 10658, 10973, 10550, 225480, int, D2GetPlayerStat, (Unit* ptChar, DWORD statID, DWORD index));//ONLY 1.11b
// F2(D2COMMON, STATLIST_GetUnitStatUnsigned, uint32_t, __stdcall, (const Unit* pUnit, Stat nStatId, uint16_t nLayer), -10520, -10973, 0x225480);
// int D2GetPlayerStat(Unit* ptChar, DWORD statID, DWORD index)

uint32_t GetD2UnitStat(const Unit* pUnit, Stat nStatId, uint16_t nLayer)
{
	uint32_t rr = D2COMMON_STATLIST_GetUnitStatUnsigned(pUnit, nStatId, nLayer);
	return rr;
}

ItemMode GetItemMode(Unit* pItem)
{
	if (!IsUnitItem(pItem)) return ItemMode::NONE;

	ItemAnimationMode mode = pItem->eItemAnimMode;
	InventoryPage inv = pItem->pItemData->nInvPage;
	int owner = pItem->pItemData->dwOwnerGUID;
	// DEBUG_LOG(L"mode, inv, owner = {} {} {}\n", (int)mode, (int)inv, (int)owner );

	switch (mode) {
	case ItemAnimationMode::GROUND:
	case ItemAnimationMode::DROPPING:
		return ItemMode::DROP; // on ground

	case ItemAnimationMode::BELT:	return ItemMode::BELT;
	case ItemAnimationMode::CURSOR: return ItemMode::PICK;
	case ItemAnimationMode::SOCKET:	return ItemMode::SOCKET;

	case ItemAnimationMode::BODY:
		return owner > 0 ? ItemMode::EQUIP : ItemMode::HIRING;

	case ItemAnimationMode::INVENTORY:
		if (inv == InventoryPage::INVENTORY) return ItemMode::INVEN;
		else if (inv == InventoryPage::STASH) return ItemMode::STASH;
		else if (inv == InventoryPage::CUBE )
			return (owner > 0 ? ItemMode::CUBE : ItemMode::VENDOR);
	}
	return ItemMode::ERROR;
}

bool IsItemDropped(Unit * pItem)
{
    return (pItem->eItemAnimMode == ItemAnimationMode::DROPPING
	|| pItem->eItemAnimMode == ItemAnimationMode::GROUND);
}

std::wstring GetCharacterName() 
{
	// first 16 bytes of PlayerData is character name in char[16]
	const char* chname = (const char *) D2CLIENT_GetPlayerUnit()->pPlayerData;
	if (!chname || !*chname) return L"";
	std::string sstr(chname); std::wstring wstr;
	wstr.assign(sstr.begin(), sstr.end());
	return wstr;
}

int GetCharacterClass() {
    return static_cast<int>( D2CLIENT_GetPlayerUnit()->dwClassId);
}

int GetCharacterLevel() {
	// return D2COMMON_STATLIST_GetUnitStatUnsigned(D2CLIENT_GetPlayerUnit(), Stat::LEVEL, 0);
	return static_cast<int>( GetD2UnitStat( D2CLIENT_GetPlayerUnit(), Stat::LEVEL, 0));
}

uint32_t GAME_EXE;	    // v114d uses this value only.
uint32_t DLLBASE_D2CLIENT;  // All others are 0 for v114d.
uint32_t DLLBASE_D2CMP;
uint32_t DLLBASE_D2COMMON;
uint32_t DLLBASE_D2GAME;
uint32_t DLLBASE_D2GFX;
uint32_t DLLBASE_D2LANG;
uint32_t DLLBASE_D2LAUNCH;
uint32_t DLLBASE_D2MCPCLIENT;
uint32_t DLLBASE_D2MULTI;
uint32_t DLLBASE_D2NET;
uint32_t DLLBASE_D2SOUND;
uint32_t DLLBASE_D2WIN;
uint32_t DLLBASE_FOG;
uint32_t DLLBASE_STORM;

int InitGame() {
	// called on D2 game launching in first part of "action.c".
	extern void InitGameVersion();
	if (D2GameVersion == D2Version::NONE) {
		InitGameVersion();

		GAME_EXE = (uint32_t)GetModuleHandle(NULL);
		DLLBASE_D2CLIENT = (uint32_t)LoadLibraryA("D2Client.dll");
		DLLBASE_D2CMP = (uint32_t)LoadLibraryA("D2CMP.dll");
		DLLBASE_D2COMMON = (uint32_t)LoadLibraryA("D2Common.dll");
		DLLBASE_D2GAME = (uint32_t)LoadLibraryA("D2Game.dll");
		DLLBASE_D2GFX = (uint32_t)LoadLibraryA("D2Gfx.dll");
		DLLBASE_D2LANG = (uint32_t)LoadLibraryA("D2Lang.dll");
		DLLBASE_D2LAUNCH = (uint32_t)LoadLibraryA("D2Launch.dll");
		DLLBASE_D2MCPCLIENT = (uint32_t)LoadLibraryA("D2MCPClient.dll");
		DLLBASE_D2MULTI = (uint32_t)LoadLibraryA("D2Multi.dll");
		DLLBASE_D2NET = (uint32_t)LoadLibraryA("D2Net.dll");
		DLLBASE_D2SOUND = (uint32_t)LoadLibraryA("D2Sound.dll");
		DLLBASE_D2WIN = (uint32_t)LoadLibraryA("D2Win.dll");
		DLLBASE_FOG = (uint32_t)LoadLibraryA("Fog.dll");
		DLLBASE_STORM = (uint32_t)LoadLibraryA("Storm.dll");
	}
	return 1;
}

// ********** std::string handling ***********

std::wstring_view ltrim(std::wstring_view s) 
{
	s.remove_prefix(std::distance(s.cbegin(), std::find_if(s.cbegin(), s.cend(),
		[](wchar_t c) {return !iswspace(c); })));
	return s;
}

std::wstring_view rtrim(std::wstring_view s)
{
	s.remove_suffix(std::distance(s.crbegin(), std::find_if(s.crbegin(), s.crend(),
		[](wchar_t c) {return !iswspace(c); })));
	return s;
}

std::wstring_view trim(std::wstring_view s) {
	return ltrim(rtrim(s));
}

// trim from both ends (copying)
std::wstring trim_copy(std::wstring s) 
{
    trim(s);
    return std::wstring(s);
}

/*
std::wstring ltrim_copy(std::wstring s) {
	ltrim(s);
	return std::wstring(s);
}

// trim from end (copying)
std::wstring rtrim_copy(std::wstring s) {
	rtrim(s);
	return std::wstring(s);
}
*/

void replace(std::wstring& subject, const std::wstring& search, const std::wstring& replace) 
{
	size_t pos = 0;
	while ((pos = subject.find(search, pos)) != std::string::npos) {
		subject.replace(pos, search.length(), replace);
		pos += replace.length();
	}
}

// find first needle in haystack, starting from offset, ignoring case.
size_t istrfind(const std::wstring& haystack, const std::wstring& needle, size_t offset /*= 0*/)
{
	auto itr = std::search(haystack.begin() + offset, haystack.end(), needle.begin(), needle.end(),
		[](wchar_t hch, wchar_t nch) { return std::tolower(hch) == std::tolower(nch); });
	return (std::distance(haystack.begin(), itr));
}

// find and replace a pattern, ignoring case
void ireplace(std::wstring& subject, const std::wstring& search, const std::wstring& replace) 
{
	size_t pos = 0;
	while ((pos = istrfind(subject, search, pos)) < subject.length()){
		subject.replace(pos, search.length(), replace);
		pos += replace.length();
	}
}

// Check if list, delimetered by delims, have the name, ignoring case.
bool ListHave(std::wstring& list, std::wstring& name, const wchar_t *delims /*= L" \t\n,:;/"*/ )
{
	size_t look = 0;
	size_t llen = list.length(), nlen = name.length();
	while ((look = istrfind(list, name, look)) < llen) {
		look += nlen;
		if (look == llen || wcschr(delims, list[look]))
			return true;
		look++;
	}
	return false;
}

// Test line starts with the literal, leading spaces and case ignored. 
// Returns index of trailing parts of the line.
size_t BeginWith(std::wstring& line, const wchar_t* match)
{
	const wchar_t* s = line.c_str();
	while (iswspace(*s)) s++;
	size_t len = wcslen(match);
	return (_wcsnicmp(s, match, len) ? 0 : (s - match + len));
}

size_t BeginWith(std::string& line, const char* match)
{
	const char* s = line.c_str();
	while (isspace(*s)) s++;
	size_t len = strlen(match);
	return (_strnicmp(s, match, len) ? 0 : (s - match + len));
}

