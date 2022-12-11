#pragma once

#include <format>
#include "D2Structs.h"
#include "D2Tables.h"

#ifdef _DEBUG
#define DEBUG_LOG(f, ...)   PrintLog(LOGLEV::DEBUG, f, __VA_ARGS__);
#else
#define DEBUG_LOG(f, ...) ;
#endif

#define INFO_LOG(f, ...)    PrintLog(LOGLEV::INFO, f, __VA_ARGS__);
#define WARN_LOG(f, ...)    PrintLog(LOGLEV::WARN, f, __VA_ARGS__);
#define ERROR_LOG(f, ...)   PrintLog(LOGLEV::ERROR, f, __VA_ARGS__);

#undef ERROR
enum LOGLEV { NONE, DEBUG = TextColor::YELLOW, INFO = TextColor::YELLOW,
	WARN = TextColor::ORANGE, ERROR = TextColor::RED };

template <typename... Args>
void PrintLog(enum LOGLEV lev, std::wstring_view fmt, Args && ... args)  {
	PrintGameString(std::vformat(fmt, std::make_wformat_args(args...)), static_cast<TextColor>(lev));
}

extern D2Version GameVersion();
extern int InitGame();

//Hooking
extern void PrintGameString(std::wstring wStr, TextColor color);

extern Unit* FindUnitFromTable(uint32_t unitId, UnitType type);
extern Unit* FindUnit(uint32_t unitId, UnitType type);

extern uint32_t __fastcall GetDllOffset(uint32_t baseAddress, int offset);

//Utility D2 Methods
extern ItemsTxt GetItemsTxt(Unit* pItem);
extern std::wstring GetItemCode(Unit* pItem);
extern int GetQualityIndex(Unit* pItem);
extern bool IsUnitItem(Unit* pUnit);

extern std::wstring GetItemName(Unit* pItem);
extern int GetItemLevel(Unit* pItem);
extern int GetQualityLevel(Unit* pItem);
extern int GetMagicLevel(Unit* pItem);
extern int GetAffixLevel(Unit* pItem);
extern int GetCraftAffixLevel(Unit* pItem);

extern int GetItemPrice(Unit* pItem);
extern int GetWeaponDamage(Unit* pItem);
extern uint32_t GetD2UnitStat(const Unit* pUnit, Stat nStatId, uint16_t nLayer);

enum class ItemMode {   
    ERROR,	// error
    DROP,	// on Ground
    GROUND = DROP,
    VENDOR,	// in store
    PICK,	// on cursor

    INVEN, 	// in inventory,
    STASH,
    CUBE,
    EQUIP, 	// Equipped on body
    BELT,	// in belt rows
    HIRING,	// equipped by mercenary
    SOCKET,	// Socketed in another Item
    NONE = 0xff
};

extern ItemMode GetItemMode(Unit* pItem);
extern bool IsItemDropped(Unit* pItem);

extern std::wstring GetCharacterName();
extern int GetCharacterClass();
extern int GetCharacterLevel();

//String funcs
extern std::wstring_view ltrim(std::wstring_view s);
extern std::wstring_view rtrim(std::wstring_view s);
extern std::wstring_view trim(std::wstring_view s);
extern std::wstring trim_copy(std::wstring s);
/*
extern std::wstring ltrim_copy(std::wstring s);
extern std::wstring rtrim_copy(std::wstring s);
*/
extern void replace(std::wstring& subject, const std::wstring& search, const std::wstring& replace);

extern size_t istrfind(const std::wstring& haystack, const std::wstring& needle, size_t offset = 0);
extern void ireplace(std::wstring& subject, const std::wstring& search, const std::wstring& replace);
extern bool ListHave(std::wstring& list, std::wstring& name, const wchar_t* delims = L" \t\n,:;/");

extern size_t BeginWith(std::wstring& line, const wchar_t* match);
extern size_t BeginWith(std::string& line, const char* match);
