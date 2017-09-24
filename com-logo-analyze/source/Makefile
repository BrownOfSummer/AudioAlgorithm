CC=gcc
CPP=g++
INCLUDE=-I /usr/local/include/
LIBS=
OBJS=608.o comskip.o ffmpeg_dxva2.o mpeg2dec.o platform.o encoding.o general_loop.o  myth.o  win32_pthread.o
BINS=

all:
	make mpeg2dec
	make ccextractor
	make video_out_dx
	make video_out_sdl
mpeg2dec: mpeg2dec.o $(OBJS)
	$(CC) -O3 -o mpeg2dec mpeg2dec.o $(OBJS) $(LIBS)
ccextractor: ccextractor.o $(OBJS)
	$(CC) -O3 -o ccextractor ccextractor.o $(OBJS) $(LIBS)
video_out_dx: video_out_dx.o $(OBJS)
	$(CC) -O3 -o video_out_dx video_out_dx.o $(OBJS) $(LIBS)
video_out_sdl: video_out_sdl.o $(OBJS)
	$(CC) -O3 -o video_out_sdl video_out_sdl.o $(OBJS) $(LIBS)

.cpp.o:
	$(CPP) $(INCLUDE) -c $< -o $@
.c.o:
	$(CC) $(INCLUDE) -c $< -o $@

clean:
	rm *.o
