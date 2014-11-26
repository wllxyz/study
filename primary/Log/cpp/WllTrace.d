WllTrace.o: cpp/WllTrace.cpp /media/work/projects/Log/include/WllTrace.h \
 /media/work/projects/Log/include/WllTraceConfig.h \
 /media/work/projects/Log/include/WllUtil.h \
 /media/work/projects/Log/include/WllRemoteTrace.h \
 /media/work/projects/Log/include/WllThread.h \
 /media/work/projects/Log/include/WllUnixSocket.h \
 /media/work/projects/Log/include/WllTraceInterface.h \
 /media/work/projects/Log/include/WllTraceConfig.h \
 /media/work/projects/Log/include/WllLocalTrace.h \
 /media/work/projects/Log/include/WllGroupFile.h \
 /media/work/projects/Log/include/WllUtil.h \
 /media/work/projects/Log/include/WllReload.h
	c++ -c -o $@ $< -g -pg  -I/media/work/projects/Log/include  -DDEBUG
