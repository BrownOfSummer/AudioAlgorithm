CC=g++
INCLUDE=$(shell pkg-config --cflags opencv)
LIBS=$(shell pkg-config --libs opencv)
BINS=image_align_smooth videostab videostabKalman
OBJS= 
all:
	make image_align_smooth
	make videostab
	make videostabKalman
image_align_smooth: image_align_smooth.o $(OBJS)
	$(CC) -O3 -o image_align_smooth image_align_smooth.o $(OBJS) $(LIBS)
videostab: videostab.o $(OBJS)
	$(CC) -O3 -o videostab videostab.o $(OBJS) $(LIBS)
videostabKalman: videostabKalman.o $(OBJS)
	$(CC) -O3 -o videostabKalman videostabKalman.o $(OBJS) $(LIBS)
videostab_nghia: videostab_nghia.o $(OBJS)
	$(CC) -O3 -o videostab_nghia videostab_nghia.o $(OBJS) $(LIBS)
.cpp.o:
	$(CC) $(INCLUDE) -c $< -o $@
.c.o:
	$(CC) $(INCLUDE) -c $< -o $@
clean:
	rm *.o
	rm $(BINS)
