.PHONY: clean, distclean
CFLAGS=-Wall -g -O3
INCLUDE=$(shell pkg-config --cflags opencv)
LIBS=$(shell pkg-config --libs opencv)
OBJS=
all:
	make mystitching
mystitching: mystitching.o
	$(CXX) $(CFLAGS) -o mystitching mystitching.o $(LIBS)
.cpp.o:
	$(CXX) $(INCLUDE) -c $< -o $@

clean:
	-rm *.o
distclean:
	-rm mystitching
