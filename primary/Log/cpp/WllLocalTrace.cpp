/*------------------------------------------------------*/
/* Local  Trace Print Class                             */
/*                                                      */
/* WllLocalTrace.cpp                                    */
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

#include "WllLocalTrace.h"

#include "WllTrace.h"
#include "WllUtil.h"
#include "WllGroupFile.h"
#include "WllTraceConfig.h"
#include "WllTraceInterface.h"
//#include "safe_str_lib.h"
using namespace Wll::Util;
using namespace Wll::Tool;
using namespace Wll::Tool::LogInnerTypes;

#include <cstring>
#include <pthread.h>
#include <new>
using namespace std;

bool InitalizeReload(void);


//implement local trace print manage

LocalTrace* LocalTrace::instance=NULL;

LocalTrace* LocalTrace::GetInstance(void)
{
        if(LocalTrace::instance==NULL)
        {
            DEBUG_LOG("LocalTrace::instance==NULL, try to lock create_instance_lock ...");
                static pthread_mutex_t create_instance_lock=PTHREAD_MUTEX_INITIALIZER;
                if(0==pthread_mutex_trylock(&create_instance_lock))
                {
                        //do the instance create
                        DEBUG_LOG("create_instance_lock locked,creating instance of LocalTrace ...");
   char date_buf[80]="";
   char process_name_buf[MAX_PROCESS_NAME]="";
   char filename_buf[MAX_FILE_NAME_LENGTH]="";
   char log_path[MAX_FILE_NAME_LENGTH]="";  
 
   GetProcessName(process_name_buf,sizeof(process_name_buf));
   GetDateStr(date_buf,sizeof(date_buf));
   GetWllLogPath(log_path,sizeof(log_path));
   
   DEBUG_LOG("repare filename: process_name["<<process_name_buf<<"],date["<<date_buf<<"],log_path["<<log_path<<"]");
   
   snprintf(filename_buf,sizeof(filename_buf),"%s/%s_%%s_%s_%%d.%d.log",log_path,process_name_buf,date_buf,getpid());
   
   DEBUG_LOG("call constructor with filename["<<filename_buf<<"]");
   
                        LocalTrace::instance=new(nothrow) LocalTrace(filename_buf);

   DEBUG_LOG("Initalizing Reload ...");
   if(!InitalizeReload())
   {
    DEBUG_LOG("InitalizeReload for local trace failed!");
   }
   DEBUG_LOG("Reload initalized.");
   
                        //release lock
                        pthread_mutex_unlock(&create_instance_lock);
                        DEBUG_LOG("create_instance_lock unlocked.");
                }
                else
                {
                    DEBUG_LOG("try create_instance_lock failed,waiting the first locked thread complete the instance creatation");
                        //wait the first locked thread complete the instance creatation
                        pthread_mutex_lock(&create_instance_lock);      //block to wait
                        DEBUG_LOG("create_instance_lock LOCKED");
                        DEBUG_LOG("do nothing...");
                        //do nothing
                        pthread_mutex_unlock(&create_instance_lock);
                        DEBUG_LOG("create_instance_lock UNLOCKED");
                }
        }
       
        DEBUG_LOG( ((LocalTrace::instance==NULL)?"LocalTrace::instance==NULL":"LocalTrace::instance!=NULL") );

        return LocalTrace::instance;
}


//constructor  
LocalTrace::LocalTrace()
{
 this->info_trace_buffered_lines=0;
 this->warning_trace_buffered_lines=0;
 this->error_trace_buffered_lines=0;
 
 this->is_opened=false;
}

//constructor
LocalTrace::LocalTrace(const char* filename_template)
{
 this->info_trace_buffered_lines=0;
 this->warning_trace_buffered_lines=0;
 this->error_trace_buffered_lines=0;
  
 this->is_opened=false;
 this->Open(filename_template,*LogPrintConfig::GetInstance());
}

LocalTrace::~LocalTrace()
{
 this->Close();
}

//open a trace obj to write local disk log
//format of filename_template : "<prefix>%s<suffix>%d" = "<prefix>info<suffix>%d" "<prefix>warning<suffix>%d" "<prefix>error<suffix>%d"
//the %s will be substitute by "info" "warning" and "error"
//the %d MUST BE AFTER THE %s
bool LocalTrace::Open(const char* filename_template,const LogPrintConfig& config)
{
 //we not use auto-close when open another file when not closed
 DEBUG_LOG("check whether LocalTrace is opened, this->is_opened=["<<this->is_opened<<"]");
 if(this->is_opened)
  {
   DEBUG_LOG("LocalTrace is already opened, return true directly, not openning new trace files");
   return true;
  }
 DEBUG_LOG("LocalTrace not opened, try open it...");

 DEBUG_LOG("CheckFileNameTemplate(filename_template="<<filename_template<<")..."); 
 //check the format of filename_template
 if(!CheckFileNameTemplate(filename_template))
 {
  TERM_ERROR("filename_template["<<filename_template<<"] check failed, return false");
  return false;
 }

 this->log_print_config=config;
 
 //copy filename_template to tmp
 char tmp[MAX_FILE_NAME_LENGTH]="";
 
 strncpy(tmp,filename_template,sizeof(tmp)-1);
 tmp[sizeof(tmp)-1]=0;
 
 //replace "%d" as "%s"
 char* p=strstr(tmp,"%d");
 
 DEBUG_LOG("replacing %d as %s in tmp["<<tmp<<"]...");

 if(p==NULL)
  {
   TERM_ERROR("%d NOT found in tmp["<<tmp<<"], return false");
   return false;
  }
 DEBUG_LOG("%d found in tmp["<<tmp<<"] at the index of ["<<(p-tmp)<<"]");
 
 *(p+1)='s';
 
 DEBUG_LOG("%d replaced as %s, now tmp=["<<tmp<<"]");
 
 DEBUG_LOG("composing info_trace_filename ...");
 //get all_trace_file name
 char info_trace_filename[MAX_FILE_NAME_LENGTH]="";
 snprintf(info_trace_filename, sizeof(info_trace_filename), tmp, "info","%d");
 DEBUG_LOG("info_trace_filename["<<info_trace_filename<<"] composed.");
 
 DEBUG_LOG("Open info_trace_file with [info_trace_filename="<<info_trace_filename<<",open_mode="<<GroupFile::OPEN_APPEND \
  <<",max_log_file_number="<<config.max_log_file_number<<",max_log_file_size="<<config.max_log_file_size \
  <<",max_log_buffered_bytes="<<config.max_log_buffered_lines*MAX_LOG_LENGTH<<"...");
 //open all_trace_file
 if(!this->info_trace_file.Open(info_trace_filename,GroupFile::OPEN_APPEND,config.max_log_file_number,config.max_log_file_size,config.max_log_buffered_lines*MAX_LOG_LENGTH))
 {
  TERM_ERROR("open info_trace_file ["<<info_trace_filename<<"] failed, return false");
  return false;
 }
 DEBUG_LOG("info_trace_file["<<info_trace_filename<<"] opened success");
 
 DEBUG_LOG("composing warning_trace_file_name ...");
 //get warning_trace_file name
 char warning_trace_filename[MAX_FILE_NAME_LENGTH]="";
 snprintf(warning_trace_filename,sizeof(warning_trace_filename),tmp,"warning","%d");
 DEBUG_LOG("warning_trace_filename["<<warning_trace_filename<<"] composed.");
 
 DEBUG_LOG("Open warning_trace_file with [warning_trace_filename="<<warning_trace_filename<<",open_mode="<<GroupFile::OPEN_APPEND \
  <<",max_log_file_number="<<config.max_log_file_number<<",max_log_file_size="<<config.max_log_file_size \
  <<",max_log_buffered_bytes="<<config.max_log_buffered_lines*MAX_LOG_LENGTH<<"...");
 //open warning_trace_file
 if(!this->warning_trace_file.Open(warning_trace_filename,GroupFile::OPEN_APPEND,config.max_log_file_number,config.max_log_file_size,config.max_log_buffered_lines*MAX_LOG_LENGTH))
 {
  TERM_ERROR("open warning_trace_file ["<<warning_trace_filename<<"] failed, return false");
  return false;
 }
 DEBUG_LOG("warning_trace_file["<<warning_trace_filename<<"] opened success");
 
 DEBUG_LOG("composing error_trace_file_name ...");
 //get error_trace_file name
 char error_trace_filename[MAX_FILE_NAME_LENGTH]="";
 snprintf(error_trace_filename,sizeof(error_trace_filename),tmp,"error","%d");
 DEBUG_LOG("error_trace_filename["<<error_trace_filename<<"] composed.");
 
 DEBUG_LOG("Open error_trace_file with [error_trace_filename="<<error_trace_filename<<",open_mode="<<GroupFile::OPEN_APPEND \
  <<",max_log_file_number="<<config.max_log_file_number<<",max_log_file_size="<<config.max_log_file_size \
  <<",max_log_buffered_bytes="<<config.max_log_buffered_lines*MAX_LOG_LENGTH<<"...");
 //open error_trace_file name
 if(!this->error_trace_file.Open(error_trace_filename,GroupFile::OPEN_APPEND,config.max_log_file_number,config.max_log_file_size,config.max_log_buffered_lines*MAX_LOG_LENGTH))
 {
  TERM_ERROR("open error_trace_file ["<<error_trace_filename<<"] failed, return false");
  return false;
 }
 DEBUG_LOG("error_trace_file["<<error_trace_filename<<"] opened success");
 
 //If all right, return true,else return false
 this->is_opened=true;
 DEBUG_LOG("Mark this->is_opened=["<<this->is_opened<<"]");
 
 return true;
}

//close all opened trace files 
void LocalTrace::Close() 
{
 if(this->is_opened)
 {
  //close all_trace_file
  this->info_trace_file.Close();
  //close warning_trace_file
  this->warning_trace_file.Close();
  //close error_trace_file
  this->error_trace_file.Close();
  this->is_opened = false;
 }
}  

//print log message on disk file 
bool LocalTrace::WriteLog(int mask,const char* message,size_t len)
{
 const char* msg=message;
 char buf[MAX_LOG_LENGTH]="";

 DEBUG_LOG("Test LocalTrace is opened or not ...");
 if(this->is_opened)
 {
  DEBUG_LOG("LocalTrace is opened, this->is_opened=["<<this->is_opened<<"]");
  
  DEBUG_LOG("Test the message["<<message<<"] is truncated or not ...");
  //when truncate string, to keep string end by '\n'
  if(message[len-1]!='\n')
  {
   DEBUG_LOG("message["<<message<<":len="<<len<<"] not end with new line");
   DEBUG_LOG("copying message to buf ...");
   memcpy(buf,message,len);
   buf[sizeof(buf)-1]=0;
   DEBUG_LOG("after copy : buf=["<<buf<<"]");
   
   DEBUG_LOG("replace the last character of message with new line");
   buf[len-1]='\n';
   DEBUG_LOG("buf["<<buf<<"] with new line at tail");
   
   DEBUG_LOG("switch msg to point buf ...");
   DEBUG_LOG("before switch : msg["<<msg<<"]==message["<<message<<"]");
   msg=buf; 
   DEBUG_LOG("after switch : msg["<<msg<<"]==buf["<<buf<<"]");
  }

  bool ret=true;
  //switch mask:
  DEBUG_LOG("Test warning trace show or not...");
  //if warning_trace is on: write warning_trace_file message
  if(this->log_print_config.IsWarningShow(mask))
  {
   DEBUG_LOG("warning show enabled, showing warning trace ...");
   ret = ret && (len == this->warning_trace_file.Write(msg,len));
   
   DEBUG_LOG("this->warning_trace_buffered_lines=["<<this->warning_trace_buffered_lines<<"]");
   this->warning_trace_buffered_lines++;
   DEBUG_LOG("this->warning_trace_buffered_lines=["<<this->warning_trace_buffered_lines<<"]");
   
   if(this->warning_trace_buffered_lines>=this->log_print_config.max_log_buffered_lines)
    {
     DEBUG_LOG("max buffered warning lines reached,warning_trace_buffered_lines["<<this->warning_trace_buffered_lines<<"]>=max_log_buffered_lines["<<this->log_print_config.max_log_buffered_lines<<"]");

     DEBUG_LOG("flushing warning_trace_file ...");
     this->warning_trace_file.Flush();
     DEBUG_LOG("warning_trace_file flushed.");
     
     DEBUG_LOG("reset warning_trace_buffered_lines=0...");
     this->warning_trace_buffered_lines=0;
     DEBUG_LOG("this->warning_trace_buffered_lines=["<<this->warning_trace_buffered_lines<<"]");
    }
  }
  
  DEBUG_LOG("Test error trace show or not ...");
  //if error_trace is on: write error_trace_file message
  if(this->log_print_config.IsErrorShow(mask))
  {
   DEBUG_LOG("error show enabled, showing error trace ...");
   ret = ret && (len == this->error_trace_file.Write(msg,len));
   
   DEBUG_LOG("this->error_trace_buffered_lines=["<<this->error_trace_buffered_lines<<"]");
   this->error_trace_buffered_lines++;
   DEBUG_LOG("this->error_trace_buffered_lines=["<<this->error_trace_buffered_lines<<"]");
   
   if(this->error_trace_buffered_lines>=this->log_print_config.max_log_buffered_lines)
    {
     DEBUG_LOG("max buffered error lines reached,error_trace_buffered_lines["<<this->error_trace_buffered_lines<<"]>=max_log_buffered_lines["<<this->log_print_config.max_log_buffered_lines<<"]");

     DEBUG_LOG("flushing error_trace_file ...");
     this->error_trace_file.Flush();
     DEBUG_LOG("error_trace_file flushed.");

     DEBUG_LOG("reset error_trace_buffered_lines=0...");
     this->error_trace_buffered_lines=0;
     DEBUG_LOG("this->error_trace_buffered_lines=["<<this->error_trace_buffered_lines<<"]");
    }   
  }
  
  DEBUG_LOG("Test info trace show or not ...");
  //Write all_trace_file message
  if(this->log_print_config.IsInfoShow(mask))
  {
   DEBUG_LOG("info show enabled, showing info trace ...");
   ret = ret && (len == this->info_trace_file.Write(msg,len));
   
   DEBUG_LOG("this->info_trace_buffered_lines=["<<this->info_trace_buffered_lines<<"]");
   this->info_trace_buffered_lines++;
   DEBUG_LOG("this->info_trace_buffered_lines=["<<this->info_trace_buffered_lines<<"]");
   
   if(this->info_trace_buffered_lines>=this->log_print_config.max_log_buffered_lines)
    {
     DEBUG_LOG("max buffered info lines reached,info_trace_buffered_lines["<<this->info_trace_buffered_lines<<"]>=max_log_buffered_lines["<<this->log_print_config.max_log_buffered_lines<<"]");

     DEBUG_LOG("flushing info_trace_file ...");
     this->info_trace_file.Flush();
     DEBUG_LOG("info_trace_file flushed.");
     
     DEBUG_LOG("reset info_trace_buffered_lines=0...");
     this->info_trace_buffered_lines=0;
     DEBUG_LOG("this->info_trace_buffered_lines=["<<this->info_trace_buffered_lines<<"]");
    }   
  }
  DEBUG_LOG("return value="<<ret);
  return ret;
 }
 else
 {
  DEBUG_LOG("LocalTrace is NOT opened, return false");
  return false;
 }
}

bool LocalTrace::Log(int mask,const char* msg)
{
  if(!this->is_opened)
   {
    DEBUG_LOG("LocalTrace is not opened, return false");
    return false;
   }
  DEBUG_LOG("confirmed that LocalTrace is opened before do logging");
  
  if( !this->log_print_config.IsErrorShow(mask) && !this->log_print_config.IsWarningShow(mask) && !this->log_print_config.IsInfoShow(mask) )
   {
    DEBUG_LOG("all log print disabled,NOT print anything,return true");
    return true;
   }
  
  static char old_date[80]="";
  char new_date[80]="";
  char process_name[MAX_PROCESS_NAME]="";
  char new_file_name[MAX_FILE_NAME_LENGTH]="";

    char log_text[MAX_LOG_LENGTH]="";
    char date_time_str[80]="";
               
  DEBUG_LOG("check inital variables : old_date["<<old_date<<"],new_date["<<new_date<<"],process_name["<<process_name<<"],new_file_name["<<new_file_name<<"]");
  DEBUG_LOG("check inital variables : log_text["<<log_text<<"],date_time_str["<<date_time_str<<"]");

  DEBUG_LOG("GetDateStr for new_date...");
  GetDateStr(new_date,sizeof(new_date));
  DEBUG_LOG("new_date["<<new_date<<"] got");
  
  DEBUG_LOG("GetDateTimeStr for date_time_str ...");
    GetDateTimeStr(date_time_str,sizeof(date_time_str));
    DEBUG_LOG("date_time_str["<<date_time_str<<"] got");
   
    DEBUG_LOG("composing log_text["<<log_text<<"] with msg["<<msg<<"]...");
    snprintf(log_text,sizeof(log_text),"[%s pid=%ld tid=%ld] %s\n",date_time_str, long(getpid()), long(pthread_self()), msg);
    DEBUG_LOG("log_text["<<log_text<<"] composed.");
  
  bool ret=true; //record error status

    static pthread_mutex_t lock=PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_lock(&lock);

  DEBUG_LOG("write log lock locked");
  
  DEBUG_LOG("Testing new_date["<<new_date<<"] and old_date["<<old_date<<"] ...");
  if(strncmp(new_date,old_date,80)!=0)
  {
   DEBUG_LOG("new_date["<<new_date<<"] != old_date["<<old_date<<"]");
   DEBUG_LOG("closing LocalTrace ...");
   this->Close();
   DEBUG_LOG("LocalTrace closed.");
  
   char log_path[MAX_FILE_NAME_LENGTH]="";
   GetWllLogPath(log_path,sizeof(log_path));
   GetProcessName(process_name,sizeof(process_name));
   
   DEBUG_LOG("composing new_file_name["<<new_file_name<<"] with log_path["<<log_path<<"],process_name["<<process_name<<"]");
   snprintf(new_file_name,sizeof(new_file_name),"%s/%s_%%s_%s_%%d.%d.log",log_path,process_name,new_date,getpid());
   DEBUG_LOG("new_file_name["<<new_file_name<<"] composed.");
   
   DEBUG_LOG("Opening new LocalTrace with new_file_name["<<new_file_name<<"] ...");  
   ret = ret && this->Open(new_file_name,*LogPrintConfig::GetInstance());
   DEBUG_LOG( (ret?"open success":"open failed") );
   strncpy(old_date,new_date,sizeof(old_date));
   old_date[sizeof(old_date)-1]=0;
  }
  DEBUG_LOG("config log print config ...");
  this->Config( *LogPrintConfig::GetInstance() );
  DEBUG_LOG("log print configed.");
  
  DEBUG_LOG("writing log["<<log_text<<"] to disk files ...");
  ret = ret && this->WriteLog(mask,log_text,strnlen(log_text,sizeof(log_text)));
    DEBUG_LOG( "ret=["<<ret<<"]" );

  pthread_mutex_unlock(&lock);
    DEBUG_LOG("write log lock unlocked.");           
  
  return ret;
}


//configure log print enable/disable   
void LocalTrace::Config(const LogPrintConfig& configs)
{
 DEBUG_LOG("Testing config is new or not ...");
 DEBUG_LOG("configs : max_log_file_number="<<configs.max_log_file_number \
  <<",max_log_file_size="<<configs.max_log_file_size \
  <<",max_log_buffered_lines="<<configs.max_log_buffered_lines \
  <<",is_show="<<configs.is_show \
  <<",is_error_show="<<configs.is_error_show \
  <<",is_warning_show="<<configs.is_warning_show \
  <<",is_info_show="<<configs.is_info_show);
 DEBUG_LOG("this->log_print_config : max_log_file_number="<<this->log_print_config.max_log_file_number \
  <<",max_log_file_size="<<this->log_print_config.max_log_file_size \
  <<",max_log_buffered_lines="<<this->log_print_config.max_log_buffered_lines \
  <<",is_show="<<this->log_print_config.is_show \
  <<",is_error_show="<<this->log_print_config.is_error_show \
  <<",is_warning_show="<<this->log_print_config.is_warning_show \
  <<",is_info_show="<<this->log_print_config.is_info_show);  
 if(!(this->log_print_config==configs))
 {
  DEBUG_LOG("configs is new,updating current used log_print_config ...");
  DEBUG_LOG("switching LocalTrace::Config...");

  //copy the configure items about enable/disable log print
  this->log_print_config=configs;
  DEBUG_LOG("config info_trace_file ...");
  //config all_trace_file's files/size/buffer configure
  this->info_trace_file.Config(this->log_print_config.max_log_file_number, this->log_print_config.max_log_file_size,this->log_print_config.max_log_buffered_lines*MAX_LOG_LENGTH);
  DEBUG_LOG("config warning_trace_file ...");
  //config warning_trace_file's files/size/buffer configure
  this->warning_trace_file.Config(this->log_print_config.max_log_file_number, this->log_print_config.max_log_file_size,this->log_print_config.max_log_buffered_lines*MAX_LOG_LENGTH);
  DEBUG_LOG("config error_trace_file ...");
  //config error_trace_file's files/size/buffer configure
  this->error_trace_file.Config(this->log_print_config.max_log_file_number, this->log_print_config.max_log_file_size,this->log_print_config.max_log_buffered_lines*MAX_LOG_LENGTH);

  DEBUG_LOG("LocalTrace::Config log_print_config switched");
 }
}


//test whether the filename_template is valid
//filename_template format:
//"<prefix>%s<middle>%d<suffix>"
bool LocalTrace::CheckFileNameTemplate(const char* filename_template)
{
 //must have a "%s"
 const char* p1=strstr(filename_template,"%s");
 if(p1==NULL) return false;
 
 //must have a "%d"
 const char* p2=strstr(filename_template,"%d");
 if(p2==NULL) return false;
 
 //"%s" must before "%d"
 if(p1>p2)
  {
   DEBUG_LOG("THE %s MUST before THE %d in filename_template["<<filename_template<<"]");
   return false;
  }
 
 //not have other "%" except one "%s" and one "%d"
 const char* p=strchr(filename_template,'%');
 DEBUG_LOG("found first % in filename_template["<<filename_template<<"] at index of ["<<(p-filename_template)<<"]");
 p=strchr(p+1,'%');
 DEBUG_LOG("found second % in filename_template["<<filename_template<<"] at index of ["<<(p-filename_template)<<"]");
 p=strchr(p+1,'%');
 if(p!=NULL)
  {
   DEBUG_LOG("found another % in filename_template["<<filename_template<<"] at index of ["<<(p-filename_template)<<"]");
   DEBUG_LOG("THE filename_template should only have one %d and one %s, no more %");
   return false;
  }
 DEBUG_LOG("filename_template["<<filename_template<<"] checked to be OK, return true");

 return true;
}

 


 
