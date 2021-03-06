#ifndef	LANGUAGE_ALGORITHM
#define LANGUAGE_ALGORITHM

#include "LanguageGrammar.h"
#include "LanguageExpressions.h"
#include "LanguageTree.h"
#include "StateTransformTable.h"
#include "States.h"
#include <vector>
using namespace std;

enum ReduceType
{
	LEFT_REDUCE,
	REVERSE_RIGHT_REDUCE
};

//Reduce the expression by rules
bool Expend(LanguageExpressions& expression,const vector<LanguageRules*>& used_rules,ReduceType type);
//for left reduce
bool LeftExpend(LanguageExpressions& expression,const vector<LanguageRules*>& used_rules);
//for left statute
bool ReverseRightExpend(LanguageExpressions& expression,const vector<LanguageRules*>& used_rules);

//Reduce the expression by rules
bool Reduce(LanguageExpressions& expression,const vector<LanguageRules*>& used_rules,ReduceType type);
//for left reduce
bool LeftReduce(LanguageExpressions& expression,const vector<LanguageRules*>& used_rules);
//for left statute
bool ReverseRightReduce(LanguageExpressions& expression,const vector<LanguageRules*>& used_rules);

bool ConstructTreeByRules(LanguageTree*& source_tree,const vector<size_t>& rules_no,const vector<LanguageRules*>& source_languages,ReduceType type);
//conduce tree by rules (用规约或者是推倒规则rules重建语法分析树source_tree,languages是用来查找rule_no用的)
bool ConstructTreeByLeftReduceRules(LanguageTree*& source_tree,const vector<size_t>& rules_no,const vector<LanguageRules*>& source_languages);
//conduce tree by rules (用规约或者是推倒规则rules重建语法分析树source_tree,languages是用来查找rule_no用的)
bool ConstructTreeByReverseRightReduceRules(LanguageTree*& source_tree,const vector<size_t>& rules_no,const vector<LanguageRules*>& source_languages);

//translate the source tree to dest tree, use dest_languages for lookup rule_no
bool TranslateTree(LanguageTree* source,LanguageTree*& dest,const vector<LanguageRules*>& destination_languages);
//explain tree to introduce the self-explain feather
bool SelfExplain(const vector<Symbols>& symbols,vector<LanguageTranslations>& languages);

//根据文法自动生成文法预测分析表(供LR1文法分析器重载)
void GenerateStateTransformTable(const LanguageIndex& languages,vector< vector<TransformEdge> >& state_transform_table, vector< StateSets<LR1States> >& state_sets);

void Closure(const LanguageIndex& languages, StateSets<LR1States>& I);

void ConvertStateTransformTable(const vector< vector<TransformEdge> >& state_transform_table, StateTransformTable& table);

//根据文法预测分析表分析文法,得到文法分析树(供LR1重载)
bool LRParse(const vector<Symbols>& symbols,LanguageTree*& tree,const StateTransformTable& state_transform_table,const vector<LanguageRules*>& languages);

#endif	//LANGUAGE_ALGORITHM

