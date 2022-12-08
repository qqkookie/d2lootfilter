#pragma once

#include <vector>
#include <unordered_map>
#include "Rule.h"

extern std::unordered_map<std::wstring, std::vector<Action*>> GlobalStyles;
extern std::vector<Rule*> GlobalRules;

namespace Configuration {
	void SaveSettings();
	void LoadRules();
	bool IsLoadedSettings();
	void UnloadSettings();
}

extern void InitGameVersion();
