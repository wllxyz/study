WllUtil.o: cpp/WllUtil.cpp /media/work/projects/Log/include/WllUtil.h \
 /media/work/projects/Log/include/WllConfigure.h
	c++ -c -o $@ $< -g -pg  -I/media/work/projects/Log/include  -DDEBUG
