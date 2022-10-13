#include "lexer.h"

#define EPSILON_POS 0
#define EOF_POS 1

typedef enum { FIRST, FOLLOW } SetType;




class Rule {
	public:
		int LHS;
		std::vector<int> RHS;

		Rule(int);
};



class Symbol {
	public:
		bool hasEpsilon;
		bool isGenerating;
		bool isReachable;
		bool isTerminal;
		bool* FIRST;
		bool* FOLLOW;
		std::string name;

		Symbol(std::string, bool);
};



class Parser {
	public:
		std::vector<Rule> rules;
		std::vector<Symbol> symbols;

		void CheckIfGrammarHasPredictiveParser();
		void ParseGrammer();
		void PrintSets(SetType);
		void PrintTerminalsAndNonTerminals();
		void RemoveUselessSymbols();


	private:
		LexicalAnalyzer lexer;

		int AddSymbol(std::string, bool);
		bool AppendSetFromTo(bool*, bool*);
		void CalculateFIRSTSets();
		void CalculateFOLLOWSets();
		Token Expect(TokenType);
		bool IsIntersectionEmpty(bool*, bool*);
		void ParseRuleList();
		void ParseIdList();
		void ParseRule();
		void ParseRHS(int);
		void PrintRules();
		void PrintSymbols();
		void RemoveNonGeneratingSymbolRules();
		void RemoveUnreachableSymbolRules();
		void SyntaxError();
};
