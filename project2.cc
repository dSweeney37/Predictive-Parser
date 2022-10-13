/*
 * Copyright (C) Mohsen Zohrevandi, 2017
 *               Rida Bazzi 2019
 * Do not share this file with anyone
 */
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include "project2.h"

using namespace std;




int main(int argc, char* argv[]) {
	int task;
	Parser input;


	if (argc < 2) {
		cout << "Error: missing argument\n";
		return 1;
	}

	/*
	   Note that by convention argv[0] is the name of your executable,
	   and the first argument to your program is stored in argv[1]
	 */
	task = atoi(argv[1]);


	// Reads the input grammar from standard input and represent it internally in data structures
	// as described in project 2 presentation file
	input.ParseGrammer();

	switch (task) {
		case 1:
			input.PrintTerminalsAndNonTerminals();
			break;

		case 2:
			input.RemoveUselessSymbols();
			break;

		case 3:
			input.PrintSets(FIRST);
			break;

		case 4:
			input.PrintSets(FOLLOW);
			break;

		case 5:
			input.CheckIfGrammarHasPredictiveParser();
			break;

		default:
			cout << "Error: unrecognized task number " << task << "\n";
			break;
	}
	return 0;
}



int Parser::AddSymbol(string pName, bool pTerminal) {
	bool found = false;
	int i = 0, max = symbols.size();


	while (found == false && i < max) {
		if (pName == symbols[i].name) { found = true; }
		else { ++i; }
	}

	if (found == true) {
		if (pTerminal == false) {
			symbols[i].isTerminal = false;
			symbols[i].isGenerating = false;
		}
	}
	else { symbols.push_back(Symbol(pName, pTerminal)); }

	return i;
}



// Appends the set of "pFrom" to "pTo". If any symbols were appended, then true is returned.
bool Parser::AppendSetFromTo(bool* pFrom, bool* pTo) {
	bool altered = false;


	// If a symbol in the "pFrom" set is true and the corresponding symbol in the set of "pTo" is
	// false, then the symbol in "pTo"'s set is set to true as well as the return value. "i" begins
	// at 1 becasue 0 is reserved for epsilon and never appended using this function.
	for (int i = 1; i < symbols.size(); ++i) {
		if (pFrom[i] == true && pTo[i] == false) {
			pTo[i] = true;
			altered = true;
		}
	}
	return altered;
}



// Task 3
void Parser::CalculateFIRSTSets() {
	bool rerun;


	// RULE I & II: If the symbol is a terminal, then its corresponding index in its FIRST set is
	//				toggled to true. The data member "hasEpsilon" is translated to into the FIRST
	//				set as well.
	for (int i = 0; i < symbols.size(); ++i) {
		// Initializes the FIRST & FOLLOW set arrays for each symbol.
		symbols[i].FIRST = new bool[symbols.size()]{ false };
		symbols[i].FOLLOW = new bool[symbols.size()]{ false };

		// Adds each terminal to its own FIRST set.
		if (symbols[i].isTerminal == true) { symbols[i].FIRST[i] = true; }

		// Translates the data member "hasEpsilon" to the FIRST set of the corresponding symbol. 
		if (symbols[i].hasEpsilon == true) { symbols[i].FIRST[EPSILON_POS] = true; }
	}

	do {
		rerun = false;

		for (int i = 0; i < rules.size(); ++i) {
			bool proceed = true;
			int j = 0, max = rules[i].RHS.size();


			while (proceed == true && j < max) {
				// RULE III: Adds the FIRST set of the RHS symbol to the FIRST set of the LHS symbol.
				if (AppendSetFromTo(symbols[rules[i].RHS[j]].FIRST, symbols[rules[i].LHS].FIRST) == true) { rerun = true; }

				// RULE IV: Proceeds to the next symbol in the RHS if the current symbol contains epsilon.
				if (symbols[rules[i].RHS[j]].FIRST[EPSILON_POS] == false) { proceed = false; }

				++j;
			}

			// RULE V: Adds epsilon to the LHS symbol's FIRST set if "proceed" still equals true after 
			//		   processing the last symobl in the RHS set.
			if (proceed == true && symbols[rules[i].LHS].FIRST[EPSILON_POS] == false) {
				symbols[rules[i].LHS].FIRST[EPSILON_POS] = true;
				rerun = true;
			}
		}
	} while (rerun == true);
}



// Task 4: For each symbol, a call to create the FIRST set is made and then the FOLLOW set is built.
void Parser::CalculateFOLLOWSets() {
	bool rerun;

	
	CalculateFIRSTSets();

	// RULE I: Toggles on the EOF symbol's value in the FOLLOW set of the start symbol.
	if (symbols.size() > 1) { symbols[2].FOLLOW[EOF_POS] = true; }

	do {
		rerun = false;

		for (int i = 0; i < rules.size(); ++i) {
			int max = rules[i].RHS.size();


			for (int j = 0; j < max; ++j) {
				if (symbols[rules[i].RHS[j]].isTerminal == false) {
					bool proceed = true;
					int k = j + 1;


					// RULE IV & V: Appends the FIRST set of the following symbol to the FOLLOW set
					//				of the current symbol. Repeats this process as long as the 
					//				following symbol contains epsilon.
					while (proceed == true && k < max) {
						if (AppendSetFromTo(symbols[rules[i].RHS[k]].FIRST, symbols[rules[i].RHS[j]].FOLLOW) == true) {
							rerun = true;
						}

						if (symbols[rules[i].RHS[k]].FIRST[EPSILON_POS] == false) { proceed = false; }

						++k;
					}

					// RULE II & III: If the last symbol in the RHS of a rule is non-terminal, then the FOLLOW
					//				  set of the LHS symbol is appended to the FOLLOW set of the last symbol.
					if (proceed == true) {
						if (AppendSetFromTo(symbols[rules[i].LHS].FOLLOW, symbols[rules[i].RHS[j]].FOLLOW) == true) {
							rerun = true;
						}
					}
				}
			}
		}
	} while (rerun == true);
}



// Task 5
void Parser::CheckIfGrammarHasPredictiveParser() {
	bool isPredicitive = true;


	CalculateFOLLOWSets();

	for (int i = 0; i < rules.size(); ++i) {
		bool proceed = true;
		bool* ruleSet1 = new bool[symbols.size()]{ false };
		int x = 0, max = rules[i].RHS.size();


		// Appends every symbol's FIRST set in the RHS to a newly created set until a non-terminal
		// or the end of the set is reached.
		while (proceed == true && x < max) {
			AppendSetFromTo(symbols[rules[i].RHS[x]].FIRST, ruleSet1);
			if (symbols[rules[i].RHS[x]].FIRST[EPSILON_POS] == false) { proceed = false; }
			++x;
		}
		if (proceed == true) { ruleSet1[EPSILON_POS] = true; }


		for (int j = i + 1; j < rules.size(); ++j) {
			if (rules[j].LHS == rules[i].LHS) {
				proceed = true;
				bool* ruleSet2 = new bool[symbols.size()]{ false };
				x = 0, max = rules[j].RHS.size();


				// Appends every symbol's FIRST set in the RHS to a newly created set until a non-terminal
				// or the end of the set is reached.
				while (proceed == true && x < max) {
					AppendSetFromTo(symbols[rules[j].RHS[x]].FIRST, ruleSet2);
					if (symbols[rules[j].RHS[x]].FIRST[EPSILON_POS] == false) { proceed = false; }
					++x;
				}
				if (proceed == true) { ruleSet2[EPSILON_POS] = true; }

				if (IsIntersectionEmpty(ruleSet1, ruleSet2) == false) { isPredicitive = false; }
			}

			if (symbols[rules[i].LHS].FIRST[EPSILON_POS] == true) {
				if (IsIntersectionEmpty(symbols[rules[i].LHS].FIRST, symbols[rules[i].LHS].FOLLOW) == false) {
					isPredicitive = false;
				}
			}
		}
	}
	
	RemoveNonGeneratingSymbolRules();
	RemoveUnreachableSymbolRules();
	if (rules.size() == 0) { isPredicitive = false; }

	if (isPredicitive == true) { cout << "YES" << endl; }
	else { cout << "NO" << endl; }
}



// This function gets a token and checks if it is of the expected type. If it is, the token is
// returned, otherwise, synatx_error() is generated this function is particularly useful to match
// terminals in a right hand side of a rule. Written by Mohsen Zohrevandi
// Note: Borrowed from Project 1
Token Parser::Expect(TokenType expected_type) {
	Token t = lexer.GetToken();


	if (t.token_type != expected_type) { SyntaxError(); }
	return t;
}



bool Parser::IsIntersectionEmpty(bool* pSet1, bool* pSet2) {
	bool empty = true;


	for (int i = 0; i < symbols.size(); ++i) {
		if (pSet1[i] == true && pSet2[i] == true) { empty = false; }
	}
	return empty;
}



void Parser::ParseGrammer() {
	// --> Rule-list & HASH

	// Adds the symbol for epsilon and EOF prior to parsing.
	symbols.push_back(Symbol("#", true));
	symbols.push_back(Symbol("$", false));

	ParseRuleList();
	
	// Verifies and consumes "HASH".
	Expect(HASH);

	// Sets the start symbol's "isReachable" data member to true.
	if (symbols.size() > 1) { symbols[2].isReachable = true; }
}



void Parser::ParseRuleList() {
	// --> Rule || Rule & Rule-list
	Token p;


	ParseRule();
	// Checks whether or not to continue parsing this block.
	p = lexer.peek(1);
	if (p.token_type == ID) { ParseRuleList(); }
}



void Parser::ParseIdList() {
	// --> ID || ID & Id-list
	int pos;
	Token p, t;


	// Verifies and consumes "ID".
	t = Expect(ID);

	// Sends the "ID" info to check whether or not a new variable should be added to "symbols".
	pos = AddSymbol(t.lexeme, true);

	// Adds the symbol's index to the latest rule's RHS.
	rules[rules.size() - 1].RHS.push_back(pos);

	// Checks whether or not to continue parsing this block.
	p = lexer.peek(1);
	if (p.token_type == ID) { ParseIdList(); }
}



void Parser::ParseRule() {
	// --> ID & ARROW & Right-hand-side & STAR
	int pos;
	Token p, t;


	// Verifies and consumes "ID".
	t = Expect(ID);

	// Check whether or not a new symbol should be added to "symbols".
	pos = AddSymbol(t.lexeme, false);

	// Creates a new rule.
	rules.push_back(Rule(pos));

	// Verifies and consumes "ARROW".
	Expect(ARROW);

	// Checks whether or not to continue parsing this block.
	p = lexer.peek(1);
	if (p.token_type != ID && p.token_type != STAR) { SyntaxError(); }
	else { ParseRHS(pos); }

	// Verifies and consumes "STAR".
	Expect(STAR);
}



void Parser::ParseRHS(int pPos) {
	// --> Id-list | E
	Token p;


	// Verifies that the next token matches one of the productions in the RHS.
	p = lexer.peek(1);
	if (p.token_type == ID) { ParseIdList(); }
	else if (p.token_type != STAR) { SyntaxError(); }
	else { symbols[pPos].hasEpsilon = true; }
}



void Parser::PrintRules() {
	for (int i = 0; i < rules.size(); ++i) {
		cout << symbols[rules[i].LHS].name << " -> ";

		for (int j = 0; j < rules[i].RHS.size(); ++j) {
			if (j > 0) { cout << " "; }
			cout << symbols[rules[i].RHS[j]].name;
		}
		cout << endl;
	}
}



// Makes a call to build the FIRST and FOLLOW sets and then prints whichever set is designated via
//  the argument.
void Parser::PrintSets(SetType pSet) {
	int max = symbols.size();
	string set;


	CalculateFOLLOWSets();

	if (pSet == FIRST) { set = "FIRST"; }
	else { set = "FOLLOW"; }

	for (int i = 2; i < max; ++i) {
		if (symbols[i].isTerminal == false) {
			bool addPunc = false;


			cout << set << "(" << symbols[i].name << ") = { ";

			for (int j = 0; j < max; ++j) {
				if (pSet == FIRST && symbols[i].FIRST[j] == true) {
					if (addPunc == true) { cout << ", "; }
					cout << symbols[j].name;
					addPunc = true;
				}

				else if (pSet == FOLLOW && symbols[i].FOLLOW[j] == true) {
					if (addPunc == true) { cout << ", "; }
					cout << symbols[j].name;
					addPunc = true;
				}
			}
			cout << " }\n";
		}
	}
}



void Parser::PrintSymbols() {
	cout << "Name\t|\tTerminal\t|\tHas Epsilon\t|\tIs Generating\n";
	cout << "------------------------------------------------------------------------------\n";

	for (int i = 0; i < symbols.size(); ++i) {
		cout
			<< symbols[i].name
			<< "\t|\t";

		if (symbols[i].isTerminal == true) { cout << "Y\t"; }
		else { cout << "N\t"; }

		cout << "\t|\t";

		if (symbols[i].hasEpsilon == true) { cout << "Y\t"; }
		else { cout << "N\t"; }

		cout << "\t|\t";

		if (symbols[i].isGenerating == true) { cout << "Y\t\n"; }

		else { cout << "N\t\n"; }
	}
}



// Task 1
void Parser::PrintTerminalsAndNonTerminals() {
	string terminals, nonTerminals;


	for (int i = 2; i < symbols.size(); ++i) {
		if (symbols[i].isTerminal == true) { terminals += symbols[i].name + " "; }
		else { nonTerminals += symbols[i].name + " "; }
	}

	cout << terminals << nonTerminals << endl;
}



// Determines which of the non-terminals are generating and then removes any rule that contains a
// non-generating symbol.
void Parser::RemoveNonGeneratingSymbolRules() {
	bool rerun;
	int i = 0, max = rules.size();


	do {
		rerun = false;

		for (int i = 0; i < rules.size(); ++i) {
			bool generating = true;


			for (int j = 0; j < rules[i].RHS.size(); ++j) {
				if (symbols[rules[i].RHS[j]].isGenerating == false) { generating = false; }
			}

			if (generating == true) {
				if (symbols[rules[i].LHS].isGenerating == false) {
					symbols[rules[i].LHS].isGenerating = true;
					rerun = true;
				}
			}
		}
	} while (rerun == true);


	while (i < max) {
		bool remove = false;


		if (symbols[rules[i].LHS].isGenerating == false) { remove = true; }
		
		else {
			for (int j = 0; j < rules[i].RHS.size(); ++j) {
				if (symbols[rules[i].RHS[j]].isGenerating == false) { remove = true; }
			}
		}

		if (remove == true) {
			rules.erase(rules.begin() + i);
			--max;
		}
		else { ++i; }
	}
}



// Toggles the "isReachable" value of all the symbols in the RHS of a rule to true if the LHS
// symbol is reachable. Any rules that are unreachable will be deleted. Repeats this process 
// for every rule in the "rules" vector.
void Parser::RemoveUnreachableSymbolRules() {
	bool rerun;
	int i = 0, max = rules.size();


	// Toggles all of the "isReachable" values accordingly and repeats until no more changes
	// are made.
	do {
		rerun = false;

		for (int i = 0; i < max; ++i) {
			if (symbols[rules[i].LHS].isReachable == true) {
				for (int j = 0; j < rules[i].RHS.size(); ++j) {
					if (symbols[rules[i].RHS[j]].isReachable == false) {
						symbols[rules[i].RHS[j]].isReachable = true;
						rerun = true;
					}
				}
			}
		}
	} while (rerun == true);

	while (i < max) {
		bool remove = false;


		if (symbols[rules[i].LHS].isReachable == false) { remove = true; }

		else {
			for (int j = 0; j < rules[i].RHS.size(); ++j) {
				if (symbols[rules[i].RHS[j]].isReachable == false) { remove = true; }
			}
		}

		if (remove == true) {
			rules.erase(rules.begin() + i);
			--max;
		}
		else { ++i; }
	}
}



// Task 2
void Parser::RemoveUselessSymbols() {
	RemoveNonGeneratingSymbolRules();
	RemoveUnreachableSymbolRules();

	for (int i = 0; i < rules.size(); ++i) {
		cout << symbols[rules[i].LHS].name << " ->";

		if (rules[i].RHS.size() == 0) { cout << " #"; }

		for (int j = 0; j < rules[i].RHS.size(); ++j) {
			cout << " " << symbols[rules[i].RHS[j]].name;
		}
		cout << endl;
	}
}



void Parser::SyntaxError() {
	cout << "SYNTAX ERROR !!!" << endl;
	exit(1);
}



// Constructor for the "Rule" class.
Rule::Rule(int pPos) { LHS = pPos; }



// Constructor for the "Symbol" class.
Symbol::Symbol(string pName, bool pTerminal) {
	hasEpsilon = false;
	if (pTerminal == true) { isGenerating = true; }
	else { isGenerating = false; }
	isReachable = false;
	isTerminal = pTerminal;
	name = pName;
}
