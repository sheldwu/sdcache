GCC = g++
INCLUDE = -I ../util -I../common
CFLAG = -c -fPIC -O3 -Wall
LIBS=-lz -lpthread
OBJS = CacheMain.o Daemon.o ServerFrame.o ../util/HttpAnalyse.o ../util/HttpRequest.o
ddcache: $(OBJS)
	g++ -o $@   $(OBJS) $(INCLUDE) $(LIBS) -O3
.cpp.o: 
	g++ -o $@  $(CFLAG) $< $(INCLUDE)
clean:
	rm $(OBJS) ddcache
