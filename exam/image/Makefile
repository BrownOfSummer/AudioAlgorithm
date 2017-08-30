GCC=gcc
GPP=g++
INCLUDE=$(shell pkg-config --cflags opencv)
LIBS=$(shell pkg-config --libs opencv)
OBJS=
all:
	make scale_img
scale_img: scale_img.o $(OBJS)
	$(GPP) -O3 -o scale_img  scale_img.o $(OBJS) $(LIBS)

.cpp.o:
	$(GPP) $(INCLUDE) -c $< -o $@
.c.o:
	$(GCC) $(INCLUDE) -c $< -o $@

clean:
	rm *.o
	rm scale_img
