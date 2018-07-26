.PHONY: clean, distclean
CFLAGS=-Wall -g -O3
INCLUDE=$(shell pkg-config --cflags opencv)
LIBS=$(shell pkg-config --libs opencv)
OBJS=
all:
	make interactiveColorDetect
	make interactiveColorSegment
interactiveColorDetect: interactiveColorDetect.o $(OBJS)
	$(CXX) $(CFLAGS) -o interactiveColorDetect interactiveColorDetect.o $(OBJS) $(LIBS)
interactiveColorSegment: interactiveColorSegment.o $(OBJS)
	$(CXX) $(CFLAGS) -o interactiveColorSegment interactiveColorSegment.o $(OBJS) $(LIBS)
.cpp.o:
	$(CXX) $(INCLUDE) -c $< -o $@

clean:
	-rm *.o
distclean:
	-rm interactiveColorDetect
	-rm interactiveColorSegment
