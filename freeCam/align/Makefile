.PHONY: clean distclean
CFLAGS=-Wall -g -O3
INCLUDE=$(shell pkg-config --cflags opencv)
LIBS=$(shell pkg-config --libs opencv)
OBJS=registration.o
CXX=g++
CC=gcc
all:
	make alignORB
	make test_wrap
alignORB: alignORB.o
	$(CXX) -o alignORB alignORB.o $(CFLAGS) $(LIBS)
test_wrap: test_wrap.o $(OBJS)
	$(CXX) $(CFLAGS) -o test_wrap test_wrap.o $(OBJS) $(LIBS)
.cpp.o:
	$(CXX) $(INCLUDE) -c $< -o $@
.c.o:
	$(CC) $(INCLUDE) -c $< -o $@
clean:
	-rm *.o
distclean:
	-rm alignORB
	-rm test_wrap
