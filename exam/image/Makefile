GCC=gcc
GPP=g++
INCLUDE=$(shell pkg-config --cflags opencv)
LIBS=$(shell pkg-config --libs opencv)
OBJS=
all:
	#make scale_img
	make scale_img2
scale_img: scale_img.o $(OBJS)
	$(GPP) -O3 -o scale_img  scale_img.o $(OBJS) $(LIBS)
scale_img2: scale_img2.o $(OBJS)
	$(GPP) -O3 -o scale_img2 scale_img2.o $(OBJS) $(LIBS)

.cpp.o:
	$(GPP) $(INCLUDE) -c $< -o $@
.c.o:
	$(GCC) $(INCLUDE) -c $< -o $@

clean:
	rm *.o
	rm scale_img
