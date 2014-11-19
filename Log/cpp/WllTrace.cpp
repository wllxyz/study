/*------------------------------------------------------*/
/* implement trace initalization and clean up methods   */
/*                                                      */
/* WllTrace.cpp                                         */
/*                                                      */
/* Copyright (C) WebEx Communications Inc.              */
/* All rights reserved                                  */
/*                                                      */
/* Author                                               */
/*      Shine Wang (shinew@hz.webex.com)                */
/*                                                      */
/* History                                              */
/*    07/30/2010  ADD TO PLUG CmTraceFromT120.h         */
/*                                                      */
/*                                                      */
/*------------------------------------------------------*/

#include <strstream>
#include <fstream>
using namespace std;
#include "WllTrace.h"
#include "WllRemoteTrace.h"
#include "WllTraceConfig.h"
#include "WllLocalTrace.h"
#include "WllUtil.h"
#include "WllReload.h"
using namespace Wll::Util;
using namespace Wll::Tool;
using namespace Wll::Tool::LogInnerTypes;
#include <sys/time.h>
#include <signal.h>
#include <string>
#include <cstdlib>
using namespace std;

static const char* LOG_SERVER_RELOAD_PORT_NAME="WLL_LOG_RELOAD_PORT";

bool RegisterSignalPipeHandler(void);
bool InitalizeReload(void);

//handle the SIGPIPE signal
void SignalPipeHandler(int signal_num)
{
 //set remote_trace off, switch remote_trace to local_trace
 //remote_trace_print_function=PrintLocalTrace;
 //default_trace_print_function=PrintLocalTrace;
 //TERM_INFO("remote trace switched to local trace");
 TERM_INFO("SIGPIPE signal captured by WllTrace lib,IGNORED");
}

bool RegisterSignalPipeHandler(void)
{
 bool registered=false;
 if(registered) return true;

 DEBUG_LOG("register signal PIPE handler...");
 
 registered=signal(SIGPIPE,SignalPipeHandler)!=SIG_ERR;
 return registered;
}

bool Reload(void)
{
 DEBUG_LOG("reloading log print config...");
 char config_filename[MAX_FILE_NAME_LENGTH]="";
 GetWllConfigFile(config_filename,sizeof(config_filename));
 DEBUG_LOG("config_filename=["<<config_filename<<"]");
 
 LogPrintConfig* p_config=LogPrintConfig::GetInstance();
 if(p_config==NULL)
  {
   TERM_ERROR("Get instance of LogPrintConfig error");
   return false;
  }

 DEBUG_LOG("log print config reloading...");

 return p_config->Load( config_filename );
}

void RemoveReloadPortFile(void)
{
 string reload_port_file_name = string( GetProcessName() ) + "__" + LOG_SERVER_RELOAD_PORT_NAME ;
 DEBUG_LOG( "unlinking reload port file [" << reload_port_file_name << "]..." );
 if( 0 == unlink( reload_port_file_name.c_str() ) )
 {
  DEBUG_LOG( "unlink file ["<<reload_port_file_name<<"] success" );
 }
 else
 {
  DEBUG_LOG( "unlink file ["<<reload_port_file_name<<"] failed" );
 }
 
}

bool InitalizeReload(void)
{
 static bool initalized=false;
 if(initalized) return true;
 
 ReloadServerThread* reload_server_thread=new (nothrow)ReloadServerThread((string(GetProcessName())+"__"+LOG_SERVER_RELOAD_PORT_NAME).c_str(),Reload);
 if(reload_server_thread==NULL)
  {
   DEBUG_LOG("create reload server thread failed,return false");
   return false;
  }
 DEBUG_LOG("reload_server_thread created success");
 DEBUG_LOG("Enabled reload_server_thread auto destroy");
 reload_server_thread->EnableAutoDestroy(true);
 DEBUG_LOG("Starting reload_server_thread...");
 reload_server_thread->Start();
 DEBUG_LOG("reload_server_thread started.");
 //RegisterSignalPipeHandler();

 DEBUG_LOG("register atexit function to remove reload port file...");
 atexit(RemoveReloadPortFile);
 DEBUG_LOG("RemoveReloadPortFile registered");

 initalized=true;
 return true;
}

bool PrintRemoteTrace(int mask,const char*msg)
{
 //RemoteTraceClient::GetInstance()->Log(mask,msg);
 return ( RemoteTraceClient::GetInstance()->Log(mask,msg) ? true : PrintLocalTrace(mask,msg) );
}

bool PrintLocalTrace(int mask,const char* msg)
{
 return ( LocalTrace::GetInstance()->Log(mask,msg) ? true : PrintTermTrace(mask,msg) );
}

bool PrintTermTrace(int mask, const char* msg)
{
 LogPrintConfig* config = LogPrintConfig::GetInstance();
 if(config==NULL)
 {
  DEBUG_LOG("get instance of LogPrintConfig failed!");
  return false;
 }
 
 if(config->is_show && (config->is_error_show || config->is_warning_show || config->is_info_show) )
 {
  //format message text with date time and process id ,thread id info
  char str_buf[Wll::Tool::LogInnerTypes::MAX_LOG_LENGTH]="";
  std::strstream str(str_buf,sizeof(str_buf));
  char date_time_str[80]="";
  str<<"["<<Wll::Util::GetDateTimeStr(date_time_str,sizeof(date_time_str))<<" pid="<<getpid()<<" tid="<<pthread_self()<<"] "<<msg;

  static pthread_mutex_t lock;
  pthread_mutex_lock(&lock);
  std::cerr<<str.str()<<std::endl;
  pthread_mutex_unlock(&lock);
 }
 return true;
}

void PrintDebugLog(const char* msg)
{
 static pthread_mutex_t lock;

 pthread_mutex_lock(&lock);

 std::cerr<<msg<<std::endl;
 
 pthread_mutex_unlock(&lock);
}


 
