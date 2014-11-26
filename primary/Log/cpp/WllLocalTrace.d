WllLocalTrace.o: cpp/WllLocalTrace.cpp \
 /media/work/projects/Log/include/WllLocalTrace.h \
 /media/work/projects/Log/include/WllGroupFile.h \
 /media/work/projects/Log/include/WllTraceConfig.h \
 /media/work/projects/Log/include/WllTraceInterface.h \
 /media/work/projects/Log/include/WllTrace.h \
 /media/work/projects/Log/include/WllUtil.h \
 /media/work/projects/Log/include/WllUtil.h \
 /media/work/projects/Log/include/WllGroupFile.h \
 /media/work/projects/Log/include/WllTraceConfig.h \
 /media/work/projects/Log/include/WllTraceInterface.h
	c++ -c -o $@ $< -g -pg  -I/media/work/projects/Log/include  -DDEBUG
