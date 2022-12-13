#pragma once

#include <unordered_map>
#include <vector>
#include <optional>
#include "D2Structs.h"


/*
expression     -> or ;
or             -> and ( ( "or" | "||" ) and )* ;
and            -> equality ( ( "and" | "&&" ) equality )* ;
equality       -> comparison ( ( "!=" | "==" ) comparison )* ;
comparison     -> term ( ( ">" | ">=" | "<" | "<=" ) term )* ;
term           -> factor ( ( "-" | "+" ) factor )* ;
factor         -> unary ( ( "/" | "*" ) unary )* ;
unary          -> ( "!" | "-" | call ) unary
			   | primary ;
primary        -> NUMBER | VARIABLE | "true" | "false" | "nil"
			   | "(" expression ")" ;
call           -> "(" expression ")" ;
*/

enum class Token : uint8_t {
	NONE,
	OR, AND,

	LEFT_PAREN, RIGHT_PAREN,
	BANG, BANG_EQUALS,

	EQUALS,
	GREATER_THAN, GREATER_THAN_EQUALS,
	LESS_THAN, LESS_THAN_EQUALS,
	_IN,

	MINUS, PLUS,
	MULTIPLY, DIVIDE,

	//funcs
	STAT, CLASS,
	TABSKILL, CLASSSKILL, CHARGEDSKILL, SKILL,
	MIN, MININ, MAX,
	COMMA,

	NUMBER, VARIABLE,
	_TRUE, _FALSE,

	END
};

static const wchar_t* OPS[] = { 
	L"", L" or ", L" and ", L"(", L")", L"!", L"!=", L"=", 
	L">", L">=", L"<", L"<=", L" in ", L"-", L"+", L"*", L"/", 
	L"Stat", L"Class", L"TabSkill", L"ClassSkill", L"ChargedSkill", L"Skill", 
	L"Min", L"MinIn", L"Max", L",", L"", L"", L"True", L"False", L""
};

class Expression {		// pure virtual type
    public:
		virtual int32_t Evaluate(Unit* pItem) = 0;
		virtual void SetVariables(std::unordered_map<std::wstring, int32_t>& variables) = 0;
		virtual std::wstring ToString(Unit* pItem) = 0;
};

class Binary : public Expression {
    protected:
		Token m_Operator;
		Expression* m_Left;
		Expression* m_Right;
    public:
		Binary(Token op, Expression *left, Expression *right) 
			: m_Operator(op), m_Left(left), m_Right(right) {};
		int32_t Evaluate(Unit* pItem) override;
		void SetVariables(std::unordered_map<std::wstring, int32_t>& variables) override;
		std::wstring ToString(Unit* pItem) override;
};

class Unary : public Binary {
	public:
		Unary(Token op, Expression* right) : Binary(op, right /*dummy*/, right) {};
		int32_t Evaluate(Unit* pItem) override;
		std::wstring ToString(Unit* pItem) override;
};

class Logical : public Binary {
	public:
		Logical(Token op, Expression* left, Expression* right) : Binary(op, left, right) {};
		int32_t Evaluate(Unit* pItem) override;
};

class In : public Binary {
	protected:
		Expression* m_Min;
		Expression* m_Max;
	public:
		In(Token op, Expression* left, Expression* min, Expression* max) :
			Binary(op, left, left/*dummy*/), m_Min(min), m_Max(max) {};
		int32_t Evaluate(Unit* pItem) override;
		std::wstring ToString(Unit* pItem) override;
};

class Literal : public Expression {
	protected:
		int32_t m_Value;
	public:
		Literal(int32_t value) : m_Value(value) {};
		int32_t Evaluate(Unit* pItem) override { return m_Value; };
		void SetVariables(std::unordered_map<std::wstring, int32_t>& variables) {};
		std::wstring ToString(Unit* pItem) override;
};

class Boolean : public Literal {
	public:
		Boolean(int32_t value) : Literal(value) {};
};

typedef int32_t(*GlobalEvalFunction)(Unit* pUnit);

class Variable : public Expression {
	protected:
		std::wstring m_Name;
		std::optional<int32_t> m_Value;
		GlobalEvalFunction m_Eval;
	public:
		Variable(std::wstring variable);
		void SetValue(const int32_t v);
		int32_t Evaluate(Unit* pItem) override;
		void SetVariables(std::unordered_map<std::wstring, int32_t>& variables) override;
		std::wstring ToString(Unit* pItem) override;
};

class ListExpression : public Expression {
	protected:
		std::vector<Expression*> m_List;
	public:
		void Push(Expression* exp) { m_List.push_back(exp); }
		int32_t Evaluate(Unit* pItem) override;
		void SetVariables(std::unordered_map<std::wstring, int32_t>& variables) override;
		std::wstring ToString(Unit* pItem) override;
		void SetValueList(std::unordered_map<std::wstring, int32_t>& variables) {
			SetVariables(variables); };
};

class FuncCall : public Expression {
	protected:
		Token m_Func;
		std::vector<Expression*> m_Args;

		int32_t EvaluateClass(Unit* pItem, std::vector<int32_t>& args);
		int32_t EvaluateChargedSkill(Unit* pItem, Stat stat, std::vector<int32_t>& args);
		int32_t EvaluateStat(Unit* pItem, Stat stat, std::vector<int32_t>& args);
	public:
		FuncCall(Token func, std::vector<Expression*> args) : m_Func(func), m_Args(args) {};
		int32_t Evaluate(Unit* pItem) override;
		void SetVariables(std::unordered_map<std::wstring, int32_t>& variables) override;
		std::wstring ToString(Unit* pItem) override;
};

// -----------------------------------------------------------------

class TokenizerToken {
	protected:
		Token m_TokenType;
		std::wstring m_Value;
	public:
		TokenizerToken(Token type, std::wstring value = L"") : m_TokenType(type), m_Value(value) {};
		Token GetTokenType() { return m_TokenType; }
		std::wstring GetValue() { return m_Value; }
};

//https://github.com/munificent/craftinginterpreters/blob/master/java/com/craftinginterpreters/lox/Scanner.java
class Tokenizer {
	protected:
		unsigned m_Current = 0;
		std::vector<TokenizerToken*> m_Tokens;

		bool IsNull(wchar_t c);
		bool IsOperator(const wchar_t*& expression);
		std::wstring ParseVariable(const wchar_t*& expression);
		std::wstring ParseQuotedVariable(const wchar_t*& expression);
		std::wstring ParseDigit(const wchar_t*& expression);
		void Tokenize(const wchar_t*& expression);
	public:
		Tokenizer(const wchar_t* expression) {
			Tokenize(expression);
		};

		std::vector<TokenizerToken*> GetTokens() { return m_Tokens; }
		bool Match(Token t);
		bool Check(Token t);
		TokenizerToken* Previous();
		TokenizerToken* Peek();
		void Reset();
		void Insert(TokenizerToken* t);
};

//https://en.wikipedia.org/wiki/Recursive_descent_parser
class Parser {
	protected:
		static Expression* _finishCall(Token call, Tokenizer* t);
		static Expression* _primary(Tokenizer* t, Expression* lhs);
		static Expression* _unary(Tokenizer* t, Expression* lhs);
		static Expression* _factor(Tokenizer* t, Expression* lhs);
		static Expression* _term(Tokenizer* t, Expression* lhs);
		static Expression* _comparison(Tokenizer* t, Expression* lhs);
		static Expression* _equality(Tokenizer* t, Expression* lhs);
		static Expression* _and(Tokenizer* t, Expression* lhs);
		static Expression* _or(Tokenizer* t, Expression* lhs);
		static Expression* _expression(Tokenizer* t, Expression* lhs);
	public:
		static Expression* Parse(const wchar_t* expression);
		static ListExpression* Parse(Variable* lhs, const wchar_t* expression);
		static Expression* ParseCall(Token func, const wchar_t* args);
};
