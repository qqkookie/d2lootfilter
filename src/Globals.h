#pragma once

#include <unordered_map>

extern int32_t FilterLevel;
extern int32_t PingLevel;
extern bool IsFilterDebug;
extern bool IsTxtDataLoaded;

extern std::unordered_map<std::wstring, int32_t> ItemTypes;
extern std::unordered_map<int32_t, std::wstring> ItemTypesLookup;

extern std::unordered_map<std::wstring, int32_t> Rarities;
extern std::unordered_map<std::wstring, int32_t> RaritiesFixed;
extern std::unordered_map<int32_t, std::wstring> RaritiesLookup;

extern std::unordered_map<std::wstring, int32_t> Qualities;
extern std::unordered_map<int32_t, std::wstring> QualitiesLookup;

extern std::unordered_map<std::wstring, std::wstring> CustomStats;

extern std::unordered_map<std::wstring, int32_t> Difficulties;
extern std::unordered_map<std::wstring, int32_t> ItemCats;

extern std::unordered_map<std::wstring, int32_t> ItemModeList;

extern std::vector<std::wstring> command_help;

extern void ShowHelp();
