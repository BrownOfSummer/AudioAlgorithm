CC=gcc
CPP=g++
INCLUDE=-I.  -Wall -I/usr/local/Cellar/argtable/2.13/include -I/usr/local/Cellar/ffmpeg/3.3.2/include -DPROCESS_CC -DDONATOR  -g -O2  -MT
INCLUDE2=-DPACKAGE_NAME=\"Comskip\" -DPACKAGE_TARNAME=\"comskip\" -DPACKAGE_VERSION=\"0.81.098\" -DPACKAGE_STRING=\"Comskip\ 0.81.098\" -DPACKAGE_BUGREPORT=\"https://github.com/BrownOfSummer/tests/tree/master/com-logo-analyze/source\" -DPACKAGE_URL=\"\" -DPACKAGE=\"comskip\" -DVERSION=\"0.81.098\" -DHAVE_LIBM=1 -DHAVE_LIBPTHREAD=1
OBJS=comskip.o  mpeg2dec.o  platform.o  video_out_dx.o  608.o  ccextractor.o  encoding.o  general_loop.o  myth.o 
LIBS=-L/usr/local/Cellar/argtable/2.13/lib -largtable2 -L/usr/local/Cellar/ffmpeg/3.3.2/lib -lavutil -lavformat -lavcodec  -lpthread -lm

BINS=Comskip

all:
	make ComSkip
ComSkip: $(OBJS)
	$(CC) -g -O2 -o ComSkip $(OBJS) $(LIBS)

.cpp.o:
	$(CPP) $(INCLUDE) -c $< -o $@
.c.o:
	$(CC) $(INCLUDE2) $(INCLUDE) $@ -c -o $@ $<

clean:
	rm *.o
	rm $(BINS)
