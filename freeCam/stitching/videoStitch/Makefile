.PHONY: clean, distclean
CFLAGS=-Wall -g -O3
INCLUDE=$(shell pkg-config --cflags opencv)
LIBS=$(shell pkg-config --libs opencv)
OBJS=stitchutils.o stitchPlane.o
all:
	make stitchPlane
stitchPlane: main.o $(OBJS)
	$(CXX) $(CFLAGS) -o stitchPlane main.o $(OBJS) $(LIBS)
.cpp.o:
	$(CXX) $(INCLUDE) -c $< -o $@

clean:
	-rm *.o
distclean:
	-rm stitchPlane
