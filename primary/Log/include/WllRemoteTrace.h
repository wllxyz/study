#ifndef CM_REMOTE_TRACER_H
#define CM_REMOTE_TRACER_H

#include <sys/types.h>
#include "WllThread.h"
#include "WllUnixSocket.h"
#include "WllTraceConfig.h"
#include "WllTraceInterface.h"

namespace Wll
{
 namespace Tool
 {
  class RemoteTraceClient : public Wll::Tool::TraceInterface
  {
  //by default use, you only need one instance for a process
  public:
   //get static instance
   static RemoteTraceClient* GetInstance(void);
   //constructor
   RemoteTraceClient(const char* server_address,const Wll::Tool::LogInnerTypes::RemoteTraceConfig& config);
   virtual ~RemoteTraceClient(){}
   //
   bool IsConnected(void)
   {
    return this->is_connected;
   }
   //write remote trace, should be multi-thread safe
   virtual bool Log(int mask,const char* msg);
   //configure trace print(SHOULD BE CONNECTED)
   virtual void Config(const Wll::Tool::LogInnerTypes::LogPrintConfig& config);

  private:
   static RemoteTraceClient* instance;      //static instance of RemoteTraceClient
   Wll::Tool::LogInnerTypes::RemoteTraceConfig config;    //log print config
   Wll::System::UnixTcpClient client_socket;     //client socket to connect remote server
   int connect_socket;        //remote tcp connect socket
   bool is_connected;        //connect flag
   pthread_mutex_t lock;
  };//end of class RemoteTracer

  class RemoteTraceServer
  {
  public:
   //constructor
   RemoteTraceServer(void);
   //run remote log server
   void Run(const char* server_address=Wll::Tool::LogInnerTypes::DEFAULT_REMOTE_LOG_SERVER_NAME);
  };//end of RemoteTraceServer

  class RemoteTraceServiceThread : public Wll::System::Thread
  {
  public:
   //constructor
   RemoteTraceServiceThread(int socket_id);
   ~RemoteTraceServiceThread();
   //run service thread
   virtual void Run(void);
  private:
   int socket_id; //service socket

  };//end of RemoteTraceServiceThread
 };//end of namespace Tool
};//end of namespace Wll

#endif //CM_REMOTE_TRACE_H

