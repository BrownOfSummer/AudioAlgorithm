CPP=g++
INCLUDE=$(shell pkg-config --cflags opencv)
LIBS=$(shell pkg-config --libs opencv)
OBJS=auxfunc.o forwadditive.o invcomp.o stdafx.o
all:
	make main
main: main.o $(OBJS)
	$(CPP) -O3 -o warp main.o $(OBJS) $(LIBS)
.cpp.o:
	$(CPP) $(INCLUDE) -c $< -o $@
.c.o:
	gcc $(INCLUDE) -c $< -o $@
clean:
	rm *.o
	rm warp
