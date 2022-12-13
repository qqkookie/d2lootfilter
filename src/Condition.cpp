#include <string>
#include <unordered_map>
#include "Condition.h"
#include "D2Ptrs.h"
#include "Globals.h"
#include "Utils.h"

const wchar_t* CONDITIONS[] = { 
	L"", L"Code", L"Type", L"Class", L"Rarity", L"Ethereal", L"Runeword", L"Rune",
	L"ItemLevel", L"Quality", L"ItemId", L"ItemMode", L"Prefix", L"Suffix",
	L"Stats", L"Identified", L"Sockets", L"Price", L"Gold",  L"Owning",
	L"ItemCat", L"ItemSize", L"AffixLevel", L"WeaponDamage", L"ArmorDefense", L"Defense",
	// L"Armor", L"Weapon", L"Width", L"Height",
	L"Or", L"Difficulty", L"AreaLevel", L"Random",
	L"CharacterClass",    L"CharacterLevel", L"CharacterName", L"CharacterMaxHP", };

static std::unordered_map<std::wstring, int32_t> ItemCodeList;
static std::unordered_map<std::wstring, int32_t> ItemTypeList;
static std::unordered_map<std::wstring, int32_t> RuneList;

void InitTypesCodesRunesList() {

    DataTables* sgptDataTables = *D2COMMON_gpDataTables;

    for (int i = 0; i < D2COMMON_ItemDataTbl->nItemsTxtRecordCount; i++) {
	auto pItemTxt = D2COMMON_ItemDataTbl->pItemsTxt[i];
	std::wstring wNameStr = D2LANG_GetStringFromTblIndex(pItemTxt.wNameStr);
	ItemTypeList[wNameStr] = pItemTxt.dwCode;
	std::wstring wCode = std::wstring(4, L' ');
	mbstowcs(&wCode[0], pItemTxt.szCode, 4);
	wCode = trim(wCode);
	ItemCodeList[wCode] = pItemTxt.dwCode;
	if (pItemTxt.wType[0] == ItemType::RUNE) {
	    int nRuneGrade = std::stoi(std::string(&pItemTxt.szCode[1], 3));
	    RuneList[wNameStr] = nRuneGrade;
	    size_t nFound = wNameStr.find(L" ");
	    if (nFound != std::wstring::npos) {
		RuneList[wNameStr.substr(0, nFound)] = nRuneGrade;
	    }
	}
    }
}

Condition::~Condition()
{
    if (m_Left) delete m_Left;
    if (m_Expression) delete m_Expression;
}

void Condition::Initialize(std::wstring& variables) {
	m_Expression = Parser::Parse(m_Left, m_Value.c_str());
}

std::wstring Condition::ToString(Unit* pItem) {
	return std::format(L"{} {}", CONDITIONS[static_cast<uint8_t>(m_Type)], m_Expression->ToString(pItem));
}

// -------------------------------------------------

void CodeCondition::Initialize(std::wstring& variables) {
	m_Expression = Parser::Parse(m_Left, m_Value.c_str());
	m_Expression->SetValueList(ItemCodeList);
}

bool CodeCondition::Evaluate(Unit* pItem) {
	m_Left->SetValue(GetItemsTxt(pItem).dwCode);
	return m_Expression->Evaluate(pItem);
}

void TypeCondition::Initialize(std::wstring& variables) {
	m_Expression = Parser::Parse(m_Left, m_Value.c_str());
	m_Expression->SetValueList(ItemTypeList);
}

bool TypeCondition::Evaluate(Unit* pItem) {
	m_Left->SetValue(GetItemsTxt(pItem).dwCode);
	return m_Expression->Evaluate(pItem);
}

void ClassCondition::Initialize(std::wstring& variable) {
	m_Expression = new ListExpression();
	m_Expression->Push(Parser::ParseCall(Token::CLASS, m_Value.c_str()));
	m_Expression->SetValueList(ItemTypes);  // from Globals.c
}

bool ClassCondition::Evaluate(Unit* pItem) {
	m_Left->SetValue(static_cast<int32_t>(GetItemsTxt(pItem).dwCode));
	auto rr = m_Expression->Evaluate(pItem);
	if (rr)
	    return rr;
        auto ty2 = GetItemsTxt(pItem).wType;
	if (ty2[0] == ItemType::NONE_1)
	    return false;
	m_Left->SetValue(static_cast<int32_t>(ty2[0]));
	rr = m_Expression->Evaluate(pItem);
	if (rr)
	    return rr;
	if (ty2[1] == ItemType::NONE_1)
	    return false;
	m_Left->SetValue(static_cast<int32_t>(ty2[1]));
	return(m_Expression->Evaluate(pItem));
}

void RarityCondition::Initialize(std::wstring& variable) {
    m_Expression = Parser::Parse(m_Left, m_Value.c_str());
    m_Expression->SetValueList(RaritiesFixed);
}

bool RarityCondition::Evaluate(Unit* pItem) {
	// m_Left->SetValue(static_cast<int32_t>(pItem->pItemData->dwRarity));
        m_Left->SetValue(static_cast<int32_t>(RarityFix(pItem)));
	return m_Expression->Evaluate(pItem);
}

ItemRarity RarityCondition::RarityFix(Unit* pItem) {
    ItemRarity ir = pItem->pItemData->dwRarity;
    // Swap numberical order of Set/Rare rarity
    if (ir == ItemRarity::RARE)
	ir = ItemRarity::SET;
    else if (ir == ItemRarity::SET)
	ir = ItemRarity::RARE;
    // Only normal weapons or armors have real NORMAL rarity. Other normals like potion is NONE rarity.
    else if (ir == ItemRarity::NORMAL) {
	if (D2COMMON_ITEMS_CheckItemTypeId(pItem, ItemType::MISSILE_POTION)
	    || (! D2COMMON_ITEMS_CheckItemTypeId(pItem, ItemType::WEAPON)
	        && ! D2COMMON_ITEMS_CheckItemTypeId(pItem, ItemType::ANY_ARMOR)))
	    ir = ItemRarity::NONE;
    }
    return(ir);
}

bool EtherealCondition::Evaluate(Unit* pItem) {
	m_Left->SetValue((pItem->pItemData->dwItemFlags & ItemFlags::ETHEREAL) == ItemFlags::ETHEREAL);
	return m_Expression->Evaluate(pItem);
}

bool RunewordCondition::Evaluate(Unit* pItem) {
	m_Left->SetValue((pItem->pItemData->dwItemFlags & ItemFlags::RUNEWORD) == ItemFlags::RUNEWORD);
	return m_Expression->Evaluate(pItem);
}

void RuneCondition::Initialize(std::wstring& variable) {
	m_Expression = Parser::Parse(m_Left, m_Value.c_str());
	m_Expression->SetValueList(RuneList);
}

bool RuneCondition::Evaluate(Unit* pItem) {
    if (!D2COMMON_ITEMS_CheckItemTypeId(pItem, ItemType::RUNE)) {
	return false;
    }
    int nRuneGrade = std::stoi(std::string(&GetItemsTxt(pItem).szCode[1], 3));
    m_Left->SetValue(nRuneGrade);
    return m_Expression->Evaluate(pItem);
}

bool ItemLevelCondition::Evaluate(Unit* pItem) {
    m_Left->SetValue(pItem->pItemData->dwItemLevel);
    return m_Expression->Evaluate(pItem);
}

void QualityCondition::Initialize(std::wstring& variable) {
	m_Expression = Parser::Parse(m_Left, m_Value.c_str());
	m_Expression->SetValueList(Qualities);
}

bool QualityCondition::Evaluate(Unit* pItem) {
    m_Left->SetValue(GetQualityIndex(pItem));
    return m_Expression->Evaluate(pItem);
}

bool ItemIdCondition::Evaluate(Unit* pItem) {
	m_Left->SetValue(pItem->dwLineId);
	return m_Expression->Evaluate(pItem);
}

void ItemModeCondition::Initialize(std::wstring& variables) {
	m_Expression = Parser::Parse(m_Left, m_Value.c_str());
	m_Expression->SetValueList(ItemModeList);
}

bool ItemModeCondition::Evaluate(Unit* pItem) {
    m_Left->SetValue(static_cast<int32_t>(GetItemMode(pItem)));
    return m_Expression->Evaluate(pItem);
}

bool PrefixCondition::Evaluate(Unit* pItem) {
    uint8_t isIdentified = (pItem->pItemData->dwItemFlags & ItemFlags::IDENTIFIED) == ItemFlags::IDENTIFIED;
    if (!isIdentified) {
	return false;
    }
    for (auto& prefix : pItem->pItemData->wMagicPrefix) {
	m_Left->SetValue(prefix);
	if (m_Expression->Evaluate(pItem)) {
	    return true;
	}
    }
    return false;
}

bool SuffixCondition::Evaluate(Unit* pItem) {
    uint8_t isIdentified = (pItem->pItemData->dwItemFlags & ItemFlags::IDENTIFIED) == ItemFlags::IDENTIFIED;
    if (!isIdentified) {
	return false;
    }
    for (auto& prefix : pItem->pItemData->wMagicSuffix) {
	m_Left->SetValue(prefix);
	if (m_Expression->Evaluate(pItem)) {
	    return true;
	}
    }
    return false;
}

// void StatsCondition::Initialize(std::unordered_map<std::wstring, int32_t> variables) {
void StatsCondition::Initialize(std::wstring& variables) {
    for (auto& stat : CustomStats) {
	replace(m_Value, stat.first, stat.second);
    }
    m_Expression = new ListExpression();
    m_Expression->Push(Parser::Parse(m_Value.c_str()));
}
bool StatsCondition::Evaluate(Unit* pItem) {
    return m_Expression->Evaluate(pItem);
}

bool IdentifiedCondition::Evaluate(Unit* pItem) {
    m_Left->SetValue((pItem->pItemData->dwItemFlags & ItemFlags::IDENTIFIED) == ItemFlags::IDENTIFIED);
    return m_Expression->Evaluate(pItem);
}

bool SocketsCondition::Evaluate(Unit* pItem) {
    m_Left->SetValue(GetD2UnitStat(pItem, Stat::ITEM_NUMSOCKETS, 0));
    return m_Expression->Evaluate(pItem);
}

bool PriceCondition::Evaluate(Unit* pItem) {
    m_Left->SetValue(GetItemPrice(pItem));
    return m_Expression->Evaluate(pItem);
}

bool GoldCondition::Evaluate(Unit* pItem) {
	if (!D2COMMON_ITEMS_CheckItemTypeId(pItem, ItemType::GOLD)) {
		return false;
	}
	m_Left->SetValue(GetD2UnitStat(pItem, Stat::GOLD, 0));
	return m_Expression->Evaluate(pItem);
}

bool OwningCondition::Evaluate(Unit* pItem) {
	//only check set/unique items for duplicates
	if (pItem->pItemData->dwRarity != ItemRarity::SET
		&& pItem->pItemData->dwRarity != ItemRarity::UNIQUE) {
		return false;
	}

	Unit* pPlayer = D2CLIENT_GetPlayerUnit();
	if (!pPlayer || !pPlayer->pInventory) {
		return false;
	}

	int32_t unitId = pItem->dwUnitId;
	int32_t fileIndex = pItem->pItemData->dwFileIndex;
	ItemRarity rarity = pItem->pItemData->dwRarity;
	int value = 0;
	
	for (int i = 0; i < 128; i++) {
		Unit* pOtherItem = FindUnitFromTable(i, UnitType::ITEM);
		while (pOtherItem) {
			if (pOtherItem->pItemData->dwRarity != ItemRarity::SET
				&& pOtherItem->pItemData->dwRarity != ItemRarity::UNIQUE) {
				pOtherItem = pOtherItem->pRoomNext;
				continue;
			}
			if (fileIndex == pOtherItem->pItemData->dwFileIndex
				&& pItem->pItemData->dwRarity == pOtherItem->pItemData->dwRarity
				&& unitId != pOtherItem->dwUnitId) {
				value = 1;
				break;
			}
			pOtherItem = pOtherItem->pRoomNext;
		}
		if (value == 1) {
			break;
		}
	}

	m_Left->SetValue(value);
	return m_Expression->Evaluate(pItem);
}


// Item Category: { Weapon, Armor, Accessory, Socketable, Consumable, Misc }
// Charm or Ring is Accessory. Gem, Jewel, Rune is Socketable. Throwing potion or Arrow is Consumable. Quest item is Misc. 
void ItemCatCondition::Initialize(std::wstring& variables) {
    m_Expression = Parser::Parse(m_Left, m_Value.c_str());
    m_Expression->SetValueList(ItemCats);
}

bool ItemCatCondition::Evaluate(Unit* pItem) {

    ItemType value = ItemType::NONE_1;
    if (D2COMMON_ITEMS_CheckItemTypeId(pItem, ItemType::MISSILE_POTION))
	value = ItemType::POTION;
    else if (D2COMMON_ITEMS_CheckItemTypeId(pItem, ItemType::WEAPON))
	value = ItemType::WEAPON;
    else if (D2COMMON_ITEMS_CheckItemTypeId(pItem, ItemType::SOCKET_FILLER))
	value = ItemType::SOCKET_FILLER;
    else if (D2COMMON_ITEMS_CheckItemTypeId(pItem, ItemType::RING)
	|| D2COMMON_ITEMS_CheckItemTypeId(pItem, ItemType::AMULET)
	|| D2COMMON_ITEMS_CheckItemTypeId(pItem, ItemType::CHARM))
	value = ItemType::RING;
    else if (D2COMMON_ITEMS_CheckItemTypeId(pItem, ItemType::ANY_ARMOR))
	value = ItemType::ARMOR;
    else if (D2COMMON_ITEMS_CheckItemTypeId(pItem, ItemType::POTION)
	|| D2COMMON_ITEMS_CheckItemTypeId(pItem, ItemType::SCROLL)
	|| D2COMMON_ITEMS_CheckItemTypeId(pItem, ItemType::BOOK)
	|| D2COMMON_ITEMS_CheckItemTypeId(pItem, ItemType::KEY)
	|| D2COMMON_ITEMS_CheckItemTypeId(pItem, ItemType::MISSILE))
	value = ItemType::POTION;
    else
	value = ItemType::QUEST;

    m_Left->SetValue(static_cast<int32_t>(value));

    return m_Expression->Evaluate(pItem);
}

bool ItemSizeCondition::Evaluate(Unit* pItem) {
	ItemsTxt tt = GetItemsTxt(pItem);
	m_Left->SetValue(tt.nInvWidth * tt.nInvHeight);
	return m_Expression->Evaluate(pItem);
}

bool AffixLevelCondition::Evaluate(Unit* pItem) {
    m_Left->SetValue(GetAffixLevel(pItem));
    return m_Expression->Evaluate(pItem);
}

bool WeaponDamageCondition::Evaluate(Unit* pItem) {
	m_Left->SetValue(GetWeaponDamage(pItem));
	return m_Expression->Evaluate(pItem);
}

bool ArmorDefenseCondition::Evaluate(Unit* pItem) {
	m_Left->SetValue((GetD2UnitStat(pItem, Stat::ARMORCLASS, 0) * 
		GetD2UnitStat(pItem, Stat::ITEM_ARMOR_PERCENT, 0) +50)/100);
	return m_Expression->Evaluate(pItem);
}

bool DefenseCondition::Evaluate(Unit* pItem) {
	m_Left->SetValue(GetD2UnitStat(pItem, Stat::ARMORCLASS, 0));
	return m_Expression->Evaluate(pItem);
}

void DifficultyCondition::Initialize(std::wstring& variable) {
    m_Expression = Parser::Parse(m_Left, m_Value.c_str());
    m_Expression->SetValueList(Difficulties);
}

bool DifficultyCondition::Evaluate(Unit* pItem) {

    int d = D2CLIENT_GetDifficulty();	    // 0-2
    int a = D2CLIENT_GetPlayerUnit()->dwAct;    // 0-4

    m_Left->SetValue(a + 1);
    if (m_Expression->Evaluate(pItem))
	return true;
    m_Left->SetValue(d * 10 + 10);
    if (m_Expression->Evaluate(pItem))
	return true;
    m_Left->SetValue(d * 10 + a + 11);
    return (m_Expression->Evaluate(pItem));
}

bool AreaLevelCondition::Evaluate(Unit* pItem) {
    return false;
}

bool RandomCondition::Evaluate(Unit* pItem) {
    m_Left->SetValue(rand() % 100);	//random between 0-99
    return m_Expression->Evaluate(pItem);
}

void CharacterClassCondition::Initialize(std::wstring& variables) 
{
	m_Expression = Parser::Parse(m_Left, m_Value.c_str());

    std::unordered_map<std::wstring, int32_t> list;
    DataTables* sgptDataTables = *D2COMMON_gpDataTables;
    for (int32_t i = 0; i < sgptDataTables->nCharStatsTxtRecordCount; i++) {
	CharStatsTxt charStats = sgptDataTables->pCharStatsTxt[i];
	std::wstring className = std::wstring(charStats.wszClassName);
	className = trim(className);
	list[className] = i;
    }
    m_Expression->SetValueList(list);
}

bool CharacterClassCondition::Evaluate(Unit* pItem) {
    m_Left->SetValue(GetCharacterClass());
    return m_Expression->Evaluate(pItem);
}

bool CharacterLevelCondition::Evaluate(Unit* pItem) {
    m_Left->SetValue(GetCharacterLevel());
    return m_Expression->Evaluate(pItem);
}


bool CharacterNameCondition::Evaluate(Unit* pItem) {
    m_Left = GetCharacterName();
    return ListHave(m_Value, m_Left, L" \t\n,.;");
}

bool CharacterMaxHPCondition::Evaluate(Unit* pItem) {
    m_Left->SetValue(GetD2UnitStat(D2CLIENT_GetPlayerUnit(), Stat::MAXHP, 0));
    return m_Expression->Evaluate(pItem);
}

/*
#include <vector>
#include <algorithm>
#include <regex>

std::vector<std::wstring> rx_split(const std::wstring& str, const std::wstring& delim_regex)
{
    static std::vector<std::wstring> result;
    const std::wregex delims(delim_regex);

    result.clear();
    std::wsregex_token_iterator iter(str.begin(), str.end(), delims, -1);
    for (std::wsregex_token_iterator end; iter != end; ++iter)
	result.push_back(iter->str());

    return result;
}

bool CharacterNameCondition::Evaluate(Unit* pItem) {
    m_Left = GetCharacterName();
    auto names = rx_split(m_Value, L",|\\s+");
    for (std::wstring name : names)
	if ( name.compare(m_Left) == 0)
	    return true;
    return false;
}
*/
