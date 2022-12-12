#pragma once

#include "Expression.h"

//to be able to access conditions by type to set variables and such
enum class ConditionType : uint8_t {
	NONE, CODE, TYPE, CLASS, RARITY, ETHEREAL, RUNEWORD, RUNE,
	ITEM_LEVEL, QUALITY, ITEM_ID, ITEM_MODE, PREFIX, SUFFIX,
	STATS, IDENTIFIED, SOCKETS, PRICE, GOLD, OWNING,
	ITEM_CAT, ITEM_SIZE, AFFIX_LEVEL, WEAPON_DAMAGE, ARMOR_DEFENSE,
	// ARMOR, WEAPON, WIDTH, HEIGHT,
	OR, DIFFICULTY, AREA_LEVEL, RANDOM,
	CHARACTER_CLASS, CHARACTER_LEVEL, CHARACTER_NAME, CHARACTER_MAXHP, 
};

extern const wchar_t* CONDITIONS[]; 
/*
=     { L"", L"Code", L"Type", L"Class", L"Rarity", L"Ethereal", L"Runeword", L"Rune",
	L"ItemLevel", L"Quality", L"ItemId", L"ItemMode", L"Prefix", L"Suffix",
	L"Stats", L"Identified", L"Sockets", L"Price", L"Gold",  L"Owning",
	L"ItemCat", L"ItemSize", L"AffixLevel", L"WeaponDamage", L"ArmorDefense", L"Defense",
	// L"Armor", L"Weapon", L"Width", L"Height",
	L"Or", L"Difficulty", L"AreaLevel", L"Random",
	L"CharacterClass",    L"CharacterLevel", L"CharacterName", L"CharacterMaxHP", };
*/

extern void InitTypesCodesRunesList();

class Condition {
    protected:
	ConditionType m_Type;
	std::wstring m_Value;
	Variable* m_Left;
	ListExpression* m_Expression;
    public:
	Condition(ConditionType type, std::wstring value) : m_Type(type), m_Value(value),
		    m_Left(new Variable(L"")), m_Expression(nullptr) {};
	~Condition();

	ConditionType GetType() { return m_Type; }
	virtual void Initialize(std::wstring& variables);
	virtual bool Evaluate(Unit* pItem) = 0;
	virtual std::wstring ToString(Unit* pItem);
};

class CodeCondition : public Condition {
    public:
	CodeCondition(std::wstring value) : Condition(ConditionType::CODE, value) {};
	void Initialize(std::wstring& variables) override;
	bool Evaluate(Unit* pItem) override;	
};

class TypeCondition : public Condition {
    public:
	TypeCondition(std::wstring value) : Condition(ConditionType::TYPE, value) {};
	void Initialize(std::wstring& variables) override;
	bool Evaluate(Unit* pItem) override;	
};

class ClassCondition : public Condition {
    public:
	ClassCondition(std::wstring value) : Condition(ConditionType::CLASS, value) {};
	void Initialize(std::wstring& variables) override;
	bool Evaluate(Unit* pItem) override;
};

class RarityCondition : public Condition {
    protected:
	ItemRarity RarityFix(Unit* pItem);
    public:
	RarityCondition(std::wstring value) : Condition(ConditionType::RARITY, value) {};
	void Initialize(std::wstring& variables) override;
	bool Evaluate(Unit* pItem) override;
};

class EtherealCondition : public Condition {
    public:
	EtherealCondition(std::wstring value) : Condition(ConditionType::ETHEREAL, value) {};
	bool Evaluate(Unit* pItem) override;
};

class RunewordCondition : public Condition {
    public:
	RunewordCondition(std::wstring value) : Condition(ConditionType::RUNEWORD, value) {};
	bool Evaluate(Unit* pItem) override;
};

class RuneCondition : public Condition {
    public:
	RuneCondition(std::wstring value) : Condition(ConditionType::RUNE, value) {};
	void Initialize(std::wstring& variables) override;
	bool Evaluate(Unit* pItem) override;
};

class ItemLevelCondition : public Condition {
    public:
	ItemLevelCondition(std::wstring value) : Condition(ConditionType::ITEM_LEVEL, value) {};
	bool Evaluate(Unit* pItem) override;
};

class QualityCondition : public Condition {
    public:
	QualityCondition(std::wstring value) : Condition(ConditionType::QUALITY, value) {};
	void Initialize(std::wstring& variables) override;
	bool Evaluate(Unit* pItem) override;	
};

class ItemIdCondition : public Condition {
    public:
	ItemIdCondition(std::wstring value) : Condition(ConditionType::ITEM_ID, value) {};
	bool Evaluate(Unit* pItem) override;	
};

class ItemModeCondition : public Condition {
    public:
	ItemModeCondition(std::wstring value) : Condition(ConditionType::ITEM_MODE, value) {};
	void Initialize(std::wstring& variables) override;
	bool Evaluate(Unit* pItem) override;
};

class PrefixCondition : public Condition {
    public:
	PrefixCondition(std::wstring value) : Condition(ConditionType::PREFIX, value) {};
	bool Evaluate(Unit* pItem) override;
};

class SuffixCondition : public Condition {
    public:
	SuffixCondition(std::wstring value) : Condition(ConditionType::SUFFIX, value) {};
	bool Evaluate(Unit* pItem) override;
};

class StatsCondition : public Condition {
    public:
	StatsCondition(std::wstring value): Condition(ConditionType::STATS, value) {};
	void Initialize(std::wstring& variables) override;
	bool Evaluate(Unit* pItem) override;
};

class IdentifiedCondition : public Condition {
    public:
	IdentifiedCondition(std::wstring value) : Condition(ConditionType::IDENTIFIED, value) {};
	bool Evaluate(Unit* pItem) override;
};

class SocketsCondition : public Condition {
    public:
	SocketsCondition(std::wstring value) : Condition(ConditionType::SOCKETS, value) {};
	bool Evaluate(Unit* pItem) override;
};

class PriceCondition : public Condition {
    public:
	PriceCondition(std::wstring value) : Condition(ConditionType::PRICE, value) {};
	bool Evaluate(Unit* pItem) override;
};

class GoldCondition : public Condition {
    public:
	GoldCondition(std::wstring value) : Condition(ConditionType::GOLD, value) {};
	bool Evaluate(Unit* pItem) override;
};

class OwningCondition : public Condition {
    public:
	OwningCondition(std::wstring value) : Condition(ConditionType::OWNING, value) {};
	bool Evaluate(Unit* pItem) override;
};

class ItemCatCondition : public Condition {
    public:
	ItemCatCondition(std::wstring value) : Condition(ConditionType::ITEM_CAT, value) {};
	void Initialize(std::wstring& variables) override;
	bool Evaluate(Unit* pItem) override;
};

class ItemSizeCondition : public Condition {
    public:
	ItemSizeCondition(std::wstring value) : Condition(ConditionType::ITEM_SIZE, value) {};
	bool Evaluate(Unit* pItem) override;
};

class AffixLevelCondition : public Condition {
    public:
	AffixLevelCondition(std::wstring value) : Condition(ConditionType::AFFIX_LEVEL, value) {};
	bool Evaluate(Unit* pItem) override;
};

class WeaponDamageCondition : public Condition {
    public:
	WeaponDamageCondition(std::wstring value) : Condition(ConditionType::WEAPON_DAMAGE, value) {};
	bool Evaluate(Unit* pItem) override;
};

class ArmorDefenseCondition : public Condition {
    public:
	ArmorDefenseCondition(std::wstring value) : Condition(ConditionType::ARMOR_DEFENSE, value) {};
	bool Evaluate(Unit* pItem) override;
};

class DefenseCondition : public Condition {
    public:
	DefenseCondition(std::wstring value) : Condition(ConditionType::ARMOR_DEFENSE, value) {};
	bool Evaluate(Unit* pItem) override;
};

class OrCondition : public Condition {
    public:
	OrCondition(std::wstring value) : Condition(ConditionType::OR, value) {};
	bool Evaluate(Unit* pItem) override { return true; };
};

class DifficultyCondition : public Condition {
    public:
	DifficultyCondition(std::wstring value) : Condition(ConditionType::DIFFICULTY, value) {};
	void Initialize(std::wstring& variables) override;
	bool Evaluate(Unit* pItem) override;
};

class AreaLevelCondition : public Condition {
    public:
	AreaLevelCondition(std::wstring value) : Condition(ConditionType::AREA_LEVEL, value) {};
	bool Evaluate(Unit* pItem) override;
};

class RandomCondition : public Condition {
    public:
	RandomCondition(std::wstring value) : Condition(ConditionType::RANDOM, value) {};
	bool Evaluate(Unit* pItem) override;
};

class CharacterClassCondition : public Condition {
    public:
	CharacterClassCondition(std::wstring value) : Condition(ConditionType::CHARACTER_CLASS, value) {};
	void Initialize(std::wstring& variables) override;
	bool Evaluate(Unit* pItem) override;
};

class CharacterLevelCondition : public Condition {;
    public:
	CharacterLevelCondition(std::wstring value) : Condition(ConditionType::CHARACTER_LEVEL, value) {};
	bool Evaluate(Unit* pItem) override;
};

class CharacterNameCondition : public Condition {
    protected:
	std::wstring m_Left;

    public:
	CharacterNameCondition(std::wstring value) : Condition(ConditionType::CHARACTER_NAME, value) {};
	bool Evaluate(Unit* pItem) override;
};

class CharacterMaxHPCondition : public Condition {
    public:
	CharacterMaxHPCondition(std::wstring value) : Condition(ConditionType::CHARACTER_MAXHP, value) {};
	bool Evaluate(Unit* pItem) override;
};
