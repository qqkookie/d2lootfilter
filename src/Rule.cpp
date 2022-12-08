#include <string>
#include "Rule.h"

bool Rule::Evaluate(Unit* pItem) {
	bool bResult = true;
	for (auto& condition : m_Conditions) {
		if ( !bResult && (condition->GetType() != ConditionType::OR ))
		    break;
		bResult = condition->Evaluate(pItem);
	}
	return bResult;
}

void Rule::EvaluateActionResult(ActionResult* pActionResult, Unit* pItem) {
	pActionResult->bCheck = false;
	for (auto& action : m_Actions) {
		action->SetResult(pActionResult, pItem);
	}
}

uint32_t Rule::GetLineNumber() {
	return m_LineNumber;
}

void Rule::SetLineNumber(uint32_t lineNumber) {
	m_LineNumber = lineNumber;
}

std::vector<Condition*> Rule::GetConditions() {
	return m_Conditions;
}

std::vector<Action*> Rule::GetActions() {
	return m_Actions;
}

void Rule::AddAction(Action* pAction, int32_t idx) {
	m_Actions.insert(m_Actions.begin() + idx, pAction);
}

void Rule::AddActions(std::vector<Action*> actions) {
	m_Actions.insert(m_Actions.end(), actions.begin(), actions.end());
}

void Rule::AddCondition(Condition* pCondition) {
	m_Conditions.push_back(pCondition);
}

void Rule::AddConditions(std::vector<Condition*> conditions) {
	m_Conditions.insert(m_Conditions.end(), conditions.begin(), conditions.end());
}