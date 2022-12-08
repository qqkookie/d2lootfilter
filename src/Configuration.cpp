#include <filesystem>
#include <sstream>
#include <algorithm>
#include "Configuration.h"
#include "D2Ptrs.h"
#include "Globals.h"
#include "mINI.h"

#define SETTINGS_SECTION	L"D2LootFilter"
#define SETTING_INI		L"d2lootfilter.ini"
#define FILTER_FILE		L"d2lootfilter-Rules.txt"
#define DEFAULT_FILTER_FILE	L"{SaveDir}\\" FILTER_FILE
#define D2SE_SETUP_INI		L"D2SE_SETUP.ini"
#define BASEMOD_INI		L"BaseMod.ini"
#define PLUGY_INI		L"PlugY.ini"

#define COMMENT_STR	L"#"
#define STYLE_STR	L"Style"
#define SHOW_STR	L"Show"
#define HIDE_STR	L"Hide"
#define CHECK_STR	L"Check"

bool checkFile(std::wstring path);
std::wstring setDefault(std::wstring& str, std::wstring defval);
void InitGameVersion();

std::unordered_map<std::wstring, std::vector<Action*>> GlobalStyles;
std::vector<Rule*> GlobalRules;

namespace Configuration {

struct Settings_st {
	std::wstring wPath;
	int32_t nFilterLevel = 0;
	int32_t nPingLevel = 0;
} Settings;

bool settings_loaded = false;
bool settings_no_save = false;

bool IsLoadedSettings() { return settings_loaded; }
void UnloadSettings() { settings_loaded = false; }

void ReadSettings();
void SaveSettings();
void LoadRules();

void HandleToken(uint32_t lineNumber, std::vector<std::wstring>& lines);
std::vector<Action*> ParseStyle(std::vector<std::wstring>& lines);
Rule* ParseRule(std::vector<std::wstring>& lines);
std::vector<Condition*> ParseConditions(std::vector<std::wstring>& lines);
std::vector<Action*> ParseActions(std::vector<std::wstring>& lines);


void ReadSettings()
{
    mINI::INIFile d2lootfilterini(SETTING_INI);
    mINI::INIStructure data;

    if (! checkFile(SETTING_INI) || ! d2lootfilterini.read(data)) {
	// try to read settings from "[D2LootFilter]" section of "BaseMod.ini" or "PlugY.ini"
	mINI::INIFile basemodini(BASEMOD_INI);
	mINI::INIFile plugyini(PLUGY_INI);
	if ((checkFile(BASEMOD_INI) && basemodini.read(data) && !data[SETTINGS_SECTION][L"Path"].empty())
	    || (checkFile(PLUGY_INI) && plugyini.read(data) && !data[SETTINGS_SECTION][L"Path"].empty())) {
	    settings_no_save = true;
	}
    }

    Settings.wPath = setDefault(data[SETTINGS_SECTION][L"Path"], DEFAULT_FILTER_FILE);
    Settings.nFilterLevel = std::stoi(setDefault(data[SETTINGS_SECTION][L"FilterLevel"], std::to_wstring(FilterLevel)));
    Settings.nPingLevel = std::stoi(setDefault(data[SETTINGS_SECTION][L"PingLevel"], std::to_wstring(PingLevel)));
}

void SaveSettings() 
{
    if (settings_no_save) return;

    _ASSERT(IsLoadedSettings());
    // if (!IsLoadedSettings())	ReadSettings();

    mINI::INIFile file(SETTING_INI);
    mINI::INIStructure data;

    data[SETTINGS_SECTION][L"Path"] = Settings.wPath;
    data[SETTINGS_SECTION][L"FilterLevel"] = std::to_wstring(FilterLevel);
    data[SETTINGS_SECTION][L"PingLevel"] = std::to_wstring(PingLevel);

    file.generate(data);
}

static std::wstring FindFilterFile(std::wstring& path_setting)
{
    wchar_t homedir[MAX_PATH] = { L'\0' };
    std::wstring filpath = path_setting,
	savedir = L"Save";

    if (GameVersion() == D2Version::V114d
	&& GetEnvironmentVariableW(L"USERPROFILE", homedir, MAX_PATH) > 0)
	savedir = wcscat(homedir, L"\\Saved Games\\Diablo II");

    replace(filpath, L"{SaveDir}", savedir);

    std::wstring wname(GetCharacterName());
    replace(filpath, L"{CharName}", wname);

    if (checkFile(filpath))
	return filpath;

    filpath = FILTER_FILE; 	// fall back
    path_setting = filpath;

    if (!checkFile(filpath)) {
	// not exist. create empty.
	std::ofstream touch(filpath);
	touch.close();
    }
    return filpath;
}

void LoadRules()
{
    _ASSERT(IsTxtDataLoaded);

    auto t1 = std::chrono::high_resolution_clock::now();

    ReadSettings();

    PingLevel = Settings.nPingLevel;
    FilterLevel = Settings.nFilterLevel;

    std::wstring filterpath = FindFilterFile(Settings.wPath);
    DEBUG_LOG(L"filter path: {}", filterpath);

    if (!settings_no_save && !checkFile(SETTING_INI))
	SaveSettings();

    std::wifstream filter(filterpath);

    GlobalRules.clear();
    GlobalStyles.clear();

    uint32_t tokenLineNumber = 0;
    uint32_t currentLineNumber = 0;
    bool isChecking = false;

    std::wstring line, buf;
    std::vector<std::wstring> lines;

    while (std::getline(filter, buf).good()) {

	currentLineNumber++;
	if (line.empty())
	    tokenLineNumber = currentLineNumber;

	if (buf.find(COMMENT_STR) != std::wstring::npos)
	    buf = buf.erase(buf.find(COMMENT_STR));

	if (!buf.empty() && buf.back() == L'\\')
	    buf.pop_back();
	buf = trim(buf);
	if (buf.empty())
	    continue;

	line.append(buf);
	if (buf.back() == L',')
	    continue;

	if (line.compare(0, 5, STYLE_STR) == 0
	    || line.compare(0, 4, SHOW_STR) == 0
	    || line.compare(0, 4, HIDE_STR) == 0
	    || line.compare(0, 5, CHECK_STR) == 0)
	{
	    if (isChecking) {
		// Append simulated "Continue" action
		lines.push_back(L"Continue");
		HandleToken(tokenLineNumber - 1, lines);
		isChecking = false;
	    }
	    if (line.compare(0, 5, CHECK_STR) == 0)
		isChecking = true;
	    HandleToken(tokenLineNumber, lines);
	    // tokenLineNumber = currentLineNumber;
	}
	lines.push_back(line);
	line.clear();
    }
    HandleToken(tokenLineNumber, lines);

    InitTypesCodesRunesList();
    std::wstring dummy1;
    for (auto& rule : GlobalRules) {
	for (auto& condition : rule->GetConditions()) {
	    condition->Initialize(dummy1);
	}
    }

    /*
    for (auto &rule : GlobalRules) {
	    for (auto& condition : rule->GetConditions()) {
		    Logger::Info("%s\n", condition->ToString().c_str());
	    }
    }
    */

    settings_loaded = true;

    auto t2 = std::chrono::high_resolution_clock::now();
    auto ms_int = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);
    DEBUG_LOG(L"Rules parsed in {}ms", ms_int.count());
}

void HandleToken(uint32_t lineNumber, std::vector<std::wstring>& lines) {
    if (lines.empty())
	return;
    std::wstring line = lines[0]; lines.erase(lines.begin());
    if (line.compare(0, 5, STYLE_STR) == 0) {
	std::wstring style = line.substr(5);
	style = trim(style);
	GlobalStyles[style] = ParseActions(lines);
    }
    else {
	Rule* rule = ParseRule(lines);
	if (line.compare(0, 4, HIDE_STR) == 0)
	    rule->AddAction(new HideAction(), 0);
	else if (line.compare(0, 4, SHOW_STR) == 0 || line.compare(0, 5, CHECK_STR) == 0)
	    rule->AddAction(new ShowAction(), 0);

	rule->SetLineNumber(lineNumber);
	GlobalRules.push_back(rule);
    }
    for (auto& line : lines) {
	ERROR_LOG(L"Rule {}: Error parsing {}.", lineNumber, line);
    }
    lines.clear();
}

std::vector<Action*> ParseStyle(std::vector<std::wstring>& lines) {
    return ParseActions(lines);
}

Rule* ParseRule(std::vector<std::wstring>& lines) {
    Rule* rule = new Rule();
    rule->AddConditions(ParseConditions(lines));
    rule->AddActions(ParseActions(lines));
    return rule;
}

#define CONDITION(NAME) if(line.compare(0, wcslen(L#NAME), L#NAME) == 0) {\
	uint32_t len = wcslen(L#NAME); items.push_back(new NAME##Condition(line.length() > len \
	? trim_copy(line.substr(len + 1)) : L"")); match = true; }
#define ACTION(NAME) if(line.compare(0, wcslen(L#NAME), L#NAME) == 0) {\
	uint32_t len = wcslen(L#NAME); items.push_back(new NAME##Action(line.length() > len \
	? trim_copy(line.substr(len + 1)) : L"")); match = true; }

std::vector<Condition*> ParseConditions(std::vector<std::wstring>& lines) 
{
    std::vector<Condition*> items;
    lines.erase(std::remove_if(lines.begin(), lines.end(), [&items](std::wstring const& line) {
	bool match = false;
	         CONDITION(Code)
	    else CONDITION(Type)
	    else CONDITION(Class)
	    else CONDITION(Rarity)
	    else CONDITION(Ethereal)
	    else CONDITION(Runeword)
	    else CONDITION(Rune)

	    else CONDITION(ItemLevel)
	    else CONDITION(Quality)
	    else CONDITION(ItemId)
	    else CONDITION(ItemMode)
	    else CONDITION(Prefix)
	    else CONDITION(Suffix)

	    else CONDITION(Stats)
	    else CONDITION(Identified)
	    else CONDITION(Sockets)
	    else CONDITION(Price)
	    else CONDITION(Gold)
	    else CONDITION(Owning)

	    else CONDITION(ItemCat)
	    else CONDITION(ItemSize)
	    else CONDITION(AffixLevel)

	    else CONDITION(Or)
	    else CONDITION(Difficulty)
	    else CONDITION(AreaLevel)
	    else CONDITION(Random)

	    else CONDITION(CharacterClass)
	    else CONDITION(CharacterLevel)
	    else CONDITION(CharacterName)
	    else CONDITION(CharacterMaxHP)

		// else CONDITION(Defense)
		// else CONDITION(Armor)
		// else CONDITION(Weapon)
		// else CONDITION(Width)
		// else CONDITION(Height)

		return match;
	}), lines.end());
    return items;
}

std::vector<Action*> ParseActions(std::vector<std::wstring>& lines) 
{
    std::vector<Action*> items;
    lines.erase(std::remove_if(lines.begin(), lines.end(), [&items](std::wstring const& line) {
	bool match = false;
		 ACTION(Continue)
	    else ACTION(SetStyle)
	    else ACTION(SetName)
	    else ACTION(SetDescription)
	    else ACTION(SetBackgroundColor)
	    else ACTION(SetInventoryColor)
	    else ACTION(SetBorderColor)
	    else ACTION(ChatNotify)
	    else ACTION(PlayAlert)
	    else ACTION(MinimapIcon)
		return match;
	}), lines.end());
    return items;
}

#undef CONDITION
#undef ACTION

/*
//init any constants used in rules i.e. item names/skill names
void InitalizeConditionVariables() {
    //replace any variables used in the filter. i.e. item names/codes/rune numbers/etc...
    InitTypesCodesRunesList();
    // InitializeClassRaritiesQualitiesDifficultiesAndItemCat();
    std::wstring dummy1;
    for (auto& rule : GlobalRules) {
	for (auto& condition : rule->GetConditions()) {
	    condition->Initialize(dummy1);
	}
    }

    /*
    for (auto &rule : GlobalRules) {
	    for (auto& condition : rule->GetConditions()) {
		    Logger::Info("%s\n", condition->ToString().c_str());
	    }
    }
    *--/
}

void InitializeTypesCodesRunesAndCharClasses() {
    std::unordered_map<std::wstring, int32_t> names;
    std::unordered_map<std::wstring, int32_t> codes;
    std::unordered_map<std::wstring, int32_t> runes;

    DataTables* sgptDataTables = *D2COMMON_gpDataTables;

    for (int i = 0; i < D2COMMON_ItemDataTbl->nItemsTxtRecordCount; i++) {
	auto pItemTxt = D2COMMON_ItemDataTbl->pItemsTxt[i];
	std::wstring wNameStr = D2LANG_GetStringFromTblIndex(pItemTxt.wNameStr);
	names[wNameStr] = pItemTxt.dwCode;
	std::wstring wCode = std::wstring(4, L' ');
	mbstowcs(&wCode[0], pItemTxt.szCode, 4);
	wCode = trim(wCode);
	codes[wCode] = pItemTxt.dwCode;
	if (pItemTxt.wType[0] == ItemType::RUNE) {
	    int nRuneNumber = std::stoi(std::string(&pItemTxt.szCode[1], 3));
	    runes[wNameStr] = nRuneNumber;
	    size_t nFound = wNameStr.find(L" ");
	    if (nFound != std::wstring::npos) {
		runes[wNameStr.substr(0, nFound)] = nRuneNumber;
	    }
	}
    }

    std::unordered_map<std::wstring, int32_t> classNames;

    for (int32_t i = 0; i < sgptDataTables->nCharStatsTxtRecordCount; i++) {
	CharStatsTxt charStats = sgptDataTables->pCharStatsTxt[i];
	std::wstring className = std::wstring(charStats.wszClassName);
	className = trim(className);
	classNames[className] = i;
    }

    for (auto& rule : GlobalRules) {
	for (auto& condition : rule->GetConditions()) {
	    switch (condition->GetType()) {
	    case ConditionType::TYPE:
		condition->Initialize(names);
		break;
	    case ConditionType::CODE:
		condition->Initialize(codes);
		break;
	    case ConditionType::RUNE:
		condition->Initialize(runes);
		break;
	    case ConditionType::CHARACTER_CLASS:
		condition->Initialize(classNames);
		break;
	    default:
		break;
	    }
	}
    }
}

void InitializeClassRaritiesQualitiesDifficultiesAndItemCat() {
    for (auto& rule : GlobalRules) {
	for (auto& condition : rule->GetConditions()) {
	    switch (condition->GetType()) {
	    //case ConditionType::CLASS:
		//condition->Initialize(ItemTypes);
		// break;
	    //case ConditionType::RARITY:
		//condition->Initialize(RaritiesFixed);
		// break;
	    //case ConditionType::QUALITY:
		//condition->Initialize(Qualities);
		// break;
	    //case ConditionType::DIFFICULTY:
		//condition->Initialize(Difficulties);
		//break;
	    //case ConditionType::ITEM_CAT:
		//condition->Initialize(ItemCats);
		//break;
	    //default:
		break;
	    }
	}
    }
}

void InitializeOther() {
    std::unordered_map<std::wstring, int32_t> variables;
    std::wstring dummy1;

    for (auto& rule : GlobalRules) {
	for (auto& condition : rule->GetConditions()) {
	    switch (condition->GetType()) {
	    case ConditionType::STATS:
	    case ConditionType::ETHEREAL:
	    case ConditionType::RUNEWORD:
	    case ConditionType::PREFIX:
	    case ConditionType::SUFFIX:
	    case ConditionType::ITEM_LEVEL:
	    case ConditionType::AREA_LEVEL:
	    case ConditionType::CHARACTER_LEVEL:
		//			case ConditionType::DIFFICULTY:
	    case ConditionType::ITEM_ID:
	    case ConditionType::GOLD:
	    case ConditionType::DEFENSE:
		//			case ConditionType::ARMOR:
		//			case ConditionType::WEAPON:
	    case ConditionType::PRICE:
	    case ConditionType::ITEM_MODE:
	    case ConditionType::IDENTIFIED:
	    case ConditionType::SOCKETS:
		//			case ConditionType::WIDTH:
		//			case ConditionType::HEIGHT:
	    case ConditionType::RANDOM:
	    case ConditionType::OWNING:

	    case ConditionType::OR:
	    case ConditionType::ITEM_CAT:
	    case ConditionType::ITEM_SIZE:
	    case ConditionType::CHARACTER_NAME:
	    case ConditionType::CHARACTER_MAXHP:

		condition->Initialize(dummy1);
		break;
	    default:
		break;
	    }
	}
    }
}
*/

}  // end of namespace Configuration.

#undef COMMENT_STR
#undef STYLE_STR
#undef SHOW_STR
#undef HIDE_STR

// get string value. If empty, set default value.
std::wstring setDefault(std::wstring& str, std::wstring defval)
{
    if (str.empty())
	str = defval;
    return str;
}

bool checkFile(std::wstring path)
{
    return std::filesystem::exists(path);
}

#pragma comment(lib, "Version.Lib")

extern D2Version D2GameVersion;

void InitGameVersion() 
{
    D2Version version = D2Version::ERROR;

    std::wstring core;
    mINI::INIFile file(D2SE_SETUP_INI);
    mINI::INIStructure coredata;

    if (checkFile(D2SE_SETUP_INI)) {
	file.read(coredata);
	core = coredata[L"Protected"][L"D2Core"];
	if (core == L"1.114d") version = D2Version::V113c;
	else if (core == L"1.13c") version = D2Version::V113c;
	else if (core == L"1.10f") version = D2Version::V110f;
	else; // version = D2Version::ERROR;

	D2GameVersion = version;
	return;
    }

    D2GameVersion = D2Version::ERROR;

    HMODULE hModule = GetModuleHandle(NULL);
    HRSRC hResInfo = FindResource(hModule, MAKEINTRESOURCE(VS_VERSION_INFO), RT_VERSION);
    if (!hResInfo)
	return;
    HGLOBAL hResData = LoadResource(hModule, hResInfo);
    if (!hResData)
	return;

    LPVOID pVersionResource = LockResource(hResData);

    VS_FIXEDFILEINFO* ptFixedFileInfo = nullptr;
    UINT uLen;
    if (!VerQueryValue(pVersionResource, L"\\", (LPVOID*)&ptFixedFileInfo, &uLen)) {
	version = D2Version::NONE;
    }
    else if (uLen != 0) {
	WORD major = HIWORD(ptFixedFileInfo->dwFileVersionMS);
	WORD minor = LOWORD(ptFixedFileInfo->dwFileVersionMS);
	WORD revision = HIWORD(ptFixedFileInfo->dwFileVersionLS);
	WORD subrevision = LOWORD(ptFixedFileInfo->dwFileVersionLS);

	if (major == 1) {
	    if (minor == 14 && revision == 3 && subrevision == 71) version = D2Version::V114d;
	    else if (minor == 0 && revision == 13 && subrevision == 60) version = D2Version::V113c;
	    else if (minor == 0 && revision == 10 && subrevision == 39) version = D2Version::V110f;
	    else;
	}
    }
    FreeResource(hResData);

    D2GameVersion = version;
    return;
}
