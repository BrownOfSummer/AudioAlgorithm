.PHONY: clean, distclean
CFLAGS=-Wall -g -O3
INCLUDE=$(shell pkg-config --cflags opencv)
LIBS=$(shell pkg-config --libs opencv)
OBJS= cropping.o
all:
	make mystitching
	make main
	make detailed
mystitching: mystitching.o
	$(CXX) $(CFLAGS) -o mystitching mystitching.o $(LIBS)
main: main.o $(OBJS)
	$(CXX) $(CFLAGS) -o main main.o $(OBJS) $(LIBS)
detailed: detailed.o $(OBJS)
	$(CXX) $(CFLAGS) -o detailed detailed.o $(OBJS) $(LIBS)
.cpp.o:
	$(CXX) $(INCLUDE) -c $< -o $@

clean:
	-rm *.o
distclean:
	-rm mystitching
	-rm detailed
	-rm main
