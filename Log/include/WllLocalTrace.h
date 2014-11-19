#ifndef LOCAL_TRACE_H
#define LOCAL_TRACE_H

#include "WllGroupFile.h"
#include "WllTraceConfig.h"
#include "WllTraceInterface.h"

namespace Wll
{
namespace Tool
{
//implement local trace print manage
class LocalTrace : public TraceInterface
{
public:
 //get static instance
 static LocalTrace* GetInstance(void);
 //constructor  
 LocalTrace();
 //constructor
 LocalTrace(const char* filename_template);
 //destructor
 virtual ~LocalTrace();
 //open a trace obj to write local disk log
 bool Open(const char* filename_template,const Wll::Tool::LogInnerTypes::LogPrintConfig& config);
 //close     
 void Close(void); 
 //print log message on disk file 
 bool WriteLog(int mask,const char* msg,size_t len);
 //configure log print enable/disable   
 virtual void Config(const Wll::Tool::LogInnerTypes::LogPrintConfig& configs);
 //format log message and output
 virtual bool Log(int mask,const char* msg);
private:
 static LocalTrace* instance;  //static instance
 GroupFile info_trace_file;  //file to print info trace
 GroupFile warning_trace_file;  //file to print warning trace
 GroupFile error_trace_file;  //file to print error trace
 int info_trace_buffered_lines;   //record info_trace_file buffered log lines
 int warning_trace_buffered_lines;  //record warning_trace_file buffered log lines
 int error_trace_buffered_lines;  //record error_trace_file buffered log lines

 Wll::Tool::LogInnerTypes::LogPrintConfig log_print_config; //config the log print or not
 bool is_opened;    //show the trace files are opened
private:
 //test whether the filename_template is valid
 bool CheckFileNameTemplate(const char* filename_template);
};//end of class LocalTrace

};//end of namespace Tool
};//end of namespace Wll

#endif //LOCAL_TRACE_H


 
