﻿#include "LR1Parsers.h"
#include "LanguageAlgorithm.h"
#include <fstream>
#include <cctype>
#include <WllTrace.h>
using namespace std;

//判定文法是否存在移近规约冲突（文法二义性判断）
bool LR1Parsers::IsAmbiguous(const vector< vector< TransformEdge > >& state_transform_table)
{
	bool flag = false;
	int n = 0;
	for(vector< vector< TransformEdge > >::const_iterator i = state_transform_table.begin(); i != state_transform_table.end(); ++i)
	{
		map<Symbols,int> stat_map;
		for(vector<TransformEdge>::const_iterator j = i->begin(); j != i->end(); ++j)
		{
			Symbols symbol = j->symbol;
			if(stat_map.find(symbol)==stat_map.end())
				stat_map[symbol]=1;
			else
				stat_map[symbol]++;
		}
		++n;
		//check & output conflict
		if(stat_map.size() != i->size())
		{
			flag = true;
			//#ifdef	DEBUG
			cerr<<"conflict in state transform table (state line = "<<n<<") :"<<endl;

			for(map<Symbols,int>::iterator it = stat_map.begin(); it != stat_map.end(); ++it)
			{
				if(it->second > 1)	//conflict item
				{
					for(vector<TransformEdge>::const_iterator j = i->begin(); j != i->end(); ++j)
					{
						if(j->symbol == it->first)
						{
							cerr<<*j<<endl;
						}
					}
				}
			}
			//#endif	//DEBUG 
		}
	}
	return flag;
}

bool LR1Parsers::AnalyzeLanguage()
{
	LanguageParsers::AnalyzeLanguage();
  	//根据文法自动生成文法预测分析表
	vector< vector<TransformEdge> > state_transform_table;
	GenerateStateTransformTable(this->languages.source_rules,state_transform_table);
  	if(this->IsAmbiguous(state_transform_table)) return false;

	this->state_transform_table.clear();
	ConvertStateTransformTable(state_transform_table,this->state_transform_table);
	DEBUG_LOG("state_transform_table=\n"<<this->state_transform_table);
	return true;
}

//根据文法规则对输入符号流进行文法结构的分析,分析结果用文法分析树表示
bool LR1Parsers::Parse()
{
	//根据文法预测分析表分析文法,得到文法分析树
  	return LRParse(this->input_symbols,this->source_tree, this->state_transform_table,this->languages.source_rules.rules);
}

bool LR1Parsers::IsXyzLanguage(const vector<Symbols>& symbols)
{
#ifdef	LR1WLLXYZ
	static LR1Parsers wll_xyz;
	static bool is_wll_xyz_loaded = false;

	//avoid dead loop of recurse call itself
	if(this==&wll_xyz)
	{
		//cerr<<"wll_xyz call IsXyzLanguage ..."<<endl;
		return LanguageParsers::IsXyzLanguage(symbols);
	}

	if(is_wll_xyz_loaded == false)
	{
		ifstream xll_xyz_grammar_file("conf/wll.xyz");
		if(xll_xyz_grammar_file.fail())
		{
			cerr<<"open grammar file conf/wll.xyz failed"<<endl;
			return false;
		}

		if( !wll_xyz.LoadLanguage(xll_xyz_grammar_file) )
		{
			cerr<<"load conf/wll.xyz failed"<<endl;
			return false;
		}
		is_wll_xyz_loaded = true;
	}

	wll_xyz.SetInput(symbols);
	return wll_xyz.Parse();
#else
	return LanguageParsers::IsXyzLanguage(symbols);
#endif	//LR1WLLXYZ
}

