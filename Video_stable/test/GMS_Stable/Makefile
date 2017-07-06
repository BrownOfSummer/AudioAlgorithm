CC=g++
INCLUDE=$(shell pkg-config --cflags opencv)
LIBS=$(shell pkg-config --libs opencv)
BINS=gms_match image_align_gms videostab_gms videostab videostab_imglist videostabKalman_imglist
OBJS=
all:
	make gms_match
	make image_align_gms
	make videostab_gms
	make videostab
	make videostab_imglist
	make videostabKalman_imglist
gms_match: demo.o $(OBJS)
	$(CC) -O3 -o gms_match demo.o $(OBJS) $(LIBS)
image_align_gms: image_align_gms.o $(OBJS)
	$(CC) -O3 -o image_align_gms image_align_gms.o $(OBJS) $(LIBS)
videostab_gms: videostab_gms.o $(OBJS)
	$(CC) -O3 -o videostab_gms videostab_gms.o $(OBJS) $(LIBS)
videostab: videostab.o $(OBJS)
	$(CC) -O3 -o videostab videostab.o $(OBJS) $(LIBS)
videostab_imglist: videostab_imglist.o $(OBJS)
	$(CC) -O3 -o videostab_imglist videostab_imglist.o $(OBJS) $(LIBS)
videostabKalman_imglist: videostabKalman_imglist.o $(OBJS)
	$(CC) -O3 -o videostabKalman_imglist videostabKalman_imglist.o $(OBJS) $(LIBS)
.cpp.o:
	$(CC) $(INCLUDE) -c $< -o $@
.c.o:
	$(CC) $(INCLUDE) -c $< -o $@
clean:
	rm *.o
	rm $(BINS)
