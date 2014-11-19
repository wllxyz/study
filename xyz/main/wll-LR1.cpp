#define TOPDOWN_PARSER  0
#define LR0_PARSER  1
#define LR1_PARSER  2

#define PARSER  LR1_PARSER

#if PARSER==LR0_PARSER
#include "LR0Parsers.h"
#elif   PARSER==TOPDOWN_PARSER
#include "TopDownParsers.h"
#elif   PARSER==LR1_PARSER
#include "LR1Parsers.h"
#endif  //PARSER

#include <fstream>
using namespace std;

int main(int argc,char** argv)
{
#if PARSER==LR0_PARSER
	LR0Parsers parser;
#elif   PARSER==TOPDOWN_PARSER
	TopDownParsers parser;
#elif   PARSER==LR1_PARSER
	LR1Parsers parser;
#endif  //PARSER
	
	parser.LoadLanguage();

	if(argc==1)
	{
		//cout<<argv[0]<<"<grammar_file> <input_file>"<<endl;
		cout<<"WLL0 language auto loaded ..."<<endl;
		do
		{
			string cmd;
			cin>>cmd;
			if(cmd=="quit")
			{
				break;
			}
			if(cmd=="load")
			{
				string file_name;
				cin>>file_name;
				ifstream input(file_name.c_str());
				if(input.fail())
				{
					cout<<"open file ["<<file_name<<" failed!"<<endl;
					continue;
				}
				cout<<"loading ["<<file_name<<"] ..."<<endl;
				if(!parser.Process(input,cout))
				{
					cout<<"process file ["<<file_name<<"] failed!"<<endl;
					continue;
				}
				cout<<"[ "<<file_name<<"] loaded."<<endl;
			}
			if(cmd=="debug")
			{
				cout<<"current loaded language rules:"<<endl;
				parser.DisplayLanguage();
			}
		}while(true);

		return 0;
	}

	for(int i=1; i<argc; i++)
	{
		ifstream input(argv[i]);
		if(input.fail())
		{
			cerr<<"open input file ["<<argv[i]<<"] failed!"<<endl;
			continue;
		}
		if(!parser.Process(input,cout))
		{
			cerr<<"process input file ["<<argv[i]<<"] failed!"<<endl;
			continue;
		}
	}

	return 0;
}

//<�޸ļ�¼>
// ʵ�����ķ������滻��ű�Ĵ���,���������ַ���,���ǵ�����.����Ҳ��Ϊ�ķ����з���ƥ��.
//</�޸ļ�¼>
