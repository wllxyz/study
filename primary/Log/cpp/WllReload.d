WllReload.o: cpp/WllReload.cpp \
 /media/work/projects/Log/include/WllReload.h \
 /media/work/projects/Log/include/WllThread.h \
 /media/work/projects/Log/include/WllUnixSocket.h \
 /media/work/projects/Log/include/WllTrace.h \
 /media/work/projects/Log/include/WllTraceConfig.h \
 /media/work/projects/Log/include/WllUtil.h
	c++ -c -o $@ $< -g -pg  -I/media/work/projects/Log/include  -DDEBUG
