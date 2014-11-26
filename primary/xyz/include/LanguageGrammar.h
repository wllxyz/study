#ifndef	LANGUAGE_GRAMMAR_H
#define LANGUAGE_GRAMMAR_H

#include "LanguageTranslations.h"
#include "LanguageRules.h"
#include <vector>
#include <map>
using namespace std;

struct LanguageIndex
{
public:
	bool GetRulesBySymbol(Symbols symbol, vector<LanguageRules*>& rules) const;
	bool GetRuleNosBySymbol(Symbols symbol, vector<size_t>& rule_nos) const;
public:
	vector<LanguageRules*> rules;
	multimap<Symbols,size_t> rule_index;
};

struct LanguageGrammar
{
public:
	void Initialize();
public:
	vector<LanguageTranslations> translation_rules;
public:
	LanguageIndex source_rules;
	LanguageIndex destination_rules;
};

ostream& operator<< (ostream& o, const LanguageGrammar& lg);

#endif	//LANGUAGE_GRAMMAR_H
