#include "WllTraceConfig.h"
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
using namespace std;
#include "WllConfigure.h"
#include "WllUtil.h"
//#include "safe_str_lib.h"
using namespace Wll::Tool;
using namespace Wll::Util;

using namespace Wll::Tool::LogInnerTypes;


LogPrintConfig* LogPrintConfig::instance=NULL;

//control the log print configs
 LogPrintConfig* LogPrintConfig::GetInstance(void)
 {
  if(instance==NULL)
         {
   instance=new (nothrow) LogPrintConfig(GetWllConfigFile());
  }
  return instance;
 }

 LogPrintConfig::LogPrintConfig()
        {
                this->is_show=true;
                this->is_info_show=true;
                this->is_warning_show=false;
                this->is_error_show=true;
                this->max_log_file_number=DEFAULT_MAX_LOG_FILE_NUMBER;
                this->max_log_file_size=DEFAULT_MAX_LOG_FILE_SIZE;
                this->max_log_buffered_lines=DEFAULT_MAX_LOG_BUFFERED_LINES;
        }

        LogPrintConfig::LogPrintConfig(const char* config_file)
        {
                this->is_show=true;
                this->is_info_show=true;
                this->is_warning_show=false;
                this->is_error_show=true;
                this->max_log_file_number=DEFAULT_MAX_LOG_FILE_NUMBER;
                this->max_log_file_size=DEFAULT_MAX_LOG_FILE_SIZE;
                this->max_log_buffered_lines=DEFAULT_MAX_LOG_BUFFERED_LINES;

                this->Load(config_file);
        }

        bool LogPrintConfig::operator== (const LogPrintConfig& that)const
        {
                return memcmp(this,&that,sizeof(LogPrintConfig))==0;
        }

        bool LogPrintConfig::Load(const char* config_file)
        {
                Configure config;
                if(!config.Load(config_file))
                {
                        std::cerr<<"load config file ["<<config_file<<"] failed\n";
                        return false;
                }

                config.Get("Trace","Enable",this->is_show);

                char buf[MAX_CONFIG_LENGTH]="";

                snprintf(buf,sizeof(buf),"%s.info",GetProcessName());
                config.Get("Trace",buf,this->is_info_show);

                snprintf(buf,sizeof(buf),"%s.warning",GetProcessName());
                config.Get("Trace",buf,this->is_warning_show);

                snprintf(buf,sizeof(buf),"%s.error",GetProcessName());
                config.Get("Trace",buf,this->is_error_show);

                snprintf(buf,sizeof(buf),"%s.max_log_file_number",GetProcessName());
                config.Get("Trace",buf,this->max_log_file_number);

                snprintf(buf,sizeof(buf),"%s.max_log_file_size", GetProcessName());
                config.Get("Trace",buf,*((long*)(&this->max_log_file_size)) );

                snprintf(buf,sizeof(buf),"%s.max_log_buffered_lines",GetProcessName());
                config.Get("Trace",buf,this->max_log_buffered_lines);

                return true;

        }

 bool LogPrintConfig::IsErrorShow(int mask) const
 {
  return (this->is_show && this->is_error_show && mask<ERROR_TRACE_LEVEL);
 }

 bool LogPrintConfig::IsWarningShow(int mask) const
 {
  return (this->is_show && this->is_warning_show && mask<WARNING_TRACE_LEVEL);
 }

 bool LogPrintConfig::IsInfoShow(int mask) const
 {
  return (this->is_show && this->is_info_show);
 }

//configs transmit from client to server
        RemoteTraceConfig::RemoteTraceConfig()
 {

 }

        RemoteTraceConfig::RemoteTraceConfig(int32_t process_id, const char* process_name, const LogPrintConfig& print_config)
        {
                this->process_id=process_id;
                strncpy(this->process_name,process_name,sizeof(this->process_name)-1);
                this->process_name[sizeof(this->process_name)-1]=0;
                this->print_config=print_config;
        }

        //write a RemoteTraceConfig obj to socket
        bool RemoteTraceConfig::Write(int socket_id)
        {
                return(sizeof(RemoteTraceConfig)==write(socket_id,this,sizeof(RemoteTraceConfig)));
        }
        //read a RemoteTraceConfig obj from socket
        bool RemoteTraceConfig::Read(int socket_id)
        {
                return(sizeof(RemoteTraceConfig)==read(socket_id,this,sizeof(RemoteTraceConfig)));
        }

//log message transmit from client to server

        //write a RemoteTraceMessage from client to server
        bool RemoteTraceMessage::Write(int socket_id)
        {
                return (sizeof(RemoteTraceMessage)==write(socket_id,this,sizeof(RemoteTraceMessage)));
        }
        //read a RemoteTraceMessage from client to server
        bool RemoteTraceMessage::Read(int socket_id)
        {
                memset((void*)this,0,sizeof(RemoteTraceMessage));
                return (sizeof(RemoteTraceMessage)==read(socket_id,this,sizeof(RemoteTraceMessage)));
        }


