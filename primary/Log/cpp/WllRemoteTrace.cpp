/*------------------------------------------------------*/
/* Remote Trace Print Class                             */
/*                                                      */
/* WllRemoteTrace.cpp                                   */
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

#include "WllRemoteTrace.h"

#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <iostream>
#include <fcntl.h>
#include <iostream>
#include <strstream>
#include <fstream>
#include <cstdio>
#include <new>
using namespace std;

#include "WllUtil.h"
#include "WllTrace.h"
#include "WllUnixSocket.h"
#include "WllLocalTrace.h"
//#include "safe_str_lib.h"
using namespace Wll::Tool::LogInnerTypes;
using namespace Wll::Tool;
using namespace Wll::System;
using namespace Wll::Util;

bool RegisterSignalPipeHandler(void);
bool InitalizeReload(void);

//#define LOCAL_TRACE(mask,msg) cerr<<__FILE__<<__LINE__<<msg;


RemoteTraceClient* RemoteTraceClient::instance=NULL;

RemoteTraceClient* RemoteTraceClient::GetInstance(void)
{
 if(RemoteTraceClient::instance==NULL)
 {
  static pthread_mutex_t create_instance_lock=PTHREAD_MUTEX_INITIALIZER;
  if(0==pthread_mutex_trylock(&create_instance_lock))
  {
   DEBUG_LOG("lock create RemoteTraceClient instance success, creating instance of RemoteTraceClient...");

   //do the instance create
   char webex_home[MAX_FILE_NAME_LENGTH]="";
   char remote_log_server_name[MAX_FILE_NAME_LENGTH]="";
   GetWllHome( webex_home, sizeof(webex_home) );
   snprintf(remote_log_server_name, sizeof(remote_log_server_name), "%s/%s", webex_home, DEFAULT_REMOTE_LOG_SERVER_NAME);
   
   DEBUG_LOG("CREATE RemoteTraceClient instance with remote_log_server_name=["<<remote_log_server_name<<"]");

   RemoteTraceClient::instance=new(nothrow) RemoteTraceClient(remote_log_server_name,RemoteTraceConfig(getpid(),GetProcessName(),*LogPrintConfig::GetInstance()));

   DEBUG_LOG("calling RegisterSignalPipeHandler ...");
   if(!RegisterSignalPipeHandler())
   {
    DEBUG_LOG("RegisterSignalPipeHandler for remote trace client application failed!");
   }

   DEBUG_LOG("calling InitalizeReload ...");
   if(!InitalizeReload())
   {
    DEBUG_LOG("InitalizeReload for remote trace client failed!");
   }
   
   //release lock
   pthread_mutex_unlock(&create_instance_lock);
   DEBUG_LOG("create_instance_lock unlocked");
   
   DEBUG_LOG("instance of RemoteTraceClient created");
  }
  else
  {
   DEBUG_LOG("wait the first locked thread complete the instance creatation");
   //wait the first locked thread complete the instance creatation
   pthread_mutex_lock(&create_instance_lock); //block to wait
   DEBUG_LOG("create_instance_lock locked,do nothing but release the lock");
   //do nothing
   pthread_mutex_unlock(&create_instance_lock);
   DEBUG_LOG("create_instance_lock unlocked.");
  }
 }
 
 DEBUG_LOG( (RemoteTraceClient::instance==NULL?"RemoteTraceClient::instance==NULL":"RemoteTraceClient::instance!=NULL") );
 return RemoteTraceClient::instance; 
}

//constructor
RemoteTraceClient::RemoteTraceClient(const char* server_address,const RemoteTraceConfig& config)
{
 pthread_mutex_init(&this->lock,NULL);
 this->is_connected=false;

 DEBUG_LOG("CONNECTING to server_address["<<server_address<<"] ...");
 this->connect_socket = this->client_socket.Open(server_address);
 if(this->connect_socket<0)
 {
  TERM_ERROR("open remote trace client ["<<server_address<<"] failed!");
  return;
 }
 DEBUG_LOG("server_address["<<server_address<<"] connected.");
 
 DEBUG_LOG("writing config to connected_socket["<<this->connect_socket<<"]");
 //
 this->config=config;
 DEBUG_LOG("config : process_name["<<config.process_name<<"],process_id["<<config.process_id<<"]" \
  <<",max_log_file_number["<<config.print_config.max_log_file_number \
  <<"],max_log_file_size["<<config.print_config.max_log_file_size \
  <<"],max_log_buffered_lines["<<config.print_config.max_log_buffered_lines \
  <<"],is_show["<<config.print_config.is_show \
  <<"],is_error_show["<<config.print_config.is_error_show \
  <<"],is_warning_show["<<config.print_config.is_warning_show\
  <<"],is_info_show["<<config.print_config.is_info_show<<"]");
 //send config
 if(!this->config.Write(this->connect_socket))
 {
  TERM_ERROR("write config error! return directly");
  return;
 }
 DEBUG_LOG("write config OK");
 
 DEBUG_LOG("reading response of OK ... from connect_socket["<<this->connect_socket<<"]");

 char buf[10]="";
 //recv OK
 read(this->connect_socket,buf,sizeof(buf));

 DEBUG_LOG("compare readed buf content with OK ...");
 if(strncmp(buf,"OK",sizeof(buf))!=0)
 {
  TERM_ERROR("read OK error!buf content is not OK");
  return;
 }
 DEBUG_LOG("response OK readed"); 

 DEBUG_LOG("RemoteTraceClient open and connect success");
 
 this->is_connected=true;
}

//write remote trace, should be multi-thread safe
bool RemoteTraceClient::Log(int mask,const char* msg)
{
 DEBUG_LOG("Testing RemoteClient is connected or not");
 if(!this->is_connected)
  {
   DEBUG_LOG("RemoteTraceClient is not connected, return false");
   return false;
  }
 DEBUG_LOG("RemoteTraceClient is connected");

 bool ret=true;
 
 DEBUG_LOG("Config print config ...");
 this->Config(*LogPrintConfig::GetInstance());
 
 DEBUG_LOG("Testing trace is enabled or not...");
 if(this->config.print_config.is_show && ( this->config.print_config.is_error_show || this->config.print_config.is_warning_show || this->config.print_config.is_info_show ))
 {
  DEBUG_LOG("trace is enabled, is_show=["<<this->config.print_config.is_show<<"]");
  DEBUG_LOG("prepareing RemoteTraceMessage ...");
  
  RemoteTraceMessage message;
  message.log_mask=mask;
  char date_time_str[80]="";
  GetDateTimeStr(date_time_str,sizeof(date_time_str));
  memset(message.log_text,0,sizeof(message.log_text)); 
  snprintf(message.log_text,sizeof(message.log_text),"[%s pid=%ld tid=%ld] %s\n",date_time_str, long(getpid()), long(pthread_self()), msg);
  DEBUG_LOG("message[log_mask="<<message.log_mask<<",log_text="<<message.log_text<<"]");
  DEBUG_LOG("mask=["<<mask<<"],msg=["<<msg<<"]");

  pthread_mutex_lock(&this->lock);
  DEBUG_LOG("write remote trace message lock locked");
  DEBUG_LOG("writing remote trace message ...");
  char remark=REMARK_MESSAGE;
  ret = ret && (1==write(this->connect_socket,&remark,1));
  ret = ret && message.Write(this->connect_socket);
  pthread_mutex_unlock(&this->lock);
  DEBUG_LOG("write remote trace message lock unlocked.");
 }
 
 DEBUG_LOG("ret=["<<ret<<"]");
 //Log should return true always when connect normal,if write error, switch the connect status false to disable remote print
 this->is_connected=ret;

 return ret;
}

void RemoteTraceClient::Config(const LogPrintConfig& config)
 {
  DEBUG_LOG("Testing config changed or not...");
  if(!(this->config.print_config==config))
  {
   DEBUG_LOG("config changed.");
   
   this->config.print_config=config;
   DEBUG_LOG("new config saved.");

   DEBUG_LOG("repare remote trace config...");
   char process_name[MAX_FILE_NAME_LENGTH]="";
   RemoteTraceConfig remote_config(getpid(),GetProcessName(process_name,sizeof(process_name)),config);
   DEBUG_LOG("remote trace config repared.");
   
   DEBUG_LOG("lock remote trace client to write remote trace config...");
   pthread_mutex_lock(&this->lock);
   DEBUG_LOG("remote trace client locked");
   
   char remark=REMARK_CONFIG;
   if( 1!=write(this->connect_socket,&remark,1) )
    {
     DEBUG_LOG("write config remark error");
    }
   DEBUG_LOG("config remark send success");
   
   if(!remote_config.Write(this->connect_socket))
    {
     DEBUG_LOG("write remote config error");
    }
   DEBUG_LOG("remote config write success");
   
   DEBUG_LOG("unlocking remote trace client...");
   pthread_mutex_unlock(&this->lock);
   DEBUG_LOG("remote trace client unlocked.");
  }
  else
   {
    DEBUG_LOG("config not changed.");
   }
 }

//constructor
RemoteTraceServer::RemoteTraceServer(void)
{

}

//run remote log server
void RemoteTraceServer::Run(const char* server_address)
{
 //create server socket
 UnixTcpServer server_socket;
 LOCAL_INFO("opening server_address["<<server_address<<"] ..."); 
 if(!server_socket.Open(server_address))
 {
  //TODO: print server log
  LOCAL_ERROR("create log print server socket ["<<server_address<<"] failed!");
  return;
 }
 LOCAL_INFO("open server socket ["<<server_address<<"] success");
 
 LOCAL_INFO("Enter service loop ...");
 while(true)
 {
  LOCAL_INFO("Calling Accept ...");
  //accept one client connect
  int client_socket=-1;
  client_socket=server_socket.Accept();
  LOCAL_INFO("socket["<<client_socket<<"] Accepted");
  
  if(client_socket<0)
  {
   LOCAL_ERROR("accept new client_socket failed! CONTINUED THE LOOP");
   continue;
  }

  LOCAL_INFO("Creating Service thread to serve client_socket["<<client_socket<<"]...");
  //create a thread to service the client
  RemoteTraceServiceThread* thread=new (nothrow)RemoteTraceServiceThread(client_socket);
  if(thread==NULL)
   {
    LOCAL_ERROR("Maybe no memory resource to create new service thread, client_socket["<<client_socket<<"] closed");
    close(client_socket);
    client_socket=-1;
    continue;
   }
  LOCAL_INFO("Set Service thread auto destory...");
  thread->EnableAutoDestroy(true);
  LOCAL_INFO("Starting service thread to run ...");
  thread->Start();
  
  LOCAL_INFO("started new service thread for client_socket ["<<client_socket<<"]");
 }
 
 LOCAL_INFO("Closing server_socket...");
 server_socket.Close();

 LOCAL_INFO("RemoteTraceServer closed normally");
}

 

//constructor
RemoteTraceServiceThread::RemoteTraceServiceThread(int socket_id)
{
 this->socket_id=socket_id;
}

RemoteTraceServiceThread::~RemoteTraceServiceThread()
 {
  if(this->socket_id>0)
   {
    close(this->socket_id);
    this->socket_id=-1;
   }
 }

//run service thread
void RemoteTraceServiceThread::Run(void)
{
 LOCAL_INFO("reading RemoteTraceConfig ...");
 //read configure info
 RemoteTraceConfig config;
 if(!config.Read(this->socket_id))
 {
  LOCAL_ERROR("read config of socket ["<<this->socket_id<<"] failed!");
  return;
 }
 LOCAL_INFO("read config of socket ["<<this->socket_id<<"] success");
 
 LOCAL_INFO("config[process_name="<<config.process_name<<",process_id="<<config.process_id \
  <<",max_log_file_number="<<config.print_config.max_log_file_number \
  <<",max_log_file_size="<<config.print_config.max_log_file_size \
  <<",max_log_buffered_lines="<<config.print_config.max_log_buffered_lines \
  <<",is_show="<<config.print_config.is_show \
  <<",is_error_show="<<config.print_config.is_error_show \
  <<",is_warning_show="<<config.print_config.is_warning_show \
  <<",is_info_show="<<config.print_config.is_info_show<<"]");
 
 //create a LocalTraceFile obj and config using reading configure info

 //set trace object attribution
 char log_path[MAX_FILE_NAME_LENGTH]="";
 char filename_template[MAX_FILE_NAME_LENGTH]="";
 char date_str[80]="";

 GetWllLogPath(log_path,sizeof(log_path));
 GetDateStr(date_str,sizeof(date_str));
 
 LOCAL_INFO("composing filename_template with log_path["<<log_path<<"],process_name["<<config.process_name<<"],process_id["<<config.process_id<<"],date["<<date_str<<"]...");
 //TODO: set trace file name template
 snprintf(filename_template,sizeof(filename_template),"%s/%s_%%s_%s_%%d.%d.log",log_path,config.process_name,date_str,config.process_id);
 LOCAL_INFO("filename_template["<<filename_template<<"] composed.");
 
 LocalTrace local_trace_file;
 
 LOCAL_INFO("openning local_trace_file with filename_template["<<filename_template<<"]...");
 if(!local_trace_file.Open(filename_template,config.print_config))
 {
  LOCAL_ERROR("open local_trace_file ["<<filename_template<<"] failed!");
  return;
 }
 LOCAL_INFO("open local_trace_file ["<<filename_template<<"] success");

 //local_trace_file.Config(config.print_config);
 //LOCAL_TRACE(INFO_TRACE, "configed local_trace_file ["<<filename_template<<"]");

 //send back OK info to let client known it's ready to print log
        //write(this->socket_id,"OK",strlen("OK")+1);
  LOCAL_INFO("writing back OK to client socket["<<this->socket_id<<"] ...");
 if( sizeof("OK") != write(this->socket_id,"OK",sizeof("OK")) )
  {
   LOCAL_ERROR("write back OK to client socket["<<this->socket_id<<"] failed, closing socket");
   close(this->socket_id);
   this->socket_id=-1;
   return;
  }
 LOCAL_INFO("write back OK to client socket_id ["<<this->socket_id<<"] completed");


 //handle log print request
 RemoteTraceMessage log_message;


 char old_date[80]="";
 char new_date[80]="";

 GetDateStr(old_date,sizeof(old_date));
 LOCAL_INFO("old_date=["<<old_date<<"]");
 
 LOCAL_INFO("Enter read-print message LOOP");
 while(true)
 {
  DEBUG_LOG("reading remark...");
  char remark=REMARK_MESSAGE;
  if(read(this->socket_id,&remark,1) != 1)
   {
    DEBUG_LOG("read remark error!");
    break;
   }
  
  if(remark==REMARK_MESSAGE)
   {
    if(!log_message.Read(this->socket_id))
    {
     DEBUG_LOG("read message error");
     break;
    }
    
    //split log file by date
    GetDateStr(new_date,sizeof(new_date));
    DEBUG_LOG("Get new_date=["<<new_date<<"]");
   
    DEBUG_LOG("comparing new_date["<<new_date<<"] and old_date["<<old_date<<"]...");
    if(strncmp(old_date,new_date,80)!=0)
    {
     LOCAL_INFO("log date changed,old_date=["<<old_date<<"],new_date=["<<new_date<<"], switching log file name...");
  
     LOCAL_INFO("closing old_date["<<old_date<<"] local_trace_file ...");
     local_trace_file.Close();
     LOCAL_INFO("old_date["<<old_date<<"] local_trace_file closed.");
     
     LOCAL_INFO("reparing new_date["<<new_date<<"] local_trace_file name ...");
     snprintf(filename_template,sizeof(filename_template),"%s/%s_%%s_%s_%%d.%d.log",log_path,config.process_name,new_date,config.process_id);
     LOCAL_INFO("new date filename_template["<<filename_template<<"] repared.");
         
     LOCAL_INFO("openning new_date["<<new_date<<"] local_trace_file["<<filename_template<<"]...");
         if(!local_trace_file.Open(filename_template,config.print_config))
        {
          LOCAL_ERROR("open local_trace_file ["<<filename_template<<"] failed!");
          return;
        }
        LOCAL_INFO("local_trace_file ["<<filename_template<<"] opened success");
       
     strncpy(old_date,new_date,sizeof(old_date));
     old_date[sizeof(old_date)-1]=0;
     //LOCAL_INFO("copy new_date["<<new_date<<"] to old_date["<<old_date<<"]...");
     //strcpy_s(old_date,sizeof(old_date),new_date);
     //old_date[sizeof(old_date)-1]=0;
     LOCAL_INFO("old_date["<<old_date<<"]==new_date["<<new_date<<"]");
  
     LOCAL_INFO("switch log file ["<<filename_template<<"] success");
    }
    
    DEBUG_LOG("WriteLog on local_trace_file : log_mask=["<<log_message.log_mask<<"],log_text=["<<log_message.log_text<<"]");
    if( !local_trace_file.WriteLog(log_message.log_mask,log_message.log_text,strlen(log_message.log_text)) )
     {
      LOCAL_ERROR("WriteLog on local_trace_file failed : log_mask=["<<log_message.log_mask<<"],log_text=["<<log_message.log_text<<"]");
     }
    DEBUG_LOG("WriteLog on local_trace_file success : log_mask=["<<log_message.log_mask<<"],log_text=["<<log_message.log_text<<"]");

   }
  else if(remark==REMARK_CONFIG)
   {
     LOCAL_INFO("reading RemoteTraceConfig from socket["<<this->socket_id<<"] ...");
     if(!config.Read(this->socket_id))
      {
       LOCAL_ERROR("read config error");
       break;
      }
     LOCAL_INFO("config read OK");

     LOCAL_INFO("config[process_name="<<config.process_name<<",process_id="<<config.process_id \
      <<",max_log_file_number="<<config.print_config.max_log_file_number \
      <<",max_log_file_size="<<config.print_config.max_log_file_size \
      <<",max_log_buffered_lines="<<config.print_config.max_log_buffered_lines \
      <<",is_show="<<config.print_config.is_show \
      <<",is_error_show="<<config.print_config.is_error_show \
      <<",is_warning_show="<<config.print_config.is_warning_show \
      <<",is_info_show="<<config.print_config.is_info_show<<"]");
      
     LOCAL_INFO("config the new log print config ..."); 
     local_trace_file.Config( config.print_config );
     LOCAL_INFO("new log print config configured.");    
      
   }
  
 }//end of while

 LOCAL_INFO("closing client socket_id["<<this->socket_id<<"]...");
 close(this->socket_id);
 LOCAL_INFO("client socket ["<<this->socket_id<<"] closed normally");

 LOCAL_INFO("closing local_trace_file ...");
 local_trace_file.Close();
 LOCAL_INFO("local_trace_file for client socket ["<<this->socket_id<<"] closed normally");
 
 this->socket_id=-1;
}


