#pragma once

#include "D2Structs.h"
#include "Expression.h"

struct ActionResult {
	std::vector<uint32_t> vMatchedRules;

	bool bHide = false;
	bool bCheck = false;

	bool bBackgroundPaletteIndexSet = false;
	uint8_t nBackgroundPaletteIndex = 0;
	DrawMode eDrawModeAlt = DrawMode::TRANS_25;
	DrawMode eDrawModeHover = DrawMode::TRANS_50;

	bool bBorderPaletteIndexSet = false;
	uint8_t nBorderPaletteIndex = 0;

	bool bInvBackgroundPaletteIndexSet = false;
	uint8_t nInvBackgroundPaletteIndex = 0;

	bool bChatAlert = false;

	std::wstring wsItemName = L"";
	std::wstring wsItemDesc = L"";

	bool bMinimapIcon = false;
	uint8_t nMinimapIconPaletteIndex = 0;
};

enum class ActionType : uint8_t {
	NONE, SHOW, HIDE, CONTINUE, SET_STYLE, SET_NAME,
	SET_DESCRIPTION, SET_BG_COLOR, SET_INVENTORY_COLOR,
	SET_BORDER_COLOR, CHAT_NOTIFY, PLAY_ALERT, MINIMAP_ICON
};

extern void EvaluteTokenValues(ActionResult* action, Unit* pItem);

#define TOK_NAME    L"{Name}"
#define TOK_DESC    L"{Description}"

class Action {
    protected:
	ActionType m_Type;
	std::wstring m_Value;
    public:
	Action( ActionType type, std::wstring value) : m_Type(type), m_Value(value) {};
	ActionType GetType() { return m_Type; }
	virtual void SetResult(ActionResult* pResult, Unit* pItem) = 0;
};

class ShowAction : public Action {
    public:
	ShowAction() : Action(ActionType::SHOW, L"Show") {};
	void SetResult(ActionResult* pResult, Unit* pItem) override;
};

class HideAction : public Action {
    public:
	HideAction() : Action(ActionType::HIDE, L"Hide") {};
	void SetResult(ActionResult* pResult, Unit* pItem) override;
};

class ContinueAction : public Action {
    public:
	ContinueAction(std::wstring value) : Action( ActionType::CONTINUE, value) { };
	void SetResult(ActionResult* pResult, Unit* pItem) override;
};

class ColorTextAction : public Action {
    public:
	ColorTextAction(ActionType type, std::wstring value);
	virtual void SetResult(ActionResult* pResult, Unit* pItem) = 0;
};

class PaletteIndexAction : public Action {
    protected:
	uint8_t m_PaletteIndex = 0;
    public:
	PaletteIndexAction(ActionType type, std::wstring value );
	virtual void SetResult(ActionResult* pResult, Unit* pItem) = 0;
};

class SetStyleAction : public Action {
    public:
	SetStyleAction(std::wstring value) : Action(ActionType::SET_STYLE, value) {};
	void SetResult(ActionResult* pResult, Unit* pItem) override;
};

class SetNameAction : public ColorTextAction {
    public:
	SetNameAction(std::wstring value) : ColorTextAction(ActionType::SET_NAME, value) {};
	void SetResult(ActionResult* pResult, Unit* pItem) override;
};

class SetDescriptionAction : public ColorTextAction {
    public:
	SetDescriptionAction(std::wstring value) : ColorTextAction(ActionType::SET_DESCRIPTION, value) {};
	void SetResult(ActionResult* pResult, Unit* pItem) override;
};

class SetBackgroundColorAction : public PaletteIndexAction {
    public:
	SetBackgroundColorAction(std::wstring value) : PaletteIndexAction( ActionType::SET_BORDER_COLOR, value) {};
	void SetResult(ActionResult* pResult, Unit* pItem) override;
};

class SetInventoryColorAction : public PaletteIndexAction {
    public:
	SetInventoryColorAction(std::wstring value) : PaletteIndexAction(ActionType::SET_INVENTORY_COLOR, value) {};
	void SetResult(ActionResult* pResult, Unit* pItem) override;
};

class SetBorderColorAction : public PaletteIndexAction {
    public:
	SetBorderColorAction(std::wstring value) : PaletteIndexAction( ActionType::SET_BORDER_COLOR, value) {};
	void SetResult(ActionResult* pResult, Unit* pItem) override;
};

class ChatNotifyAction : public Action {
    protected:
	Expression* m_Expression;
    public:
	ChatNotifyAction(std::wstring value);
	void SetResult(ActionResult* pResult, Unit* pItem) override;
};

class PlayAlertAction : public Action {
    public:
	PlayAlertAction(std::wstring value) : Action(ActionType::PLAY_ALERT, value) {};
	void SetResult(ActionResult* pResult, Unit* pItem) override;
};

class MinimapIconAction : public PaletteIndexAction {
    public:
	MinimapIconAction(std::wstring value) : PaletteIndexAction(ActionType::MINIMAP_ICON, value) {};
	void SetResult(ActionResult* pResult, Unit* pItem) override;
};