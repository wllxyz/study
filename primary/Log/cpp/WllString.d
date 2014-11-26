WllString.o: cpp/WllString.cpp \
 /media/work/projects/Log/include/WllString.h
	c++ -c -o $@ $< -g -pg  -I/media/work/projects/Log/include  -DDEBUG
