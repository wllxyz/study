WllConfigure.o: cpp/WllConfigure.cpp \
 /media/work/projects/Log/include/WllConfigure.h \
 /media/work/projects/Log/include/WllString.h \
 /media/work/projects/Log/include/WllTrace.h \
 /media/work/projects/Log/include/WllTraceConfig.h \
 /media/work/projects/Log/include/WllUtil.h
	c++ -c -o $@ $< -g -pg  -I/media/work/projects/Log/include  -DDEBUG
