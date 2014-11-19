#ifndef WLL_UTIL_H
#define WLL_UTIL_H

#include <stdio.h>

namespace Wll
{
namespace Util
{

static const int MAX_TEMP_BUF_LENGTH=1024;

//get webex home path
char* GetWllHome(char* str=NULL,size_t size=0);
//get webex config file path
char* GetWllConfigFile(char* buf=NULL,size_t size=0);
//get webex log path
char* GetWllLogPath(char* buf=NULL,size_t size=0);
//get process name
char* GetProcessName(char* str=NULL,size_t size=0);
//get date and time string
char* GetDateTimeStr(char* str=NULL,size_t size=0);
//get date string
char* GetDateStr(char* str=NULL,size_t size=0);
//get file length
long GetFileLength(FILE* file);

};//end of namespace Util
};//end of namespace Wll

#endif //WLL_UTIL_H

