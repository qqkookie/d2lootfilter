// Call InitGame() before initializing any static constants in "D2Ptrs.h" 
// to avoid "static initialization order fiasco" or race condition.
extern int InitGame();
static int dummy = InitGame();

#include <string>
#include <unordered_map>
#include "Action.h"
#include "Configuration.h"
#include "D2Ptrs.h"
#include "Globals.h"

#define COLOR(STR, IDX) { L"{"#STR"}", ##IDX## }
static std::unordered_map<std::wstring, std::wstring> COLOR_TO_STRING = {
	COLOR(White, TEXT_WHITE),
	COLOR(Red, TEXT_RED),
	COLOR(Green, TEXT_GREEN),
	COLOR(Blue, TEXT_BLUE),
	COLOR(Gold, TEXT_GOLD),
	COLOR(Gray, TEXT_GRAY),
	COLOR(Grey, TEXT_GRAY),
	COLOR(Black, TEXT_BLACK),
	COLOR(Tan, TEXT_TAN),
	COLOR(Orange, TEXT_ORANGE),
	COLOR(Yellow, TEXT_YELLOW),
	COLOR(Purple, TEXT_PURPLE),
	COLOR(DarkGreen, TEXT_DARK_GREEN),
	//Glide Only
	COLOR(Coral, TEXT_CORAL),
	COLOR(Sage, TEXT_SAGE),
	COLOR(Teal, TEXT_TEAL),
	COLOR(LightGray, TEXT_LIGHT_GRAY),
	COLOR(LightGrey, TEXT_LIGHT_GRAY),

	COLOR(MediumRed, TEXT_MEDIUM_RED),	    // blood red    
	COLOR(MediumGreen, TEXT_MEDIUM_GREEN),	    // grass green
	COLOR(MediumYellow, TEXT_MEDIUM_YELLOW),    // dim yellow
	COLOR(MediumBlue, TEXT_MEDIUM_BLUE),	    // turquoise
	COLOR(BlueGreen, TEXT_MEDIUM_BLUE),	    // same as medium blue
	COLOR(DarkBlue, TEXT_DARK_BLUE),	    // hard to read
};
#undef COLOR

#define COLOR(STR, IDX) { L#STR, ##IDX## }, { L"\""#STR"\"", ##IDX## } 
static std::unordered_map<std::wstring, uint8_t> COLOR_TO_PALETTE_IDX = {
	COLOR(White, 0x20),
	COLOR(White, 0x20),
	COLOR(Red, 0x0A),
	COLOR(Green, 0x84),
	COLOR(Blue, 0x97),
	COLOR(Gold, 0x0D),
	COLOR(Gray, 0xD0),
	COLOR(Grey, 0xD0),
	COLOR(Black, 0x00),
	COLOR(Tan, 0x5A),
	COLOR(Orange, 0x60),
	COLOR(Yellow, 0x0C),
	COLOR(Purple, 0x9B),
	COLOR(DarkGreen, 0x76),
	COLOR(Coral, 0x66),
	COLOR(Sage, 0x82),
	COLOR(Teal, 0xCB),
	COLOR(LightGray, 0xD6),
	COLOR(LightGrey, 0xD6)
};
#undef COLOR


static std::wstring TokItemLevel(ActionResult* action, Unit* pItem) {
    return std::format(L"{}", GetItemLevel(pItem));
}

static std::wstring TokItemTypeName(ActionResult* action, Unit* pItem) {
    return ItemTypesLookup[static_cast<int32_t>(GetItemsTxt(pItem).dwCode)];
}

static std::wstring TokItemSockets(ActionResult* action, Unit* pItem) {
    return std::to_wstring(GetD2UnitStat(pItem, Stat::ITEM_NUMSOCKETS, 0));
}

static std::wstring TokItemPrice(ActionResult* action, Unit* pItem) {
    return std::to_wstring(GetItemPrice(pItem));
}

static std::wstring TokPotionNumber(ActionResult* action, Unit* pItem)
{
    if (D2COMMON_ITEMS_CheckItemTypeId(pItem, ItemType::HEALING_POTION)
	|| D2COMMON_ITEMS_CheckItemTypeId(pItem, ItemType::MANA_POTION)) {
	return GetItemCode(pItem).substr(2);
    }
    if (D2COMMON_ITEMS_CheckItemTypeId(pItem, ItemType::REJUV_POTION)) {
	std::wstring code = GetItemCode(pItem);
	return code == L"rvl" ? L"2" : L"1";
    }
    if (D2COMMON_ITEMS_CheckItemTypeId(pItem, ItemType::POTION)) {
	return L"1";
    }
    return L"";
}

static std::wstring TokRuneNumber(ActionResult* action, Unit* pItem) {
    if (D2COMMON_ITEMS_CheckItemTypeId(pItem, ItemType::RUNE)) {
	return std::to_wstring(_wtoi(GetItemCode(pItem).substr(1).c_str()));
    }
    return L"";
}

static std::wstring TokItemCode(ActionResult* action, Unit* pItem) {
    return std::to_wstring(GetItemsTxt(pItem).dwCode);
}

static std::wstring TokDefense(ActionResult* action, Unit* pItem) {
    return std::to_wstring(GetD2UnitStat(pItem, Stat::ARMORCLASS, 0));
}

static std::wstring TokAffixLevel(ActionResult* action, Unit* pItem) {
    return std::to_wstring(GetAffixLevel(pItem));
}

static std::wstring TokNewline(ActionResult* action, Unit* pItem) {
    return L"\n";
}


static std::unordered_map<std::wstring, std::wstring> TOKEN_VALUES;

void EvaluteTokenValues(ActionResult *action, Unit* pItem)
{
	typedef std::wstring(*TokenReplaceFunction)(ActionResult* action, Unit* pItem);
	static std::unordered_map<std::wstring, TokenReplaceFunction> TOKEN_REPLACEMENT_FUNCTIONS = {
	{ L"{Sockets}", &TokItemSockets },
	{ L"{Price}", &TokItemPrice },
	{ L"{RuneNumber}", &TokRuneNumber },
	{ L"{PotionNumber}", &TokPotionNumber },

	{ L"{ItemLevel}", &TokItemLevel },
	{ L"{ItemType}", &TokItemTypeName },
	{ L"{ItemTCode}", &TokItemCode },
	{ L"{AffixLevel}", &TokAffixLevel },
	{ L"{NewLine}", &TokNewline } };

	TOKEN_VALUES.clear();
	for (auto& token : TOKEN_REPLACEMENT_FUNCTIONS) {
		TOKEN_VALUES.insert({ token.first, (token.second)(action, pItem) }) ;
	}
}

ColorTextAction::ColorTextAction(ActionType type, std::wstring value) : Action(type, value) {
	for (auto const& color : COLOR_TO_STRING) {
		ireplace(m_Value, color.first, color.second);
	}
}

PaletteIndexAction::PaletteIndexAction(ActionType type, std::wstring value) : Action(type, value ) {
	if (COLOR_TO_PALETTE_IDX.contains(value)) {
		m_PaletteIndex = COLOR_TO_PALETTE_IDX[value];
	}
	else {
		m_PaletteIndex = static_cast<uint8_t>(std::stoi(value, nullptr, 16));
	}
}


void HideAction::SetResult(ActionResult* action, Unit* pItem) {
	action->bHide = true;
}

void ShowAction::SetResult(ActionResult* action, Unit* pItem) {
	action->bHide = false;
}

void ContinueAction::SetResult(ActionResult* action, Unit* pItem) {
	action->bCheck = true;
}

void SetStyleAction::SetResult(ActionResult* action, Unit* pItem) {
	if (GlobalStyles.contains(m_Value)) {
		for (auto act : GlobalStyles[m_Value]) {
			act->SetResult(action, pItem);
		}
	}
}

void SetNameAction::SetResult(ActionResult* action, Unit* pItem) {
	
	//we got here from a CONTINUE
	std::wstring result = m_Value;
	for (const auto& token : TOKEN_VALUES) {
	    if (token.first.compare(TOK_DESC))
		ireplace(result, token.first, token.second);
	}
	TOKEN_VALUES[TOK_NAME] = result;
	action->wsItemName = result;
}

void SetDescriptionAction::SetResult(ActionResult* action, Unit* pItem) {

	//we got here from a CONTINUE
	std::wstring result = m_Value;
	for (const auto& token : TOKEN_VALUES) {
		ireplace(result, token.first, token.second);
	}
	TOKEN_VALUES[TOK_DESC] = result;
	action->wsItemDesc = result;
}

void SetBackgroundColorAction::SetResult(ActionResult* action, Unit* pItem) {
	action->bBackgroundPaletteIndexSet = true;
	action->nBackgroundPaletteIndex = m_PaletteIndex;
}

void SetInventoryColorAction::SetResult(ActionResult* action, Unit* pItem) {
	action->bInvBackgroundPaletteIndexSet = true;
	action->nInvBackgroundPaletteIndex = m_PaletteIndex;
}

void SetBorderColorAction::SetResult(ActionResult* action, Unit* pItem) {
	action->bBorderPaletteIndexSet = true;
	action->nBorderPaletteIndex = m_PaletteIndex;
}

ChatNotifyAction::ChatNotifyAction(std::wstring value) : Action( ActionType::CHAT_NOTIFY, value) {

	std::wstring arg(trim(value));
	if ( !arg.empty() && isdigit(arg[0]) && (arg.length() == 1 || !isdigit(arg[1]))) {
		// arg is single digit number. (no quote) Notify me when ping level is equal or lower than me.
		value = L"{PingLevel} <= ";
		value.push_back(arg[0]);
	}
	m_Expression = Parser::Parse(value.c_str());
}

void ChatNotifyAction::SetResult(ActionResult* action, Unit* pItem) {
	action->bChatAlert = m_Expression->Evaluate(pItem) != 0;
}

void PlayAlertAction::SetResult(ActionResult* action, Unit* pItem) {
}

void MinimapIconAction::SetResult(ActionResult* action, Unit* pItem) {
	action->bMinimapIcon = true;
	action->nMinimapIconPaletteIndex = m_PaletteIndex;
}