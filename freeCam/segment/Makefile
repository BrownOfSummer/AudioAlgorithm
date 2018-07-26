.PHONY: clean, distclean
CFLAGS=-Wall -g -O3
INCLUDE=$(shell pkg-config --cflags opencv)
LIBS=$(shell pkg-config --libs opencv)
OBJS=segmentation.o
all:
	make main
main: main.o $(OBJS)
	$(CXX) $(CFLAGS) -o seg main.o $(OBJS) $(LIBS)

.cpp.o:
	$(CXX) $(INCLUDE) -c $< -o $@

clean:
	-rm *.o
distclean:
	-rm seg
